#include "domain/rules/VoiceLeadingRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Interval.h"
#include "domain/music/Pitch.h"
#include <iostream>

namespace MusicTrainer {
namespace music {
namespace rules {

std::unique_ptr<VoiceLeadingRule> VoiceLeadingRule::create() {
    return std::unique_ptr<VoiceLeadingRule>(new VoiceLeadingRule());
}

VoiceLeadingRule::VoiceLeadingRule() : violationDescription() {}

VoiceLeadingRule::VoiceLeadingRule(const std::string& initialViolation)
    : violationDescription(initialViolation) {}

bool VoiceLeadingRule::evaluate(const Score& score) {
    return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool VoiceLeadingRule::evaluateIncremental(
    const Score& score,
    size_t startMeasure,
    size_t endMeasure) const {
    
    clearViolationDescription();
    
    size_t voiceCount = score.getVoiceCount();
    if (voiceCount < 1) {
        return true; // No voice leading issues possible with less than 1 voice
    }

    // Check each voice for voice leading within itself
    for (size_t i = 0; i < voiceCount; ++i) {
        const Voice* voice = score.getVoice(i);
        if (!voice) continue;

        auto notes = voice->getNotesInRange(startMeasure, endMeasure);
        if (notes.size() < 2) continue;

        // Check for melodic intervals larger than an octave
        for (size_t k = 0; k < notes.size() - 1; ++k) {
            // Skip if notes don't immediately follow each other
            if (notes[k].getPosition() + notes[k].getDuration() != notes[k+1].getPosition()) {
                continue;
            }

            Interval interval = Interval::fromPitches(notes[k].getPitch(), notes[k+1].getPitch());
            int semitones = std::abs(interval.getSemitones());

            if (semitones > 12) { // More than an octave
                std::string desc = "Voice " + std::to_string(i) + 
                                 " has a melodic interval larger than an octave at measure " +
                                 std::to_string(startMeasure + k);
                setViolationDescription(desc);
                return false;
            }
        }
    }

    // Check voice crossing between pairs of voices
    for (size_t i = 0; i < voiceCount - 1; ++i) {
        const Voice* upperVoice = score.getVoice(i);
        if (!upperVoice) continue;

        auto upperNotes = upperVoice->getNotesInRange(startMeasure, endMeasure);
        if (upperNotes.empty()) continue;

        for (size_t j = i + 1; j < voiceCount; ++j) {
            const Voice* lowerVoice = score.getVoice(j);
            if (!lowerVoice) continue;

            auto lowerNotes = lowerVoice->getNotesInRange(startMeasure, endMeasure);
            if (lowerNotes.empty()) continue;

            // Check for voice crossing at each position where both voices have notes
            for (size_t k = 0; k < std::min(upperNotes.size(), lowerNotes.size()); ++k) {
                if (upperNotes[k].getPosition() != lowerNotes[k].getPosition()) continue;

                // Compare pitches - lower voice should not be higher than upper voice
                if (lowerNotes[k].getPitch() > upperNotes[k].getPitch()) {
                    std::string desc = "Voice crossing detected between voices " +
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

std::string VoiceLeadingRule::getViolationDescription() const {
    return violationDescription;
}

std::string VoiceLeadingRule::getName() const {
    return "Voice Leading Rule";
}

Rule* VoiceLeadingRule::clone() const {
    return new VoiceLeadingRule(violationDescription);
}

} // namespace rules
} // namespace music
} // namespace MusicTrainer

