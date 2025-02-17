#include "domain/events/NoteEventHandler.h"
#include "utils/TrackedLock.h"
#include <algorithm>
#include <string>
#include <locale>
#include <ios>
#include <iostream>
#include <sstream>

namespace music::events {

std::shared_ptr<NoteEventHandler> NoteEventHandler::create() {
	return std::shared_ptr<NoteEventHandler>(new NoteEventHandler());
}

void NoteEventHandler::handle(const Event& event) {
	if (const auto* noteEvent = dynamic_cast<const NoteAddedEvent*>(&event)) {
		::utils::TrackedUniqueLock lock(mutex_, "NoteEventHandler::mutex_", ::utils::LockLevel::REPOSITORY);
		auto clonedEvent = noteEvent->clone();
		noteHistory.push_back(std::unique_ptr<NoteAddedEvent>(
			static_cast<NoteAddedEvent*>(clonedEvent.release())));
			
		// Add to correlation map if correlation ID exists
		const auto& correlationId = noteEvent->getCorrelationId();
		if (!correlationId.empty()) {
			correlatedNotes[correlationId].push_back(noteHistory.back().get());
		}
	}
}


bool NoteEventHandler::canHandle(const std::string& eventType) const {
	return eventType == NoteAddedEvent::TYPE;
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getRecentNotes(std::chrono::milliseconds window) const {
	::utils::TrackedUniqueLock lock(mutex_, "NoteEventHandler::mutex_", ::utils::LockLevel::REPOSITORY);
	std::vector<const NoteAddedEvent*> result;
	auto now = std::chrono::system_clock::now();
	
	for (const auto& note : noteHistory) {
		if (now - note->getTimestamp() <= window) {
			result.push_back(note.get());
		}
	}
	
	// Sort by timestamp (oldest first)
	std::sort(result.begin(), result.end(),
		[](const NoteAddedEvent* a, const NoteAddedEvent* b) {
			return a->getTimestamp() < b->getTimestamp();
		});
	
	return result;
}

std::vector<const NoteAddedEvent*> NoteEventHandler::getCorrelatedNotes(const std::string& correlationId) const {
	::utils::TrackedUniqueLock lock(mutex_, "NoteEventHandler::mutex_", ::utils::LockLevel::REPOSITORY);
	auto it = correlatedNotes.find(correlationId);
	if (it != correlatedNotes.end()) {
		return it->second;
	}
	return {};
}

void NoteEventHandler::clear() {
	::utils::TrackedUniqueLock lock(mutex_, "NoteEventHandler::mutex_", ::utils::LockLevel::REPOSITORY);
	noteHistory.clear();
	correlatedNotes.clear();
}

} // namespace music::events
