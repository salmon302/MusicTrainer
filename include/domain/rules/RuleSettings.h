#pragma once

#include <unordered_map>
#include <string>
#include <memory>

namespace MusicTrainer::music::rules {

class RuleSettings {
public:
    static RuleSettings& instance();
    
    // Enable/disable individual rules
    void setRuleEnabled(const ::std::string& ruleName, bool enabled);
    bool isRuleEnabled(const ::std::string& ruleName) const;
    
    // Reset all settings to defaults
    void resetToDefaults();

private:
    RuleSettings() = default;
    RuleSettings(const RuleSettings&) = delete;
    RuleSettings& operator=(const RuleSettings&) = delete;
    
    ::std::unordered_map<::std::string, bool> m_ruleStates;
};

} // namespace MusicTrainer::music::rules