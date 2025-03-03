#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QWidget>
#include <memory>
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QResizeEvent;
class QMouseEvent;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class ScoreView : public QGraphicsView {
    Q_OBJECT
public:
    explicit ScoreView(QWidget* parent = nullptr);
    ~ScoreView() override;

    void updateScore(const music::Score& score);
    void clearScore();
    void highlightNote(int position, int voiceIndex);

signals:
    void noteSelected(int position, int voiceIndex);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    class ScoreViewImpl;
    std::unique_ptr<ScoreViewImpl> m_impl;
};

} // namespace MusicTrainer::presentation