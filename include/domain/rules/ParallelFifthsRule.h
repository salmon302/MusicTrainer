#ifndef MUSICTRAINERV3_PARALLELFIFTHSRULE_H
#define MUSICTRAINERV3_PARALLELFIFTHSRULE_H

#include "IncrementalRule.h"
#include "../music/Score.h"
#include <mutex>

namespace music::rules {

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
	mutable std::string violationDescription;
};

} // namespace music::rules

#endif // MUSICTRAINERV3_PARALLELFIFTHSRULE_H

