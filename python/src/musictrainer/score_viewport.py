"""Piano-roll score viewport widget (PyQt6 scaffold).

Mirrors the responsibilities of the former C++ ``ScoreViewportWidget`` but is
implemented in PyQt. Drawing is intentionally minimal: it demonstrates the
PyQt -> musictrainer_core bridge by reading the bound ``Score`` object and
painting its notes. Full interaction (drag/resize, selection, MIDI playback
cursor) is layered on top of this scaffold incrementally.
"""

from __future__ import annotations

from typing import Callable, Optional

from PyQt6.QtCore import Qt, QPoint, QRect
from PyQt6.QtGui import QColor, QPainter
from PyQt6.QtWidgets import QWidget

from .core import core


class ScoreViewportWidget(QWidget):
    """A simple piano-roll renderer bound to a ``musictrainer_core.Score``."""

    def __init__(self, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self._score = None
        self._engine = None
        # Callback used to request a note addition from the main window.
        self.on_add_note: Optional[Callable[[core.Note], None]] = None

        self.setMouseTracking(True)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

        # Viewport state (previously members of the C++ widget).
        self.horizontal_scroll = 0.0  # beats
        self.vertical_scroll = 60.0   # centre MIDI note
        self.horizontal_zoom = 10.0    # pixels per beat
        self.vertical_zoom = 5.0       # pixels per semitone

    # ------------------------------------------------------------------
    # Model binding
    # ------------------------------------------------------------------
    def set_score(self, score) -> None:
        self._score = score
        self.update()

    def set_rule_engine(self, engine) -> None:
        self._engine = engine

    # ------------------------------------------------------------------
    # Painting
    # ------------------------------------------------------------------
    def paintEvent(self, event) -> None:  # noqa: N802 (Qt signature)
        painter = QPainter(self)
        try:
            painter.setRenderHint(QPainter.RenderHint.Antialiasing)
            painter.fillRect(self.rect(), QColor(Qt.GlobalColor.darkGray))

            if self._score is None:
                return

            width = self.width()
            height = self.height()

            # Horizontal pitch lines + C highlights.
            pixels_per_semitone = self.vertical_zoom
            visible_range = height / pixels_per_semitone
            min_note = max(0, int(self.vertical_scroll - visible_range / 2.0) - 1)
            max_note = min(127, int(self.vertical_scroll + visible_range / 2.0) + 1)
            for midi in range(min_note, max_note + 1):
                y = self._pitch_to_y(midi)
                color = QColor(80, 80, 80) if midi % 12 != 0 else QColor(120, 120, 120)
                painter.setPen(color)
                painter.drawLine(0, y, width, y)

            # Notes.
            painter.setPen(QColor(Qt.GlobalColor.black))
            painter.setBrush(QColor(Qt.GlobalColor.cyan))
            for voice in self._score.get_all_voices():
                for note in voice.notes:
                    rect = self._note_rect(note)
                    painter.drawRect(rect)
        finally:
            painter.end()

    # ------------------------------------------------------------------
    # Interaction (scaffold: click empty space to add a note)
    # ------------------------------------------------------------------
    def mousePressEvent(self, event) -> None:  # noqa: N802 (Qt signature)
        if (
            event.button() == Qt.MouseButton.LeftButton
            and self._score is not None
            and self.on_add_note is not None
        ):
            pos = event.pos()
            beats = (pos.x() / self.horizontal_zoom) + self.horizontal_scroll
            y_center_offset = (self.height() / 2.0) - pos.y()
            midi = int(round(self.vertical_scroll - (y_center_offset / self.vertical_zoom)))
            midi = max(0, min(127, midi))
            try:
                note = core.Note(core.Pitch(midi), core.Position(beats), 1.0, 100, 0)
            except Exception as exc:  # pragma: no cover - defensive
                print(f"[viewport] invalid note: {exc}")
                return
            self.on_add_note(note)
        super().mousePressEvent(event)

    # ------------------------------------------------------------------
    # Coordinate helpers
    # ------------------------------------------------------------------
    def _pitch_to_y(self, midi_note: int) -> int:
        y_center_offset = (midi_note - self.vertical_scroll) * self.vertical_zoom
        return int((self.height() / 2.0) - y_center_offset)

    def _time_to_x(self, beats: float) -> int:
        return int((beats - self.horizontal_scroll) * self.horizontal_zoom)

    def _note_rect(self, note) -> QRect:
        x = self._time_to_x(note.get_start_time().beats)
        y = self._pitch_to_y(note.get_pitch().get_midi_note_number())
        width = max(2, int(note.get_duration() * self.horizontal_zoom))
        height = max(2, int(self.vertical_zoom))
        return QRect(x, y - height // 2, width, height)
