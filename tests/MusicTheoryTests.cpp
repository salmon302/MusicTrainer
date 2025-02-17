#include <gtest/gtest.h>
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/music/Interval.h"

using namespace music;

TEST(PitchTests, MidiNoteConversion) {
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	EXPECT_EQ(c4.getMidiNote(), 60);
	
	auto fSharp4 = Pitch::create(Pitch::NoteName::F, 4, 1);
	EXPECT_EQ(fSharp4.getMidiNote(), 66);
}

TEST(PitchTests, StringRepresentation) {
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	EXPECT_EQ(c4.toString(), "C4");
	
	auto fSharp4 = Pitch::create(Pitch::NoteName::F, 4, 1);
	EXPECT_EQ(fSharp4.toString(), "F#4");
}

TEST(DurationTests, TotalBeats) {
	auto whole = Duration::createWhole();
	EXPECT_DOUBLE_EQ(whole.getTotalBeats(), 1.0);
	
	auto dottedHalf = Duration::create(Duration::Type::HALF, 1);
	EXPECT_DOUBLE_EQ(dottedHalf.getTotalBeats(), 3.0);
}

TEST(DurationTests, Comparison) {
	auto quarter = Duration::createQuarter();
	auto half = Duration::createHalf();
	EXPECT_TRUE(quarter < half);
	EXPECT_FALSE(quarter > half);
}

TEST(IntervalTests, SemitoneCalculation) {
	auto perfect5th = Interval::create(IntervalNumber::FIFTH, IntervalQuality::PERFECT);
	EXPECT_EQ(perfect5th.getSemitones(), 7);
	
	auto major3rd = Interval::create(IntervalNumber::THIRD, IntervalQuality::MAJOR);
	EXPECT_EQ(major3rd.getSemitones(), 4);
}

TEST(IntervalTests, FromPitches) {
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto g4 = Pitch::create(Pitch::NoteName::G, 4);
	auto interval = Interval::fromPitches(c4, g4);
	
	EXPECT_EQ(interval.getNumber(), IntervalNumber::FIFTH);
	EXPECT_EQ(interval.getQuality(), IntervalQuality::PERFECT);
}