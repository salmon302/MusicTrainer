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

void RecoveryStrategy::setConfig(const StrategyConfig& config) {
	::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
	config_ = config;
}

const RecoveryStrategy::StrategyConfig& RecoveryStrategy::getConfig() const {
	::utils::TrackedSharedMutexLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
	return config_;
}

void RecoveryStrategy::registerStrategy(const std::string& errorType,
									  StrategyType type,
									  RecoveryAction action,
									  FallbackAction fallback) {
	::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
	Strategy strategy{
		type,
		std::move(action),
		std::move(fallback),
		0,
		std::chrono::system_clock::now(),
		false
	};
	strategies_[errorType] = std::move(strategy);
}

RecoveryStrategy::RecoveryResult RecoveryStrategy::attemptRecovery(const MusicTrainerError& error) {
	// Get strategy with write lock since we'll be modifying its state
	::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
	auto it = strategies_.find(error.getType());
	if (it == strategies_.end()) {
		return {false, "No recovery strategy registered", std::chrono::microseconds(0)};
	}
	
	// Execute strategy with direct reference to map entry
	auto& strategy = it->second;
	auto result = executeStrategy(strategy, error);
	
	// Update strategy state based on result
	updateStrategyState(strategy, result.successful);
	
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

	// Handle validation errors immediately without retries
	if (error.getType() == "ValidationError") {
		return attempt(false);
	}

	switch (strategy.type) {
		case StrategyType::RETRY: {
			// Store error type and config before releasing lock
			auto errorType = error.getType();
			auto maxAttempts = config_.maxAttempts;
			
			RecoveryResult result;
			for (size_t i = 0; i < maxAttempts; ++i) {
				// Release lock before logging and attempting recovery
				{
					::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
					ErrorLogger::getInstance().logRecoveryAttempt(
						errorType, 
						"Attempt " + std::to_string(i + 1) + " of " + std::to_string(maxAttempts),
						"", ErrorLogger::LogLevel::INFO);
				}
				
				result = attempt(false);
				
				// Reacquire lock for logging result
				{
					::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
					if (result.successful) {
						ErrorLogger::getInstance().logRecoveryResult(
							errorType, true,
							"Retry successful on attempt " + std::to_string(i + 1),
							ErrorLogger::LogLevel::INFO);
						return result;
					}
				}
				
				if (i < maxAttempts - 1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			
			// Log failure and try fallback
			{
				::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
				ErrorLogger::getInstance().logRecoveryResult(
					errorType, false, 
					"All retry attempts failed, trying fallback",
					ErrorLogger::LogLevel::WARNING);
			}
			
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
			return attempt(true);  // Try fallback
		}

		case StrategyType::CIRCUIT_BREAKER:
			if (strategy.circuitOpen) {
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
	if (strategy.circuitOpen) {
		auto now = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			now - strategy.lastAttempt);
		if (elapsed > config_.timeout) {
			return true;  // Allow a single test request
		}
		return false;
	}
	return true;
}

void RecoveryStrategy::updateStrategyState(Strategy& strategy, bool success) {
	strategy.lastAttempt = std::chrono::system_clock::now();
	
	if (success) {
		strategy.failureCount = 0;
		strategy.circuitOpen = false;
	} else {
		strategy.failureCount++;
		if (strategy.failureCount >= config_.circuitBreakerThreshold) {
			strategy.circuitOpen = true;
		}
	}
}

void RecoveryStrategy::clearStrategies() {
	::utils::TrackedUniqueLock lock(mutex_, "RecoveryStrategy::mutex_", ::utils::LockLevel::RECOVERY);
	strategies_.clear();
}

} // namespace MusicTrainer