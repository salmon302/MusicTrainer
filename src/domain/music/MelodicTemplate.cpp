#include "domain/music/MelodicTemplate.h"
#include "domain/music/Interval.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <future>

using namespace MusicTrainer;

namespace music {

MelodicTemplate::MelodicTemplate()
	: minPitch(Pitch::create(Pitch::NoteName::C, 4))
	, maxPitch(Pitch::create(Pitch::NoteName::C, 5))
{
	// Initialize default category probabilities
	categoryProbs[PatternCategory::OPENING] = 0.2;
	categoryProbs[PatternCategory::MIDDLE] = 0.6;
	categoryProbs[PatternCategory::CADENCE] = 0.2;
	categoryProbs[PatternCategory::GENERAL] = 1.0;

	// Initialize default harmonic context probabilities
	harmonicContextProbs[HarmonicContext::TONIC] = 0.4;
	harmonicContextProbs[HarmonicContext::SUBDOMINANT] = 0.2;
	harmonicContextProbs[HarmonicContext::DOMINANT] = 0.3;
	harmonicContextProbs[HarmonicContext::SECONDARY_DOMINANT] = 0.1;
}

std::unique_ptr<MelodicTemplate> MelodicTemplate::create() {
	return std::unique_ptr<MelodicTemplate>(new MelodicTemplate());
}

void MelodicTemplate::addPattern(
	const std::vector<Pitch>& pitches,
	const std::vector<Duration>& durations,
	double weight,
	PatternCategory category,
	HarmonicContext context
) {
	try {
		if (pitches.size() != durations.size()) {
			throw MusicTrainer::ValidationError("Pitch and duration vectors must have the same size");
		}
		if (weight <= 0.0) {
			throw MusicTrainer::ValidationError("Pattern weight must be positive");
		}
		
		MelodicTemplate::Pattern pattern;
		pattern.pitches = pitches;
		pattern.durations = durations;
		pattern.weight = weight;
		pattern.category = category;
		pattern.harmonicContext = context;
		
		if (validatePattern(pattern)) {
			patterns.push_back(std::move(pattern));
		}
	} catch (const MusicTrainer::ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::setPitchRange(Pitch min, Pitch max) {
	try {
		if (max < min) {
			throw MusicTrainer::ValidationError("Maximum pitch must be greater than or equal to minimum pitch");
		}
		minPitch = min;
		maxPitch = max;
	} catch (const ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::setMaximumLeap(int semitones) {
	try {
		if (semitones < 0) {
			throw MusicTrainer::ValidationError("Maximum leap must be non-negative");
		}
		maximumLeap = semitones;
	} catch (const ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::setStepwiseMotionProbability(double probability) {
	try {
		if (probability < 0.0 || probability > 1.0) {
			throw MusicTrainer::ValidationError("Probability must be between 0 and 1");
		}
		stepwiseMotionProb = probability;
	} catch (const ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::setPatternCategoryProbability(PatternCategory category, double probability) {
	try {
		if (probability < 0.0 || probability > 1.0) {
			throw ValidationError("Probability must be between 0 and 1");
		}
		categoryProbs[category] = probability;
	} catch (const ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::addPreferredInterval(const Interval& interval, double weight) {
	preferredIntervals.emplace_back(interval, weight);
}

void MelodicTemplate::setHarmonicContextProbability(music::HarmonicContext context, double probability) {
	try {
		if (probability < 0.0 || probability > 1.0) {
			throw ValidationError("Probability must be between 0 and 1");
		}
		harmonicContextProbs[context] = probability;
	} catch (const ValidationError& error) {
		HANDLE_ERROR(error);
		throw;
	}
}

void MelodicTemplate::addVoiceLeadingRule(std::function<bool(const Pitch&, const Pitch&)> rule) {
	voiceLeadingRules.push_back(rule);
}

void MelodicTemplate::enablePatternTransformation(bool enable) {
	enableTransformation = enable;
}

void MelodicTemplate::setTransformationProbability(double probability) {
	transformProb = std::clamp(probability, 0.0, 1.0);
}

bool MelodicTemplate::isValidTransition(const Pitch& from, const Pitch& to) const {
	int interval = std::abs(to.getMidiNote() - from.getMidiNote());
	
	// Hard constraint: maximum leap
	if (interval > maximumLeap) return false;
	
	// Apply stepwise motion probability
	if (interval > 2) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		if (dis(gen) > stepwiseMotionProb) return false;
	}
	
	// Apply voice leading rules
	for (const auto& rule : voiceLeadingRules) {
		if (!rule(from, to)) return false;
	}
	
	return true;
}

MelodicTemplate::Pattern MelodicTemplate::transformPattern(const MelodicTemplate::Pattern& original) const {
	MelodicTemplate::Pattern transformed = original;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> pitchDis(-2, 2); // Small pitch variations
	
	// Transform pitches while maintaining intervals
	for (auto& pitch : transformed.pitches) {
		int variation = pitchDis(gen);
		int newNote = pitch.getMidiNote() + variation;
		if (newNote >= minPitch.getMidiNote() && newNote <= maxPitch.getMidiNote()) {
			pitch = Pitch::fromMidiNote(newNote);
		}
	}
	
	return transformed;
}

bool MelodicTemplate::validatePattern(const MelodicTemplate::Pattern& pattern) const {
	// Check pitch range
	for (const auto& pitch : pattern.pitches) {
		if (pitch.getMidiNote() < minPitch.getMidiNote() || 
			pitch.getMidiNote() > maxPitch.getMidiNote()) {
			return false;
		}
	}
	
	// Skip interval validation for patterns since they are templates
	// that will be transformed and validated during melody generation
	return true;
}




MelodicTemplate::Pattern MelodicTemplate::selectPattern(PatternCategory preferredCategory, music::HarmonicContext context) const {
	if (patterns.empty()) {
		std::string errorMsg = "No patterns available for melody generation";
		MusicTrainer::ValidationError error(errorMsg);
		HANDLE_ERROR(error);
		throw error;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	
	// First try: exact context match
	std::vector<std::pair<MelodicTemplate::Pattern, double>> weightedPatterns;
	double totalWeight = 0.0;
	
	for (const auto& pattern : patterns) {
		if (pattern.harmonicContext == context) {
			double weight = pattern.weight;
			if (pattern.category == preferredCategory) {
				weight *= categoryProbs.at(preferredCategory);
			}
			weightedPatterns.emplace_back(pattern, weight);
			totalWeight += weight;
		}
	}
	
	// Fallback: if no patterns match the context, use any pattern with matching category
	if (weightedPatterns.empty()) {
		for (const auto& pattern : patterns) {
			if (pattern.category == preferredCategory) {
				double weight = pattern.weight * categoryProbs.at(preferredCategory);
				weightedPatterns.emplace_back(pattern, weight);
				totalWeight += weight;
			}
		}
	}
	
	// Final fallback: use any available pattern
	if (weightedPatterns.empty()) {
		for (const auto& pattern : patterns) {
			weightedPatterns.emplace_back(pattern, pattern.weight);
			totalWeight += pattern.weight;
		}
	}
	
	if (weightedPatterns.empty()) {
		std::string errorMsg = "No patterns available for melody generation";
		MusicTrainer::ValidationError error(errorMsg);
		HANDLE_ERROR(error);
		throw error;
	}
	
	// Select pattern based on weights
	double rand = dis(gen) * totalWeight;
	double sum = 0.0;
	for (const auto& [pattern, weight] : weightedPatterns) {
		sum += weight;
		if (rand <= sum) return pattern;
	}
	
	return weightedPatterns.front().first; // Fallback to first pattern
}

std::vector<std::pair<Pitch, Duration>> MelodicTemplate::generateMelody(
	size_t measureCount,
	const std::vector<music::HarmonicContext>& harmonicProgression) const {
	
	try {
		std::vector<std::pair<Pitch, Duration>> melody;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
	
	// Use provided harmonic progression or generate default
	std::vector<music::HarmonicContext> progression = harmonicProgression;
	if (progression.empty()) {
		progression.resize(measureCount, music::HarmonicContext::TONIC);
		for (size_t i = 1; i < measureCount - 1; ++i) {
			double rand = dis(gen);
			double sum = 0.0;
			for (const auto& [context, prob] : harmonicContextProbs) {
				sum += prob;
				if (rand <= sum) {
					progression[i] = context;
					break;
				}
			}
		}
		progression.back() = music::HarmonicContext::TONIC; // End on tonic
	}
	
	size_t currentBeat = 0;
	size_t totalBeats = measureCount * 4;
	
	while (currentBeat < totalBeats) {
		size_t currentMeasure = currentBeat / 4;
		PatternCategory category = currentMeasure == 0 ? PatternCategory::OPENING :
								 currentMeasure == measureCount - 1 ? PatternCategory::CADENCE :
								 PatternCategory::MIDDLE;
		
		MelodicTemplate::Pattern pattern = selectPattern(category, progression[currentMeasure]);
		
		// Apply pattern transformation if enabled
		if (enableTransformation && dis(gen) < transformProb) {
			pattern = transformPattern(pattern);
		}
		
		// Add pattern notes to melody
		for (size_t i = 0; i < pattern.pitches.size() && currentBeat < totalBeats; ++i) {
			Pitch candidatePitch = pattern.pitches[i];
			
			// Ensure pitch is within range and follows voice leading rules
			if (!melody.empty()) {
				if (!isValidTransition(melody.back().first, candidatePitch)) {
					int direction = (candidatePitch.getMidiNote() > melody.back().first.getMidiNote()) ? 1 : -1;
					candidatePitch = Pitch::fromMidiNote(melody.back().first.getMidiNote() + (direction * 2));
				}
			}
			
			melody.emplace_back(candidatePitch, pattern.durations[i]);
			currentBeat++;
		}
	}
	
	return melody;
} catch (const MusicTrainer::ValidationError& error) {
	HANDLE_ERROR(error);
	throw;
}
}




} // namespace music