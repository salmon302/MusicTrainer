#pragma once

#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>
#include <memory>
#include <vector>
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"

namespace MusicTrainer::presentation::viewmodels {

class ScoreViewModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ScoreViewModel(QObject* parent = nullptr);
    ~ScoreViewModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setScore(std::shared_ptr<music::Score> score);
    std::shared_ptr<music::Score> getScore() const;

    void addNote(int voiceIndex, int pitch, double duration, int position);
    void removeNote(int voiceIndex, int position);
    void addVoice();
    void removeVoice(int voiceIndex);
    int getVoiceCount() const;
    bool isDirty() const;

signals:
    void scoreChanged();
    void noteAdded(int voiceIndex, const music::Note& note);
    void noteRemoved(int voiceIndex, int position);
    void voiceAdded(const music::Voice& voice);
    void voiceRemoved(int voiceIndex);

private:
    std::shared_ptr<music::Score> m_score;
    bool m_isDirty{false};
    std::vector<music::Voice> m_voices;
};

} // namespace MusicTrainer::presentation::viewmodels