#include <gtest/gtest.h>
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include "../../src/domain/TimeSignature.h"
#include "../../src/domain/KeySignature.h"
#include <nlohmann/json.hpp>

using namespace MusicTrainer::Domain;
using nlohmann::json;

// Regression test for the Score::Voice JSON (de)serialization bug:
// Voice (de)serialization must be provided by free to_json/from_json functions
// (found via ADL), otherwise ScoreLoader and this round-trip fail to compile.
TEST(ScoreJsonTest, VoiceAndSignatureRoundTrip) {
    Score s;
    int v0 = s.addVoice("Cantus");
    int v1 = s.addVoice("Counterpoint");
    ASSERT_NE(s.getVoice(v0), nullptr);
    ASSERT_NE(s.getVoice(v1), nullptr);

    s.addNote(Note(Pitch(60), Position(0.0), 1.0, 100, v0));
    s.addNote(Note(Pitch(64), Position(1.0), 0.5, 90, v0));
    s.addNote(Note(Pitch(67), Position(0.0), 2.0, 80, v1));
    s.addTimeSignatureChange(Position(4.0), TimeSignature(3, 4));
    s.addKeySignatureChange(Position(2.0), KeySignature(7, Mode::Major));

    // Serialize
    json j = s;

    // Deserialize
    Score back = j.get<Score>();

    EXPECT_EQ(back.getVoiceCount(), 2);
    EXPECT_EQ(back.getVoice(0)->name, std::string("Cantus"));
    EXPECT_EQ(back.getVoice(1)->name, std::string("Counterpoint"));
    ASSERT_EQ(back.getVoice(0)->notes.size(), 2u);
    ASSERT_EQ(back.getVoice(1)->notes.size(), 1u);

    EXPECT_TRUE(back.getTimeSignatureAt(Position(5.0)) == TimeSignature(3, 4));
    EXPECT_TRUE(back.getKeySignatureAt(Position(3.0)) == KeySignature(7, Mode::Major));

    // A specific note round-trips with identical fields.
    EXPECT_TRUE(back.getVoice(0)->notes[0] == Note(Pitch(60), Position(0.0), 1.0, 100, v0));
}
