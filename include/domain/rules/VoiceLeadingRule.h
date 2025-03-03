#ifndef MUSICTRAINERV3_VOICELEADINGRULE_H
#define MUSICTRAINERV3_VOICELEADINGRULE_H

#include "IncrementalRule.h"
#include <string>
#include <memory>

namespace MusicTrainer {
namespace music {
class Score;

namespace rules {

class VoiceLeadingRule : public IncrementalRule {
public:
    static std::unique_ptr<VoiceLeadingRule> create();
    
    bool evaluate(const Score& score) override;
    bool evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const override;
    std::string getViolationDescription() const override;
    std::string getName() const override;
    std::unique_ptr<Rule> clone() const override;
    
private:
    // Default constructor
    VoiceLeadingRule();
    
    // Constructor that takes an initial violation description (used by clone)
    explicit VoiceLeadingRule(const std::string& initialViolation);
    
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

#endif // MUSICTRAINERV3_VOICELEADINGRULE_H


