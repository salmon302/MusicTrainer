#include "presentation/grid/QtBatchedGridRenderer.h"
#include <QPainter>
#include <QGraphicsRectItem>

namespace MusicTrainer::presentation::grid {

QtBatchedGridRenderer::QtBatchedGridRenderer(QGraphicsScene* scene)
    : m_scene(scene)
{
}

QtBatchedGridRenderer::~QtBatchedGridRenderer() {
    clear();
}

void QtBatchedGridRenderer::setRenderStyle(const RenderStyle& style) {
    m_style = style;
}

const QtBatchedGridRenderer::RenderStyle& QtBatchedGridRenderer::getRenderStyle() const {
    return m_style;
}

void QtBatchedGridRenderer::beginBatch(const Rectangle& visibleRect) {
    BatchedGridRenderer::beginBatch(visibleRect);
    ++m_currentBatchId;

    // Create a new batch group
    auto group = std::make_unique<QGraphicsItemGroup>();
    m_scene->addItem(group.get());
    m_batchGroups[m_currentBatchId] = std::move(group);

    // Remove any existing preview group
    if (m_previewGroup) {
        m_scene->removeItem(m_previewGroup);
        m_previewGroup = nullptr;
    }

    cleanupOldBatches();
}

void QtBatchedGridRenderer::renderCell(const GridCell& cell, const IGridModel& model) {
    BatchedGridRenderer::renderCell(cell, model);

    auto it = m_batchGroups.find(m_currentBatchId);
    if (it != m_batchGroups.end()) {
        QGraphicsItem* cellItem = createCellItem(cell);
        it->second->addToGroup(cellItem);
    }
}

void QtBatchedGridRenderer::renderPreview(const GridCell& cell) {
    // Create the preview group if it doesn't exist
    if (!m_previewGroup) {
        m_previewGroup = new QGraphicsItemGroup();
        m_scene->addItem(m_previewGroup);
        
        // Set preview group to render on top of normal cells
        m_previewGroup->setZValue(10);
    }
    
    // Create and add the preview item
    QGraphicsItem* previewItem = createPreviewItem(cell);
    m_previewGroup->addToGroup(previewItem);
}

void QtBatchedGridRenderer::endBatch() {
    BatchedGridRenderer::endBatch();

    // Optimize the regular batch
    auto it = m_batchGroups.find(m_currentBatchId);
    if (it != m_batchGroups.end()) {
        optimizeBatch(it->second.get());
    }

    // Also optimize the preview group if it exists
    if (m_previewGroup) {
        optimizeBatch(m_previewGroup);
    }
}

void QtBatchedGridRenderer::clear() {
    for (auto& [_, group] : m_batchGroups) {
        m_scene->removeItem(group.get());
    }
    m_batchGroups.clear();

    // Clear the preview group
    if (m_previewGroup) {
        m_scene->removeItem(m_previewGroup);
        m_previewGroup = nullptr;
    }
}

void QtBatchedGridRenderer::setViewportSize(int width, int height) {
    BatchedGridRenderer::setViewportSize(width, height);
    m_scene->setSceneRect(0, 0, width, height);
}

void QtBatchedGridRenderer::setScale(float scale) {
    BatchedGridRenderer::setScale(scale);
    
    // Update scale of all batch groups
    for (auto& [_, group] : m_batchGroups) {
        group->setScale(scale);
    }
}

QGraphicsItem* QtBatchedGridRenderer::createCellItem(const GridCell& cell) {
    auto* rectItem = new QGraphicsRectItem();
    
    // Convert cell coordinates to scene coordinates
    float x = static_cast<float>(cell.coord.position) * getCellWidth();
    float y = static_cast<float>(cell.coord.pitch) * getCellHeight();
    
    rectItem->setRect(x, y, getCellWidth(), getCellHeight());
    
    // Set appearance based on cell type
    QPen pen(m_style.cellColor);
    pen.setWidthF(m_style.cellBorderWidth);
    rectItem->setPen(pen);
    
    // If the cell has a note, fill it
    if (std::holds_alternative<std::shared_ptr<MusicTrainer::music::Note>>(cell.value)) {
        rectItem->setBrush(m_style.cellColor);
    }
    
    return rectItem;
}

QGraphicsItem* QtBatchedGridRenderer::createPreviewItem(const GridCell& cell) {
    auto* rectItem = new QGraphicsRectItem();
    
    // Convert cell coordinates to scene coordinates
    float x = static_cast<float>(cell.coord.position) * getCellWidth();
    float y = static_cast<float>(cell.coord.pitch) * getCellHeight();
    
    rectItem->setRect(x, y, getCellWidth(), getCellHeight());
    
    // Set preview appearance with distinct style
    QPen pen(m_style.previewColor);
    pen.setWidthF(m_style.previewBorderWidth);
    rectItem->setPen(pen);
    
    // Fill with semi-transparent preview color
    QBrush brush(m_style.previewColor);
    rectItem->setBrush(brush);
    rectItem->setOpacity(m_style.previewOpacity);
    
    return rectItem;
}

void QtBatchedGridRenderer::optimizeBatch(QGraphicsItemGroup* group) {
    // Optimize the batch by combining adjacent cells where possible
    // This is a placeholder for more sophisticated optimization
    group->setFlag(QGraphicsItem::ItemHasNoContents, false);
    group->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

void QtBatchedGridRenderer::cleanupOldBatches() {
    // Keep only the last few batches to manage memory
    constexpr int MAX_BATCHES = 3;
    
    while (m_batchGroups.size() > MAX_BATCHES) {
        auto oldestBatch = std::min_element(
            m_batchGroups.begin(), 
            m_batchGroups.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; }
        );
        
        if (oldestBatch != m_batchGroups.end()) {
            m_scene->removeItem(oldestBatch->second.get());
            m_batchGroups.erase(oldestBatch);
        }
    }
}

} // namespace MusicTrainer::presentation::grid