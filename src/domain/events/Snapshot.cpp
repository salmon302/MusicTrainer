#include "domain/events/Snapshot.h"
#include "domain/errors/DomainErrors.h"

namespace music::events {

std::unique_ptr<Snapshot> Snapshot::create(const Score& score) {
	return std::unique_ptr<Snapshot>(new Snapshot(score));
}

Snapshot::Snapshot(Score score)
	: score(std::move(score))
	, timestamp(std::chrono::system_clock::now())
	, version(0)
{}

std::unique_ptr<Score> Snapshot::reconstruct() const {
	try {
		return std::make_unique<Score>(score);
	} catch (const std::exception& e) {
		throw MusicTrainer::RepositoryError("Failed to reconstruct score from snapshot: " + std::string(e.what()));
	}
}

std::unique_ptr<Snapshot> Snapshot::clone() const {
	::utils::TrackedUniqueLock lock(mutex_, "Snapshot::mutex_", ::utils::LockLevel::REPOSITORY);
	auto cloned = std::make_unique<Snapshot>(score);
	cloned->setVersion(version);
	return cloned;
}

} // namespace music::events
