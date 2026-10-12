---
name: odyssey-code-style
description: Hazel-derived C++ code style for the Odyssey engine. Use whenever writing, editing, or reviewing any C++ or shader code in this repository.
---

# Odyssey Code Style

Source of truth: Hazel's Developer Guide naming rules, plus the specifics below. The goal is to *blend in*:
when unsure, mirror the surrounding code. `clang-format` (repo `.clang-format`) handles whitespace,
and review covers everything else.

## Naming
| Symbol | Style | Example |
|---|---|---|
| Classes, structs, enums, enum values, namespaces, functions, methods, files | PascalCase | `SceneRenderer`, `ShadowQuality::High`, `LoadTexture()`, `SceneRenderer.cpp` |
| Local variables, parameters | camelCase | `cascadeIndex`, `deltaTime` |
| Private/protected members | `m_` + PascalCase | `m_FramebufferSize` |
| Statics (TU or class) | `s_` + PascalCase | `s_Instance` |
| Globals (avoid) | `g_` + PascalCase | `g_AllocationCount` |
| Macros | `OD_` + UPPER_SNAKE | `OD_CORE_ASSERT` |
| Public POD fields (components, specs) | PascalCase, no prefix | `TransformComponent::Translation` |
| Template params | PascalCase | `template<typename TComponent>` |
| Lua API | PascalCase modules/functions | `Physics.Raycast`, `entity:GetComponent("RigidBody")` |
| GLSL | `u_` uniforms, `a_` inputs, `v_` varyings, `o_` outputs | |

## Layout
```cpp
#pragma once

#include "Odyssey/Core/Base.h"

namespace Odyssey {

	class ShadowPass
	{
	public:
		explicit ShadowPass(const ShadowPassSpecification& specification);

		void Execute(nvrhi::ICommandList* commandList, const SceneRenderData& renderData);
	private:
		void UpdateCascades(const CameraData& camera);
	private:
		ShadowPassSpecification m_Specification;
		std::array<CascadeData, 4> m_Cascades{};
	};

}
```
- Tabs, Allman braces for types, functions and control flow. Namespace brace stays on the same
  line and its contents are indented.
- Order: public API, then private functions, then private data, each with its own access label.
- Includes: own header first, then `Odyssey/...`, then vendor, then std, separated by blank lines.
  The precompiled header is `odpch.h`.

## Semantics
- `Ref<T>` = `std::shared_ptr`, `Scope<T>` = `std::unique_ptr`, created with `CreateRef`/`CreateScope`.
  Raw pointers are non-owning observers only.
- No exceptions cross engine API boundaries. Recoverable failures return `Result<T>` (value or `Error`
  with message). Programmer errors use `OD_CORE_ASSERT` / `OD_CORE_VERIFY`.
- Logging: `OD_CORE_TRACE/INFO/WARN/ERROR` in the engine, `OD_*` in the editor and runtime.
- `const` correctness everywhere; `[[nodiscard]]` on functions returning `Result` or handles.
- No `using namespace` in headers. No `auto` where the type isn't obvious from the right-hand side.
- Prefer explicit loops to dense `<algorithm>`/ranges chains when readability suffers (Hazel's
  "read like an English book" rule).
- Comments explain *why*. Doxygen `///` only on public engine API.
- No magic numbers in rendering or physics math. Name each as a PascalCase `constexpr` with its unit
  in the name, e.g. `constexpr float ShadowDepthBiasTexels = 1.5f;`.
- Platform code lives only in `Platform/`, behind an interface.
- Every new source file gets a matching test file in `Odyssey-Tests/` (see `odyssey-testing`).
