# Odyssey Engine

A production-grade, deliberately simple 3D game engine for Windows, macOS and Linux (Ubuntu 24+), built in C++20
on Vulkan (via NVRHI). It is designed to be operated fully by AI agents as well as people.

**Status:** Phase 0 (foundation) is complete: build system, CI, tooling, guidelines and a fully tested Core module.
See [docs/Roadmap.md](docs/Roadmap.md) for the plan and progress.

## Building

Requirements:

- CMake 3.25+ and Ninja.
- A C++20 compiler: MSVC 2022 17.8+, GCC 13+, Clang 17+ or Xcode 16+.
- Python 3.
- The Vulkan SDK, from Phase 1 onward.

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Presets: `debug`, `release`, `dist`, `debug-sanitizers` (Linux/macOS), and `vs2022` (Visual Studio, Windows).
Style check: `python3 tools/check_style.py` (`--fix` applies clang-format).

## Documentation

| Document | Contents |
|---|---|
| [AGENTS.md](AGENTS.md) | Development rules: code style (Hazel conventions), testing, Definition of Done |
| [docs/Architecture.md](docs/Architecture.md) | Design and rationale for every subsystem, data formats, dependencies |
| [docs/Roadmap.md](docs/Roadmap.md) | Phases, exit criteria and status |
| [.claude/skills/](.claude/skills/) | Step-by-step playbooks for common development tasks |

## Layout

| Path | Contents |
|---|---|
| `Odyssey/` | Engine static library |
| `OdysseyTests/` | Unit and integration tests (doctest) |
| `OdysseyEditor/`, `OdysseyRuntime/` | Editor and game player executables (upcoming phases) |
| `cmake/` | Build configuration |
| `tools/` | Developer tooling |
| `docs/` | Architecture and roadmap |
