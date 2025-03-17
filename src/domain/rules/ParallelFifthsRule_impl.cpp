#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Pitch.h"

#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/Rule.h"
#include "domain/rules/IncrementalRule.h"

#include <sstream>
#include <cstdlib>

namespace MusicTrainer::music::rules {

namespace {
bool isParallelMotion(int motion1, int motion2) {
    return (motion1 > 0 && motion2 > 0) || (motion1 < 0 && motion2 < 0);
}
} // namespace

bool ParallelFifthsRule::evaluateIncremental(
    const Score& score,
    ::std::size_t startMeasure,
    ::std::size_t endMeasure) const 
{
    if (!this->isEnabled()) {
        return true;
    }

    const_cast<ParallelFifthsRule*>(this)->clearViolationDescription();

    if (score.getVoiceCount() < 2) {
        return true;
    }

    for (::std::size_t i = 0; i < score.getVoiceCount() - 1; ++i) {
        for (::std::size_t j = i + 1; j < score.getVoiceCount(); ++j) {
            const Voice* voice1 = score.getVoice(i);
            const Voice* voice2 = score.getVoice(j);

            if (!voice1 || !voice2) {
                continue;
            }

            const auto& notes1 = voice1->getAllNotes();
            const auto& notes2 = voice2->getAllNotes();

            if (notes1.size() < 2 || notes2.size() < 2) {
                continue;
            }

            for (::std::size_t pos = 0; pos < notes1.size() - 1 && pos < notes2.size() - 1; ++pos) {
                const Note& note1First = notes1[pos];
                const Note& note1Second = notes1[pos + 1];
                const Note& note2First = notes2[pos];
                const Note& note2Second = notes2[pos + 1];

                const Pitch& pitch1First = note1First.getPitch();
                const Pitch& pitch1Second = note1Second.getPitch();
                const Pitch& pitch2First = note2First.getPitch();
                const Pitch& pitch2Second = note2Second.getPitch();

                int interval1 = ::std::abs(pitch2First.getMidiNote() - pitch1First.getMidiNote()) % 12;
                int interval2 = ::std::abs(pitch2Second.getMidiNote() - pitch1Second.getMidiNote()) % 12;

                if ((interval1 == 7 && interval2 == 7) || (interval1 == 0 && interval2 == 0)) {
                    int voice1Motion = pitch1Second.getMidiNote() - pitch1First.getMidiNote();
                    int voice2Motion = pitch2Second.getMidiNote() - pitch2First.getMidiNote();

                    if (isParallelMotion(voice1Motion, voice2Motion)) {
                        ::std::ostringstream ss;
                        ss << "Parallel " << (interval1 == 7 ? "fifths" : "octaves")
                           << " between voice " << i + 1 
                           << " and voice " << j + 1
                           << " at position " << pos + 1;
                            
                        const_cast<ParallelFifthsRule*>(this)->setViolationDescription(ss.str());
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace MusicTrainer::music::rules