#ifndef MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H
#define MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H

#include <unordered_map>
#include <memory>
#include <chrono>
#include <atomic>
#include <functional>
#include "domain/ports/ScoreRepository.h"
#include "domain/music/Score.h"

namespace music::adapters {

class CachingScoreRepository : public ports::ScoreRepository {
public:
	static std::unique_ptr<CachingScoreRepository> create(std::unique_ptr<ScoreRepository> baseRepository);
	
	void save(const std::string& name, const Score& score) override;
	std::unique_ptr<Score> load(const std::string& name) override;
	std::vector<std::string> listScores() override;
	void remove(const std::string& name) override;
	
	void clearCache();
	void setCacheTimeout(std::chrono::seconds timeout);
	size_t getCacheSize() const;
	double getCacheHitRate() const;

private:
	CachingScoreRepository(std::unique_ptr<ScoreRepository> baseRepository);
	
	struct CacheEntry {
		std::unique_ptr<Score> score;
		std::chrono::system_clock::time_point lastAccess;
		std::atomic<bool> valid{true};
		
		CacheEntry() : lastAccess(std::chrono::system_clock::now()) {}
		CacheEntry(std::unique_ptr<Score> s, std::chrono::system_clock::time_point t)
			: score(std::move(s)), lastAccess(t), valid(true) {}
	};
	
	void updateCache(const std::string& name, const std::function<void(std::unordered_map<std::string, CacheEntry>&)>& updateFn) {
		auto newCache = std::unordered_map<std::string, CacheEntry>();
		for (const auto& [key, entry] : cache) {
			if (entry.score && entry.valid.load()) {
				auto& newEntry = newCache[key];
				newEntry.score = std::make_unique<Score>(*entry.score);
				newEntry.lastAccess = entry.lastAccess;
				newEntry.valid.store(true);
			}
		}
		updateFn(newCache);
		cache = std::move(newCache);
	}

	
	std::unique_ptr<ScoreRepository> baseRepository;
	std::unordered_map<std::string, CacheEntry> cache;
	std::atomic<std::chrono::seconds::rep> cacheTimeout{300}; // 5 minutes default
	
	std::atomic<size_t> cacheHits{0};
	std::atomic<size_t> totalAccesses{0};
	
	void cleanExpiredEntries();
	bool isExpired(const CacheEntry& entry) const;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H