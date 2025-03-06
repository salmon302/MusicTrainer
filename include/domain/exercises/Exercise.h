#ifndef MUSICTRAINERV3_EXERCISE_H
#define MUSICTRAINERV3_EXERCISE_H

#include <string>
#include <vector>
#include <memory>
#include "domain/music/Score.h"
#include "domain/rules/Rule.h"

namespace MusicTrainer {
namespace domain {
namespace exercises {

enum class ExerciseDifficulty {
    BEGINNER,
    INTERMEDIATE,
    ADVANCED
};

enum class ExerciseCategory {
    FIRST_SPECIES,
    SECOND_SPECIES,
    THIRD_SPECIES,
    FOURTH_SPECIES,
    FIFTH_SPECIES,
    FREE_COUNTERPOINT
};

/**
 * @brief Represents a music theory exercise
 * 
 * An exercise contains a template score, a set of rules to validate against,
 * and metadata such as name, description, difficulty, and category.
 */
class Exercise {
public:
    /**
     * @brief Create a new exercise
     * @param name The name of the exercise
     * @param description A detailed description of the exercise
     * @param difficulty The difficulty level
     * @param category The exercise category
     * @return A unique pointer to the new exercise
     */
    static std::unique_ptr<Exercise> create(
        const std::string& name,
        const std::string& description,
        ExerciseDifficulty difficulty,
        ExerciseCategory category);
    
    /**
     * @brief Get the name of the exercise
     * @return The exercise name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * @brief Get the description of the exercise
     * @return The exercise description
     */
    const std::string& getDescription() const { return m_description; }
    
    /**
     * @brief Get the difficulty level
     * @return The difficulty level
     */
    ExerciseDifficulty getDifficulty() const { return m_difficulty; }
    
    /**
     * @brief Get the exercise category
     * @return The exercise category
     */
    ExerciseCategory getCategory() const { return m_category; }
    
    /**
     * @brief Get the template score
     * @return The template score
     */
    const music::Score* getTemplateScore() const { return m_templateScore.get(); }
    
    /**
     * @brief Set the template score
     * @param score The template score
     */
    void setTemplateScore(std::unique_ptr<music::Score> score) { m_templateScore = std::move(score); }
    
    /**
     * @brief Add a rule to the exercise
     * @param rule The rule to add
     */
    void addRule(std::unique_ptr<music::rules::Rule> rule);
    
    /**
     * @brief Get all rules for this exercise
     * @return The rules
     */
    const std::vector<std::unique_ptr<music::rules::Rule>>& getRules() const { return m_rules; }
    
    /**
     * @brief Validate a score against the exercise rules
     * @param score The score to validate
     * @return True if the score passes all rules, false otherwise
     */
    bool validate(const music::Score& score) const;
    
    /**
     * @brief Get the validation errors from the last validation
     * @return The validation errors
     */
    std::vector<std::string> getValidationErrors() const { return m_validationErrors; }
    
    /**
     * @brief Convert difficulty to string
     * @param difficulty The difficulty level
     * @return String representation
     */
    static std::string difficultyToString(ExerciseDifficulty difficulty);
    
    /**
     * @brief Convert category to string
     * @param category The exercise category
     * @return String representation
     */
    static std::string categoryToString(ExerciseCategory category);

private:
    Exercise(
        const std::string& name,
        const std::string& description,
        ExerciseDifficulty difficulty,
        ExerciseCategory category);
    
    std::string m_name;
    std::string m_description;
    ExerciseDifficulty m_difficulty;
    ExerciseCategory m_category;
    std::unique_ptr<music::Score> m_templateScore;
    std::vector<std::unique_ptr<music::rules::Rule>> m_rules;
    mutable std::vector<std::string> m_validationErrors;
};

} // namespace exercises
} // namespace domain
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_EXERCISE_H