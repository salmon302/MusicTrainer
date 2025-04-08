#ifndef MUSIC_TRAINER_IO_SCORELOADER_H
#define MUSIC_TRAINER_IO_SCORELOADER_H

#include "../domain/Score.h" // Include the Score definition
#include <string>
#include <filesystem> // Requires C++17

namespace MusicTrainer {
namespace IO {

/**
 * @brief Handles saving and loading of Score objects to/from files.
 *
 * Uses JSON format for serialization.
 */
class ScoreLoader {
public:
    /**
     * @brief Loads a Score from the specified file path.
     * @param filePath Path to the score file (expects JSON format, e.g., .mtx).
     * @return The loaded Score object.
     * @throws std::runtime_error if the file cannot be opened or parsed,
     *         or if the JSON structure is invalid for a Score.
     */
    static Domain::Score loadFromFile(const std::filesystem::path& filePath);

    /**
     * @brief Saves a Score object to the specified file path.
     * @param score The Score object to save.
     * @param filePath Path where the score file should be saved (e.g., .mtx).
     * @throws std::runtime_error if the file cannot be opened for writing or
     *         if there's an error during JSON serialization.
     */
    static void saveToFile(const Domain::Score& score, const std::filesystem::path& filePath);

}; // class ScoreLoader

} // namespace IO
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_IO_SCORELOADER_H