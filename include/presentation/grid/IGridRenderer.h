#pragma once

#include <vector>
#include "IGridModel.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Interface for grid rendering
 */
class IGridRenderer {
public:
    virtual ~IGridRenderer() = default;
    
    /**
     * @brief Begin a new render batch
     * @param visibleRect The currently visible rectangle
     */
    virtual void beginBatch(const Rectangle& visibleRect) = 0;
    
    /**
     * @brief Render a cell in the current batch
     * @param cell Cell to render
     * @param model Grid model containing cell data
     */
    virtual void renderCell(const GridCell& cell, const IGridModel& model) = 0;
    
    /**
     * @brief Render a preview cell with special styling
     * @param cell Cell to render as preview
     */
    virtual void renderPreview(const GridCell& cell) = 0;
    
    /**
     * @brief End the current render batch
     * Optimizes and executes all pending render operations
     */
    virtual void endBatch() = 0;
    
    /**
     * @brief Clear all rendered content
     */
    virtual void clear() = 0;
    
    /**
     * @brief Update the viewport size
     * @param width New width
     * @param height New height
     */
    virtual void setViewportSize(int width, int height) = 0;
    
    /**
     * @brief Set the scale factor for rendering
     * @param scale New scale factor
     */
    virtual void setScale(float scale) = 0;
};

} // namespace MusicTrainer::presentation::grid