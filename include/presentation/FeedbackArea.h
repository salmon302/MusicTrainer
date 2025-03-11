#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QListWidget>
#include "domain/music/Score.h"
#include "presentation/VoiceChart.h"
#include "domain/analysis/VoiceAnalyzer.h"
#include <memory>
#include <vector>
#include <string>

QT_BEGIN_NAMESPACE
class QHeaderView;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class FeedbackArea : public QWidget {
    Q_OBJECT

public:
    explicit FeedbackArea(QWidget* parent = nullptr);
    ~FeedbackArea() override;

    void updateWithScore(const MusicTrainer::music::Score& score);
    void addValidationMessage(const QString& message, bool isError);
    void clearFeedback();

Q_SIGNALS:
    void feedbackItemSelected(int position, int voiceIndex);

private Q_SLOTS:
    void onTabChanged(int index);
    void onVoiceAnalysisItemClicked(int row, int column);
    void onValidationMessageClicked(QListWidgetItem* item);

private:
    void setupUi();
    void updateVoiceAnalysis(const MusicTrainer::music::Score& score);
    void updateIntervalAnalysis(const MusicTrainer::music::Score& score);
    QString formatIntervalList(const std::vector<MusicTrainer::music::Interval>& intervals);

    VoiceChart* m_chartWidget{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    QTableWidget* m_voiceAnalysisTable{nullptr};
    QTableWidget* m_intervalAnalysisTable{nullptr};
    QTextEdit* m_improvementsSuggestions{nullptr};
    std::unique_ptr<MusicTrainer::music::analysis::VoiceAnalyzer> m_analyzer;

    struct ValidationLink {
        QString message;
        int position;
        int voiceIndex;
        bool isError;
    };
    std::vector<ValidationLink> m_validationLinks;
};

} // namespace MusicTrainer::presentation