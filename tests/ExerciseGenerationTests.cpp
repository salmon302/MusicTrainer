#include <gtest/gtest.h>
#include "adapters/TemplateBasedExerciseGenerator.h"
#include "domain/music/MelodicTemplate.h"
#include "domain/ports/ExerciseParameters.h"
#include <vector>

using namespace MusicTrainer::music;

TEST(MelodicTemplateTest, GeneratesMelodyWithinConstraints) {
    auto tmpl = MelodicTemplate::create();
    tmpl->setPitchRange(Pitch::create(Pitch::NoteName::C, 4, 0), Pitch::create(Pitch::NoteName::C, 5, 0));
    
    tmpl->setMaximumLeap(6);
    tmpl->setStepwiseMotionProbability(0.8);
    
    // Add opening pattern
    std::vector<Pitch> openingPitches = {
        Pitch::create(Pitch::NoteName::C, 4),
        Pitch::create(Pitch::NoteName::D, 4),
        Pitch::create(Pitch::NoteName::E, 4),
        Pitch::create(Pitch::NoteName::F, 4)
    };
    std::vector<Duration> openingDurations(4, Duration::create(Duration::Type::QUARTER));
    tmpl->addPattern(openingPitches, openingDurations, 2.0, PatternCategory::OPENING);
    
    // Add middle pattern
    std::vector<Pitch> middlePitches = {
        Pitch::create(Pitch::NoteName::E, 4),
        Pitch::create(Pitch::NoteName::F, 4),
        Pitch::create(Pitch::NoteName::G, 4)
    };
    std::vector<Duration> middleDurations(3, Duration::create(Duration::Type::QUARTER));
    tmpl->addPattern(middlePitches, middleDurations, 1.0, PatternCategory::MIDDLE);
    
    // Add cadence pattern
    std::vector<Pitch> cadencePitches = {
        Pitch::create(Pitch::NoteName::D, 4),
        Pitch::create(Pitch::NoteName::C, 4)
    };
    std::vector<Duration> cadenceDurations(2, Duration::create(Duration::Type::QUARTER));
    tmpl->addPattern(cadencePitches, cadenceDurations, 1.5, PatternCategory::CADENCE);
    
    // Add general pattern
    std::vector<Pitch> generalPitches = {
        Pitch::create(Pitch::NoteName::C, 4),
        Pitch::create(Pitch::NoteName::E, 4),
        Pitch::create(Pitch::NoteName::G, 4)
    };
    std::vector<Duration> generalDurations(3, Duration::create(Duration::Type::QUARTER));
    tmpl->addPattern(generalPitches, generalDurations, 1.0, PatternCategory::GENERAL);
    
    // Set category probabilities
    tmpl->setPatternCategoryProbability(PatternCategory::OPENING, 0.2);
    tmpl->setPatternCategoryProbability(PatternCategory::MIDDLE, 0.6);
    tmpl->setPatternCategoryProbability(PatternCategory::CADENCE, 0.2);
    tmpl->setPatternCategoryProbability(PatternCategory::GENERAL, 1.0);
    
    auto melody = tmpl->generateMelody(4);
    ASSERT_FALSE(melody.empty());
    
    // Verify melody follows constraints
    for (size_t i = 1; i < melody.size(); ++i) {
        const auto& prevPitch = melody[i-1].first;
        const auto& currentPitch = melody[i].first;
        
        int interval = std::abs(currentPitch.getMidiNote() - prevPitch.getMidiNote());
        ASSERT_LE(interval, 6);
        
        ASSERT_GE(currentPitch, tmpl->getMinPitch());
        ASSERT_LE(currentPitch, tmpl->getMaxPitch());
    }
}

TEST(ExerciseGeneratorTest, GeneratesValidExercise) {
    auto generator = music::adapters::TemplateBasedExerciseGenerator::create();
    
    music::ports::ExerciseParameters params;
    params.voiceCount = 4;
    params.measureCount = 2;
    
    auto score = generator->generateExercise(params);
    ASSERT_NE(score, nullptr);
    ASSERT_EQ(score->getVoiceCount(), 4);
    
    // Test each voice
    for (size_t i = 0; i < score->getVoiceCount(); ++i) {
        auto voice = score->getVoice(i);
        ASSERT_NE(voice, nullptr);
        const auto& notes = voice->getAllNotes();
        ASSERT_GT(notes.size(), 0);
    }
}

TEST(ExerciseGeneratorTest, GeneratesConstrainedMelodies) {
    auto generator = music::adapters::TemplateBasedExerciseGenerator::create();
    
    music::ports::ExerciseParameters params;
    params.voiceCount = 4;
    params.measureCount = 4;
    
    auto score = generator->generateExercise(params);
    ASSERT_NE(score, nullptr);
    ASSERT_EQ(score->getVoiceCount(), 4);
    
    // Test soprano voice constraints
    auto soprano = score->getVoice(0);
    ASSERT_NE(soprano, nullptr);
    
    const auto& notes = soprano->getAllNotes();
    for (size_t i = 0; i < notes.size(); ++i) {
        const auto& note = notes[i];
        ASSERT_GE(note.getPitch().getMidiNote(), 60);  // C4
        ASSERT_LE(note.getPitch().getMidiNote(), 79);  // G5
        
        if (i > 0) {
            const auto& prevNote = notes[i-1];
            int interval = std::abs(note.getPitch().getMidiNote() - prevNote.getPitch().getMidiNote());
            ASSERT_LE(interval, 8); // Max leap of perfect fifth
        }
    }
    
    // Test bass voice constraints
    auto bass = score->getVoice(3);
    ASSERT_NE(bass, nullptr);
    
    const auto& bassNotes = bass->getAllNotes();
    for (size_t i = 0; i < bassNotes.size(); ++i) {
        const auto& note = bassNotes[i];
        ASSERT_GE(note.getPitch().getMidiNote(), 36);  // C2
        ASSERT_LE(note.getPitch().getMidiNote(), 55);  // G3
        
        if (i > 0) {
            const auto& prevNote = bassNotes[i-1];
            int interval = std::abs(note.getPitch().getMidiNote() - prevNote.getPitch().getMidiNote());
            ASSERT_LE(interval, 12); // Max leap of octave
        }
    }

}

TEST(MelodicTemplateTest, GeneratesWithHarmonicContext) {
    auto tmpl = MelodicTemplate::create();
    tmpl->setPitchRange(Pitch::create(Pitch::NoteName::C, 4, 0), Pitch::create(Pitch::NoteName::C, 5, 0));
    
    std::vector<Duration> durations(3, Duration::create(Duration::Type::QUARTER));
    
    // Add patterns for all harmonic contexts
    // Tonic pattern
    std::vector<Pitch> tonicPitches = {
        Pitch::create(Pitch::NoteName::C, 4),
        Pitch::create(Pitch::NoteName::E, 4),
        Pitch::create(Pitch::NoteName::G, 4)
    };
    tmpl->addPattern(tonicPitches, durations, 1.0, PatternCategory::MIDDLE, HarmonicContext::TONIC);
    
    // Dominant pattern
    std::vector<Pitch> dominantPitches = {
        Pitch::create(Pitch::NoteName::B, 4),
        Pitch::create(Pitch::NoteName::G, 4),
        Pitch::create(Pitch::NoteName::D, 4)
    };
    tmpl->addPattern(dominantPitches, durations, 1.0, PatternCategory::MIDDLE, HarmonicContext::DOMINANT);
    
    // Subdominant pattern
    std::vector<Pitch> subdominantPitches = {
        Pitch::create(Pitch::NoteName::F, 4),
        Pitch::create(Pitch::NoteName::A, 4),
        Pitch::create(Pitch::NoteName::C, 4)
    };
    tmpl->addPattern(subdominantPitches, durations, 1.0, PatternCategory::MIDDLE, HarmonicContext::SUBDOMINANT);
    
    // Secondary dominant pattern
    std::vector<Pitch> secondaryDominantPitches = {
        Pitch::create(Pitch::NoteName::A, 4),
        Pitch::create(Pitch::NoteName::C, 4),
        Pitch::create(Pitch::NoteName::E, 4)
    };
    tmpl->addPattern(secondaryDominantPitches, durations, 1.0, PatternCategory::MIDDLE, HarmonicContext::SECONDARY_DOMINANT);
    
    // Set harmonic probabilities
    tmpl->setHarmonicContextProbability(HarmonicContext::TONIC, 0.4);
    tmpl->setHarmonicContextProbability(HarmonicContext::DOMINANT, 0.3);
    tmpl->setHarmonicContextProbability(HarmonicContext::SUBDOMINANT, 0.2);
    tmpl->setHarmonicContextProbability(HarmonicContext::SECONDARY_DOMINANT, 0.1);
    
    std::vector<HarmonicContext> progression = {
        HarmonicContext::TONIC,
        HarmonicContext::SUBDOMINANT,
        HarmonicContext::DOMINANT,
        HarmonicContext::TONIC
    };
    
    auto melody = tmpl->generateMelody(4, progression);
    ASSERT_FALSE(melody.empty());
}

TEST(MelodicTemplateTest, AppliesPatternTransformation) {
    auto tmpl = MelodicTemplate::create();
    tmpl->setPitchRange(Pitch::create(Pitch::NoteName::C, 4, 0), Pitch::create(Pitch::NoteName::C, 5, 0));
    
    // Add patterns for all necessary categories
    std::vector<Pitch> pitches = {
        Pitch::create(Pitch::NoteName::C, 4),
        Pitch::create(Pitch::NoteName::E, 4),
        Pitch::create(Pitch::NoteName::G, 4)
    };
    std::vector<Duration> durations(3, Duration::create(Duration::Type::QUARTER));
    
    // Add patterns for each category
    tmpl->addPattern(pitches, durations, 1.0, PatternCategory::OPENING);
    tmpl->addPattern(pitches, durations, 1.0, PatternCategory::MIDDLE);
    tmpl->addPattern(pitches, durations, 1.0, PatternCategory::CADENCE);
    
    // Set high probabilities for all categories
    tmpl->setPatternCategoryProbability(PatternCategory::OPENING, 1.0);
    tmpl->setPatternCategoryProbability(PatternCategory::MIDDLE, 1.0);
    tmpl->setPatternCategoryProbability(PatternCategory::CADENCE, 1.0);
    
    tmpl->enablePatternTransformation(true);
    tmpl->setTransformationProbability(1.0); // Always transform
    
    auto melody = tmpl->generateMelody(4);
    ASSERT_FALSE(melody.empty());
    
    // Verify transformation occurred by checking if notes differ from original pattern
    bool hasTransformation = false;
    for (const auto& [pitch, _] : melody) {
        if (std::find_if(pitches.begin(), pitches.end(),
            [&pitch](const Pitch& p) { return p.getMidiNote() == pitch.getMidiNote(); }) == pitches.end()) {
            hasTransformation = true;
            break;
        }
    }
    ASSERT_TRUE(hasTransformation);
}

TEST(ExerciseGeneratorTest, GeneratesWithVoiceLeadingRules) {
    auto generator = music::adapters::TemplateBasedExerciseGenerator::create();
    
    music::ports::ExerciseParameters params;
    params.voiceCount = 4;
    params.measureCount = 4;
    
    auto score = generator->generateExercise(params);
    ASSERT_NE(score, nullptr);
    
    // Test soprano voice for augmented second avoidance
    auto soprano = score->getVoice(0);
    ASSERT_NE(soprano, nullptr);
    
    const auto& notes = soprano->getAllNotes();
    for (size_t i = 1; i < notes.size(); ++i) {
        const auto& prevNote = notes[i-1];
        const auto& currNote = notes[i];
        int interval = std::abs(currNote.getPitch().getMidiNote() - prevNote.getPitch().getMidiNote());
        ASSERT_NE(interval, 3); // No augmented seconds (3 semitones)
    }
}


