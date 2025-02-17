#include "domain/statesync/StateSynchronizationManager.h"

namespace music::statesync {

StateSynchronizationManager::StateSynchronizationManager(events::EventBus& eventBus)
	: eventBus(eventBus) {}

} // namespace music::statesync

