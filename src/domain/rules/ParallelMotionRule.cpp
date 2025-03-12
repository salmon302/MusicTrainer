#include "domain/rules/ParallelMotionRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include <sstream>

namespace MusicTrainer::music::rules {

ParallelMotionRule* ParallelMotionRule::clone() const {
    return new ParallelMotionRule(*this);
}

bool ParallelMotionRule::evaluate(const Score& score) {
    // Early return if rule is disabled
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    if (score.getVoiceCount() < 2) return true;
    
    // Compare each pair of voices
    for (size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice& voice1 = score.getVoice(i);
            const Voice& voice2 = score.getVoice(j);
            
            // Need at least 2 notes to check for parallel motion
            if (voice1.getNoteCount() < 2 || voice2.getNoteCount() < 2) continue;
            
            // Check consecutive notes
            for (size_t pos = 0; pos < voice1.getNoteCount() - 1; ++pos) {
                if (pos + 1 >= voice2.getNoteCount()) break;
                
                const Note& note1First = voice1.getNote(pos);
                const Note& note1Second = voice1.getNote(pos + 1);
                const Note& note2First = voice2.getNote(pos);
                const Note& note2Second = voice2.getNote(pos + 1);
                
                // Skip if any note is a rest
                if (note1First.isRest() || note1Second.isRest() || 
                    note2First.isRest() || note2Second.isRest()) continue;
                
                int interval1 = std::abs(note2First.getMidiPitch() - note1First.getMidiPitch()) % 12;
                int interval2 = std::abs(note2Second.getMidiPitch() - note1Second.getMidiPitch()) % 12;
                
                // Check for parallel fifths or octaves
                if ((interval1 == 7 && interval2 == 7) || // Perfect fifths
                    (interval1 == 0 && interval2 == 0)) { // Octaves/unisons
                    
                    // Check if motion is parallel
                    int voice1Motion = note1Second.getMidiPitch() - note1First.getMidiPitch();
                    int voice2Motion = note2Second.getMidiPitch() - note2First.getMidiPitch();
                    
                    if ((voice1Motion > 0 && voice2Motion > 0) || 
                        (voice1Motion < 0 && voice2Motion < 0)) {
                        std::stringstream ss;
                        ss << "Parallel " << (interval1 == 7 ? "fifths" : "octaves")
                           << " between voice " << i + 1 << " and voice " << j + 1
                           << " at position " << pos + 1;
                        m_violationDescription = ss.str();
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

std::string ParallelMotionRule::getViolationDescription() const {
    return m_violationDescription;
}

std::string ParallelMotionRule::getName() const {
    return "ParallelMotionRule";
}