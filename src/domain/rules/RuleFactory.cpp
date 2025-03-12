#include "domain/rules/RuleFactory.h"
#include "domain/rules/ConsonantIntervalRule.h"
#include "domain/rules/ParallelMotionRule.h"
#include "domain/rules/MelodicIntervalRule.h"
#include "domain/rules/DissonancePreparationRule.h"
#include "domain/state/SettingsState.h"
#include <unordered_map>

namespace MusicTrainer::music::rules {

std::vector<std::unique_ptr<Rule>> RuleFactory::createRuleSet(const std::string& exerciseType) {
    if (exerciseType == "First Species") {
        return createFirstSpeciesRules();
    } else if (exerciseType == "Second Species") {
        return createSecondSpeciesRules();
    } else if (exerciseType == "Third Species") {
        return createThirdSpeciesRules();
    } else if (exerciseType == "Fourth Species") {
        return createFourthSpeciesRules();
    } else if (exerciseType == "Fifth Species") {
        return createFifthSpeciesRules();
    }
    return {}; // Return empty set for unknown types
}

std::unique_ptr<Rule> RuleFactory::createRule(const std::string& ruleName) {
    // Create rule based on name and set its enabled state from settings
    std::unique_ptr<Rule> rule;
    auto& settings = state::SettingsState::instance();
    
    if (ruleName == "ConsonantIntervalRule") {
        rule = std::make_unique<ConsonantIntervalRule>();
    } else if (ruleName == "ParallelMotionRule") {
        rule = std::make_unique<ParallelMotionRule>();
    } else if (ruleName == "MelodicIntervalRule") {
        rule = std::make_unique<MelodicIntervalRule>();
    } else if (ruleName == "DissonancePreparationRule") {
        rule = std::make_unique<DissonancePreparationRule>();
    }
    
    if (rule) {
        rule->setEnabled(settings.getRuleEnabled(QString::fromStdString(ruleName)));
    }
    
    return rule;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createFirstSpeciesRules() {
    std::vector<std::unique_ptr<Rule>> rules;
    
    // Add core first species rules
    rules.push_back(createRule("ConsonantIntervalRule"));
    rules.push_back(createRule("ParallelMotionRule"));
    rules.push_back(createRule("MelodicIntervalRule"));
    
    return rules;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createSecondSpeciesRules() {
    std::vector<std::unique_ptr<Rule>> rules;
    
    // Add second species rules
    rules.push_back(createRule("ConsonantIntervalRule"));
    rules.push_back(createRule("ParallelMotionRule"));
    rules.push_back(createRule("MelodicIntervalRule"));
    rules.push_back(createRule("DissonancePreparationRule"));
    
    return rules;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createThirdSpeciesRules() {
    std::vector<std::unique_ptr<Rule>> rules;
    
    // Add third species rules (similar to second species for now)
    rules.push_back(createRule("ConsonantIntervalRule"));
    rules.push_back(createRule("ParallelMotionRule"));
    rules.push_back(createRule("MelodicIntervalRule"));
    rules.push_back(createRule("DissonancePreparationRule"));
    
    return rules;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createFourthSpeciesRules() {
    std::vector<std::unique_ptr<Rule>> rules;
    
    // Add fourth species rules (focus on suspensions)
    rules.push_back(createRule("ConsonantIntervalRule"));
    rules.push_back(createRule("ParallelMotionRule"));
    rules.push_back(createRule("MelodicIntervalRule"));
    rules.push_back(createRule("DissonancePreparationRule"));
    
    return rules;
}

std::vector<std::unique_ptr<Rule>> RuleFactory::createFifthSpeciesRules() {
    std::vector<std::unique_ptr<Rule>> rules;
    
    // Add all available rules for fifth species
    rules.push_back(createRule("ConsonantIntervalRule"));
    rules.push_back(createRule("ParallelMotionRule"));
    rules.push_back(createRule("MelodicIntervalRule"));
    rules.push_back(createRule("DissonancePreparationRule"));
    
    return rules;
}