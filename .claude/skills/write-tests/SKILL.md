---
name: write-tests
description: Write unit and integration tests for Odyssey with doctest. Use whenever adding or changing engine logic, fixing a bug (regression test first), or reviewing whether a change is adequately tested.
---

# Writing Odyssey tests

## Where and how

- File: `OdysseyTests/src/<Module>/<Type>Tests.cpp`, mirroring `Odyssey/src/Odyssey/<Module>/<Type>.h`.
  Add the file to `OdysseyTests/CMakeLists.txt` (`check_style.py` fails otherwise).
- Name: `TEST_CASE("<Type>: <behaviour in plain English>")`, eg. `"LayerStack: pop detaches and returns ownership"`.
- One behaviour per test case. Arrange, act, check. Prefer `REQUIRE` for preconditions that later checks depend on
  (eg. `REQUIRE(result.has_value())` before dereferencing) and `CHECK` for everything else.
- Use `static_assert` for compile-time facts (type traits, constexpr values, GLFW value equality).

## What to cover for each unit

1. The normal path, with realistic values.
2. Boundaries: empty, zero, one, maximum, exactly at a limit, one past it.
3. Invalid input: malformed strings, missing files, null pointers, out-of-range indices. Recoverable errors must
   return failure and log. Invariant violations must trigger a verify; test that with
   `Testing::CaptureAssertionFailure()`.
4. State transitions and lifecycle: construction, move, destruction order, repeated calls, reuse after clear.
5. Concurrency, if the type claims to be thread-safe: hammer it from several threads and check the results are
   exact. Sanitizer CI (ThreadSanitizer locally) must stay clean.
6. Serialization: round trip, then byte-identical re-save; old-format migration; corrupt input.

## Helpers (`OdysseyTests/src/TestUtilities.h`)

- `Testing::CaptureAssertionFailure([&]() { ... })` returns the first assert/verify failure (expression, file,
  line, formatted message) instead of breaking or aborting.
- `Testing::TemporaryDirectory` is a unique empty directory, deleted recursively afterwards. Never write anywhere
  else.
- The global log has an in-memory sink: `Log::GetMemorySink()->GetEntries(sequenceBefore)` shows what was logged.

Assert-only behaviour must be guarded, because asserts are compiled out in Dist:

```cpp
#if defined(OD_ENABLE_ASSERTS)
TEST_CASE("Thing: asserts on misuse") { ... }
#endif
```

## doctest pitfalls

- Never write `&&` or `||` at the top level of `CHECK`/`REQUIRE`; doctest cannot decompose them. Split into two
  checks, or compute a named `bool` first.
- Comparing two `const char*` compares pointers. Wrap one side in `std::string` or `std::string_view`.
- Compare floats with `doctest::Approx(expected)` unless the value is exactly representable and copied bit-for-bit.

## Determinism rules

- No upper bounds on elapsed time, no sleeps used for synchronization, no dependence on test order or on other
  tests' global state (restore anything you change), no network, no absolute machine paths.
- Seed any randomness that affects expectations, or assert only properties that hold for every value.

## Bug fixes

Write the regression test first, watch it fail, then fix the bug and watch it pass. Name it after the behaviour,
not the bug ID.
