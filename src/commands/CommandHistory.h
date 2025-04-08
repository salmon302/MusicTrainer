#ifndef MUSIC_TRAINER_COMMANDS_COMMANDHISTORY_H
#define MUSIC_TRAINER_COMMANDS_COMMANDHISTORY_H

#include "ICommand.h"
#include <vector>
#include <memory> // For std::shared_ptr or std::unique_ptr
#include <stack>

namespace MusicTrainer {
namespace Commands {

/**
 * @brief Manages the execution, undoing, and redoing of commands.
 *
 * Maintains undo and redo stacks for command objects.
 */
class CommandHistory {
private:
    // Using unique_ptr as commands are typically owned solely by the history
    std::stack<std::unique_ptr<ICommand>> undoStack;
    std::stack<std::unique_ptr<ICommand>> redoStack;

    // Optional: Limit the size of the stacks
    size_t maxHistorySize = 100; // Example limit

    // Helper to clear the redo stack (called when a new command is executed)
    void clearRedoStack();

    // Helper to manage stack size limits
    void limitUndoStackSize();

public:
    CommandHistory() = default;

    /**
     * @brief Executes a new command and adds it to the undo stack.
     * Clears the redo stack.
     * @param command A unique pointer to the command to execute.
     */
    void executeCommand(std::unique_ptr<ICommand> command);

    /**
     * @brief Undoes the most recent command from the undo stack.
     * Moves the undone command to the redo stack.
     */
    void undo();

    /**
     * @brief Redoes the most recent command from the redo stack.
     * Moves the redone command back to the undo stack.
     */
    void redo();

    /**
     * @brief Checks if there are any commands available to undo.
     * @return True if undo is possible, false otherwise.
     */
    bool canUndo() const;

    /**
     * @brief Checks if there are any commands available to redo.
     * @return True if redo is possible, false otherwise.
     */
    bool canRedo() const;

    /**
     * @brief Clears both the undo and redo stacks.
     * Typically used when loading a new document/score.
     */
    void clear();

    /**
     * @brief Gets the description of the next command to be undone (for UI).
     * @return std::string Description, or empty string if no command to undo.
     */
    std::string getUndoText() const;

    /**
     * @brief Gets the description of the next command to be redone (for UI).
     * @return std::string Description, or empty string if no command to redo.
     */
    std::string getRedoText() const;

}; // class CommandHistory

} // namespace Commands
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_COMMANDS_COMMANDHISTORY_H