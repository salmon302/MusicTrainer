#include "domain/events/Snapshot.h"
#include "domain/errors/DomainErrors.h"

namespace music::events {

std::unique_ptr<Snapshot> Snapshot::create(const Score& score) {
	return std::unique_ptr<Snapshot>(new Snapshot(score));
}

Snapshot::Snapshot(Score score)
	: score(std::move(score))
	, timestamp(std::chrono::system_clock::now().time_since_epoch().count())
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
	auto cloned = std::make_unique<Snapshot>(score);
	cloned->setVersion(getVersion());
	return cloned;
}

} // namespace music::events

