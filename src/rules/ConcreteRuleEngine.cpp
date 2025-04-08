#include "ConcreteRuleEngine.h"
#include "Preset.h"
#include <stdexcept> // For std::runtime_error, std::out_of_range
#include <iostream> // For temporary debug/conflict output

namespace MusicTrainer {
namespace Rules {

ConcreteRuleEngine::ConcreteRuleEngine() {
    // Constructor: Can pre-register built-in rules here if desired
    // Example: registerRulePlugin(std::make_shared<ParallelFifthsRule>());
}

void ConcreteRuleEngine::registerRulePlugin(std::shared_ptr<IRule> rulePlugin) {
    if (!rulePlugin) return;
    RuleId id = rulePlugin->getId();
    if (id.empty()) {
        // Handle error: Rule plugin must have a valid ID
        throw std::runtime_error("Attempted to register a rule plugin with an empty ID.");
    }
    registeredRulePlugins[id] = std::move(rulePlugin);
    // std::cout << "Registered rule plugin: " << id << std::endl; // Debug
}

void ConcreteRuleEngine::loadRuleSet(const Preset& preset) {
    activeRules.clear();
    loadedPreset = preset; // Store the loaded preset definition

    // std::cout << "Loading preset: " << preset.name << std::endl; // Debug

    for (const auto& config : preset.ruleConfigs) {
        if (config.isEnabled) {
            auto it = registeredRulePlugins.find(config.ruleId);
            if (it != registeredRulePlugins.end()) {
                // Found a registered plugin matching the rule ID
                std::shared_ptr<IRule> ruleInstance = it->second; // Use the registered instance
                try {
                    ruleInstance->configure(config.parameters); // Apply preset-specific parameters
                    activeRules.push_back(ruleInstance);
                    // std::cout << "  Activated rule: " << config.ruleId << std::endl; // Debug
                } catch (const std::exception& e) {
                    // Handle configuration error (e.g., invalid parameter)
                     throw std::runtime_error("Error configuring rule '" + config.ruleId + "' from preset '" + preset.id + "': " + e.what());
                }
            } else {
                // Handle error: Rule ID specified in preset is not registered
                 throw std::runtime_error("Rule ID '" + config.ruleId + "' specified in preset '" + preset.id + "' is not registered.");
            }
        }
    }
     // Optionally run conflict check after loading
     // checkRuleConflicts(); // This might print warnings/errors
}

ValidationResult ConcreteRuleEngine::validateScore(const Domain::Score& score) {
    ValidationResult combinedResult;
    if (!loadedPreset.has_value()) {
         // Maybe return an error or default result if no preset is loaded
         return combinedResult; // Or throw?
    }

    // std::cout << "Validating score against preset: " << loadedPreset->name << std::endl; // Debug
    for (const auto& rule : activeRules) {
        // std::cout << "  Running rule: " << rule->getId() << std::endl; // Debug
        rule->validate(score, combinedResult); // Each rule appends its violations
    }
    return combinedResult;
}

ValidationResult ConcreteRuleEngine::validateRegion(const Domain::Score& score, const TimeRange& dirtyRegion) {
    // TODO: Implement incremental validation logic.
    // This is more complex. It requires:
    // 1. Identifying which rules *might* be affected by changes in the dirtyRegion.
    //    (e.g., based on rule type or specific dependencies).
    // 2. Passing the dirtyRegion context to the relevant rule's validate method,
    //    or having rules designed to work incrementally.
    // 3. Clearing previous violations only within that region or from affected rules.
    // For now, just re-validate the whole score as a fallback.
    // std::cout << "Warning: validateRegion called, performing full score validation as fallback." << std::endl; // Debug
    return validateScore(score);
}

std::vector<std::string> ConcreteRuleEngine::checkRuleConflicts(const std::optional<Preset>& presetOpt) {
    // TODO: Implement rule conflict detection logic.
    // This typically involves building a dependency graph based on IRule::getDependencies()
    // for the active rules (either from loadedPreset or the provided presetOpt)
    // and checking for cycles or incompatible constraints.
    std::vector<std::string> conflicts;
    const Preset* presetToCheck = nullptr;

    if (presetOpt.has_value()) {
        presetToCheck = &presetOpt.value();
         // std::cout << "Checking conflicts for provided preset: " << presetToCheck->name << std::endl; // Debug
    } else if (loadedPreset.has_value()) {
        presetToCheck = &loadedPreset.value();
         // std::cout << "Checking conflicts for loaded preset: " << presetToCheck->name << std::endl; // Debug
    } else {
        // std::cout << "No preset loaded or provided to check for conflicts." << std::endl; // Debug
        return conflicts; // No rules active to check
    }

    // Placeholder: Add actual graph traversal and conflict logic here
    // Example: Check if Rule A depends on Rule B and Rule B depends on Rule A
    // Example: Check if Rule X requires max interval=5 and Rule Y requires min interval=6

    // if (/* conflict detected */) {
    //     conflicts.push_back("Conflict detected between Rule A and Rule B: Circular dependency.");
    // }

     if (conflicts.empty()) {
         // std::cout << "No rule conflicts detected." << std::endl; // Debug
     } else {
         // std::cout << "Rule conflicts detected:" << std::endl; // Debug
         // for(const auto& c : conflicts) { std::cout << " - " << c << std::endl; }
     }

    return conflicts;
}

const RuleParameters& ConcreteRuleEngine::getActiveRuleParameters(RuleId id) const {
    if (!loadedPreset.has_value()) {
        throw std::runtime_error("No preset loaded, cannot get rule parameters.");
    }
    // Find the active rule instance first (more robust than just checking preset)
    for (const auto& rule : activeRules) {
        if (rule->getId() == id) {
            // Found the active, configured rule instance
            return rule->getConfiguration();
        }
    }
    // If not found among active rules (maybe disabled?), check the preset definition
    const PresetRuleConfig* config = loadedPreset->findRuleConfig(id);
    if (config) {
         // Rule exists in preset but might not be active or configured yet
         // Returning config->parameters might be okay, but indicates an issue
         // Let's throw instead, as the request is for *active* parameters
         throw std::out_of_range("Rule '" + id + "' is defined in the preset but not currently active.");
    }

    throw std::out_of_range("Rule '" + id + "' not found in the active rule set.");
}


} // namespace Rules
} // namespace MusicTrainer