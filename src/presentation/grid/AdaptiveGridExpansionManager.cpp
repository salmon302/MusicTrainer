#include "presentation/grid/AdaptiveGridExpansionManager.h"
#include <algorithm>

namespace MusicTrainer::presentation::grid {

AdaptiveGridExpansionManager::AdaptiveGridExpansionManager(
    IGridModel& model,
    ViewportLayoutManager& layoutManager,
    ExpansionConfig config)
    : m_model(model)
    , m_layoutManager(layoutManager)
    , m_config(std::move(config))
    , m_currentExpansionRate(m_config.baseExpansionRate)
    , m_lastExpansionTime(std::chrono::steady_clock::now())
{
    m_usageHistory.reserve(m_config.usageHistorySize);
}

void AdaptiveGridExpansionManager::updateUsagePattern(const Rectangle& visibleArea) {
    updateUsageHistory(visibleArea);
    if (shouldTriggerExpansion()) {
        performExpansion();
    }
}

void AdaptiveGridExpansionManager::checkAndExpand() {
    if (!isInCooldown() && shouldTriggerExpansion()) {
        performExpansion();
    }
}

void AdaptiveGridExpansionManager::resetExpansionState() {
    m_currentExpansionRate = m_config.baseExpansionRate;
    m_usageHistory.clear();
    m_lastExpansionTime = std::chrono::steady_clock::now();
}

void AdaptiveGridExpansionManager::setConfig(const ExpansionConfig& config) {
    m_config = config;
    if (m_usageHistory.size() > m_config.usageHistorySize) {
        m_usageHistory.resize(m_config.usageHistorySize);
    }
}

const AdaptiveGridExpansionManager::ExpansionConfig& 
AdaptiveGridExpansionManager::getConfig() const {
    return m_config;
}

float AdaptiveGridExpansionManager::getCurrentExpansionRate() const {
    return m_currentExpansionRate;
}

bool AdaptiveGridExpansionManager::isInCooldown() const {
    auto now = std::chrono::steady_clock::now();
    return (now - m_lastExpansionTime) < m_config.cooldownPeriod;
}

float AdaptiveGridExpansionManager::calculateExpansionRate() const {
    if (m_usageHistory.size() < 2) return m_config.baseExpansionRate;

    float totalGrowth = 0.0f;
    for (size_t i = 1; i < m_usageHistory.size(); ++i) {
        const auto& prev = m_usageHistory[i - 1];
        const auto& curr = m_usageHistory[i];

        float widthGrowth = curr.width / prev.width;
        float heightGrowth = curr.height / prev.height;
        totalGrowth += std::max(widthGrowth, heightGrowth);
    }

    float avgGrowth = totalGrowth / (m_usageHistory.size() - 1);
    return std::clamp(avgGrowth, m_config.baseExpansionRate, m_config.maxExpansionRate);
}

bool AdaptiveGridExpansionManager::shouldTriggerExpansion() const {
    if (m_usageHistory.empty()) return false;
    
    const auto& currentArea = m_usageHistory.back();
    auto dimensions = m_model.getDimensions();
    
    float horizontalUsage = currentArea.width / 
        (dimensions.endPosition - dimensions.startPosition);
    float verticalUsage = currentArea.height / 
        (dimensions.maxPitch - dimensions.minPitch);

    return horizontalUsage > 0.8f || verticalUsage > 0.8f;
}

void AdaptiveGridExpansionManager::performExpansion() {
    m_currentExpansionRate = calculateExpansionRate();
    
    auto dimensions = m_model.getDimensions();
    int horizontalExpansion = static_cast<int>(
        (dimensions.endPosition - dimensions.startPosition) * 
        (m_currentExpansionRate - 1.0f)
    );
    int verticalExpansion = static_cast<int>(
        (dimensions.maxPitch - dimensions.minPitch) * 
        (m_currentExpansionRate - 1.0f)
    );

    dimensions.startPosition -= horizontalExpansion / 2;
    dimensions.endPosition += horizontalExpansion / 2;
    dimensions.minPitch -= verticalExpansion / 2;
    dimensions.maxPitch += verticalExpansion / 2;

    m_model.setDimensions(dimensions);
    m_lastExpansionTime = std::chrono::steady_clock::now();
}

void AdaptiveGridExpansionManager::updateUsageHistory(const Rectangle& visibleArea) {
    if (m_usageHistory.size() >= m_config.usageHistorySize) {
        m_usageHistory.erase(m_usageHistory.begin());
    }
    m_usageHistory.push_back(visibleArea);
}

void AdaptiveGridExpansionManager::notifyManualExpansion(GridDirection direction, int amount) {
    // Reset the cooldown timer when manual expansion occurs
    m_lastExpansionTime = std::chrono::steady_clock::now() - m_config.cooldownPeriod;
    
    // Update the expansion rate based on the manual expansion
    float expansionFactor = 1.0f + (amount / 100.0f);
    m_currentExpansionRate = std::clamp(
        m_currentExpansionRate * expansionFactor,
        m_config.baseExpansionRate,
        m_config.maxExpansionRate
    );
    
    // If the usage history is empty, add a placeholder
    if (m_usageHistory.empty()) {
        auto dimensions = m_model.getDimensions();
        Rectangle initialRect{
            static_cast<float>(dimensions.startPosition),
            static_cast<float>(dimensions.minPitch),
            static_cast<float>(dimensions.endPosition - dimensions.startPosition),
            static_cast<float>(dimensions.maxPitch - dimensions.minPitch)
        };
        m_usageHistory.push_back(initialRect);
    }
}

} // namespace MusicTrainer::presentation::grid