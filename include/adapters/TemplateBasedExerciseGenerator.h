#ifndef MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H
#define MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H

#include "domain/ports/ExercisePort.h"
#include "domain/music/MelodicTemplate.h"
#include "domain/music/Score.h"
#include "domain/music/Interval.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace music::adapters {

enum class VoiceType {
	Soprano,
	Alto,
	Tenor,
	Bass
};

struct VoiceConstraints {
	int minPitch{60};  // Default to middle C
	int maxPitch{72};  // Default to C5
	double stepwiseMotionProbability{0.7};
	int maxLeap{12};  // Default to octave
	std::vector<Interval> preferredIntervals{};
	std::map<music::HarmonicContext, double> harmonicWeights{};

	VoiceConstraints() = default;
	VoiceConstraints(int min, int max, double prob, int leap, 
					std::vector<Interval> intervals,
					std::map<music::HarmonicContext, double> weights = {})
		: minPitch(min), maxPitch(max), stepwiseMotionProbability(prob), 
		  maxLeap(leap), preferredIntervals(std::move(intervals)),
		  harmonicWeights(std::move(weights)) {}
};


class TemplateBasedExerciseGenerator : public ports::ExercisePort {
public:
	static std::unique_ptr<TemplateBasedExerciseGenerator> create();
	
	// ExercisePort interface implementation
	std::unique_ptr<Score> generateExercise(const ports::ExerciseParameters& params) override;
	bool validateExercise(const Score& score, const std::vector<std::unique_ptr<rules::Rule>>& rules) override;
	std::vector<std::string> getValidationErrors() const override;

private:
	TemplateBasedExerciseGenerator() = default;
	std::vector<std::string> validationErrors;
	
	// Template management
	std::unique_ptr<MelodicTemplate> createVoiceTemplate(VoiceType type);
	const MelodicTemplate* getTemplateForVoice(size_t voiceIndex, size_t totalVoices);
	
	// Voice templates
	std::unique_ptr<MelodicTemplate> sopranoTemplate;
	std::unique_ptr<MelodicTemplate> altoTemplate;
	std::unique_ptr<MelodicTemplate> tenorTemplate;
	std::unique_ptr<MelodicTemplate> bassTemplate;

    // Constraint management
    std::map<VoiceType, VoiceConstraints> voiceConstraints;
    void initializeVoiceConstraints();
    void applyConstraintsToTemplate(MelodicTemplate& tmpl, const VoiceConstraints& constraints);

// Pattern management
struct PatternBank {
	struct PatternEntry {
		std::vector<Pitch> pitches;
		std::vector<Duration> durations;
		music::HarmonicContext context;

		PatternEntry() = default;
		PatternEntry(std::vector<Pitch> p, std::vector<Duration> d, music::HarmonicContext c = music::HarmonicContext::TONIC)
			: pitches(std::move(p)), durations(std::move(d)), context(c) {}
	};
	std::vector<PatternEntry> openingPatterns;
	std::vector<PatternEntry> middlePatterns;
	std::vector<PatternEntry> cadencePatterns;
};
    std::map<VoiceType, PatternBank> patternBanks;
    void initializePatternBanks();
    void applyPatternsToTemplate(MelodicTemplate& tmpl, const PatternBank& patterns);
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H