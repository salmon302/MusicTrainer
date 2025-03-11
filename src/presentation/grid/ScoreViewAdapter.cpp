#include "presentation/grid/ScoreViewAdapter.h"
#include <QGraphicsScene>

namespace MusicTrainer::presentation::grid {

ScoreViewAdapter::ScoreViewAdapter(MusicTrainer::presentation::ScoreView* scoreView)
    : m_scoreView(scoreView)
{
}

ScoreViewAdapter::~ScoreViewAdapter() = default;

void ScoreViewAdapter::initialize() {
    // Get the scene from the ScoreView
    QGraphicsScene* scene = m_scoreView->scene();
    if (!scene) {
        qWarning() << "ScoreViewAdapter::initialize: ScoreView has no scene";
        return;
    }
    
    // Create the NoteGridAdapter
    m_noteGridAdapter = std::make_unique<NoteGridAdapter>(m_scoreView->getNoteGrid());
    
    // Create the ViewportManagerAdapter
    m_viewportManagerAdapter = std::make_unique<ViewportManagerAdapter>(m_scoreView->getViewportManager());
    
    // Create the GridManager with the scene
    m_gridManager = std::make_unique<GridManager>(scene);
    
    // Configure the GridManager
    GridMemoryManager::MemoryConfig memConfig;
    memConfig.maxMemoryUsage = 50 * 1024 * 1024; // 50MB limit
    memConfig.cleanupInterval = 100;
    memConfig.bufferZoneSize = 1.5f;
    m_gridManager->configureMemoryManagement(memConfig);
    
    // Set initial viewport size
    QSize viewSize = m_scoreView->size();
    m_gridManager->setViewportSize(viewSize.width(), viewSize.height());
    
    // Connect signals
    connectSignals();
    
    // Initialize the grid with the current score
    updateFromScore(m_scoreView->getScore());
    
    // Update the grid to render the initial state
    m_gridManager->update();
}

void ScoreViewAdapter::updateFromScore(std::shared_ptr<MusicTrainer::music::Score> score) {
    if (!score) return;
    
    // Clear the grid
    m_gridManager->clear();
    
    // Add notes from the score
    for (size_t voiceIndex = 0; voiceIndex < score->getVoiceCount(); ++voiceIndex) {
        const auto* voice = score->getVoice(voiceIndex);
        if (!voice) continue;

        // Get all notes from the voice
        const auto notes = voice->getAllNotes();
        for (const auto& note : notes) {
            m_gridManager->addNote(note, static_cast<int>(voiceIndex));
        }
    }
}

void ScoreViewAdapter::handleViewportResize(int width, int height) {
    m_gridManager->setViewportSize(width, height);
}

void ScoreViewAdapter::handleViewportScroll(float x, float y) {
    m_gridManager->setViewportPosition(x, y);
}

void ScoreViewAdapter::handleZoomChange(float zoom) {
    m_gridManager->setZoom(zoom);
}

void ScoreViewAdapter::connectSignals() {
    // Connect ScoreView signals to adapter methods
    QObject::connect(m_scoreView, &MusicTrainer::presentation::ScoreView::noteAdded,
                    [this](int pitch, double duration, int position) {
                        handleNoteAdded(pitch, duration, position);
                    });
    
    // Connect GridManager signals to adapter methods
    QObject::connect(m_gridManager.get(), &GridManager::gridChanged,
                    [this]() {
                        handleGridChanged();
                    });
    
    QObject::connect(m_gridManager.get(), &GridManager::viewportChanged,
                    [this]() {
                        handleViewportChanged();
                    });
    
    QObject::connect(m_gridManager.get(), &GridManager::memoryWarning,
                    [this](size_t currentUsage, size_t limit) {
                        handleMemoryWarning(currentUsage, limit);
                    });
}

void ScoreViewAdapter::handleNoteAdded(int pitch, double duration, int position) {
    // Create a note from the parameters using proper constructor
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(pitch);
    MusicTrainer::music::Note note(notePitch, duration, position);
    
    // Add the note to the grid
    m_gridManager->addNote(note, position);
}

void ScoreViewAdapter::handleGridChanged() {
    // Update the ScoreView when the grid changes
    m_scoreView->update();
}

void ScoreViewAdapter::handleViewportChanged() {
    // Update the ScoreView when the viewport changes
    m_scoreView->update();
    
    // Emit the viewportExpanded signal if needed
    auto viewportBounds = m_gridManager->getLayoutManager()->getCurrentViewport();
    QRectF qViewportBounds(viewportBounds.x, viewportBounds.y, 
                          viewportBounds.width, viewportBounds.height);
    
    Q_EMIT m_scoreView->viewportExpanded(qViewportBounds);
}

void ScoreViewAdapter::handleMemoryWarning(size_t currentUsage, size_t limit) {
    // Log a warning about memory usage
    qWarning() << "Memory warning: Using" << currentUsage << "bytes out of" << limit << "bytes limit";
}

void ScoreViewAdapter::handleGridExpansion(GridDirection direction, int amount) {
    // Expand the grid in the specified direction
    m_gridManager->expandGrid(direction, amount);
    
    // Update the viewport manager as well for consistency
    m_viewportManagerAdapter->expandGrid(direction, amount);
    
    // Signal the change
    handleGridChanged();
}

void ScoreViewAdapter::showNotePreview(int position, int pitch, double duration) {
    // Create a temporary note object for preview using proper constructor
    MusicTrainer::music::Pitch notePitch = MusicTrainer::music::Pitch::fromMidiNote(pitch);
    MusicTrainer::music::Note previewNote(notePitch, duration, position);
    
    // Show the preview in the grid manager
    m_gridManager->showNotePreview(previewNote, position);
}

void ScoreViewAdapter::hideNotePreview() {
    // Hide any note preview in the grid manager
    m_gridManager->hideNotePreview();
}

// Helper method to map screen coordinates to musical space using the new grid system
QPointF ScoreViewAdapter::mapToMusicalSpace(const QPoint& screenPos) {
    auto [position, pitch] = m_viewportManagerAdapter->mapToMusicalSpace(
        static_cast<float>(screenPos.x()), 
        static_cast<float>(screenPos.y())
    );
    
    return QPointF(position, pitch);
}

// Helper method to map musical coordinates to screen space using the new grid system
QPoint ScoreViewAdapter::mapFromMusicalSpace(const QPointF& musicalPos) {
    auto [screenX, screenY] = m_viewportManagerAdapter->mapFromMusicalSpace(
        static_cast<int>(musicalPos.x()), 
        static_cast<int>(musicalPos.y())
    );
    
    return QPoint(static_cast<int>(screenX), static_cast<int>(screenY));
}

} // namespace MusicTrainer::presentation::grid