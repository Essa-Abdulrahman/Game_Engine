#pragma once

#include "Odyssey/Core/Buffer.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace Odyssey {

	// File helpers. All functions are thread-safe with respect to each other, report failures through
	// their return value and log the reason. They never throw for I/O errors.
	class FileSystem
	{
	public:
		// Reads the whole file in binary mode (no newline translation).
		static std::optional<std::string> ReadTextFile(const std::filesystem::path& filepath);
		static std::optional<Buffer> ReadBinaryFile(const std::filesystem::path& filepath);

		// Atomically replaces the file: data is written to a temporary file in the same directory which is then
		// renamed over the target, so readers never observe a partially written file. Parent directories are
		// created as needed. Text is written in binary mode (no newline translation).
		static bool WriteTextFile(const std::filesystem::path& filepath, std::string_view contents);
		static bool WriteBinaryFile(const std::filesystem::path& filepath, std::span<const uint8_t> data);

		// UTF-8 conversions. Use these instead of path::string(), which is lossy on Windows.
		static std::string ToUTF8String(const std::filesystem::path& path);
		// Like ToUTF8String() but always uses '/' as the separator. Use for paths stored in project files.
		static std::string ToGenericUTF8String(const std::filesystem::path& path);
		static std::filesystem::path FromUTF8String(std::string_view utf8);
	};

}
