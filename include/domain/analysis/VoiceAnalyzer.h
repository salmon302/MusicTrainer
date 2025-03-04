#pragma once

#include "domain/music/Voice.h"
#include "domain/music/Interval.h"
#include "domain/music/Pitch.h"
#include <vector>
#include <memory>
#include <string>

namespace MusicTrainer {
namespace music {
namespace analysis {

struct VoiceCharacteristics {
    Pitch lowestPitch;
    Pitch highestPitch;
    std::vector<Interval> commonIntervals;
    bool hasRangeIssues{false};
    bool hasLeadingIssues{false};
    std::string analysis;
    std::vector<std::string> issues;  // Added issues vector
};

class VoiceAnalyzer {
public:
    static std::unique_ptr<VoiceAnalyzer> create();  // Added create method
    
    VoiceAnalyzer();
    ~VoiceAnalyzer();

    VoiceCharacteristics analyzeVoice(const Voice& voice) const;
    
    std::vector<Interval> analyzeVoiceRelationship(const Voice& voice1, const Voice& voice2) const;
    
    // Analysis helpers
    bool hasVoiceLeadingIssue(const Voice& voice) const;
    bool hasRangeIssue(const Voice& voice) const;

private:
    class VoiceAnalyzerImpl;
    std::unique_ptr<VoiceAnalyzerImpl> m_impl;
};

} // namespace analysis
} // namespace music
} // namespace MusicTrainer