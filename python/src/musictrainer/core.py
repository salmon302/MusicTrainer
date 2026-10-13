"""Convenience re-export of the compiled C++ core bindings.

Importing this module raises a clear, actionable error if the
``musictrainer_core`` native extension has not been built yet. Everything in the
GUI should ``from .core import core`` and then access the bound classes
(``core.Score``, ``core.ConcreteRuleEngine``, ``core.CommandHistory``, ...).
"""

try:
    import musictrainer_core as core
except ImportError as exc:  # pragma: no cover - environment dependent
    raise ImportError(
        "The 'musictrainer_core' native module is not importable. Build the "
        "project with -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON and ensure the "
        "resulting musictrainer_core.{pyd,so} is on the Python path "
        "(e.g. next to this package). See docs/current/PyQtMigration.md."
    ) from exc

__all__ = ["core"]
