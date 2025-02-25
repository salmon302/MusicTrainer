#ifndef MUSICTRAINERV3_PARALLELFIFTHSRULE_H
#define MUSICTRAINERV3_PARALLELFIFTHSRULE_H

#include "IncrementalRule.h"
#include <string>
#include <memory>

namespace music {
class Score;

namespace rules {

class ParallelFifthsRule : public IncrementalRule {
public:
	static std::unique_ptr<ParallelFifthsRule> create();
	
	bool evaluate(const Score& score) const override;
	bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const override;
	std::string getViolationDescription() const override;
	std::string getName() const override;
	std::unique_ptr<Rule> clone() const override;

private:
	ParallelFifthsRule() = default;
	
	void setViolationDescription(const std::string& desc) const {
		violationDescription = desc;
	}
	
	void clearViolationDescription() const {
		violationDescription.clear();
	}
	
	mutable std::string violationDescription;
};

} // namespace rules
} // namespace music

#endif // MUSICTRAINERV3_PARALLELFIFTHSRULE_H


