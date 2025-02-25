#pragma once

#include "domain/events/EventHandler.h"
#include "domain/events/EventBus.h"
#include "domain/events/NoteAddedEvent.h"
#include <vector>
#include <chrono>
#include <unordered_map>
#include <memory>

namespace music::events {

class NoteEventHandler : public EventHandler {
public:
	static std::shared_ptr<NoteEventHandler> create();
	
	void handle(const Event& event) override;
	bool canHandle(const std::string& eventType) const override;
	int getPriority() const override { return 10; }
	
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

} // namespace music::events

