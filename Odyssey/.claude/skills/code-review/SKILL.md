---
name: code-review
description: Review a change in the Odyssey repository against its production-quality bar, Hazel code style and testing rules. Use before declaring any task done, before committing, or when asked to review code or a diff.
---

# Code review

Review the **entire diff** (`git diff` against the base, including new untracked files), not just the parts you
remember writing. Read each changed file in full where the change touches control flow or ownership.

## Procedure

1. Run the `build-and-test` full verification. Review does not start on a red build.
2. Walk the checklist below for every changed file and write down each finding as
   `file:line — problem — fix`.
3. Fix every finding, or justify it explicitly in the summary if it is a deliberate, documented exception.
4. Re-run the full verification after fixes.
5. Report: what changed, how it was verified (which configurations ran locally versus only in CI), findings and
   their resolutions, and any follow-ups added to `docs/Roadmap.md`.

## Checklist

**Correctness**
- Edge cases: empty, zero, maximum, overflow (eg. `offset + size`), negative, null, missing file, repeated calls.
- Ownership and lifetimes: who owns each object, no dangling references or iterators, no use after move,
  correct destruction order, no container modification during iteration.
- Error handling: recoverable failures return a result and log with context. Invariants use verify. No exceptions
  escape engine code. Third-party exceptions are caught at the boundary.
- Thread safety matches the documented contract. Locks have a consistent order. No callbacks or logging while
  holding a lock that the callee could take.
- No undefined behaviour: alignment, aliasing, signed overflow, uninitialized reads, out-of-range shifts.
- Asserts have no side effects. Dist builds behave correctly with asserts compiled out.

**Style (AGENTS.md)**
- Naming table: PascalCase types and functions, camelCase locals and parameters, `m_` members, `s_` statics, `OD_`
  macros, PascalCase file names.
- Formatting is clang-format clean; tabs; namespace indentation and blank lines; Allman braces.
- Includes: `odpch.h` first in engine `.cpp` files, own header next, rooted include paths, headers include what they
  use, no platform headers in headers, no `using namespace` in headers.
- No Win32 macro-colliding names. No direct console output in engine code. Paths logged via `ToUTF8String()`.
- Code reads plainly. Comments explain why. No dead code, commented-out code, or `TODO` without a roadmap entry.

**Design**
- Module dependency direction respected (see `docs/Architecture.md`). No editor code in the engine.
- Public API is minimal, with ownership, units, thread safety and failure behaviour clear from the signature or a
  short comment.
- No per-frame or per-entity heap allocations or `std::function` in hot loops without a measured reason.
- New dependencies follow the `add-dependency` skill.

**Tests**
- Every new behaviour is tested, including failure paths and boundaries (see the `write-tests` skill).
- Bug fixes include a regression test that failed before the fix.
- Tests are deterministic, isolated, and use `TemporaryDirectory` for files.
- Components and script APIs: the `add-component` / `add-script-api` checklists are complete.

**Docs**
- `docs/Roadmap.md` status updated. `docs/Architecture.md` updated for design changes. API references regenerated.

Be strict. The goal is code that is still obviously correct to a new engineer a year from now.
