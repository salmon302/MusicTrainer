#ifndef MUSICTRAINERV3_PROGRESSIONSYSTEM_H
#define MUSICTRAINERV3_PROGRESSIONSYSTEM_H

#include <memory>
#include <vector>
#include <map>
#include <shared_mutex>
#include "../music/Score.h"
#include "../events/Event.h"

namespace music::progression {

struct SkillMetrics {
	double rhythmAccuracy{0.0};
	double pitchAccuracy{0.0};
	double speedConsistency{0.0};
	double overallProgress{0.0};
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
		size_t totalAttempts{0};
		double averageAccuracy{0.0};
		double averageCompletionTime{0.0};
		std::vector<double> progressTrend;
	};
	
	ExerciseAnalytics getAnalytics() const;

private:
	ProgressionSystem(); // Remove =default
	
	mutable std::shared_mutex progressMutex;  // Mutex for thread safety
	
	// Internal tracking
	std::vector<double> accuracyHistory;
	std::vector<double> completionTimes;
	size_t exerciseAttempts;
	double currentDifficulty;
	
	// Skill assessment
	double calculateAccuracy(const Score& exercise, const Score& attempt) const;
	
	// Current skill levels
	SkillMetrics currentSkills;

};

} // namespace music::progression

#endif // MUSICTRAINERV3_PROGRESSIONSYSTEM_H