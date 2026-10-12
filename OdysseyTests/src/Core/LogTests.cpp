#include <doctest/doctest.h>

#include "TestUtilities.h"

#include "Odyssey/Core/FileSystem.h"
#include "Odyssey/Core/Log.h"

#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace Odyssey;

TEST_CASE("Log: level names round-trip and parse case-insensitively")
{
	for (LogLevel level : { LogLevel::Trace, LogLevel::Info, LogLevel::Warn, LogLevel::Error, LogLevel::Critical, LogLevel::Off })
	{
		const std::optional<LogLevel> parsed = LogLevelFromString(LogLevelToString(level));
		REQUIRE(parsed.has_value());
		CHECK(*parsed == level);
	}

	CHECK(LogLevelFromString("WARN") == LogLevel::Warn);
	CHECK(LogLevelFromString("Critical") == LogLevel::Critical);
	CHECK_FALSE(LogLevelFromString("warning").has_value());
	CHECK_FALSE(LogLevelFromString("").has_value());
}

TEST_CASE("Log: FormatLogLine contains level, logger name and text")
{
	LogMessage message;
	message.Level = LogLevel::Warn;
	message.LoggerName = "ODYSSEY";
	message.Text = "Something happened";
	message.Time = std::chrono::system_clock::now();

	const std::string line = FormatLogLine(message);
	CHECK(line.find("[warn] ODYSSEY: Something happened") != std::string::npos);
	CHECK(line.front() == '[');
	CHECK(line.find('\n') == std::string::npos);
}

TEST_CASE("Log: logger formats messages and forwards them to sinks")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(16);
	logger.AddSink(sink);

	logger.Info("Loaded {} meshes in {:.1f} ms", 12, 3.26);

	const std::vector<MemoryLogEntry> entries = sink->GetEntries();
	REQUIRE(entries.size() == 1);
	CHECK(entries[0].Message.Text == "Loaded 12 meshes in 3.3 ms");
	CHECK(entries[0].Message.LoggerName == "TEST");
	CHECK(entries[0].Message.Level == LogLevel::Info);
	CHECK(entries[0].Sequence == 1);
}

TEST_CASE("Log: logger level filters messages")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(16);
	logger.AddSink(sink);
	logger.SetLevel(LogLevel::Warn);

	logger.Trace("trace");
	logger.Info("info");
	logger.Warn("warn");
	logger.Error("error");

	const std::vector<MemoryLogEntry> entries = sink->GetEntries();
	REQUIRE(entries.size() == 2);
	CHECK(entries[0].Message.Text == "warn");
	CHECK(entries[1].Message.Text == "error");

	logger.SetLevel(LogLevel::Off);
	logger.Critical("critical");
	CHECK(sink->GetEntries().size() == 2);
	CHECK_FALSE(logger.ShouldLog(LogLevel::Off));
}

TEST_CASE("Log: sink level filters independently of the logger")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> verboseSink = CreateRef<MemoryLogSink>(16);
	Ref<MemoryLogSink> errorSink = CreateRef<MemoryLogSink>(16);
	errorSink->SetLevel(LogLevel::Error);
	logger.AddSink(verboseSink);
	logger.AddSink(errorSink);

	logger.Info("info");
	logger.Error("error");

	CHECK(verboseSink->GetEntries().size() == 2);
	REQUIRE(errorSink->GetEntries().size() == 1);
	CHECK(errorSink->GetEntries()[0].Message.Text == "error");
}

TEST_CASE("Log: sinks are added once and can be removed")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(16);
	logger.AddSink(sink);
	logger.AddSink(sink);
	logger.AddSink(nullptr);
	CHECK(logger.GetSinkCount() == 1);

	logger.Info("once");
	CHECK(sink->GetEntries().size() == 1);

	logger.RemoveSink(sink);
	CHECK(logger.GetSinkCount() == 0);
	logger.Info("not recorded");
	CHECK(sink->GetEntries().size() == 1);
}

TEST_CASE("Log: memory sink keeps only the most recent entries")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(3);
	logger.AddSink(sink);

	for (int i = 1; i <= 5; i++)
		logger.Info("message {}", i);

	const std::vector<MemoryLogEntry> entries = sink->GetEntries();
	REQUIRE(entries.size() == 3);
	CHECK(entries[0].Message.Text == "message 3");
	CHECK(entries[2].Message.Text == "message 5");
	CHECK(entries[0].Sequence == 3);
	CHECK(entries[2].Sequence == 5);
	CHECK(sink->GetLatestSequence() == 5);
	CHECK(sink->GetCapacity() == 3);
}

TEST_CASE("Log: memory sink returns entries after a sequence number")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(8);
	logger.AddSink(sink);

	logger.Info("first");
	logger.Info("second");
	const uint64_t seen = sink->GetLatestSequence();
	logger.Info("third");

	const std::vector<MemoryLogEntry> newEntries = sink->GetEntries(seen);
	REQUIRE(newEntries.size() == 1);
	CHECK(newEntries[0].Message.Text == "third");

	sink->Clear();
	CHECK(sink->GetEntries().empty());
	CHECK(sink->GetLatestSequence() == 3);

	logger.Info("fourth");
	REQUIRE(sink->GetEntries(seen).size() == 1);
	CHECK(sink->GetEntries(seen)[0].Sequence == 4);
}

TEST_CASE("Log: memory sink capacity is at least one")
{
	MemoryLogSink sink(0);
	CHECK(sink.GetCapacity() == 1);
}

TEST_CASE("Log: concurrent logging delivers every message exactly once")
{
	Logger logger("TEST");
	Ref<MemoryLogSink> sink = CreateRef<MemoryLogSink>(10000);
	logger.AddSink(sink);

	constexpr int threadCount = 8;
	constexpr int messagesPerThread = 500;
	std::vector<std::thread> threads;
	for (int thread = 0; thread < threadCount; thread++)
	{
		threads.emplace_back([&logger, thread]()
		{
			for (int i = 0; i < messagesPerThread; i++)
				logger.Info("{}:{}", thread, i);
		});
	}
	for (std::thread& thread : threads)
		thread.join();

	const std::vector<MemoryLogEntry> entries = sink->GetEntries();
	REQUIRE(entries.size() == static_cast<size_t>(threadCount * messagesPerThread));

	std::set<std::string> uniqueTexts;
	for (size_t i = 0; i < entries.size(); i++)
	{
		uniqueTexts.insert(entries[i].Message.Text);
		CHECK(entries[i].Sequence == i + 1);
	}
	CHECK(uniqueTexts.size() == entries.size());
}

TEST_CASE("Log: file sink writes formatted lines")
{
	Testing::TemporaryDirectory directory;
	const std::filesystem::path logPath = directory.GetPath() / "Logs" / "Test.log";

	{
		Logger logger("FILE");
		Ref<FileLogSink> sink = CreateRef<FileLogSink>(logPath);
		REQUIRE(sink->IsOpen());
		logger.AddSink(sink);
		logger.Info("first line");
		logger.Error("second line");
		logger.Flush();
	}

	const std::optional<std::string> contents = FileSystem::ReadTextFile(logPath);
	REQUIRE(contents.has_value());
	CHECK(contents->find("[info] FILE: first line\n") != std::string::npos);
	CHECK(contents->find("[error] FILE: second line\n") != std::string::npos);
}

TEST_CASE("Log: global loggers route through the memory sink")
{
	Ref<MemoryLogSink> sink = Log::GetMemorySink();
	REQUIRE(sink != nullptr);
	CHECK(Log::IsInitialized());

	const uint64_t before = sink->GetLatestSequence();
	OD_CORE_WARN("core warning {}", 1);
	OD_INFO("client info {}", 2);

	const std::vector<MemoryLogEntry> entries = sink->GetEntries(before);
	REQUIRE(entries.size() == 2);
	CHECK(entries[0].Message.LoggerName == "ODYSSEY");
	CHECK(entries[0].Message.Text == "core warning 1");
	CHECK(entries[0].Message.Level == LogLevel::Warn);
	CHECK(entries[1].Message.LoggerName == "APP");
	CHECK(entries[1].Message.Text == "client info 2");
}
