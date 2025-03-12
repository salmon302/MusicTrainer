#pragma once

#include <memory>
#include <vector>
#include <string>

namespace MusicTrainer::music::rules {

class Rule;

/**
 * @brief Factory class for creating rule sets based on exercise type
 */
class RuleFactory {
public:
    /**
     * @brief Create a set of rules for the given exercise type
     * @param exerciseType The type of exercise
     * @return Vector of unique_ptr to Rule objects
     */
    static std::vector<std::unique_ptr<Rule>> createRuleSet(const std::string& exerciseType);

    /**
     * @brief Create a rule by name
     * @param ruleName The name of the rule to create
     * @return unique_ptr to Rule object, or nullptr if rule not found
     */
    static std::unique_ptr<Rule> createRule(const std::string& ruleName);

private:
    static std::vector<std::unique_ptr<Rule>> createFirstSpeciesRules();
    static std::vector<std::unique_ptr<Rule>> createSecondSpeciesRules();
    static std::vector<std::unique_ptr<Rule>> createThirdSpeciesRules();
    static std::vector<std::unique_ptr<Rule>> createFourthSpeciesRules();
    static std::vector<std::unique_ptr<Rule>> createFifthSpeciesRules();
};