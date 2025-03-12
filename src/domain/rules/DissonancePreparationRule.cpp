#include "domain/rules/DissonancePreparationRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Interval.h"
#include <sstream>

namespace MusicTrainer::music::rules {

DissonancePreparationRule* DissonancePreparationRule::clone() const {
    return new DissonancePreparationRule(*this);
}

bool DissonancePreparationRule::evaluate(const music::Score& score) {
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    if (score.getVoiceCount() < 2) return true;
    
    // Check each pair of voices
    for (size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice& voice1 = score.getVoice(i);
            const Voice& voice2 = score.getVoice(j);
            
            // Need at least 3 notes to check preparation and resolution
            if (voice1.getNoteCount() < 3 || voice2.getNoteCount() < 3) continue;
            
            // Check each potential dissonance
            for (size_t pos = 1; pos < voice1.getNoteCount() - 1; ++pos) {
                if (pos >= voice2.getNoteCount() - 1) break;
                
                const Note& prev1 = voice1.getNote(pos - 1);
                const Note& curr1 = voice1.getNote(pos);
                const Note& next1 = voice1.getNote(pos + 1);
                
                const Note& prev2 = voice2.getNote(pos - 1);
                const Note& curr2 = voice2.getNote(pos);
                const Note& next2 = voice2.getNote(pos + 1);
                
                // Skip if any note is a rest
                if (prev1.isRest() || curr1.isRest() || next1.isRest() ||
                    prev2.isRest() || curr2.isRest() || next2.isRest()) continue;
                
                int currInterval = std::abs(curr2.getMidiPitch() - curr1.getMidiPitch()) % 12;
                
                if (Interval::isDissonant(currInterval)) {
                    // Check preparation
                    int prevInterval = std::abs(prev2.getMidiPitch() - prev1.getMidiPitch()) % 12;
                    if (!Interval::isConsonant(prevInterval)) {
                        std::stringstream ss;
                        ss << "Unprepared dissonance (" << Interval::getIntervalName(currInterval)
                           << ") between voice " << i + 1 << " and voice " << j + 1
                           << " at position " << pos + 1;
                        m_violationDescription = ss.str();
                        return false;
                    }
                    
                    // Check resolution
                    bool voice1Steps = Interval::isStepwise(std::abs(next1.getMidiPitch() - curr1.getMidiPitch()));
                    bool voice2Steps = Interval::isStepwise(std::abs(next2.getMidiPitch() - curr2.getMidiPitch()));
                    
                    // For passing tones, at least one voice must move by step
                    if (!voice1Steps && !voice2Steps) {
                        std::stringstream ss;
                        ss << "Unresolved dissonance (" << Interval::getIntervalName(currInterval)
                           << ") between voice " << i + 1 << " and voice " << j + 1
                           << " at position " << pos + 1 << ": no stepwise resolution";
                        m_violationDescription = ss.str();
                        return false;
                    }
                    
                    // For suspensions, the dissonant note must resolve down by step
                    if (curr1.getMidiPitch() == prev1.getMidiPitch()) {
                        // Voice 1 has the suspension
                        if (next1.getMidiPitch() >= curr1.getMidiPitch()) {
                            std::stringstream ss;
                            ss << "Suspension in voice " << i + 1
                               << " at position " << pos + 1
                               << " must resolve downward by step";
                            m_violationDescription = ss.str();
                            return false;
                        }
                    } else if (curr2.getMidiPitch() == prev2.getMidiPitch()) {
                        // Voice 2 has the suspension
                        if (next2.getMidiPitch() >= curr2.getMidiPitch()) {
                            std::stringstream ss;
                            ss << "Suspension in voice " << j + 1
                               << " at position " << pos + 1
                               << " must resolve downward by step";
                            m_violationDescription = ss.str();
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

std::string DissonancePreparationRule::getViolationDescription() const {
    return m_violationDescription;
}

std::string DissonancePreparationRule::getName() const {
    return "DissonancePreparationRule";
}