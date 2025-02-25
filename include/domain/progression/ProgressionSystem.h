#ifndef MUSICTRAINERV3_PROGRESSIONSYSTEM_H
#define MUSICTRAINERV3_PROGRESSIONSYSTEM_H

#include <memory>
#include <vector>
#include <map>
#include <atomic>
#include "../music/Score.h"
#include "../events/Event.h"
#include "adapters/LockFreeEventQueue.h"

namespace music::progression {

struct SkillMetrics {
	std::atomic<double> rhythmAccuracy{0.0};
	std::atomic<double> pitchAccuracy{0.0};
	std::atomic<double> speedConsistency{0.0};
	std::atomic<double> overallProgress{0.0};
	
	SkillMetrics() = default;
	
	SkillMetrics(const SkillMetrics& other) {
		rhythmAccuracy.store(other.rhythmAccuracy.load(std::memory_order_acquire));
		pitchAccuracy.store(other.pitchAccuracy.load(std::memory_order_acquire));
		speedConsistency.store(other.speedConsistency.load(std::memory_order_acquire));
		overallProgress.store(other.overallProgress.load(std::memory_order_acquire));
	}
	
	SkillMetrics& operator=(const SkillMetrics& other) {
		if (this != &other) {
			rhythmAccuracy.store(other.rhythmAccuracy.load(std::memory_order_acquire));
			pitchAccuracy.store(other.pitchAccuracy.load(std::memory_order_acquire));
			speedConsistency.store(other.speedConsistency.load(std::memory_order_acquire));
			overallProgress.store(other.overallProgress.load(std::memory_order_acquire));
		}
		return *this;
	}
	
	SkillMetrics(SkillMetrics&& other) noexcept {
		rhythmAccuracy.store(other.rhythmAccuracy.load(std::memory_order_acquire));
		pitchAccuracy.store(other.pitchAccuracy.load(std::memory_order_acquire));
		speedConsistency.store(other.speedConsistency.load(std::memory_order_acquire));
		overallProgress.store(other.overallProgress.load(std::memory_order_acquire));
	}
	
	SkillMetrics& operator=(SkillMetrics&& other) noexcept {
		if (this != &other) {
			rhythmAccuracy.store(other.rhythmAccuracy.load(std::memory_order_acquire));
			pitchAccuracy.store(other.pitchAccuracy.load(std::memory_order_acquire));
			speedConsistency.store(other.speedConsistency.load(std::memory_order_acquire));
			overallProgress.store(other.overallProgress.load(std::memory_order_acquire));
		}
		return *this;
	}
};

class ProgressionSystem {
public:
	static std::unique_ptr<ProgressionSystem> create();

	// Skill tracking
	void recordExerciseAttempt(const Score& exercise, const Score& attempt, double timeSpent);
	SkillMetrics getCurrentSkillLevel() const;
	
	// Difficulty management
	double calculateNextDifficultyLevel() const;
	bool shouldIncreaseComplexity() const;
	
	// Analytics
	struct ExerciseAnalytics {
		std::atomic<size_t> totalAttempts{0};
		std::atomic<double> averageAccuracy{0.0};
		std::atomic<double> averageCompletionTime{0.0};
		std::vector<double> progressTrend;  // Protected by version
		
		ExerciseAnalytics() = default;
		
		ExerciseAnalytics(const ExerciseAnalytics& other) {
			totalAttempts.store(other.totalAttempts.load(std::memory_order_acquire));
			averageAccuracy.store(other.averageAccuracy.load(std::memory_order_acquire));
			averageCompletionTime.store(other.averageCompletionTime.load(std::memory_order_acquire));
			progressTrend = other.progressTrend;
		}
		
		ExerciseAnalytics& operator=(const ExerciseAnalytics& other) {
			if (this != &other) {
				totalAttempts.store(other.totalAttempts.load(std::memory_order_acquire));
				averageAccuracy.store(other.averageAccuracy.load(std::memory_order_acquire));
				averageCompletionTime.store(other.averageCompletionTime.load(std::memory_order_acquire));
				progressTrend = other.progressTrend;
			}
			return *this;
		}
		
		ExerciseAnalytics(ExerciseAnalytics&& other) noexcept {
			totalAttempts.store(other.totalAttempts.load(std::memory_order_acquire));
			averageAccuracy.store(other.averageAccuracy.load(std::memory_order_acquire));
			averageCompletionTime.store(other.averageCompletionTime.load(std::memory_order_acquire));
			progressTrend = std::move(other.progressTrend);
		}
		
		ExerciseAnalytics& operator=(ExerciseAnalytics&& other) noexcept {
			if (this != &other) {
				totalAttempts.store(other.totalAttempts.load(std::memory_order_acquire));
				averageAccuracy.store(other.averageAccuracy.load(std::memory_order_acquire));
				averageCompletionTime.store(other.averageCompletionTime.load(std::memory_order_acquire));
				progressTrend = std::move(other.progressTrend);
			}
			return *this;
		}
	};
	
	ExerciseAnalytics getAnalytics() const;

private:
	ProgressionSystem();
	
	// Version control for concurrent access
	std::atomic<uint64_t> version{0};
	
	// Lock-free history tracking
	adapters::LockFreeEventQueue<std::unique_ptr<events::Event>, 1024> historyQueue;
	std::vector<double> accuracyHistory;  // Protected by version
	std::vector<double> completionTimes;  // Protected by version
	std::atomic<size_t> exerciseAttempts{0};
	std::atomic<double> currentDifficulty{1.0};
	
	// Skill assessment
	double calculateAccuracy(const Score& exercise, const Score& attempt) const;
	
	// Current skill levels
	SkillMetrics currentSkills;
	
	// Version management
	void incrementVersion() { version.fetch_add(1, std::memory_order_acq_rel); }
	uint64_t getCurrentVersion() const { return version.load(std::memory_order_acquire); }
};

} // namespace music::progression

#endif // MUSICTRAINERV3_PROGRESSIONSYSTEM_H