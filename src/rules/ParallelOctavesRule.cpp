#include "ParallelOctavesRule.h"
#include "../domain/Score.h"
#include "../domain/Note.h"
#include "../domain/Interval.h"
#include "RuleTypes.h" // Include for TimeEvent definition reuse (or move TimeEvent)
#include <vector>
#include <map>
#include <optional>
#include <algorithm> // For std::sort
#include <iostream> // Temp for debug

namespace MusicTrainer {
namespace Rules {

// Using namespace for convenience within this cpp file
using namespace Domain;

// Re-define TimeEvent locally or move to a common header if used by more rules
// For now, duplicate for simplicity, but refactoring is recommended.
struct TimeEvent {
    Position time;
    const Note* note;
    bool isStart; // true if note start, false if note end

    bool operator<(const TimeEvent& other) const {
        if (time != other.time) {
            return time < other.time;
        }
        return !isStart && other.isStart;
    }
};


ParallelOctavesRule::ParallelOctavesRule() {
    // Initialize members if needed
}

RuleId ParallelOctavesRule::getId() const {
    return "ParallelOctaves"; // Unique ID
}

std::string ParallelOctavesRule::getName() const {
    return "Prohibit Parallel Perfect Octaves/Unisons";
}

RuleType ParallelOctavesRule::getType() const {
    return RuleType::Harmonic;
}

void ParallelOctavesRule::configure(const RuleParameters& params) {
    currentParams = params;
    // Example: Check for parameter like "allowUnisonIfDifferentVoices"
}

RuleParameters ParallelOctavesRule::getConfiguration() const {
    return currentParams;
}

std::vector<RuleId> ParallelOctavesRule::getDependencies() const {
    return {}; // No strict dependencies for this logic
}


void ParallelOctavesRule::validate(const Score& score, ValidationResult& result) const {
    const auto& voices = score.getAllVoices();
    if (voices.size() < 2) {
        return; // Need at least two voices
    }

    // Iterate through all unique pairs of voices
    for (size_t i = 0; i < voices.size(); ++i) {
        for (size_t j = i + 1; j < voices.size(); ++j) {
            const auto& voice1 = voices[i];
            const auto& voice2 = voices[j];

            // 1. Create merged list of time events
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

            // 2. Sort events
            std::sort(events.begin(), events.end());

            // 3. Iterate through time intervals
            const Note* activeNote1 = nullptr;
            const Note* activeNote2 = nullptr;
            std::optional<Interval> previousInterval;
            const Note* prevIntervalNote1 = nullptr;
            const Note* prevIntervalNote2 = nullptr;
            Position previousEventTime(0.0);

            for (const auto& event : events) {
                double intervalDuration = event.time - previousEventTime;

                if (intervalDuration > 1e-9 && activeNote1 && activeNote2) {
                    Interval currentInterval(activeNote1->getPitch(), activeNote2->getPitch());
                    // Check for Perfect Octave (12) or Perfect Unison (0)
                    bool isCurrentPerfectOctaveOrUnison = currentInterval.getSimpleInterval().isPerfectOctave() || currentInterval.getSimpleInterval().isPerfectUnison();

                    if (previousInterval.has_value() && prevIntervalNote1 && prevIntervalNote2) {
                        bool isPreviousPerfectOctaveOrUnison = previousInterval->getSimpleInterval().isPerfectOctave() || previousInterval->getSimpleInterval().isPerfectUnison();

                        // Check for P8/P1 -> P8/P1
                        if (isCurrentPerfectOctaveOrUnison && isPreviousPerfectOctaveOrUnison) {
                            // Check notes actually moved
                            if (activeNote1->getPitch() != prevIntervalNote1->getPitch() ||
                                activeNote2->getPitch() != prevIntervalNote2->getPitch())
                            {
                                std::string intervalType = currentInterval.isPerfectUnison() ? "unison" : "octave";
                                Violation v(
                                    getId(),
                                    "Parallel perfect " + intervalType + " between voice " + std::to_string(voice1.id) +
                                    " (" + activeNote1->getPitch().debugName() + ") and voice " +
                                    std::to_string(voice2.id) + " (" + activeNote2->getPitch().debugName() + ").",
                                    Severity::Error,
                                    TimeRange(previousEventTime, event.time),
                                    {activeNote1, activeNote2, prevIntervalNote1, prevIntervalNote2}
                                );
                                result.addViolation(v);
                            }
                        }
                    }
                    previousInterval = currentInterval;
                    prevIntervalNote1 = activeNote1;
                    prevIntervalNote2 = activeNote2;
                } else if (intervalDuration > 1e-9) {
                     previousInterval.reset();
                     prevIntervalNote1 = nullptr;
                     prevIntervalNote2 = nullptr;
                }

                // Update active notes
                if (event.note->getVoiceId() == voice1.id) {
                    activeNote1 = event.isStart ? event.note : nullptr;
                } else if (event.note->getVoiceId() == voice2.id) {
                    activeNote2 = event.isStart ? event.note : nullptr;
                }
                previousEventTime = event.time;
            }
        }
    }
}


} // namespace Rules
} // namespace MusicTrainer