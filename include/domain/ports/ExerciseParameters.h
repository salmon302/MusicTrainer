#ifndef MUSICTRAINERV3_EXERCISEPARAMETERS_H
#define MUSICTRAINERV3_EXERCISEPARAMETERS_H

#include <vector>
#include <memory>
#include "../rules/Rule.h"

namespace music::ports {

struct ExerciseParameters {
	size_t voiceCount{4};     // Default to SATB
	size_t measureCount{4};   // Default to 4 measures
	double difficulty{1.0};   // Scale from 0.0 to 2.0
	bool enforceVoiceRanges{true};
	std::vector<std::unique_ptr<MusicTrainer::music::rules::Rule>> rules; // Exercise-specific rules
};

} // namespace music::ports

#endif // MUSICTRAINERV3_EXERCISEPARAMETERS_H