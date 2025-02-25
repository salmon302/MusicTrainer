#ifndef MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H
#define MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H

#include <unordered_map>
#include <functional>
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
	
	void updateScores(const std::string& name, const std::function<void(std::unordered_map<std::string, std::unique_ptr<Score>>&)>& updateFn) {
		auto newScores = std::unordered_map<std::string, std::unique_ptr<Score>>();
		for (const auto& [key, value] : scores) {
			if (value) {
				newScores[key] = std::unique_ptr<Score>(new Score(*value));
			}
		}
		updateFn(newScores);
		scores = std::move(newScores);
	}

	
	std::unordered_map<std::string, std::unique_ptr<Score>> scores;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_INMEMORYSCOREREPOSITORY_H