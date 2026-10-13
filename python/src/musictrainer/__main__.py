"""Allow ``python -m musictrainer`` to launch the PyQt6 frontend."""

from .app import main

if __name__ == "__main__":
    raise SystemExit(main())
