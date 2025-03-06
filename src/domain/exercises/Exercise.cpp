#include "domain/exercises/Exercise.h"
#include "domain/rules/ValidationPipeline.h"

namespace MusicTrainer {
namespace domain {
namespace exercises {

std::unique_ptr<Exercise> Exercise::create(
    const std::string& name,
    const std::string& description,
    ExerciseDifficulty difficulty,
    ExerciseCategory category)
{
    return std::unique_ptr<Exercise>(new Exercise(name, description, difficulty, category));
}

Exercise::Exercise(
    const std::string& name,
    const std::string& description,
    ExerciseDifficulty difficulty,
    ExerciseCategory category)
    : m_name(name)
    , m_description(description)
    , m_difficulty(difficulty)
    , m_category(category)
{
}

void Exercise::addRule(std::unique_ptr<music::rules::Rule> rule)
{
    if (rule) {
        m_rules.push_back(std::move(rule));
    }
}

bool Exercise::validate(const music::Score& score) const
{
    // Clear previous validation errors
    m_validationErrors.clear();
    
    // Create a validation pipeline
    auto pipeline = music::rules::ValidationPipeline::create();
    
    // Add all rules to the pipeline
    for (const auto& rule : m_rules) {
        if (rule) {
            // Clone the rule to avoid modifying the original
            pipeline->addRule(std::unique_ptr<music::rules::Rule>(rule->clone()));
        }
    }
    
    // Compile the rules
    pipeline->compileRules();
    
    // Validate the score
    bool result = pipeline->validate(score);
    
    // If validation failed, get the violations
    if (!result) {
        m_validationErrors = pipeline->getViolations();
    }
    
    return result;
}

std::string Exercise::difficultyToString(ExerciseDifficulty difficulty)
{
    switch (difficulty) {
        case ExerciseDifficulty::BEGINNER:
            return "Beginner";
        case ExerciseDifficulty::INTERMEDIATE:
            return "Intermediate";
        case ExerciseDifficulty::ADVANCED:
            return "Advanced";
        default:
            return "Unknown";
    }
}

std::string Exercise::categoryToString(ExerciseCategory category)
{
    switch (category) {
        case ExerciseCategory::FIRST_SPECIES:
            return "First Species";
        case ExerciseCategory::SECOND_SPECIES:
            return "Second Species";
        case ExerciseCategory::THIRD_SPECIES:
            return "Third Species";
        case ExerciseCategory::FOURTH_SPECIES:
            return "Fourth Species";
        case ExerciseCategory::FIFTH_SPECIES:
            return "Fifth Species";
        case ExerciseCategory::FREE_COUNTERPOINT:
            return "Free Counterpoint";
        default:
            return "Unknown";
    }
}

} // namespace exercises
} // namespace domain
} // namespace MusicTrainer