#include "adapters/InMemoryExerciseRepository.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "domain/rules/VoiceLeadingRule.h"
#include <algorithm>

namespace MusicTrainer {
namespace adapters {

std::unique_ptr<InMemoryExerciseRepository> InMemoryExerciseRepository::create()
{
    auto repository = std::unique_ptr<InMemoryExerciseRepository>(new InMemoryExerciseRepository());
    repository->initializeDefaultExercises();
    return repository;
}

std::vector<std::shared_ptr<domain::exercises::Exercise>> InMemoryExerciseRepository::getAllExercises()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::shared_ptr<domain::exercises::Exercise>> exercises;
    exercises.reserve(m_exercises.size());
    
    for (const auto& [_, exercise] : m_exercises) {
        exercises.push_back(exercise);
    }
    
    return exercises;
}

std::vector<std::shared_ptr<domain::exercises::Exercise>> InMemoryExerciseRepository::getExercisesByDifficulty(domain::exercises::ExerciseDifficulty difficulty)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::shared_ptr<domain::exercises::Exercise>> exercises;
    
    for (const auto& [_, exercise] : m_exercises) {
        if (exercise->getDifficulty() == difficulty) {
            exercises.push_back(exercise);
        }
    }
    
    return exercises;
}

std::vector<std::shared_ptr<domain::exercises::Exercise>> InMemoryExerciseRepository::getExercisesByCategory(domain::exercises::ExerciseCategory category)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::shared_ptr<domain::exercises::Exercise>> exercises;
    
    for (const auto& [_, exercise] : m_exercises) {
        if (exercise->getCategory() == category) {
            exercises.push_back(exercise);
        }
    }
    
    return exercises;
}

std::optional<std::shared_ptr<domain::exercises::Exercise>> InMemoryExerciseRepository::getExerciseByName(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_exercises.find(name);
    if (it != m_exercises.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

bool InMemoryExerciseRepository::addExercise(std::shared_ptr<domain::exercises::Exercise> exercise)
{
    if (!exercise) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    const std::string& name = exercise->getName();
    if (m_exercises.find(name) != m_exercises.end()) {
        return false; // Exercise with this name already exists
    }
    
    m_exercises[name] = std::move(exercise);
    return true;
}

bool InMemoryExerciseRepository::removeExercise(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_exercises.find(name);
    if (it == m_exercises.end()) {
        return false; // Exercise not found
    }
    
    m_exercises.erase(it);
    return true;
}

bool InMemoryExerciseRepository::save()
{
    // In-memory repository doesn't persist data
    // In a real implementation, this would save to a file or database
    return true;
}

bool InMemoryExerciseRepository::load()
{
    // In-memory repository doesn't persist data
    // In a real implementation, this would load from a file or database
    return true;
}

void InMemoryExerciseRepository::initializeDefaultExercises()
{
    using namespace domain::exercises;
    
    // First Species Exercises
    auto firstSpeciesBasic = Exercise::create(
        "First Species - C Major Scale",
        "Create a first species counterpoint against a C major cantus firmus. "
        "Each note in the counterpoint corresponds to one note in the cantus firmus. "
        "Use only consonant intervals (P1, P5, P8, M3, m3, M6, m6).",
        ExerciseDifficulty::BEGINNER,
        ExerciseCategory::FIRST_SPECIES
    );
    
    // Add rules to the exercise
    firstSpeciesBasic->addRule(music::rules::ParallelFifthsRule::create());
    firstSpeciesBasic->addRule(music::rules::ParallelOctavesRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(firstSpeciesBasic));
    
    // First Species - Simple Cantus Firmus
    auto firstSpeciesSimple = Exercise::create(
        "First Species - Simple Cantus Firmus",
        "Create a first species counterpoint against a simple cantus firmus. "
        "Focus on creating a melodic line that complements the cantus firmus "
        "while following the rules of counterpoint.",
        ExerciseDifficulty::BEGINNER,
        ExerciseCategory::FIRST_SPECIES
    );
    
    // Add rules to the exercise
    firstSpeciesSimple->addRule(music::rules::ParallelFifthsRule::create());
    firstSpeciesSimple->addRule(music::rules::ParallelOctavesRule::create());
    firstSpeciesSimple->addRule(music::rules::VoiceLeadingRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(firstSpeciesSimple));
    
    // Second Species - Basic Rhythms
    auto secondSpeciesBasic = Exercise::create(
        "Second Species - Basic Rhythms",
        "Create a second species counterpoint where each measure in the counterpoint "
        "contains two notes against one note in the cantus firmus. "
        "Strong beats must be consonant, while weak beats may be dissonant if approached and left by step.",
        ExerciseDifficulty::BEGINNER,
        ExerciseCategory::SECOND_SPECIES
    );
    
    // Add rules to the exercise
    secondSpeciesBasic->addRule(music::rules::ParallelFifthsRule::create());
    secondSpeciesBasic->addRule(music::rules::ParallelOctavesRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(secondSpeciesBasic));
    
    // Third Species - Complex Rhythms
    auto thirdSpeciesComplex = Exercise::create(
        "Third Species - Complex Rhythms",
        "Create a third species counterpoint with four notes in the counterpoint "
        "against one note in the cantus firmus. This allows for more complex "
        "rhythmic patterns and melodic lines.",
        ExerciseDifficulty::INTERMEDIATE,
        ExerciseCategory::THIRD_SPECIES
    );
    
    // Add rules to the exercise
    thirdSpeciesComplex->addRule(music::rules::ParallelFifthsRule::create());
    thirdSpeciesComplex->addRule(music::rules::ParallelOctavesRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(thirdSpeciesComplex));
    
    // Fourth Species - Suspensions
    auto fourthSpeciesSuspensions = Exercise::create(
        "Fourth Species - Suspensions",
        "Create a fourth species counterpoint characterized by suspensions. "
        "Notes in the counterpoint are offset from the cantus firmus, "
        "creating tension and resolution.",
        ExerciseDifficulty::INTERMEDIATE,
        ExerciseCategory::FOURTH_SPECIES
    );
    
    // Add rules to the exercise
    fourthSpeciesSuspensions->addRule(music::rules::ParallelFifthsRule::create());
    fourthSpeciesSuspensions->addRule(music::rules::ParallelOctavesRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(fourthSpeciesSuspensions));
    
    // Fifth Species - Florid Counterpoint
    auto fifthSpeciesFlorid = Exercise::create(
        "Fifth Species - Florid Counterpoint",
        "Create a fifth species (florid) counterpoint, which combines elements "
        "from all previous species. It allows for the greatest rhythmic and "
        "melodic freedom.",
        ExerciseDifficulty::ADVANCED,
        ExerciseCategory::FIFTH_SPECIES
    );
    
    // Add rules to the exercise
    fifthSpeciesFlorid->addRule(music::rules::ParallelFifthsRule::create());
    fifthSpeciesFlorid->addRule(music::rules::ParallelOctavesRule::create());
    fifthSpeciesFlorid->addRule(music::rules::VoiceLeadingRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(fifthSpeciesFlorid));
    
    // Free Counterpoint - Three Voices
    auto freeCounterpointThree = Exercise::create(
        "Free Counterpoint - Three Voices",
        "Create a free counterpoint with three voices. You'll need to manage "
        "the relationships between all voices simultaneously, maintaining "
        "independence of each voice while creating a cohesive musical texture.",
        ExerciseDifficulty::ADVANCED,
        ExerciseCategory::FREE_COUNTERPOINT
    );
    
    // Add rules to the exercise
    freeCounterpointThree->addRule(music::rules::ParallelFifthsRule::create());
    freeCounterpointThree->addRule(music::rules::ParallelOctavesRule::create());
    freeCounterpointThree->addRule(music::rules::VoiceLeadingRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(freeCounterpointThree));
    
    // Free Counterpoint - Four Voices
    auto freeCounterpointFour = Exercise::create(
        "Free Counterpoint - Four Voices",
        "Create a free counterpoint with four voices. This advanced exercise "
        "requires balancing harmonic and contrapuntal considerations across "
        "all four voices.",
        ExerciseDifficulty::ADVANCED,
        ExerciseCategory::FREE_COUNTERPOINT
    );
    
    // Add rules to the exercise
    freeCounterpointFour->addRule(music::rules::ParallelFifthsRule::create());
    freeCounterpointFour->addRule(music::rules::ParallelOctavesRule::create());
    freeCounterpointFour->addRule(music::rules::VoiceLeadingRule::create());
    
    // Add the exercise to the repository
    addExercise(std::move(freeCounterpointFour));
}

} // namespace adapters
} // namespace MusicTrainer