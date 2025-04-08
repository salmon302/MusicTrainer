#ifndef MUSIC_TRAINER_RULES_PRESET_H
#define MUSIC_TRAINER_RULES_PRESET_H

#include "RuleTypes.h" // Needs RuleId, RuleParameters
#include <string>
#include <vector>
#include <map>

namespace MusicTrainer {
namespace Rules {

/**
 * @brief Represents a configured rule within a preset.
 */
struct PresetRuleConfig {
    RuleId ruleId;      // ID of the rule implementation to use
    bool isEnabled;     // Whether this rule is active in the preset
    RuleParameters parameters; // Specific parameters for this rule instance

    // Default constructor
    PresetRuleConfig(RuleId id = "", bool enabled = true, RuleParameters params = {})
        : ruleId(std::move(id)), isEnabled(enabled), parameters(std::move(params)) {}
};

/**
 * @brief Represents a named collection of rule configurations (a Preset).
 *
 * Presets define which rules are active and how they are parameterized
 * for a specific validation context (e.g., "Strict Species 1", "Basic Harmony Check").
 * Presets are typically loaded from external files (e.g., JSON).
 */
struct Preset {
    std::string id;         // Unique identifier for the preset (e.g., "fuxian_s1")
    std::string name;       // Human-readable name (e.g., "Fuxian First Species")
    std::string description;// Optional description
    std::vector<PresetRuleConfig> ruleConfigs; // List of configured rules

    // Default constructor
    Preset() = default;

    // Constructor
    Preset(std::string pid, std::string pname, std::string pdesc = "", std::vector<PresetRuleConfig> configs = {})
        : id(std::move(pid)),
          name(std::move(pname)),
          description(std::move(pdesc)),
          ruleConfigs(std::move(configs))
          {}

    // Helper to find a specific rule config by ID
    const PresetRuleConfig* findRuleConfig(const RuleId& ruleId) const {
        for (const auto& config : ruleConfigs) {
            if (config.ruleId == ruleId) {
                return &config;
            }
        }
        return nullptr;
    }
};

} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_PRESET_H