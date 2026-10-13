#include <gtest/gtest.h>
#include "../../src/commands/CommandHistory.h"
#include "../../src/commands/AddNoteCommand.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include <memory>

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Commands;

TEST(CommandHistoryTest, UndoRedoFlow) {
    Score score;
    int v = score.addVoice("V");
    Note note(Pitch(60), Position(0.0), 1.0, 100, v);

    CommandHistory history;
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());

    history.executeCommand(std::make_unique<AddNoteCommand>(score, note));
    EXPECT_TRUE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);

    history.undo();
    EXPECT_FALSE(history.canUndo());
    EXPECT_TRUE(history.canRedo());
    EXPECT_TRUE(score.getVoice(v)->notes.empty());

    history.redo();
    EXPECT_TRUE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);

    history.clear();
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
}

TEST(CommandHistoryTest, NewCommandClearsRedo) {
    Score score;
    int v = score.addVoice("V");
    Note n1(Pitch(60), Position(0.0), 1.0, 100, v);
    Note n2(Pitch(62), Position(1.0), 1.0, 100, v);
    CommandHistory history;
    history.executeCommand(std::make_unique<AddNoteCommand>(score, n1));
    history.undo();
    EXPECT_TRUE(history.canRedo());
    history.executeCommand(std::make_unique<AddNoteCommand>(score, n2));
    EXPECT_FALSE(history.canRedo()); // redo cleared by the new command
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u); // only n2 present
}
