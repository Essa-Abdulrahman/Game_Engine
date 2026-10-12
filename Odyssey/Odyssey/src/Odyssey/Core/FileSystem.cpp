#include "odpch.h"
#include "Odyssey/Core/FileSystem.h"

#include "Odyssey/Core/Log.h"
#include "Odyssey/Core/UUID.h"

#include <format>
#include <fstream>

namespace Odyssey {

	namespace {

		std::optional<uint64_t> GetRegularFileSize(const std::filesystem::path& filepath)
		{
			std::error_code error;
			if (!std::filesystem::is_regular_file(filepath, error))
			{
				OD_CORE_ERROR("FileSystem: '{}' is not a readable file{}{}", FileSystem::ToUTF8String(filepath), error ? ": " : "", error ? error.message() : "");
				return std::nullopt;
			}

			const std::uintmax_t size = std::filesystem::file_size(filepath, error);
			if (error)
			{
				OD_CORE_ERROR("FileSystem: failed to query size of '{}': {}", FileSystem::ToUTF8String(filepath), error.message());
				return std::nullopt;
			}
			return static_cast<uint64_t>(size);
		}

		bool ReadExactly(const std::filesystem::path& filepath, char* destination, uint64_t size)
		{
			std::ifstream stream(filepath, std::ios::in | std::ios::binary);
			if (!stream)
			{
				OD_CORE_ERROR("FileSystem: failed to open '{}' for reading", FileSystem::ToUTF8String(filepath));
				return false;
			}

			if (size > 0)
				stream.read(destination, static_cast<std::streamsize>(size));

			if (static_cast<uint64_t>(stream.gcount()) != size)
			{
				OD_CORE_ERROR("FileSystem: failed to read '{}' (expected {} bytes, got {})", FileSystem::ToUTF8String(filepath), size, stream.gcount());
				return false;
			}
			return true;
		}

		bool WriteFileAtomically(const std::filesystem::path& filepath, const void* data, uint64_t size)
		{
			std::error_code error;
			const std::filesystem::path parent = filepath.parent_path();
			if (!parent.empty())
			{
				std::filesystem::create_directories(parent, error);
				if (error)
				{
					OD_CORE_ERROR("FileSystem: failed to create directory '{}': {}", FileSystem::ToUTF8String(parent), error.message());
					return false;
				}
			}

			// A random suffix keeps concurrent writers of the same file from sharing a temporary file.
			std::filesystem::path temporaryPath = filepath;
			temporaryPath += std::format(".{}.tmp", UUID().ToString());

			{
				std::ofstream stream(temporaryPath, std::ios::out | std::ios::binary | std::ios::trunc);
				if (!stream)
				{
					OD_CORE_ERROR("FileSystem: failed to open '{}' for writing", FileSystem::ToUTF8String(temporaryPath));
					return false;
				}

				if (size > 0)
					stream.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
				stream.flush();

				if (!stream)
				{
					OD_CORE_ERROR("FileSystem: failed to write '{}'", FileSystem::ToUTF8String(temporaryPath));
					stream.close();
					std::filesystem::remove(temporaryPath, error);
					return false;
				}
			}

			std::filesystem::rename(temporaryPath, filepath, error);
			if (error)
			{
				OD_CORE_ERROR("FileSystem: failed to replace '{}': {}", FileSystem::ToUTF8String(filepath), error.message());
				std::error_code removeError;
				std::filesystem::remove(temporaryPath, removeError);
				return false;
			}
			return true;
		}

	}

	std::optional<std::string> FileSystem::ReadTextFile(const std::filesystem::path& filepath)
	{
		const std::optional<uint64_t> size = GetRegularFileSize(filepath);
		if (!size)
			return std::nullopt;

		std::string contents(static_cast<size_t>(*size), '\0');
		if (!ReadExactly(filepath, contents.data(), *size))
			return std::nullopt;

		return contents;
	}

	std::optional<Buffer> FileSystem::ReadBinaryFile(const std::filesystem::path& filepath)
	{
		const std::optional<uint64_t> size = GetRegularFileSize(filepath);
		if (!size)
			return std::nullopt;

		Buffer buffer(*size);
		if (!ReadExactly(filepath, reinterpret_cast<char*>(buffer.GetData()), *size))
			return std::nullopt;

		return buffer;
	}

	bool FileSystem::WriteTextFile(const std::filesystem::path& filepath, std::string_view contents)
	{
		return WriteFileAtomically(filepath, contents.data(), contents.size());
	}

	bool FileSystem::WriteBinaryFile(const std::filesystem::path& filepath, std::span<const uint8_t> data)
	{
		return WriteFileAtomically(filepath, data.data(), data.size());
	}

	std::string FileSystem::ToUTF8String(const std::filesystem::path& path)
	{
		const std::u8string utf8 = path.u8string();
		return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
	}

	std::string FileSystem::ToGenericUTF8String(const std::filesystem::path& path)
	{
		const std::u8string utf8 = path.generic_u8string();
		return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
	}

	std::filesystem::path FileSystem::FromUTF8String(std::string_view utf8)
	{
		return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
	}

}
