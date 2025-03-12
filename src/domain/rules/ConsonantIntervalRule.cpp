#include "domain/rules/ConsonantIntervalRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include <sstream>

namespace MusicTrainer::music::rules {

ConsonantIntervalRule* ConsonantIntervalRule::clone() const {
    return new ConsonantIntervalRule(*this);
}

bool ConsonantIntervalRule::evaluate(const Score& score) {
    // Early return if rule is disabled
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    if (score.getVoiceCount() < 2) return true;
    
    // Compare each pair of voices
    for (size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice& voice1 = score.getVoice(i);
            const Voice& voice2 = score.getVoice(j);
            
            // Compare notes at each position
            for (size_t pos = 0; pos < voice1.getNoteCount(); ++pos) {
                if (pos >= voice2.getNoteCount()) break;
                
                const Note& note1 = voice1.getNote(pos);
                const Note& note2 = voice2.getNote(pos);
                
                // Skip if either note is a rest
                if (note1.isRest() || note2.isRest()) continue;
                
                int interval = std::abs(note2.getMidiPitch() - note1.getMidiPitch()) % 12;
                
                // Check if the interval is dissonant
                if (!isConsonant(interval)) {
                    std::stringstream ss;
                    ss << "Dissonant interval (" << getIntervalName(interval) 
                       << ") between voice " << i + 1 << " and voice " << j + 1
                       << " at position " << pos + 1;
                    m_violationDescription = ss.str();
                    return false;
                }
            }
        }
    }
    
    return true;
}

std::string ConsonantIntervalRule::getViolationDescription() const {
    return m_violationDescription;
}

std::string ConsonantIntervalRule::getName() const {
    return "ConsonantIntervalRule";
}

bool ConsonantIntervalRule::isConsonant(int interval) const {
    // Perfect consonances: unison, perfect fifth, octave
    if (interval == 0 || interval == 7) return true;
    
    // Imperfect consonances: major and minor thirds, major and minor sixths
    if (interval == 3 || interval == 4 || interval == 8 || interval == 9) return true;
    
    return false;
}

std::string ConsonantIntervalRule::getIntervalName(int interval) const {
    switch (interval) {
        case 0: return "unison";
        case 1: return "minor second";
        case 2: return "major second";
        case 3: return "minor third";
        case 4: return "major third";
        case 5: return "perfect fourth";
        case 6: return "tritone";
        case 7: return "perfect fifth";
        case 8: return "minor sixth";
        case 9: return "major sixth";
        case 10: return "minor seventh";
        case 11: return "major seventh";
        default: return "unknown interval";
    }
}