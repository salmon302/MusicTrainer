#include "ParallelFifthsRule.h"
#include "../domain/Score.h"
#include "../domain/Note.h"
#include "../domain/Interval.h"
#include <vector>
#include <map> // For tracking intervals between voice pairs

namespace MusicTrainer {
namespace Rules {

// Using namespace for convenience within this cpp file
using namespace Domain;

ParallelFifthsRule::ParallelFifthsRule() {
    // Initialize any members if needed
}

RuleId ParallelFifthsRule::getId() const {
    return "ParallelFifths"; // Unique ID for this rule
}

std::string ParallelFifthsRule::getName() const {
    return "Prohibit Parallel Perfect Fifths";
}

RuleType ParallelFifthsRule::getType() const {
    return RuleType::Harmonic; // This rule checks intervals between voices
}

void ParallelFifthsRule::configure(const RuleParameters& params) {
    // Store parameters if this rule becomes configurable
    currentParams = params;
    // Example: Check for a parameter like "allowInCadences"
    // if (params.count("allowInCadences")) {
    //     allowInCadences = std::get<bool>(params.at("allowInCadences"));
    // }
}

RuleParameters ParallelFifthsRule::getConfiguration() const {
    return currentParams;
}

std::vector<RuleId> ParallelFifthsRule::getDependencies() const {
    // This rule doesn't strictly depend on others for its core logic
    return {};
}

// Helper structure to represent a time point event (note start or end)
struct TimeEvent {
    Position time;
    const Note* note;
    bool isStart; // true if note start, false if note end

    // Comparison for sorting events by time
    bool operator<(const TimeEvent& other) const {
        if (time != other.time) {
            return time < other.time;
        }
        // If times are equal, process end events before start events
        // to correctly handle consecutive notes
        return !isStart && other.isStart;
    }
};


void ParallelFifthsRule::validate(const Score& score, ValidationResult& result) const {
    const auto& voices = score.getAllVoices();
    if (voices.size() < 2) {
        return; // Need at least two voices
    }

    // Iterate through all unique pairs of voices
    for (size_t i = 0; i < voices.size(); ++i) {
        for (size_t j = i + 1; j < voices.size(); ++j) {
            const auto& voice1 = voices[i];
            const auto& voice2 = voices[j];

            // 1. Create a merged list of time events for the voice pair
            std::vector<TimeEvent> events;
            events.reserve(voice1.notes.size() * 2 + voice2.notes.size() * 2);
            for (const auto& note : voice1.notes) {
                events.push_back({note.getStartTime(), &note, true});
                events.push_back({note.getEndTime(), &note, false});
            }
            for (const auto& note : voice2.notes) {
                events.push_back({note.getStartTime(), &note, true});
                events.push_back({note.getEndTime(), &note, false});
            }

            // 2. Sort events by time
            std::sort(events.begin(), events.end());

            // 3. Iterate through time intervals defined by events
            const Note* activeNote1 = nullptr;
            const Note* activeNote2 = nullptr;
            std::optional<Interval> previousInterval;
            const Note* prevIntervalNote1 = nullptr; // Note from voice1 forming previous interval
            const Note* prevIntervalNote2 = nullptr; // Note from voice2 forming previous interval
            Position previousEventTime(0.0); // Track time for interval duration

            for (const auto& event : events) {
                // Determine the interval duration before processing the current event
                double intervalDuration = event.time - previousEventTime;

                // Process the interval *before* the current event time if notes were active
                if (intervalDuration > 1e-9 && activeNote1 && activeNote2) { // Use tolerance
                    Interval currentInterval(activeNote1->getPitch(), activeNote2->getPitch());
                    bool isCurrentPerfectFifth = currentInterval.getSimpleInterval().isPerfectFifth();

                    // Check against the *previous* interval state
                    if (previousInterval.has_value() && prevIntervalNote1 && prevIntervalNote2) {
                        bool isPreviousPerfectFifth = previousInterval->getSimpleInterval().isPerfectFifth();

                        if (isCurrentPerfectFifth && isPreviousPerfectFifth) {
                            // Check that the notes forming the interval actually moved
                            // (i.e., it's not just the same notes held over)
                            // We compare the notes active *during* the previous interval
                            // with the notes active *during* the current interval.
                            if (activeNote1->getPitch() != prevIntervalNote1->getPitch() ||
                                activeNote2->getPitch() != prevIntervalNote2->getPitch())
                            {
                                Violation v(
                                    getId(),
                                    "Parallel perfect fifth between voice " + std::to_string(voice1.id) +
                                    " (" + activeNote1->getPitch().debugName() + ") and voice " + // TODO: Add debugName to Pitch
                                    std::to_string(voice2.id) + " (" + activeNote2->getPitch().debugName() + ").",
                                    Severity::Error,
                                    TimeRange(previousEventTime, event.time), // Location of the second fifth
                                    {activeNote1, activeNote2, prevIntervalNote1, prevIntervalNote2}
                                );
                                result.addViolation(v);
                            }
                        }
                    }
                    // Store the state of this interval as the "previous" for the next check
                    previousInterval = currentInterval;
                    prevIntervalNote1 = activeNote1;
                    prevIntervalNote2 = activeNote2;
                } else if (intervalDuration > 1e-9) {
                     // If one or both voices were silent during the interval, reset sequence
                     previousInterval.reset();
                     prevIntervalNote1 = nullptr;
                     prevIntervalNote2 = nullptr;
                }


                // Update active notes based on the current event
                if (event.note->getVoiceId() == voice1.id) {
                    activeNote1 = event.isStart ? event.note : nullptr;
                } else if (event.note->getVoiceId() == voice2.id) {
                    activeNote2 = event.isStart ? event.note : nullptr;
                }

                // Update the time for the next interval calculation
                previousEventTime = event.time;
            }
        }
    }
}

// This helper is no longer needed as logic is within validate
// std::optional<int> ParallelFifthsRule::calculateSemitoneInterval(const Note* n1, const Note* n2) {
//      if (!n1 || !n2) return std::nullopt;
//      return Interval(n1->getPitch(), n2->getPitch()).getSemitones();
// }


} // namespace Rules
} // namespace MusicTrainer