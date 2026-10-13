// pybind11 bindings for the Qt-free MusicTrainer core.
//
// This is a SCAFFOLD pass (see docs/current/PyQtMigration.md). It exposes the
// core API surface that the PyQt6 frontend needs. Items intentionally left for
// the follow-up port:
//   * Full polymorphic IRule registration from Python (only the two built-in
//     rules are registered via register_default_rules()).
//   * MidiSystem input callbacks (std::function wrappers).
//   * Violation::involvedNotes (raw Note pointers) -- omitted for now.
//   * Rule parameters / preset editing APIs.
//
// Build with -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <vector>

#include "domain/Pitch.h"
#include "domain/Position.h"
#include "domain/Note.h"
#include "domain/Score.h"
#include "domain/TimeSignature.h"
#include "domain/KeySignature.h"

#include "rules/ConcreteRuleEngine.h"
#include "rules/ParallelFifthsRule.h"
#include "rules/ParallelOctavesRule.h"
#include "rules/Preset.h"
#include "rules/RuleTypes.h"

#include "io/PresetLoader.h"
#include "io/ScoreLoader.h"

#include "midi/RtMidiSystem.h"

#include "commands/CommandHistory.h"
#include "commands/AddNoteCommand.h"
#include "commands/DeleteNoteCommand.h"
#include "commands/MoveNotesCommand.h"
#include "commands/ResizeNoteCommand.h"

namespace py = pybind11;
using namespace MusicTrainer;

// Register the built-in rule plugins. Mirrors what the former C++ MainWindow
// did explicitly (ParallelFifthsRule + ParallelOctavesRule).
void register_default_rules(Rules::ConcreteRuleEngine& engine) {
    engine.registerRulePlugin(std::make_shared<Rules::ParallelFifthsRule>());
    engine.registerRulePlugin(std::make_shared<Rules::ParallelOctavesRule>());
}

// Command helpers: construct the command and execute it through the history.
// Exposed as module-level functions because pybind11 <-> std::unique_ptr<ICommand>
// argument conversion is more friction than this thin wrapper is worth for a scaffold.
void py_add_note(Commands::CommandHistory& history, Domain::Score& score,
                 const Domain::Note& note) {
    history.executeCommand(std::make_unique<Commands::AddNoteCommand>(score, note));
}

void py_delete_notes(Commands::CommandHistory& history, Domain::Score& score,
                     const std::vector<Domain::Note>& notes) {
    history.executeCommand(std::make_unique<Commands::DeleteNoteCommand>(score, notes));
}

void py_move_notes(Commands::CommandHistory& history, Domain::Score& score,
                   const std::vector<Domain::Note>& originalNotes,
                   double dt, int dp) {
    history.executeCommand(std::make_unique<Commands::MoveNotesCommand>(
        score, originalNotes, dt, dp));
}

void py_resize_note(Commands::CommandHistory& history, Domain::Score& score,
                    const Domain::Note& originalNote, double newDuration) {
    history.executeCommand(std::make_unique<Commands::ResizeNoteCommand>(
        score, originalNote, newDuration));
}

PYBIND11_MODULE(musictrainer_core, m) {
    m.doc() = "Python bindings for the MusicTrainer C++ core "
              "(domain, rules, midi, io, commands). Qt-free.";

    // ---------------- Domain: Pitch ----------------
    py::class_<Domain::Pitch>(m, "Pitch")
        .def(py::init<int>(), py::arg("midi_number"))
        .def(py::init<>())
        .def("get_midi_note_number", &Domain::Pitch::getMidiNoteNumber)
        .def("transpose", &Domain::Pitch::transpose, py::arg("semitones"))
        .def_static("middle_c", &Domain::Pitch::MiddleC)
        .def("debug_name", &Domain::Pitch::debugName)
        .def(py::self == py::self)
        .def("__repr__", [](const Domain::Pitch& p) {
            return "Pitch(" + std::to_string(p.getMidiNoteNumber()) + ")";
        });

    // ---------------- Domain: Position ----------------
    py::class_<Domain::Position>(m, "Position")
        .def(py::init<double>(), py::arg("beats") = 0.0)
        .def(py::init<>())
        .def_readwrite("beats", &Domain::Position::beats)
        .def("__repr__", [](const Domain::Position& p) {
            return "Position(" + std::to_string(p.beats) + ")";
        });

    // ---------------- Domain: Note ----------------
    py::class_<Domain::Note>(m, "Note")
        .def(py::init<const Domain::Pitch&, const Domain::Position&, double, int, int>(),
             py::arg("pitch"), py::arg("start"), py::arg("duration"),
             py::arg("velocity"), py::arg("voice"))
        .def("get_pitch", &Domain::Note::getPitch, py::return_value_policy::reference_internal)
        .def("get_start_time", &Domain::Note::getStartTime, py::return_value_policy::reference_internal)
        .def("get_end_time", &Domain::Note::getEndTime)
        .def("get_duration", &Domain::Note::getDuration)
        .def("get_velocity", &Domain::Note::getVelocity)
        .def("get_voice_id", &Domain::Note::getVoiceId)
        .def(py::self == py::self);

    // ---------------- Domain: TimeSignature / KeySignature ----------------
    py::class_<Domain::TimeSignature>(m, "TimeSignature")
        .def_static("common_time", &Domain::TimeSignature::CommonTime)
        .def_static("cut_time", &Domain::TimeSignature::CutTime)
        .def_static("waltz_time", &Domain::TimeSignature::WaltzTime)
        .def("get_beats_per_measure", &Domain::TimeSignature::getBeatsPerMeasure)
        .def("get_measure_duration_in_beats", &Domain::TimeSignature::getMeasureDurationInBeats);

    py::class_<Domain::KeySignature>(m, "KeySignature")
        .def_static("c_major", &Domain::KeySignature::CMajor)
        .def_static("a_minor", &Domain::KeySignature::AMinor);

    // ---------------- Domain: Score::Voice ----------------
    py::class_<Domain::Score::Voice>(m, "Voice")
        .def_readonly("id", &Domain::Score::Voice::id)
        .def_readonly("name", &Domain::Score::Voice::name)
        .def_readonly("notes", &Domain::Score::Voice::notes);

    // ---------------- Domain: Score ----------------
    py::class_<Domain::Score>(m, "Score")
        .def(py::init<>())
        .def("add_voice", &Domain::Score::addVoice, py::arg("name") = "")
        .def("get_voice_count", &Domain::Score::getVoiceCount)
        .def("get_voice",
             [](const Domain::Score& s, int id) -> const Domain::Score::Voice* {
                 return s.getVoice(id);
             },
             py::arg("voice_id"), py::return_value_policy::reference_internal)
        .def("get_all_voices",
             [](const Domain::Score& s) { return s.getAllVoices(); })
        .def("add_note", &Domain::Score::addNote, py::arg("note"))
        .def("remove_note", &Domain::Score::removeNote, py::arg("note"))
        .def("get_time_signature_at",
             [](const Domain::Score& s, const Domain::Position& p)
                 -> const Domain::TimeSignature& { return s.getTimeSignatureAt(p); },
             py::arg("position"), py::return_value_policy::reference_internal)
        .def("get_key_signature_at",
             [](const Domain::Score& s, const Domain::Position& p)
                 -> const Domain::KeySignature& { return s.getKeySignatureAt(p); },
             py::arg("position"), py::return_value_policy::reference_internal)
        .def("is_empty", &Domain::Score::isEmpty);

    // ---------------- Rules: enums ----------------
    py::enum_<Rules::Severity>(m, "Severity")
        .value("INFO", Rules::Severity::Info)
        .value("WARNING", Rules::Severity::Warning)
        .value("ERROR", Rules::Severity::Error);

    py::enum_<Rules::RuleType>(m, "RuleType")
        .value("MELODIC", Rules::RuleType::Melodic)
        .value("HARMONIC", Rules::RuleType::Harmonic)
        .value("RHYTHMIC", Rules::RuleType::Rhythmic)
        .value("STRUCTURAL", Rules::RuleType::Structural);

    // ---------------- Rules: TimeRange / Violation / ValidationResult ----------------
    py::class_<Rules::TimeRange>(m, "TimeRange")
        .def(py::init<double, double>(), py::arg("start") = 0.0, py::arg("end") = 0.0)
        .def_readwrite("start_time_beats", &Rules::TimeRange::startTimeBeats)
        .def_readwrite("end_time_beats", &Rules::TimeRange::endTimeBeats);

    // involvedNotes (raw Note pointers) is intentionally omitted for the scaffold.
    py::class_<Rules::Violation>(m, "Violation")
        .def_readonly("rule_id", &Rules::Violation::ruleId)
        .def_readonly("description", &Rules::Violation::description)
        .def_readonly("severity", &Rules::Violation::severity)
        .def_readonly("location", &Rules::Violation::location);

    py::class_<Rules::ValidationResult>(m, "ValidationResult")
        .def(py::init<>())
        .def_readonly("violations", &Rules::ValidationResult::violations)
        .def_readonly("is_valid", &Rules::ValidationResult::isValid);

    // ---------------- Rules: Preset ----------------
    py::class_<Rules::PresetRuleConfig>(m, "PresetRuleConfig")
        .def_readonly("rule_id", &Rules::PresetRuleConfig::ruleId)
        .def_readonly("is_enabled", &Rules::PresetRuleConfig::isEnabled);

    py::class_<Rules::Preset>(m, "Preset")
        .def(py::init<>())
        .def_readonly("id", &Rules::Preset::id)
        .def_readonly("name", &Rules::Preset::name)
        .def_readonly("description", &Rules::Preset::description)
        .def_readonly("rule_configs", &Rules::Preset::ruleConfigs);

    // ---------------- Rules: ConcreteRuleEngine ----------------
    py::class_<Rules::ConcreteRuleEngine>(m, "ConcreteRuleEngine")
        .def(py::init<>())
        .def("load_rule_set", &Rules::ConcreteRuleEngine::loadRuleSet, py::arg("preset"))
        .def("validate_score", &Rules::ConcreteRuleEngine::validateScore, py::arg("score"))
        .def("validate_region", &Rules::ConcreteRuleEngine::validateRegion,
             py::arg("score"), py::arg("dirty_region"));

    m.def("register_default_rules", &register_default_rules, py::arg("engine"),
          "Register the built-in rule plugins (ParallelFifths, ParallelOctaves).");

    // ---------------- IO ----------------
    py::class_<IO::PresetLoader>(m, "PresetLoader")
        .def_static("load_from_file", &IO::PresetLoader::loadFromFile, py::arg("path"));

    py::class_<IO::ScoreLoader>(m, "ScoreLoader")
        .def_static("load_from_file", &IO::ScoreLoader::loadFromFile, py::arg("path"))
        .def_static("save_to_file", &IO::ScoreLoader::saveToFile,
                    py::arg("score"), py::arg("path"));

    // ---------------- MIDI ----------------
    py::class_<Midi::MidiEvent>(m, "MidiEvent")
        .def(py::init<>())
        .def_readwrite("status", &Midi::MidiEvent::status)
        .def_readwrite("data1", &Midi::MidiEvent::data1)
        .def_readwrite("data2", &Midi::MidiEvent::data2)
        .def_readwrite("timestamp", &Midi::MidiEvent::timestamp);

    py::class_<Midi::RtMidiSystem>(m, "RtMidiSystem")
        .def(py::init<>())
        .def("list_input_devices", &Midi::RtMidiSystem::listInputDevices)
        .def("list_output_devices", &Midi::RtMidiSystem::listOutputDevices)
        .def("open_input_device", &Midi::RtMidiSystem::openInputDevice, py::arg("device_name"))
        .def("open_output_device", &Midi::RtMidiSystem::openOutputDevice, py::arg("device_name"))
        .def("close_input_device", &Midi::RtMidiSystem::closeInputDevice)
        .def("close_output_device", &Midi::RtMidiSystem::closeOutputDevice)
        .def("is_input_device_open", &Midi::RtMidiSystem::isInputDeviceOpen)
        .def("is_output_device_open", &Midi::RtMidiSystem::isOutputDeviceOpen)
        .def("get_open_input_device_name", &Midi::RtMidiSystem::getOpenInputDeviceName)
        .def("get_open_output_device_name", &Midi::RtMidiSystem::getOpenOutputDeviceName)
        .def("send_event", &Midi::RtMidiSystem::sendEvent, py::arg("event"))
        .def("send_note_on", &Midi::RtMidiSystem::sendNoteOn,
             py::arg("channel"), py::arg("note"), py::arg("velocity"))
        .def("send_note_off", &Midi::RtMidiSystem::sendNoteOff,
             py::arg("channel"), py::arg("note"), py::arg("velocity") = 0)
        .def("send_control_change", &Midi::RtMidiSystem::sendControlChange,
             py::arg("channel"), py::arg("cc_number"), py::arg("value"));
    // NOTE: set_input_callback(std::function) is intentionally not bound yet.

    // ---------------- Commands ----------------
    py::class_<Commands::CommandHistory>(m, "CommandHistory")
        .def(py::init<>())
        .def("undo", &Commands::CommandHistory::undo)
        .def("redo", &Commands::CommandHistory::redo)
        .def("can_undo", &Commands::CommandHistory::canUndo)
        .def("can_redo", &Commands::CommandHistory::canRedo)
        .def("clear", &Commands::CommandHistory::clear)
        .def("get_undo_text", &Commands::CommandHistory::getUndoText)
        .def("get_redo_text", &Commands::CommandHistory::getRedoText);

    m.def("add_note", &py_add_note, py::arg("history"), py::arg("score"), py::arg("note"));
    m.def("delete_notes", &py_delete_notes, py::arg("history"), py::arg("score"), py::arg("notes"));
    m.def("move_notes", &py_move_notes,
          py::arg("history"), py::arg("score"), py::arg("original_notes"),
          py::arg("dt"), py::arg("dp"));
    m.def("resize_note", &py_resize_note,
          py::arg("history"), py::arg("score"), py::arg("original_note"),
          py::arg("new_duration"));
}
