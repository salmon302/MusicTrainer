#include "domain/rules/ParallelOctavesRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Interval.h"
#include "domain/music/Pitch.h"
#include <iostream>

namespace MusicTrainer {
namespace music {
namespace rules {

std::unique_ptr<ParallelOctavesRule> ParallelOctavesRule::create() {
    return std::unique_ptr<ParallelOctavesRule>(new ParallelOctavesRule());
}

ParallelOctavesRule::ParallelOctavesRule() : violationDescription() {}

ParallelOctavesRule::ParallelOctavesRule(const std::string& initialViolation)
    : violationDescription(initialViolation) {}

ParallelOctavesRule::~ParallelOctavesRule() = default;

bool ParallelOctavesRule::evaluate(const Score& score) {
    return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool ParallelOctavesRule::evaluateIncremental(
    const Score& score,
    size_t startMeasure,
    size_t endMeasure) const {
    
    clearViolationDescription();
    
    size_t voiceCount = score.getVoiceCount();
    if (voiceCount < 2) {
        return true; // No parallel octaves possible with less than 2 voices
    }

    // Check each pair of voices
    for (size_t i = 0; i < voiceCount - 1; ++i) {
        for (size_t j = i + 1; j < voiceCount; ++j) {
            const Voice* voice1 = score.getVoice(i);
            const Voice* voice2 = score.getVoice(j);
            if (!voice1 || !voice2) continue;

            // Get notes in the measure range
            auto notes1 = voice1->getNotesInRange(startMeasure, endMeasure);
            auto notes2 = voice2->getNotesInRange(startMeasure, endMeasure);

            // Skip if either voice has no notes
            if (notes1.empty() || notes2.empty()) continue;

            // Check for parallel octaves between consecutive notes
            for (size_t k = 0; k < std::min(notes1.size(), notes2.size()) - 1; ++k) {
                // Skip if either pair of notes doesn't overlap in time
                if (notes1[k].getPosition() != notes2[k].getPosition() ||
                    notes1[k+1].getPosition() != notes2[k+1].getPosition()) {
                    continue;
                }

                // Calculate intervals
                Interval curr = Interval::fromPitches(notes1[k].getPitch(), notes2[k].getPitch());
                Interval next = Interval::fromPitches(notes1[k+1].getPitch(), notes2[k+1].getPitch());

                // Debug output
                std::cout << "Checking intervals between voices " << i << " and " << j << ":" << std::endl;
                std::cout << "  Current interval: " << curr.toString()
                          << " (Notes: " << notes1[k].getPitch().toString() 
                          << ", " << notes2[k].getPitch().toString() << ")" << std::endl;
                std::cout << "  Next interval: " << next.toString()
                          << " (Notes: " << notes1[k+1].getPitch().toString() 
                          << ", " << notes2[k+1].getPitch().toString() << ")" << std::endl;

                // Check for parallel octaves
                if (curr.getNumber() == Interval::Number::Octave &&
                    next.getNumber() == Interval::Number::Octave &&
                    curr.getQuality() == Interval::Quality::Perfect &&
                    next.getQuality() == Interval::Quality::Perfect) {
                    
                    std::string desc = "Parallel octaves found between voices " + 
                                     std::to_string(i) + " and " + std::to_string(j) +
                                     " at measure " + std::to_string(startMeasure + k);
                    setViolationDescription(desc);
                    return false;
                }
            }
        }
    }
    
    return true;
}

std::string ParallelOctavesRule::getViolationDescription() const {
    return violationDescription;
}

std::string ParallelOctavesRule::getName() const {
    return "Parallel Octaves Rule";
}

std::unique_ptr<Rule> ParallelOctavesRule::clone() const {
    return std::unique_ptr<Rule>(new ParallelOctavesRule(violationDescription));
}

} // namespace rules
} // namespace music
} // namespace MusicTrainer