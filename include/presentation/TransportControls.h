#pragma once

#include <QWidget>
#include <memory>
#include "domain/ports/MidiAdapter.h"
#include "domain/events/GuiStateEvent.h"
#include "domain/events/EventBus.h"

class QPushButton;
class QSlider;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;

namespace MusicTrainer::presentation {

/**
 * @brief Control panel for transport and MIDI settings
 * 
 * This widget provides controls for playback, tempo, and MIDI device selection.
 */
class TransportControls : public QWidget {
    Q_OBJECT

public:
    explicit TransportControls(std::shared_ptr<ports::MidiAdapter> midiAdapter,
                             std::shared_ptr<music::events::EventBus> eventBus = nullptr,
                             QWidget* parent = nullptr);
    ~TransportControls() override;

    // State setters
    void setPlaybackState(bool isPlaying);
    void setRecordingState(bool isRecording);
    void setTempo(int tempo);
    void setMetronomeEnabled(bool enabled);

    // Device management
    void refreshDeviceList();

Q_SIGNALS:
    /**
     * @brief Emitted when playback should start
     */
    void playbackStarted();

    /**
     * @brief Emitted when playback should stop
     */
    void playbackStopped();

    /**
     * @brief Emitted when recording should start
     */
    void recordingStarted();

    /**
     * @brief Emitted when the tempo changes
     * @param bpm New tempo in beats per minute
     */
    void tempoChanged(int bpm);

    /**
     * @brief Emitted when the metronome setting changes
     * @param enabled Whether the metronome is enabled
     */
    void metronomeToggled(bool enabled);

    /**
     * @brief Emitted when a MIDI device is selected
     * @param portIndex Index of the selected MIDI port
     */
    void midiDeviceSelected(int portIndex);

    /**
     * @brief Emitted when a MIDI error occurs
     * @param error Error message
     */
    void midiError(const QString& error);

private Q_SLOTS:
    void onPlayClicked();
    void onStopClicked();
    void onRecordClicked();
    void onTempoChanged(int value);
    void onMetronomeToggled(bool checked);
    void onDeviceChanged(int index);
    void onRefreshDevicesClicked();
    void onMidiThroughToggled(bool enabled);
    void onLatencyChanged(int value);

private:
    void setupUi();
    void connectSignals();
    void updatePlaybackState();
    void publishStateChange();
    void handleMidiError(const std::string& error);

    // Dependencies
    std::shared_ptr<ports::MidiAdapter> m_midiAdapter;
    std::shared_ptr<music::events::EventBus> m_eventBus;

    // UI Components
    QPushButton* m_playButton{nullptr};
    QPushButton* m_stopButton{nullptr};
    QPushButton* m_recordButton{nullptr};
    QSlider* m_tempoSlider{nullptr};
    QSpinBox* m_tempoSpinBox{nullptr};
    QCheckBox* m_metronomeCheckBox{nullptr};
    QComboBox* m_deviceComboBox{nullptr};
    QPushButton* m_refreshDevicesButton{nullptr};
    QCheckBox* m_midiThroughCheckBox{nullptr};
    QSpinBox* m_latencySpinBox{nullptr};
    QLabel* m_statusLabel{nullptr};

    // State
    bool m_isPlaying{false};
    bool m_isRecording{false};
    int m_tempo{120};
    bool m_metronomeEnabled{false};
};

} // namespace MusicTrainer::presentation