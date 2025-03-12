#include "presentation/ExercisePanel.h"
#include "domain/rules/Rule.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QDebug>

namespace MusicTrainer::presentation {

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
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Exercise description
    auto* descriptionLabel = new QLabel(tr("Exercise Description"), this);
    descriptionLabel->setStyleSheet("font-weight: bold;");
    m_descriptionTextEdit = new QTextEdit(this);
    m_descriptionTextEdit->setReadOnly(true);
    m_descriptionTextEdit->setMinimumHeight(100);
    
    // Violations list
    auto* violationsLabel = new QLabel(tr("Rule Violations"), this);
    violationsLabel->setStyleSheet("font-weight: bold;");
    m_violationsListWidget = new QListWidget(this);
    m_violationsListWidget->setMinimumHeight(150);
    
    // Progress section
    auto* progressLabel = new QLabel(tr("Progress"), this);
    progressLabel->setStyleSheet("font-weight: bold;");
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat(tr("%p% Complete"));
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    m_checkSolutionButton = new QPushButton(tr("Check Solution"), this);
    m_checkSolutionButton->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    
    m_hintButton = new QPushButton(tr("Hint"), this);
    m_hintButton->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    
    buttonLayout->addWidget(m_checkSolutionButton);
    buttonLayout->addWidget(m_hintButton);
    buttonLayout->addStretch();
    
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
    connect(m_checkSolutionButton, &QPushButton::clicked, 
            this, &ExercisePanel::onCheckSolutionClicked);
    connect(m_hintButton, &QPushButton::clicked,
            this, &ExercisePanel::onHintClicked);
    connect(m_violationsListWidget, &QListWidget::currentRowChanged,
            this, &ExercisePanel::onRuleViolationSelected);
}

void ExercisePanel::setExerciseDescription(const QString& description)
{
    m_descriptionTextEdit->setHtml(description);
    Q_EMIT exerciseChanged(description);
}

void ExercisePanel::setProgress(int percentage)
{
    percentage = qBound(0, percentage, 100);
    m_progressBar->setValue(percentage);
    
    // Update progress bar color based on value
    QString styleSheet;
    if (percentage < 33) {
        styleSheet = "QProgressBar::chunk { background-color: #f44336; }"; // Red
    } else if (percentage < 66) {
        styleSheet = "QProgressBar::chunk { background-color: #ff9800; }"; // Orange
    } else {
        styleSheet = "QProgressBar::chunk { background-color: #4caf50; }"; // Green
    }
    m_progressBar->setStyleSheet(styleSheet);
}

void ExercisePanel::addRuleViolation(const QString& message, const music::rules::Rule* rule, int severity)
{
    // Create list item with icon based on severity
    auto* item = new QListWidgetItem(m_violationsListWidget);
    QString text = message;
    
    switch (severity) {
        case 0: // Info
            item->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
            break;
        case 1: // Warning
            item->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
            break;
        case 2: // Error
            item->setIcon(style()->standardIcon(QStyle::SP_MessageBoxCritical));
            break;
    }
    
    item->setText(text);
    item->setToolTip(text);
    
    // Store violation info
    RuleViolation violation{message, rule, severity};
    m_violations.push_back(violation);
    
    // Update item foreground color based on severity
    QColor textColor;
    switch (severity) {
        case 0: textColor = QColor("#2196F3"); break; // Blue for info
        case 1: textColor = QColor("#FF9800"); break; // Orange for warning
        case 2: textColor = QColor("#F44336"); break; // Red for error
    }
    item->setForeground(textColor);
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
    // In the future, this could show a menu to select hint level
    Q_EMIT hintRequested(1);
}

void ExercisePanel::onRuleViolationSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(m_violations.size())) {
        qDebug() << "Rule violation selected:" << m_violations[index].message;
        // The main window will handle highlighting the relevant part of the score
    }
}

} // namespace MusicTrainer::presentation