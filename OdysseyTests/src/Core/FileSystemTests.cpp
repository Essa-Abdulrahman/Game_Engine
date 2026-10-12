#include <doctest/doctest.h>

#include "TestUtilities.h"

#include "Odyssey/Core/FileSystem.h"

#include <vector>

using namespace Odyssey;

namespace {

	size_t CountDirectoryEntries(const std::filesystem::path& directory)
	{
		size_t count = 0;
		for ([[maybe_unused]] const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory))
			count++;
		return count;
	}

}

TEST_CASE("FileSystem: text files round-trip exactly")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / "Scene.json";

	const std::string contents = "{\n  \"name\": \"Test\"\r\n}\n";
	REQUIRE(FileSystem::WriteTextFile(filepath, contents));

	const std::optional<std::string> read = FileSystem::ReadTextFile(filepath);
	REQUIRE(read.has_value());
	CHECK(*read == contents);
}

TEST_CASE("FileSystem: writing replaces existing contents and leaves no temporary files")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / "Settings.txt";

	REQUIRE(FileSystem::WriteTextFile(filepath, "a much longer first version of the file"));
	REQUIRE(FileSystem::WriteTextFile(filepath, "short"));

	CHECK(FileSystem::ReadTextFile(filepath) == std::optional<std::string>("short"));
	CHECK(CountDirectoryEntries(directory.GetPath()) == 1);
}

TEST_CASE("FileSystem: writing creates missing parent directories")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / "Assets" / "Scenes" / "Main.json";

	REQUIRE(FileSystem::WriteTextFile(filepath, "{}"));
	CHECK(std::filesystem::is_regular_file(filepath));
}

TEST_CASE("FileSystem: empty files round-trip")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / "Empty.bin";

	REQUIRE(FileSystem::WriteBinaryFile(filepath, {}));
	const std::optional<Buffer> binary = FileSystem::ReadBinaryFile(filepath);
	REQUIRE(binary.has_value());
	CHECK(binary->IsEmpty());

	const std::optional<std::string> text = FileSystem::ReadTextFile(filepath);
	REQUIRE(text.has_value());
	CHECK(text->empty());
}

TEST_CASE("FileSystem: binary files preserve every byte value")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / "AllBytes.bin";

	std::vector<uint8_t> bytes;
	for (int repeat = 0; repeat < 4; repeat++)
	{
		for (int value = 0; value < 256; value++)
			bytes.push_back(static_cast<uint8_t>(value));
	}

	REQUIRE(FileSystem::WriteBinaryFile(filepath, bytes));
	const std::optional<Buffer> read = FileSystem::ReadBinaryFile(filepath);
	REQUIRE(read.has_value());
	REQUIRE(read->GetSize() == bytes.size());

	bool identical = true;
	for (size_t i = 0; i < bytes.size(); i++)
		identical = identical && read->GetData()[i] == bytes[i];
	CHECK(identical);
}

TEST_CASE("FileSystem: reading missing files and directories fails gracefully")
{
	Testing::TemporaryDirectory directory;
	CHECK_FALSE(FileSystem::ReadTextFile(directory.GetPath() / "Missing.txt").has_value());
	CHECK_FALSE(FileSystem::ReadBinaryFile(directory.GetPath() / "Missing.bin").has_value());
	CHECK_FALSE(FileSystem::ReadTextFile(directory.GetPath()).has_value());
}

TEST_CASE("FileSystem: writing to a path occupied by a directory fails gracefully")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path occupied = directory.GetPath() / "Occupied";
	std::filesystem::create_directory(occupied);

	CHECK_FALSE(FileSystem::WriteTextFile(occupied, "data"));
	CHECK(std::filesystem::is_directory(occupied));
	CHECK(CountDirectoryEntries(directory.GetPath()) == 1);
}

TEST_CASE("FileSystem: UTF-8 paths round-trip and work for file I/O")
{
	const std::string utf8Name = "Ass\xc3\xa9ts/\xd1\x84\xd0\xb0\xd0\xb9\xd0\xbb.txt";
	const std::filesystem::path path = FileSystem::FromUTF8String(utf8Name);
	CHECK(FileSystem::ToGenericUTF8String(path) == utf8Name);

	Testing::TemporaryDirectory directory;
	const std::filesystem::path filepath = directory.GetPath() / path;
	REQUIRE(FileSystem::WriteTextFile(filepath, "unicode"));
	CHECK(FileSystem::ReadTextFile(filepath) == std::optional<std::string>("unicode"));
}

TEST_CASE("FileSystem: generic strings always use forward slashes")
{
	std::filesystem::path path = "Assets";
	path /= "Textures";
	path /= "Brick.png";
	CHECK(FileSystem::ToGenericUTF8String(path) == "Assets/Textures/Brick.png");
}
