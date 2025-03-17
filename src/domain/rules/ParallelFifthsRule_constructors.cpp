#include "domain/rules/ParallelFifthsRule.h"

namespace MusicTrainer::music::rules {

ParallelFifthsRule::ParallelFifthsRule() 
    : IncrementalRule()
    , m_violationDescription()
{
}

ParallelFifthsRule::ParallelFifthsRule(const ::std::string& initialViolation)
    : IncrementalRule()
    , m_violationDescription(initialViolation)
{
}

} // namespace MusicTrainer::music::rules