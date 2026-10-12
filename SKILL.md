---
name: odyssey-editor-automation
description: How an AI agent builds a complete game (e.g. "make me Tetris") in the Odyssey editor without human intervention, using odyssey-cli and the editor automation API. Use for any request to create, modify, play-test, or export a game or scene.
---

# Building Games Through Editor Automation

Every editor capability is reachable through `odyssey-cli`, a JSON-RPC client for
`Odyssey-Editor --automation-port`. Never try to drive the ImGui UI with clicks.

## 0. Start
```bash
odyssey-cli project create --name Tetris --path Projects/Tetris --template Empty3D
odyssey-cli editor start --project Projects/Tetris/Tetris.oproj --headless   # or omit --headless to watch
odyssey-cli api describe > /tmp/api.json      # full list of methods, components, fields, Lua API
```
Read `api.describe` before anything else. It is the authoritative schema for the build you are
running.

## 1. Plan before building
Write `Projects/<Game>/DESIGN.md` covering rules, controls, entities and prefabs, scripts, scenes, and
acceptance checks. Keep it short, and keep it updated as the game changes.

## 2. Build loop
```bash
# entities & components
odyssey-cli entity create --name Board --parent none
odyssey-cli component add Board MeshRenderer --set Mesh=builtin:Cube --set Material=Assets/Materials/Board.omat
odyssey-cli component set Board Transform.Scale=[10,20,0.2]

# prefabs
odyssey-cli prefab create-from-entity Block --out Assets/Prefabs/Block.oprefab

# scripts: write Lua files directly into Assets/Scripts/, then attach
odyssey-cli component add GameManager Script --set Script=Assets/Scripts/GameManager.lua \
    --set Fields.BlockPrefab=Assets/Prefabs/Block.oprefab

# assets
odyssey-cli asset import path/inside/project/model.glb
odyssey-cli scene save
```
`odyssey-cli batch commands.json` runs many commands as one undoable transaction. Prefer it for
large edits.

## 3. Verify, then iterate (mandatory)
```bash
odyssey-cli play
odyssey-cli input simulate --key Left --frames 5   # scripted input for play-testing
odyssey-cli step --frames 120
odyssey-cli console read --level warn              # must be empty of errors
odyssey-cli capture --out /tmp/frame.png           # look at it
odyssey-cli stop
odyssey-cli validate                               # schema + missing-asset check
```
Write gameplay tests as a project test scene using the `Test` Lua module. For example, a test can spawn
a piece, simulate drops, and assert that a cleared line increments the score. Run it with
`odyssey-cli test run`. The game is only done when its tests pass, the console shows no errors, and
screenshots match the design.

## 4. Export
```bash
odyssey-cli export --out Exports/Tetris            # host platform, Dist runtime
Exports/Tetris/Tetris --headless --test            # exported build passes its own tests
```

## Rules
- All mutations go through the CLI or API, or through direct YAML and Lua edits followed by
  `odyssey-cli validate` and `scene reload`.
- If a capability you need is missing from `api.describe`, do not hack around it. Add it to the engine
  following the `odyssey-add-component` and `odyssey-scripting-api` skills, with tests, and then
  continue. Missing automation is an engine bug.
- Assets: use what is in the project or the built-in primitives. To download an HDRI from Poly Haven,
  ask the user first.
