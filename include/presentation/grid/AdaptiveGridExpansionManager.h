#pragma once

#include "IGridModel.h"
#include "ViewportLayoutManager.h"
#include <chrono>

namespace MusicTrainer::presentation::grid {

/**
 * @brief Manages adaptive grid expansion based on usage patterns
 */
class AdaptiveGridExpansionManager {
public:
    struct ExpansionConfig {
        float baseExpansionRate;         // Base rate for expansion
        float maxExpansionRate;          // Maximum expansion rate
        std::chrono::seconds cooldownPeriod; // Cooldown between expansions
        size_t usageHistorySize;           // Size of usage history to track
        
        ExpansionConfig()
            : baseExpansionRate(1.2f)
            , maxExpansionRate(2.0f)
            , cooldownPeriod(5)
            , usageHistorySize(10)
        {}
    };

    AdaptiveGridExpansionManager(IGridModel& model,
                                ViewportLayoutManager& layoutManager,
                                ExpansionConfig config = ExpansionConfig());

    // Expansion control
    void updateUsagePattern(const Rectangle& visibleArea);
    void checkAndExpand();
    void resetExpansionState();
    void notifyManualExpansion(GridDirection direction, int amount);

    // Configuration
    void setConfig(const ExpansionConfig& config);
    const ExpansionConfig& getConfig() const;

    // State queries
    float getCurrentExpansionRate() const;
    bool isInCooldown() const;

private:
    IGridModel& m_model;
    ViewportLayoutManager& m_layoutManager;
    ExpansionConfig m_config;
    float m_currentExpansionRate;
    std::chrono::steady_clock::time_point m_lastExpansionTime;
    std::vector<Rectangle> m_usageHistory;

    float calculateExpansionRate() const;
    bool shouldTriggerExpansion() const;
    void performExpansion();
    void updateUsageHistory(const Rectangle& visibleArea);
};

} // namespace MusicTrainer::presentation::grid