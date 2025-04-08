#ifndef MUSIC_TRAINER_COMMANDS_RESIZENOTECOMMAND_H
#define MUSIC_TRAINER_COMMANDS_RESIZENOTECOMMAND_H

#include "ICommand.h"
#include "../domain/Note.h" // Include Note definition
#include <memory> // For potential use with Score pointer

// Forward declaration
namespace MusicTrainer { namespace Domain { class Score; } }

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Command to change the duration of a single note.
 */
class ResizeNoteCommand : public ICommand {
private:
    Domain::Score& score; // Reference to the score to modify
    Domain::Note originalNote; // Store copy of the note *before* the resize
    Domain::Note resizedNote;  // Store the note with the *new* duration

    bool successfullyExecuted = false; // Track if execute was successful for undo

public:
    /**
     * @brief Constructor.
     * @param targetScore Reference to the Score object to modify.
     * @param noteToResize A copy of the Note object in its original state.
     * @param newDuration The new duration in beats.
     */
    ResizeNoteCommand(Domain::Score& targetScore,
                      const Domain::Note& noteToResize,
                      double newDuration);

    // --- ICommand Interface ---
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;

}; // class ResizeNoteCommand

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_RESIZENOTECOMMAND_H