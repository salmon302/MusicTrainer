#pragma once

#include "Rule.h"

namespace MusicTrainer::music::rules {

/**
 * @brief Rule that enforces valid melodic intervals
 * 
 * This rule checks for:
 * - Stepwise motion is preferred
 * - Disallows diminished intervals
 * - Disallows intervals larger than an octave
 * - Checks for proper resolution of large leaps
 */
class MelodicIntervalRule : public Rule {
public:
    MelodicIntervalRule* clone() const override;
    bool evaluate(const music::Score& score) override;
    std::string getViolationDescription() const override;
    std::string getName() const override;

private:
    std::string m_violationDescription;
    bool checkMelodicInterval(const Voice& voice, size_t noteIndex);
};