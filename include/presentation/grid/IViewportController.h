#pragma once

#include <optional>
#include "../GridTypes.h"
#include "IGridModel.h"

namespace MusicTrainer::presentation::grid {

/**
 * @brief Interface for viewport control
 * 
 * This interface defines the contract for viewport controllers, providing
 * methods for managing the viewport state, handling expansion, and
 * coordinate transformations.
 */
class IViewportController {
public:
    /**
     * @brief Viewport state structure
     */
    struct ViewportState {
        Rectangle visibleArea;      // Current viewport bounds in musical coordinates
        float zoomLevel;            // Current zoom level
        struct {                    // Scroll position
            float x;
            float y;
        } scrollPosition;
        bool preserveOctaveExpansion = false; // Whether to maintain expanded octaves
    };
    
    /**
     * @brief Loading boundaries structure
     */
    struct LoadingBoundaries {
        float horizontalBuffer;     // Additional horizontal area to load
        float verticalBuffer;       // Additional vertical area to load
    };
    
    virtual ~IViewportController() = default;
    
    /**
     * @brief Set the viewport bounds
     * @param bounds New viewport bounds
     */
    virtual void setViewportBounds(const Rectangle& bounds) = 0;
    
    /**
     * @brief Get the current viewport bounds
     * @return Current viewport bounds
     */
    virtual Rectangle getViewportBounds() const = 0;
    
    /**
     * @brief Get the current viewport state
     * @return Current viewport state
     */
    virtual const ViewportState& getViewportState() const = 0;
    
    /**
     * @brief Update the view size
     * @param width View width
     * @param height View height
     */
    virtual void updateViewSize(float width, float height) = 0;
    
    /**
     * @brief Update the scroll position
     * @param x X coordinate
     * @param y Y coordinate
     */
    virtual void updateScrollPosition(float x, float y) = 0;
    
    /**
     * @brief Update the zoom level
     * @param zoom New zoom level
     */
    virtual void updateZoomLevel(float zoom) = 0;
    
    /**
     * @brief Expand the grid in the specified direction
     * @param direction Direction to expand
     * @param amount Amount to expand by
     */
    virtual void expandGrid(GridDirection direction, int amount) = 0;
    
    /**
     * @brief Collapse the grid in the specified direction
     * @param direction Direction to collapse
     */
    virtual void collapseGrid(GridDirection direction) = 0;
    
    /**
     * @brief Check if the grid can be collapsed in the specified direction
     * @param direction Direction to check
     * @return True if the grid can be collapsed
     */
    virtual bool canCollapse(GridDirection direction) const = 0;
    
    /**
     * @brief Check if the grid should expand based on current viewport state
     * @return Direction to expand in, or std::nullopt if no expansion needed
     */
    virtual std::optional<GridDirection> shouldExpand() const = 0;
    
    /**
     * @brief Get the current loading boundaries
     * @return Current loading boundaries
     */
    virtual const LoadingBoundaries& getLoadingBoundaries() const = 0;
    
    /**
     * @brief Set the loading boundaries
     * @param boundaries New loading boundaries
     */
    virtual void setLoadingBoundaries(const LoadingBoundaries& boundaries) = 0;
    
    /**
     * @brief Compact unused regions
     * Frees memory for regions outside the buffer zone
     */
    virtual void compactUnusedRegions() = 0;
    
    /**
     * @brief Map a point from screen space to musical space
     * @param screenX X coordinate in screen space
     * @param screenY Y coordinate in screen space
     * @return Point in musical space (position, pitch)
     */
    virtual std::pair<int, int> mapToMusicalSpace(float screenX, float screenY) const = 0;
    
    /**
     * @brief Map a point from musical space to screen space
     * @param position Position in musical space
     * @param pitch Pitch in musical space
     * @return Point in screen space (x, y)
     */
    virtual std::pair<float, float> mapFromMusicalSpace(int position, int pitch) const = 0;

    /**
     * @brief Update viewport position
     * @param x New x coordinate
     * @param y New y coordinate
     */
    virtual void setPosition(float x, float y) = 0;

    /**
     * @brief Get current viewport position
     * @return Current position as x,y pair
     */
    virtual std::pair<float, float> getPosition() const = 0;

    /**
     * @brief Set viewport zoom level
     * @param scale New zoom scale
     */
    virtual void setZoom(float scale) = 0;

    /**
     * @brief Get current zoom level
     * @return Current zoom scale
     */
    virtual float getZoom() const = 0;

    /**
     * @brief Update viewport dimensions
     * @param width New width
     * @param height New height
     */
    virtual void resize(int width, int height) = 0;

    /**
     * @brief Get current viewport dimensions
     * @return Width and height as pair
     */
    virtual std::pair<int, int> getDimensions() const = 0;

    /**
     * @brief Convert screen coordinates to grid coordinates
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     * @return Grid coordinates
     */
    virtual GridCoordinate screenToGrid(float screenX, float screenY) const = 0;

    /**
     * @brief Convert grid coordinates to screen coordinates
     * @param gridCoord Grid coordinates
     * @return Screen coordinates as x,y pair
     */
    virtual std::pair<float, float> gridToScreen(const GridCoordinate& gridCoord) const = 0;

    /**
     * @brief Get the currently visible grid region
     * @return Visible region
     */
    virtual GridRegion getVisibleRegion() const = 0;
};

} // namespace MusicTrainer::presentation::grid