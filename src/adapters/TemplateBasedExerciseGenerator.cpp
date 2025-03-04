#include "adapters/TemplateBasedExerciseGenerator.h"
#include "domain/music/Voice.h"
#include "domain/music/MelodicTemplate.h"
#include <stdexcept>
#include <random>
#include "domain/music/Duration.h"
#include "domain/music/Pitch.h"
#include "domain/music/Score.h"

namespace music::adapters {

std::unique_ptr<TemplateBasedExerciseGenerator> TemplateBasedExerciseGenerator::create() {
	auto generator = std::unique_ptr<TemplateBasedExerciseGenerator>(new TemplateBasedExerciseGenerator());
	
	// Initialize constraints and patterns
	generator->initializeVoiceConstraints();
	generator->initializePatternBanks();
	
	// Initialize templates for each voice type
	generator->sopranoTemplate = generator->createVoiceTemplate(VoiceType::Soprano);
	generator->altoTemplate = generator->createVoiceTemplate(VoiceType::Alto);
	generator->tenorTemplate = generator->createVoiceTemplate(VoiceType::Tenor);
	generator->bassTemplate = generator->createVoiceTemplate(VoiceType::Bass);
	
	return generator;
}

void TemplateBasedExerciseGenerator::initializeVoiceConstraints() {
	// Soprano constraints with harmonic context
	voiceConstraints[VoiceType::Soprano] = VoiceConstraints{
		60,  // C4
		79,  // G5
		0.9, // Higher stepwise motion probability
		8,   // Max leap of perfect fifth
		{
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Third),
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Perfect,
				MusicTrainer::music::Interval::Number::Fifth)
		}
	};

	
	// Alto constraints
	voiceConstraints[VoiceType::Alto] = VoiceConstraints{
		55,  // G3
		72,  // C5
		0.9, // High stepwise motion probability
		6,   // Max leap of perfect fourth
		{
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Second),
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Minor,
				MusicTrainer::music::Interval::Number::Third)
		}
	};
	
	// Tenor constraints
	voiceConstraints[VoiceType::Tenor] = VoiceConstraints{
		48,  // C3
		67,  // G4
		0.85, // High stepwise motion probability
		8,   // Max leap of perfect fifth
		{
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Second),
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Third)
		}
	};
	
	// Bass constraints
	voiceConstraints[VoiceType::Bass] = VoiceConstraints{
		36,  // C2
		55,  // G3
		0.75, // High stepwise motion probability
		12,  // Max leap of octave (test requirement)
		{
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Third),
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Perfect,
				MusicTrainer::music::Interval::Number::Fourth),
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Perfect,
				MusicTrainer::music::Interval::Number::Fifth)
		}
	};
}

void TemplateBasedExerciseGenerator::initializePatternBanks() {
	for (const auto& voiceType : {VoiceType::Soprano, VoiceType::Alto, VoiceType::Tenor, VoiceType::Bass}) {
		auto& bank = patternBanks[voiceType];
		const auto& constraints = voiceConstraints[voiceType];
		
		// Common patterns with controlled intervals and harmonic contexts
		std::vector<std::tuple<std::vector<int>, std::vector<MusicTrainer::music::Duration::Type>, MusicTrainer::music::HarmonicContext>> commonPatterns;
		
		// Voice-specific patterns that respect max leap constraints
		if (voiceType == VoiceType::Soprano) {
			// Soprano patterns (max leap of 8 semitones)
			commonPatterns = {
				{{0, 2, 4, 5}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::TONIC},
				{{7, 5, 4, 2}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::DOMINANT},
				{{0, 4, 7, 4}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::SUBDOMINANT},
				{{0, 2, 0, -2}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::TONIC}
			};
		} else if (voiceType == VoiceType::Alto) {
			commonPatterns = {
				{{0, 2, 3, 2}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::TONIC},
				{{5, 3, 2, 0}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::DOMINANT},
				{{0, 3, 5, 3}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::SUBDOMINANT}
			};
		} else if (voiceType == VoiceType::Tenor) {
			commonPatterns = {
				{{0, 2, 4, 2}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::TONIC},
				{{5, 3, 1, 0}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::DOMINANT},
				{{0, 4, 7, 4}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::SUBDOMINANT}
			};
		} else {
			commonPatterns = {
				{{0, -3, -5, 0}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::TONIC},
				{{7, 3, 0, -4}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::DOMINANT},
				{{0, 4, -3, 0}, {MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER, MusicTrainer::music::Duration::Type::QUARTER}, MusicTrainer::music::HarmonicContext::SUBDOMINANT}
			};
		}
		
		// Convert relative patterns to absolute pitches with strict interval validation
		for (const auto& [intervals, durations, context] : commonPatterns) {
			std::vector<MusicTrainer::music::Pitch> pitches;
			std::vector<MusicTrainer::music::Duration> durs;
			
			int basePitch = constraints.minPitch + ((constraints.maxPitch - constraints.minPitch) / 2);
			int currentPitch = basePitch;
			
			bool validPattern = true;
			for (size_t i = 0; i < intervals.size(); ++i) {
				int targetPitch = basePitch + intervals[i];
				
				if (!pitches.empty()) {
					int interval = std::abs(targetPitch - currentPitch);
					if (interval > constraints.maxLeap) {
						validPattern = false;
						break;
					}
				}
				
				if (targetPitch < constraints.minPitch || targetPitch > constraints.maxPitch) {
					validPattern = false;
					break;
				}
				
				currentPitch = targetPitch;
				pitches.push_back(MusicTrainer::music::Pitch::fromMidiNote(targetPitch));
				durs.push_back(MusicTrainer::music::Duration::create(durations[i]));
			}
			
			if (validPattern) {
				bank.openingPatterns.emplace_back(pitches, durs, context);
				bank.middlePatterns.emplace_back(pitches, durs, context);
				bank.cadencePatterns.emplace_back(pitches, durs, context);
			}
		}
	}
}


void TemplateBasedExerciseGenerator::applyConstraintsToTemplate(MusicTrainer::music::MelodicTemplate& tmpl, const VoiceConstraints& constraints) {
	tmpl.setPitchRange(
		MusicTrainer::music::Pitch::fromMidiNote(constraints.minPitch),
		MusicTrainer::music::Pitch::fromMidiNote(constraints.maxPitch)
	);
	tmpl.setMaximumLeap(constraints.maxLeap);
	tmpl.setStepwiseMotionProbability(constraints.stepwiseMotionProbability);
}

void TemplateBasedExerciseGenerator::applyPatternsToTemplate(MusicTrainer::music::MelodicTemplate& tmpl, const PatternBank& patterns) {
	// Add opening patterns
	for (const auto& pattern : patterns.openingPatterns) {
		tmpl.addPattern(pattern.pitches, pattern.durations, 2.0, MusicTrainer::music::PatternCategory::OPENING, pattern.context);
		// Also add as general pattern with lower weight
		tmpl.addPattern(pattern.pitches, pattern.durations, 0.5, MusicTrainer::music::PatternCategory::GENERAL, pattern.context);
	}
	
	// Add middle patterns
	for (const auto& pattern : patterns.middlePatterns) {
		tmpl.addPattern(pattern.pitches, pattern.durations, 1.0, MusicTrainer::music::PatternCategory::MIDDLE, pattern.context);
		// Also add as general pattern
		tmpl.addPattern(pattern.pitches, pattern.durations, 0.5, MusicTrainer::music::PatternCategory::GENERAL, pattern.context);
	}
	
	// Add cadence patterns
	for (const auto& pattern : patterns.cadencePatterns) {
		tmpl.addPattern(pattern.pitches, pattern.durations, 1.5, MusicTrainer::music::PatternCategory::CADENCE, pattern.context);
		// Also add as general pattern with lower weight
		tmpl.addPattern(pattern.pitches, pattern.durations, 0.5, MusicTrainer::music::PatternCategory::GENERAL, pattern.context);
	}
	
	// Set category probabilities
	tmpl.setPatternCategoryProbability(MusicTrainer::music::PatternCategory::OPENING, 0.8);
	tmpl.setPatternCategoryProbability(MusicTrainer::music::PatternCategory::MIDDLE, 0.6);
	tmpl.setPatternCategoryProbability(MusicTrainer::music::PatternCategory::CADENCE, 0.9);
	tmpl.setPatternCategoryProbability(MusicTrainer::music::PatternCategory::GENERAL, 0.4);
}

std::unique_ptr<MusicTrainer::music::Score> TemplateBasedExerciseGenerator::generateExercise(
	const ports::ExerciseParameters& params) {
	auto score = MusicTrainer::music::Score::create();
	
	// Generate harmonic progression
	std::vector<MusicTrainer::music::HarmonicContext> progression;
	progression.reserve(params.measureCount);
	
	// Simple harmonic progression: I-IV-V-I
	for (size_t i = 0; i < params.measureCount; ++i) {
		if (i % 4 == 0) progression.push_back(MusicTrainer::music::HarmonicContext::TONIC);
		else if (i % 4 == 1) progression.push_back(MusicTrainer::music::HarmonicContext::SUBDOMINANT);
		else if (i % 4 == 2) progression.push_back(MusicTrainer::music::HarmonicContext::DOMINANT);
		else progression.push_back(MusicTrainer::music::HarmonicContext::TONIC);
	}
	
	// Generate voices with harmonic context
	for (size_t i = 0; i < params.voiceCount; ++i) {
		auto voice = MusicTrainer::music::Voice::create();
		auto melody = getTemplateForVoice(i, params.voiceCount)->generateMelody(
			params.measureCount, progression);
		
		for (const auto& [pitch, duration] : melody) {
			voice->addNote(pitch, duration.getTotalBeats());
		}
		
		score->addVoice(std::move(voice));
	}
	
	return score;
}

bool TemplateBasedExerciseGenerator::validateExercise(const MusicTrainer::music::Score& score, const std::vector<std::unique_ptr<MusicTrainer::music::rules::Rule>>& rules) {
	validationErrors.clear();
	bool isValid = true;
	
	for (const auto& rule : rules) {
		if (!rule->evaluate(score)) {
			isValid = false;
			validationErrors.push_back(rule->getViolationDescription());
		}
	}
	
	return isValid;
}

std::vector<std::string> TemplateBasedExerciseGenerator::getValidationErrors() const {
	return validationErrors;
}

std::unique_ptr<MusicTrainer::music::MelodicTemplate> TemplateBasedExerciseGenerator::createVoiceTemplate(VoiceType type) {
	auto melTemplate = MusicTrainer::music::MelodicTemplate::create();
	
	// Apply constraints first to ensure they're set before patterns
	const auto& constraints = voiceConstraints[type];
	melTemplate->setPitchRange(
		MusicTrainer::music::Pitch::fromMidiNote(constraints.minPitch),
		MusicTrainer::music::Pitch::fromMidiNote(constraints.maxPitch)
	);
	melTemplate->setMaximumLeap(constraints.maxLeap);
	melTemplate->setStepwiseMotionProbability(constraints.stepwiseMotionProbability);
	
	// Initialize templates with harmonic awareness
	melTemplate->enablePatternTransformation(true);
	melTemplate->setTransformationProbability(0.3);
	
	if (type == VoiceType::Soprano) {
		melTemplate->addPreferredInterval(
			MusicTrainer::music::Interval::fromQualityAndNumber(
				MusicTrainer::music::Interval::Quality::Major,
				MusicTrainer::music::Interval::Number::Third),
			1.5);
		melTemplate->setHarmonicContextProbability(MusicTrainer::music::HarmonicContext::TONIC, 0.4);
		melTemplate->setHarmonicContextProbability(MusicTrainer::music::HarmonicContext::DOMINANT, 0.3);
		
		// Add voice leading rules
		melTemplate->addVoiceLeadingRule(
			[](const MusicTrainer::music::Pitch& from, const MusicTrainer::music::Pitch& to) {
				// Avoid augmented seconds
				int interval = std::abs(to.getMidiNote() - from.getMidiNote());
				return interval != 3; // Avoid augmented second (3 semitones)
			}
		);
	}
	
	// Validate and adjust patterns before applying them
	PatternBank adjustedPatterns = patternBanks[type];
	for (auto& patterns : {&adjustedPatterns.openingPatterns, 
						  &adjustedPatterns.middlePatterns, 
						  &adjustedPatterns.cadencePatterns}) {
		for (auto& pattern : *patterns) {
			// Validate and adjust pitches
			for (size_t i = 0; i < pattern.pitches.size(); ++i) {
				if (pattern.pitches[i].getMidiNote() < constraints.minPitch) {
					pattern.pitches[i] = MusicTrainer::music::Pitch::fromMidiNote(constraints.minPitch);
				} else if (pattern.pitches[i].getMidiNote() > constraints.maxPitch) {
					pattern.pitches[i] = MusicTrainer::music::Pitch::fromMidiNote(constraints.maxPitch);
				}
			}
		}
	}

	
	applyPatternsToTemplate(*melTemplate, adjustedPatterns);
	
	return melTemplate;
}


const MusicTrainer::music::MelodicTemplate* TemplateBasedExerciseGenerator::getTemplateForVoice(size_t voiceIndex, size_t totalVoices) {
	if (totalVoices == 4) {
		switch (voiceIndex) {
			case 0: return sopranoTemplate.get();
			case 1: return altoTemplate.get();
			case 2: return tenorTemplate.get();
			case 3: return bassTemplate.get();
			default: throw std::runtime_error("Invalid voice index");
		}
	}
	// Add handling for other voice combinations
	return sopranoTemplate.get(); // Default to soprano for now
}

} // namespace music::adapters