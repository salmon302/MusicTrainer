#include "domain/progression/ProgressionSystem.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <numeric>
#include <algorithm>

namespace music::progression {

ProgressionSystem::ProgressionSystem() 
	: currentDifficulty(1.0), exerciseAttempts(0) {}

std::unique_ptr<ProgressionSystem> ProgressionSystem::create() {
	return std::unique_ptr<ProgressionSystem>(new ProgressionSystem());
}

void ProgressionSystem::recordExerciseAttempt(const Score& exercise, const Score& attempt, double timeSpent) {
	// Calculate accuracy outside of lock
	double accuracy = calculateAccuracy(exercise, attempt);
	
	// Calculate skill metrics outside of lock
	double avgCompletionTime = 0.0;
	std::vector<double> times;
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		times = completionTimes;
	}
	
	if (!times.empty()) {
		avgCompletionTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
	} else {
		avgCompletionTime = timeSpent;
	}
	
	// Calculate new metrics
	const double alpha = 0.5;
	SkillMetrics newSkills;
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		newSkills = currentSkills;
	}
	
	newSkills.pitchAccuracy = (1 - alpha) * newSkills.pitchAccuracy + alpha * accuracy;
	newSkills.rhythmAccuracy = (1 - alpha) * newSkills.rhythmAccuracy + alpha * accuracy;
	newSkills.speedConsistency = (1 - alpha) * newSkills.speedConsistency + 
								alpha * (1.0 / (1.0 + std::abs(timeSpent - avgCompletionTime)));
	newSkills.overallProgress = (0.45 * newSkills.pitchAccuracy + 
								0.45 * newSkills.rhythmAccuracy + 
								0.1 * newSkills.speedConsistency);
	
	// Calculate new difficulty level outside of lock
	double newDifficulty = calculateNextDifficultyLevel();
	
	// Update all state under a single lock
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		currentSkills = newSkills;
		exerciseAttempts++;
		accuracyHistory.push_back(accuracy);
		completionTimes.push_back(timeSpent);
		currentDifficulty = newDifficulty;
	}
	
	std::cout << "[Progression] Exercise attempt recorded - Accuracy: " << accuracy 
			  << ", Time: " << timeSpent << "s" << std::endl;
}

SkillMetrics ProgressionSystem::getCurrentSkillLevel() const {
	::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
	return currentSkills;
}

double ProgressionSystem::calculateNextDifficultyLevel() const {
	// Get all required data under a single lock
	std::vector<double> history;
	double currentDiff;
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		if (accuracyHistory.empty()) return currentDifficulty;
		history = accuracyHistory;
		currentDiff = currentDifficulty;
	}
	
	size_t samples = std::min(history.size(), size_t(5));
	auto start = history.end() - samples;
	double avgAccuracy = std::accumulate(start, history.end(), 0.0) / samples;
	
	return avgAccuracy >= 0.75 ? currentDiff + 0.25 : currentDiff;
}


bool ProgressionSystem::shouldIncreaseComplexity() const {
	// Get all required data under a single lock
	std::vector<double> history;
	size_t attempts;
	double progress;
	bool hasEnoughData;
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		hasEnoughData = !accuracyHistory.empty() && exerciseAttempts >= 5;
		if (!hasEnoughData) return false;
		
		history = accuracyHistory;
		attempts = exerciseAttempts;
		progress = currentSkills.overallProgress;
	}
	
	// Calculate recent accuracy without holding lock
	size_t samples = std::min(history.size(), size_t(5));
	auto start = history.end() - samples;
	double recentAccuracy = std::accumulate(start, history.end(), 0.0) / samples;
	
	return progress > 0.7 && recentAccuracy >= 0.75;
}

ProgressionSystem::ExerciseAnalytics ProgressionSystem::getAnalytics() const {
	// Get required data under lock
	std::vector<double> history;
	std::vector<double> times;
	size_t attempts;
	{
		::utils::TrackedUniqueLock lock(progressMutex, "ProgressionSystem::progressMutex", ::utils::LockLevel::PROGRESSION);
		history = accuracyHistory;
		times = completionTimes;
		attempts = exerciseAttempts;
	}
	
	ExerciseAnalytics analytics;
	analytics.totalAttempts = attempts;
	
	if (!history.empty()) {
		analytics.averageAccuracy = std::accumulate(history.begin(), 
												  history.end(), 0.0) / history.size();
	}
	
	if (!times.empty()) {
		analytics.averageCompletionTime = std::accumulate(times.begin(), 
														times.end(), 0.0) / times.size();
	}
	
	analytics.progressTrend = history;
	return analytics;
}

double ProgressionSystem::calculateAccuracy(const Score& exercise, const Score& attempt) const {
	// Compare notes, rhythm, and timing
	// TODO: Implement detailed comparison
	return 0.75; // Placeholder
}

} // namespace music::progression
