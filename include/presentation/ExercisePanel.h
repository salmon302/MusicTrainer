#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <memory>

namespace MusicTrainer {

namespace music::rules {
class Rule;
}

namespace presentation {

/**
 * @brief Panel for displaying exercise information and validation results
 * 
 * This widget shows the current exercise details, rule violations, and provides
 * controls for checking solutions and getting hints.
 */
class ExercisePanel : public QWidget {
    Q_OBJECT

public:
    explicit ExercisePanel(QWidget* parent = nullptr);
    ~ExercisePanel() override;

    /**
     * @brief Set the exercise description
     * @param description Text description of the exercise
     */
    void setExerciseDescription(const QString& description);

    /**
     * @brief Set the progress percentage
     * @param percentage Completion percentage (0-100)
     */
    void setProgress(int percentage);

    /**
     * @brief Add a rule violation to the list
     * @param message Description of the violation
     * @param rule The rule that was violated
     * @param severity Severity level (0-2: info, warning, error)
     */
    void addRuleViolation(const QString& message, const music::rules::Rule* rule, int severity);

    /**
     * @brief Clear all rule violations
     */
    void clearRuleViolations();

Q_SIGNALS:
    /**
     * @brief Emitted when the user requests solution checking
     */
    void checkSolutionRequested();

    /**
     * @brief Emitted when the user requests a hint
     * @param level Hint level (0-2: minimal, medium, detailed)
     */
    void hintRequested(int level);

    /**
     * @brief Emitted when the exercise type changes
     * @param exerciseType The new exercise type
     */
    void exerciseChanged(const QString& exerciseType);

private Q_SLOTS:
    void onCheckSolutionClicked();
    void onHintClicked();
    void onRuleViolationSelected(int index);

private:
    void setupUi();
    void setupConnections();

    // UI components
    QTextEdit* m_descriptionTextEdit{nullptr};
    QListWidget* m_violationsListWidget{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QPushButton* m_checkSolutionButton{nullptr};
    QPushButton* m_hintButton{nullptr};

    // Track rule violations for selection
    struct RuleViolation {
        QString message;
        const music::rules::Rule* rule;
        int severity;
    };
    std::vector<RuleViolation> m_violations;
};

} // namespace presentation
} // namespace MusicTrainer