#include "domain/rules/RuleSettings.h"

namespace MusicTrainer::music::rules {

RuleSettings& RuleSettings::instance() {
    static RuleSettings instance;
    return instance;
}

void RuleSettings::setRuleEnabled(const std::string& ruleName, bool enabled) {
    m_ruleStates[ruleName] = enabled;
}

bool RuleSettings::isRuleEnabled(const std::string& ruleName) const {
    auto it = m_ruleStates.find(ruleName);
    // Rules are enabled by default if not explicitly disabled
    return it == m_ruleStates.end() ? true : it->second;
}

void RuleSettings::resetToDefaults() {
    m_ruleStates.clear();
    
    // Set default states for all known rules
    m_ruleStates["ParallelFifthsRule"] = true;
    m_ruleStates["ParallelOctavesRule"] = true;
    m_ruleStates["VoiceLeadingRule"] = true;
    m_ruleStates["MelodicIntervalRule"] = true;
    m_ruleStates["DissonancePreparationRule"] = true;
}