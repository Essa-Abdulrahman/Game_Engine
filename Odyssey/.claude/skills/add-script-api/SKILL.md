---
name: add-script-api
description: Expose engine functionality to Lua scripts in Odyssey through the script API registry (bindings, documentation, tests, generated references and FeatureTest coverage). Use whenever adding or changing any Lua-visible function, property, type or namespace. Applies from Phase 7 onward.
---

# Adding or changing script API

## Design rules

- The API is small, consistent and hard to misuse. Lua names follow engine naming: namespaces and functions in
  PascalCase (`Scene.Instantiate`, `Input.IsKeyDown`), fields in PascalCase (`Transform.Translation`), and method
  calls with `:` on instances (`entity:GetComponent("RigidBody")`).
- Functions validate their arguments. Invalid calls (destroyed entity, wrong type, missing asset) raise a Lua
  error with a clear message. They never crash and never silently do nothing.
- No raw pointers escape to Lua. Entities are referenced by handle and checked for validity on every access.
- Structural changes (create, destroy, reparent) during iteration are deferred to the end of the step; document
  this on the function.
- Nothing in the API may break the sandbox: no file system, process, OS or debug access.

## Steps

1. **Binding** in `Odyssey/src/Odyssey/Scripting/` in the file for its namespace (eg. `ScriptSceneApi.cpp`),
   registered through the API registry with:
   - its Lua name;
   - its signature, with parameter names and types;
   - a one-sentence description, plus notes on errors and deferral.
2. **Tests** (`OdysseyTests/src/Scripting/`):
   - run a Lua snippet in a headless scene that calls the function and checks its effect on engine state;
   - cover each error path, asserting that the error message is raised and the engine stays consistent.
3. **Generated references.** Regenerate `docs/ScriptingAPI.md` and `odyssey.d.lua`, and commit them. A test fails
   when they are stale.
4. **FeatureTest coverage.** Call the function from a FeatureTest script with `Test.Expect(...)` on its result
   or effect. The coverage check fails for any registered function with zero calls.
5. **Examples.** If the function is part of a common workflow, add it to the relevant example in
   `docs/Architecture.md` section 8 or the `build-a-game` skill.

Run the `code-review` skill afterwards.
