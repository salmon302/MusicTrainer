#ifndef MUSICTRAINERV3_INCREMENTALRULE_H
#define MUSICTRAINERV3_INCREMENTALRULE_H

#include "Rule.h"

namespace music::rules {

class IncrementalRule : public Rule {
public:
	virtual ~IncrementalRule() = default;
	
	// Evaluate rule for a specific measure range
	virtual bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const = 0;
};

} // namespace music::rules

#endif // MUSICTRAINERV3_INCREMENTALRULE_H