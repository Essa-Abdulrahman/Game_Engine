---
name: implement-roadmap-phase
description: Plan and execute a phase (or the next slice of a phase) from docs/Roadmap.md for the Odyssey engine. Use when asked to continue development, start the next phase, or implement a roadmap item such as the renderer, editor, physics or scripting.
---

# Implementing a roadmap phase

## 1. Orient

- Read `docs/Roadmap.md`: find the first phase that is not done, and its open items. Previous-phase open items
  come first (for example, Phase 0's first CMake run).
- Read the matching sections of `docs/Architecture.md`. The design there is agreed. Implement it; do not redesign
  it. If you find a real problem with the design, fix the design in the document first, in its own commit, with
  the reason, then implement.
- Read `AGENTS.md` again if you have not done so in this session.

## 2. Plan slices

Break the phase into slices that each build, pass all tests, and add a coherent piece. For example, Phase 1 slices
are:

1. GLFW window plus events (no rendering).
2. Vulkan device and swapchain clear color.
3. Shader build step plus triangle.
4. Headless device plus readback plus golden test.
5. ImGui backend.
6. Lavapipe CI job.

Write the slice list into the phase's section of `docs/Roadmap.md` as a checklist, so progress survives across
sessions.

## 3. For each slice

1. Add any dependency first, following the `add-dependency` skill.
2. Write the code following `AGENTS.md`, with tests alongside, following the `write-tests` skill. Platform code
   gets the same tests on all three platforms, or a documented reason why a test is platform-specific.
3. Run the full verification from the `build-and-test` skill.
4. Run the `code-review` skill over the slice's diff and fix the findings.
5. Tick the slice in `docs/Roadmap.md`, then commit (`Area: Imperative summary`).

## 4. Finish the phase

- Check every exit criterion literally, one by one, and record the evidence (test names, CI run, screenshots) in
  the phase notes. If one cannot be met, the phase is not done; record what remains.
- Update the status table, `docs/Architecture.md` (for example the dependency table and anything that changed),
  and `AGENTS.md` if conventions evolved.

## Working rules

- Prefer finishing a slice completely over starting several.
- Never leave code exposed that is half-implemented. Either finish it, or keep it out of the build and roadmap it.
- GPU features need golden-image tests (Phase 1 provides the helper). Visual inspection alone is not verification.
- When something is genuinely blocked (missing SDK, hardware-only behaviour), say exactly what is blocked and why,
  finish everything else, and record the blocker in the roadmap.
