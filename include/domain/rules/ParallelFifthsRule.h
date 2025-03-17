#ifndef MUSICTRAINER_PARALLELFIFTHSRULE_H
#define MUSICTRAINER_PARALLELFIFTHSRULE_H

#include "domain/rules/Rule.h"
#include "domain/rules/IncrementalRule.h"
#include "domain/music/Score.h"
#include <string>
#include <memory>

using Score = MusicTrainer::music::Score;

namespace MusicTrainer::music::rules {

/**
 * @brief Rule that checks for parallel fifths between voices
 */
class ParallelFifthsRule final : public IncrementalRule {
public:
    static ::std::unique_ptr<ParallelFifthsRule> create();
    
    ~ParallelFifthsRule() override;

    ::std::unique_ptr<Rule> clone() const override;
    bool evaluate(const Score& score) override;
    ::std::string getViolationDescription() const override;
    ::std::string getName() const override;
    bool evaluateIncremental(const Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const override;

protected:
    ParallelFifthsRule();
    explicit ParallelFifthsRule(const ::std::string& initialViolation);
    
    void clearViolationDescription() const { m_violationDescription.clear(); }
    void setViolationDescription(const ::std::string& desc) const { m_violationDescription = desc; }

private:
    mutable ::std::string m_violationDescription;
};

} // namespace MusicTrainer::music::rules

#endif // MUSICTRAINER_PARALLELFIFTHSRULE_H


