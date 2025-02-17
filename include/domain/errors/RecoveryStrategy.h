#pragma once
#include "ErrorBase.h"
#include <functional>
#include <vector>
#include <memory>
#include <shared_mutex>
#include "../../utils/TrackedLock.h"
#include <chrono>
#include <unordered_map>
#include <cmath>

namespace MusicTrainer {

// Recovery strategy class with lock level 10 (RECOVERY)
// Must be acquired after ERROR_HANDLER (level 9)
class RecoveryStrategy {
public:
	// Recovery result with context
	struct RecoveryResult {
		bool successful{false};
		std::string message;
		std::chrono::microseconds duration{0};
	};

	// Strategy types
	enum class StrategyType {
		RETRY,              // Simple retry
		EXPONENTIAL_BACKOFF,// Retry with increasing delays
		CIRCUIT_BREAKER,    // Prevent cascading failures
		FALLBACK,          // Use alternative approach
		COMPENSATION       // Compensating action
	};

	// Strategy configuration
	struct StrategyConfig {
		size_t maxAttempts{3};
		std::chrono::milliseconds timeout{1000};
		std::chrono::milliseconds backoffInitial{100};
		double backoffMultiplier{2.0};
		size_t circuitBreakerThreshold{5};
	};

	using RecoveryAction = std::function<bool(const MusicTrainerError&)>;
	using FallbackAction = std::function<bool(const MusicTrainerError&)>;

	static RecoveryStrategy& getInstance();
	void setConfig(const StrategyConfig& config);
	const StrategyConfig& getConfig() const;
	void registerStrategy(const std::string& errorType, 
						 StrategyType type,
						 RecoveryAction action,
						 FallbackAction fallback = nullptr);
	RecoveryResult attemptRecovery(const MusicTrainerError& error);

	void clearStrategies();

	// Mutex access for ErrorHandler
	std::shared_mutex& getMutex() { return mutex_; }

private:
	RecoveryStrategy() = default;

	struct Strategy {
		StrategyType type;
		RecoveryAction action;
		FallbackAction fallback;
		size_t failureCount{0};
		std::chrono::system_clock::time_point lastAttempt;
		bool circuitOpen{false};
	};

	RecoveryResult executeStrategy(const Strategy& strategy, const MusicTrainerError& error);
	bool shouldAttemptRecovery(const Strategy& strategy) const;
	void updateStrategyState(Strategy& strategy, bool success);

	mutable std::shared_mutex mutex_; // Level 10 (RECOVERY) - Must be acquired after ERROR_HANDLER
	std::unordered_map<std::string, Strategy> strategies_;
	StrategyConfig config_;
};

} // namespace MusicTrainer

