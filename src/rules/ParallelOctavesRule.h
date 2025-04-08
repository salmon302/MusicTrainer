#ifndef MUSIC_TRAINER_RULES_PARALLELOCTAVESRULE_H
#define MUSIC_TRAINER_RULES_PARALLELOCTAVESRULE_H

#include "RuleEngine.h" // Base class IRule and supporting types
#include "../domain/Interval.h" // Need interval calculation
#include <vector>
#include <string>
#include <optional> // For std::optional

namespace MusicTrainer {
namespace Rules {

/**
 * @brief Implements the rule prohibiting parallel perfect octaves (and unisons)
 *        between voices.
 *
 * Checks consecutive intervals between pairs of voices.
 */
class ParallelOctavesRule : public IRule {
private:
    // Rule parameters (if any needed later)
    RuleParameters currentParams;

public:
    ParallelOctavesRule(); // Constructor

    // --- IRule Interface Implementation ---
    RuleId getId() const override;
    std::string getName() const override;
    RuleType getType() const override;
    void configure(const RuleParameters& params) override;
    RuleParameters getConfiguration() const override;
    void validate(const Domain::Score& score, ValidationResult& result) const override;
    std::vector<RuleId> getDependencies() const override;

}; // class ParallelOctavesRule

} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_PARALLELOCTAVESRULE_H