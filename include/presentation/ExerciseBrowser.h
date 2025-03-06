#pragma once

#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QProgressBar>
#include "domain/exercises/Exercise.h"
#include "domain/exercises/ExerciseRepository.h"
#include <memory>

namespace MusicTrainer {
namespace presentation {

/**
 * @brief Dialog for browsing and selecting exercises
 * 
 * This dialog allows users to browse available exercises by difficulty level
 * and category, view their progress, and create custom exercises.
 */
class ExerciseBrowser : public QDialog {
    Q_OBJECT

public:
    explicit ExerciseBrowser(QWidget* parent = nullptr);
    ~ExerciseBrowser() override;

    /**
     * @brief Get the selected exercise
     * @return Shared pointer to the selected exercise or nullptr if none selected
     */
    std::shared_ptr<domain::exercises::Exercise> getSelectedExercise() const;
    
    /**
     * @brief Set the exercise repository
     * @param repository The repository to use
     */
    void setExerciseRepository(std::shared_ptr<domain::exercises::ExerciseRepository> repository);

signals:
    /**
     * @brief Signal emitted when an exercise is selected
     * @param exercise The selected exercise
     */
    void exerciseSelected(std::shared_ptr<domain::exercises::Exercise> exercise);

private slots:
    void onDifficultyChanged(int index);
    void onCategoryChanged(int index);
    void onExerciseSelected(QListWidgetItem* item);
    void onCreateCustomExerciseClicked();
    void onSelectExerciseClicked();

private:
    void setupUi();
    void setupConnections();
    void populateExercises();
    void updateExerciseDetails();

    // UI Components
    QComboBox* m_difficultyComboBox{nullptr};
    QComboBox* m_categoryComboBox{nullptr};
    QListWidget* m_exerciseListWidget{nullptr};
    QTextEdit* m_exerciseDescriptionTextEdit{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QPushButton* m_createCustomButton{nullptr};
    QPushButton* m_selectButton{nullptr};
    QPushButton* m_cancelButton{nullptr};

    // Data
    std::shared_ptr<domain::exercises::Exercise> m_selectedExercise{nullptr};
    std::shared_ptr<domain::exercises::ExerciseRepository> m_repository{nullptr};
};

} // namespace presentation
} // namespace MusicTrainer