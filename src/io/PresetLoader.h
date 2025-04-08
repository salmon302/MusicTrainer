#ifndef MUSIC_TRAINER_IO_PRESETLOADER_H
#define MUSIC_TRAINER_IO_PRESETLOADER_H

#include "../rules/Preset.h" // Include the Preset structure definition
#include <string>
#include <vector>
#include <filesystem> // Requires C++17
#include <optional>

namespace MusicTrainer {
namespace IO {

/**
 * @brief Handles loading of Rule Presets from files (e.g., JSON).
 */
class PresetLoader {
public:
    /**
     * @brief Loads a single Preset from the specified file path.
     * @param filePath Path to the preset file (expects JSON format).
     * @return The loaded Preset object.
     * @throws std::runtime_error if the file cannot be opened or parsed,
     *         or if the JSON structure is invalid.
     */
    static Rules::Preset loadFromFile(const std::filesystem::path& filePath);

    /**
     * @brief Loads all presets found in a specified directory.
     * @param directoryPath Path to the directory containing preset files (e.g., *.preset.json).
     * @return A vector of loaded Preset objects. Errors loading individual files are logged.
     */
    static std::vector<Rules::Preset> loadFromDirectory(const std::filesystem::path& directoryPath);

    // TODO: Add saveToFile method later for saving custom presets (FP-04)

private:
    // Helper to parse JSON data into a Preset object (implementation in .cpp)
    static Rules::Preset parsePresetJson(const std::string& jsonData, const std::filesystem::path& sourcePath);
};

} // namespace IO
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_IO_PRESETLOADER_H