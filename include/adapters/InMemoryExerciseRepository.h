#ifndef MUSICTRAINERV3_INMEMORYEXERCISEREPOSITORY_H
#define MUSICTRAINERV3_INMEMORYEXERCISEREPOSITORY_H

#include "domain/exercises/ExerciseRepository.h"
#include <map>
#include <mutex>

namespace MusicTrainer {
namespace adapters {

/**
 * @brief In-memory implementation of ExerciseRepository
 */
class InMemoryExerciseRepository : public domain::exercises::ExerciseRepository {
public:
    /**
     * @brief Create a new in-memory exercise repository
     * @return A unique pointer to the repository
     */
    static std::unique_ptr<InMemoryExerciseRepository> create();
    
    // ExerciseRepository interface implementation
    std::vector<std::shared_ptr<domain::exercises::Exercise>> getAllExercises() override;
    std::vector<std::shared_ptr<domain::exercises::Exercise>> getExercisesByDifficulty(domain::exercises::ExerciseDifficulty difficulty) override;
    std::vector<std::shared_ptr<domain::exercises::Exercise>> getExercisesByCategory(domain::exercises::ExerciseCategory category) override;
    std::optional<std::shared_ptr<domain::exercises::Exercise>> getExerciseByName(const std::string& name) override;
    bool addExercise(std::shared_ptr<domain::exercises::Exercise> exercise) override;
    bool removeExercise(const std::string& name) override;
    bool save() override;
    bool load() override;
    
    /**
     * @brief Initialize the repository with default exercises
     */
    void initializeDefaultExercises();

private:
    InMemoryExerciseRepository() = default;
    
    std::map<std::string, std::shared_ptr<domain::exercises::Exercise>> m_exercises;
    mutable std::mutex m_mutex;
};

} // namespace adapters
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_INMEMORYEXERCISEREPOSITORY_H