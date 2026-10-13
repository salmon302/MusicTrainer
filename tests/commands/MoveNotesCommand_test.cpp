#include <gtest/gtest.h>
#include "../../src/commands/MoveNotesCommand.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include <memory>

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Commands;

TEST(MoveNotesCommandTest, ExecuteAndUndo) {
    Score score;
    int v = score.addVoice("V");
    Note note(Pitch(60), Position(0.0), 1.0, 100, v);
    score.addNote(note);
    MoveNotesCommand cmd(score, std::vector<Note>{note}, 1.0, 2);
    EXPECT_TRUE(cmd.execute());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);
    EXPECT_DOUBLE_EQ(score.getVoice(v)->notes[0].getStartTime().beats, 1.0);
    EXPECT_EQ(score.getVoice(v)->notes[0].getPitch().getMidiNoteNumber(), 62);
    EXPECT_TRUE(cmd.undo());
    ASSERT_EQ(score.getVoice(v)->notes.size(), 1u);
    EXPECT_DOUBLE_EQ(score.getVoice(v)->notes[0].getStartTime().beats, 0.0);
    EXPECT_EQ(score.getVoice(v)->notes[0].getPitch().getMidiNoteNumber(), 60);
}
