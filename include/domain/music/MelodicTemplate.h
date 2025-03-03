#ifndef MUSICTRAINERV3_MELODICTEMPLATE_H
#define MUSICTRAINERV3_MELODICTEMPLATE_H

#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/music/Interval.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <functional>

namespace MusicTrainer {
namespace music {

enum class PatternCategory {
    OPENING,
    MIDDLE,
    CADENCE,
    GENERAL
};

enum class HarmonicContext {
    TONIC,
    SUBDOMINANT,
    DOMINANT,
    SECONDARY_DOMINANT
};

class MelodicTemplate {
public:
    static std::unique_ptr<MelodicTemplate> create();
    
    // Pattern management with enhanced context
    void addPattern(const std::vector<MusicTrainer::music::Pitch>& pitches, 
                   const std::vector<MusicTrainer::music::Duration>& durations,
                   double weight = 1.0,
                   PatternCategory category = PatternCategory::GENERAL,
                   HarmonicContext context = HarmonicContext::TONIC);
    
    // Enhanced melody generation with harmonic context
    std::vector<std::pair<MusicTrainer::music::Pitch, MusicTrainer::music::Duration>> generateMelody(
        size_t measureCount,
        const std::vector<HarmonicContext>& harmonicProgression = {}) const;
    
    // Existing range and basic constraint methods
    void setPitchRange(const MusicTrainer::music::Pitch& minPitch, const MusicTrainer::music::Pitch& maxPitch);
    void setMaximumLeap(int semitones);
    void setStepwiseMotionProbability(double probability);
    void setPatternCategoryProbability(PatternCategory category, double probability);
    
    // New harmonic constraint methods
    void addPreferredInterval(const MusicTrainer::music::Interval& interval, double weight = 1.0);
    void setHarmonicContextProbability(HarmonicContext context, double probability);
    void addVoiceLeadingRule(std::function<bool(const MusicTrainer::music::Pitch&, const MusicTrainer::music::Pitch&)> rule);
    
    // Pattern transformation methods
    void enablePatternTransformation(bool enable);
    void setTransformationProbability(double probability);
    
    // Getters
    const MusicTrainer::music::Pitch& getMinPitch() const { return minPitch; }
    const MusicTrainer::music::Pitch& getMaxPitch() const { return maxPitch; }
    bool isTransformationEnabled() const { return enableTransformation; }

private:
    MelodicTemplate();
    
    struct Pattern {
        std::vector<MusicTrainer::music::Pitch> pitches;
        std::vector<MusicTrainer::music::Duration> durations;
        double weight;
        PatternCategory category;
        HarmonicContext harmonicContext;
    };
    
    bool isValidTransition(const MusicTrainer::music::Pitch& from, const MusicTrainer::music::Pitch& to) const;
    Pattern selectPattern(PatternCategory preferredCategory, HarmonicContext context) const;
    Pattern transformPattern(const Pattern& original) const;
    bool validatePattern(const Pattern& pattern) const;
    
    std::vector<Pattern> patterns;
    std::vector<std::pair<MusicTrainer::music::Interval, double>> preferredIntervals;
    std::vector<std::function<bool(const MusicTrainer::music::Pitch&, const MusicTrainer::music::Pitch&)>> voiceLeadingRules;
    
    MusicTrainer::music::Pitch minPitch;
    MusicTrainer::music::Pitch maxPitch;
    
    // Melodic constraints
    int maximumLeap{12};
    double stepwiseMotionProb{0.7};
    std::map<PatternCategory, double> categoryProbs;
    std::map<HarmonicContext, double> harmonicContextProbs;
    
    // Pattern transformation
    bool enableTransformation{false};
    double transformProb{0.3};
};

} // namespace music
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_MELODICTEMPLATE_H