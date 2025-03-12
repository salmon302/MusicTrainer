#pragma once

#include "Rule.h"

namespace MusicTrainer::music::rules {

/**
 * @brief Rule that checks for parallel perfect fifths and octaves
 */
class ParallelMotionRule : public Rule {
public:
    ParallelMotionRule* clone() const override;
    bool evaluate(const music::Score& score) override;
    std::string getViolationDescription() const override;
    std::string getName() const override;

private:
    std::string m_violationDescription;
};