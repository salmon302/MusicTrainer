#ifndef MUSICTRAINERV3_VOICELEADINGRULE_H
#define MUSICTRAINERV3_VOICELEADINGRULE_H

#include "IncrementalRule.h"
#include <string>
#include <memory>

namespace music {
class Score;

namespace rules {

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
	
	void setViolationDescription(const std::string& desc) const {
		violationDescription = desc;
	}
	
	void clearViolationDescription() const {
		violationDescription.clear();
	}
	
	mutable std::string violationDescription;
	const int maxLeapSize; // Maximum allowed leap size in semitones
};

} // namespace rules
} // namespace music

#endif // MUSICTRAINERV3_VOICELEADINGRULE_H


