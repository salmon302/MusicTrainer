#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/RecoveryStrategy.h"
#include <iostream>
#include <future>

namespace MusicTrainer {

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
		 // First try to handle the error using registered handlers
		bool handled = false;
		
		// Find the appropriate handler
		ErrorCallback handler;
		auto it = handlers.find(errorType);
		
		if (it != handlers.end() && it->second->handler) {
			std::cout << "Found specific handler for error type: " << errorType << std::endl;
			handler = it->second->handler;
			handled = true;
		} else if (globalHandler) {
			std::cout << "Using global handler for error type: " << errorType << std::endl;
			handler = globalHandler;
			handled = true;
		}
		
		// Try recovery strategies before calling the handler
		auto& strategy = RecoveryStrategy::getInstance();
		bool recovered = strategy.attemptRecovery(error).successful;
		
		// Call the handler even if recovery succeeded
		if (handled) {
			// Call the handler in a separate thread
			std::thread([=]() {
				try {
					std::cout << "Calling error handler for: " << errorType << std::endl;
					handler(error);
					std::cout << "Handler completed for: " << errorType << std::endl;
					promise->set_value();
				} catch (const std::exception& e) {
					std::cerr << "Error in handler: " << e.what() << std::endl;
					promise->set_exception(std::current_exception());
				}
			}).detach();
		} else {
			std::cerr << "No handler found for error type: " << errorType << std::endl;
			promise->set_value();
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception in handleError: " << e.what() << std::endl;
		promise->set_exception(std::current_exception());
	}
	
	return promise->get_future();
}

} // namespace MusicTrainer


