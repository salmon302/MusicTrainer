#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/IncrementalRule.h"
#include <memory>

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

::std::unique_ptr<ParallelFifthsRule> ParallelFifthsRule::create() {
    return ::std::make_unique<ParallelFifthsRule>();
}

Rule* ParallelFifthsRule::clone() const {
    return new ParallelFifthsRule(m_violationDescription);
}

::std::string ParallelFifthsRule::getName() const {
    return "Parallel Fifths Rule";
}

::std::string ParallelFifthsRule::getViolationDescription() const {
    return m_violationDescription;
}

} // namespace MusicTrainer::music::rules