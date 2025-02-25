#include "domain/rules/ParallelOctavesRule.h"
#include "domain/music/Interval.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <sstream>
#include <iostream>
#include <vector>

namespace music::rules {

std::unique_ptr<ParallelOctavesRule> ParallelOctavesRule::create() {
	return std::unique_ptr<ParallelOctavesRule>(new ParallelOctavesRule());
}

bool ParallelOctavesRule::evaluate(const Score& score) const {
	return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool ParallelOctavesRule::evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const {
	try {
		size_t voiceCount = score.getVoiceCount();
		if (voiceCount < 2) {
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

				if (notes1.size() < 2 || notes2.size() < 2) continue;

				// Check consecutive notes for parallel octaves
				for (size_t k = 0; k < std::min(notes1.size(), notes2.size()) - 1; ++k) {
					Interval curr = Interval::fromPitches(notes1[k].pitch, notes2[k].pitch);
					Interval next = Interval::fromPitches(notes1[k+1].pitch, notes2[k+1].pitch);

					if (curr.getNumber() == IntervalNumber::OCTAVE && 
						next.getNumber() == IntervalNumber::OCTAVE &&
						curr.getQuality() == IntervalQuality::PERFECT &&
						next.getQuality() == IntervalQuality::PERFECT) {

						int motion1 = notes1[k+1].pitch.getMidiNote() - notes1[k].pitch.getMidiNote();
						int motion2 = notes2[k+1].pitch.getMidiNote() - notes2[k].pitch.getMidiNote();

						if ((motion1 > 0 && motion2 > 0) || (motion1 < 0 && motion2 < 0)) {
							std::stringstream ss;
							ss << "parallel octaves found between voice " << i << " and " << j 
							   << " at measure " << startMeasure + k;
							setViolationDescription(ss.str());
							return false;
						}
					}
				}
			}
		}

		clearViolationDescription();
		return true;

	} catch (const std::exception& e) {
		std::cerr << "[ParallelOctavesRule] Error during evaluation: " << e.what() << std::endl;
		setViolationDescription("Error during parallel octaves check: " + std::string(e.what()));
		return false;
	}
}

std::string ParallelOctavesRule::getViolationDescription() const {
	return violationDescription;
}

std::string ParallelOctavesRule::getName() const {
	return "Parallel Octaves Rule";
}

std::unique_ptr<Rule> ParallelOctavesRule::clone() const {
	return create();
}

} // namespace music::rules