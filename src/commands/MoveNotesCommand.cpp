#include "MoveNotesCommand.h"
#include "../domain/Score.h" // Need full definition to call methods
#include <string>
#include <stdexcept> // For exceptions
#include <iostream>  // For debug/error output

namespace MusicTrainer {
namespace Commands {

MoveNotesCommand::MoveNotesCommand(Domain::Score& targetScore,
                                   std::vector<Domain::Note> notesToMove,
                                   double dt,
                                   int dp)
    : score(targetScore),
      originalNotes(std::move(notesToMove)), // Store original notes
      successfullyExecuted(false)
      // timeOffsetBeats(dt), // Store offsets if needed, but calculating movedNotes is better
      // pitchOffsetSemitones(dp)
{
    // Pre-calculate the moved notes in the constructor
    movedNotes.reserve(originalNotes.size());
    for (const auto& originalNote : originalNotes) {
        try {
            Domain::Position newPos = originalNote.getStartTime() + dt;
            // TODO: Add time snapping logic here if required before creating note
            Domain::Pitch newPitch = originalNote.getPitch().transpose(dp);
            // Create the new note with same duration, velocity, voice ID
            movedNotes.emplace_back(newPitch, newPos,
                                    originalNote.getDuration(),
                                    originalNote.getVelocity(),
                                    originalNote.getVoiceId());
        } catch (const std::out_of_range& e) {
            // Handle cases where transposition goes out of MIDI range
            std::cerr << "Warning: Note move resulted in invalid pitch for note "
                      << originalNote.getPitch().debugName() << " at beat "
                      << originalNote.getStartTime().beats << ". Skipping move for this note. Error: "
                      << e.what() << std::endl;
            // Don't add this note to movedNotes, effectively cancelling its move
        } catch (const std::exception& e) {
             std::cerr << "Error calculating moved note position/pitch: " << e.what() << std::endl;
             // Decide how to handle - skip this note? Abort command?
             // For now, skip this note's move.
        }
    }
    // If pre-calculation resulted in no notes actually being movable,
    // the command won't do anything in execute/undo.
}

bool MoveNotesCommand::execute() {
    if (originalNotes.empty() || movedNotes.empty() || originalNotes.size() != movedNotes.size()) {
         // This check handles cases where pitch transposition failed for all notes in constructor
        return false; // Nothing to do or inconsistent state
    }

    successfullyExecuted = false; // Reset flag
    bool changed = false;

    // 1. Remove original notes
    for (const auto& note : originalNotes) {
        if (score.removeNote(note)) {
            changed = true;
        } else {
             std::cerr << "Warning: Original note not found during MoveNotesCommand execute: "
                       << note.getPitch().debugName() << " at beat " << note.getStartTime().beats << std::endl;
             // This might indicate an inconsistent state if some originals were removed but others weren't found.
             // For robustness, maybe undo should add back only those successfully removed? Complex.
        }
    }

    // 2. Add moved notes
    for (const auto& note : movedNotes) {
         try {
            score.addNote(note);
            // If at least one note was added, consider the execution successful for undo purposes
            successfullyExecuted = true;
         } catch (const std::exception& e) {
             std::cerr << "Error adding moved note during MoveNotesCommand execute: " << e.what() << std::endl;
             // If adding fails, the state is inconsistent. Should we try to undo the removals?
             // This highlights the need for transactional operations or more careful state management.
             // For now, we proceed, but the state might be partially moved.
             successfullyExecuted = false; // Mark as failed if any add fails?
             return false; // Indicate overall failure?
         }
    }

    // Return true if *any* state change occurred (either removal or addition)
    // and if the process wasn't considered a failure due to add errors.
    return changed || successfullyExecuted;
}

bool MoveNotesCommand::undo() {
    if (!successfullyExecuted) {
        // If execute didn't succeed (e.g., failed adding moved notes after removing originals),
        // undo might be complex or impossible. For now, just don't undo if execute failed.
        // A more robust implementation might store exactly which notes were removed/added in execute.
        return false;
    }
     if (originalNotes.empty() || movedNotes.empty() || originalNotes.size() != movedNotes.size()) {
        return false; // Nothing to undo or inconsistent state
    }

    bool changed = false;

    // 1. Remove the moved notes
    for (const auto& note : movedNotes) {
        if (score.removeNote(note)) {
            changed = true;
        } else {
             std::cerr << "Warning: Moved note not found during MoveNotesCommand undo: "
                       << note.getPitch().debugName() << " at beat " << note.getStartTime().beats << std::endl;
        }
    }

    // 2. Add original notes back
    for (const auto& note : originalNotes) {
         try {
            score.addNote(note);
            changed = true;
         } catch (const std::exception& e) {
             std::cerr << "Error adding original note back during MoveNotesCommand undo: " << e.what() << std::endl;
             // State is inconsistent if this fails.
         }
    }

    return changed; // Return true if any state change occurred
}

std::string MoveNotesCommand::getDescription() const {
    if (originalNotes.size() == 1) {
        return "Move Note";
    } else {
        return "Move " + std::to_string(originalNotes.size()) + " Notes";
    }
}

} // namespace Commands
} // namespace MusicTrainer