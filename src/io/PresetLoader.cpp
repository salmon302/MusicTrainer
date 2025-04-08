#include "PresetLoader.h"
#include "../rules/Preset.h"
#include "../rules/RuleTypes.h" // Access RuleParameterValue definition
#include <nlohmann/json.hpp> // Include the json library header
#include <fstream>      // For std::ifstream
#include <iostream>     // For std::cerr
#include <filesystem>   // For directory iteration
#include <stdexcept>    // For exceptions

// Use nlohmann::json namespace
using json = nlohmann::json;

namespace MusicTrainer {
namespace IO {

// Helper to safely get values from JSON, providing default or throwing
template<typename T>
T get_json_value(const json& j, const std::string& key, const T& default_value, bool required = false) {
    if (j.contains(key)) {
        try {
            return j.at(key).get<T>();
        } catch (const json::type_error& e) {
            throw std::runtime_error("Type error for key '" + key + "': " + e.what());
        }
    } else if (required) {
        throw std::runtime_error("Missing required key: '" + key + "'");
    }
    return default_value;
}

// Helper to parse RuleParameters map
Rules::RuleParameters parse_rule_parameters(const json& params_json) {
    Rules::RuleParameters params;
    if (!params_json.is_object()) {
        // Allow empty/null params, but throw if it's not an object when present
        if (!params_json.is_null()) {
             throw std::runtime_error("Rule 'params' field must be a JSON object.");
        }
        return params; // Return empty map
    }

    for (auto& [key, val] : params_json.items()) {
        if (val.is_boolean()) {
            params[key] = val.get<bool>();
        } else if (val.is_number_integer()) {
            params[key] = val.get<int>(); // Store integers as int
        } else if (val.is_number_float()) {
            params[key] = val.get<double>();
        } else if (val.is_string()) {
            params[key] = val.get<std::string>();
        } else if (val.is_array()) {
            // Example: Handle array of integers (e.g., prohibited intervals)
            // TODO: Extend this to handle other array types if needed
            try {
                 std::vector<int> int_array = val.get<std::vector<int>>();
                 // How to store array in variant? Maybe serialize to string or require specific keys?
                 // For now, let's skip arrays or throw, needs more design on RuleParameterValue
                 throw std::runtime_error("Array parameters not fully supported yet in PresetLoader for key: " + key);
            } catch (const json::type_error& e) {
                 throw std::runtime_error("Type error parsing array parameter for key '" + key + "': " + e.what());
            }
        }
        // Add more type handling as needed for RuleParameterValue variant
    }
    return params;
}


// --- Public Static Methods ---

Rules::Preset PresetLoader::loadFromFile(const std::filesystem::path& filePath) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open preset file: " + filePath.string());
    }

    try {
        json j;
        ifs >> j; // Parse the file stream into json object
        return parsePresetJson(j.dump(), filePath); // Pass raw json string or parsed object? Pass parsed.
    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parse error in file " + filePath.string() + ": " + e.what());
    } catch (const std::exception& e) {
         // Catch other potential errors during parsing (e.g., type errors from helpers)
         throw std::runtime_error("Error processing preset file " + filePath.string() + ": " + e.what());
    }
}

std::vector<Rules::Preset> PresetLoader::loadFromDirectory(const std::filesystem::path& directoryPath) {
    std::vector<Rules::Preset> presets;
    if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
        std::cerr << "Warning: Preset directory not found or not a directory: " << directoryPath.string() << std::endl;
        return presets;
    }

    // TODO: Define a specific file extension (e.g., ".preset.json")
    std::string extension = ".preset.json";

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            try {
                presets.push_back(loadFromFile(entry.path()));
                 std::cout << "Loaded preset: " << entry.path().filename().string() << std::endl; // Debug
            } catch (const std::exception& e) {
                std::cerr << "Error loading preset file " << entry.path().string() << ": " << e.what() << std::endl;
                // Continue loading other files
            }
        }
    }
    return presets;
}


// --- Private Helper ---

Rules::Preset PresetLoader::parsePresetJson(const std::string& jsonData, const std::filesystem::path& sourcePath) {
     // Re-parse the string (or pass the json object directly)
     json j = json::parse(jsonData);
     Rules::Preset preset;

     // Extract top-level fields
     preset.id = get_json_value<std::string>(j, "id", "", true); // ID is required
     preset.name = get_json_value<std::string>(j, "name", preset.id); // Use ID if name missing
     preset.description = get_json_value<std::string>(j, "description", "");

     // Extract rules array
     if (!j.contains("rules") || !j.at("rules").is_array()) {
         throw std::runtime_error("Preset file must contain a 'rules' array. Source: " + sourcePath.string());
     }

     for (const auto& rule_json : j.at("rules")) {
         if (!rule_json.is_object()) {
             throw std::runtime_error("Items in 'rules' array must be objects. Source: " + sourcePath.string());
         }
         Rules::PresetRuleConfig config;
         config.ruleId = get_json_value<std::string>(rule_json, "ruleId", "", true); // Rule ID required
         config.isEnabled = get_json_value<bool>(rule_json, "enabled", true); // Default to enabled
         if (rule_json.contains("params")) {
             config.parameters = parse_rule_parameters(rule_json.at("params"));
         }
         preset.ruleConfigs.push_back(config);
     }

     return preset;
}


} // namespace IO
} // namespace MusicTrainer