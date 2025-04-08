#ifndef MUSIC_TRAINER_RULES_RULETYPES_H
#define MUSIC_TRAINER_RULES_RULETYPES_H

#include <string>
#include <vector>
#include <cstdint>
#include <variant> // For RuleParameters if needed later
#include <map>

// Include necessary Domain headers instead of just forward declaring
#include "../domain/Position.h"
#include "../domain/Note.h" // Needed for involvedNotes vector

// Forward declarations no longer needed for included headers
// namespace MusicTrainer { namespace Domain { class Note; class Position; } }


namespace MusicTrainer {
namespace Rules {

// Unique identifier for a specific rule implementation
// Could be an enum class if rules are fixed, or string/hash for dynamic loading
using RuleId = std::string;

// Type categorization for rules
enum class RuleType : std::uint8_t {
    Melodic,    // Rules applied to a single voice melody
    Harmonic,   // Rules applied between two or more voices
    Rhythmic,   // Rules related to timing and duration
    Structural  // Rules related to overall form or sections (e.g., cadences)
};

// Severity level for a rule violation
enum class Severity : std::uint8_t {
    Info,       // Informational message
    Warning,    // Potential issue, stylistic suggestion
    Error       // Clear violation of the rule
};

// Represents a range in musical time (could potentially move to Domain later)
// Using simple doubles for now, might refine with Position later if needed
struct TimeRange {
    double startTimeBeats = 0.0;
    double endTimeBeats = 0.0;

    // Default constructor
    TimeRange() = default;

    // Constructor
    TimeRange(double start, double end) : startTimeBeats(start), endTimeBeats(end) {}

     // Constructor from Positions
    TimeRange(const Domain::Position& start, const Domain::Position& end)
        : startTimeBeats(start.beats), endTimeBeats(end.beats) {}

    double getDuration() const { return endTimeBeats - startTimeBeats; }
};


// Represents a single validation failure identified by a rule
struct Violation {
    RuleId ruleId;              // Which rule was violated
    std::string description;    // Human-readable explanation
    Severity severity;          // How critical the violation is
    std::vector<const Domain::Note*> involvedNotes; // Pointers to notes involved
    TimeRange location;         // Time range where the violation occurred

    // Constructor
    Violation(RuleId id, std::string desc, Severity sev, TimeRange loc = {}, std::vector<const Domain::Note*> notes = {})
        : ruleId(std::move(id)),
          description(std::move(desc)),
          severity(sev),
          involvedNotes(std::move(notes)),
          location(loc)
          {}
};

// Result of a validation pass for a score or section
struct ValidationResult {
    std::vector<Violation> violations;
    bool isValid = true; // Overall validity (usually true if no Errors)

    void addViolation(const Violation& v) {
        violations.push_back(v);
        if (v.severity == Severity::Error) {
            isValid = false;
        }
    }

    void clear() {
        violations.clear();
        isValid = true;
    }
};

// Type alias for rule parameters (can be expanded)
// Using a map allows named parameters, variant allows different types.
using RuleParameterValue = std::variant<int, double, bool, std::string>;
using RuleParameters = std::map<std::string, RuleParameterValue>;


} // namespace Rules
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_RULES_RULETYPES_H