#ifndef MUSIC_TRAINER_COMMANDS_DELETENOTECOMMAND_H
#define MUSIC_TRAINER_COMMANDS_DELETENOTECOMMAND_H

#include "ICommand.h"
#include "../domain/Note.h" // Include Note definition
#include <vector>
#include <memory> // For potential use with Score pointer

// Forward declaration
namespace MusicTrainer { namespace Domain { class Score; } }

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Command to delete one or more notes from the score.
 */
class DeleteNoteCommand : public ICommand {
private:
    Domain::Score& score; // Reference to the score to modify
    std::vector<Domain::Note> notesToDelete; // Store copies of the notes that were deleted

public:
    /**
     * @brief Constructor.
     * @param targetScore Reference to the Score object to modify.
     * @param notes A vector containing copies of the Note objects to be deleted.
     */
    DeleteNoteCommand(Domain::Score& targetScore, std::vector<Domain::Note> notes);

    // --- ICommand Interface ---
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;

}; // class DeleteNoteCommand

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_DELETENOTECOMMAND_H