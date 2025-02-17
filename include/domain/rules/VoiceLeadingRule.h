#ifndef MUSICTRAINERV3_VOICELEADINGRULE_H
#define MUSICTRAINERV3_VOICELEADINGRULE_H

#include "IncrementalRule.h"
#include "../music/Score.h"

namespace music::rules {

class VoiceLeadingRule : public IncrementalRule {
public:
	static std::unique_ptr<VoiceLeadingRule> create(int maxLeapSize = 9);
	
	bool evaluate(const Score& score) const override;
	bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const override;
	std::string getViolationDescription() const override;
	std::string getName() const override;
	std::unique_ptr<Rule> clone() const override;

private:
	explicit VoiceLeadingRule(int maxLeapSize);
	mutable std::string violationDescription;
	int maxLeapSize; // Maximum allowed leap size in semitones
};

} // namespace music::rules

#endif // MUSICTRAINERV3_VOICELEADINGRULE_H

