#ifndef MUSIC_TRAINER_RULES_PARALLELFIFTHSRULE_H
#define MUSIC_TRAINER_RULES_PARALLELFIFTHSRULE_H

#include "RuleEngine.h" // Base class IRule and supporting types
#include "../domain/Interval.h" // Need interval calculation (TODO: Create Interval.h)
#include <vector>
#include <string>

namespace MusicTrainer {
namespace Rules {

/**
 * @brief Implements the rule prohibiting parallel perfect fifths between voices.
 *
 * Checks consecutive intervals between pairs of voices.
 */
class ParallelFifthsRule : public IRule {
private:
    // Rule parameters (if any needed later, e.g., allow in specific contexts)
    RuleParameters currentParams;

    // Helper function to check intervals between two notes
    // TODO: Move interval calculation logic to a dedicated Domain::Interval class/function
    std::optional<int> calculateSemitoneInterval(const Domain::Note* n1, const Domain::Note* n2);

public:
    ParallelFifthsRule(); // Constructor

    // --- IRule Interface Implementation ---
    RuleId getId() const override;
    std::string getName() const override;
    RuleType getType() const override;
    void configure(const RuleParameters& params) override;
    RuleParameters getConfiguration() const override;
    void validate(const Domain::Score& score, ValidationResult& result) const override;
    std::vector<RuleId> getDependencies() const override;

}; // class ParallelFifthsRule

} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_PARALLELFIFTHSRULE_H