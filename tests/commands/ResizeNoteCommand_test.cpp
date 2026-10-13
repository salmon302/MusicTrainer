#include <gtest/gtest.h>
#include "../../src/commands/ResizeNoteCommand.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include <memory>

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Commands;

TEST(ResizeNoteCommandTest, ExecuteAndUndo) {
    Score score;
    int v = score.addVoice("V");
    Note note(Pitch(60), Position(0.0), 1.0, 100, v);
    score.addNote(note);
    ResizeNoteCommand cmd(score, note, 2.0);
    EXPECT_TRUE(cmd.execute());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);
    EXPECT_DOUBLE_EQ(score.getVoice(v)->notes[0].getDuration(), 2.0);
    EXPECT_TRUE(cmd.undo());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);
    EXPECT_DOUBLE_EQ(score.getVoice(v)->notes[0].getDuration(), 1.0);
}
