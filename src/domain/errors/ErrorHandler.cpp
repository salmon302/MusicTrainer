#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/RecoveryStrategy.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <future>

namespace MusicTrainer {

ErrorHandler& ErrorHandler::getInstance() {
	static ErrorHandler instance;
	return instance;
}

void ErrorHandler::registerRecoveryStrategy(const std::string& errorType,
										  RecoveryCallback strategy) {
	::utils::TrackedUniqueLock lock(mutex_, "ErrorHandler::mutex_", ::utils::LockLevel::ERROR_HANDLER);
	auto& config = handlers_[errorType];
	config.recoveryStrategies.push_back(std::move(strategy));
}

std::future<void> ErrorHandler::handleError(const MusicTrainerError& error) {
	auto errorType = error.getType();
	auto promise = std::make_shared<std::promise<void>>();
	
	try {
		// First get the handler (ERROR_HANDLER level 9)
		ErrorCallback handler;
		{
			::utils::TrackedUniqueLock errorLock(mutex_, "ErrorHandler::mutex_", ::utils::LockLevel::ERROR_HANDLER);
			auto it = handlers_.find(errorType);
			handler = (it != handlers_.end()) ? it->second.handler : globalHandler_;
		}
		
		// Then attempt recovery (RECOVERY level 10)
		auto& recovery = RecoveryStrategy::getInstance();
		auto result = recovery.attemptRecovery(error);
		
		// If recovery failed and we have a handler, use it
		if (!result.successful && handler) {
			handler(error);
		} else if (!result.successful) {
			std::cerr << "Unhandled error: " << error.what() << std::endl;
		}
		
		promise->set_value();
	} catch (const std::exception& e) {
		promise->set_exception(std::current_exception());
	}
	
	return promise->get_future();


}

void ErrorHandler::setGlobalHandler(ErrorCallback handler) {
	::utils::TrackedUniqueLock lock(mutex_, "ErrorHandler::mutex_", ::utils::LockLevel::ERROR_HANDLER);
	globalHandler_ = std::move(handler);
}

void ErrorHandler::clearHandlers() {
	::utils::TrackedUniqueLock lock(mutex_, "ErrorHandler::mutex_", ::utils::LockLevel::ERROR_HANDLER);
	handlers_.clear();
	globalHandler_ = nullptr;
}

} // namespace MusicTrainer

