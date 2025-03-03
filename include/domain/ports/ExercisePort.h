#ifndef MUSICTRAINERV3_EXERCISEPORT_H
#define MUSICTRAINERV3_EXERCISEPORT_H

#include <memory>
#include <vector>
#include "../music/Score.h"
#include "../rules/Rule.h"
#include "ExerciseParameters.h"

namespace music::ports {

class ExercisePort {
public:
	virtual ~ExercisePort() = default;
	
	// Generate a new exercise based on parameters
	virtual std::unique_ptr<MusicTrainer::music::Score> generateExercise(const ExerciseParameters& params) = 0;
	
	// Validate a score against exercise rules
	virtual bool validateExercise(const MusicTrainer::music::Score& score, const std::vector<std::unique_ptr<MusicTrainer::music::rules::Rule>>& rules) = 0;
	
	// Get feedback on why validation failed
	virtual std::vector<std::string> getValidationErrors() const = 0;
};

} // namespace music::ports

#endif // MUSICTRAINERV3_EXERCISEPORT_H
