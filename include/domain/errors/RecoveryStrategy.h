#pragma once
#include "ErrorBase.h"
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <cmath>

namespace MusicTrainer {

class RecoveryStrategy {
public:
	struct RecoveryResult {
		bool successful{false};
		std::string message;
		std::chrono::microseconds duration{0};
	};

	enum class StrategyType {
		RETRY,
		EXPONENTIAL_BACKOFF,
		CIRCUIT_BREAKER,
		FALLBACK,
		COMPENSATION
	};

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
	void setConfig(const StrategyConfig& config) {
		config_ = config;
	}
	const StrategyConfig& getConfig() const {
		return config_;
	}
	void registerStrategy(const std::string& errorType, 
						 StrategyType type,
						 RecoveryAction action,
						 FallbackAction fallback = nullptr);
	RecoveryResult attemptRecovery(const MusicTrainerError& error);
	void clearStrategies() {
		strategies.clear();
	}

private:
	RecoveryStrategy() = default;

	struct Strategy {
		StrategyType type;
		RecoveryAction action;
		FallbackAction fallback;
		std::atomic<size_t> failureCount{0};
		std::atomic<std::chrono::system_clock::time_point::rep> lastAttempt{
			std::chrono::system_clock::now().time_since_epoch().count()
		};
		std::atomic<bool> circuitOpen{false};
	};

	RecoveryResult executeStrategy(const Strategy& strategy, const MusicTrainerError& error);
	bool shouldAttemptRecovery(const Strategy& strategy) const;
	void updateStrategyState(Strategy& strategy, bool success);

	StrategyConfig config_;
	std::unordered_map<std::string, std::unique_ptr<Strategy>> strategies;
};

} // namespace MusicTrainer


