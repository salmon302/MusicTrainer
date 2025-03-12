#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QVBoxLayout>
#include <QPointF>
#include <memory>
#include "presentation/GridConstants.h"
#include "presentation/GridTypes.h"
#include "domain/music/Duration.h"
#include "domain/music/Voice.h"
#include "domain/events/GuiStateEvent.h"
#include "domain/events/EventBus.h"

QT_BEGIN_NAMESPACE
class QResizeEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace MusicTrainer::music {
class Score;
class Note;
}

namespace MusicTrainer::presentation::grid {
class ScoreViewAdapter;
}

namespace MusicTrainer::presentation {

class NoteGrid;
class ViewportManager;

class ScoreView : public QGraphicsView {
    Q_OBJECT
public:
    explicit ScoreView(std::shared_ptr<music::events::EventBus> eventBus, QWidget* parent = nullptr);
    ~ScoreView() override;
    
    // Constants for grid sizing
    static constexpr double GRID_UNIT = GridConstants::GRID_UNIT;
    static constexpr double NOTE_HEIGHT = GridConstants::NOTE_HEIGHT;
    static constexpr double GRID_ZOOM_BASE = GridConstants::GRID_ZOOM_BASE;
   
    void updateScore(const music::Score& score);
    void clearScore();
    void highlightNote(int position, int voiceIndex);
    
    void setScore(std::shared_ptr<MusicTrainer::music::Score> score);
    std::shared_ptr<MusicTrainer::music::Score> getScore() const;
    QRectF getViewportBounds() const;
    QPointF mapToMusicalSpace(const QPointF& screenPoint) const;
    QPointF mapFromMusicalSpace(const QPointF& musicalPoint) const;
    MusicTrainer::music::Duration convertToMusicalDuration(double numericDuration);
    void expandGrid(GridDirection direction, int amount);

    // Add getter methods
    NoteGrid* getNoteGrid() const { return m_noteGrid.get(); }
    ViewportManager* getViewportManager() const { return m_viewportManager.get(); }

Q_SIGNALS:
    void noteSelected(int position, int voiceIndex);
    void viewportExpanded(const QRectF& newBounds);
    void noteAdded(int pitch, double duration, int position = 0);

public Q_SLOTS:
    void onScoreChanged();
    void onVoiceAdded(const MusicTrainer::music::Voice& voice);
    void onNoteAdded(const MusicTrainer::music::Note& note);
    void checkViewportExpansion();
    void handleNoteAdded(int pitch, double duration, int position = 0);
    
    // UI settings methods moved to public
    void setShowMeasureNumbers(bool show);
    void setShowKeySignature(bool show);
    void setShowVoiceLabels(bool show);
    bool showMeasureNumbers() const { return m_showMeasureNumbers; }
    bool showKeySignature() const { return m_showKeySignature; }
    bool showVoiceLabels() const { return m_showVoiceLabels; }
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    void initializeViewport();
    void updateGridVisuals();
    void publishViewportState();
    
    class ScoreViewImpl;
    std::unique_ptr<ScoreViewImpl> m_impl;
    
    std::unique_ptr<NoteGrid> m_noteGrid;
    std::unique_ptr<ViewportManager> m_viewportManager;
    std::shared_ptr<MusicTrainer::music::Score> m_score;
    std::shared_ptr<music::events::EventBus> m_eventBus;
    
    // New adapter for the grid architecture
    std::unique_ptr<grid::ScoreViewAdapter> m_gridAdapter;
    
    bool m_isSelecting = false;
    QPoint m_lastMousePos;

    bool m_isDragging{false};
    int m_draggedNotePosition{-1};
    int m_draggedNotePitch{-1};
    double m_currentNoteDuration{1.0}; // Default to quarter note
    QGraphicsItem* m_selectedNote{nullptr};

    // Add toolbar and button members
    QToolBar* m_durationToolbar{nullptr};
    QLabel* m_durationLabel{nullptr};
    QButtonGroup* m_durationButtons{nullptr};
    
    // Expansion button visual elements
    QGraphicsRectItem* m_topExpandButton{nullptr};
    QGraphicsRectItem* m_bottomExpandButton{nullptr};
    QGraphicsRectItem* m_rightExpandButton{nullptr};

    // Helper methods
    void setupDurationToolbar();
    QString getDurationName(double duration) const;
    void updateDurationLabel();
    void deleteNoteAt(const QPointF& scenePos);
    void startNoteDrag(const QPointF& scenePos);
    void updateNoteDrag(const QPointF& scenePos);
    void finishNoteDrag(const QPointF& scenePos);
    void cycleDuration(); // Cycles through available note durations
    
private:
    // UI display settings
    bool m_showMeasureNumbers{true};
    bool m_showKeySignature{true};
    bool m_showVoiceLabels{true};
};

} // namespace MusicTrainer::presentation