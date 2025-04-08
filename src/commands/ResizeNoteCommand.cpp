#include "ResizeNoteCommand.h"
#include "../domain/Score.h" // Need full definition to call methods
#include <string>
#include <stdexcept> // For exceptions
#include <iostream>  // For debug/error output

namespace MusicTrainer {
namespace Commands {

ResizeNoteCommand::ResizeNoteCommand(Domain::Score& targetScore,
                                     const Domain::Note& noteToResize,
                                     double newDuration)
    : score(targetScore),
      originalNote(noteToResize), // Store copy of original note
      // Initialize resizedNote directly using original note's properties and new duration
      resizedNote(originalNote.getPitch(),
                  originalNote.getStartTime(),
                  // Validate duration before passing to Note constructor
                  (newDuration <= 0 ? throw std::invalid_argument("ResizeNoteCommand: New duration must be positive.") : newDuration),
                  originalNote.getVelocity(),
                  originalNote.getVoiceId()),
      successfullyExecuted(false)
{
    // Constructor body is now empty as initialization is done above
}

bool ResizeNoteCommand::execute() {
    // Don't execute if duration didn't actually change
    // Use tolerance for float comparison
    if (std::fabs(originalNote.getDuration() - resizedNote.getDuration()) < 1e-9) {
        return false;
    }

    successfullyExecuted = false; // Reset flag

    // 1. Remove original note
    if (!score.removeNote(originalNote)) {
        std::cerr << "Warning: Original note not found during ResizeNoteCommand execute: "
                  << originalNote.getPitch().debugName() << " at beat " << originalNote.getStartTime().beats << std::endl;
        return false; // Cannot proceed if original note isn't there
    }

    // 2. Add resized note
    try {
        score.addNote(resizedNote);
        successfullyExecuted = true; // Mark as successful only if add succeeds
        std::cout << "ResizeNoteCommand executed." << std::endl; // Debug
        return true; // State changed
    } catch (const std::exception& e) {
        std::cerr << "Error adding resized note during ResizeNoteCommand execute: " << e.what() << std::endl;
        // Attempt to rollback by adding original note back
        try {
             score.addNote(originalNote); // Put original back
             std::cerr << "Rollback attempted: Re-added original note after resize failure." << std::endl;
        } catch (const std::exception& e2) {
             std::cerr << "CRITICAL ERROR: Failed to add original note back after resize failure. Score state inconsistent. Error: " << e2.what() << std::endl;
             // Application state is now potentially corrupt
        }
        successfullyExecuted = false;
        return false; // Indicate overall failure
    }
}

bool ResizeNoteCommand::undo() {
    if (!successfullyExecuted) {
        // If execute didn't succeed, don't attempt undo
        return false;
    }
     // Don't undo if duration didn't actually change
    if (std::fabs(originalNote.getDuration() - resizedNote.getDuration()) < 1e-9) {
        return false;
    }

    bool changed = false;

    // 1. Remove the resized note
    if (score.removeNote(resizedNote)) {
        changed = true;
    } else {
        std::cerr << "Warning: Resized note not found during ResizeNoteCommand undo: "
                  << resizedNote.getPitch().debugName() << " at beat " << resizedNote.getStartTime().beats << std::endl;
        // Continue anyway to try and add original back
    }

    // 2. Add original note back
    try {
        score.addNote(originalNote);
        changed = true;
        std::cout << "ResizeNoteCommand undone." << std::endl; // Debug
    } catch (const std::exception& e) {
        std::cerr << "Error adding original note back during ResizeNoteCommand undo: " << e.what() << std::endl;
        // State is inconsistent if this fails.
        changed = false; // Mark undo as failed if re-add fails?
    }

    return changed; // Return true if any state change occurred
}

std::string ResizeNoteCommand::getDescription() const {
    return "Resize Note";
}

} // namespace Commands
} // namespace MusicTrainer