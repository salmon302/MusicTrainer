#include "domain/rules/ParallelOctavesRule.h"
#include "domain/music/Interval.h"
#include <sstream>
#include <iostream>

namespace music::rules {

std::unique_ptr<ParallelOctavesRule> ParallelOctavesRule::create() {

	return std::unique_ptr<ParallelOctavesRule>(new ParallelOctavesRule());
}

bool ParallelOctavesRule::evaluate(const Score& score) const {
	return evaluateIncremental(score, 0, score.getMeasureCount());
}

bool ParallelOctavesRule::evaluateIncremental(const Score& score, size_t startMeasure, size_t endMeasure) const {
	try {
		// Get score snapshot with shorter timeout
		Score::ScoreSnapshot snapshot;
		{
			snapshot = score.createSnapshot();
		}
		
		// Process snapshot data without holding any locks
		if (snapshot.voiceNotes.size() < 2) {
			clearViolationDescription();
			return true;
		}

		// Compare voices without holding locks
		for (size_t i = 0; i < snapshot.voiceNotes.size() - 1; ++i) {
			for (size_t j = i + 1; j < snapshot.voiceNotes.size(); ++j) {
				const auto& notes1 = snapshot.voiceNotes[i];
				const auto& notes2 = snapshot.voiceNotes[j];

				if (notes1.size() < 2 || notes2.size() < 2) continue;

				// Check consecutive notes
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
	::utils::TrackedSharedMutexLock lock(mutex_, "ParallelOctavesRule::mutex_", ::utils::LockLevel::VALIDATION);
	return violationDescription;
}

void ParallelOctavesRule::setViolationDescription(const std::string& desc) const {
	::utils::TrackedUniqueLock lock(mutex_, "ParallelOctavesRule::mutex_", ::utils::LockLevel::VALIDATION);
	violationDescription = desc;
}

void ParallelOctavesRule::clearViolationDescription() const {
	::utils::TrackedUniqueLock lock(mutex_, "ParallelOctavesRule::mutex_", ::utils::LockLevel::VALIDATION);
	violationDescription.clear();
}

std::string ParallelOctavesRule::getName() const {
	return "Parallel Octaves Rule";
}

std::unique_ptr<Rule> ParallelOctavesRule::clone() const {
	return create();
}

} // namespace music::rules

