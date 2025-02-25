#include "domain/events/NoteEventHandler.h"
#include <algorithm>

namespace music::events {

std::shared_ptr<NoteEventHandler> NoteEventHandler::create() {
	return std::shared_ptr<NoteEventHandler>(new NoteEventHandler());
}

void NoteEventHandler::handle(const Event& event) {
	if (auto noteEvent = dynamic_cast<const NoteAddedEvent*>(&event)) {
		auto newNote = std::make_unique<NoteAddedEvent>(*noteEvent);
		noteHistory.push_back(std::move(newNote));
		
		const auto& correlationId = noteEvent->getCorrelationId();
		if (!correlationId.empty()) {
			correlatedNotes[correlationId].push_back(noteHistory.back().get());
		}
	}
}

bool NoteEventHandler::canHandle(const std::string& eventType) const {
	return eventType == "NoteAddedEvent";
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getRecentNotes(std::chrono::milliseconds window) const {
	std::vector<const NoteAddedEvent*> recentNotes;
	auto now = std::chrono::system_clock::now();
	
	for (const auto& note : noteHistory) {
		if (now - note->getTimestamp() <= window) {
			recentNotes.push_back(note.get());
		}
	}
	
	return recentNotes;
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getCorrelatedNotes(const std::string& correlationId) const {
	auto it = correlatedNotes.find(correlationId);
	if (it != correlatedNotes.end()) {
		return it->second;
	}
	return {};
}

} // namespace music::events

