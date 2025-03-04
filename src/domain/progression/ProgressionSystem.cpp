#include "domain/progression/ProgressionSystem.h"
#include <iostream>
#include <numeric>
#include <algorithm>

namespace music::progression {

ProgressionSystem::ProgressionSystem() 
	: version(0)
	, currentDifficulty(1.0)
	, exerciseAttempts(0) {}

std::unique_ptr<ProgressionSystem> ProgressionSystem::create() {
	return std::unique_ptr<ProgressionSystem>(new ProgressionSystem());
}

void ProgressionSystem::recordExerciseAttempt(const MusicTrainer::music::Score& exercise, const MusicTrainer::music::Score& attempt, double timeSpent) {
	// Calculate accuracy
	double accuracy = calculateAccuracy(exercise, attempt);
	
	// Calculate average completion time
	double avgCompletionTime = timeSpent;
	if (!completionTimes.empty()) {
		avgCompletionTime = std::accumulate(completionTimes.begin(), completionTimes.end(), 0.0) / completionTimes.size();
	}
	
	// Calculate new metrics
	const double alpha = 0.5;
	SkillMetrics newSkills;
	
	newSkills.pitchAccuracy.store((1 - alpha) * currentSkills.pitchAccuracy.load(std::memory_order_acquire) + 
								 alpha * accuracy, std::memory_order_release);
	
	newSkills.rhythmAccuracy.store((1 - alpha) * currentSkills.rhythmAccuracy.load(std::memory_order_acquire) + 
								  alpha * accuracy, std::memory_order_release);
	
	newSkills.speedConsistency.store((1 - alpha) * currentSkills.speedConsistency.load(std::memory_order_acquire) + 
									alpha * (1.0 / (1.0 + std::abs(timeSpent - avgCompletionTime))), 
									std::memory_order_release);
	
	newSkills.overallProgress.store((0.45 * newSkills.pitchAccuracy.load(std::memory_order_acquire) + 
								   0.45 * newSkills.rhythmAccuracy.load(std::memory_order_acquire) + 
								   0.1 * newSkills.speedConsistency.load(std::memory_order_acquire)),
								   std::memory_order_release);
	
	// Update state atomically
	currentSkills = newSkills;
	exerciseAttempts.fetch_add(1, std::memory_order_acq_rel);
	accuracyHistory.push_back(accuracy);
	completionTimes.push_back(timeSpent);
	
	// Update difficulty
	double newDifficulty = calculateNextDifficultyLevel();
	currentDifficulty.store(newDifficulty, std::memory_order_release);
	
	incrementVersion();

	
	std::cout << "[Progression] Exercise attempt recorded - Accuracy: " << accuracy 
			  << ", Time: " << timeSpent << "s" << std::endl;
}

SkillMetrics ProgressionSystem::getCurrentSkillLevel() const {
	return currentSkills;
}


double ProgressionSystem::calculateNextDifficultyLevel() const {
	if (accuracyHistory.empty()) {
		return currentDifficulty.load(std::memory_order_acquire);
	}
	
	size_t samples = std::min(accuracyHistory.size(), size_t(5));
	auto start = accuracyHistory.end() - samples;
	double avgAccuracy = std::accumulate(start, accuracyHistory.end(), 0.0) / samples;
	
	return avgAccuracy >= 0.75 ? 
		   currentDifficulty.load(std::memory_order_acquire) + 0.25 : 
		   currentDifficulty.load(std::memory_order_acquire);
}



bool ProgressionSystem::shouldIncreaseComplexity() const {
	size_t attempts = exerciseAttempts.load(std::memory_order_acquire);
	if (accuracyHistory.empty() || attempts < 5) {
		return false;
	}
	
	// Calculate recent accuracy
	size_t samples = std::min(accuracyHistory.size(), size_t(5));
	auto start = accuracyHistory.end() - samples;
	double recentAccuracy = std::accumulate(start, accuracyHistory.end(), 0.0) / samples;
	
	return currentSkills.overallProgress.load(std::memory_order_acquire) > 0.7 && 
		   recentAccuracy >= 0.75;
}


ProgressionSystem::ExerciseAnalytics ProgressionSystem::getAnalytics() const {
	ExerciseAnalytics analytics;
	analytics.totalAttempts.store(exerciseAttempts.load(std::memory_order_acquire));
	
	if (!accuracyHistory.empty()) {
		analytics.averageAccuracy.store(
			std::accumulate(accuracyHistory.begin(), accuracyHistory.end(), 0.0) / accuracyHistory.size()
		);
	}
	
	if (!completionTimes.empty()) {
		analytics.averageCompletionTime.store(
			std::accumulate(completionTimes.begin(), completionTimes.end(), 0.0) / completionTimes.size()
		);
	}
	
	analytics.progressTrend = accuracyHistory;
	return analytics;
}


double ProgressionSystem::calculateAccuracy(const MusicTrainer::music::Score& exercise, const MusicTrainer::music::Score& attempt) const {
	// Compare notes, rhythm, and timing
	// TODO: Implement detailed comparison
	return 0.75; // Placeholder
}

} // namespace music::progression
