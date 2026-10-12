# Roadmap

A milestone is done only when its exit criteria pass in CI on Windows, macOS, and Ubuntu 24.04, and
it has been code-reviewed and pushed.

| # | Milestone | Exit criteria |
|---|---|---|
| M0 | Repo, CMake presets, submodules, CI matrix, doctest, clang-format check | Empty test suite green on all 3 OSes |
| M1 | Core: Log, Assert, UUID, FileSystem, Application loop, GLFW window, Input | Unit tests for UUID/FileSystem/Input state; window opens |
| M2 | NVRHI Vulkan device, swapchain, shader build pipeline, offscreen golden-image harness | Clear-color + triangle golden tests on lavapipe |
| M3 | Scene/ECS, Transform hierarchy, YAML serializer with versioning | Round-trip tests for every component |
| M4 | Asset registry, glTF importer, textures, PBR forward pass, camera | Khronos sample models (DamagedHelmet, MetalRoughSpheres) golden tests |
| M5 | IBL from HDRI, HDR target, ACES/AgX tonemapping, exposure, bloom | Prefilter/irradiance numeric tests, golden images |
| M6 | Cascaded shadow maps + PCSS, spot/point shadows | Golden tests, cascade stability test (camera jitter) |
| M7 | GTAO | Golden test; perf budget ≤ 1.5 ms at 1080p on reference GPU |
| M8 | Jolt physics + components + contact events | Deterministic stepping tests, event tests |
| M9 | miniaudio integration (null backend in tests) | Source/listener tests via null device |
| M10 | Lua scripting, full API, sandbox, error isolation, API registry + docs generation | Binding unit tests; FeatureTest scene with 100% API coverage |
| M11 | Prefabs, runtime spawn/destroy | Override + spawn tests |
| M12 | Editor: panels, gizmo, play mode, undo/redo command layer | Command round-trip tests (execute/undo/redo == identity) |
| M13 | Automation server, headless mode, odyssey-cli, `odyssey-editor-automation` skill | Integration test that builds a small game purely via CLI |
| M14 | Export pipeline (asset cooking, Dist runtime, .app bundle) | Exported FeatureTest runs headless and passes on each OS |
| M15 | Acceptance: an agent builds Tetris through automation only | Game exported and playable; recorded as a sample project |

# Decisions log (`docs/Decisions.md` format)

```
## D-001 Lua 5.4 + sol2 for scripting (2026-10-12)
Context: Brief requested Lua, open to discussion.
Decision: Lua 5.4 (not LuaJIT: 5.4 is maintained, has integers, runs on all targets incl. Apple Silicon without JIT restrictions).
Consequences: sol2 compile times — mitigate by keeping bindings in few TUs.
```
