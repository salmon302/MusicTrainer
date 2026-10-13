"""Application bootstrap for the PyQt6 MusicTrainer frontend."""

from __future__ import annotations

import sys
from typing import List, Optional

from PyQt6.QtWidgets import QApplication

from .main_window import MainWindow


def main(argv: Optional[List[str]] = None) -> int:
    """Create the QApplication, show the main window, and run the event loop."""
    app = QApplication(sys.argv if argv is None else argv)
    app.setApplicationName("MusicTrainer")
    app.setOrganizationName("MusicTrainer")

    window = MainWindow()
    window.show()

    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
