#ifndef MUSICTRAINERV3_SCOREREPOSITORY_H
#define MUSICTRAINERV3_SCOREREPOSITORY_H

#include <memory>
#include <string>
#include <vector>
#include "../music/Score.h"

namespace music::ports {

class ScoreRepository {
public:
	virtual ~ScoreRepository() = default;
	
	// Save a score with the given name
	virtual void save(const std::string& name, const MusicTrainer::music::Score& score) = 0;
	
	// Load a score by name
	virtual std::unique_ptr<MusicTrainer::music::Score> load(const std::string& name) = 0;
	
	// List all available scores
	virtual std::vector<std::string> listScores() = 0;
	
	// Delete a score by name
	virtual void remove(const std::string& name) = 0;
};

} // namespace music::ports

#endif // MUSICTRAINERV3_SCOREREPOSITORY_H