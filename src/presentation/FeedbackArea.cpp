#include "presentation/FeedbackArea.h"
#include "presentation/VoiceChart.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <QDebug>
#include <sstream>

namespace MusicTrainer {
namespace presentation {

FeedbackArea::FeedbackArea(QWidget* parent)
    : QWidget(parent)
    , m_analyzer(music::analysis::VoiceAnalyzer::create())
{
    setupUi();
}

FeedbackArea::~FeedbackArea() = default;

void FeedbackArea::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    
    // Create charts
    m_chartWidget = new VoiceChart(this);
    m_chartWidget->setMinimumHeight(150);
    
    m_tabWidget = new QTabWidget(this);
    
    // Voice Analysis Tab
    auto* voiceAnalysisWidget = new QWidget(this);
    auto* voiceAnalysisLayout = new QVBoxLayout(voiceAnalysisWidget);
    m_voiceAnalysisTable = new QTableWidget(this);
    m_voiceAnalysisTable->setColumnCount(4);
    m_voiceAnalysisTable->setHorizontalHeaderLabels({tr("Voice"), tr("Range"), tr("Intervals"), tr("Issues")});
    m_voiceAnalysisTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    voiceAnalysisLayout->addWidget(m_voiceAnalysisTable);
    voiceAnalysisWidget->setLayout(voiceAnalysisLayout);
    
    // Interval Analysis Tab
    auto* intervalAnalysisWidget = new QWidget(this);
    auto* intervalAnalysisLayout = new QVBoxLayout(intervalAnalysisWidget);
    m_intervalAnalysisTable = new QTableWidget(this);
    m_intervalAnalysisTable->setColumnCount(3);
    m_intervalAnalysisTable->setHorizontalHeaderLabels({tr("Voice Pair"), tr("Common Intervals"), tr("Problems")});
    m_intervalAnalysisTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    intervalAnalysisLayout->addWidget(m_intervalAnalysisTable);
    intervalAnalysisWidget->setLayout(intervalAnalysisLayout);
    
    // Improvements Tab
    auto* improvementsWidget = new QWidget(this);
    auto* improvementsLayout = new QVBoxLayout(improvementsWidget);
    m_improvementsSuggestions = new QTextEdit(this);
    m_improvementsSuggestions->setReadOnly(true);
    improvementsLayout->addWidget(m_improvementsSuggestions);
    improvementsWidget->setLayout(improvementsLayout);
    
    // Add tabs
    m_tabWidget->addTab(voiceAnalysisWidget, tr("Voice Analysis"));
    m_tabWidget->addTab(intervalAnalysisWidget, tr("Interval Analysis"));
    m_tabWidget->addTab(improvementsWidget, tr("Improvements"));
    
    layout->addWidget(m_chartWidget);
    layout->addWidget(m_tabWidget);
    
    setLayout(layout);
    
    // Connect signals
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &FeedbackArea::onTabChanged);
    connect(m_voiceAnalysisTable, &QTableWidget::cellClicked, this, &FeedbackArea::onVoiceAnalysisItemClicked);
}

void FeedbackArea::updateWithScore(const music::Score& score)
{
    updateVoiceAnalysis(score);
    updateIntervalAnalysis(score);
    
    // Update chart based on current tab
    int currentTab = m_tabWidget->currentIndex();
    if (currentTab == 0 && score.getVoiceCount() > 0) {
        // Voice Analysis tab - show first voice by default
        if (auto* voice = score.getVoice(0)) {
            static_cast<VoiceChart*>(m_chartWidget)->updateVoiceData(*voice);
        }
    } else if (currentTab == 1 && score.getVoiceCount() >= 2) {
        // Interval Analysis tab - show first two voices by default
        if (auto* voice1 = score.getVoice(0)) {
            if (auto* voice2 = score.getVoice(1)) {
                static_cast<VoiceChart*>(m_chartWidget)->updateVoiceRelationship(*voice1, *voice2);
            }
        }
    }
    
    // Update improvement suggestions based on analysis
    std::stringstream suggestions;
    suggestions << "Voice Analysis:\n";
    for (size_t i = 0; i < score.getVoiceCount(); ++i) {
        if (auto* voice = score.getVoice(i)) {
            auto chars = m_analyzer->analyzeVoice(*voice);
            if (!chars.issues.empty()) {
                suggestions << "\nVoice " << (i + 1) << ":\n";
                for (const auto& issue : chars.issues) {
                    suggestions << "- " << issue << "\n";
                }
            }
        }
    }
    m_improvementsSuggestions->setText(QString::fromStdString(suggestions.str()));
}

void FeedbackArea::updateVoiceAnalysis(const music::Score& score)
{
    m_voiceAnalysisTable->setRowCount(score.getVoiceCount());
    
    for (size_t i = 0; i < score.getVoiceCount(); ++i) {
        if (auto* voice = score.getVoice(i)) {
            // Voice name
            auto* voiceItem = new QTableWidgetItem(QString("Voice %1").arg(i + 1));
            m_voiceAnalysisTable->setItem(i, 0, voiceItem);
            
            // Analyze voice
            auto chars = m_analyzer->analyzeVoice(*voice);
            
            // Range
            QString range = QString("%1-%2")
                .arg(QString::fromStdString(chars.lowestPitch.toString()))
                .arg(QString::fromStdString(chars.highestPitch.toString()));
            auto* rangeItem = new QTableWidgetItem(range);
            m_voiceAnalysisTable->setItem(i, 1, rangeItem);
            
            // Intervals
            auto* intervalItem = new QTableWidgetItem(formatIntervalList(chars.commonIntervals));
            m_voiceAnalysisTable->setItem(i, 2, intervalItem);
            
            // Issues
            QString issues = chars.issues.empty() ? "None" : 
                QString::fromStdString(chars.issues.front());
            auto* issuesItem = new QTableWidgetItem(issues);
            m_voiceAnalysisTable->setItem(i, 3, issuesItem);
        }
    }
}

void FeedbackArea::updateIntervalAnalysis(const music::Score& score)
{
    size_t voiceCount = score.getVoiceCount();
    size_t pairCount = (voiceCount * (voiceCount - 1)) / 2;
    m_intervalAnalysisTable->setRowCount(pairCount);
    
    size_t row = 0;
    for (size_t i = 0; i < voiceCount - 1; ++i) {
        for (size_t j = i + 1; j < voiceCount; ++j) {
            auto* voice1 = score.getVoice(i);
            auto* voice2 = score.getVoice(j);
            if (!voice1 || !voice2) continue;
            
            // Voice pair label
            QString pairLabel = QString("Voice %1 - Voice %2")
                .arg(i + 1).arg(j + 1);
            auto* pairItem = new QTableWidgetItem(pairLabel);
            m_intervalAnalysisTable->setItem(row, 0, pairItem);
            
            // Analyze intervals between voices
            auto intervals = m_analyzer->analyzeVoiceRelationship(*voice1, *voice2);
            auto* intervalsItem = new QTableWidgetItem(formatIntervalList(intervals));
            m_intervalAnalysisTable->setItem(row, 1, intervalsItem);
            
            // Problems (placeholder for now)
            auto* problemsItem = new QTableWidgetItem("None");
            m_intervalAnalysisTable->setItem(row, 2, problemsItem);
            
            row++;
        }
    }
}

QString FeedbackArea::formatIntervalList(const std::vector<music::Interval>& intervals)
{
    QStringList formatted;
    for (const auto& interval : intervals) {
        formatted.append(QString::fromStdString(interval.toString()));
    }
    return formatted.join(", ");
}

void FeedbackArea::onTabChanged(int index)
{
    if (!m_chartWidget) return;
    
    auto* score = dynamic_cast<const music::Score*>(property("currentScore").value<void*>());
    if (!score) return;
    
    if (index == 0 && score->getVoiceCount() > 0) {
        // Voice Analysis tab - show first voice
        if (auto* voice = score->getVoice(0)) {
            static_cast<VoiceChart*>(m_chartWidget)->updateVoiceData(*voice);
        }
    } else if (index == 1 && score->getVoiceCount() >= 2) {
        // Interval Analysis tab - show first two voices
        if (auto* voice1 = score->getVoice(0)) {
            if (auto* voice2 = score->getVoice(1)) {
                static_cast<VoiceChart*>(m_chartWidget)->updateVoiceRelationship(*voice1, *voice2);
            }
        }
    }
}

void FeedbackArea::onVoiceAnalysisItemClicked(int row, int column)
{
    // When a cell in the voice analysis is clicked, emit a signal to highlight that voice
    if (row >= 0 && row < m_voiceAnalysisTable->rowCount()) {
        emit feedbackItemSelected(0, row);
    }
}

void FeedbackArea::onValidationMessageClicked(QListWidgetItem* item)
{
    // Future: Add validation message handling
}

void FeedbackArea::addValidationMessage(const QString& message, bool isError)
{
    // Add to improvement suggestions for now
    QString coloredMessage = isError ? 
        QString("<span style='color: red'>%1</span>").arg(message) :
        QString("<span style='color: orange'>%1</span>").arg(message);
    m_improvementsSuggestions->append(coloredMessage);
}

void FeedbackArea::clearFeedback()
{
    m_voiceAnalysisTable->setRowCount(0);
    m_intervalAnalysisTable->setRowCount(0);
    m_improvementsSuggestions->clear();
    m_validationLinks.clear();
}

} // namespace presentation
} // namespace MusicTrainer