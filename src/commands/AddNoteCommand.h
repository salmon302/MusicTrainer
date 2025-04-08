#ifndef MUSIC_TRAINER_COMMANDS_ADDNOTECOMMAND_H
#define MUSIC_TRAINER_COMMANDS_ADDNOTECOMMAND_H

#include "ICommand.h"
#include "../domain/Note.h" // Include Note definition
#include <memory> // For potential use with Score pointer

// Forward declaration
namespace MusicTrainer { namespace Domain { class Score; } }

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Command to add a single note to the score.
 */
class AddNoteCommand : public ICommand {
private:
    Domain::Score& score; // Reference to the score to modify
    Domain::Note noteToAdd; // Store the note value

public:
    /**
     * @brief Constructor.
     * @param targetScore Reference to the Score object to modify.
     * @param note The Note object to be added.
     */
    AddNoteCommand(Domain::Score& targetScore, const Domain::Note& note);

    // --- ICommand Interface ---
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;

}; // class AddNoteCommand

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_ADDNOTECOMMAND_H