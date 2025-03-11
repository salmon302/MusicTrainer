#include "presentation/grid/GridManager.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QObject>

namespace MusicTrainer::presentation::grid {

// Fully qualify all method implementations with the class name and namespace

GridManager::GridManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
{
    initializeComponents(scene);
    connectSignals();
}

GridManager::~GridManager() = default;

void GridManager::update() {
    auto viewportRect = m_layoutManager->getCurrentViewport();
    m_renderer->beginBatch(viewportRect);

    // Render the grid cells
    auto gridRegion = m_transformer->screenRectToGrid(viewportRect);
    m_memoryManager->setActiveRegion(gridRegion);
    auto cells = m_model->getCellsInRegion(gridRegion);
    for (const auto& cell : cells) {
        m_renderer->renderCell(cell, *m_model);
    }

    // Render the preview if active
    if (m_previewState.active) {
        GridCoordinate previewCoord{m_previewState.position, m_previewState.note.getPitch().getMidiNote()};
        GridCell previewCell{
            previewCoord,
            std::make_shared<MusicTrainer::music::Note>(m_previewState.note)
        };
        m_renderer->renderPreview(previewCell);
    }

    m_renderer->endBatch();
    m_expansionManager->updateUsagePattern(viewportRect);
    checkMemoryUsage();
}

void GridManager::reset() {
    m_model->clear();
    m_renderer->clear();
    m_expansionManager->resetExpansionState();
    
    // Reset to default dimensions
    m_model->setDimensions(GridDimensions{48, 72, 0, 16});
    Q_EMIT gridChanged();
}

void GridManager::clear() {
    m_model->clear();
    m_renderer->clear();
    Q_EMIT gridChanged();
}

void GridManager::setViewportSize(int width, int height) {
    m_renderer->setViewportSize(width, height);
    m_layoutManager->updateViewport(Rectangle{0, 0, static_cast<float>(width), static_cast<float>(height)});
    handleViewportUpdate();
}

void GridManager::setViewportPosition(float x, float y) {
    m_layoutManager->updateViewport(Rectangle{x, y, 
        m_layoutManager->getCurrentViewport().width,
        m_layoutManager->getCurrentViewport().height});
    handleViewportUpdate();
}

void GridManager::setZoom(float scale) {
    m_transformer->setScale(scale);
    m_renderer->setScale(scale);
    update();
    Q_EMIT viewportChanged();
}

void GridManager::addNote(const MusicTrainer::music::Note& note, int position) {
    GridCoordinate coord{position, note.getPitch().getMidiNote()};
    m_model->setCellValue(coord, std::make_shared<MusicTrainer::music::Note>(note));
    m_memoryManager->reportOperation();
    update();
    Q_EMIT gridChanged();
}

void GridManager::removeNote(int position, int pitch) {
    GridCoordinate coord{position, pitch};
    m_model->removeCell(coord);
    m_memoryManager->reportOperation();
    update();
    Q_EMIT gridChanged();
}

std::vector<std::pair<MusicTrainer::music::Note, int>>
GridManager::getNotesInRegion(const MusicTrainer::presentation::grid::GridRegion& region) const {
    std::vector<std::pair<MusicTrainer::music::Note, int>> result;
    auto cells = m_model->getCellsInRegion(region);

    for (const auto& cell : cells) {
        if (auto notePtr = std::get_if<std::shared_ptr<MusicTrainer::music::Note>>(&cell.value)) {
            if (*notePtr) {
                result.emplace_back(**notePtr, cell.coord.position);
            }
        }
    }

    return result;
}

void GridManager::initializeComponents(QGraphicsScene* scene) {
    m_model = std::make_unique<MusicTrainer::presentation::grid::DefaultGridModel>();
    m_transformer = std::make_unique<MusicTrainer::presentation::grid::CoordinateTransformer>();
    m_renderer = std::make_unique<MusicTrainer::presentation::grid::QtBatchedGridRenderer>(scene);
    m_layoutManager = std::make_unique<MusicTrainer::presentation::grid::ViewportLayoutManager>(*m_model, *m_transformer);
    m_expansionManager = std::make_unique<MusicTrainer::presentation::grid::AdaptiveGridExpansionManager>(*m_model, *m_layoutManager);
    m_memoryManager = std::make_unique<MusicTrainer::presentation::grid::GridMemoryManager>(m_model->getSpatialIndex());
}

void GridManager::configureMemoryManagement(const MusicTrainer::presentation::grid::GridMemoryManager::MemoryConfig& config) {
    m_memoryManager->setMemoryConfig(config);
    checkMemoryUsage();
}

size_t GridManager::getCurrentMemoryUsage() const {
    return m_memoryManager->getCurrentMemoryUsage();
}

void GridManager::connectSignals() {
    // No Qt signals to connect yet, but we can add them here when needed
}

void GridManager::handleViewportUpdate() {
    auto viewportRect = m_layoutManager->getCurrentViewport();
    auto gridRegion = m_transformer->screenRectToGrid(viewportRect);
    m_memoryManager->setActiveRegion(gridRegion);
    
    update();
    handleGridExpansion();
    Q_EMIT viewportChanged();
}

void GridManager::handleGridExpansion() {
    if (m_layoutManager->needsExpansion()) {
        m_expansionManager->checkAndExpand();
        update();
        Q_EMIT gridChanged();
    }
}

void GridManager::checkMemoryUsage() {
    auto currentUsage = m_memoryManager->getCurrentMemoryUsage();
    auto config = m_memoryManager->getMemoryConfig();
    
    if (config.maxMemoryUsage > 0 && currentUsage > config.maxMemoryUsage * 0.9) {
        Q_EMIT memoryWarning(currentUsage, config.maxMemoryUsage);
    }
}

void GridManager::showNotePreview(const MusicTrainer::music::Note& note, int position) {
    m_previewState.active = true;
    m_previewState.note = note;
    m_previewState.position = position;
    
    // Update the grid to show the preview
    renderNotePreview();
    Q_EMIT gridChanged();
}

void GridManager::hideNotePreview() {
    if (m_previewState.active) {
        m_previewState.active = false;
        
        // Update the grid to remove the preview
        update();
        Q_EMIT gridChanged();
    }
}

void GridManager::expandGrid(MusicTrainer::presentation::grid::GridDirection direction, int amount) {
    // Update the grid dimensions based on the direction
    MusicTrainer::presentation::grid::GridDimensions dims = m_model->getDimensions();
    
    switch (direction) {
        case MusicTrainer::presentation::grid::GridDirection::Up:
            dims.minPitch -= amount;
            break;
        case MusicTrainer::presentation::grid::GridDirection::Down:
            dims.maxPitch += amount;
            break;
        case MusicTrainer::presentation::grid::GridDirection::Left:
            dims.startPosition -= amount;
            break;
        case MusicTrainer::presentation::grid::GridDirection::Right:
            dims.endPosition += amount;
            break;
    }
    
    // Apply the new dimensions
    m_model->setDimensions(dims);
    
    // Update the expansion manager
    m_expansionManager->notifyManualExpansion(direction, amount);
    
    // Update the grid display
    update();
    Q_EMIT gridChanged();
}

void GridManager::renderNotePreview() {
    if (!m_previewState.active) return;
    
    auto viewportRect = m_layoutManager->getCurrentViewport();
    m_renderer->beginBatch(viewportRect);
    
    // Add the preview note to the renderer
    GridCoordinate previewCoord{m_previewState.position, m_previewState.note.getPitch().getMidiNote()};
    GridCell previewCell{
        previewCoord,
        std::make_shared<MusicTrainer::music::Note>(m_previewState.note)
    };
    
    // Render the preview with a special visual indicator
    m_renderer->renderPreview(previewCell);
    
    // Render the regular cells
    auto gridRegion = m_transformer->screenRectToGrid(viewportRect);
    auto cells = m_model->getCellsInRegion(gridRegion);
    for (const auto& cell : cells) {
        m_renderer->renderCell(cell, *m_model);
    }
    
    m_renderer->endBatch();
}

} // namespace MusicTrainer::presentation::grid