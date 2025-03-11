#pragma once

#include "IGridModel.h"
#include "CoordinateTransformer.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Manages viewport layout and dynamic expansion
 */
class ViewportLayoutManager {
public:
    struct LayoutConfig {
        float expansionThreshold;  // When to trigger expansion (% of viewport)
        float contractionThreshold; // When to trigger contraction (% of viewport)
        float expansionFactor;     // How much to expand by
        bool enableDynamicExpansion;
        
        LayoutConfig()
            : expansionThreshold(0.8f)
            , contractionThreshold(0.2f)
            , expansionFactor(1.5f)
            , enableDynamicExpansion(true)
        {}
    };

    ViewportLayoutManager(IGridModel& model,
                         CoordinateTransformer& transformer,
                         LayoutConfig config = LayoutConfig());

    // Layout operations
    void updateViewport(const Rectangle& viewportRect);
    void checkAndExpandViewport();
    bool tryContractViewport();
    
    // Configuration
    void setConfig(const LayoutConfig& config);
    const LayoutConfig& getConfig() const;
    
    // State queries
    bool needsExpansion() const;
    bool canContract() const;
    Rectangle getCurrentViewport() const;

private:
    IGridModel& m_model;
    CoordinateTransformer& m_transformer;
    LayoutConfig m_config;
    Rectangle m_currentViewport;

    bool checkExpansionThreshold() const;
    bool checkContractionThreshold() const;
    void expandViewport(const GridRegion& expansion);
};

} // namespace MusicTrainer::presentation::grid