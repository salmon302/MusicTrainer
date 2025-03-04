#include "domain/analysis/VoiceAnalyzer.h"
#include <sstream>
#include <algorithm>

namespace MusicTrainer::music::analysis {

std::vector<Interval> getMelodicIntervals(const Voice& voice);
std::string getVoiceDescription(const VoiceCharacteristics& chars);

} // namespace MusicTrainer::music::analysis
#include <map>
#include <iostream>

namespace MusicTrainer::music::analysis {

std::unique_ptr<VoiceAnalyzer> VoiceAnalyzer::create() {
    return std::unique_ptr<VoiceAnalyzer>(new VoiceAnalyzer());
}

VoiceCharacteristics VoiceAnalyzer::analyzeVoice(const Voice& voice) const {
    VoiceCharacteristics chars;
    auto notes = voice.getAllNotes();
    
    if (notes.empty()) {
        return chars;
    }
    
    // Find range
    auto [minIt, maxIt] = std::minmax_element(notes.begin(), notes.end(),
        [](const auto& a, const auto& b) {
            return a.getPitch().getMidiNote() < b.getPitch().getMidiNote();
        });
        
    chars.lowestPitch = minIt->getPitch();
    chars.highestPitch = maxIt->getPitch();
    
    // Analyze melodic intervals
    chars.commonIntervals = getMelodicIntervals(voice);
    
    // Check for issues
    if (hasVoiceLeadingIssue(voice)) {
        chars.issues.push_back("Contains large melodic leaps");
    }
    if (hasRangeIssue(voice)) {
        chars.issues.push_back("Range exceeds recommended limits");
    }
    
    return chars;
}

std::vector<Interval> getMelodicIntervals(const Voice& voice) {
    std::vector<Interval> intervals;
    auto notes = voice.getAllNotes();
    
    // Track interval frequency
    std::map<Interval, int> intervalCounts;
    
    // Analyze consecutive notes
    for (size_t i = 0; i < notes.size() - 1; ++i) {
        auto interval = Interval::fromPitches(notes[i].getPitch(), notes[i + 1].getPitch());
        intervalCounts[interval]++;
    }
    
    // Get most common intervals (top 3)
    std::vector<std::pair<Interval, int>> sortedIntervals(
        intervalCounts.begin(), intervalCounts.end());
    std::sort(sortedIntervals.begin(), sortedIntervals.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
        
    for (size_t i = 0; i < std::min(size_t(3), sortedIntervals.size()); ++i) {
        intervals.push_back(sortedIntervals[i].first);
    }
    
    return intervals;
}

std::vector<Interval> VoiceAnalyzer::analyzeVoiceRelationship(
    const Voice& voice1, const Voice& voice2) const {
    std::vector<Interval> harmonicIntervals;
    
    auto notes1 = voice1.getAllNotes();
    auto notes2 = voice2.getAllNotes();
    
    // Find simultaneous notes and analyze their intervals
    std::map<Interval, int> intervalCounts;
    
    for (size_t i = 0; i < std::min(notes1.size(), notes2.size()); ++i) {
        auto interval = Interval::fromPitches(notes1[i].getPitch(), notes2[i].getPitch());
        intervalCounts[interval]++;
    }
    
    // Get most common harmonic intervals (top 3)
    std::vector<std::pair<Interval, int>> sortedIntervals(
        intervalCounts.begin(), intervalCounts.end());
    std::sort(sortedIntervals.begin(), sortedIntervals.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
        
    for (size_t i = 0; i < std::min(size_t(3), sortedIntervals.size()); ++i) {
        harmonicIntervals.push_back(sortedIntervals[i].first);
    }
    
    return harmonicIntervals;
}

std::string getVoiceDescription(const VoiceCharacteristics& chars) {
    std::stringstream ss;
    
    // Range description
    ss << "Range: " << chars.lowestPitch.toString() << " to " 
       << chars.highestPitch.toString() << "\n";
       
    // Common intervals
    ss << "Common intervals: ";
    for (const auto& interval : chars.commonIntervals) {
        ss << interval.toString() << " ";
    }
    ss << "\n";
    
    // Issues
    if (!chars.issues.empty()) {
        ss << "Issues:\n";
        for (const auto& issue : chars.issues) {
            ss << "- " << issue << "\n";
        }
    }
    
    return ss.str();
}

bool VoiceAnalyzer::hasVoiceLeadingIssue(const Voice& voice) const {
    auto notes = voice.getAllNotes();
    
    for (size_t i = 0; i < notes.size() - 1; ++i) {
        auto interval = Interval::fromPitches(notes[i].getPitch(), notes[i + 1].getPitch());
        // Check for leaps larger than an octave
        if (interval.getSemitones() > 12) {
            return true;
        }
    }
    
    return false;
}

bool VoiceAnalyzer::hasRangeIssue(const Voice& voice) const {
    auto notes = voice.getAllNotes();
    if (notes.empty()) return false;
    
    // Find total range
    auto [minIt, maxIt] = std::minmax_element(notes.begin(), notes.end(),
        [](const auto& a, const auto& b) {
            return a.getPitch().getMidiNote() < b.getPitch().getMidiNote();
        });
        
    // Check if range exceeds two octaves
    return (maxIt->getPitch().getMidiNote() - minIt->getPitch().getMidiNote()) > 24;
}


} // namespace MusicTrainer::music::analysis

namespace MusicTrainer::music::analysis {

class VoiceAnalyzer::VoiceAnalyzerImpl {
    public:
        VoiceAnalyzerImpl() = default;
        ~VoiceAnalyzerImpl() = default;
};

} // namespace MusicTrainer::music::analysis

MusicTrainer::music::analysis::VoiceAnalyzer::VoiceAnalyzer() = default;
MusicTrainer::music::analysis::VoiceAnalyzer::~VoiceAnalyzer() = default;