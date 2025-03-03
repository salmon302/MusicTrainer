#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <memory>

namespace MusicTrainer {

namespace ports {
class MidiAdapter;
}

namespace presentation {

/**
 * @brief Control panel for transport and MIDI settings
 * 
 * This widget provides controls for playback, tempo, and MIDI device selection.
 */
class TransportControls : public QWidget {
    Q_OBJECT

public:
    explicit TransportControls(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget* parent = nullptr);
    ~TransportControls() override;

signals:
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

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onRecordClicked();
    void onTempoChanged(int bpm);
    void onMetronomeToggled(bool checked);
    void onDeviceSelectionChanged(int index);
    void refreshDeviceList();

private:
    void setupUi();
    void setupConnections();

    // MIDI service
    std::shared_ptr<ports::MidiAdapter> m_midiAdapter;

    // UI components
    QPushButton* m_playButton{nullptr};
    QPushButton* m_stopButton{nullptr};
    QPushButton* m_recordButton{nullptr};
    QSlider* m_tempoSlider{nullptr};
    QSpinBox* m_tempoSpinBox{nullptr};
    QCheckBox* m_metronomeCheckBox{nullptr};
    QComboBox* m_deviceComboBox{nullptr};
    QPushButton* m_refreshDevicesButton{nullptr};

    // State
    bool m_isPlaying{false};
    bool m_isRecording{false};
    int m_currentTempo{120};
    bool m_metronomeEnabled{true};
    int m_selectedDeviceIndex{-1};
};

} // namespace presentation
} // namespace MusicTrainer