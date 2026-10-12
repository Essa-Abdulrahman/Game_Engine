---
name: build-and-test
description: Build Odyssey and run its tests and style checks. Use when compiling, running unit tests, checking a change before declaring it done, or diagnosing build, test, sanitizer or CI failures.
---

# Build and test Odyssey

## Full verification (run before declaring any change done)

```bash
cmake --preset debug   && cmake --build --preset debug   && ctest --preset debug
cmake --preset release && cmake --build --preset release && ctest --preset release
cmake --preset dist    && cmake --build --preset dist    && ctest --preset dist
cmake --preset debug-sanitizers && cmake --build --preset debug-sanitizers && ctest --preset debug-sanitizers   # Linux/macOS
python3 tools/check_style.py
```

Configure once per preset; afterwards `cmake --build --preset <name>` is enough. On Windows, run the Ninja presets
from an "x64 Native Tools Command Prompt for VS 2022", or use `vs2022` with `vs2022-debug` / `vs2022-release` /
`vs2022-dist` build and test presets.

## Fast iteration

- Build only the tests: `cmake --build --preset debug --target OdysseyTests`
- Run a subset: `build/debug/bin/OdysseyTests --test-case="Buffer*"`
  (doctest also accepts `--test-case-exclude=`, `--success`, `--duration=true`, `--list-test-cases`).
- See engine log output while testing: set `ODYSSEY_TEST_LOG=1`.
- Debug a failing test: run the test binary directly under a debugger with `--test-case="<exact name>"`.

## Diagnosing failures

- **Warning as error:** fix the code. Never add `#pragma warning(disable)`, `-Wno-*`, or casts that only hide a
  real problem. Conversion warnings usually mean a type should change, not that a cast is needed.
- **Fails only in Dist:** something depends on an assert's side effect, or on `OD_DEBUG`/`OD_RELEASE`-only code.
- **Fails only with sanitizers:** treat it as a real bug (use-after-free, overflow, data race), even if other
  configurations pass. Add a regression test.
- **Fails on one platform only:** look for path handling (use `FileSystem::ToUTF8String`), newline handling,
  `size_t` versus `uint64_t` differences, Win32 macro collisions (see AGENTS.md), and uninitialized values.
- **Flaky test:** find the nondeterminism (timing upper bounds, test order, shared global state, unseeded
  randomness) and remove it. Never retry or loosen a test to hide it.
- **`check_style.py` failures:** `--fix` applies clang-format. Lint violations (unlisted CMake sources, missing
  `#pragma once`, pch-first include, console output, NULL, file naming) must be fixed by hand.

## Rules

- A change is not done until the full verification passes. If you cannot run a configuration locally (eg. MSVC on
  Linux), say so explicitly in your summary and rely on CI for it.
- Never commit with known failing tests or a red CI.
