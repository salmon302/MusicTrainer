#pragma once

#include "domain/events/EventHandler.h"
#include "domain/events/EventBus.h"
#include "domain/events/NoteAddedEvent.h"
#include <vector>
#include <chrono>
#include <unordered_map>
#include <memory>

namespace MusicTrainer::music::events {

class NoteEventHandler : public EventHandler {
public:
    static std::shared_ptr<NoteEventHandler> create();
    
    // Implement pure virtual method from EventHandler
    void handleEvent(const Event& event) override;
    
    // Override default handler methods
    bool canHandle(const Event& event) const override {
        return dynamic_cast<const NoteAddedEvent*>(&event) != nullptr;
    }
    
    int getPriority() const override { return 10; }
    
    // Note-specific methods
    std::vector<const NoteAddedEvent*> getRecentNotes(std::chrono::milliseconds window) const;
    std::vector<const NoteAddedEvent*> getCorrelatedNotes(const std::string& correlationId) const;
    
    void clear() {
        noteHistory.clear();
        correlatedNotes.clear();
    }

private:
    NoteEventHandler() = default;
    
    std::vector<std::unique_ptr<NoteAddedEvent>> noteHistory;
    std::unordered_map<std::string, std::vector<const NoteAddedEvent*>> correlatedNotes;
};

} // namespace MusicTrainer::music::events

