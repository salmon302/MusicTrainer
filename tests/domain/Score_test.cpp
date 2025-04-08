#include <gtest/gtest.h>
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include "../../src/domain/TimeSignature.h"
#include "../../src/domain/KeySignature.h"
#include <stdexcept> // For testing exceptions

using namespace MusicTrainer::Domain;

// Test fixture for Score class
class ScoreTest : public ::testing::Test {
protected:
    Score score;
    Pitch c4, d4, e4, f4, g4;
    Position p0, p1, p2, p3, p4;
    TimeSignature ts4_4, ts3_4;
    KeySignature ksCMaj, ksGMaj;

    void SetUp() override {
        // Score is default constructed (Cmaj, 4/4 at pos 0)
        c4 = Pitch(60); d4 = Pitch(62); e4 = Pitch(64); f4 = Pitch(65); g4 = Pitch(67);
        p0 = Position(0.0); p1 = Position(1.0); p2 = Position(2.0); p3 = Position(3.0); p4 = Position(4.0);
        ts4_4 = TimeSignature(4, 4);
        ts3_4 = TimeSignature(3, 4);
        ksCMaj = KeySignature(0, Mode::Major);
        ksGMaj = KeySignature(7, Mode::Major);
    }
};

// Test case: Default Construction
TEST_F(ScoreTest, DefaultConstruction) {
    EXPECT_TRUE(score.isEmpty());
    EXPECT_EQ(score.getVoiceCount(), 0);
    // Check default signatures at position 0
    EXPECT_EQ(score.getTimeSignatureAt(p0), ts4_4);
    EXPECT_EQ(score.getKeySignatureAt(p0), ksCMaj);
    // Check default signatures slightly after position 0
    EXPECT_EQ(score.getTimeSignatureAt(Position(0.5)), ts4_4);
    EXPECT_EQ(score.getKeySignatureAt(Position(0.5)), ksCMaj);
}

// Test case: Voice Management
TEST_F(ScoreTest, VoiceManagement) {
    EXPECT_EQ(score.getVoiceCount(), 0);
    int voice0_id = score.addVoice("Soprano");
    EXPECT_EQ(voice0_id, 0);
    EXPECT_EQ(score.getVoiceCount(), 1);
    int voice1_id = score.addVoice(); // Default name
    EXPECT_EQ(voice1_id, 1);
    EXPECT_EQ(score.getVoiceCount(), 2);

    Score::Voice* v0 = score.getVoice(voice0_id);
    ASSERT_NE(v0, nullptr);
    EXPECT_EQ(v0->id, 0);
    EXPECT_EQ(v0->name, "Soprano");
    EXPECT_TRUE(v0->notes.empty());

    Score::Voice* v1 = score.getVoice(voice1_id);
    ASSERT_NE(v1, nullptr);
    EXPECT_EQ(v1->id, 1);
    EXPECT_EQ(v1->name, "");
    EXPECT_TRUE(v1->notes.empty());

    // Test getting non-existent voice
    EXPECT_EQ(score.getVoice(2), nullptr);
    EXPECT_EQ(score.getVoice(-1), nullptr);

    // Test const getter
    const Score& constScore = score;
    const Score::Voice* cv0 = constScore.getVoice(voice0_id);
     ASSERT_NE(cv0, nullptr);
     EXPECT_EQ(cv0->id, 0);
}

// Test case: Note Management (Add and Sort)
TEST_F(ScoreTest, NoteManagement) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    Note n1(c4, p1, 1.0, 100, v0); // Beat 1
    Note n3(e4, p3, 0.5, 90, v0);  // Beat 3
    Note n0(d4, p0, 1.0, 80, v0);  // Beat 0
    Note n2(f4, p2, 1.0, 110, v0); // Beat 2
    Note n_v1(g4, p1, 2.0, 70, v1); // Beat 1, voice 1

    score.addNote(n1);
    score.addNote(n3);
    score.addNote(n0);
    score.addNote(n2);
    score.addNote(n_v1);

    EXPECT_FALSE(score.isEmpty());

    // Verify notes in voice 0 are sorted by time
    const Score::Voice* voice0 = score.getVoice(v0);
    ASSERT_NE(voice0, nullptr);
    ASSERT_EQ(voice0->notes.size(), 4);
    EXPECT_EQ(voice0->notes[0], n0); // Beat 0
    EXPECT_EQ(voice0->notes[1], n1); // Beat 1
    EXPECT_EQ(voice0->notes[2], n2); // Beat 2
    EXPECT_EQ(voice0->notes[3], n3); // Beat 3

    // Verify note in voice 1
    const Score::Voice* voice1 = score.getVoice(v1);
    ASSERT_NE(voice1, nullptr);
    ASSERT_EQ(voice1->notes.size(), 1);
    EXPECT_EQ(voice1->notes[0], n_v1);

    // Test adding note to non-existent voice
    Note n_bad_voice(c4, p0, 1.0, 100, 99);
    EXPECT_THROW(score.addNote(n_bad_voice), std::runtime_error);
}

// Test case: Signature Management and Lookup
TEST_F(ScoreTest, SignatureManagement) {
    // Add changes
    score.addTimeSignatureChange(p2, ts3_4); // 3/4 starts at beat 2
    score.addKeySignatureChange(p4, ksGMaj); // G Major starts at beat 4

    // Verify lookups
    EXPECT_EQ(score.getTimeSignatureAt(p0), ts4_4);
    EXPECT_EQ(score.getTimeSignatureAt(p1), ts4_4);
    EXPECT_EQ(score.getTimeSignatureAt(Position(1.99)), ts4_4);
    EXPECT_EQ(score.getTimeSignatureAt(p2), ts3_4); // Change takes effect exactly at p2
    EXPECT_EQ(score.getTimeSignatureAt(p3), ts3_4);

    EXPECT_EQ(score.getKeySignatureAt(p0), ksCMaj);
    EXPECT_EQ(score.getKeySignatureAt(p3), ksCMaj);
    EXPECT_EQ(score.getKeySignatureAt(Position(3.99)), ksCMaj);
    EXPECT_EQ(score.getKeySignatureAt(p4), ksGMaj); // Change takes effect exactly at p4
    EXPECT_EQ(score.getKeySignatureAt(Position(5.0)), ksGMaj);

    // Test replacing a signature
    score.addTimeSignatureChange(p2, TimeSignature(2, 4)); // Change to 2/4 at beat 2
    EXPECT_EQ(score.getTimeSignatureAt(p2), TimeSignature(2, 4));
    EXPECT_EQ(score.getTimeSignatureAt(p3), TimeSignature(2, 4));
}