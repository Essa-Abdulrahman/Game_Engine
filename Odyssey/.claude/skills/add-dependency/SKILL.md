---
name: add-dependency
description: Add or upgrade a third-party library in the Odyssey CMake build with exact pinning. Use when integrating any external library (GLFW, NVRHI, glm, EnTT, Jolt, Lua, sol2, miniaudio, ImGui, stb, cgltf, etc.) or bumping a pinned version.
---

# Adding a dependency

## Before adding

- It must be listed in `docs/Architecture.md` section 15, or you must add it there with its purpose, license and
  why existing code or dependencies do not cover the need. Prefer small, widely used, permissively licensed
  (MIT/zlib/BSD/public domain) libraries.
- Add it in the same change as the first code that uses it, never speculatively.

## Pinning

- Use an exact release tag (`GIT_TAG v3.13.2`) or a full 40-character commit SHA. Never use a branch name
  (`main`, `master`, `docking`) or `latest`.
- Confirm the tag exists (`git ls-remote --tags <repository> <tag>`) before writing it down.
- Record the exact pin in the section 15 table, and set its status to "Integrated".

## CMake (`cmake/OdysseyDependencies.cmake`)

```cmake
FetchContent_Declare(entt
	GIT_REPOSITORY https://github.com/skypjack/entt.git
	GIT_TAG        v3.13.2
	GIT_SHALLOW    TRUE
	SYSTEM)
FetchContent_MakeAvailable(entt)
```

- Use `SYSTEM` so its headers never produce warnings in our code. Never call `odyssey_configure_target` on
  third-party targets, and never change its warning flags.
- Turn off the dependency's own tests, examples, docs and install rules through its cache options before
  `FetchContent_MakeAvailable`, using `set(<OPTION> OFF CACHE BOOL "" FORCE)`.
- **Header-only or no usable CMake** (eg. doctest, stb, cgltf, Lua): declare with
  `SOURCE_SUBDIR odyssey-header-only` so its CMakeLists.txt is not run, then define our own target
  (`INTERFACE` for header-only; `STATIC` listing its sources, for example Lua) with `SYSTEM` include directories
  and a namespaced alias (`Lua::Lua`).
- For single-file libraries needing one implementation translation unit (stb, cgltf, miniaudio), put that
  definition in exactly one engine `.cpp` (for example `Odyssey/src/Odyssey/Asset/StbImpl.cpp`) with a comment.
- Link dependencies `PRIVATE` unless their types appear in Odyssey's public headers.

## Verify

- Run the full `build-and-test` verification on a clean build directory (delete `build/<preset>`), so cached
  state cannot hide problems.
- Check the CI configure step still works on all three platforms. Watch for CMake minimum-version deprecation
  errors in newer CMake releases; the `SOURCE_SUBDIR` approach avoids them for header-only libraries.

## Upgrading

Change the pin, read the library's changelog for breaking changes, run the full verification, and update the
table. Upgrade one dependency per change.
