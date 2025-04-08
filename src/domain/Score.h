#ifndef MUSIC_TRAINER_DOMAIN_SCORE_H
#define MUSIC_TRAINER_DOMAIN_SCORE_H

#include "Note.h"
#include "TimeSignature.h"
#include "KeySignature.h"
#include "Position.h"
#include <vector>
#include <map>
#include <memory> // For std::shared_ptr potentially if Notes are managed complexly
#include <string>
#include <optional>
#include <algorithm> // For std::find, std::lower_bound
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

// Forward declaration if needed, though includes should suffice
// class Note;
// class TimeSignature;
// class KeySignature;
// class Position;

/**
 * @brief Represents a complete musical score.
 *
 * Contains voices with notes, and time-ordered lists of time signature
 * and key signature changes. Designed with copy-on-write semantics in mind
 * for efficient modification and undo/redo support.
 */
class Score {
public:
    // Represents a single voice within the score
    struct Voice {
        int id;
        std::string name; // Optional name (e.g., "Soprano", "CF")
        std::vector<Note> notes; // Notes sorted by start time

        explicit Voice(int v_id, std::string v_name = "") : id(v_id), name(std::move(v_name)) {}

        // TODO: Add methods for efficient note lookup within the voice if needed
    };

    // --- JSON Serialization for Voice ---
    inline void to_json(json& j, const Voice& v) {
        j = json{
            {"id", v.id},
            {"name", v.name},
            {"notes", v.notes} // Uses Note's to_json
        };
    }

    inline void from_json(const json& j, Voice& v) {
         if (!j.is_object() || !j.contains("id") || !j.contains("notes")) {
             throw json::type_error::create(302, "JSON object for Voice must contain id and notes", j);
         }
         try {
            v.id = j.at("id").get<int>();
            v.name = j.value("name", ""); // Optional name
            v.notes = j.at("notes").get<std::vector<Note>>(); // Uses Note's from_json

            // Ensure notes are sorted after loading (in case JSON wasn't sorted)
            std::sort(v.notes.begin(), v.notes.end());

         } catch (const json::type_error& e) {
            throw json::type_error::create(302, std::string("Type error parsing Voice JSON: ") + e.what(), j);
         } catch (const std::exception& e) {
             throw json::other_error::create(501, std::string("Validation error creating Voice from JSON: ") + e.what(), j);
         }
    }


private:
    // Using map to store time/key signatures allows easy lookup of the
    // signature active at any given Position. Key is start time in beats.
    std::map<Position, TimeSignature> timeSignatures;
    std::map<Position, KeySignature> keySignatures;
    std::vector<Voice> voices;

    // TODO: Consider copy-on-write implementation details if performance demands it.
    // For now, standard containers are used.

public:
    // Default constructor (creates an empty score, typically with default C Major, 4/4 at start)
    Score() {
        // Initialize with default signatures at the beginning
        timeSignatures[Position(0.0)] = TimeSignature::CommonTime();
        keySignatures[Position(0.0)] = KeySignature::CMajor();
    }

    // --- Voice Management ---
    int addVoice(const std::string& name = "") {
        int newId = static_cast<int>(voices.size());
        voices.emplace_back(newId, name);
        return newId;
    }

    size_t getVoiceCount() const {
        return voices.size();
    }

    Voice* getVoice(int voiceId) {
        if (voiceId >= 0 && static_cast<size_t>(voiceId) < voices.size()) {
            return &voices[voiceId];
        }
        return nullptr; // Or throw exception
    }

    const Voice* getVoice(int voiceId) const {
         if (voiceId >= 0 && static_cast<size_t>(voiceId) < voices.size()) {
            return &voices[voiceId];
        }
        return nullptr; // Or throw exception
    }

    const std::vector<Voice>& getAllVoices() const {
        return voices;
    }

    // --- Note Management ---
    // Adds a note to the specified voice. Keeps the voice's note list sorted.
    void addNote(const Note& note) {
        Voice* voice = getVoice(note.getVoiceId());
        if (!voice) {
            // Handle error: voice doesn't exist
            throw std::runtime_error("Attempted to add note to non-existent voice ID: " + std::to_string(note.getVoiceId()));
        }
        // Insert note while maintaining sort order (by start time, then pitch)
        auto it = std::lower_bound(voice->notes.begin(), voice->notes.end(), note);
        voice->notes.insert(it, note);
    }

    // Removes the first occurrence of a note matching the value provided.
    // Returns true if a note was found and removed, false otherwise.
    // Note: This relies on Note::operator== and might be slow for large scores.
    // A Command pattern using note IDs or iterators would be more robust.
    bool removeNote(const Note& noteToRemove) {
        Voice* voice = getVoice(noteToRemove.getVoiceId());
        if (!voice) {
            return false; // Voice doesn't exist
        }

        auto it = std::find(voice->notes.begin(), voice->notes.end(), noteToRemove);
        if (it != voice->notes.end()) {
            voice->notes.erase(it);
            return true;
        }
        return false; // Note not found in the specified voice
    }

    // TODO: Add efficient query methods (getNotesInRange, getNotesAtTime etc.)

    // --- Signature Management ---
    void addTimeSignatureChange(const Position& position, const TimeSignature& ts) {
        timeSignatures[position] = ts; // Map automatically handles insertion/replacement
    }

    void addKeySignatureChange(const Position& position, const KeySignature& ks) {
        keySignatures[position] = ks; // Map automatically handles insertion/replacement
    }

    // Gets the time signature active at a given position
    TimeSignature getTimeSignatureAt(const Position& position) const {
        auto it = timeSignatures.upper_bound(position);
        if (it == timeSignatures.begin()) {
            // Position is before the first defined signature (shouldn't happen with default)
            // Or map is empty (also shouldn't happen)
            return TimeSignature::CommonTime(); // Return default as fallback
        }
        // Return the signature defined at or before the given position
        return (--it)->second;
    }

    // Gets the key signature active at a given position
    KeySignature getKeySignatureAt(const Position& position) const {
         auto it = keySignatures.upper_bound(position);
        if (it == keySignatures.begin()) {
            return KeySignature::CMajor(); // Return default as fallback
        }
        // Return the signature defined at or before the given position
        return (--it)->second;
    }

    // --- Utility ---
    bool isEmpty() const {
        for(const auto& voice : voices) {
            if (!voice.notes.empty()) return false;
        }
        return true;
    }

    // TODO: Add methods for calculating total duration, etc.

}; // class Score


// --- JSON Serialization for Score ---

// Helper to serialize map<Position, T>
template<typename T>
void serialize_position_map(json& j, const std::map<Position, T>& map) {
    j = json::object(); // Create a JSON object
    for (const auto& [pos, value] : map) {
        // Use beat number as string key, as JSON object keys must be strings
        // Consider precision issues or alternative key representations if needed
        std::string key = std::to_string(pos.beats);
        j[key] = value; // Uses T's to_json
    }
}

// Helper to deserialize map<Position, T>
template<typename T>
void deserialize_position_map(const json& j, std::map<Position, T>& map) {
    if (!j.is_object()) {
        throw json::type_error::create(302, "Expected JSON object for Position map", j);
    }
    map.clear();
    for (auto& [key, value] : j.items()) {
        try {
            double beat = std::stod(key); // Convert string key back to double
            Position pos(beat);
            map[pos] = value.get<T>(); // Uses T's from_json
        } catch (const std::invalid_argument& e) {
            throw json::type_error::create(302, "Invalid beat number key in Position map: " + key, j);
        } catch (const std::out_of_range& e) {
             throw json::type_error::create(302, "Beat number key out of range in Position map: " + key, j);
        }
        // Catch errors from T's from_json? The main from_json for Score will catch them.
    }
}


inline void to_json(json& j, const Score& s) {
    j = json::object(); // Start with an empty object
    serialize_position_map(j["timeSignatures"], s.timeSignatures);
    serialize_position_map(j["keySignatures"], s.keySignatures);
    j["voices"] = s.voices; // Uses Voice's to_json
}

inline void from_json(const json& j, Score& s) {
     if (!j.is_object()) {
         throw json::type_error::create(302, "JSON value for Score must be an object", j);
     }
     try {
        // Clear existing data and load defaults first?
        s = Score(); // Reset to default state (Cmaj, 4/4) before loading

        if (j.contains("timeSignatures")) {
            deserialize_position_map(j.at("timeSignatures"), s.timeSignatures);
        }
        if (j.contains("keySignatures")) {
             deserialize_position_map(j.at("keySignatures"), s.keySignatures);
        }
        if (j.contains("voices")) {
            s.voices = j.at("voices").get<std::vector<Score::Voice>>(); // Uses Voice's from_json
        }

        // Ensure default signatures exist if map is empty after load (shouldn't happen if Score constructor ran)
        if (s.timeSignatures.empty()) {
             s.timeSignatures[Position(0.0)] = TimeSignature::CommonTime();
        }
         if (s.keySignatures.empty()) {
             s.keySignatures[Position(0.0)] = KeySignature::CMajor();
        }

     } catch (const json::type_error& e) {
        throw json::type_error::create(302, std::string("Type error parsing Score JSON: ") + e.what(), j);
     } catch (const std::exception& e) { // Catch validation errors etc.
         throw json::other_error::create(501, std::string("Error creating Score from JSON: ") + e.what(), j);
     }
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_SCORE_H