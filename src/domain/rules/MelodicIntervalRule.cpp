#include "domain/rules/MelodicIntervalRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Interval.h"
#include <sstream>

namespace MusicTrainer::music::rules {

MelodicIntervalRule* MelodicIntervalRule::clone() const {
    return new MelodicIntervalRule(*this);
}

bool MelodicIntervalRule::evaluate(const music::Score& score) {
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    for (size_t voiceIndex = 0; voiceIndex < score.getVoiceCount(); ++voiceIndex) {
        const Voice& voice = score.getVoice(voiceIndex);
        
        // Need at least 2 notes to check intervals
        if (voice.getNoteCount() < 2) continue;
        
        // Check all intervals between consecutive notes
        for (size_t noteIndex = 1; noteIndex < voice.getNoteCount(); ++noteIndex) {
            if (!checkMelodicInterval(voice, noteIndex)) {
                return false;
            }
        }
    }
    
    return true;
}

bool MelodicIntervalRule::checkMelodicInterval(const Voice& voice, size_t noteIndex) {
    const Note& curr = voice.getNote(noteIndex);
    const Note& prev = voice.getNote(noteIndex - 1);
    
    // Skip if either note is a rest
    if (curr.isRest() || prev.isRest()) return true;
    
    int interval = std::abs(curr.getMidiPitch() - prev.getMidiPitch());
    
    // Check for diminished intervals
    if (Interval::isDiminishedInterval(interval)) {
        std::stringstream ss;
        ss << "Invalid melodic interval: diminished " << Interval::getIntervalName(interval)
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
    if (!Interval::isValidMelodicInterval(interval)) {
        std::stringstream ss;
        ss << "Invalid melodic interval: " << Interval::getIntervalName(interval)
           << " at position " << noteIndex + 1;
        m_violationDescription = ss.str();
        return false;
    }
    
    // For large leaps (P5 or larger), check if followed by stepwise motion in opposite direction
    if (interval >= 7 && noteIndex + 1 < voice.getNoteCount()) {
        const Note& next = voice.getNote(noteIndex + 1);
        if (!next.isRest()) {
            int nextInterval = next.getMidiPitch() - curr.getMidiPitch();
            
            // Check if motion is in opposite direction and stepwise
            bool oppositeDirection = (nextInterval * (curr.getMidiPitch() - prev.getMidiPitch()) < 0);
            bool isStepwise = Interval::isStepwise(std::abs(nextInterval));
            
            if (!(oppositeDirection && isStepwise)) {
                std::stringstream ss;
                ss << "Large leap of " << Interval::getIntervalName(interval)
                   << " at position " << noteIndex + 1
                   << " should be followed by stepwise motion in opposite direction";
                m_violationDescription = ss.str();
                return false;
            }
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