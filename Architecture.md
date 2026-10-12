# Odyssey Architecture

## 1. Targets

```
            ┌──────────────────────┐
            │  Odyssey (static lib)│  Core, Platform, Renderer, Scene, Asset,
            └─────────┬────────────┘  Physics, Audio, Script, Project
        ┌─────────────┼───────────────┬──────────────────┐
┌───────▼───────┐ ┌───▼────────────┐ ┌▼──────────────┐ ┌─▼────────────┐
│ Odyssey-Editor│ │ Odyssey-Runtime│ │ Odyssey-Tests │ │ odyssey-cli  │
│ ImGui, gizmo, │ │ player, no     │ │ doctest       │ │ talks JSON-  │
│ commands, RPC │ │ editor code    │ │               │ │ RPC to editor│
└───────────────┘ └────────────────┘ └───────────────┘ └──────────────┘
```

The editor is a separate executable. The runtime links only the engine, so exported games are
editor-free by construction rather than by `#ifdef`.

## 2. Core loop

```
Application::Run
  PollEvents → Input snapshot
  accumulator += dt
  while accumulator >= fixedDt:          (default 1/60 s)
      ScriptEngine::OnFixedUpdate
      PhysicsWorld::Step(fixedDt)        → collision events queued
      ScriptEngine::DispatchCollisionEvents
  ScriptEngine::OnUpdate(dt)
  Scene::FlushPendingOperations          (deferred create/destroy, prefab spawns)
  AudioEngine::Update                    (listener + source transforms)
  SceneRenderer::Render(alpha)           (interpolated transforms)
  Editor/ImGui layer (editor only)
  Present
```

## 3. Scene and ECS

EnTT registry owned by `Scene`. `Entity` is a thin `(entt::entity, Scene*)` wrapper. Every entity has
`IDComponent` (UUID), `TagComponent`, `TransformComponent`, and `RelationshipComponent`
(parent/children by UUID).

Initial component set:

| Component | Purpose |
|---|---|
| Transform | Translation, rotation (quat), scale; world matrix cached and dirty-flagged |
| Relationship | Hierarchy |
| Camera | Perspective/ortho, primary flag, exposure |
| MeshRenderer | Mesh asset handle, submesh index, material overrides, cast/receive shadows |
| DirectionalLight / PointLight / SpotLight | Color, intensity (physical units), range, shadow settings |
| SkyLight | HDRI environment asset, intensity, rotation |
| PostProcessVolume | Tonemapper, exposure, GTAO, bloom settings (global for v1) |
| RigidBody | Static/Dynamic/Kinematic, mass, damping, gravity factor, locks, layer |
| BoxCollider / SphereCollider / CapsuleCollider / MeshCollider | Shape, offset, friction, restitution, trigger flag |
| AudioSource | Clip, volume, pitch, loop, spatial, play-on-awake |
| AudioListener | |
| Script | Lua script asset, exposed fields (serialized overrides) |
| PrefabInstance | Source prefab UUID, override list |

Scenes, prefabs, and projects serialize to YAML (`.oscene`, `.oprefab`, `.oproj`). Each file carries
a `Version` field and the serializer owns migrations.

## 4. Assets

`AssetHandle` = UUID. `AssetRegistry` (`Assets/AssetRegistry.yaml`) maps handle → path + type.
Importers:
- **glTF/GLB** (cgltf): meshes (positions, normals, tangents generated with MikkTSpace if absent, UV0),
  metal-roughness PBR materials, textures (sRGB vs linear by usage), node hierarchy → prefab.
- **Textures**: PNG/JPG/TGA via stb_image; GPU mip generation; BC7 compression in the export step.
- **HDRI**: `.hdr` (stb_image) and `.exr` (tinyexr), equirectangular, as downloaded from Poly Haven.
  Users drop files in `Assets/`; the engine never downloads at runtime.

Imported data is cached in `Cache/` and keyed by source hash, so imports are deterministic and skippable.

## 5. Renderer

NVRHI on Vulkan. `RenderDevice` owns the device, swapchain and frames in flight (2). `SceneRenderer`
executes a fixed, explicit pass list (a render graph is not needed for v1):

1. **Depth + normal prepass**: depth and view-space normals (RG16F octahedral).
2. **Shadow pass**: directional light uses 4 cascades with stable (texel-snapped) projections. Spot
   lights get a single map each; point lights get cubemaps, all from an atlas budget.
3. **GTAO**: ground-truth ambient occlusion from depth and normals at half resolution, spatial
   denoise, then depth-aware bilateral upsample. Applied only to indirect lighting.
4. **Forward PBR opaque**: Cook-Torrance GGX, Smith height-correlated, Schlick Fresnel; metal-roughness
   workflow plus normal, occlusion and emissive maps. Soft shadows use PCSS: blocker search followed
   by variable-kernel Poisson PCF, with a cascade blend band.
5. **Sky**: the environment cubemap.
6. **Transparent forward**: sorted back to front.
7. **Post**: bloom (dual-filter downsample/upsample), exposure (manual or auto-histogram), then
   tonemapping (ACES fitted and AgX selectable) to the sRGB swapchain.
8. **Editor overlays**: grid, selection outline, gizmos, ImGui.

The scene color target is `RGBA16F` throughout.

**IBL**: the HDRI loads into an equirect texture. Compute shaders convert it to a cubemap
(1024²), an irradiance map (32² cube, or L2 spherical harmonics), and a GGX prefiltered specular cube
(256², 6 mips, importance-sampled). The split-sum BRDF LUT is generated once. Results are cached per
HDRI.

Shaders are GLSL with `#include` support, compiled at build time to SPIR-V and embedded. Hot reload
is available in the editor only.

## 6. Physics (Jolt)

`PhysicsWorld` wraps a `JPH::PhysicsSystem`. Bodies are created when a scene starts playing, from
RigidBody + Collider components. There is a body↔UUID map, and a contact listener queues
enter/stay/exit for both collisions and triggers. Script API: forces, impulses, velocities, teleport,
raycast and shape casts, and layer masks. Simulation is fixed-step and deterministic for identical
inputs, which the tests rely on.

## 7. Scripting (Lua 5.4 + sol2)

- One Lua state per playing scene, sandboxed: no `io`, `os.execute`, or `loadfile`; `require` is
  restricted to the project's `Scripts/`.
- Script file returns a table with optional `OnCreate`, `OnUpdate(dt)`, `OnFixedUpdate(dt)`,
  `OnDestroy`, `OnCollisionEnter/Exit(other)`, `OnTriggerEnter/Exit(other)` and a `Fields` table of
  editor-exposed properties (number, int, bool, string, vec2/3/4, color, Entity, Prefab, Asset).
- `self.Entity` is an `Entity` userdata. Component access returns proxy userdata that reads and
  writes through to the registry on each access, so there is no stale copy.
- Global modules: `Scene`, `Input`, `Physics`, `Audio`, `Time`, `Log`, `Math` (vec/quat types
  bound from glm), `Application`, plus `Test` (FeatureTest builds only).
- Every binding is registered through `ScriptGlue::Register(name, ...)`, which records it in the API
  registry used for coverage checks and to generate `docs/ScriptingAPI.md`.
- Errors in scripts are caught, logged with file and line, and disable that script instance. They
  never crash the engine.

## 8. Editor and automation

Panels: Viewport (ImGuizmo translate/rotate/scale with snapping), Scene Hierarchy, Inspector, Content
Browser, Console, Project Settings, Play/Pause/Step toolbar. Play mode copies the scene, and
stopping restores it.

**Command layer.** Every mutation is an `EditorCommand` (Execute/Undo, serializable to JSON), for
example `CreateEntity`, `SetComponentField`, `AddComponent`, `ImportAsset`, `SaveScene`. The UI,
undo stack and automation all go through this one path.

**Automation server.** `Odyssey-Editor --automation-port 47100` (localhost only, token in
`Projects/<P>/.odyssey/automation-token`) exposes JSON-RPC 2.0. It offers every command, plus queries
(`scene.list_entities`, `entity.get`, `asset.list`, `api.describe` for the full schema), plus control
(`play`, `stop`, `step_frames`, `capture_viewport` → PNG path, `read_console`). It also runs headless:
`Odyssey-Editor --headless --project X --run script.json`. `odyssey-cli` is a thin client. With it, an
agent can create a project, write Lua, place entities, play, observe console output and screenshots,
iterate, and export.

## 9. Export

`odyssey-cli export --project X --out dist/` (or File → Export) does the following:
1. Validates the project.
2. Cooks assets (compressed textures, packed meshes) into `Game.opak`.
3. Copies the prebuilt `Odyssey-Runtime` (Dist configuration) for the host platform, renamed to the
   game name.
4. Writes `Game.oproj` (start scene, window settings).

On macOS it produces a `.app` bundle with MoltenVK. On Windows and Linux it produces a folder.
Cross-compiling is out of scope, so each platform exports on its own CI runner.

## 10. Platform notes

- **macOS**: enable `VK_KHR_portability_enumeration` and `VK_KHR_portability_subset`. GTAO and
  shadows must stay within MoltenVK limits, which CI checks on an Apple Silicon runner.
- **Linux**: X11 and Wayland via GLFW 3.4. CI uses Ubuntu 24.04 with lavapipe for GPU tests.
- **Windows**: MSVC 2022 and clang-cl.
