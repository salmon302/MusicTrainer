#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/RecoveryStrategy.h"
#include <iostream>
#include <future>

namespace MusicTrainer {

ErrorHandler& ErrorHandler::getInstance() {
	static ErrorHandler instance;
	return instance;
}

void ErrorHandler::registerRecoveryStrategy(const std::string& errorType,
										  RecoveryCallback strategy) {
	auto it = handlers.find(errorType);
	if (it == handlers.end()) {
		auto config = std::make_shared<ErrorConfig>();
		config->recoveryStrategies.push_back(std::move(strategy));
		handlers[errorType] = config;
	} else {
		it->second->recoveryStrategies.push_back(std::move(strategy));
	}
}

std::future<void> ErrorHandler::handleError(const MusicTrainerError& error) {
	auto errorType = error.getType();
	auto promise = std::make_shared<std::promise<void>>();
	
	try {
		// Find the appropriate handler
		ErrorCallback handler;
		auto it = handlers.find(errorType);
		if (it != handlers.end() && it->second->handler) {
			handler = it->second->handler;
		} else if (globalHandler) {
			handler = globalHandler;
		}
		
		// Attempt recovery
		auto& recovery = RecoveryStrategy::getInstance();
		auto result = recovery.attemptRecovery(error);
		
		// Handle the error if recovery failed
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

} // namespace MusicTrainer


