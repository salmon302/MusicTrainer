#include "ScoreLoader.h"
#include "../domain/Score.h"
#include <nlohmann/json.hpp> // Include the json library header
#include <fstream>      // For std::ifstream, std::ofstream
#include <iostream>     // For std::cerr
#include <stdexcept>    // For exceptions
#include <iomanip>      // For std::setw (pretty printing JSON)

// Use nlohmann::json namespace
using json = nlohmann::json;

namespace MusicTrainer {
namespace IO {

// --- Public Static Methods ---

Domain::Score ScoreLoader::loadFromFile(const std::filesystem::path& filePath) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open score file for reading: " + filePath.string());
    }

    try {
        json j;
        ifs >> j; // Parse the file stream into json object

        // Use the from_json function defined for Score in Score.h
        Domain::Score loadedScore = j.get<Domain::Score>();
        std::cout << "Successfully loaded score from: " << filePath.string() << std::endl; // Debug
        return loadedScore;

    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parse error in score file " + filePath.string() + ": " + e.what());
    } catch (const json::type_error& e) {
         throw std::runtime_error("JSON type error loading score file " + filePath.string() + ": " + e.what());
    } catch (const json::other_error& e) { // Catch errors from domain object validation (e.g., invalid MIDI number)
         throw std::runtime_error("Validation error loading score file " + filePath.string() + ": " + e.what());
    } catch (const std::exception& e) {
         // Catch other potential errors during parsing or Score construction
         throw std::runtime_error("Error loading score file " + filePath.string() + ": " + e.what());
    }
}

void ScoreLoader::saveToFile(const Domain::Score& score, const std::filesystem::path& filePath) {
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        throw std::runtime_error("Could not open score file for writing: " + filePath.string());
    }

    try {
        // Use the to_json function defined for Score in Score.h
        json j = score;

        // Write to file with pretty printing (indentation)
        ofs << std::setw(4) << j << std::endl;
        std::cout << "Successfully saved score to: " << filePath.string() << std::endl; // Debug

    } catch (const json::type_error& e) {
         throw std::runtime_error("JSON type error saving score file " + filePath.string() + ": " + e.what());
    } catch (const std::exception& e) {
         // Catch other potential errors during serialization
         throw std::runtime_error("Error saving score file " + filePath.string() + ": " + e.what());
    }
}


} // namespace IO
} // namespace MusicTrainer