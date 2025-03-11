#include "presentation/ExercisePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace MusicTrainer {
namespace presentation {

ExercisePanel::ExercisePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
}

ExercisePanel::~ExercisePanel() = default;

void ExercisePanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    
    // Exercise description
    auto* descriptionLabel = new QLabel(tr("Exercise Description"), this);
    m_descriptionTextEdit = new QTextEdit(this);
    m_descriptionTextEdit->setReadOnly(true);
    
    // Violations list
    auto* violationsLabel = new QLabel(tr("Rule Violations"), this);
    m_violationsListWidget = new QListWidget(this);
    
    // Progress bar
    auto* progressLabel = new QLabel(tr("Progress"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    m_checkSolutionButton = new QPushButton(tr("Check Solution"), this);
    m_hintButton = new QPushButton(tr("Hint"), this);
    buttonLayout->addWidget(m_checkSolutionButton);
    buttonLayout->addWidget(m_hintButton);
    
    // Add widgets to layout
    layout->addWidget(descriptionLabel);
    layout->addWidget(m_descriptionTextEdit);
    layout->addWidget(violationsLabel);
    layout->addWidget(m_violationsListWidget);
    layout->addWidget(progressLabel);
    layout->addWidget(m_progressBar);
    layout->addLayout(buttonLayout);
    
    setLayout(layout);
}

void ExercisePanel::setupConnections()
{
    connect(m_checkSolutionButton, &QPushButton::clicked, this, &ExercisePanel::onCheckSolutionClicked);
    connect(m_hintButton, &QPushButton::clicked, this, &ExercisePanel::onHintClicked);
    connect(m_violationsListWidget, &QListWidget::currentRowChanged, this, &ExercisePanel::onRuleViolationSelected);
}

void ExercisePanel::setExerciseDescription(const QString& description)
{
    m_descriptionTextEdit->setText(description);
}

void ExercisePanel::setProgress(int percentage)
{
    m_progressBar->setValue(percentage);
}

void ExercisePanel::addRuleViolation(const QString& message, const music::rules::Rule* rule, int severity)
{
    RuleViolation violation{message, rule, severity};
    m_violations.push_back(violation);
    
    // Create list item with icon based on severity
    QListWidgetItem* item = new QListWidgetItem(message, m_violationsListWidget);
    switch (severity) {
        case 0: // Info
            item->setIcon(QIcon::fromTheme("dialog-information"));
            break;
        case 1: // Warning
            item->setIcon(QIcon::fromTheme("dialog-warning"));
            break;
        case 2: // Error
            item->setIcon(QIcon::fromTheme("dialog-error"));
            break;
    }
    m_violationsListWidget->addItem(item);
}

void ExercisePanel::clearRuleViolations()
{
    m_violationsListWidget->clear();
    m_violations.clear();
}

void ExercisePanel::onCheckSolutionClicked()
{
    Q_EMIT checkSolutionRequested();
}

void ExercisePanel::onHintClicked()
{
    // For now, always request a medium level hint (level 1)
    Q_EMIT hintRequested(1);
}

void ExercisePanel::onRuleViolationSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(m_violations.size())) {
        qDebug() << "Rule violation selected:" << m_violations[index].message;
        // You can add additional code here to highlight the rule violation in the score
    }
}

} // namespace presentation
} // namespace MusicTrainer