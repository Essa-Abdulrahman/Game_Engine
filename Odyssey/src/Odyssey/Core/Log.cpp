#include "odpch.h"
#include "Odyssey/Core/Log.h"

#include "Odyssey/Core/Environment.h"
#include "Odyssey/Core/StringUtils.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#if defined(OD_PLATFORM_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif

namespace Odyssey {

	namespace {

		std::FILE* GetConsoleFile(ConsoleStream stream)
		{
			return stream == ConsoleStream::StdErr ? stderr : stdout;
		}

		bool IsTerminal(std::FILE* file)
		{
#if defined(OD_PLATFORM_WINDOWS)
			return _isatty(_fileno(file)) != 0;
#else
			return isatty(fileno(file)) != 0;
#endif
		}

		bool EnableVirtualTerminalProcessing(ConsoleStream stream)
		{
#if defined(OD_PLATFORM_WINDOWS)
			HANDLE handle = GetStdHandle(stream == ConsoleStream::StdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
			if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
				return false;

			DWORD mode = 0;
			if (!GetConsoleMode(handle, &mode))
				return false;

			if (!SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
				return false;

			SetConsoleOutputCP(CP_UTF8);
			return true;
#else
			(void)stream;
			return true;
#endif
		}

		const char* GetLevelColor(LogLevel level)
		{
			switch (level)
			{
				case LogLevel::Trace:    return "\x1b[37m";
				case LogLevel::Info:     return "\x1b[32m";
				case LogLevel::Warn:     return "\x1b[33m";
				case LogLevel::Error:    return "\x1b[31m";
				case LogLevel::Critical: return "\x1b[1;41m";
				case LogLevel::Off:      return "";
			}
			return "";
		}

		constexpr const char* s_ColorReset = "\x1b[0m";

		struct LogData
		{
			std::mutex Mutex;
			bool Initialized = false;
			std::vector<Ref<LogSink>> Sinks;
			Ref<MemoryLogSink> MemorySink;
		};

		// The log state and loggers are intentionally never destroyed so that logging stays valid
		// during static destruction (eg. from destructors of other globals).
		LogData& GetLogData()
		{
			static LogData* s_Data = new LogData();
			return *s_Data;
		}

	}

	const char* LogLevelToString(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Trace:    return "trace";
			case LogLevel::Info:     return "info";
			case LogLevel::Warn:     return "warn";
			case LogLevel::Error:    return "error";
			case LogLevel::Critical: return "critical";
			case LogLevel::Off:      return "off";
		}
		return "unknown";
	}

	std::optional<LogLevel> LogLevelFromString(std::string_view name)
	{
		constexpr LogLevel levels[] = { LogLevel::Trace, LogLevel::Info, LogLevel::Warn, LogLevel::Error, LogLevel::Critical, LogLevel::Off };
		for (LogLevel level : levels)
		{
			if (StringUtils::EqualsIgnoreCase(name, LogLevelToString(level)))
				return level;
		}
		return std::nullopt;
	}

	std::string FormatLogLine(const LogMessage& message)
	{
		using namespace std::chrono;

		const std::time_t time = system_clock::to_time_t(message.Time);
		std::tm localTime = {};
#if defined(OD_PLATFORM_WINDOWS)
		localtime_s(&localTime, &time);
#else
		localtime_r(&time, &localTime);
#endif
		const auto milliseconds = duration_cast<std::chrono::milliseconds>(message.Time.time_since_epoch()).count() % 1000;

		return std::format("[{:02}:{:02}:{:02}.{:03}] [{}] {}: {}",
			localTime.tm_hour, localTime.tm_min, localTime.tm_sec, milliseconds,
			LogLevelToString(message.Level), message.LoggerName, message.Text);
	}

	// -----------------------------------------------------------------------------------------
	// LogSink
	// -----------------------------------------------------------------------------------------

	void LogSink::Write(const LogMessage& message)
	{
		if (message.Level == LogLevel::Off || message.Level < GetLevel())
			return;

		std::scoped_lock lock(m_Mutex);
		WriteImpl(message);
	}

	void LogSink::Flush()
	{
		std::scoped_lock lock(m_Mutex);
		FlushImpl();
	}

	// -----------------------------------------------------------------------------------------
	// ConsoleLogSink
	// -----------------------------------------------------------------------------------------

	ConsoleLogSink::ConsoleLogSink(ConsoleStream stream)
		: m_Stream(stream)
	{
		// https://no-color.org
		const std::optional<std::string> noColor = Environment::GetVariable("NO_COLOR");
		const bool colorDisabledByUser = noColor.has_value() && !noColor->empty();
		if (!colorDisabledByUser && IsTerminal(GetConsoleFile(stream)))
			m_ColorEnabled = EnableVirtualTerminalProcessing(stream);
	}

	void ConsoleLogSink::WriteImpl(const LogMessage& message)
	{
		std::FILE* file = GetConsoleFile(m_Stream);
		const std::string line = FormatLogLine(message);

		if (m_ColorEnabled)
			std::fprintf(file, "%s%s%s\n", GetLevelColor(message.Level), line.c_str(), s_ColorReset);
		else
			std::fprintf(file, "%s\n", line.c_str());

		// Flushing every line keeps output ordered and complete when piped to tools and agents.
		std::fflush(file);
	}

	void ConsoleLogSink::FlushImpl()
	{
		std::fflush(GetConsoleFile(m_Stream));
	}

	// -----------------------------------------------------------------------------------------
	// FileLogSink
	// -----------------------------------------------------------------------------------------

	FileLogSink::FileLogSink(const std::filesystem::path& filepath, bool truncate)
	{
		std::error_code error;
		if (filepath.has_parent_path())
			std::filesystem::create_directories(filepath.parent_path(), error);

		const std::ios::openmode mode = std::ios::out | std::ios::binary | (truncate ? std::ios::trunc : std::ios::app);
		m_Stream.open(filepath, mode);
	}

	bool FileLogSink::IsOpen() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_Stream.is_open();
	}

	void FileLogSink::WriteImpl(const LogMessage& message)
	{
		if (!m_Stream.is_open())
			return;

		m_Stream << FormatLogLine(message) << '\n';

		// Errors are flushed immediately so they survive a crash that follows them.
		if (message.Level >= LogLevel::Error)
			m_Stream.flush();
	}

	void FileLogSink::FlushImpl()
	{
		if (m_Stream.is_open())
			m_Stream.flush();
	}

	// -----------------------------------------------------------------------------------------
	// MemoryLogSink
	// -----------------------------------------------------------------------------------------

	MemoryLogSink::MemoryLogSink(size_t capacity)
		: m_Capacity(std::max<size_t>(capacity, 1))
	{
	}

	std::vector<MemoryLogEntry> MemoryLogSink::GetEntries(uint64_t afterSequence) const
	{
		std::scoped_lock lock(m_Mutex);

		std::vector<MemoryLogEntry> result;
		for (const MemoryLogEntry& entry : m_Entries)
		{
			if (entry.Sequence > afterSequence)
				result.push_back(entry);
		}
		return result;
	}

	uint64_t MemoryLogSink::GetLatestSequence() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_NextSequence - 1;
	}

	void MemoryLogSink::Clear()
	{
		std::scoped_lock lock(m_Mutex);
		m_Entries.clear();
	}

	void MemoryLogSink::WriteImpl(const LogMessage& message)
	{
		if (m_Entries.size() == m_Capacity)
			m_Entries.pop_front();

		m_Entries.push_back({ m_NextSequence++, message });
	}

	// -----------------------------------------------------------------------------------------
	// Logger
	// -----------------------------------------------------------------------------------------

	Logger::Logger(std::string name)
		: m_Name(std::move(name))
	{
	}

	void Logger::AddSink(Ref<LogSink> sink)
	{
		if (!sink)
			return;

		std::scoped_lock lock(m_Mutex);
		if (std::find(m_Sinks.begin(), m_Sinks.end(), sink) == m_Sinks.end())
			m_Sinks.push_back(std::move(sink));
	}

	void Logger::RemoveSink(const Ref<LogSink>& sink)
	{
		std::scoped_lock lock(m_Mutex);
		m_Sinks.erase(std::remove(m_Sinks.begin(), m_Sinks.end(), sink), m_Sinks.end());
	}

	void Logger::ClearSinks()
	{
		std::scoped_lock lock(m_Mutex);
		m_Sinks.clear();
	}

	size_t Logger::GetSinkCount() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_Sinks.size();
	}

	void Logger::Submit(LogLevel level, std::string text)
	{
		if (!ShouldLog(level))
			return;

		LogMessage message;
		message.Level = level;
		message.LoggerName = m_Name;
		message.Text = std::move(text);
		message.Time = std::chrono::system_clock::now();

		std::scoped_lock lock(m_Mutex);
		for (const Ref<LogSink>& sink : m_Sinks)
			sink->Write(message);
	}

	void Logger::Flush()
	{
		std::scoped_lock lock(m_Mutex);
		for (const Ref<LogSink>& sink : m_Sinks)
			sink->Flush();
	}

	// -----------------------------------------------------------------------------------------
	// Log
	// -----------------------------------------------------------------------------------------

	void Log::Init(const LogSpecification& specification)
	{
		Shutdown();

		LogData& data = GetLogData();
		std::scoped_lock lock(data.Mutex);

		if (specification.EnableConsole)
		{
			Ref<ConsoleLogSink> consoleSink = CreateRef<ConsoleLogSink>(specification.ConsoleTarget);
			consoleSink->SetLevel(specification.ConsoleLevel);
			data.Sinks.push_back(consoleSink);
		}

		if (!specification.FilePath.empty())
		{
			Ref<FileLogSink> fileSink = CreateRef<FileLogSink>(specification.FilePath);
			fileSink->SetLevel(specification.FileLevel);
			if (fileSink->IsOpen())
				data.Sinks.push_back(fileSink);
			else
				std::fprintf(stderr, "Odyssey: failed to open log file, file logging is disabled.\n");
		}

		if (specification.MemoryCapacity > 0)
		{
			data.MemorySink = CreateRef<MemoryLogSink>(specification.MemoryCapacity);
			data.Sinks.push_back(data.MemorySink);
		}

		Logger& coreLogger = GetCoreLogger();
		Logger& clientLogger = GetClientLogger();
		coreLogger.SetLevel(specification.CoreLevel);
		clientLogger.SetLevel(specification.ClientLevel);
		for (const Ref<LogSink>& sink : data.Sinks)
		{
			coreLogger.AddSink(sink);
			clientLogger.AddSink(sink);
		}

		data.Initialized = true;
	}

	void Log::Shutdown()
	{
		LogData& data = GetLogData();
		std::scoped_lock lock(data.Mutex);
		if (!data.Initialized)
			return;

		Logger& coreLogger = GetCoreLogger();
		Logger& clientLogger = GetClientLogger();
		coreLogger.Flush();
		clientLogger.Flush();
		coreLogger.ClearSinks();
		clientLogger.ClearSinks();

		data.Sinks.clear();
		data.MemorySink.reset();
		data.Initialized = false;
	}

	bool Log::IsInitialized()
	{
		LogData& data = GetLogData();
		std::scoped_lock lock(data.Mutex);
		return data.Initialized;
	}

	Logger& Log::GetCoreLogger()
	{
		static Logger* s_CoreLogger = new Logger("ODYSSEY");
		return *s_CoreLogger;
	}

	Logger& Log::GetClientLogger()
	{
		static Logger* s_ClientLogger = new Logger("APP");
		return *s_ClientLogger;
	}

	Ref<MemoryLogSink> Log::GetMemorySink()
	{
		LogData& data = GetLogData();
		std::scoped_lock lock(data.Mutex);
		return data.MemorySink;
	}

	void Log::Flush()
	{
		GetCoreLogger().Flush();
		GetClientLogger().Flush();
	}

}
