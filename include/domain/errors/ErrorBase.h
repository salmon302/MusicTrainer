#pragma once
#include <stdexcept>
#include <string>
#include <chrono>

namespace MusicTrainer {

struct ErrorContext {
	std::string file;
	int line;
	std::string function;
	std::chrono::system_clock::time_point timestamp;
	std::string additionalInfo;

	ErrorContext(const char* f = __FILE__, 
				int l = __LINE__, 
				const char* fn = __FUNCTION__,
				std::string info = "")
		: file(f), line(l), function(fn)
		, timestamp(std::chrono::system_clock::now())
		, additionalInfo(std::move(info)) {}
};

class MusicTrainerError : public std::runtime_error {
public:
	explicit MusicTrainerError(const std::string& message, 
							 const std::string& type = "MusicTrainerError",
							 const ErrorContext& context = ErrorContext())
		: std::runtime_error(formatMessage(message, context))
		, type_(type)
		, context_(context) {}

	const ErrorContext& getContext() const { return context_; }
	const std::string& getType() const { return type_; }

protected:
	static std::string formatMessage(const std::string& message, 
								   const ErrorContext& context) {
		return context.file + ":" + 
			   std::to_string(context.line) + " [" + 
			   context.function + "] - " + message +
			   (context.additionalInfo.empty() ? "" : 
				"\nAdditional Info: " + context.additionalInfo);
	}

private:
	std::string type_;
	ErrorContext context_;
};

class DomainError : public MusicTrainerError {
public:
	explicit DomainError(const std::string& message,
						const std::string& type = "DomainError",
						const ErrorContext& context = ErrorContext())
		: MusicTrainerError(message, type, context) {}
};

class SystemError : public MusicTrainerError {
public:
	explicit SystemError(const std::string& message,
						const std::string& type = "SystemError",
						const ErrorContext& context = ErrorContext())
		: MusicTrainerError(message, type, context) {}
};

} // namespace MusicTrainer