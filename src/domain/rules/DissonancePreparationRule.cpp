// Force recompile check

#include "domain/rules/DissonancePreparationRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Interval.h"
// #include <sstream> // Removed to avoid compiler/library issues
#include <cstdlib>  // For std::abs
#include <string>   // For std::string, std::to_string
#include <cstddef>  // For std::size_t
#include <algorithm> // For std::max

namespace MusicTrainer::music::rules {

DissonancePreparationRule* DissonancePreparationRule::clone() const {
    return new DissonancePreparationRule(*this);
}

bool DissonancePreparationRule::evaluate(const Score& score) {
    if (!isEnabled()) return true;
    
    m_violationDescription.clear();
    
    // Need at least 2 voices to check dissonance preparation
    if (score.getVoiceCount() < 2) return true;
    
    // Check each pair of voices
    for (std::size_t v1 = 0; v1 < score.getVoiceCount(); ++v1) {
        for (std::size_t v2 = v1 + 1; v2 < score.getVoiceCount(); ++v2) {
            const Voice* voice1 = score.getVoice(v1);
            const Voice* voice2 = score.getVoice(v2);
            
            if (!voice1 || !voice2) continue;
            
            // Get note count using getAllNotes().size() as getNoteCount() doesn't exist
            std::size_t maxNotes = std::max(voice1->getAllNotes().size(), voice2->getAllNotes().size());
            // Start from second note as we need previous note for preparation
            for (std::size_t i = 1; i < maxNotes; ++i) {
                if (!checkDissonancePreparation(*voice1, *voice2, i)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool DissonancePreparationRule::checkDissonancePreparation(
    const Voice& voice1, const Voice& voice2, std::size_t noteIndex) {
    
    const Note* curr1 = voice1.getNoteAt(noteIndex); // Use . for reference
    const Note* curr2 = voice2.getNoteAt(noteIndex); // Use . for reference
    const Note* prev1 = voice1.getNoteAt(noteIndex - 1); // Use . for reference
    const Note* prev2 = voice2.getNoteAt(noteIndex - 1); // Use . for reference
    
    // Skip if any note is missing or is a rest
    if (!curr1 || !curr2 || !prev1 || !prev2 ||
        curr1->isRest() || curr2->isRest() || 
        prev1->isRest() || prev2->isRest()) {
        return true;
    }
    
    // Calculate current and previous intervals
    int currInterval = std::abs(curr1->getPitch().getMidiNote() - curr2->getPitch().getMidiNote()) % 12;
    int prevInterval = std::abs(prev1->getPitch().getMidiNote() - prev2->getPitch().getMidiNote()) % 12;
    
    // Check if current interval is dissonant
    if (music::Interval::isDissonant(currInterval)) {
        // Dissonance must be prepared by consonance
        if (music::Interval::isDissonant(prevInterval)) {
            m_violationDescription = "Dissonant interval at position " +
                                     std::to_string(noteIndex + 1) +
                                     " must be prepared by consonant interval";
            return false;
        }
        
        // At least one voice must approach dissonance by step
        int motion1 = std::abs(curr1->getPitch().getMidiNote() - prev1->getPitch().getMidiNote());
        int motion2 = std::abs(curr2->getPitch().getMidiNote() - prev2->getPitch().getMidiNote());
        
        if (!music::Interval::isStepwise(motion1) && !music::Interval::isStepwise(motion2)) {
            m_violationDescription = "Dissonant interval at position " +
                                     std::to_string(noteIndex + 1) +
                                     " must be approached by step in at least one voice";
            return false;
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

} // namespace MusicTrainer::music::rules