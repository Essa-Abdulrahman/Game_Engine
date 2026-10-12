# Odyssey Roadmap

Each phase ends with an engine that builds, passes every test on all three platforms, and is a strict superset of
the previous phase. A phase is complete only when **every** exit criterion holds; partial work stays on a branch
or behind a clearly unfinished, unexposed module. Update the status column and the phase notes as work lands.

| Phase | Name | Status |
|---|---|---|
| 0 | Foundation | Done (pending first CMake run, see notes) |
| 1 | Platform and RHI | Not started |
| 2 | Scene and ECS | Not started |
| 3 | Assets | Not started |
| 4 | Renderer | Not started |
| 5 | Editor | Not started |
| 6 | Physics | Not started |
| 7 | Scripting | Not started |
| 8 | Audio | Not started |
| 9 | Agent control | Not started |
| 10 | Export and runtime | Not started |
| 11 | FeatureTest and hardening | Not started |

Phases 6, 7 and 8 depend only on Phase 2 (plus Phase 3 for assets) and may proceed in any order after Phase 5.

---

## Phase 0: Foundation

**Delivered:** repository layout, CMake build (Debug/Release/Dist, presets, warnings as errors, sanitizers),
GitHub Actions CI (Windows, macOS, Ubuntu 24 across all configurations plus sanitizers and a format check),
`.clang-format`, `.clang-tidy`, `tools/check_style.py`, `AGENTS.md`, skills, architecture and roadmap docs, and the
Core module (`Base`, `Assert`, `Log`, `UUID`, `Buffer`, `FileSystem`, `CommandLine`, `Environment`, `KeyCodes`,
`Events`, `Layer`/`LayerStack`, `Hash`, `StringUtils`, `Timer`, `Timestep`) with 79 unit test cases.

**Verified:** GCC 13 builds with `-Werror` in Debug (AddressSanitizer, UndefinedBehaviorSanitizer and leak
detection), Release, Dist and ThreadSanitizer; all tests pass in each. The fatal-verify path was confirmed to log
and abort in Dist. `check_style.py` passes and was self-tested against deliberately bad files.

**Open items (do these first, in order):**

1. Run `cmake --preset debug && cmake --build --preset debug && ctest --preset debug` on each platform. The CMake
   files were written in an environment without CMake, so this is their first real run. Fix anything that fails.
2. Run `python3 tools/check_style.py --fix` with clang-format 18 and commit the result. The code was formatted by
   hand to match `.clang-format`; small differences are expected.
3. Run `clang-tidy` with the repository's `.clang-tidy` over `Odyssey/src` and fix any naming findings.
4. Push and confirm CI is green on all jobs (MSVC and Apple Clang have not compiled this code yet).

## Phase 1: Platform and RHI

**Deliverables:**

- GLFW `Window` and `Input`.
- `Application` with the main loop and an `EntryPoint`.
- `VulkanDevice` (instance, device, queues, swapchain, resize and minimize handling, validation layers in Debug,
  MoltenVK portability on macOS) wrapped in NVRHI.
- Build-time HLSL to SPIR-V compilation with embedded shader binaries.
- ImGui integration with an NVRHI renderer backend written in this repository.
- Headless device mode with offscreen render targets, GPU readback and PNG writing (`stb_image_write`).
- Golden-image test helper.
- Lavapipe GPU tests in Linux CI.

**Exit criteria:**

- A sample app opens a window and renders a colored triangle plus an ImGui window on Windows, macOS and Linux.
- Resizing, minimizing and restoring work without validation errors. Validation layers are clean in Debug.
- A headless GPU test renders a triangle offscreen and matches a golden image in CI (lavapipe).
- NVRHI's required Vulkan features and extensions are documented in `docs/Architecture.md` and confirmed to work
  on MoltenVK.

## Phase 2: Scene and ECS

**Deliverables:**

- glm, EnTT and nlohmann/json integrated.
- `Scene`, `Entity`, the initial component structs (data only for renderer, physics, script and audio
  components), hierarchy with world-transform computation, and the component registry.
- JSON scene and prefab serialization with format versioning, migrations and precise error reporting.
- `Scene::Copy` for play mode.

**Exit criteria:**

- Round trips are tested for every registered component: save, load and save again gives identical bytes.
- Malformed scene files produce errors with JSON paths and never crash. This is covered by tests for each failure
  class.
- A registry test fails if a component type is missing its descriptor, serializer, or copy support.
- Hierarchy edge cases are tested: reparenting, cycles rejected, destroying subtrees, transform propagation.

## Phase 3: Assets

**Deliverables:**

- Project file (`.odproj`).
- Asset handles and `.meta` files.
- Editor `AssetManager` with polling file watcher and hot reload.
- Texture loading (stb_image, including HDR).
- glTF/GLB import (cgltf, MikkTSpace) producing meshes, materials and a prefab.
- Material assets (`.odmat`).
- Environment assets with GPU precomputation (cubemap, irradiance, prefiltered specular, BRDF LUT) cached in
  `Cache/`.

**Exit criteria:**

- Importing a fixture glTF (several meshes, PBR textures, a node hierarchy, missing tangents) gives the expected
  assets and prefab, verified by tests.
- Re-importing is deterministic: handles are unchanged and files are byte-identical.
- Renaming or moving an asset on disk keeps all references valid.
- A corrupt or missing asset produces a logged error and a visible fallback (pink material, checker texture),
  never a crash.

## Phase 4: Renderer

**Deliverables:** `SceneRenderer` implementing the frame in `docs/Architecture.md` section 5.3:

- Shadows: cascaded shadow maps with PCSS for directional lights, plus spot light shadows.
- Depth and normal prepass.
- XeGTAO SSAO.
- Forward PBR with punctual lights and IBL.
- Skybox, transparent pass, optional bloom.
- Exposure and tonemappers (ACES, AgX, PBR Neutral), then FXAA.
- Screen-space sprites and SDF text.
- Debug views.
- Render settings in `PostProcessComponent`.

**Exit criteria:**

- Golden-image tests cover each feature: material spheres grid (metallic × roughness), IBL-only scene, shadow
  cascades and penumbra, SSAO on and off, each tonemapper, text and sprites, transparency ordering.
- No validation errors in any test scene.
- A documented performance budget is met on a reference GPU: the PBR test scene at 1080p in under 8 ms GPU time.
- Poly Haven HDRIs at 2k and 4k load and prefilter correctly.

## Phase 5: Editor

**Deliverables:**

- `OdysseyEditor` with docking layout and the panels in `docs/Architecture.md` section 12.1.
- The command system with undo/redo, through which all mutations flow.
- ImGuizmo gizmos with snapping, and ID-buffer picking with selection outline.
- Play, pause and step.
- Project create/open, and scene new/open/save with unsaved-changes prompts.
- Headless startup (`--headless`) and `--validate`.

**Exit criteria:**

- Every command has tests for do, undo and redo, and for undo/redo sequences across mixed commands.
- Play mode never mutates the edit scene (tested).
- A user can build a lit scene with imported glTF models, an HDRI environment and shadows entirely from the UI,
  save it, and reopen it identically.

## Phase 6: Physics

**Deliverables:**

- Jolt integration with rigid bodies and colliders created from components.
- Fixed timestep with accumulator.
- Main-thread collision and trigger events.
- Raycast, shape cast and overlap queries.
- Deterministic mode.
- Collider visualization in the editor.

**Exit criteria:**

- Deterministic mode produces bit-identical transforms across runs (tested).
- Stacking, trigger enter/exit and kinematic interaction scenarios are covered by tests.
- Stopping play destroys all physics state without leaks (sanitizers clean).

## Phase 7: Scripting

**Deliverables:**

- Lua 5.4 built from source, and sol2.
- Script assets and the lifecycle callbacks.
- The API namespaces in `docs/Architecture.md` section 8, with deferred structural changes.
- Sandbox, per-callback error isolation and hot reload.
- The API registry with generated `docs/ScriptingAPI.md` and `odyssey.d.lua`.
- `script.check` (compile only).

**Exit criteria:**

- Every API function has a test that calls it from Lua and checks its effect.
- Sandbox escapes (`io`, `os.execute`, `loadfile`, `require` outside the project, `debug`) are tested to fail.
- A runtime error in one script disables only that instance and logs script, line and entity (tested).
- The generated docs are up to date, checked by a test comparing the generated output with the committed files.

## Phase 8: Audio

**Deliverables:** miniaudio engine, audio clip assets (WAV, FLAC, MP3, OGG), audio source and listener components
with spatialization, the Lua `Audio` API, and the null backend for headless runs.

**Exit criteria:** playback, looping, spatial attenuation and stop-on-scene-end are tested on the null backend.

## Phase 9: Agent control

**Deliverables:**

- JSON-RPC 2.0 server over stdio and localhost, plus MCP over stdio, exposing the methods in
  `docs/Architecture.md` section 12.2.
- Input injection.
- Offscreen viewport capture with debug views.
- `docs/AgentProtocol.md`.
- The `build-a-game` skill.

**Exit criteria:**

- An automated test drives the headless editor to create a small game from scratch, without touching files
  directly: a falling block that lands on a floor, a scored collision, and text showing the score. The test then
  steps play mode with injected input, captures a screenshot that matches a golden image, and saves.
- Claude Code can register the editor as an MCP server using a documented one-line command.
- **Acceptance:** an agent following the `build-a-game` skill builds a playable Tetris (7 pieces, rotation, line
  clears, scoring, game over, restart) with no human intervention.

## Phase 10: Export and runtime

**Deliverables:**

- `OdysseyRuntime`.
- Asset dependency collection.
- `.odpak` archives with CRC checks.
- `.odgame` manifests.
- `--export` and `build.export`.
- Platform packaging per `docs/Architecture.md` section 13.

**Exit criteria:**

- On each platform, the exported FeatureTest and Tetris projects run on a clean machine (CI runner) without the
  editor or SDK installed, verified by a headless smoke run that exits cleanly.
- Exports are reproducible: exporting twice gives identical archives.
- No editor symbols are present in the runtime binary (checked).

## Phase 11: FeatureTest and hardening

**Deliverables:**

- `TestProjects/FeatureTest` covering every component and every script API function.
- Enforced coverage checks.
- libFuzzer targets for scene/prefab/material parsing, `.odpak` reading and the agent protocol (Linux, Clang).
- Soak tests (long play sessions with entity churn).
- Documentation pass.

**Exit criteria:**

- The FeatureTest run is part of CI on all platforms and fails on any missing coverage.
- Fuzzers run in CI for a fixed time budget without findings.
- A 30-minute soak test shows no memory growth or leaks.
