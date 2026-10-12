# AGENTS.md — Odyssey Engine development guide

Odyssey is a production-grade, deliberately simple 3D game engine for Windows, macOS and Linux (Ubuntu 24+).
This file is the contract for everyone working in this repository, human or AI. Read it fully before changing
anything. Design rationale lives in `docs/Architecture.md`; the plan and current status live in `docs/Roadmap.md`.

## Non-negotiables

1. **Production quality.** No hacks, placeholders, or "temporary" shortcuts. If something cannot be done properly
   yet, leave it out and record it in `docs/Roadmap.md`.
2. **Everything is tested.** New logic ships with unit tests. Bug fixes ship with a regression test that failed
   before the fix. Never weaken, skip, or delete a test to make a build pass.
3. **Warnings are errors** on every platform and configuration. Never disable a warning to silence it; fix the code.
4. **Hazel code style** (summarized below) for all first-party code.
5. **Self-contained.** Use only code in this repository and the pinned dependencies fetched by CMake. Never read,
   copy, or reference code from elsewhere on the machine.
6. **Review before done.** Every change passes the Definition of Done, including the `code-review` skill.

## Working autonomously

- Work without asking questions. Ask only when blocked by something you cannot decide or obtain yourself
  (credentials, a product decision with no reasonable default, an irreversible action). When you choose a
  default, write it down in the relevant doc so the decision is visible.
- Work in small, coherent steps that each leave the tree building and green.
- Commit messages: `Area: Imperative summary` (eg. `Core: Add atomic file writes`, `Renderer: Fix cascade seams`).

## Repository layout

```
Odyssey/              Engine static library (no editor code, ever)
  src/odpch.h         Precompiled header, first include of every engine .cpp
  src/Odyssey.h       Umbrella header for applications (not for engine code)
  src/Odyssey/<Module>/   Core, Events, Platform, Renderer, Scene, Asset, Physics, Audio, Scripting, ...
OdysseyTests/         doctest unit and integration tests (mirrors the engine layout)
OdysseyEditor/        Editor executable, including headless/agent mode (Phase 5+)
OdysseyRuntime/       Game player executable used by exported games (Phase 10)
TestProjects/         Projects used by automated tests, including FeatureTest (Phase 11)
cmake/                Build configuration modules
tools/                Developer tooling (check_style.py)
docs/                 Architecture.md, Roadmap.md, formats and API references
.claude/skills/       Task playbooks for agents (see "Skills" below)
```

Module dependency direction is strict: `Core <- Platform/Renderer/Asset/Physics/Audio <- Scene <- Scripting`.
`OdysseyEditor` and `OdysseyRuntime` depend on `Odyssey`; the engine never depends on either.
Editor-only code lives in `OdysseyEditor`, never behind `#ifdef` in the engine.

## Build and test

Requirements: CMake 3.25+, Ninja, a C++20 compiler (MSVC 2022 17.8+, GCC 13+, Clang 17+, Xcode 16+), Python 3.
From Phase 1 the Vulkan SDK is also required (headers, loader, DXC, validation layers, MoltenVK on macOS).

```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
cmake --preset release && cmake --build --preset release && ctest --preset release
cmake --preset dist && cmake --build --preset dist && ctest --preset dist
cmake --preset debug-sanitizers && cmake --build --preset debug-sanitizers && ctest --preset debug-sanitizers  # Linux/macOS
python3 tools/check_style.py          # add --fix to apply clang-format
build/debug/bin/OdysseyTests --test-case="UUID*"   # run a subset
```

On Windows use a "x64 Native Tools" prompt for the Ninja presets, or the `vs2022` preset for Visual Studio.
Configurations: **Debug** (asserts, no optimization), **Release** (optimized + debug info + asserts),
**Dist** (optimized, asserts compiled out; what ships). Set `ODYSSEY_TEST_LOG=1` to see log output from tests.

## Definition of Done

A change is done only when all of these hold:

1. Builds warning-free in Debug, Release and Dist (CI covers Windows, macOS and Linux).
2. All tests pass, including the sanitizer preset. New logic has unit tests; fixed bugs have regression tests.
3. `python3 tools/check_style.py` passes (clang-format plus project lint rules, including CMake source lists).
4. New components follow the `add-component` skill; new script API follows `add-script-api`; both are exercised by
   the FeatureTest project (enforced by tests from Phase 11).
5. Docs are updated: `docs/Roadmap.md` status, `docs/Architecture.md` for design changes, this file for convention
   changes, API references for public API changes.
6. The `code-review` skill has been run over the full diff and every finding is fixed or explicitly justified.

## Code style (Hazel conventions)

The reference is the Hazel developer guide. When in doubt, look at existing code in this repository and blend in.
Code should read as plainly as possible: descriptive names, straightforward control flow, explicit types where
they help the reader, and no clever template or macro machinery unless it removes a real source of bugs (and then
it gets a comment explaining why, eg. the key code list in `KeyCodes.h`).

### Naming

| Kind | Style | Example |
|---|---|---|
| Namespaces, classes, structs, enums, type aliases | PascalCase | `Odyssey`, `LayerStack`, `LogLevel` |
| Functions and member functions | PascalCase | `PushLayer()`, `GetSize()` |
| Enum values (prefer `enum class`) | PascalCase | `LogLevel::Warn` |
| Local variables and parameters | camelCase | `layerCount`, `filepath` |
| Private/protected member variables | `m_` + PascalCase | `m_LayerInsertIndex` |
| Public data members (plain structs, components) | PascalCase | `Translation`, `Handled` |
| Static storage (file scope, anonymous namespace, class static, function static, `thread_local`), constants included | `s_` + PascalCase | `s_CoreLogger`, `s_KeyCodeNames` |
| Macros | `OD_` + UPPER_SNAKE_CASE | `OD_CORE_ASSERT` |
| Template parameters | PascalCase | `T`, `Args` |
| Source files | PascalCase, named after the main type | `LayerStack.h`, `LayerStack.cpp` |

Getters are `GetX()`, boolean queries are `IsX()`/`HasX()`/`ShouldX()`, setters are `SetX()`.
Lower-case names are allowed only where the language or STL interop requires them (`begin`/`end` for range-for,
`std::hash` specializations).

### Formatting

`.clang-format` is authoritative; run `python3 tools/check_style.py --fix`. In short: tabs for indentation,
Allman braces, everything inside `namespace Odyssey {` is indented, with a blank line after the opening brace and
before the closing one; `T* pointer` and `T& reference`; no column limit, but keep lines readable (around 140);
short inline getters may stay on one line. Single-statement `if`/`for` bodies may omit braces but go on their own
line. Access specifiers are repeated to separate member functions from member variables:

```cpp
namespace Odyssey {

	class Example
	{
	public:
		void DoThing(int inputValue);

		uint32_t GetCount() const { return m_Count; }
	private:
		void Helper();
	private:
		uint32_t m_Count = 0;
	};

}
```

### Files and includes

- Headers start with `#pragma once`. Include paths are rooted at a target's `src/`: `#include "Odyssey/Core/Log.h"`.
- Engine `.cpp` order: `"odpch.h"`, the file's own header, other Odyssey headers, third-party headers, standard
  headers, then platform headers. Headers include what they use; never rely on the precompiled header in a header.
- Source files are listed explicitly in their target's `CMakeLists.txt` (`check_style.py` enforces this).
- File-local helpers go in an anonymous namespace in the `.cpp`. No `using namespace` in headers.
- Platform headers (`windows.h`, X11, Cocoa/AppKit) only in platform-specific `.cpp` files, included last.
  Avoid names that collide with Win32 macros: `FormatMessage`, `GetMessage`, `CreateWindow`, `LoadImage`,
  `DrawText`, `GetObject`, `CreateFile`, `DeleteFile`, `CopyFile`, `MoveFile`, `GetCurrentDirectory`,
  `GetEnvironmentVariable`, `PlaySound`, `SendMessage`, `CreateEvent`, `near`, `far`, `min`, `max`.

### C++ usage

- C++20. Use the standard library first (`<format>`, `<span>`, `<filesystem>`, `<chrono>`).
- **Ownership:** `Scope<T>` (unique) by default, `Ref<T>` (shared) only for genuinely shared ownership. Raw pointers
  and references are non-owning. No naked `new`/`delete` outside deliberate exceptions with a comment (eg. leaked
  singletons that must survive static destruction).
- **Errors:** engine code does not throw. Report recoverable failures through return values (`bool`,
  `std::optional`, a result struct) and log the reason with context. Third-party code that throws (eg. JSON parsing)
  is wrapped at the boundary and converted to an error result.
- **Asserts:** `OD_CORE_ASSERT` (engine) / `OD_ASSERT` (client) check assumptions in Debug and Release and compile
  out of Dist; never put side effects in them. `OD_CORE_VERIFY` / `OD_VERIFY` are always evaluated and fatal: use
  them for invariants whose violation would corrupt memory or state. Messages use `std::format` syntax:
  `OD_CORE_VERIFY(index < count, "Index {} out of range ({})", index, count);`. Wrap conditions containing
  top-level commas in parentheses.
- **Logging:** `OD_CORE_TRACE/INFO/WARN/ERROR/CRITICAL` in the engine, `OD_*` in applications. Never write to
  `stdout`/`stderr` directly. Log messages state what failed and include the relevant values and paths.
  Paths are logged with `FileSystem::ToUTF8String()` (never `path::string()`, which is lossy on Windows).
- **Paths in project files** are stored with `FileSystem::ToGenericUTF8String()` (forward slashes, UTF-8, relative
  to the project root). Identifiers are stored as `UUID::ToString()` strings.
- **Threading:** the main thread owns the window, input, scene and render submission. Types document their thread
  safety in a header comment when it is not "main thread only".
- **Performance:** no heap allocation or `std::function` in per-entity or per-frame hot loops when it can be
  avoided; reserve and reuse containers. Measure before optimizing; never trade readability for unmeasured gains.
- **Comments** explain *why*, not *what*. Public API gets a short comment when its contract is not obvious from
  the signature (ownership, units, thread safety, failure behaviour).
- No `TODO` without a matching entry in `docs/Roadmap.md`.

## Testing rules

- Framework: doctest. Tests live in `OdysseyTests/src/<Module>/<Type>Tests.cpp`, mirroring the engine.
- Test names: `"<Type>: <behaviour in plain English>"`, eg. `"Buffer: out-of-range access is verified"`.
- Use `Testing::CaptureAssertionFailure()` to test asserts/verifies and `Testing::TemporaryDirectory` for files.
- Never use `&&` or `||` at the top level of `CHECK`/`REQUIRE` (doctest cannot decompose them); write two checks.
- Tests must be deterministic: no dependence on timing upper bounds, wall-clock time, test order, network,
  or machine-specific paths.
- GPU tests (Phase 1+) render offscreen and compare against golden images; they run on Mesa lavapipe in CI.
- The FeatureTest project (Phase 11) must cover every component and every script API function; adding either
  without coverage fails the build.

## Dependencies

Pinned exact versions only, fetched with `FetchContent` in `cmake/OdysseyDependencies.cmake`, consumed as
`SYSTEM` includes, never given Odyssey's warning flags. Add a dependency only together with the code that uses it,
and record it in the table in `docs/Architecture.md`. Follow the `add-dependency` skill.

## Skills

Task playbooks live in `.claude/skills/`. Use them:

| Skill | Use when |
|---|---|
| `build-and-test` | Building, running tests, diagnosing failures |
| `write-tests` | Adding or changing any logic |
| `code-review` | Before declaring any change done |
| `implement-roadmap-phase` | Starting or continuing work on a phase of `docs/Roadmap.md` |
| `add-dependency` | Adding or upgrading a third-party library |
| `add-component` | Adding an ECS component (Phase 2+) |
| `add-script-api` | Exposing anything to Lua (Phase 7+) |
