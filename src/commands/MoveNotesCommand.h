#ifndef MUSIC_TRAINER_COMMANDS_MOVENOTECOMMAND_H
#define MUSIC_TRAINER_COMMANDS_MOVENOTECOMMAND_H

#include "ICommand.h"
#include "../domain/Note.h" // Include Note definition
#include "../domain/Position.h" // Include Position definition
#include <vector>
#include <memory> // For potential use with Score pointer
#include <map>

// Forward declaration
namespace MusicTrainer { namespace Domain { class Score; } }

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Command to move one or more notes by a time and pitch offset.
 */
class MoveNotesCommand : public ICommand {
private:
    Domain::Score& score; // Reference to the score to modify
    // Store original notes *before* the move
    std::vector<Domain::Note> originalNotes;
    // Store the calculated new notes *after* the move
    std::vector<Domain::Note> movedNotes;
    // Store the offsets (might not be needed if movedNotes is calculated correctly)
    // double timeOffsetBeats;
    // int pitchOffsetSemitones;

    bool successfullyExecuted = false; // Track if execute was successful for undo

public:
    /**
     * @brief Constructor.
     * @param targetScore Reference to the Score object to modify.
     * @param notesToMove Vector containing copies of the Note objects in their original state before the move.
     * @param dt The time offset in beats to apply.
     * @param dp The pitch offset in semitones to apply.
     */
    MoveNotesCommand(Domain::Score& targetScore,
                     std::vector<Domain::Note> notesToMove,
                     double dt,
                     int dp);

    // --- ICommand Interface ---
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;

}; // class MoveNotesCommand

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_MOVENOTECOMMAND_H