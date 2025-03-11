#pragma once

#include <vector>
#include <queue>
#include "IGridRenderer.h"

// Forward declarations for Qt classes
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsItemGroup;

namespace MusicTrainer::presentation::grid {

// Simple color class to avoid Qt dependency
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Simple point class to avoid Qt dependency
struct Point {
    float x, y;
    
    Point(float xPos = 0.0f, float yPos = 0.0f) : x(xPos), y(yPos) {}
};

/**
 * @brief Batched renderer for efficient grid rendering
 * 
 * This class implements the IGridRenderer interface using a batched rendering
 * approach for improved performance. It groups similar elements together to
 * reduce the number of QGraphicsItems and improve rendering speed.
 */
class BatchedGridRenderer : public IGridRenderer {
public:
    /**
     * @brief Constructor
     */
    explicit BatchedGridRenderer();
    
    /**
     * @brief Destructor
     */
    ~BatchedGridRenderer() override = default;
    
    // IGridRenderer interface implementation
    void beginBatch(const Rectangle& visibleRect) override;
    void renderCell(const GridCell& cell, const IGridModel& model) override;
    void renderPreview(const GridCell& cell) override;
    void endBatch() override;
    void clear() override;
    void setViewportSize(int width, int height) override;
    void setScale(float scale) override;

protected:
    // Helper methods for derived classes
    float getCellWidth() const { return 40.0f * m_scale; }
    float getCellHeight() const { return 15.0f * m_scale; }

private:
    struct RenderCommand {
        GridCell cell;
        std::reference_wrapper<const IGridModel> model;
        bool isPreview{false};
    };

    std::queue<RenderCommand> m_renderQueue;
    Rectangle m_visibleRect;
    int m_viewportWidth;
    int m_viewportHeight;
    float m_scale{1.0f};

    void processRenderQueue();
    bool isCellVisible(const GridCell& cell) const;
};

} // namespace MusicTrainer::presentation::grid