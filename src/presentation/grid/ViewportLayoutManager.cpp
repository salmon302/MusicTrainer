#include "presentation/grid/ViewportLayoutManager.h"

namespace MusicTrainer::presentation::grid {

ViewportLayoutManager::ViewportLayoutManager(IGridModel& model,
                                           CoordinateTransformer& transformer,
                                           LayoutConfig config)
    : m_model(model)
    , m_transformer(transformer)
    , m_config(std::move(config))
{
}

void ViewportLayoutManager::updateViewport(const Rectangle& viewportRect) {
    m_currentViewport = viewportRect;
    if (m_config.enableDynamicExpansion) {
        checkAndExpandViewport();
    }
}

void ViewportLayoutManager::checkAndExpandViewport() {
    if (checkExpansionThreshold()) {
        GridRegion currentRegion = m_transformer.screenRectToGrid(m_currentViewport);
        int horizontalExpansion = static_cast<int>((currentRegion.endPosition - currentRegion.startPosition) 
                                                  * (m_config.expansionFactor - 1.0f));
        int verticalExpansion = static_cast<int>((currentRegion.maxPitch - currentRegion.minPitch) 
                                                * (m_config.expansionFactor - 1.0f));

        GridRegion expansionRegion = {
            currentRegion.startPosition - horizontalExpansion,
            currentRegion.endPosition + horizontalExpansion,
            currentRegion.minPitch - verticalExpansion,
            currentRegion.maxPitch + verticalExpansion
        };

        expandViewport(expansionRegion);
    }
}

bool ViewportLayoutManager::tryContractViewport() {
    if (!canContract()) return false;

    if (checkContractionThreshold()) {
        GridRegion currentRegion = m_transformer.screenRectToGrid(m_currentViewport);
        int horizontalContraction = static_cast<int>((currentRegion.endPosition - currentRegion.startPosition) 
                                                   * (1.0f - m_config.contractionThreshold));
        int verticalContraction = static_cast<int>((currentRegion.maxPitch - currentRegion.minPitch) 
                                                  * (1.0f - m_config.contractionThreshold));

        GridRegion contractedRegion = {
            currentRegion.startPosition + horizontalContraction,
            currentRegion.endPosition - horizontalContraction,
            currentRegion.minPitch + verticalContraction,
            currentRegion.maxPitch - verticalContraction
        };

        m_model.setDimensions(GridDimensions{
            contractedRegion.minPitch,
            contractedRegion.maxPitch,
            contractedRegion.startPosition,
            contractedRegion.endPosition
        });

        return true;
    }

    return false;
}

void ViewportLayoutManager::setConfig(const LayoutConfig& config) {
    m_config = config;
    if (m_config.enableDynamicExpansion) {
        checkAndExpandViewport();
    }
}

const ViewportLayoutManager::LayoutConfig& ViewportLayoutManager::getConfig() const {
    return m_config;
}

bool ViewportLayoutManager::needsExpansion() const {
    return m_config.enableDynamicExpansion && checkExpansionThreshold();
}

bool ViewportLayoutManager::canContract() const {
    GridRegion currentRegion = m_transformer.screenRectToGrid(m_currentViewport);
    int minWidth = static_cast<int>((currentRegion.endPosition - currentRegion.startPosition) 
                                   * m_config.contractionThreshold);
    int minHeight = static_cast<int>((currentRegion.maxPitch - currentRegion.minPitch) 
                                    * m_config.contractionThreshold);

    return minWidth > 1 && minHeight > 1;
}

Rectangle ViewportLayoutManager::getCurrentViewport() const {
    return m_currentViewport;
}

bool ViewportLayoutManager::checkExpansionThreshold() const {
    GridRegion currentRegion = m_transformer.screenRectToGrid(m_currentViewport);
    auto dimensions = m_model.getDimensions();
    
    float horizontalUsage = static_cast<float>(currentRegion.endPosition - currentRegion.startPosition) /
                           static_cast<float>(dimensions.endPosition - dimensions.startPosition);
    float verticalUsage = static_cast<float>(currentRegion.maxPitch - currentRegion.minPitch) /
                         static_cast<float>(dimensions.maxPitch - dimensions.minPitch);

    return horizontalUsage > m_config.expansionThreshold ||
           verticalUsage > m_config.expansionThreshold;
}

bool ViewportLayoutManager::checkContractionThreshold() const {
    GridRegion currentRegion = m_transformer.screenRectToGrid(m_currentViewport);
    auto dimensions = m_model.getDimensions();
    
    float horizontalUsage = static_cast<float>(currentRegion.endPosition - currentRegion.startPosition) /
                           static_cast<float>(dimensions.endPosition - dimensions.startPosition);
    float verticalUsage = static_cast<float>(currentRegion.maxPitch - currentRegion.minPitch) /
                         static_cast<float>(dimensions.maxPitch - dimensions.minPitch);

    return horizontalUsage < m_config.contractionThreshold &&
           verticalUsage < m_config.contractionThreshold;
}

void ViewportLayoutManager::expandViewport(const GridRegion& expansion) {
    m_model.setDimensions(GridDimensions{
        expansion.minPitch,
        expansion.maxPitch,
        expansion.startPosition,
        expansion.endPosition
    });
}

} // namespace MusicTrainer::presentation::grid