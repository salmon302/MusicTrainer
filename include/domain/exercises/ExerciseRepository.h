#ifndef MUSICTRAINERV3_EXERCISEREPOSITORY_H
#define MUSICTRAINERV3_EXERCISEREPOSITORY_H

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "Exercise.h"

namespace MusicTrainer {
namespace domain {
namespace exercises {

/**
 * @brief Repository interface for managing exercises
 */
class ExerciseRepository {
public:
    virtual ~ExerciseRepository() = default;
    
    /**
     * @brief Get all exercises
     * @return Vector of exercises
     */
    virtual std::vector<std::shared_ptr<Exercise>> getAllExercises() = 0;
    
    /**
     * @brief Get exercises by difficulty
     * @param difficulty The difficulty level
     * @return Vector of exercises with the specified difficulty
     */
    virtual std::vector<std::shared_ptr<Exercise>> getExercisesByDifficulty(ExerciseDifficulty difficulty) = 0;
    
    /**
     * @brief Get exercises by category
     * @param category The exercise category
     * @return Vector of exercises in the specified category
     */
    virtual std::vector<std::shared_ptr<Exercise>> getExercisesByCategory(ExerciseCategory category) = 0;
    
    /**
     * @brief Get an exercise by name
     * @param name The exercise name
     * @return The exercise if found, nullopt otherwise
     */
    virtual std::optional<std::shared_ptr<Exercise>> getExerciseByName(const std::string& name) = 0;
    
    /**
     * @brief Add an exercise to the repository
     * @param exercise The exercise to add
     * @return True if the exercise was added successfully, false otherwise
     */
    virtual bool addExercise(std::shared_ptr<Exercise> exercise) = 0;
    
    /**
     * @brief Remove an exercise from the repository
     * @param name The name of the exercise to remove
     * @return True if the exercise was removed successfully, false otherwise
     */
    virtual bool removeExercise(const std::string& name) = 0;
    
    /**
     * @brief Save the repository to persistent storage
     * @return True if the repository was saved successfully, false otherwise
     */
    virtual bool save() = 0;
    
    /**
     * @brief Load the repository from persistent storage
     * @return True if the repository was loaded successfully, false otherwise
     */
    virtual bool load() = 0;
};

} // namespace exercises
} // namespace domain
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_EXERCISEREPOSITORY_H