#include "presentation/VoiceChart.h"
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QLinearGradient>
#include <QtGui/QPen>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/Qt>
#include <algorithm>

namespace MusicTrainer::presentation {

using namespace MusicTrainer::music;

VoiceChart::VoiceChart(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
}

void VoiceChart::updateVoiceData(const Voice& voice)
{
    m_isRelationshipMode = false;
    m_voiceData = VoiceData();
    
    auto notes = voice.getAllNotes();
    if (notes.empty()) return;
    
    // Find range
    auto [minIt, maxIt] = std::minmax_element(notes.begin(), notes.end(),
        [](const auto& a, const auto& b) {
            return a.getPitch().getMidiNote() < b.getPitch().getMidiNote();
        });
    m_voiceData.lowestNote = minIt->getPitch().getMidiNote();
    m_voiceData.highestNote = maxIt->getPitch().getMidiNote();
    
    // Build interval distribution
    for (size_t i = 0; i < notes.size() - 1; ++i) {
        auto interval = Interval::fromPitches(notes[i].getPitch(), notes[i + 1].getPitch());
        m_voiceData.intervalDistribution[interval]++;
    }
    
    update();
}

void VoiceChart::updateVoiceRelationship(const Voice& voice1, const Voice& voice2)
{
    m_isRelationshipMode = true;
    m_voiceData = VoiceData();
    
    auto notes1 = voice1.getAllNotes();
    auto notes2 = voice2.getAllNotes();
    if (notes1.empty() || notes2.empty()) return;
    
    // Analyze harmonic intervals
    size_t minNotes = std::min(notes1.size(), notes2.size());
    for (size_t i = 0; i < minNotes; ++i) {
        auto interval = Interval::fromPitches(notes1[i].getPitch(), notes2[i].getPitch());
        m_voiceData.intervalDistribution[interval]++;
    }
    
    update();
}

void VoiceChart::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    if (m_isRelationshipMode) {
        // For voice relationships, just show the interval histogram
        drawIntervalHistogram(painter, rect);
    } else {
        // For single voice analysis, split the area between range and intervals
        QRect leftRect(rect.x(), rect.y(), rect.width() / 2, rect.height());
        QRect rightRect(rect.x() + rect.width() / 2, rect.y(), 
                       rect.width() / 2, rect.height());
        
        drawRangeChart(painter, leftRect);
        drawIntervalHistogram(painter, rightRect);
    }
}

void VoiceChart::drawRangeChart(QPainter& painter, const QRect& rect)
{
    // Draw piano-roll style range visualization
    int totalNotes = m_voiceData.highestNote - m_voiceData.lowestNote + 1;
    if (totalNotes <= 0) return;
    
    double noteHeight = rect.height() / static_cast<double>(totalNotes);
    
    // Draw the range as a gradient rectangle
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, QColor(100, 200, 255));
    gradient.setColorAt(1, QColor(50, 150, 255));
    
    painter.fillRect(rect, gradient);
    
    // Draw horizontal lines for each semitone
    painter.setPen(QPen(Qt::lightGray, 1));
    for (int i = 0; i <= totalNotes; ++i) {
        int y = rect.bottom() - static_cast<int>(i * noteHeight);
        painter.drawLine(rect.left(), y, rect.right(), y);
    }
    
    // Label some key notes
    painter.setPen(Qt::black);
    for (int note = m_voiceData.lowestNote; note <= m_voiceData.highestNote; note += 12) {
        int y = rect.bottom() - static_cast<int>((note - m_voiceData.lowestNote) * noteHeight);
        painter.drawText(rect.left() + 5, y - 5, 
                        QString("C%1").arg((note / 12) - 1)); // C4 is MIDI note 60
    }
}

void VoiceChart::drawIntervalHistogram(QPainter& painter, const QRect& rect)
{
    if (m_voiceData.intervalDistribution.empty()) return;
    
    // Find the maximum count for scaling
    int maxCount = 0;
    for (const auto& [interval, count] : m_voiceData.intervalDistribution) {
        maxCount = std::max(maxCount, count);
    }
    
    // Calculate bar width and spacing
    int numBars = m_voiceData.intervalDistribution.size();
    double barWidth = rect.width() / (numBars * 1.5); // Leave some space between bars
    
    // Draw bars
    int i = 0;
    for (const auto& [interval, count] : m_voiceData.intervalDistribution) {
        double height = (count * rect.height()) / static_cast<double>(maxCount);
        QRectF barRect(
            rect.left() + (i * barWidth * 1.5),
            rect.bottom() - height,
            barWidth,
            height
        );
        
        // Color based on interval type
        QColor color = interval.getQuality() == Interval::Quality::Perfect ?
            QColor(100, 200, 100) : QColor(200, 100, 100);
        
        painter.fillRect(barRect, color);
        painter.setPen(Qt::black);
        painter.drawRect(barRect);
        
        // Draw interval name
        painter.save();
        QPointF center = barRect.center();
        painter.translate(center);
        painter.rotate(-45);
        painter.drawText(QPointF(0, -5), 
                        QString::fromStdString(interval.toString()));
        painter.restore();
        
        i++;
    }
}

} // namespace MusicTrainer::presentation