// Global includes first
#include <cstddef>
#include <string>
#include <memory>
#include <sstream>
#include <cmath>

// Project includes second
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Pitch.h"
#include "domain/rules/Rule.h"
#include "domain/rules/IncrementalRule.h"

namespace MusicTrainer::music::rules {

bool isParallelMotion(int motion1, int motion2) {
    return (motion1 > 0 && motion2 > 0) || (motion1 < 0 && motion2 < 0);
}

bool ParallelFifthsRule::evaluate(const ::MusicTrainer::music::Score& score) {
return evaluateIncremental(score, 0, score.getMeasureCount());
}
class ConcreteParallelFifthsRule : public ParallelFifthsRule {
public:
    ConcreteParallelFifthsRule(const ParallelFifthsRule& other) : ParallelFifthsRule(other.getViolationDescription()) {}
    bool evaluate(const ::MusicTrainer::music::Score& score) override { return evaluateIncremental(score, 0, score.getMeasureCount()); }
    ::std::string getViolationDescription() const override { return ParallelFifthsRule::getViolationDescription(); }
    ::std::string getName() const override { return "Parallel Fifths Rule"; }
    bool evaluateIncremental(const ::MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const override { 
        return ParallelFifthsRule::evaluateIncremental(score, startMeasure, endMeasure); 
    }
};

ParallelFifthsRule::ParallelFifthsRule()
    : m_violationDescription()
{
}

ParallelFifthsRule::ParallelFifthsRule(const ::std::string& initialViolation)
    : m_violationDescription(initialViolation)
{
}

ParallelFifthsRule::~ParallelFifthsRule() = default;

Rule* ParallelFifthsRule::clone() const {
    return new ConcreteParallelFifthsRule(*this);
}

::std::string ParallelFifthsRule::getViolationDescription() const {
    return m_violationDescription;
}

::std::string ParallelFifthsRule::getName() const {
    return "Parallel Fifths Rule";
}

bool ParallelFifthsRule::evaluateIncremental(
    const ::MusicTrainer::music::Score& score,
    ::std::size_t startMeasure,
    ::std::size_t endMeasure) const
{
    if (!isEnabled()) {
        return true;
    }
    clearViolationDescription();
    if (score.getVoiceCount() < 2) {
        return true;
    }

    // Check each pair of voices
    for (::std::size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (::std::size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice* voice1 = score.getVoice(i);
            const Voice* voice2 = score.getVoice(j);
            if (!voice1 || !voice2) {
                continue;
            }

            // Get notes for the measure range in each voice
            auto notes1 = voice1->getNotesInRange(startMeasure, endMeasure);
            auto notes2 = voice2->getNotesInRange(startMeasure, endMeasure);

            // Need at least 2 notes in each voice to check for parallel motion
            if (notes1.size() < 2 || notes2.size() < 2) {
                continue;
            }

            // Check consecutive notes for parallel fifths
            for (::std::size_t k = 0; k < notes1.size() - 1 && k < notes2.size() - 1; ++k) {
                const Note& note1First = notes1[k];
                const Note& note1Second = notes1[k + 1];
                const Note& note2First = notes2[k];
                const Note& note2Second = notes2[k + 1];

                // Skip if notes don't overlap in time
                if (note1First.getPosition() != note2First.getPosition() ||
                    note1Second.getPosition() != note2Second.getPosition()) {
                    continue;
                }

                // Skip if any note is a rest
                if (note1First.isRest() || note1Second.isRest() ||
                    note2First.isRest() || note2Second.isRest()) {
                    continue;
                }

                // Calculate intervals between voices
                int interval1 = ::std::abs(note2First.getPitch().getMidiNote() -
                                       note1First.getPitch().getMidiNote()) % 12;
                int interval2 = ::std::abs(note2Second.getPitch().getMidiNote() -
                                       note1Second.getPitch().getMidiNote()) % 12;

                // Check for consecutive perfect fifths (interval of 7 semitones)
                if (interval1 == 7 && interval2 == 7) {
                    // Check if motion is parallel
                    int voice1Motion = note1Second.getPitch().getMidiNote() -
                                     note1First.getPitch().getMidiNote();
                    int voice2Motion = note2Second.getPitch().getMidiNote() -
                                     note2First.getPitch().getMidiNote();

                    if (isParallelMotion(voice1Motion, voice2Motion)) {
                        ::std::stringstream ss;
                        ss << "Parallel fifths between voice " << i + 1
                           << " and voice " << j + 1
                           << " at measure " << startMeasure + k;
                        setViolationDescription(ss.str());
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

} // namespace MusicTrainer::music::rules