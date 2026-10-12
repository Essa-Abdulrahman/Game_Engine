#pragma once

#include "Odyssey/Core/Assert.h"

#include <filesystem>
#include <functional>
#include <optional>

namespace Odyssey::Testing {

	// Runs func with an assertion handler installed that turns OD_*_ASSERT / OD_*_VERIFY failures into an
	// exception, so tests can observe them without breaking into the debugger or aborting.
	// Returns the first failure, or nullopt if no assertion failed.
	std::optional<AssertionFailure> CaptureAssertionFailure(const std::function<void()>& func);

	// Creates a unique, empty directory under the system temporary directory and deletes it recursively on destruction.
	class TemporaryDirectory
	{
	public:
		TemporaryDirectory();
		~TemporaryDirectory();

		TemporaryDirectory(const TemporaryDirectory&) = delete;
		TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

		const std::filesystem::path& GetPath() const { return m_Path; }
	private:
		std::filesystem::path m_Path;
	};

}
