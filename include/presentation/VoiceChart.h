#pragma once

#include <QtWidgets/QWidget>
#include "domain/music/Voice.h"
#include "domain/music/Interval.h"
#include <map>

QT_BEGIN_NAMESPACE
class QPainter;
class QPaintEvent;
class QRect;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class VoiceChart : public QWidget {
    Q_OBJECT

public:
    explicit VoiceChart(QWidget* parent = nullptr);

    void updateVoiceData(const MusicTrainer::music::Voice& voice);
    void updateVoiceRelationship(const MusicTrainer::music::Voice& voice1, 
                                const MusicTrainer::music::Voice& voice2);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct VoiceData {
        int lowestNote{0};
        int highestNote{0};
        std::map<MusicTrainer::music::Interval, int> intervalDistribution;
    };

    void drawRangeChart(QPainter& painter, const QRect& rect);
    void drawIntervalHistogram(QPainter& painter, const QRect& rect);

    VoiceData m_voiceData;
    bool m_isRelationshipMode{false};
};

} // namespace MusicTrainer::presentation