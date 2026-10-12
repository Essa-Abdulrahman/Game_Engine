---
name: add-component
description: Add a new ECS component to the Odyssey engine end to end (struct, registry descriptor, serialization, inspector, Lua binding, agent protocol, tests and FeatureTest coverage). Use whenever a new component type is introduced or a component's fields change. Applies from Phase 2 onward.
---

# Adding or changing a component

A component is never half-supported. All of these steps are required in the same change. File locations follow
`docs/Architecture.md` section 6; if a step's system does not exist yet (eg. Lua before Phase 7), skip that step.
The system's own phase then covers every existing component.

## Steps

1. **Struct** in `Odyssey/src/Odyssey/Scene/Components.h`. Plain data, PascalCase public fields, sensible
   defaults on every field, and no pointers to other entities (store `UUID`s; store `AssetHandle`s for assets).
   Rotations are `glm::quat`. Units are SI (meters, kilograms, seconds, radians internally). Document the units
   in a comment when they are not obvious.
2. **Registry descriptor** in `Odyssey/src/Odyssey/Scene/ComponentRegistry.cpp`. Give the serialized name
   (PascalCase, without the `Component` suffix, eg. `"RigidBody"`), and every field with name, type, default,
   valid range and a tooltip. Also give add, remove and copy functions. Field order here is the serialized and
   inspector order.
3. **Serialization.** This comes from the descriptor. Add a format migration if an existing field changed
   meaning, name or type: bump `FormatVersion` and write a migration function with a test.
4. **Runtime behaviour** in the owning system (renderer, physics, audio, scripting), including how the component
   reacts to being added, removed or edited during play.
5. **Inspector.** This is generated from the descriptor. Add a custom drawer only when generic widgets cannot
   express it (eg. a color picker with intensity), in `OdysseyEditor`.
6. **Lua binding.** Expose the component and its fields through the registry-driven binding, and add any
   component-specific functions with the `add-script-api` skill.
7. **Agent protocol.** `component.add/get/set/remove` work through the registry. Confirm that the field paths
   appear in `component.listTypes`.
8. **Tests** (`OdysseyTests/src/Scene/`):
   - defaults;
   - serialization round trip with non-default values, then byte-identical re-save;
   - malformed values rejected with a JSON-path error;
   - copy via `Scene::Copy`;
   - runtime behaviour in the owning system.
9. **FeatureTest coverage.** Add the component, with non-default values, to
   `TestProjects/FeatureTest/Assets/Scenes/FeatureTest.odscene`, and add script expectations exercising it.
   The coverage test fails if this is missing.
10. **Docs.** Update the component table in `docs/Architecture.md` section 6 and the generated API references.

Run the `code-review` skill afterwards. Its "Tests" section checks this list.
