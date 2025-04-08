#include "ScoreViewportWidget.h"
#include "../domain/Score.h"
#include "../domain/Note.h"
#include "../domain/Pitch.h"
#include "../domain/Position.h"
#include "MainWindow.h" // Include full definition for method call
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QKeyEvent> // Include for keyPressEvent
#include <iostream> // For debug output
#include <cmath> // For calculations
#include <algorithm> // For std::find, std::max, std::min
#include <vector>
#include <map>

namespace MusicTrainer {
namespace UI {

ScoreViewportWidget::ScoreViewportWidget(QWidget *parent)
    : QWidget(parent)
{
    // Enable mouse tracking to receive mouseMoveEvents even when no button is pressed
    setMouseTracking(true);

    // Set focus policy to accept keyboard events
    setFocusPolicy(Qt::StrongFocus);

    // Initial background color (optional)
    // setAutoFillBackground(true);
    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, Qt::white);
    // setPalette(pal);

     std::cout << "ScoreViewportWidget created." << std::endl;
}

void ScoreViewportWidget::setMainWindow(MainWindow* mw) {
    mainWindowPtr = mw;
}

void ScoreViewportWidget::setScore(const Domain::Score* scoreData) {
    currentScore = scoreData;
    // TODO: Reset view state (scroll, zoom?) or update based on score content
    selectedNotes.clear(); // Clear selection when score changes
    update(); // Trigger repaint

void ScoreViewportWidget::setPlaybackPosition(const std::optional<Domain::Position>& pos) {
    currentPlaybackPosition = pos;
    showPlaybackCursor = pos.has_value();
    update(); // Trigger repaint to show/hide/move cursor
}

}

// --- Event Handlers ---

void ScoreViewportWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event); // Mark parameter as unused for now
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background (optional, can be done with stylesheets too)
    painter.fillRect(rect(), Qt::darkGray); // Example background

    // TODO: Implement drawing logic based on viewport state (scroll, zoom)
    drawGrid(painter);
    if (currentScore) {
        drawNotes(painter);
        drawSignatures(painter);
    }
    drawPlaybackCursor(painter); // Draw cursor on top
    // drawSelectionRectangle(painter);

    // Example: Draw placeholder text
    // painter.setPen(Qt::white);
    // painter.drawText(rect(), Qt::AlignCenter, "Score Viewport - Drawing Pending");
}

void ScoreViewportWidget::mousePressEvent(QMouseEvent *event) {
    std::cout << "Mouse Press at: (" << event->pos().x() << ", " << event->pos().y() << ")" << std::endl;

    if (event->button() == Qt::LeftButton) {

        // Check if clicking on a resize handle first
        isResizingNote = false; // Reset flag
        noteBeingResized = nullptr;
        if (!selectedNotes.empty()) { // Can only resize selected notes
            for (const auto* notePtr : selectedNotes) {
                 if (notePtr) {
                     QRectF handleRect = getNoteResizeHandleRect(notePtr);
                     if (handleRect.contains(QPointF(event->pos()))) {
                         // Start resizing this note
                         isResizingNote = true;
                         noteBeingResized = notePtr;
                         resizeNoteOriginalEnd = notePtr->getEndTime();
                         resizeNoteTemporaryDuration = notePtr->getDuration(); // Start with original
                         dragStartPoint = event->pos(); // Use drag start for reference
                         std::cout << "Starting resize on note." << std::endl;
                         // Prevent starting a note drag or changing selection
                         event->accept(); // Indicate we handled this press
                         return; // Don't process further for selection/add
                     }
                 }
            }
        }

        // If not resizing, proceed with selection / potential drag start / add note
        const Domain::Note* clickedNote = getNoteAtPoint(event->pos());

        if (clickedNote) {
            // --- Note Selection Logic (FV-15, FV-16) ---
            bool shiftPressed = (event->modifiers() & Qt::ShiftModifier);

            // Check if already selected
            auto it = std::find(selectedNotes.begin(), selectedNotes.end(), clickedNote);
            bool alreadySelected = (it != selectedNotes.end());

            if (!shiftPressed) {
                // If Shift is not pressed, clear previous selection unless clicking an already selected note
                if (!alreadySelected || selectedNotes.size() > 1) {
                     selectedNotes.clear();
                     selectedNotes.push_back(clickedNote);
                     std::cout << "Selected single note." << std::endl;
                     update(); // Repaint needed for selection change
                }
                // If clicking the single already selected note, do nothing (allows starting drag)
            } else {
                // If Shift is pressed, toggle selection
                if (alreadySelected) {
                    selectedNotes.erase(it);
                     std::cout << "Deselected note (Shift)." << std::endl;
                } else {
                    selectedNotes.push_back(clickedNote);
                     std::cout << "Added note to selection (Shift)." << std::endl;
                }
                update(); // Repaint needed for selection change
            }
             // Start dragging if clicking on an already selected note
             // Need to check if alreadySelected *before* potential modification by shift-click logic
             bool shouldStartDrag = false;
             if (!shiftPressed && alreadySelected) { // Clicked on single selected note
                 shouldStartDrag = true;
             } else if (shiftPressed && !alreadySelected) { // Shift-clicked to add this note to selection
                 // Check if any *other* selected note was already selected before this click
                 // This logic is getting complex, maybe simplify drag start condition?
                 // For now, let's only allow drag if single-clicking an already selected note.
                 // shouldStartDrag = true; // If wanting to drag multi-selection immediately
             } else if (shiftPressed && alreadySelected) {
                 // Shift-clicked to deselect, don't start drag
                 shouldStartDrag = false;
             }


             if (shouldStartDrag) {
                  std::cout << "Starting drag on selected note(s)." << std::endl;
                  isDraggingNotes = true;
                  dragStartPoint = event->pos();
                  // Store initial state of selected notes
                  dragNotesInitialPos.clear();
                  dragNotesInitialPitch.clear();
                  dragTimeOffsetBeats = 0.0;
                  dragPitchOffsetSemitones = 0;
                  for (const auto* notePtr : selectedNotes) {
                      if (notePtr) {
                          dragNotesInitialPos[notePtr] = notePtr->getStartTime();
                          dragNotesInitialPitch[notePtr] = notePtr->getPitch();
                      }
                  }
             }

        } else {
             // --- Clicked on Empty Space ---
            // Clear selection
            if (!selectedNotes.empty()) {
                selectedNotes.clear();
                std::cout << "Cleared selection." << std::endl;
                update(); // Repaint needed
            }

            // --- Request Add Note on Click (FN-01) ---
            if (mainWindowPtr && currentScore) { // Need main window and score
                // Convert click position to musical coordinates
                std::pair<Domain::Position, Domain::Pitch> musicalPos = widgetToMusical(event->pos());

                // TODO: Implement snapping to grid (time quantization)
                // TODO: Get default duration, velocity, and target voice from UI/settings
                double defaultDuration = 1.0;
                int defaultVelocity = 100;
                int targetVoiceId = 0; // Default to voice 0

                try {
                    Note potentialNote(musicalPos.second, musicalPos.first, defaultDuration, defaultVelocity, targetVoiceId);

                    // Delegate the actual addition request to MainWindow
                    mainWindowPtr->requestNoteAddition(potentialNote);

                     std::cout << "Requesting Note Addition: " << potentialNote.getPitch().debugName()
                               << " at beat " << potentialNote.getStartTime().beats
                           << " to voice " << targetVoiceId << std::endl;

                // NOTE: update() is no longer called here directly. MainWindow should trigger it
                // after the score is actually modified.

            } catch (const std::exception& e) {
                std::cerr << "Error creating potential note: " << e.what() << std::endl;
            }
        } else {
             std::cerr << "Cannot request note addition: Missing MainWindow pointer or Score." << std::endl;
        }

        // --- Other potential actions (Selection Rectangle Drag) ---
        // dragStartPosition = event->pos();
        // isDragging = true; // Set interaction mode

    }
    } else {
        // Handle other mouse buttons if needed
    }

    // QWidget::mousePressEvent(event); // Decide whether to call base class
    event->accept(); // Indicate we handled it (or might handle it)
}

void ScoreViewportWidget::mouseMoveEvent(QMouseEvent *event) {
     // std::cout << "Mouse Move at: (" << event->pos().x() << ", " << event->pos().y() << ")" << std::endl; // Debug

     if (isDraggingNotes) {
         // Calculate screen offset
         QPoint currentPos = event->pos();
         QPoint offset = currentPos - dragStartPoint;

         // Convert screen offset to musical offset
         // Horizontal offset (Time)
         dragTimeOffsetBeats = static_cast<double>(offset.x()) / horizontalZoom;
         // TODO: Implement time snapping here based on grid/zoom level

         // Vertical offset (Pitch)
         // Y increases downwards, Pitch increases upwards
         double pitchOffsetExact = static_cast<double>(-offset.y()) / verticalZoom;
         // Snap to nearest semitone
         dragPitchOffsetSemitones = static_cast<int>(std::round(pitchOffsetExact));

         // Trigger repaint to show notes at dragged position
         update();
         event->accept();
     } else if (isResizingNote && noteBeingResized) {
         // Calculate target end position based on mouse X
         QPoint currentPos = event->pos();
         // Use widgetToMusical to find the beat corresponding to the mouse X
         // We only care about the time component here
         double targetEndBeat = widgetToMusical(QPointF(currentPos)).first.beats;

         // Calculate new duration (must be positive)
         double newDuration = targetEndBeat - noteBeingResized->getStartTime().beats;
         // TODO: Add snapping for duration based on grid/zoom

         // Apply minimum duration (e.g., 1/16th note or smallest representable unit)
         const double minDuration = 0.1; // Example minimum duration in beats
         resizeNoteTemporaryDuration = std::max(minDuration, newDuration);

         // Trigger repaint to show note with new temporary duration
         update();
         event->accept();

     } else if (isDragging) { // Handle other potential drag modes (e.g., selection rect)
         // ...
         // event->accept();
     } else {
        // Optionally handle hover effects here if needed, using setMouseTracking(true)
        QWidget::mouseMoveEvent(event); // Pass to base if not handled
     }
}

void ScoreViewportWidget::mouseReleaseEvent(QMouseEvent *event) {
     std::cout << "Mouse Release at: (" << event->pos().x() << ", " << event->pos().y() << ")" << std::endl;

     if (event->button() == Qt::LeftButton) {
         if (isDraggingNotes) {
             // Finalize note drag
             isDraggingNotes = false;

             // Check if there was a significant drag (offsets are non-zero)
             if (std::abs(dragTimeOffsetBeats) > 1e-6 || dragPitchOffsetSemitones != 0) {
                 // Create copies of the notes to be moved (using original positions/pitches)
                 std::vector<Domain::Note> notesToMove;
                 notesToMove.reserve(selectedNotes.size());
                 for(const auto* notePtr : selectedNotes) {
                     if(notePtr) {
                         // Reconstruct the note as it was *before* the drag started
                         // Note: This assumes duration/velocity/voice didn't change during drag
                         notesToMove.emplace_back(dragNotesInitialPitch.at(notePtr),
                                                  dragNotesInitialPos.at(notePtr),
                                                  notePtr->getDuration(),
                                                  notePtr->getVelocity(),
                                                  notePtr->getVoiceId());
                     }
                 }

                 // Request the move via MainWindow, passing original notes and the offsets
                 if (mainWindowPtr && !notesToMove.empty()) {
                     std::cout << "Requesting Note Move: dt=" << dragTimeOffsetBeats
                                   << ", dp=" << dragPitchOffsetSemitones << std::endl;
                     // Call the MainWindow method to execute the move command
                     mainWindowPtr->requestNotesMove(notesToMove, dragTimeOffsetBeats, dragPitchOffsetSemitones);
                 }
             } else {
                  std::cout << "Note drag ended with no significant movement." << std::endl;
             }

             // Clear drag state regardless of movement
             dragNotesInitialPos.clear();
             dragNotesInitialPitch.clear();
             dragTimeOffsetBeats = 0.0;
             dragPitchOffsetSemitones = 0;

             // Trigger a final repaint (MainWindow should handle this after command execution)
             // update(); // Removed - MainWindow triggers update

             event->accept();

         } else if (isResizingNote && noteBeingResized) {
             // Finalize note resize
             isResizingNote = false;
             double finalDuration = resizeNoteTemporaryDuration;

             // Check if duration actually changed significantly
             if (std::abs(finalDuration - noteBeingResized->getDuration()) > 1e-6) {
                 // Create copy of the original note
                 Domain::Note originalNote = *noteBeingResized;

                 // Request the resize via MainWindow
                 if (mainWindowPtr) {
                      std::cout << "Requesting Note Resize: Orig Dur=" << originalNote.getDuration()
                                << ", New Dur=" << finalDuration << std::endl;
                     // Call the MainWindow method to execute the resize command
                     mainWindowPtr->requestNoteResize(originalNote, finalDuration);
                 }
             } else {
                  std::cout << "Note resize ended with no significant change." << std::endl;
             }

             // Clear resize state
             noteBeingResized = nullptr;
             resizeNoteTemporaryDuration = 0.0;

             // Trigger final repaint (MainWindow should handle this)
             // update(); // Removed

             event->accept();

         } else if (isDragging) { // Handle other drag modes ending
             isDragging = false;
             // update();
             event->accept();
         } else {
              QWidget::mouseReleaseEvent(event); // Pass to base if not handled
         }
     } else {
          QWidget::mouseReleaseEvent(event); // Pass other buttons to base
     }
}

void ScoreViewportWidget::wheelEvent(QWheelEvent *event) {
    // TODO: Implement zooming or scrolling based on wheel event
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        // Conventional scroll wheel (vertical)
        if (event->modifiers() & Qt::ControlModifier) {
             // Zoom (Vertical or Horizontal based on preference?)
             double zoomFactor = 1.0 + (numDegrees.y() > 0 ? 0.1 : -0.1);
             verticalZoom *= zoomFactor; // Example: vertical zoom
             std::cout << "Zooming (V): " << verticalZoom << std::endl;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            // Scroll Horizontally
            horizontalScroll -= numDegrees.y() * 1.0; // Adjust multiplier as needed
             std::cout << "Scrolling (H): " << horizontalScroll << std::endl;
        } else {
            // Scroll Vertically
            verticalScroll -= numDegrees.y() * 1.0; // Adjust multiplier as needed
             std::cout << "Scrolling (V): " << verticalScroll << std::endl;
        }
        update(); // Trigger repaint after scroll/zoom
    } else if (!numPixels.isNull()) {
        // High-precision touchpad scroll (can be horizontal or vertical)
        // TODO: Implement smooth scrolling based on pixelDelta
    }

    event->accept(); // Indicate event was handled
    // QWidget::wheelEvent(event); // Don't call base if we handle it fully
}

void ScoreViewportWidget::resizeEvent(QResizeEvent *event) {
    // TODO: Handle widget resize, might affect visible range or calculations
    std::cout << "Viewport Resized to: " << event->size().width() << "x" << event->size().height() << std::endl;
    QWidget::resizeEvent(event);
}

void ScoreViewportWidget::keyPressEvent(QKeyEvent *event) {
     std::cout << "Key Press: " << event->key() << std::endl; // Debug
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (!selectedNotes.empty() && mainWindowPtr) {
             std::cout << "Requesting deletion of " << selectedNotes.size() << " notes." << std::endl;
            // Delegate deletion request to MainWindow
            mainWindowPtr->requestSelectedNotesDeletion(selectedNotes); // Pass the vector of pointers

            // Clear local selection after requesting deletion
            selectedNotes.clear();
            // No need to call update() here, MainWindow will trigger it after score modification
            event->accept(); // Indicate we handled the key press
        } else {
             QWidget::keyPressEvent(event); // Pass unhandled keys to base class
        }
    } else {
        // Handle other key presses if needed (e.g., navigation)
        QWidget::keyPressEvent(event); // Pass unhandled keys to base class
    }
}

// --- Private Drawing Helpers ---

void ScoreViewportWidget::drawGrid(QPainter& painter) {
    // Define colors
    QColor measureLineColor = QColor(120, 120, 120);
    QColor beatLineColor = QColor(90, 90, 90);
    QColor pitchLineColor = QColor(100, 100, 100); // Example: C lines darker?

    // --- Vertical Lines (Time) ---
    // Determine visible time range in beats
    // TODO: Need widgetToMusical for accurate range calculation
    double beatsPerPixel = 1.0 / horizontalZoom;
    double visibleStartBeat = horizontalScroll; // Simplification
    double visibleEndBeat = horizontalScroll + width() * beatsPerPixel; // Simplification

    // Get time signature active at the start of the view (simplification)
    TimeSignature ts = currentScore ? currentScore->getTimeSignatureAt(Position(visibleStartBeat)) : TimeSignature::CommonTime();
    double beatsPerMeasure = ts.getMeasureDurationInBeats(); // Assuming quarter note beat for now

    // Find the first measure line visible
    double firstMeasureBeat = std::floor(visibleStartBeat / beatsPerMeasure) * beatsPerMeasure;

    for (double beat = firstMeasureBeat; beat < visibleEndBeat; beat += 1.0) { // Iterate beats
        Position currentPos(beat);
        QPointF screenPos = musicalToWidget(currentPos, Pitch(60)); // Use arbitrary pitch for X coord

        // Check if it's a measure line
        // Use tolerance for floating point comparison with modulo
        bool isMeasureStart = std::fmod(beat + 1e-9, beatsPerMeasure) < 1e-6;

        if (isMeasureStart) {
            painter.setPen(QPen(measureLineColor, 1.5)); // Thicker line for measures
        } else {
            painter.setPen(QPen(beatLineColor, 0.5)); // Thinner line for beats
        }

        // Draw line if within widget bounds (painter might clip anyway)
        if (screenPos.x() >= 0 && screenPos.x() <= width()) {
             painter.drawLine(QPointF(screenPos.x(), 0), QPointF(screenPos.x(), height()));
        }

        // TODO: Add subdivision lines based on zoom level
    }


    // --- Horizontal Lines (Pitch) ---
    // Determine visible pitch range (MIDI notes)
    // TODO: Need widgetToMusical for accurate range calculation
    double pixelsPerSemitone = verticalZoom;
    double visibleCenterPitch = verticalScroll; // Simplification
    double visiblePitchRange = height() / pixelsPerSemitone;
    int minMidiNote = static_cast<int>(std::floor(visibleCenterPitch - visiblePitchRange / 2.0));
    int maxMidiNote = static_cast<int>(std::ceil(visibleCenterPitch + visiblePitchRange / 2.0));

    minMidiNote = std::max(0, minMidiNote);   // Clamp to MIDI range 0-127
    maxMidiNote = std::min(127, maxMidiNote);

    for (int midiNote = minMidiNote; midiNote <= maxMidiNote; ++midiNote) {
        Pitch currentPitch(midiNote);
        QPointF screenPos = musicalToWidget(Position(visibleStartBeat), currentPitch); // Use arbitrary time for Y coord

        // Example: Highlight C notes
        if (midiNote % 12 == 0) { // C, C#, D, D#, E, F, F#, G, G#, A, A#, B -> 0, 1, ... 11
             painter.setPen(QPen(pitchLineColor.darker(150), 1.0)); // Darker line for C
        } else {
             painter.setPen(QPen(pitchLineColor, 0.5));
        }

        // Draw line if within widget bounds
         if (screenPos.y() >= 0 && screenPos.y() <= height()) {
            painter.drawLine(QPointF(0, screenPos.y()), QPointF(width(), screenPos.y()));
         }
         // TODO: Add lines for black keys (e.g., slightly different color/style)
    }
}

void ScoreViewportWidget::drawNotes(QPainter& painter) {
    if (!currentScore) {
        return;
    }

    // Define note appearance
    QColor noteFillColor = Qt::cyan;
    QColor noteOutlineColor = Qt::black;
    // painter.setPen(noteOutlineColor); // Pen is set inside loop now

    // Determine visible time/pitch range (approximate for now)
    // TODO: Use widgetToMusical for accurate range calculation
    double beatsPerPixel = 1.0 / horizontalZoom;
    double visibleStartBeat = horizontalScroll;
    double visibleEndBeat = horizontalScroll + width() * beatsPerPixel;
    double pixelsPerSemitone = verticalZoom;
    double visibleCenterPitch = verticalScroll;
    double visiblePitchRange = height() / pixelsPerSemitone;
    int minVisibleMidi = static_cast<int>(std::floor(visibleCenterPitch - visiblePitchRange / 2.0)) - 1; // Add buffer
    int maxVisibleMidi = static_cast<int>(std::ceil(visibleCenterPitch + visiblePitchRange / 2.0)) + 1;  // Add buffer

    // Iterate through voices and notes
    for (const auto& voice : currentScore->getAllVoices()) {
        // TODO: Use different colors per voice (FV-02)
        // noteFillColor = getVoiceColor(voice.id);
        QColor currentVoiceNoteFillColor = noteFillColor; // Base color for this voice

        for (const auto& note : voice.notes) {
            // Basic visibility check (culling)
            if (note.getEndTime().beats < visibleStartBeat || note.getStartTime().beats > visibleEndBeat) {
                continue; // Note is completely outside the horizontal view
            }
            int noteMidi = note.getPitch().getMidiNoteNumber();
            if (noteMidi < minVisibleMidi || noteMidi > maxVisibleMidi) {
                 continue; // Note is completely outside the vertical view
            }

            // Determine if this note is being dragged (needed before getNoteRect)
            bool isBeingDragged = false;
            if (isDraggingNotes) {
                auto posIt = dragNotesInitialPos.find(&note);
                if (posIt != dragNotesInitialPos.end()) {
                    isBeingDragged = true;
                }
            }

            // Calculate screen rectangle using helper function
            // Pass 'true' to use temporary drag/resize offsets for drawing feedback
            QRectF noteRect = getNoteRect(&note, true);
            if (noteRect.isNull()) continue; // Skip if rect calculation failed

            // Consider velocity for color intensity (FV-07) - simple example
            int alpha = 100 + (note.getVelocity() * 155 / 127); // Map velocity 0-127 to alpha 100-255
            QColor currentNoteFillColor = currentVoiceNoteFillColor; // Start with voice color
            currentNoteFillColor.setAlpha(std::min(255, alpha));


            // Check if note is selected OR being dragged and adjust appearance (FV-17)
            bool isSelected = std::find(selectedNotes.begin(), selectedNotes.end(), &note) != selectedNotes.end();
            if (isSelected || isBeingDragged) {
                QPen selectionPen(Qt::yellow, isBeingDragged ? 1 : 2); // Thinner line while dragging?
                painter.setPen(selectionPen);
                // Make fill semi-transparent while dragging or resizing
                if (isBeingDragged || (isResizingNote && &note == noteBeingResized)) {
                    currentNoteFillColor.setAlpha(150);
                }
            } else {
                 painter.setPen(noteOutlineColor); // Reset to default outline
            }

            painter.setBrush(currentNoteFillColor); // Set brush potentially modified by dragging/resizing alpha
            painter.drawRect(noteRect);

            // Draw resize handle if note is selected and not currently being dragged or resized
            if (isSelected && !isBeingDragged && !(isResizingNote && &note == noteBeingResized)) {
                 QRectF handleRect = getNoteResizeHandleRect(&note);
                 painter.setPen(Qt::yellow);
                 painter.setBrush(Qt::yellow);
                 painter.drawRect(handleRect);
            }

            // TODO: Draw note names or other details inside/near the rect if zoom allows

             // Reset pen if it was changed for selection/dragging
             if (isSelected || isBeingDragged) {
                 painter.setPen(noteOutlineColor);
             }
        }
    }
}

void ScoreViewportWidget::drawSignatures(QPainter& painter) {
    // TODO: Draw time and key signatures at appropriate positions
}

void ScoreViewportWidget::drawPlaybackCursor(QPainter& painter) {
    // TODO: Draw a vertical line indicating current playback time
}

void ScoreViewportWidget::drawSelectionRectangle(QPainter& painter) {
    // TODO: Draw the rectangle if user is dragging a selection
}

// --- Private Coordinate Helpers ---

QPointF ScoreViewportWidget::musicalToWidget(const Domain::Position& time, const Domain::Pitch& pitch) const {
    // X coordinate: based on time (beats), horizontal scroll, and zoom
    // 0,0 is top-left
    double x = (time.beats - horizontalScroll) * horizontalZoom;

    // Y coordinate: based on MIDI note number, vertical scroll, and zoom
    // Higher MIDI notes should be higher on screen (lower Y value)
    // We need a reference pitch for the vertical scroll (e.g., verticalScroll is the MIDI note at the top edge, or center?)
    // Let's assume verticalScroll is the MIDI note number visible at the *center* of the widget height for now.
    double y_center_offset = (static_cast<double>(pitch.getMidiNoteNumber()) - verticalScroll) * verticalZoom;

void ScoreViewportWidget::drawPlaybackCursor(QPainter& painter) {
    if (!showPlaybackCursor || !currentPlaybackPosition.has_value()) {
        return;
    }

    // Calculate screen X coordinate for the cursor
    QPointF screenPos = musicalToWidget(currentPlaybackPosition.value(), Pitch(60)); // Use arbitrary pitch for X

    // Draw a vertical line
    painter.setPen(QPen(Qt::red, 1.5)); // Example: Red line
    painter.drawLine(QPointF(screenPos.x(), 0), QPointF(screenPos.x(), height()));
}

    double y = (height() / 2.0) - y_center_offset; // Y increases downwards

    return QPointF(x, y);
}

// TODO: Refine this implementation, especially pitch calculation accuracy
std::pair<Domain::Position, Domain::Pitch> ScoreViewportWidget::widgetToMusical(const QPointF& point) const {
    // Inverse X calculation
    double time_beats = (point.x() / horizontalZoom) + horizontalScroll;

    // Inverse Y calculation (based on current musicalToWidget logic where verticalScroll is center pitch)
    double y_center_offset = (height() / 2.0) - point.y();
    double pitch_midi_exact = verticalScroll - (y_center_offset / verticalZoom);

    // Clamp MIDI pitch to valid range [0, 127] and round
    int pitch_midi_rounded = static_cast<int>(std::round(pitch_midi_exact));
    int pitch_midi_clamped = std::max(0, std::min(127, pitch_midi_rounded));

    // Use clamped value, Pitch constructor handles final validation if needed again,
    // though clamping here prevents throwing exceptions during simple coordinate lookup.
    return { Domain::Position(time_beats), Domain::Pitch(pitch_midi_clamped) };
}

// --- Private Helper: Find Note ---

const Domain::Note* ScoreViewportWidget::getNoteAtPoint(const QPoint& point) const {
    if (!currentScore) {
        return nullptr;
    }

    // Iterate through all notes in all voices (could be optimized)
    // Check in reverse order of voices? (e.g., if higher voices overlap lower ones visually)
    for (int v = currentScore->getVoiceCount() - 1; v >= 0; --v) {
        const auto* voice = currentScore->getVoice(v);
        if (!voice) continue;

        // Iterate notes in reverse order? (If later notes draw over earlier ones)
        for (auto it = voice->notes.rbegin(); it != voice->notes.rend(); ++it) {
            const auto& note = *it;

            // Calculate the note's bounding box on screen using helper
            QRectF noteRect = getNoteRect(&note); // Use helper, don't need drag offset for hit testing

            // Check if the point is inside the rectangle
            if (noteRect.contains(QPointF(point))) { // Convert point to QPointF for contains check
                return &note; // Return pointer to the first note found under the cursor
            }
        }
    }

    return nullptr; // No note found at this point
}


// --- Private Helper: Geometry ---

QRectF ScoreViewportWidget::getNoteRect(const Domain::Note* note, bool useDragOffset) const {
    if (!note) return QRectF();

    Domain::Position drawPos = note->getStartTime();
    Domain::Pitch drawPitch = note->getPitch();
    double drawDuration = note->getDuration();

    if (useDragOffset && isDraggingNotes) {
        auto posIt = dragNotesInitialPos.find(note);
        if (posIt != dragNotesInitialPos.end()) {
            drawPos = posIt->second + dragTimeOffsetBeats;
            try {
                 drawPitch = dragNotesInitialPitch.at(note).transpose(dragPitchOffsetSemitones);
            } catch (...) { /* Handle potential out_of_range, use original pitch */ }
        }
    } else if (useDragOffset && isResizingNote && note == noteBeingResized) {
        // Use temporary duration during resize for visual feedback
        drawDuration = resizeNoteTemporaryDuration;
    }

    QPointF topLeft = musicalToWidget(drawPos, drawPitch);
    double noteHeight = verticalZoom;
    double noteWidth = drawDuration * horizontalZoom;
    topLeft.setY(topLeft.y() - noteHeight / 2.0); // Adjust Y to top edge

    return QRectF(topLeft, QSizeF(noteWidth, noteHeight));
}

QRectF ScoreViewportWidget::getNoteResizeHandleRect(const Domain::Note* note) const {
    if (!note) return QRectF();

    QRectF noteRect = getNoteRect(note); // Get rect at current position
    if (noteRect.isNull() || noteRect.isEmpty()) return QRectF();

    // Define handle size (e.g., 6x6 pixels)
    const double handleSize = 6.0;
    // Position handle at the middle of the right edge
    return QRectF(noteRect.right() - handleSize / 2.0,
                  noteRect.center().y() - handleSize / 2.0,
                  handleSize,
                  handleSize);
}


} // namespace UI
} // namespace MusicTrainer