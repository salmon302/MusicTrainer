#include "CommandHistory.h"
#include <iostream> // For debug output

namespace MusicTrainer {
namespace Commands {

void CommandHistory::clearRedoStack() {
    while (!redoStack.empty()) {
        redoStack.pop();
    }
    // TODO: Emit signal indicating redo availability changed (if using signals/slots)
}

void CommandHistory::limitUndoStackSize() {
    while (undoStack.size() > maxHistorySize) {
        // To limit the stack size, we need to remove elements from the bottom.
        // std::stack doesn't provide direct access to the bottom.
        // A common workaround is to transfer elements to a temporary stack/vector,
        // remove the bottom, and transfer back. Or use std::deque instead of std::stack.

        // Simple approach for now: Just warn and don't actually limit if using std::stack
        // Or, if using std::deque: undoStack.pop_front();
        std::cerr << "Warning: Undo history limit reached (max=" << maxHistorySize << "). Oldest command lost." << std::endl;
        // This requires changing std::stack to std::deque to actually remove from bottom.
        // For now, the stack will just grow indefinitely until std::deque is used.
        break; // Prevent infinite loop if pop_front isn't available
    }
}

void CommandHistory::executeCommand(std::unique_ptr<ICommand> command) {
    if (!command) {
        return;
    }

    // Execute the command
    if (command->execute()) { // Only add to history if execution was successful/meaningful
        // Clear the redo stack, as a new action invalidates the previous redo path
        clearRedoStack();

        // Add the command to the undo stack
        undoStack.push(std::move(command));

        // Ensure the undo stack doesn't exceed the size limit
        limitUndoStackSize();

        // TODO: Emit signal indicating undo/redo availability changed
         std::cout << "Command executed: " << undoStack.top()->getDescription() << std::endl; // Debug
    } else {
         std::cout << "Command execution failed or no state change: " << command->getDescription() << std::endl; // Debug
        // Command failed or did nothing, don't add to history
    }
}

void CommandHistory::undo() {
    if (!canUndo()) {
        return;
    }

    // Get the top command from the undo stack
    std::unique_ptr<ICommand> commandToUndo = std::move(undoStack.top());
    undoStack.pop();

    // Attempt to undo it
    if (commandToUndo->undo()) { // Only move to redo if undo was successful
        // Move the successfully undone command to the redo stack
        redoStack.push(std::move(commandToUndo));
        // TODO: Emit signal indicating undo/redo availability changed
         std::cout << "Command undone: " << redoStack.top()->getDescription() << std::endl; // Debug
    } else {
         std::cout << "Command undo failed: " << commandToUndo->getDescription() << std::endl; // Debug
        // Undo failed, discard the command (don't put on redo stack)
        // Optionally, try to push it back onto undo stack? Depends on desired behavior.
    }
}

void CommandHistory::redo() {
    if (!canRedo()) {
        return;
    }

    // Get the top command from the redo stack
    std::unique_ptr<ICommand> commandToRedo = std::move(redoStack.top());
    redoStack.pop();

    // Attempt to re-execute it
    if (commandToRedo->execute()) { // Only move back to undo if redo was successful
        // Move the successfully redone command back to the undo stack
        undoStack.push(std::move(commandToRedo));
        // TODO: Emit signal indicating undo/redo availability changed
         std::cout << "Command redone: " << undoStack.top()->getDescription() << std::endl; // Debug
    } else {
         std::cout << "Command redo failed: " << commandToRedo->getDescription() << std::endl; // Debug
        // Redo failed, discard the command (don't put back on undo stack)
        // Optionally, try to push it back onto redo stack? Depends on desired behavior.
    }
}

bool CommandHistory::canUndo() const {
    return !undoStack.empty();
}

bool CommandHistory::canRedo() const {
    return !redoStack.empty();
}

void CommandHistory::clear() {
    while (!undoStack.empty()) {
        undoStack.pop();
    }
    clearRedoStack(); // Also clears redo stack
    // TODO: Emit signal indicating undo/redo availability changed
     std::cout << "Command history cleared." << std::endl; // Debug
}

std::string CommandHistory::getUndoText() const {
    if (canUndo()) {
        return undoStack.top()->getDescription();
    }
    return "";
}

std::string CommandHistory::getRedoText() const {
    if (canRedo()) {
        return redoStack.top()->getDescription();
    }
    return "";
}


} // namespace Commands
} // namespace MusicTrainer