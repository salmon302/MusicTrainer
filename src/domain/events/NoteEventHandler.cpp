#include "domain/events/NoteEventHandler.h"
#include <algorithm>
#include <chrono>

namespace MusicTrainer::music::events {

std::shared_ptr<NoteEventHandler> NoteEventHandler::create() {
    return std::shared_ptr<NoteEventHandler>(new NoteEventHandler());
}

void NoteEventHandler::handleEvent(const Event& event) {
    if (const auto* noteEvent = dynamic_cast<const NoteAddedEvent*>(&event)) {
        // Store in history with a proper clone cast to NoteAddedEvent
        auto clone = std::unique_ptr<NoteAddedEvent>(
            static_cast<NoteAddedEvent*>(noteEvent->clone().release())
        );
        noteHistory.push_back(std::move(clone));
        
        // Store in correlation index if needed
        const std::string& correlationId = noteEvent->getCorrelationId();
        if (!correlationId.empty()) {
            correlatedNotes[correlationId].push_back(noteEvent);
        }
    }
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getRecentNotes(std::chrono::milliseconds window) const {
    std::vector<const NoteAddedEvent*> recentNotes;
    auto now = std::chrono::system_clock::now();
    
    for (const auto& note : noteHistory) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - note->getTimestamp()) <= window) {
            recentNotes.push_back(note.get());
        }
    }
    
    return recentNotes;
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getCorrelatedNotes(const std::string& correlationId) const {
    auto it = correlatedNotes.find(correlationId);
    return it != correlatedNotes.end() ? it->second : std::vector<const NoteAddedEvent*>();
}

} // namespace MusicTrainer::music::events

