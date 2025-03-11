#pragma once

#include "DefaultGridModel.h"
#include "QtBatchedGridRenderer.h"
#include "ViewportLayoutManager.h"
#include "AdaptiveGridExpansionManager.h"
#include "GridMemoryManager.h"
#include <QObject>
#include <memory>

namespace MusicTrainer::presentation::grid {

class GridManager : public QObject {
    Q_OBJECT

public:
    explicit GridManager(QGraphicsScene* scene, QObject* parent = nullptr);
    ~GridManager() override;

    // Core operations
    void update();
    void reset();
    void clear();

    // View control
    void setViewportSize(int width, int height);
    void setViewportPosition(float x, float y);
    void setZoom(float scale);

    // Grid operations
    void addNote(const MusicTrainer::music::Note& note, int position);
    void removeNote(int position, int pitch);
    std::vector<std::pair<MusicTrainer::music::Note, int>> getNotesInRegion(const GridRegion& region) const;

    // Note preview
    void showNotePreview(const MusicTrainer::music::Note& note, int position);
    void hideNotePreview();

    // Grid expansion
    void expandGrid(GridDirection direction, int amount);

    // Memory management
    void configureMemoryManagement(const GridMemoryManager::MemoryConfig& config);
    size_t getCurrentMemoryUsage() const;

    // Component access
    IGridModel* getModel() const { return m_model.get(); }
    IGridRenderer* getRenderer() const { return m_renderer.get(); }
    ViewportLayoutManager* getLayoutManager() const { return m_layoutManager.get(); }
    GridMemoryManager* getMemoryManager() const { return m_memoryManager.get(); }

Q_SIGNALS:
    void gridChanged();
    void viewportChanged();
    void memoryWarning(size_t currentUsage, size_t limit);

private:
    std::unique_ptr<DefaultGridModel> m_model;
    std::unique_ptr<QtBatchedGridRenderer> m_renderer;
    std::unique_ptr<ViewportLayoutManager> m_layoutManager;
    std::unique_ptr<AdaptiveGridExpansionManager> m_expansionManager;
    std::unique_ptr<CoordinateTransformer> m_transformer;
    std::unique_ptr<GridMemoryManager> m_memoryManager;

    // Preview state
    struct PreviewState {
        bool active = false;
        MusicTrainer::music::Note note;
        int position = 0;
    };
    PreviewState m_previewState;

    void initializeComponents(QGraphicsScene* scene);
    void connectSignals();
    void handleViewportUpdate();
    void handleGridExpansion();
    void checkMemoryUsage();
    void renderNotePreview();
};

} // namespace MusicTrainer::presentation::grid