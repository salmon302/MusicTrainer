#ifndef MUSICTRAINERV3_PARALLELOCTAVESRULE_H
#define MUSICTRAINERV3_PARALLELOCTAVESRULE_H

#include "IncrementalRule.h"
#include "../music/Score.h"
#include "../../utils/TrackedLock.h"
#include <shared_mutex>
#include <string>

namespace music::rules {

class ParallelOctavesRule : public IncrementalRule {
public:
	static std::unique_ptr<ParallelOctavesRule> create();
	
	bool evaluate(const Score& score) const override;
	bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const override;
	std::string getViolationDescription() const override;
	std::string getName() const override;
	std::unique_ptr<Rule> clone() const override;

private:
	ParallelOctavesRule() = default;
	mutable std::shared_mutex mutex_;
	mutable std::string violationDescription;
	
	void setViolationDescription(const std::string& desc) const;
	void clearViolationDescription() const;
};

} // namespace music::rules

#endif // MUSICTRAINERV3_PARALLELOCTAVESRULE_H

