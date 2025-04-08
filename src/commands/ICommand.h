#ifndef MUSIC_TRAINER_COMMANDS_ICOMMAND_H
#define MUSIC_TRAINER_COMMANDS_ICOMMAND_H

#include <string>

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Abstract base class for the Command pattern.
 *
 * Represents an operation that can be executed and undone.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief Executes the command.
     * @return True if execution was successful and caused a state change, false otherwise.
     */
    virtual bool execute() = 0;

    /**
     * @brief Undoes the command.
     * @return True if undo was successful and caused a state change, false otherwise.
     */
    virtual bool undo() = 0;

    /**
     * @brief Gets a human-readable description of the command (for UI).
     * @return std::string Description (e.g., "Add Note", "Delete Selection").
     */
    virtual std::string getDescription() const = 0;

    // Optional: Methods for merging commands (e.g., merging consecutive typing)
    // virtual bool mergeWith(const ICommand* other) { return false; }
    // virtual int id() const { return -1; } // ID for command merging

}; // class ICommand

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_ICOMMAND_H