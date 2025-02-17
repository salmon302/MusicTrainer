#include "domain/rules/VoiceLeadingRule.h"
#include "domain/music/Interval.h"
#include <sstream>
#include <cstdlib>
#include <iostream>

namespace music::rules {

std::unique_ptr<VoiceLeadingRule> VoiceLeadingRule::create(int maxLeapSize) {

	return std::unique_ptr<VoiceLeadingRule>(new VoiceLeadingRule(maxLeapSize));
}

VoiceLeadingRule::VoiceLeadingRule(int maxLeapSize) : maxLeapSize(maxLeapSize) {}

bool VoiceLeadingRule::evaluate(const Score& score) const {
	return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool VoiceLeadingRule::evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const {
	try {
		// Add maximum iteration limit
		const size_t MAX_ITERATIONS = 1000;
		size_t iterationCount = 0;
		
		// Get score snapshot (this handles Voice and Score locks internally with correct ordering)
		auto snapshot = score.createSnapshot();
		const auto& voiceNotes = snapshot.voiceNotes;

		
		// Check for large leaps in each voice
		for (size_t i = 0; i < voiceNotes.size() && iterationCount < MAX_ITERATIONS; ++i) {
			const auto& notes = voiceNotes[i];
			if (notes.size() < 2) continue;
			
			// Check consecutive notes for large leaps
			for (size_t j = 0; j < notes.size() - 1 && iterationCount < MAX_ITERATIONS; ++j) {
				iterationCount++;
				int leap = std::abs(notes[j+1].pitch.getMidiNote() - notes[j].pitch.getMidiNote());
				if (leap > maxLeapSize) {
					std::stringstream ss;
					ss << "voice leading warning: large leap of " << leap 
					   << " semitones detected between " << notes[j].pitch.toString() 
					   << " and " << notes[j+1].pitch.toString()
					   << " (max allowed: " << maxLeapSize << ")";
					violationDescription = ss.str();
					return false;
				}
			}
		}
		
		// Check for voice crossing if we have at least 2 voices
		if (voiceNotes.size() >= 2) {
			for (size_t i = 0; i < voiceNotes.size() - 1 && iterationCount < MAX_ITERATIONS; ++i) {
				const auto& upperNotes = voiceNotes[i];
				const auto& lowerNotes = voiceNotes[i + 1];
				
				// Check each pair of notes for voice crossing
				for (size_t j = 0; j < std::min(upperNotes.size(), lowerNotes.size()) && iterationCount < MAX_ITERATIONS; ++j) {
					iterationCount++;
					if (upperNotes[j].pitch.getMidiNote() < lowerNotes[j].pitch.getMidiNote()) {
						std::stringstream ss;
						ss << "voice crossing detected at measure " << startMeasure 
						   << ": " << upperNotes[j].pitch.toString() 
						   << " is below " << lowerNotes[j].pitch.toString();
						violationDescription = ss.str();
						return false;
					}
				}
			}
		}
		
		// Safety exit
		if (iterationCount >= MAX_ITERATIONS) {
			violationDescription = "Maximum iteration limit reached during voice leading check";
			return false;
		}
		
		violationDescription.clear();
		return true;
	} catch (const std::exception& e) {
		std::cerr << "[VoiceLeadingRule] Error during evaluation: " << e.what() << std::endl;
		violationDescription = "Error during voice leading check: " + std::string(e.what());
		return false;
	}
}

std::string VoiceLeadingRule::getViolationDescription() const {
	return violationDescription;
}


std::string VoiceLeadingRule::getName() const {
	return "Voice Leading Rule";
}

std::unique_ptr<Rule> VoiceLeadingRule::clone() const {
	return create(maxLeapSize);
}

} // namespace music::rules
