#pragma once

#include "presentation/grid/BatchedGridRenderer.h"
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <memory>
#include <unordered_map>

namespace MusicTrainer::presentation::grid {

class QtBatchedGridRenderer : public BatchedGridRenderer {
public:
    explicit QtBatchedGridRenderer(QGraphicsScene* scene);
    ~QtBatchedGridRenderer() override;

    // Render configuration
    struct RenderStyle {
        QColor cellColor{Qt::black};
        QColor highlightColor{Qt::blue};
        QColor selectionColor{Qt::yellow};
        QColor previewColor{Qt::green};  // Color for note previews
        float cellBorderWidth{1.0f};
        float highlightBorderWidth{2.0f};
        float previewBorderWidth{2.0f};  // Border width for preview notes
        float previewOpacity{0.7f};      // Transparency for preview notes
    };

    void setRenderStyle(const RenderStyle& style);
    const RenderStyle& getRenderStyle() const;

    // Make GridManager a friend class so it can access protected methods
    friend class GridManager;

protected:
    // Override BatchedGridRenderer methods
    void beginBatch(const Rectangle& visibleRect) override;
    void renderCell(const GridCell& cell, const IGridModel& model) override;
    void renderPreview(const GridCell& cell) override;
    void endBatch() override;
    void clear() override;
    void setViewportSize(int width, int height) override;
    void setScale(float scale) override;

private:
    QGraphicsScene* m_scene;
    RenderStyle m_style;
    std::unordered_map<int, std::unique_ptr<QGraphicsItemGroup>> m_batchGroups;
    int m_currentBatchId{0};
    QGraphicsItemGroup* m_previewGroup{nullptr}; // Separate group for preview items

    QGraphicsItem* createCellItem(const GridCell& cell);
    QGraphicsItem* createPreviewItem(const GridCell& cell);
    void optimizeBatch(QGraphicsItemGroup* group);
    void cleanupOldBatches();
};

} // namespace MusicTrainer::presentation::grid