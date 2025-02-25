#include "domain/rules/ParallelFifthsRule.h"
#include "domain/music/Interval.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <sstream>
#include <iostream>
#include <vector>

namespace music::rules {

std::unique_ptr<ParallelFifthsRule> ParallelFifthsRule::create() {
	return std::unique_ptr<ParallelFifthsRule>(new ParallelFifthsRule());
}

bool ParallelFifthsRule::evaluate(const Score& score) const {
	return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool ParallelFifthsRule::evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const {
	try {
		std::cout << "[ParallelFifthsRule] Starting evaluation from measure " << startMeasure << " to " << endMeasure << std::endl;
		
		size_t voiceCount = score.getVoiceCount();
		if (voiceCount < 2) {
			std::cout << "[ParallelFifthsRule] Not enough voices to check for parallel fifths" << std::endl;
			clearViolationDescription();
			return true;
		}

		// Compare each pair of voices
		for (size_t i = 0; i < voiceCount - 1; ++i) {
			for (size_t j = i + 1; j < voiceCount; ++j) {
				const Voice* voice1 = score.getVoice(i);
				const Voice* voice2 = score.getVoice(j);
				if (!voice1 || !voice2) continue;
				
				auto notes1 = voice1->getNotesInRange(startMeasure, endMeasure);
				auto notes2 = voice2->getNotesInRange(startMeasure, endMeasure);

				std::cout << "[ParallelFifthsRule] Checking voice pair " << i << " and " << j << std::endl;
				std::cout << "[ParallelFifthsRule] Voice " << i << " has " << notes1.size() << " notes" << std::endl;
				std::cout << "[ParallelFifthsRule] Voice " << j << " has " << notes2.size() << " notes" << std::endl;

				if (notes1.size() < 2 || notes2.size() < 2) continue;

				// Check consecutive notes for parallel fifths
				for (size_t k = 0; k < std::min(notes1.size(), notes2.size()) - 1; ++k) {
					// Check motion direction first
					int motion1 = notes1[k+1].pitch.getMidiNote() - notes1[k].pitch.getMidiNote();
					int motion2 = notes2[k+1].pitch.getMidiNote() - notes2[k].pitch.getMidiNote();

					std::cout << "[ParallelFifthsRule] Motion analysis:" << std::endl;
					std::cout << "  Voice 1: " << notes1[k].pitch.toString() << "(" << notes1[k].pitch.getMidiNote()
							 << ") -> " << notes1[k+1].pitch.toString() << "(" << notes1[k+1].pitch.getMidiNote()
							 << ") Motion: " << motion1 << std::endl;
					std::cout << "  Voice 2: " << notes2[k].pitch.toString() << "(" << notes2[k].pitch.getMidiNote()
							 << ") -> " << notes2[k+1].pitch.toString() << "(" << notes2[k+1].pitch.getMidiNote()
							 << ") Motion: " << motion2 << std::endl;

					// Only check intervals if moving in parallel motion
					if ((motion1 > 0 && motion2 > 0) || (motion1 < 0 && motion2 < 0)) {
						std::cout << "[ParallelFifthsRule] Detected parallel motion, checking intervals" << std::endl;
						// Calculate intervals from lower to higher voice
						// Current interval
						Interval curr = (notes1[k].pitch.getMidiNote() < notes2[k].pitch.getMidiNote())
							? Interval::fromPitches(notes1[k].pitch, notes2[k].pitch)
							: Interval::fromPitches(notes2[k].pitch, notes1[k].pitch);
						
						// Next interval
						Interval next = (notes1[k+1].pitch.getMidiNote() < notes2[k+1].pitch.getMidiNote())
							? Interval::fromPitches(notes1[k+1].pitch, notes2[k+1].pitch)
							: Interval::fromPitches(notes2[k+1].pitch, notes1[k+1].pitch);


						// Check for perfect fifths
						std::cout << "[ParallelFifthsRule] Interval analysis:" << std::endl;
						std::cout << "  Current interval: " << curr.toString()
								 << " (Number=" << static_cast<int>(curr.getNumber())
								 << " Quality=" << static_cast<int>(curr.getQuality()) << ")" << std::endl;
						std::cout << "  Next interval: " << next.toString()
								 << " (Number=" << static_cast<int>(next.getNumber())
								 << " Quality=" << static_cast<int>(next.getQuality()) << ")" << std::endl;

						if (curr.getNumber() == IntervalNumber::FIFTH &&
							next.getNumber() == IntervalNumber::FIFTH &&
							curr.getQuality() == IntervalQuality::PERFECT &&
							next.getQuality() == IntervalQuality::PERFECT) {
							
							std::cout << "[ParallelFifthsRule] Found parallel perfect fifths!" << std::endl;
							
							std::stringstream ss;
							ss << "Parallel fifths found between voice " << i << " and " << j
							   << " at measure " << startMeasure + k;
							setViolationDescription(ss.str());
							std::cout << "[ParallelFifthsRule] Setting violation: " << ss.str() << std::endl;
							return false;  // Return false to indicate rule violation
						}
					}
				}
			}
		}

		clearViolationDescription();
		return true;

	} catch (const std::exception& e) {
		std::cerr << "[ParallelFifthsRule] Error during evaluation: " << e.what() << std::endl;
		setViolationDescription("Error during parallel fifths check: " + std::string(e.what()));
		return false;
	}
}

std::string ParallelFifthsRule::getViolationDescription() const {
	return violationDescription;
}

std::string ParallelFifthsRule::getName() const {
	return "Parallel Fifths Rule";
}

std::unique_ptr<Rule> ParallelFifthsRule::clone() const {
	return create();
}

} // namespace music::rules