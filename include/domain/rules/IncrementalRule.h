#ifndef MUSICTRAINERV3_INCREMENTALRULE_H
#define MUSICTRAINERV3_INCREMENTALRULE_H

#include "Rule.h"
#include <cstddef>

namespace MusicTrainer::music::rules {

class IncrementalRule : public Rule {
public:
    ~IncrementalRule() override = default;
    // Evaluate rule for a specific measure range
    virtual bool evaluateIncremental(const MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const = 0;
};

} // namespace MusicTrainer::music::rules

#endif // MUSICTRAINERV3_INCREMENTALRULE_H