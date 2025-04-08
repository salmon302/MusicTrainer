#include "domain/rules/MelodicIntervalRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Interval.h"
#include "domain/music/Pitch.h"
#include <sstream>
#include <string>
#include <cstdlib>

namespace MusicTrainer::music::rules {

MelodicIntervalRule* MelodicIntervalRule::clone() const {
    return new MelodicIntervalRule(*this);
}

bool MelodicIntervalRule::evaluate(const Score& score) {
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    for (std::size_t voiceIndex = 0; voiceIndex < score.getVoiceCount(); ++voiceIndex) {
        const Voice* voice = score.getVoice(voiceIndex);
        if (!voice) continue;
        
        // Need at least 2 notes to check intervals
        const Note* curr = voice->getNoteAt(1);
        if (!curr) continue;
        
        // Check all intervals between consecutive notes
        for (std::size_t noteIndex = 1; curr != nullptr; ++noteIndex) {
            if (!checkMelodicInterval(*voice, noteIndex)) {
                return false;
            }
            curr = voice->getNoteAt(noteIndex + 1);
        }
    }
    return true;
}

bool MelodicIntervalRule::checkMelodicInterval(const Voice& voice, std::size_t noteIndex) {
    const Note* curr = voice.getNoteAt(noteIndex);
    const Note* prev = voice.getNoteAt(noteIndex - 1);
    
    if (!curr || !prev) return true;
    
    // Skip if either note is a rest
    if (curr->isRest() || prev->isRest()) return true;
    
    int interval = std::abs(curr->getPitch().getMidiNote() - prev->getPitch().getMidiNote());
    
    // Check for diminished intervals
    if (music::Interval::isDiminishedInterval(interval)) {
        std::stringstream ss;
        ss << "Invalid melodic interval: diminished " << music::Interval::getIntervalName(interval)
           << " at position " << noteIndex + 1;
        m_violationDescription = ss.str();
        return false;
    }
    
    // Check for intervals larger than an octave
    if (interval > 12) {
        std::stringstream ss;
        ss << "Invalid melodic interval: leap of " << interval
           << " semitones at position " << noteIndex + 1
           << " exceeds an octave";
        m_violationDescription = ss.str();
        return false;
    }
    
    // Check that the interval is valid for melodic motion
    if (!music::Interval::isValidMelodicInterval(interval)) {
        std::stringstream ss;
        ss << "Invalid melodic interval: " << music::Interval::getIntervalName(interval)
           << " at position " << noteIndex + 1;
        m_violationDescription = ss.str();
        return false;
    }
    
    // For large leaps (P5 or larger), check if followed by stepwise motion in opposite direction
    const Note* next = voice.getNoteAt(noteIndex + 1);
    if (interval >= 7 && next && !next->isRest()) {
        int nextInterval = next->getPitch().getMidiNote() - curr->getPitch().getMidiNote();
        // Check if motion is in opposite direction and stepwise
        bool oppositeDirection = (nextInterval * (curr->getPitch().getMidiNote() - prev->getPitch().getMidiNote()) < 0);
        bool isStepwise = music::Interval::isStepwise(std::abs(nextInterval));
        
        if (!(oppositeDirection && isStepwise)) {
            std::stringstream ss;
            ss << "Large leap of " << music::Interval::getIntervalName(interval)
               << " at position " << noteIndex + 1
               << " should be followed by stepwise motion in opposite direction";
            m_violationDescription = ss.str();
            return false;
        }
    }
    return true;
}

std::string MelodicIntervalRule::getViolationDescription() const {
    return m_violationDescription;
}

std::string MelodicIntervalRule::getName() const {
    return "MelodicIntervalRule";
}

} // namespace MusicTrainer::music::rules