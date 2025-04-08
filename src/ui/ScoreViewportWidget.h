#ifndef MUSIC_TRAINER_UI_SCOREVIEWPORTWIDGET_H
#define MUSIC_TRAINER_UI_SCOREVIEWPORTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
// Consider QAbstractScrollArea for easier scrollbar integration later
// #include <QAbstractScrollArea>

// Forward declarations
namespace MusicTrainer { namespace Domain { class Score; class Note; class Position; } }

namespace MusicTrainer {
namespace UI {

/**
 * @brief Widget responsible for rendering the musical score (piano roll style)
 *        and handling user interactions like scrolling, zooming, and note selection/editing.
 */
class ScoreViewportWidget : public QWidget // Or QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit ScoreViewportWidget(QWidget *parent = nullptr);
    ~ScoreViewportWidget() override = default;

    // Set the score data to be displayed
    void setScore(const Domain::Score* scoreData);

    // TODO: Add methods for setting zoom levels, scroll position, visible range etc.
    // void setHorizontalZoom(double factor);
    // void setVerticalZoom(double factor);
    // void scrollToPosition(const Domain::Position& pos);

    // Set the current playback position to draw the cursor
    void setPlaybackPosition(const std::optional<Domain::Position>& pos);

protected:
    // Qt Event Handlers
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override; // Added key press handler

    // Set a pointer to the main window to request actions
    void setMainWindow(MainWindow* mw);

private:
    // Helper methods for drawing
    void drawGrid(QPainter& painter);
    void drawNotes(QPainter& painter);
    void drawSignatures(QPainter& painter);
    void drawPlaybackCursor(QPainter& painter); // If needed
    void drawSelectionRectangle(QPainter& painter); // If needed

    // Helper methods for coordinate transformations
    QPointF musicalToWidget(const Domain::Position& time, const Domain::Pitch& pitch) const;
    std::pair<Domain::Position, Domain::Pitch> widgetToMusical(const QPointF& point) const;

    // Helper method to find note at a screen point
    const Domain::Note* getNoteAtPoint(const QPoint& point) const;

    // Helper method to get the screen rectangle for a note
    QRectF getNoteRect(const Domain::Note* note, bool useDragOffset = false) const;

    // Helper method to get the screen rectangle for a note's resize handle
    QRectF getNoteResizeHandleRect(const Domain::Note* note) const;


    // --- Member Variables ---
    const Domain::Score* currentScore = nullptr; // Pointer to the score data (read-only view)

    // Viewport state
    double horizontalScroll = 0.0; // Scroll position in beats
    double verticalScroll = 60.0;  // Scroll position (e.g., center MIDI note)
    double horizontalZoom = 10.0;  // Pixels per beat (example)
    double verticalZoom = 5.0;     // Pixels per semitone (example)

    // Interaction state
    bool isDragging = false;
    QPoint dragStartPosition;
    // Add state for note selection, resizing, etc.
    std::vector<const Domain::Note*> selectedNotes; // Pointers to selected notes

    // State for dragging notes
    bool isDraggingNotes = false;
    QPoint dragStartPoint; // Screen coordinates where drag started
    // Store original positions/pitches of notes being dragged
    std::map<const Domain::Note*, Domain::Position> dragNotesInitialPos;
    std::map<const Domain::Note*, Domain::Pitch> dragNotesInitialPitch;
    // Store temporary offsets during drag for visual feedback
    double dragTimeOffsetBeats = 0.0;
    int dragPitchOffsetSemitones = 0;

    // State for resizing notes
    bool isResizingNote = false;
    const Domain::Note* noteBeingResized = nullptr;
    Domain::Position resizeNoteOriginalEnd;
    double resizeNoteTemporaryDuration = 0.0;

    // Playback cursor state
    std::optional<Domain::Position> currentPlaybackPosition;
    bool showPlaybackCursor = false;


    MainWindow* mainWindowPtr = nullptr; // Pointer back to main window

}; // class ScoreViewportWidget

} // namespace UI
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_UI_SCOREVIEWPORTWIDGET_H