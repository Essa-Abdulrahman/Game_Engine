---
name: odyssey-testing
description: How to write, run, and maintain Odyssey tests (doctest unit tests, GPU golden-image tests, and the FeatureTest scene). Use when adding any feature, fixing a bug, or before committing.
---

# Odyssey Testing

## Run
```bash
cmake --build --preset debug
ctest --preset debug --output-on-failure              # everything
ctest --preset debug -L unit                          # fast, no GPU
ctest --preset debug -L gpu                           # needs Vulkan (lavapipe ok)
ctest --preset debug -L feature                       # FeatureTest scene
./build/debug/bin/Odyssey-Tests -tc="Prefab*"         # doctest filter
```
**Never commit with a red suite.** A flaky test counts as a failing test. Fix it, or quarantine it
with an issue link, and only with the user's approval.

## Unit tests (doctest)
File mirrors the source: `Odyssey/src/Odyssey/Scene/Prefab.cpp` → `Odyssey-Tests/Scene/PrefabTests.cpp`.
```cpp
#include "OdysseyTest.h"   // doctest + headless engine fixtures

TEST_SUITE("Prefab")
{
	TEST_CASE("Instantiate copies components and remaps UUIDs")
	{
		Odyssey::Test::HeadlessScene fixture;
		Odyssey::Ref<Odyssey::Prefab> prefab = fixture.MakePrefab({ "Transform", "MeshRenderer" });

		Odyssey::Entity instance = fixture.Scene->Instantiate(prefab);

		CHECK(instance.HasComponent<Odyssey::MeshRendererComponent>());
		CHECK(instance.GetUUID() != prefab->GetRootUUID());
	}
}
```
Guidelines:
- One behavior per `TEST_CASE`, and test names are sentences.
- No sleeps or wall-clock timing; use the injected `Timestep`.
- No filesystem access outside the `Test::TempDirectory` fixture.
- Cover the happy path, edge cases, and failure paths, including `Result` errors and Lua errors.

## Required coverage per change
| Change | Must add |
|---|---|
| New component | Serialization round-trip, script binding test, Inspector field-mapping test, FeatureTest entity |
| New Lua function | Binding unit test (success + error), FeatureTest script call |
| Renderer feature | Golden-image test, plus numeric test for any precomputation (BRDF LUT, SH coefficients) |
| Physics | Deterministic step test (same inputs give bit-identical transforms after N steps) |
| Audio | Test against miniaudio's null backend, with no audio device required |
| Editor command | Execute → Undo → Redo test, and JSON round-trip of the command |
| Automation RPC method | Integration test through `odyssey-cli` against a headless editor |
| Bug fix | Regression test that fails without the fix |

## GPU golden images
- Harness: `Test::OffscreenRenderer` renders a 256×256 frame with a fixed camera and fixed seed, with
  auto-exposure off.
- Compare against `Odyssey-Tests/Golden/<Name>.png`. Tolerance: mean absolute error ≤ 0.5/255 and no
  pixel more than 8/255 off.
- On failure the harness writes `<Name>.actual.png` and `<Name>.diff.png`. View both before deciding.
- Regenerate with `ODYSSEY_UPDATE_GOLDEN=1 ctest -L gpu -R <Name>`. The commit message must justify
  the visual change.

## FeatureTest scene
`Projects/FeatureTest/` holds one entity per component type with non-default values. It also holds
`Scripts/Tests/*.lua`, which use `Test.Assert`, `Test.AssertNear`, `Test.ExpectError(fn)`, and
`Test.Done()`.

Run it headless with `Odyssey-Runtime --headless --test Projects/FeatureTest --max-frames 600`. The
run fails if any of the following happens:
- An assertion fails, or a script errors unexpectedly.
- `Test.Done()` is not reached within the frame budget.
- Coverage is incomplete: every binding in the `ScriptGlue` registry must be called at least once, and
  every type in the `ComponentRegistry` must be present and serialized.
