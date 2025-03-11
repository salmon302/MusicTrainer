#pragma once

#include "presentation/ScoreView.h"
#include "GridManager.h"
#include "NoteGridAdapter.h"
#include "ViewportManagerAdapter.h"
#include <memory>

namespace MusicTrainer::presentation::grid {

/**
 * @brief Adapter class that integrates the new grid components with ScoreView
 * 
 * This adapter facilitates the integration of the new grid architecture
 * with the existing ScoreView, allowing for incremental migration.
 */
class ScoreViewAdapter {
public:
    explicit ScoreViewAdapter(MusicTrainer::presentation::ScoreView* scoreView);
    ~ScoreViewAdapter();
    
    // Initialize the adapter with the new grid components
    void initialize();
    
    // Update the grid when the score changes
    void updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score);
    
    // Handle viewport changes
    void handleViewportResize(int width, int height);
    void handleViewportScroll(float x, float y);
    void handleZoomChange(float zoom);
    
    // Handle grid expansion from ScoreView
    void handleGridExpansion(GridDirection direction, int amount);
    
    // Note preview management
    void showNotePreview(int position, int pitch, double duration);
    void hideNotePreview();
    
    // Coordinate transformations
    QPointF mapToMusicalSpace(const QPoint& screenPos);
    QPoint mapFromMusicalSpace(const QPointF& musicalPos);
    
    // Handle note addition from ScoreView
    void handleNoteAdded(int pitch, double duration, int position);
    
    // Access to the wrapped components
    GridManager* getGridManager() const { return m_gridManager.get(); }
    NoteGridAdapter* getNoteGridAdapter() const { return m_noteGridAdapter.get(); }
    ViewportManagerAdapter* getViewportManagerAdapter() const { return m_viewportManagerAdapter.get(); }
    
private:
    MusicTrainer::presentation::ScoreView* m_scoreView;
    std::unique_ptr<GridManager> m_gridManager;
    std::unique_ptr<NoteGridAdapter> m_noteGridAdapter;
    std::unique_ptr<ViewportManagerAdapter> m_viewportManagerAdapter;
    
    // Connect signals between ScoreView and the new grid components
    void connectSignals();
    
    // Handle grid changes
    void handleGridChanged();
    void handleViewportChanged();
    void handleMemoryWarning(size_t currentUsage, size_t limit);
};

} // namespace MusicTrainer::presentation::grid