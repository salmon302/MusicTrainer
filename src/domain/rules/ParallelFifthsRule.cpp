#include "domain/rules/ParallelFifthsRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Interval.h"
#include <sstream>

namespace MusicTrainer::music::rules {

ParallelFifthsRule* ParallelFifthsRule::clone() const {
    return new ParallelFifthsRule(*this);
}

bool ParallelFifthsRule::evaluate(const music::Score& score) {
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    if (score.getVoiceCount() < 2) return true;
    
    // Check each pair of voices
    for (size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice& voice1 = score.getVoice(i);
            const Voice& voice2 = score.getVoice(j);
            
            // Need at least 2 notes to check for parallel motion
            if (voice1.getNoteCount() < 2 || voice2.getNoteCount() < 2) continue;
            
            // Check consecutive notes for parallel fifths
            for (size_t noteIndex = 1; noteIndex < std::min(voice1.getNoteCount(), voice2.getNoteCount()); ++noteIndex) {
                const Note& prev1 = voice1.getNote(noteIndex - 1);
                const Note& curr1 = voice1.getNote(noteIndex);
                const Note& prev2 = voice2.getNote(noteIndex - 1);
                const Note& curr2 = voice2.getNote(noteIndex);
                
                // Skip if any note is a rest
                if (prev1.isRest() || curr1.isRest() || prev2.isRest() || curr2.isRest()) continue;
                
                // Calculate intervals
                int prevInterval = std::abs(prev2.getMidiPitch() - prev1.getMidiPitch()) % 12;
                int currInterval = std::abs(curr2.getMidiPitch() - curr1.getMidiPitch()) % 12;
                
                // Check for parallel fifths (perfect fifth = 7 semitones)
                if (prevInterval == 7 && currInterval == 7) {
                    // Verify it's parallel motion (both voices moving in same direction)
                    int voice1Motion = curr1.getMidiPitch() - prev1.getMidiPitch();
                    int voice2Motion = curr2.getMidiPitch() - prev2.getMidiPitch();
                    
                    if ((voice1Motion > 0 && voice2Motion > 0) || 
                        (voice1Motion < 0 && voice2Motion < 0)) {
                        std::stringstream ss;
                        ss << "Parallel fifths between voice " << i + 1
                           << " and voice " << j + 1
                           << " at position " << noteIndex + 1;
                        m_violationDescription = ss.str();
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

std::string ParallelFifthsRule::getViolationDescription() const {
    return m_violationDescription;
}

std::string ParallelFifthsRule::getName() const {
    return "ParallelFifthsRule";
}

} // namespace rules
} // namespace music
} // namespace MusicTrainer