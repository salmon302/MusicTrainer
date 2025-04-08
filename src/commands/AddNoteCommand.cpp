#include "AddNoteCommand.h"
#include "../domain/Score.h" // Need full definition to call methods
#include <string>
#include <stdexcept> // Include for std::exception
#include <iostream>  // Include for std::cerr, std::endl

namespace MusicTrainer {
namespace Commands {

AddNoteCommand::AddNoteCommand(Domain::Score& targetScore, const Domain::Note& note)
    : score(targetScore), noteToAdd(note) // Store reference and copy note value
{}

bool AddNoteCommand::execute() {
    // addNote should handle validation (e.g., voice exists) and sorting
    try {
        score.addNote(noteToAdd);
        return true; // Assume success if no exception
    } catch (const std::exception& e) {
        // Log error?
        std::cerr << "Error executing AddNoteCommand: " << e.what() << std::endl;
        return false; // Indicate failure
    }
}

bool AddNoteCommand::undo() {
    // removeNote returns true if successful
    return score.removeNote(noteToAdd);
}

std::string AddNoteCommand::getDescription() const {
    // Provide a user-friendly description
    // Could include note details if desired
    return "Add Note";
}

} // namespace Commands
} // namespace MusicTrainer