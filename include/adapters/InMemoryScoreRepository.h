#ifndef MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H
#define MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H

#include <unordered_map>
#include <shared_mutex>
#include "domain/ports/ScoreRepository.h"

namespace music::adapters {

class InMemoryScoreRepository : public ports::ScoreRepository {
public:
	static std::unique_ptr<InMemoryScoreRepository> create();
	
	void save(const std::string& name, const Score& score) override;
	std::unique_ptr<Score> load(const std::string& name) override;
	std::vector<std::string> listScores() override;
	void remove(const std::string& name) override;

private:
	InMemoryScoreRepository() = default;
	std::unordered_map<std::string, std::unique_ptr<Score>> scores;
	mutable std::shared_mutex mutex_;  // Mutex for thread safety
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H