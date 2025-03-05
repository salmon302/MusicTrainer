#include "presentation/viewmodels/ScoreViewModel.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Note.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <QDebug>

using MusicTrainer::music::Voice;
using MusicTrainer::music::Note;
using MusicTrainer::music::Score;
using MusicTrainer::music::Pitch;
using MusicTrainer::music::Duration;

namespace MusicTrainer::presentation::viewmodels {

ScoreViewModel::ScoreViewModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_score(nullptr)
    , m_isDirty(false)
{
}

ScoreViewModel::~ScoreViewModel() = default;

void ScoreViewModel::setScore(std::shared_ptr<Score> score)
{
    if (m_score == score) {
        return;
    }
    
    // Disconnect from old score if it exists
    if (m_score) {
        // In a complete implementation, we would disconnect signal/slot connections
        // to the old score here
    }
    
    m_score = score;
    m_isDirty = false;
    
    // Connect to new score
    if (m_score) {
        // In a complete implementation, we would connect to the score's events
        // (e.g., note added, note removed) to update the UI
    }
    
    // Notify that the score has changed
    emit scoreChanged();
}

std::shared_ptr<Score> ScoreViewModel::getScore() const
{
    return m_score;
}

void ScoreViewModel::addNote(int voiceIndex, int pitch, double duration, int position)
{
    if (!m_score) {
        return;
    }
    
    // Make sure we have enough voices
    while (getVoiceCount() <= voiceIndex) {
        addVoice();
    }
    
    try {
        // Create note with proper constructor
        Pitch notePitch = Pitch::fromMidiNote(pitch);
        Note note(notePitch, duration, position);
        
        // Add note to the voice
        auto voice = m_score->getVoice(voiceIndex);
        if (voice) {
            voice->addNote(notePitch, duration, position);
            m_isDirty = true;
            emit noteAdded(voiceIndex, note);
        }
    } catch (const std::exception& e) {
        qWarning("Failed to add note: %s", e.what());
    }
}

void ScoreViewModel::removeNote(int voiceIndex, int position)
{
    if (!m_score || voiceIndex >= getVoiceCount()) {
        return;
    }
    
    try {
        // Get the voice and remove the note
        if (auto voice = m_score->getVoice(voiceIndex)) {
            voice->removeNote(position);
            m_isDirty = true;
            emit noteRemoved(voiceIndex, position);
        }
    } catch (const std::exception& e) {
        qWarning("Failed to remove note: %s", e.what());
    }
}

void ScoreViewModel::addVoice()
{
    if (!m_score) {
        return;
    }
    
    try {
        auto voice = Voice::create(m_score->getTimeSignature());
        if (voice) {
            m_score->addVoice(std::move(voice));
            m_isDirty = true;
            // Get the newly added voice to emit signal
            if (auto* addedVoice = m_score->getVoice(getVoiceCount() - 1)) {
                emit voiceAdded(*addedVoice);
            }
        }
    } catch (const std::exception& e) {
        qWarning("Failed to add voice: %s", e.what());
    }
}

void ScoreViewModel::removeVoice(int voiceIndex)
{
    if (!m_score || voiceIndex >= getVoiceCount()) {
        return;
    }
    
    try {
        // Remove the voice from the score in the future implementation
        // m_score->removeVoice(voiceIndex);
        
        m_isDirty = true;
        
        // Emit signal that a voice was removed
        emit voiceRemoved(voiceIndex);
    } catch (const std::exception& e) {
        // Handle exceptions from domain model
        qWarning("Failed to remove voice: %s", e.what());
    }
}

int ScoreViewModel::getVoiceCount() const
{
    if (!m_score) {
        return 0;
    }
    
    // Return the number of voices in the score
    return m_score->getVoiceCount();
}

bool ScoreViewModel::isDirty() const
{
    return m_isDirty;
}

int ScoreViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_score ? m_score->getVoiceCount() : 0; // Return voice count or 0 if no score
}

QVariant ScoreViewModel::data(const QModelIndex& index, int role) const
{
    if (!m_score || !index.isValid() || role != Qt::DisplayRole) {
        return QVariant(); // Return empty QVariant for invalid cases
    }

    int row = index.row();
    if (row < 0 || row >= getVoiceCount()) {
        return QVariant(); // Return empty QVariant for invalid rows
    }

    // For now, just return voice index as string for display
    return QString::number(row);
}

} // namespace MusicTrainer::presentation::viewmodels