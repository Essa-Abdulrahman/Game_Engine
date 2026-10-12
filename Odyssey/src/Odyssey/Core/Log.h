#pragma once

#include "Odyssey/Core/Base.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Odyssey {

	enum class LogLevel : uint8_t
	{
		Trace = 0,
		Info,
		Warn,
		Error,
		Critical,
		Off
	};

	const char* LogLevelToString(LogLevel level);
	// Case-insensitive. Accepts the names produced by LogLevelToString().
	std::optional<LogLevel> LogLevelFromString(std::string_view name);

	struct LogMessage
	{
		LogLevel Level = LogLevel::Info;
		std::string LoggerName;
		std::string Text;
		std::chrono::system_clock::time_point Time;
	};

	// Formats a message as a single line without a trailing newline, eg. "[14:03:22.117] [warn] ODYSSEY: Text".
	std::string FormatLogLine(const LogMessage& message);

	// Base class for log outputs. Thread-safe: Write() and Flush() serialize access to the implementation.
	// Implementations must never log themselves (that would deadlock) and must never assert.
	class LogSink
	{
	public:
		virtual ~LogSink() = default;

		void Write(const LogMessage& message);
		void Flush();

		void SetLevel(LogLevel level) { m_Level.store(level, std::memory_order_relaxed); }
		LogLevel GetLevel() const { return m_Level.load(std::memory_order_relaxed); }
	protected:
		virtual void WriteImpl(const LogMessage& message) = 0;
		virtual void FlushImpl() {}
	protected:
		mutable std::mutex m_Mutex;
	private:
		std::atomic<LogLevel> m_Level = LogLevel::Trace;
	};

	enum class ConsoleStream : uint8_t
	{
		StdOut = 0,
		// Use StdErr whenever stdout carries machine-readable output (eg. the headless agent protocol).
		StdErr
	};

	class ConsoleLogSink : public LogSink
	{
	public:
		explicit ConsoleLogSink(ConsoleStream stream = ConsoleStream::StdOut);

		bool IsColorEnabled() const { return m_ColorEnabled; }
	protected:
		void WriteImpl(const LogMessage& message) override;
		void FlushImpl() override;
	private:
		ConsoleStream m_Stream;
		bool m_ColorEnabled = false;
	};

	class FileLogSink : public LogSink
	{
	public:
		// Creates parent directories as needed. Check IsOpen() afterwards.
		explicit FileLogSink(const std::filesystem::path& filepath, bool truncate = true);

		bool IsOpen() const;
	protected:
		void WriteImpl(const LogMessage& message) override;
		void FlushImpl() override;
	private:
		std::ofstream m_Stream;
	};

	struct MemoryLogEntry
	{
		// Strictly increasing per sink, starting at 1. Lets readers poll for new entries.
		uint64_t Sequence = 0;
		LogMessage Message;
	};

	// Keeps the most recent messages in a ring buffer. Backs the editor console and the agent protocol.
	class MemoryLogSink : public LogSink
	{
	public:
		explicit MemoryLogSink(size_t capacity);

		// Returns all retained entries with a sequence number greater than afterSequence, oldest first.
		std::vector<MemoryLogEntry> GetEntries(uint64_t afterSequence = 0) const;
		uint64_t GetLatestSequence() const;
		size_t GetCapacity() const { return m_Capacity; }
		void Clear();
	protected:
		void WriteImpl(const LogMessage& message) override;
	private:
		size_t m_Capacity;
		uint64_t m_NextSequence = 1;
		std::deque<MemoryLogEntry> m_Entries;
	};

	class Logger
	{
	public:
		explicit Logger(std::string name);

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		const std::string& GetName() const { return m_Name; }

		void SetLevel(LogLevel level) { m_Level.store(level, std::memory_order_relaxed); }
		LogLevel GetLevel() const { return m_Level.load(std::memory_order_relaxed); }
		bool ShouldLog(LogLevel level) const { return level != LogLevel::Off && level >= GetLevel(); }

		void AddSink(Ref<LogSink> sink);
		void RemoveSink(const Ref<LogSink>& sink);
		void ClearSinks();
		size_t GetSinkCount() const;

		// Sends an already formatted message to all sinks.
		void Submit(LogLevel level, std::string text);
		void Flush();

		template<typename... Args>
		void Write(LogLevel level, std::format_string<Args...> format, Args&&... args)
		{
			if (!ShouldLog(level))
				return;

			Submit(level, std::format(format, std::forward<Args>(args)...));
		}

		template<typename... Args>
		void Trace(std::format_string<Args...> format, Args&&... args) { Write(LogLevel::Trace, format, std::forward<Args>(args)...); }

		template<typename... Args>
		void Info(std::format_string<Args...> format, Args&&... args) { Write(LogLevel::Info, format, std::forward<Args>(args)...); }

		template<typename... Args>
		void Warn(std::format_string<Args...> format, Args&&... args) { Write(LogLevel::Warn, format, std::forward<Args>(args)...); }

		template<typename... Args>
		void Error(std::format_string<Args...> format, Args&&... args) { Write(LogLevel::Error, format, std::forward<Args>(args)...); }

		template<typename... Args>
		void Critical(std::format_string<Args...> format, Args&&... args) { Write(LogLevel::Critical, format, std::forward<Args>(args)...); }
	private:
		std::string m_Name;
		std::atomic<LogLevel> m_Level = LogLevel::Trace;
		mutable std::mutex m_Mutex;
		std::vector<Ref<LogSink>> m_Sinks;
	};

	struct LogSpecification
	{
		bool EnableConsole = true;
		ConsoleStream ConsoleTarget = ConsoleStream::StdOut;
		LogLevel ConsoleLevel = LogLevel::Trace;

		// Empty path disables file logging.
		std::filesystem::path FilePath;
		LogLevel FileLevel = LogLevel::Trace;

		// 0 disables the in-memory sink.
		size_t MemoryCapacity = 4096;

		LogLevel CoreLevel = LogLevel::Trace;
		LogLevel ClientLevel = LogLevel::Trace;
	};

	// Global engine ("ODYSSEY") and client ("APP") loggers.
	// The loggers always exist, so logging before Init() or after Shutdown() is a safe no-op.
	class Log
	{
	public:
		// Calling Init() again replaces the current configuration.
		static void Init(const LogSpecification& specification = {});
		static void Shutdown();
		static bool IsInitialized();

		static Logger& GetCoreLogger();
		static Logger& GetClientLogger();

		// Null when the in-memory sink is disabled or the log system is not initialized.
		static Ref<MemoryLogSink> GetMemorySink();

		static void Flush();
	};

}

// Engine logging
#define OD_CORE_TRACE(...)    ::Odyssey::Log::GetCoreLogger().Trace(__VA_ARGS__)
#define OD_CORE_INFO(...)     ::Odyssey::Log::GetCoreLogger().Info(__VA_ARGS__)
#define OD_CORE_WARN(...)     ::Odyssey::Log::GetCoreLogger().Warn(__VA_ARGS__)
#define OD_CORE_ERROR(...)    ::Odyssey::Log::GetCoreLogger().Error(__VA_ARGS__)
#define OD_CORE_CRITICAL(...) ::Odyssey::Log::GetCoreLogger().Critical(__VA_ARGS__)

// Client (game / editor / runtime) logging
#define OD_TRACE(...)         ::Odyssey::Log::GetClientLogger().Trace(__VA_ARGS__)
#define OD_INFO(...)          ::Odyssey::Log::GetClientLogger().Info(__VA_ARGS__)
#define OD_WARN(...)          ::Odyssey::Log::GetClientLogger().Warn(__VA_ARGS__)
#define OD_ERROR(...)         ::Odyssey::Log::GetClientLogger().Error(__VA_ARGS__)
#define OD_CRITICAL(...)      ::Odyssey::Log::GetClientLogger().Critical(__VA_ARGS__)
