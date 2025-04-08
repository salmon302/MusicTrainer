#include "DeleteNoteCommand.h"
#include "../domain/Score.h" // Need full definition to call methods
#include <string>
#include <iostream> // For debug output

namespace MusicTrainer {
namespace Commands {

DeleteNoteCommand::DeleteNoteCommand(Domain::Score& targetScore, std::vector<Domain::Note> notes)
    : score(targetScore), notesToDelete(std::move(notes)) // Store reference and move the vector of note values
{}

bool DeleteNoteCommand::execute() {
    if (notesToDelete.empty()) {
        return false; // Nothing to delete
    }
    bool changed = false;
    for (const auto& note : notesToDelete) {
        // removeNote returns true if successful
        if (score.removeNote(note)) {
            changed = true;
        } else {
             // Log warning? Note might have been deleted by another command already
             std::cerr << "Warning: Note not found during DeleteNoteCommand execute: "
                       << note.getPitch().debugName() << " at beat " << note.getStartTime().beats << std::endl;
        }
    }
    return changed; // Return true if at least one note was actually removed
}

bool DeleteNoteCommand::undo() {
    if (notesToDelete.empty()) {
        return false; // Nothing to undo
    }
    bool changed = false;
     // Add the notes back (addNote handles sorting)
    for (const auto& note : notesToDelete) {
         try {
            score.addNote(note);
            changed = true; // Assume success if no exception
         } catch (const std::exception& e) {
             std::cerr << "Error during DeleteNoteCommand undo (adding note back): " << e.what() << std::endl;
             // If adding back fails, the state might be inconsistent.
             // Should we attempt to rollback other additions from this undo? Complex.
         }
    }
     return changed; // Return true if at least one note was added back
}

std::string DeleteNoteCommand::getDescription() const {
    if (notesToDelete.size() == 1) {
        return "Delete Note";
    } else {
        return "Delete " + std::to_string(notesToDelete.size()) + " Notes";
    }
}

} // namespace Commands
} // namespace MusicTrainer