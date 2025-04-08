#ifndef MUSIC_TRAINER_RULES_CONCRETERULEENGINE_H
#define MUSIC_TRAINER_RULES_CONCRETERULEENGINE_H

#include "RuleEngine.h" // Base interface
#include "Preset.h"     // Needs Preset definition (TODO: Create Preset.h)
#include <vector>
#include <string>
#include <memory>       // For std::shared_ptr
#include <map>
#include <optional>

namespace MusicTrainer {
namespace Rules {

/**
 * @brief Concrete implementation of the RuleEngine interface.
 */
class ConcreteRuleEngine : public RuleEngine {
private:
    // Registry of all available rule plugin implementations, mapped by RuleId
    std::map<RuleId, std::shared_ptr<IRule>> registeredRulePlugins;

    // Currently active rules, configured according to the loaded preset
    std::vector<std::shared_ptr<IRule>> activeRules;

    // Store the loaded preset definition for reference
    std::optional<Preset> loadedPreset;

public:
    ConcreteRuleEngine();
    virtual ~ConcreteRuleEngine() override = default;

    // --- RuleEngine Interface Implementation ---

    void loadRuleSet(const Preset& preset) override;

    void registerRulePlugin(std::shared_ptr<IRule> rulePlugin) override;

    ValidationResult validateScore(const Domain::Score& score) override;

    ValidationResult validateRegion(const Domain::Score& score, const TimeRange& dirtyRegion) override;

    std::vector<std::string> checkRuleConflicts(const std::optional<Preset>& preset = std::nullopt) override;

    const RuleParameters& getActiveRuleParameters(RuleId id) const override;

}; // class ConcreteRuleEngine

} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_CONCRETERULEENGINE_H