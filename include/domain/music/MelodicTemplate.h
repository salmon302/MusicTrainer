#ifndef MUSICTRAINERV3_MELODICTEMPLATE_H
#define MUSICTRAINERV3_MELODICTEMPLATE_H

#include "Pitch.h"
#include "Duration.h"
#include "Interval.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <functional>

namespace music {

enum class PatternCategory {
	OPENING,
	MIDDLE,
	CADENCE,
	GENERAL
};

enum class HarmonicContext {
	TONIC,
	SUBDOMINANT,
	DOMINANT,
	SECONDARY_DOMINANT
};

class MelodicTemplate {
public:
	static std::unique_ptr<MelodicTemplate> create();
	
	// Pattern management with enhanced context
	void addPattern(const std::vector<Pitch>& pitches, 
				   const std::vector<Duration>& durations,
				   double weight = 1.0,
				   PatternCategory category = PatternCategory::GENERAL,
				   music::HarmonicContext context = music::HarmonicContext::TONIC);
	
	// Enhanced melody generation with harmonic context
	std::vector<std::pair<Pitch, Duration>> generateMelody(
		size_t measureCount,
		const std::vector<music::HarmonicContext>& harmonicProgression = {}) const;
	
	// Existing range and basic constraint methods
	void setPitchRange(Pitch minPitch, Pitch maxPitch);
	void setMaximumLeap(int semitones);
	void setStepwiseMotionProbability(double probability);
	void setPatternCategoryProbability(PatternCategory category, double probability);
	
	// New harmonic constraint methods
	void addPreferredInterval(const Interval& interval, double weight = 1.0);
	void setHarmonicContextProbability(music::HarmonicContext context, double probability);
	void addVoiceLeadingRule(std::function<bool(const Pitch&, const Pitch&)> rule);
	
	// Pattern transformation methods
	void enablePatternTransformation(bool enable);
	void setTransformationProbability(double probability);
	
	// Getters
	const Pitch& getMinPitch() const { return minPitch; }
	const Pitch& getMaxPitch() const { return maxPitch; }
	bool isTransformationEnabled() const { return enableTransformation; }

private:
	MelodicTemplate();
	
	struct Pattern {
		std::vector<Pitch> pitches;
		std::vector<Duration> durations;
		double weight;
		PatternCategory category;
		music::HarmonicContext harmonicContext;
	};
	
	bool isValidTransition(const Pitch& from, const Pitch& to) const;
	Pattern selectPattern(PatternCategory preferredCategory, music::HarmonicContext context) const;
	Pattern transformPattern(const Pattern& original) const;
	bool validatePattern(const Pattern& pattern) const;
	
	std::vector<Pattern> patterns;
	std::vector<std::pair<Interval, double>> preferredIntervals;
	std::vector<std::function<bool(const Pitch&, const Pitch&)>> voiceLeadingRules;
	
	Pitch minPitch;
	Pitch maxPitch;
	
	// Melodic constraints
	int maximumLeap{12};
	double stepwiseMotionProb{0.7};
	std::map<PatternCategory, double> categoryProbs;
	std::map<music::HarmonicContext, double> harmonicContextProbs;
	
	// Pattern transformation
	bool enableTransformation{false};
	double transformProb{0.3};
};

} // namespace music

#endif // MUSICTRAINERV3_MELODICTEMPLATE_H