#ifndef MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H
#define MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H

#include <unordered_map>
#include <memory>
#include <chrono>
#include <atomic>
#include <functional>
#include "domain/ports/ScoreRepository.h"
#include "domain/music/Score.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/DomainErrors.h" // Include this to get RepositoryError definition

namespace music::adapters {

// Forward declaration of recovery result
struct RecoveryResult {
    bool success() const { return _success; }
    std::unique_ptr<MusicTrainer::music::Score> score;
    bool _success = false;
};

class CachingScoreRepository : public ports::ScoreRepository {
public:
    using ErrorHandler = std::function<void(const MusicTrainer::MusicTrainerError&)>;
    using RecoveryStrategy = std::function<RecoveryResult(const MusicTrainer::RepositoryError&)>;

    static std::unique_ptr<CachingScoreRepository> create(std::unique_ptr<ports::ScoreRepository> baseRepository);
    
    void save(const std::string& name, const MusicTrainer::music::Score& score) override;
    std::unique_ptr<MusicTrainer::music::Score> load(const std::string& name) override;
    std::vector<std::string> listScores() override;
    void remove(const std::string& name) override;
    
    void clearCache();
    void setCacheTimeout(std::chrono::seconds timeout);
    size_t getCacheSize() const;
    double getCacheHitRate() const;

    // Add methods to set recovery strategy and error handler
    void setRecoveryStrategy(RecoveryStrategy strategy) {
        recoveryStrategy = std::move(strategy);
    }

    void setErrorHandler(ErrorHandler handler) {
        errorHandler = std::move(handler);
    }

    ~CachingScoreRepository(); // Moved from private to public

private:
    CachingScoreRepository(std::unique_ptr<ports::ScoreRepository> baseRepository);
    
    struct CacheEntry {
        std::unique_ptr<MusicTrainer::music::Score> score;
        std::chrono::system_clock::time_point lastAccess;
        std::atomic<bool> valid{true};
        
        CacheEntry() : lastAccess(std::chrono::system_clock::now()) {}
        CacheEntry(std::unique_ptr<MusicTrainer::music::Score> s, std::chrono::system_clock::time_point t)
            : score(std::move(s)), lastAccess(t), valid(true) {}
    };
    
    void updateCache(const std::string& name, const std::function<void(std::unordered_map<std::string, CacheEntry>&)>& updateFn) {
        auto newCache = std::unordered_map<std::string, CacheEntry>();
        for (const auto& [key, entry] : cache) {
            if (entry.score && entry.valid.load()) {
                auto& newEntry = newCache[key];
                newEntry.score = std::make_unique<MusicTrainer::music::Score>(*entry.score);
                newEntry.lastAccess = entry.lastAccess;
                newEntry.valid.store(true);
            }
        }
        updateFn(newCache);
        cache = std::move(newCache);
    }
    
    std::unique_ptr<ports::ScoreRepository> baseRepository;
    std::unordered_map<std::string, CacheEntry> cache;
    std::atomic<std::chrono::seconds::rep> cacheTimeout{300}; // 5 minutes default
    
    std::atomic<size_t> cacheHits{0};
    std::atomic<size_t> totalAccesses{0};
    
    // Add missing member variables for error handling and recovery
    RecoveryStrategy recoveryStrategy;
    ErrorHandler errorHandler;
    
    void cleanExpiredEntries();
    bool isExpired(const CacheEntry& entry) const;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H