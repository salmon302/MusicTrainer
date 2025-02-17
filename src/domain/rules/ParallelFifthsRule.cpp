#include "domain/rules/ParallelFifthsRule.h"
#include "domain/music/Interval.h"
#include <sstream>
#include <iostream>

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
		
		// Get score snapshot (this handles Voice and Score locks internally with correct ordering)
		auto snapshot = score.createSnapshot();
		
		std::cout << "[ParallelFifthsRule] Checking " << snapshot.voiceNotes.size() << " voices" << std::endl;
		if (snapshot.voiceNotes.size() < 2) {
			std::cout << "[ParallelFifthsRule] Not enough voices to check for parallel fifths" << std::endl;
			violationDescription.clear();
			return true;
		}

		// Compare each pair of voices
		for (size_t i = 0; i < snapshot.voiceNotes.size() - 1; ++i) {
			for (size_t j = i + 1; j < snapshot.voiceNotes.size(); ++j) {
				std::cout << "[ParallelFifthsRule] Checking voice pair " << i << " and " << j << std::endl;
				const auto& notes1 = snapshot.voiceNotes[i];
				const auto& notes2 = snapshot.voiceNotes[j];

				std::cout << "[ParallelFifthsRule] Voice " << i << " has " << notes1.size() << " notes" << std::endl;
				std::cout << "[ParallelFifthsRule] Voice " << j << " has " << notes2.size() << " notes" << std::endl;

				if (notes1.size() < 2 || notes2.size() < 2) continue;


				// Check consecutive notes for parallel fifths
				for (size_t k = 0; k < std::min(notes1.size(), notes2.size()) - 1; ++k) {
					Interval curr = (notes1[k].pitch.getMidiNote() > notes2[k].pitch.getMidiNote())
						? Interval::fromPitches(notes2[k].pitch, notes1[k].pitch)
						: Interval::fromPitches(notes1[k].pitch, notes2[k].pitch);

					Interval next = (notes1[k+1].pitch.getMidiNote() > notes2[k+1].pitch.getMidiNote())
						? Interval::fromPitches(notes2[k+1].pitch, notes1[k+1].pitch)
						: Interval::fromPitches(notes1[k+1].pitch, notes2[k+1].pitch);

					std::cout << "[ParallelFifthsRule] Interval pair " << k << ": "
							  << "Current=" << static_cast<int>(curr.getNumber()) << "(" << curr.getQuality() << "), "
							  << "Next=" << static_cast<int>(next.getNumber()) << "(" << next.getQuality() << ")" << std::endl;

					if (curr.getNumber() == IntervalNumber::FIFTH && 
						next.getNumber() == IntervalNumber::FIFTH &&
						curr.getQuality() == IntervalQuality::PERFECT &&
						next.getQuality() == IntervalQuality::PERFECT) {

						int motion1 = notes1[k+1].pitch.getMidiNote() - notes1[k].pitch.getMidiNote();
						int motion2 = notes2[k+1].pitch.getMidiNote() - notes2[k].pitch.getMidiNote();

						std::cout << "[ParallelFifthsRule] Found fifths, checking motion: "
								  << "Voice1=" << motion1 << ", Voice2=" << motion2 << std::endl;

						if ((motion1 > 0 && motion2 > 0) || (motion1 < 0 && motion2 < 0)) {
							std::stringstream ss;
							ss << "parallel fifths found between voice " << i << " and " << j 
							   << " at measure " << startMeasure + k;
							violationDescription = ss.str();
							std::cout << "[ParallelFifthsRule] " << violationDescription << std::endl;
							return false;
						}
					}
				}
			}
		}

		violationDescription.clear();
		std::cout << "[ParallelFifthsRule] No parallel fifths found" << std::endl;
		return true;
	} catch (const std::exception& e) {
		std::cerr << "[ParallelFifthsRule] Error during evaluation: " << e.what() << std::endl;
		violationDescription = "Error during parallel fifths check: " + std::string(e.what());
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


