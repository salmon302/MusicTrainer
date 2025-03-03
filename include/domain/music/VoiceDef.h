#pragma once

#include "domain/music/Pitch.h"
#include <string>

namespace MusicTrainer {
namespace music {

struct VoiceDef {
    std::string name;
    MusicTrainer::music::Pitch lowestAllowedPitch;
    MusicTrainer::music::Pitch highestAllowedPitch;
    
    VoiceDef() = default;
    VoiceDef(const std::string& voiceName, 
             const MusicTrainer::music::Pitch& lowest, 
             const MusicTrainer::music::Pitch& highest)
        : name(voiceName)
        , lowestAllowedPitch(lowest)
        , highestAllowedPitch(highest)
    {}
};

} // namespace music
} // namespace MusicTrainer