#pragma once

#include "Rule.h"

namespace MusicTrainer::music::rules {

/**
 * @brief Rule that enforces proper preparation and resolution of dissonances
 * 
 * This rule checks:
 * - Dissonances must be prepared by consonances
 * - Passing tones must move by step
 * - Suspensions must resolve downward by step
 */
class DissonancePreparationRule : public Rule {
public:
    DissonancePreparationRule* clone() const override;
    bool evaluate(const music::Score& score) override;
    std::string getViolationDescription() const override;
    std::string getName() const override;

private:
    std::string m_violationDescription;
    bool isDissonant(int interval) const;
    bool isConsonant(int interval) const;
    bool isStepwiseMotion(int interval) const;
    std::string getIntervalName(int interval) const;
};