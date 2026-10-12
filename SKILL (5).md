---
name: odyssey-scripting-api
description: Rules for writing Odyssey Lua game scripts and for adding or changing C++ Lua bindings (ScriptGlue, sol2). Use when writing gameplay Lua or extending the scripting API.
---

# Odyssey Scripting API (Lua 5.4)

## Writing a game script
```lua
-- Assets/Scripts/Player.lua
local Player = {
	Fields = {
		MoveSpeed = { Type = "Number", Default = 6.0 },
		BulletPrefab = { Type = "Prefab" },
	},
}

function Player:OnCreate()
	self.Body = self.Entity:GetComponent("RigidBody")
end

function Player:OnUpdate(dt)
	local move = Math.Vec3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"))
	self.Body:SetLinearVelocity(move * self.MoveSpeed)

	if Input.IsKeyPressed(Key.Space) then
		local bullet = Scene.Instantiate(self.BulletPrefab, self.Entity.Transform.Translation)
		bullet:GetComponent("RigidBody"):AddImpulse(self.Entity.Transform:Forward() * 20)
	end
end

function Player:OnCollisionEnter(other)
	if other.Tag == "Enemy" then
		Scene.Destroy(self.Entity)   -- deferred to end of frame
	end
end

return Player
```
API modules: `Scene`, `Entity` methods, component proxies, `Input`, `Key`/`MouseButton`, `Physics`,
`Audio`, `Time`, `Math`, `Log`, and `Application`. The generated reference is `docs/ScriptingAPI.md`;
check it, and do not guess function names. In a running editor, `odyssey-cli api describe --lua`
prints the live list.

## Adding a binding (C++)
1. Implement it in the matching `Odyssey/src/Odyssey/Script/Glue/<Module>Glue.cpp`, and register it
   only through:
   ```cpp
   ScriptGlue::Register(lua, "Physics", "Raycast", &PhysicsGlue::Raycast,
   	"Raycast(origin: Vec3, direction: Vec3, maxDistance: number, layerMask?: integer) -> RaycastHit|nil");
   ```
   The signature string feeds the docs and `api.describe`. Bindings registered any other way fail the
   coverage check.
2. Validate every argument: type, range, normalized vectors, and entity validity. On bad input, raise
   a Lua error with a clear message via `ScriptGlue::RaiseError`, and never assert or crash.
3. Entity arguments arrive as UUID-backed handles. Call `ScriptGlue::ResolveEntity`, which raises
   "entity has been destroyed" when appropriate.
4. Do not keep raw component pointers across frames. Proxies re-resolve on access.
5. Allocate nothing per call where it can be avoided. Bindings are hot paths.
6. Tests:
   - A unit test in `Odyssey-Tests/Script/<Module>GlueTests.cpp` covering success and every error
     branch.
   - A call in `Projects/FeatureTest/Scripts/Tests/<Module>Test.lua`, which the coverage gate requires.
7. Regenerate the docs with `cmake --build --preset debug --target GenerateScriptDocs`.

## Sandbox (do not weaken without a Decisions.md entry)
- Allowed: `base` (minus `dofile`, `loadfile`, `load` with binary chunks), `string`, `table`,
  `math`, `utf8`, and `coroutine`.
- Removed: `io`, `os` (except `os.clock` and `os.time`), `debug`, `package.loadlib`, and C modules.
- `require` resolves only inside the project's `Assets/Scripts/`.
