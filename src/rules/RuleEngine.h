#ifndef MUSIC_TRAINER_RULES_RULEENGINE_H
#define MUSIC_TRAINER_RULES_RULEENGINE_H

#include "RuleTypes.h"
#include "../domain/Score.h" // Rule engine operates on Scores
#include <vector>
#include <string>
#include <memory> // For std::shared_ptr

// Forward declaration
namespace MusicTrainer { namespace Domain { class Score; } }

namespace MusicTrainer {
namespace Rules {

// Forward declaration
class Preset; // Presets define rule configurations

/**
 * @brief Interface for individual musical rules (Plugin).
 *
 * Each concrete rule implementation will inherit from this interface.
 * Rules are responsible for checking a specific musical constraint.
 */
class IRule {
public:
    virtual ~IRule() = default;

    // Get unique identifier for this rule implementation
    virtual RuleId getId() const = 0;

    // Get human-readable name
    virtual std::string getName() const = 0;

    // Get the category this rule belongs to
    virtual RuleType getType() const = 0;

    // Configure the rule with specific parameters (e.g., max interval size)
    virtual void configure(const RuleParameters& params) = 0;

    // Get the current parameters the rule is configured with
    virtual RuleParameters getConfiguration() const = 0;

    // Perform validation on the given score, adding Violations to the result
    virtual void validate(const Domain::Score& score, ValidationResult& result) const = 0;

    // Declare dependencies on other rules (for conflict detection)
    virtual std::vector<RuleId> getDependencies() const = 0;
};


/**
 * @brief Interface for the main Rule Engine component.
 *
 * Manages a set of active rules (loaded via Presets), orchestrates
 * validation, and potentially handles rule conflict detection.
 */
class RuleEngine {
public:
    virtual ~RuleEngine() = default;

    /**
     * @brief Loads and configures rules based on a Preset definition.
     * Clears any previously loaded rules.
     * @param preset The Preset containing the rule configurations.
     */
    virtual void loadRuleSet(const Preset& preset) = 0;

    /**
     * @brief Registers a rule plugin implementation with the engine.
     * Allows dynamic addition of new rule types.
     * @param rulePlugin A shared pointer to an object implementing IRule.
     */
    virtual void registerRulePlugin(std::shared_ptr<IRule> rulePlugin) = 0;

    /**
     * @brief Validates the entire score against the currently loaded rule set.
     * @param score The Score object to validate.
     * @return ValidationResult Containing all detected violations.
     */
    virtual ValidationResult validateScore(const Domain::Score& score) = 0;

    /**
     * @brief Validates a specific time range or region of the score.
     * Useful for incremental validation after user edits.
     * @param score The Score object.
     * @param dirtyRegion The time range that needs re-validation.
     * @return ValidationResult Containing violations found within the region.
     */
    virtual ValidationResult validateRegion(const Domain::Score& score, const TimeRange& dirtyRegion) = 0;

    /**
     * @brief Checks the currently loaded rule set (or a given Preset) for potential conflicts.
     * Based on declared dependencies between rules.
     * @param preset (Optional) A Preset to check. If nullopt, checks the currently loaded ruleset.
     * @return Vector of strings describing detected conflicts. Empty if no conflicts found.
     */
    virtual std::vector<std::string> checkRuleConflicts(const std::optional<Preset>& preset = std::nullopt) = 0;

    /**
     * @brief Gets the currently active rule parameters for a specific rule ID.
     * @param id The ID of the rule.
     * @return The parameters, or throws if rule not active/found.
     */
    virtual const RuleParameters& getActiveRuleParameters(RuleId id) const = 0;

}; // class RuleEngine

} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_RULEENGINE_H