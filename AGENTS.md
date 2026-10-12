# AGENTS.md — Odyssey Game Engine

This file is the contract for every agent (and human) working in this repository.
Read it fully before making changes. Task-specific procedures live in `.claude/skills/`.

## 1. Ground rules

1. **Stay inside the repository.** Never read, copy, or reference code from outside this working
   directory. Third-party code enters only as pinned submodules under `Odyssey/vendor/`.
2. **Work autonomously.** Ask the user only when a decision is irreversible, costs money, or contradicts
   this document. Otherwise make the call, record it in `docs/Decisions.md`, and continue.
3. **Production grade, always.** No TODO-driven stubs merged to `main`, no commented-out code, no
   "temporary" hacks. If something cannot be finished, it is not merged.
4. **Nothing lands untested.** Every behavioral change ships with unit tests and, where applicable, an
   addition to the FeatureTest scene (§6). All tests must pass locally before commit.
5. **Review before every commit.** Follow `.claude/skills/odyssey-review-and-commit/SKILL.md` exactly.

## 2. Repository layout

```
Odyssey/                 Engine static library (namespace Odyssey, macro prefix OD_)
  src/Odyssey/
    Core/                Application, Log, Assert, Base.h (Ref/Scope), Timestep, UUID, FileSystem
    Platform/            Window (GLFW), Input, platform utilities
    Renderer/            NVRHI device, render graph-lite, PBR, IBL, shadows, GTAO, post
    Scene/               Scene, Entity, Components, SceneSerializer, Prefab
    Asset/               AssetManager, AssetRegistry, importers (glTF, HDR, textures)
    Physics/             Jolt integration (PhysicsWorld, bodies, queries)
    Audio/               miniaudio integration (AudioEngine, sources, listener)
    Script/              Lua VM, ScriptEngine, bindings (ScriptGlue)
    Project/             Project, ProjectSerializer, Exporter
  shaders/               GLSL sources, compiled to SPIR-V at build time
  vendor/                Pinned git submodules only
Odyssey-Editor/          Editor executable (ImGui + ImGuizmo), headless mode, automation server
Odyssey-Runtime/         Shipping player executable (no editor code linked)
Odyssey-Tests/           doctest unit + integration tests (engine library only)
Tools/odyssey-cli/       Command-line client for the editor automation API
Projects/FeatureTest/    The engine-wide feature/regression test project
docs/                    Architecture.md, Roadmap.md, Decisions.md, ScriptingAPI.md
```

## 3. Technology decisions (locked unless `docs/Decisions.md` is amended)

| Concern | Choice | Why |
|---|---|---|
| Language / build | C++20, CMake ≥ 3.25, Ninja | Cross-platform, presets for every config |
| Window / input | GLFW 3.4 | Requested; mature on all three platforms |
| GPU abstraction | NVRHI, Vulkan backend everywhere (MoltenVK on macOS) | Requested; one backend to test |
| Shaders | GLSL → SPIR-V via `glslc` (Vulkan SDK) at build time | No runtime compiler dependency |
| Math | glm | Requested |
| ECS | EnTT | Proven, header-only, same model Hazel uses |
| Physics | Jolt Physics | Deterministic, fast, permissive license, simple API |
| Audio | miniaudio | Requested |
| Scripting | Lua 5.4 + sol2 | Small, embeddable, safe sandboxing; sol2 gives type-safe bindings |
| Model import | cgltf | Single-file glTF 2.0 / GLB parser |
| Images | stb_image (LDR + Radiance/`.hdr`), tinyexr for `.exr` HDRIs | Poly Haven ships both |
| Editor UI | Dear ImGui (docking branch) + ImGuizmo | Industry standard for tools |
| Serialization | yaml-cpp for scenes/prefabs/projects; nlohmann/json for automation RPC | Text formats agents can read and diff |
| Logging | spdlog | |
| Tests | doctest | Single header, fast, simple |

## 4. Build and run

```bash
cmake --preset debug            # also: release, dist; add -linux/-macos/-windows if presets demand
cmake --build --preset debug
ctest --preset debug --output-on-failure
./build/debug/bin/Odyssey-Editor --project Projects/FeatureTest/FeatureTest.oproj
```

Configurations:
- **Debug**: asserts, validation layers, no optimization.
- **Release**: optimized, asserts and validation layers on, editor available.
- **Dist**: optimized, asserts off, editor code compiled out (`OD_DIST`), used by the exporter.

Prerequisite: the Vulkan SDK (1.3+), installed and on `PATH` (provides `glslc` and validation layers).
Submodules: `git submodule update --init --recursive`.

## 5. Code style (summary — full rules in `odyssey-code-style` skill)

Hazel conventions, enforced by `.clang-format` and review:
- PascalCase: classes, structs, functions, member functions, enums and enum values, namespaces, file names.
- camelCase: local variables and parameters.
- `m_` + PascalCase for members, `s_` + PascalCase for statics, `g_` + PascalCase for (rare) globals.
- Macros `OD_UPPER_SNAKE`. Tabs for indentation. Braces on their own line for types and functions.
- `#pragma once`; `Ref<T>`/`Scope<T>` from `Core/Base.h`; no raw `new`/`delete` outside allocators.
- Self-documenting code: descriptive names over clever templates; comments explain *why*.

## 6. Testing policy

Three layers, all run by `ctest`:
1. **Unit tests** (`Odyssey-Tests/`, label `unit`): pure logic, no GPU/window. Math helpers, ECS,
   serialization round-trips, asset parsing, physics stepping, script bindings against a headless Scene.
2. **GPU tests** (label `gpu`): offscreen rendering through NVRHI with golden-image comparison
   (per-pixel tolerance). On CI they run on lavapipe (Mesa software Vulkan).
3. **FeatureTest scene** (label `feature`): `Odyssey-Runtime --headless --test Projects/FeatureTest`
   loads a scene containing **every component type** and Lua scripts that exercise **every scripting
   API function**, runs N fixed frames, and exits non-zero on any failed `Test.Assert`. A coverage check
   fails the run if any registered component or Lua binding was not exercised.

Rules: new component → add to FeatureTest. New Lua binding → call it in a FeatureTest script.
Bug fix → regression test that fails before the fix.

## 7. Engine invariants (never violate)

- Entity handles exposed to Lua are `(UUID)`-based and validated on every access; touching a destroyed
  entity raises a Lua error, never UB.
- Entity creation/destruction requested during iteration is deferred to the end of the frame.
- Physics runs on a fixed timestep (default 60 Hz) with interpolation for rendering.
- The renderer never blocks on asset loading; missing assets render with a visible fallback.
- All editor mutations go through the command system (`EditorCommand`), so every change is undoable and
  equally available to the UI, headless mode, and the automation API.
- Runtime/Dist builds contain zero editor code (`OD_DIST` and target separation, verified by a test that
  scans the runtime binary's link map for `Editor` symbols).

## 8. AI control of the editor

Agents build games through the automation API, never by guessing at UI clicks. See the
`odyssey-editor-automation` skill. Scene, prefab, and project files are also plain YAML, so direct edits
are allowed, but they must be validated with `odyssey-cli validate` before commit.

## 9. Skills index

| Skill | Use when |
|---|---|
| `odyssey-code-style` | Writing or reviewing any C++ |
| `odyssey-testing` | Adding tests, running the suite, updating golden images |
| `odyssey-review-and-commit` | Before every commit and push |
| `odyssey-add-component` | Adding or changing an ECS component |
| `odyssey-scripting-api` | Adding or changing Lua bindings |
| `odyssey-editor-automation` | Building a game or scene through the editor (e.g. "make Tetris") |
