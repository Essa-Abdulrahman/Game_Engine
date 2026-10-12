---
name: odyssey-add-component
description: Step-by-step checklist for adding or changing an ECS component in Odyssey so it works in serialization, the editor Inspector, Lua, automation, and tests. Use whenever a component struct is created or its fields change.
---

# Adding a Component

A component is only "done" when every one of the 9 touchpoints below is complete. Missing any of
them breaks the FeatureTest coverage check.

1. **Struct.** Declare it in `Odyssey/src/Odyssey/Scene/Components.h`. It is a POD-like struct with
   PascalCase fields and sensible defaults, and it holds no pointers to other entities; reference
   them by `UUID`.
   ```cpp
   struct SpinComponent
   {
   	glm::vec3 Axis = { 0.0f, 1.0f, 0.0f };
   	float DegreesPerSecond = 90.0f;
   };
   ```
2. **Reflection.** Register it in `ComponentRegistry.cpp` with `OD_REGISTER_COMPONENT(Spin, ...)`. The
   field list carries name, type, range, and tooltip. This single description drives steps 3–6, so do
   not hand-write per-system code when reflection can do it.
3. **Serialization.** `SceneSerializer` uses the reflection data. Add migration code only when
   renaming or removing a field, and bump `SceneSerializer::CurrentVersion` when you do.
4. **Copying.** Make sure `Scene::Copy` (play-mode snapshot) and prefab instantiation copy it. This is
   automatic via the registry; verify it with a test.
5. **Inspector.** The editor draws fields from reflection. Add a custom drawer in
   `Odyssey-Editor/src/Panels/ComponentDrawers/` only when a field needs one, such as an asset picker
   or curve.
6. **Lua.** The reflection-driven proxy exposes fields automatically
   (`entity:GetComponent("Spin").DegreesPerSecond = 45`). Add methods through `ScriptGlue`, following
   `odyssey-scripting-api`.
7. **Automation.** `component.add`, `component.set`, and `component.get` work via reflection. Confirm
   the type appears in `api.describe`.
8. **Runtime system.** If the component has behavior, add a system function in `Scene::OnUpdate*` or in
   the owning subsystem (Physics, Audio, Renderer), and keep the call order documented in
   Architecture §2.
9. **Tests.** Add all of the following:
   - A serialization round-trip with non-default values.
   - Copy and prefab tests.
   - A Lua get/set test.
   - A behavior test, if the component has behavior.
   - An entity with this component and non-default values in `Projects/FeatureTest/Scenes/FeatureTest.oscene`.
   - A FeatureTest script assertion that reads it back.

For a field change on an existing component, still walk steps 3, 6, 7, and 9.
