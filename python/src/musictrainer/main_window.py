"""Main application window (PyQt6 scaffold).

Ports the responsibilities of the former C++ ``MainWindow`` to PyQt while keeping
all logic in the bound C++ core (``musictrainer_core``). This scaffold wires up
menus, toolbars, docks, the score viewport, undo/redo, open/save, and live
validation feedback. Playback and full MIDI input handling are left as follow-up
work (see docs/current/PyQtMigration.md).
"""

from __future__ import annotations

import os
from typing import Optional

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeySequence
from PyQt6.QtWidgets import (
    QApplication,
    QDockWidget,
    QFileDialog,
    QListWidget,
    QListWidgetItem,
    QMainWindow,
    QMessageBox,
    QLabel,
    QToolBar,
    QSpinBox,
)

from .core import core
from .score_viewport import ScoreViewportWidget


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("Music Trainer")
        self.setMinimumSize(900, 600)

        # --- Core components (bound C++ objects) ---
        self.score = core.Score()
        self.engine = core.ConcreteRuleEngine()
        self.history = core.CommandHistory()
        self.midi = core.RtMidiSystem()

        # Register built-in rules + load default preset (mirrors the C++ ctor).
        try:
            core.register_default_rules(self.engine)
            preset_path = os.path.join("presets", "default.preset.json")
            preset = core.PresetLoader.load_from_file(preset_path)
            self.engine.load_rule_set(preset)
        except Exception as exc:  # pragma: no cover - environment dependent
            QMessageBox.critical(
                self, "Initialization Error",
                f"Failed to initialize core components: {exc}",
            )

        self.current_file_path = ""

        # --- UI assembly ---
        self._create_actions()
        self._create_menus()
        self._create_toolbars()
        self._create_statusbar()
        self._create_docks()
        self._setup_central_widget()

        self._validate_and_update_feedback()
        self._update_undo_redo_actions()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------
    def _create_actions(self) -> None:
        self.open_act = QAction("&Open...", self)
        self.open_act.setShortcut(QKeySequence.StandardKey.Open)
        self.open_act.triggered.connect(self.open_file_action)

        self.save_act = QAction("&Save", self)
        self.save_act.setShortcut(QKeySequence.StandardKey.Save)
        self.save_act.triggered.connect(self.save_file_action)

        self.exit_act = QAction("E&xit", self)
        self.exit_act.setShortcut(QKeySequence.StandardKey.Quit)
        self.exit_act.triggered.connect(self.close)

        self.undo_act = QAction("&Undo", self)
        self.undo_act.setShortcut(QKeySequence.StandardKey.Undo)
        self.undo_act.triggered.connect(self.undo)
        self.undo_act.setEnabled(False)

        self.redo_act = QAction("&Redo", self)
        self.redo_act.setShortcut(QKeySequence.StandardKey.Redo)
        self.redo_act.triggered.connect(self.redo)
        self.redo_act.setEnabled(False)

        self.play_act = QAction("&Play", self)
        self.stop_act = QAction("&Stop", self)
        self.stop_act.setEnabled(False)

    def _create_menus(self) -> None:
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction(self.open_act)
        file_menu.addAction(self.save_act)
        file_menu.addSeparator()
        file_menu.addAction(self.exit_act)

        edit_menu = self.menuBar().addMenu("&Edit")
        edit_menu.addAction(self.undo_act)
        edit_menu.addAction(self.redo_act)

        self.view_menu = self.menuBar().addMenu("&View")
        self.menuBar().addMenu("&Help")

    def _create_toolbars(self) -> None:
        transport = QToolBar("Transport", self)
        transport.setObjectName("TransportToolBar")
        transport.addAction(self.play_act)
        transport.addAction(self.stop_act)
        self.addToolBar(transport)

        tempo_bar = QToolBar("Tempo", self)
        tempo_bar.setObjectName("TempoToolBar")
        tempo_label = QLabel(" Tempo (BPM): ")
        self.tempo_spin = QSpinBox()
        self.tempo_spin.setRange(30, 240)
        self.tempo_spin.setValue(120)
        tempo_bar.addWidget(tempo_label)
        tempo_bar.addWidget(self.tempo_spin)
        self.addToolBar(tempo_bar)

    def _create_statusbar(self) -> None:
        self.statusBar().showMessage("Ready")

    def _create_docks(self) -> None:
        # Feedback panel.
        feedback_dock = QDockWidget("Feedback", self)
        feedback_dock.setObjectName("FeedbackDockWidget")
        self.feedback_label = QLabel("Initializing...")
        self.feedback_label.setWordWrap(True)
        self.feedback_label.setAlignment(Qt.AlignmentFlag.AlignTop)
        feedback_dock.setWidget(self.feedback_label)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, feedback_dock)
        self.view_menu.addAction(feedback_dock.toggleViewAction())

        # Exercise browser.
        exercise_dock = QDockWidget("Exercises", self)
        exercise_dock.setObjectName("ExerciseDockWidget")
        self.exercise_list = QListWidget(exercise_dock)
        self.exercise_list.setToolTip("Double-click an exercise to load it.")
        exercise_dir = "exercises"
        if os.path.isdir(exercise_dir):
            for name in sorted(os.listdir(exercise_dir)):
                if name.endswith(".mtx"):
                    item = QListWidgetItem(os.path.splitext(name)[0])
                    item.setData(Qt.ItemDataRole.UserRole, os.path.join(exercise_dir, name))
                    self.exercise_list.addItem(item)
        self.exercise_list.itemDoubleClicked.connect(self.load_selected_exercise)
        exercise_dock.setWidget(self.exercise_list)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, exercise_dock)
        self.view_menu.addAction(exercise_dock.toggleViewAction())

    def _setup_central_widget(self) -> None:
        self.viewport = ScoreViewportWidget(self)
        self.viewport.set_score(self.score)
        self.viewport.set_rule_engine(self.engine)
        self.viewport.on_add_note = self.request_note_addition
        self.setCentralWidget(self.viewport)

    # ------------------------------------------------------------------
    # File actions
    # ------------------------------------------------------------------
    def open_file_action(self) -> None:
        path, _ = QFileDialog.getOpenFileName(
            self, "Open Score", self.current_file_path,
            "Music Trainer Files (*.mtx);;All Files (*)",
        )
        if not path:
            return
        try:
            self.score = core.ScoreLoader.load_from_file(path)
            self.current_file_path = path
            self.setWindowTitle(f"Music Trainer - {os.path.basename(path)}")
            self.history.clear()
            self.viewport.set_score(self.score)
            self._validate_and_update_feedback()
            self._update_undo_redo_actions()
            self.statusBar().showMessage(f"Opened {os.path.basename(path)}", 5000)
        except Exception as exc:  # pragma: no cover - environment dependent
            QMessageBox.critical(self, "Load Error", f"Could not load file:\n{exc}")

    def save_file_action(self) -> None:
        if not self.current_file_path:
            self.save_file_as_action()
            return
        self._save_score_to_file(self.current_file_path)

    def save_file_as_action(self) -> None:
        path, _ = QFileDialog.getSaveFileName(
            self, "Save Score As", self.current_file_path,
            "Music Trainer Files (*.mtx);;All Files (*)",
        )
        if path:
            self._save_score_to_file(path)

    def _save_score_to_file(self, path: str) -> None:
        try:
            core.ScoreLoader.save_to_file(self.score, path)
            self.current_file_path = path
            self.setWindowTitle(f"Music Trainer - {os.path.basename(path)}")
            self.statusBar().showMessage(f"Saved to {os.path.basename(path)}", 5000)
        except Exception as exc:  # pragma: no cover - environment dependent
            QMessageBox.critical(self, "Save Error", f"Could not save file:\n{exc}")

    def load_selected_exercise(self, item: QListWidgetItem) -> None:
        path = item.data(Qt.ItemDataRole.UserRole)
        if not path:
            return
        try:
            self.score = core.ScoreLoader.load_from_file(path)
            self.current_file_path = path
            self.setWindowTitle(f"Music Trainer - {item.text()}")
            self.history.clear()
            self.viewport.set_score(self.score)
            self._validate_and_update_feedback()
            self._update_undo_redo_actions()
        except Exception as exc:  # pragma: no cover - environment dependent
            QMessageBox.critical(self, "Load Error", f"Could not load exercise:\n{exc}")

    # ------------------------------------------------------------------
    # Note editing (delegated to the command history on the core)
    # ------------------------------------------------------------------
    def request_note_addition(self, note: core.Note) -> None:
        core.add_note(self.history, self.score, note)
        self._after_score_modified()

    def undo(self) -> None:
        if self.history.can_undo():
            self.history.undo()
            self._after_score_modified()

    def redo(self) -> None:
        if self.history.can_redo():
            self.history.redo()
            self._after_score_modified()

    def _after_score_modified(self) -> None:
        self._validate_and_update_feedback()
        self.viewport.update()
        self._update_undo_redo_actions()

    # ------------------------------------------------------------------
    # Validation feedback
    # ------------------------------------------------------------------
    def _validate_and_update_feedback(self) -> None:
        if self.feedback_label is None:
            return
        result = self.engine.validate_score(self.score)
        if not result.violations:
            self.feedback_label.setText("No rule violations detected.")
            return
        lines = [f"Violations ({len(result.violations)}):"]
        for violation in result.violations[:10]:
            level = "Error" if violation.severity == core.Severity.ERROR else "Warning"
            lines.append(f"- [{violation.rule_id}] {violation.description} (Severity: {level})")
        if len(result.violations) > 10:
            lines.append("... (more violations exist)")
        self.feedback_label.setText("\n".join(lines))

    def _update_undo_redo_actions(self) -> None:
        self.undo_act.setEnabled(self.history.can_undo())
        self.redo_act.setEnabled(self.history.can_redo())
