#ifndef MUSICTRAINERV3_PARALLELOCTAVESRULE_H
#define MUSICTRAINERV3_PARALLELOCTAVESRULE_H

#include "IncrementalRule.h"
#include <string>
#include <memory>

namespace MusicTrainer {
namespace music {
class Score;

namespace rules {

class ParallelOctavesRule : public IncrementalRule {
public:
    static std::unique_ptr<ParallelOctavesRule> create();
    
    bool evaluate(const Score& score) override;
    bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const override;
    std::string getViolationDescription() const override;
    std::string getName() const override;
    Rule* clone() const override;
    
    // Destructor
    ~ParallelOctavesRule() override;
    
private:
    // Default constructor now explicitly defined in the cpp file
    ParallelOctavesRule();
    
    // Constructor that takes an initial violation description (used by clone)
    explicit ParallelOctavesRule(const std::string& initialViolation);
    
    void setViolationDescription(const std::string& desc) const {
        violationDescription = desc;
    }
    
    void clearViolationDescription() const {
        violationDescription.clear();
    }
    
    mutable std::string violationDescription;
};

} // namespace rules
} // namespace music
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_PARALLELOCTAVESRULE_H


