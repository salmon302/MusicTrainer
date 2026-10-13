#include <gtest/gtest.h>
#include "../../src/commands/DeleteNoteCommand.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include <memory>

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Commands;

TEST(DeleteNoteCommandTest, ExecuteRemovesAndUndoRestores) {
    Score score;
    int v = score.addVoice("V");
    Note note(Pitch(60), Position(0.0), 1.0, 100, v);
    score.addNote(note);
    DeleteNoteCommand cmd(score, std::vector<Note>{note});
    EXPECT_TRUE(cmd.execute());
    EXPECT_TRUE(score.getVoice(v)->notes.empty());
    EXPECT_TRUE(cmd.undo());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);
    EXPECT_TRUE(score.getVoice(v)->notes[0] == note);
}
