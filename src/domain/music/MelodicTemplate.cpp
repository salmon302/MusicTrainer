#include "domain/music/MelodicTemplate.h"
#include "domain/music/Interval.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <future>

using namespace MusicTrainer;

namespace MusicTrainer {
namespace music {

std::unique_ptr<MelodicTemplate> MelodicTemplate::create() {
    return std::unique_ptr<MelodicTemplate>(new MelodicTemplate());
}

MelodicTemplate::MelodicTemplate()
    : minPitch(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4))
    , maxPitch(MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 5)) {
    // Initialize probabilities
    categoryProbs[PatternCategory::GENERAL] = 1.0;
    categoryProbs[PatternCategory::OPENING] = 0.8;
    categoryProbs[PatternCategory::MIDDLE] = 0.6;
    categoryProbs[PatternCategory::CADENCE] = 0.4;

    harmonicContextProbs[HarmonicContext::TONIC] = 1.0;
    harmonicContextProbs[HarmonicContext::SUBDOMINANT] = 0.7;
    harmonicContextProbs[HarmonicContext::DOMINANT] = 0.8;
    harmonicContextProbs[HarmonicContext::SECONDARY_DOMINANT] = 0.5;
}

void MelodicTemplate::addPattern(
    const std::vector<MusicTrainer::music::Pitch>& pitches,
    const std::vector<MusicTrainer::music::Duration>& durations,
    double weight,
    PatternCategory category,
    HarmonicContext context) {
    
    if (pitches.empty() || durations.empty() || pitches.size() != durations.size()) {
        return;
    }

    Pattern pattern;
    pattern.pitches = pitches;
    pattern.durations = durations;
    pattern.weight = weight;
    pattern.category = category;
    pattern.harmonicContext = context;

    if (validatePattern(pattern)) {
        patterns.push_back(std::move(pattern));
    }
}

void MelodicTemplate::setPitchRange(const MusicTrainer::music::Pitch& min, const MusicTrainer::music::Pitch& max) {
    if (min.getMidiNote() <= max.getMidiNote()) {
        minPitch = min;
        maxPitch = max;
    }
}

void MelodicTemplate::setMaximumLeap(int semitones) {
    maximumLeap = semitones;
}

void MelodicTemplate::setStepwiseMotionProbability(double probability) {
    stepwiseMotionProb = std::clamp(probability, 0.0, 1.0);
}

void MelodicTemplate::setPatternCategoryProbability(PatternCategory category, double probability) {
    categoryProbs[category] = std::clamp(probability, 0.0, 1.0);
}

void MelodicTemplate::addPreferredInterval(const MusicTrainer::music::Interval& interval, double weight) {
    preferredIntervals.emplace_back(interval, weight);
}

void MelodicTemplate::setHarmonicContextProbability(HarmonicContext context, double probability) {
    harmonicContextProbs[context] = std::clamp(probability, 0.0, 1.0);
}

void MelodicTemplate::addVoiceLeadingRule(
    std::function<bool(const MusicTrainer::music::Pitch&, const MusicTrainer::music::Pitch&)> rule) {
    voiceLeadingRules.push_back(rule);
}

void MelodicTemplate::enablePatternTransformation(bool enable) {
    enableTransformation = enable;
}

void MelodicTemplate::setTransformationProbability(double probability) {
    transformProb = std::clamp(probability, 0.0, 1.0);
}

bool MelodicTemplate::isValidTransition(const MusicTrainer::music::Pitch& from, const MusicTrainer::music::Pitch& to) const {
    int interval = std::abs(to.getMidiNote() - from.getMidiNote());
    if (interval > maximumLeap) {
        return false;
    }

    for (const auto& rule : voiceLeadingRules) {
        if (!rule(from, to)) {
            return false;
        }
    }

    return true;
}

MelodicTemplate::Pattern MelodicTemplate::selectPattern(
    PatternCategory preferredCategory,
    HarmonicContext context) const {
    
    std::vector<std::pair<const Pattern*, double>> weightedPatterns;
    for (const auto& pattern : patterns) {
        double weight = pattern.weight;
        
        // Apply category probability
        weight *= categoryProbs.at(pattern.category);
        if (pattern.category == preferredCategory) {
            weight *= 1.5; // Boost preferred category
        }
        
        // Apply harmonic context probability
        if (pattern.harmonicContext == context) {
            weight *= 1.5; // Boost matching context
        }
        weight *= harmonicContextProbs.at(pattern.harmonicContext);
        
        if (weight > 0) {
            weightedPatterns.emplace_back(&pattern, weight);
        }
    }
    
    if (weightedPatterns.empty()) {
        return Pattern(); // Return empty pattern if none available
    }
    
    // Extract weights into a separate vector for discrete_distribution
    std::vector<double> weights;
    weights.reserve(weightedPatterns.size());
    for (const auto& wp : weightedPatterns) {
        weights.push_back(wp.second);
    }
    
    // Random selection based on weights
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist(weights.begin(), weights.end());
    
    Pattern selected = *weightedPatterns[dist(gen)].first;
    
    if (enableTransformation && std::bernoulli_distribution(transformProb)(gen)) {
        selected = transformPattern(selected);
    }
    
    return selected;
}

MelodicTemplate::Pattern MelodicTemplate::transformPattern(const Pattern& original) const {
    Pattern transformed = original;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (auto& pitch : transformed.pitches) {
        // Random transposition within range
        int originalNote = pitch.getMidiNote();
        int minNote = minPitch.getMidiNote();
        int maxNote = maxPitch.getMidiNote();
        
        std::uniform_int_distribution<> transposeRange(-2, 2);
        int transposition = transposeRange(gen);
        int newNote = originalNote + transposition;
        
        if (newNote >= minNote && newNote <= maxNote) {
            pitch = MusicTrainer::music::Pitch::fromMidiNote(newNote);
        }
    }
    
    return transformed;
}

bool MelodicTemplate::validatePattern(const Pattern& pattern) const {
    // Check pitch range
    for (const auto& pitch : pattern.pitches) {
        if (pitch.getMidiNote() < minPitch.getMidiNote() ||
            pitch.getMidiNote() > maxPitch.getMidiNote()) {
            return false;
        }
    }
    
    // Check intervals between consecutive notes
    for (size_t i = 1; i < pattern.pitches.size(); ++i) {
        if (!isValidTransition(pattern.pitches[i-1], pattern.pitches[i])) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::pair<MusicTrainer::music::Pitch, MusicTrainer::music::Duration>> 
MelodicTemplate::generateMelody(size_t measureCount,
                               const std::vector<HarmonicContext>& harmonicProgression) const {
    std::vector<std::pair<MusicTrainer::music::Pitch, MusicTrainer::music::Duration>> melody;
    if (patterns.empty() || measureCount == 0) {
        return melody;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    size_t currentMeasure = 0;
    while (currentMeasure < measureCount) {
        PatternCategory category;
        if (currentMeasure == 0) {
            category = PatternCategory::OPENING;
        } else if (currentMeasure == measureCount - 1) {
            category = PatternCategory::CADENCE;
        } else {
            category = PatternCategory::MIDDLE;
        }

        HarmonicContext context = HarmonicContext::TONIC;
        if (!harmonicProgression.empty()) {
            context = harmonicProgression[currentMeasure % harmonicProgression.size()];
        }

        Pattern pattern = selectPattern(category, context);
        if (pattern.pitches.empty()) continue;

        for (size_t i = 0; i < pattern.pitches.size(); ++i) {
            melody.emplace_back(pattern.pitches[i], pattern.durations[i]);
        }

        currentMeasure++;
    }

    return melody;
}

} // namespace music
} // namespace MusicTrainer