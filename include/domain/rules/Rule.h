#pragma once

#include "domain/music/Score.h"
#include "RuleSettings.h"
#include <string>

namespace MusicTrainer::music::rules {

class Rule {
public:
    virtual ~Rule() = default;
    
    virtual Rule* clone() const = 0;
    virtual bool evaluate(const music::Score& score) = 0;
    virtual std::string getViolationDescription() const = 0;
    virtual std::string getName() const = 0;
    
    // Enable/disable the rule
    void setEnabled(bool enabled) { 
        RuleSettings::instance().setRuleEnabled(getName(), enabled); 
    }
    
    bool isEnabled() const { 
        return RuleSettings::instance().isRuleEnabled(getName()); 
    }

protected:
    Rule() = default;
    Rule(const Rule&) = default;
    Rule& operator=(const Rule&) = default;
};

} // namespace MusicTrainer::music::rules
