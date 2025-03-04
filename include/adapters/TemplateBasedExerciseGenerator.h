#ifndef MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H
#define MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H

#include "domain/ports/ExercisePort.h"
#include "domain/music/MelodicTemplate.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <map>
#include <vector>
#include <memory>

namespace music::adapters {

enum class VoiceType {
    Soprano,
    Alto,
    Tenor,
    Bass
};

struct VoiceConstraints {
    VoiceConstraints() = default;
    
    VoiceConstraints(int min, int max, double prob, int leap,
                    std::vector<MusicTrainer::music::Interval> intervals,
                    std::map<MusicTrainer::music::HarmonicContext, double> contextProbs = {})
        : minPitch(min), maxPitch(max), stepwiseMotionProbability(prob),
          maxLeap(leap), preferredIntervals(std::move(intervals)),
          harmonicContextProbabilities(std::move(contextProbs)) {}
    
    int minPitch;
    int maxPitch;
    double stepwiseMotionProbability;
    int maxLeap;
    std::vector<MusicTrainer::music::Interval> preferredIntervals;
    std::map<MusicTrainer::music::HarmonicContext, double> harmonicContextProbabilities;
};

struct Pattern {
    std::vector<MusicTrainer::music::Pitch> pitches;
    std::vector<MusicTrainer::music::Duration> durations;
    MusicTrainer::music::HarmonicContext context;
    
    Pattern(std::vector<MusicTrainer::music::Pitch> p,
            std::vector<MusicTrainer::music::Duration> d,
            MusicTrainer::music::HarmonicContext c)
        : pitches(std::move(p)), durations(std::move(d)), context(c) {}
};

struct PatternBank {
    std::vector<Pattern> openingPatterns;
    std::vector<Pattern> middlePatterns;
    std::vector<Pattern> cadencePatterns;
};

class TemplateBasedExerciseGenerator : public music::ports::ExercisePort {
public:
    static std::unique_ptr<TemplateBasedExerciseGenerator> create();
    
    // Implementation of ExercisePort interface
    std::unique_ptr<MusicTrainer::music::Score> generateExercise(
        const music::ports::ExerciseParameters& params) override;
    bool validateExercise(const MusicTrainer::music::Score& score,
                         const std::vector<std::unique_ptr<MusicTrainer::music::rules::Rule>>& rules) override;
    std::vector<std::string> getValidationErrors() const override;

protected:
    TemplateBasedExerciseGenerator() = default;
    
    void initializeVoiceConstraints();
    void initializePatternBanks();
    void applyConstraintsToTemplate(MusicTrainer::music::MelodicTemplate& tmpl,
                                  const VoiceConstraints& constraints);
    void applyPatternsToTemplate(MusicTrainer::music::MelodicTemplate& tmpl,
                               const PatternBank& patterns);
    
    std::unique_ptr<MusicTrainer::music::MelodicTemplate> createVoiceTemplate(VoiceType type);
    const MusicTrainer::music::MelodicTemplate* getTemplateForVoice(size_t voiceIndex,
                                                                   size_t totalVoices);
    
private:
    std::map<VoiceType, VoiceConstraints> voiceConstraints;
    std::map<VoiceType, PatternBank> patternBanks;
    std::vector<std::string> validationErrors;
    
    std::unique_ptr<MusicTrainer::music::MelodicTemplate> sopranoTemplate;
    std::unique_ptr<MusicTrainer::music::MelodicTemplate> altoTemplate;
    std::unique_ptr<MusicTrainer::music::MelodicTemplate> tenorTemplate;
    std::unique_ptr<MusicTrainer::music::MelodicTemplate> bassTemplate;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_TEMPLATEBASEDEXERCISEGENERATOR_H