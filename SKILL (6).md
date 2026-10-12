---
name: odyssey-review-and-commit
description: Mandatory pre-commit code review and git commit/push procedure for Odyssey. Use before every commit and push.
---

# Review and Commit

Remote: `https://github.com/Essa-Abdulrahman/Game_Engine.git`, default branch `main`.
Use `feature/<short-name>` branches for multi-commit work.

## 1. Self-review (complete every step; fix findings before moving on)
1. Read `git diff --staged` **line by line** as a reviewer, not as the author.
2. Style:
   - Run `clang-format --dry-run --Werror` on changed files.
   - Check the naming rules in `odyssey-code-style` by hand, since clang-format cannot check them.
3. Correctness checklist:
   - **Lifetimes:** ownership is clear, and no `Entity`, Lua userdata, or NVRHI handle dangles across
     a scene reload or play/stop.
   - **GPU:** resources are released, resource states and barriers are correct, and a Debug run
     produces zero Vulkan validation messages.
   - **Performance:** no blocking I/O on the main/render thread, and no per-frame heap allocation in
     hot loops without justification.
   - **Errors:** every `Result` is checked and Lua errors are caught. There are no silent failures.
   - **Threads:** for every new function, it is known which thread calls it.
   - **Leftovers:** no commented-out code, stray logs, hardcoded absolute paths, or TODO without an
     issue.
4. Tests:
   - Coverage required by `odyssey-testing` exists.
   - `ctest --preset debug` is fully green.
   - Also run `ctest --preset release` when touching the renderer or physics.
5. Docs: regenerate `docs/ScriptingAPI.md` if the API changed. Update `docs/Architecture.md` and
   `docs/Decisions.md` when the design changed.

## 2. Commit
The subject is imperative, at most 72 characters, and prefixed with the subsystem: `Core`,
`Renderer`, `Scene`, `Asset`, `Physics`, `Audio`, `Script`, `Editor`, `Runtime`, `Automation`,
`Build`, `Docs`, or `Tests`. The body covers what changed, the risks, and how it was tested.

```
Renderer: Add PCSS soft shadows for directional light

- Blocker search (16 taps) + Poisson PCF (32 taps), light size in world units
- Cascade blend band hides seams between cascades

Risk: cost on MoltenVK; measured 0.4 ms @1080p on M2.
Tests: ShadowPCSS golden, CascadeStability; ctest debug+release green.
```
One logical change per commit. Never commit build output, golden `.actual`/`.diff` images, automation
tokens, or any other secret.

## 3. Push
`git push origin <branch>`. If the push is rejected, run `git pull --rebase`, re-run the tests, and
push again. Never force-push `main`.
