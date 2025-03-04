#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QWidget>
#include <memory>
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "presentation/GridConstants.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QResizeEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class NoteGrid;
class ViewportManager;

class ScoreView : public QGraphicsView {
    Q_OBJECT
public:
    explicit ScoreView(QWidget* parent = nullptr);
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

signals:
    void noteSelected(int position, int voiceIndex);
    void viewportExpanded(const QRectF& newBounds);
    void noteAdded(int pitch, double duration, int position = 0);

public slots:
    void onScoreChanged();
    void onVoiceAdded(const MusicTrainer::music::Voice& voice);
    void onNoteAdded(const MusicTrainer::music::Note& note);
    void checkViewportExpansion();
    void handleNoteAdded(int pitch, double duration, int position = 0);

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
    
    class ScoreViewImpl;
    std::unique_ptr<ScoreViewImpl> m_impl;
    
    std::unique_ptr<NoteGrid> m_noteGrid;
    std::unique_ptr<ViewportManager> m_viewportManager;
    std::shared_ptr<MusicTrainer::music::Score> m_score;
    
    bool m_isSelecting = false;
    QPoint m_lastMousePos;
};

} // namespace MusicTrainer::presentation