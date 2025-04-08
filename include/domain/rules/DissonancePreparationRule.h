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
    // Member variable to store violation details
    std::string m_violationDescription;

    // Helper function defined in the .cpp file to check dissonance at a specific index
    bool checkDissonancePreparation(const Voice& voice1, const Voice& voice2, std::size_t noteIndex);

    // Removed unused private helper declarations (functionality provided by music::Interval)
    // bool isDissonant(int interval) const;
    // bool isConsonant(int interval) const;
    // bool isStepwiseMotion(int interval) const;
    // std::string getIntervalName(int interval) const;
};