#include "domain/errors/RecoveryStrategy.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorLogger.h"
#include <iostream>
#include <thread>

namespace MusicTrainer {

RecoveryStrategy& RecoveryStrategy::getInstance() {
	static RecoveryStrategy instance;
	return instance;
}

void RecoveryStrategy::registerStrategy(const std::string& errorType,
									  StrategyType type,
									  RecoveryAction action,
									  FallbackAction fallback) {
	auto strategy = std::make_unique<Strategy>();
	strategy->type = type;
	strategy->action = std::move(action);
	strategy->fallback = std::move(fallback);
	strategy->failureCount.store(0, std::memory_order_release);
	strategy->lastAttempt.store(
		std::chrono::system_clock::now().time_since_epoch().count(),
		std::memory_order_release);
	strategy->circuitOpen.store(false, std::memory_order_release);
	
	strategies[errorType] = std::move(strategy);
}

RecoveryStrategy::RecoveryResult RecoveryStrategy::attemptRecovery(const MusicTrainerError& error) {
	auto it = strategies.find(error.getType());
	if (it == strategies.end()) {
		return {false, "No recovery strategy registered", std::chrono::microseconds(0)};
	}
	
	auto& strategy = it->second;
	auto result = executeStrategy(*strategy, error);
	updateStrategyState(*strategy, result.successful);
	
	return result;
}

RecoveryStrategy::RecoveryResult RecoveryStrategy::executeStrategy(
	const Strategy& strategy,
	const MusicTrainerError& error) {
	
	auto attempt = [&](bool isFallback) -> RecoveryResult {
		try {
			bool success;
			if (isFallback && strategy.fallback) {
				success = strategy.fallback(error);
			} else {
				success = strategy.action(error);
			}
			return RecoveryResult{
				.successful = success,
				.message = success ? "Recovery successful" : "Recovery failed"
			};
		} catch (const std::exception& e) {
			return RecoveryResult{
				.successful = false,
				.message = std::string("Recovery threw exception: ") + e.what()
			};
		}
	};

	if (error.getType() == "ValidationError") {
		return attempt(false);
	}

	switch (strategy.type) {
		case StrategyType::RETRY: {
			auto errorType = error.getType();
			auto maxAttempts = config_.maxAttempts;
			
			RecoveryResult result;
			for (size_t i = 0; i < maxAttempts; ++i) {
				ErrorLogger::getInstance().logRecoveryAttempt(
					errorType, 
					"Attempt " + std::to_string(i + 1) + " of " + std::to_string(maxAttempts),
					"", ErrorLogger::LogLevel::INFO);
				
				result = attempt(false);
				
				if (result.successful) {
					ErrorLogger::getInstance().logRecoveryResult(
						errorType, true,
						"Retry successful on attempt " + std::to_string(i + 1),
						ErrorLogger::LogLevel::INFO);
					return result;
				}
				
				if (i < maxAttempts - 1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			
			ErrorLogger::getInstance().logRecoveryResult(
				errorType, false, 
				"All retry attempts failed, trying fallback",
				ErrorLogger::LogLevel::WARNING);
			
			return attempt(true);
		}

		case StrategyType::EXPONENTIAL_BACKOFF: {
			auto delay = config_.backoffInitial;
			for (size_t i = 0; i < config_.maxAttempts; ++i) {
				auto result = attempt(false);
				if (result.successful) return result;
				std::this_thread::sleep_for(delay);
				delay *= config_.backoffMultiplier;
			}
			return attempt(true);
		}

		case StrategyType::CIRCUIT_BREAKER:
			if (strategy.circuitOpen.load(std::memory_order_acquire)) {
				return RecoveryResult{
					.successful = false,
					.message = "Circuit breaker open"
				};
			}
			return attempt(false);

		case StrategyType::FALLBACK:
			return attempt(true);

		case StrategyType::COMPENSATION:
			return attempt(false);

		default:
			return RecoveryResult{
				.successful = false,
				.message = "Unknown strategy type"
			};
	}
}

bool RecoveryStrategy::shouldAttemptRecovery(const Strategy& strategy) const {
	if (strategy.circuitOpen.load(std::memory_order_acquire)) {
		auto now = std::chrono::system_clock::now();
		auto lastAttemptTime = std::chrono::system_clock::time_point(
			std::chrono::system_clock::duration(
				strategy.lastAttempt.load(std::memory_order_acquire)));
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttemptTime);
		if (elapsed > config_.timeout) {
			return true;
		}
		return false;
	}
	return true;
}

void RecoveryStrategy::updateStrategyState(Strategy& strategy, bool success) {
	strategy.lastAttempt.store(
		std::chrono::system_clock::now().time_since_epoch().count(),
		std::memory_order_release);
	
	if (success) {
		strategy.failureCount.store(0, std::memory_order_release);
		strategy.circuitOpen.store(false, std::memory_order_release);
	} else {
		auto currentFailures = strategy.failureCount.fetch_add(1, std::memory_order_acq_rel) + 1;
		if (currentFailures >= config_.circuitBreakerThreshold) {
			strategy.circuitOpen.store(true, std::memory_order_release);
		}
	}
}

} // namespace MusicTrainer


