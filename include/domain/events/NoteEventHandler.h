#pragma once

#include "domain/events/EventHandler.h"
#include "domain/events/EventBus.h"
#include "domain/events/NoteAddedEvent.h"
#include <vector>
#include <chrono>
#include <unordered_map>
#include <shared_mutex>

namespace music::events {

class NoteEventHandler : public EventHandler {
public:
	static std::shared_ptr<NoteEventHandler> create();
	
	void handle(const Event& event) override;
	bool canHandle(const std::string& eventType) const override;
	int getPriority() const override { return 10; }  // High priority for note events
	
	// Get all notes added within a time window
	std::vector<const NoteAddedEvent*> getRecentNotes(std::chrono::milliseconds window) const;
	
	// Get correlated notes by ID
	std::vector<const NoteAddedEvent*> getCorrelatedNotes(const std::string& correlationId) const;
	
	// Clear event history
	void clear();

private:
	NoteEventHandler() = default;
	std::vector<std::unique_ptr<NoteAddedEvent>> noteHistory;
	std::unordered_map<std::string, std::vector<const NoteAddedEvent*>> correlatedNotes;
	mutable std::shared_mutex mutex_;  // Mutex for thread safety
};

} // namespace music::events
