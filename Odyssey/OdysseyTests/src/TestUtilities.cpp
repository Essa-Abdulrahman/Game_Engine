#include "TestUtilities.h"

#include "Odyssey/Core/UUID.h"

#include <stdexcept>

namespace Odyssey::Testing {

	namespace {

		struct AssertionFailedException
		{
			AssertionFailure Failure;
		};

		bool ThrowingAssertHandler(const AssertionFailure& failure)
		{
			throw AssertionFailedException { failure };
		}

		class ScopedAssertHandler
		{
		public:
			explicit ScopedAssertHandler(AssertHandler handler)
				: m_PreviousHandler(Assert::SetHandler(handler))
			{
			}

			~ScopedAssertHandler()
			{
				Assert::SetHandler(m_PreviousHandler);
			}

			ScopedAssertHandler(const ScopedAssertHandler&) = delete;
			ScopedAssertHandler& operator=(const ScopedAssertHandler&) = delete;
		private:
			AssertHandler m_PreviousHandler;
		};

	}

	std::optional<AssertionFailure> CaptureAssertionFailure(const std::function<void()>& func)
	{
		ScopedAssertHandler handler(&ThrowingAssertHandler);
		try
		{
			func();
		}
		catch (const AssertionFailedException& exception)
		{
			return exception.Failure;
		}
		return std::nullopt;
	}

	TemporaryDirectory::TemporaryDirectory()
	{
		m_Path = std::filesystem::temp_directory_path() / ("OdysseyTests-" + UUID().ToString());
		if (!std::filesystem::create_directories(m_Path))
			throw std::runtime_error("Failed to create temporary test directory");
	}

	TemporaryDirectory::~TemporaryDirectory()
	{
		std::error_code error;
		std::filesystem::remove_all(m_Path, error);
	}

}
