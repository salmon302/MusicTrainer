#include "presentation/TransportControls.h"
#include "domain/ports/MidiAdapter.h"
#include "domain/ports/MidiPort.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QIcon>
#include <QMessageBox>

namespace MusicTrainer {
namespace presentation {

TransportControls::TransportControls(std::shared_ptr<ports::MidiAdapter> midiAdapter,
                                         std::shared_ptr<music::events::EventBus> eventBus,
                                         QWidget* parent)
    : QWidget(parent)
    , m_midiAdapter(std::move(midiAdapter))
    , m_eventBus(std::move(eventBus))
{
    setupUi();
    connectSignals();
    
    // Initial device list population if MIDI adapter is available
    if (m_midiAdapter) {
        refreshDeviceList();
        
        // Restore previous device settings if available
        if (m_midiAdapter->isOpen()) {
            m_deviceComboBox->setCurrentIndex(m_midiAdapter->getCurrentOutputDevice());
            m_midiThroughCheckBox->setChecked(m_midiAdapter->getMidiThrough());
            m_latencySpinBox->setValue(m_midiAdapter->getLatency());
        }
    }
}

TransportControls::~TransportControls() = default;

void TransportControls::setupUi()
{
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Playback Controls Group
    QGroupBox* playbackGroup = new QGroupBox(tr("Playback"), this);
    QHBoxLayout* playbackLayout = new QHBoxLayout(playbackGroup);
    
    // Transport buttons
    m_playButton = new QPushButton(tr("Play"), this);
    m_playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    m_playButton->setToolTip(tr("Start playback"));
    
    m_stopButton = new QPushButton(tr("Stop"), this);
    m_stopButton->setIcon(QIcon::fromTheme("media-playback-stop"));
    m_stopButton->setToolTip(tr("Stop playback"));
    m_stopButton->setEnabled(false);
    
    m_recordButton = new QPushButton(tr("Record"), this);
    m_recordButton->setIcon(QIcon::fromTheme("media-record"));
    m_recordButton->setToolTip(tr("Start recording"));
    m_recordButton->setCheckable(true);
    
    playbackLayout->addWidget(m_playButton);
    playbackLayout->addWidget(m_stopButton);
    playbackLayout->addWidget(m_recordButton);
    
    // Tempo Group
    QGroupBox* tempoGroup = new QGroupBox(tr("Tempo"), this);
    QHBoxLayout* tempoLayout = new QHBoxLayout(tempoGroup);
    
    m_tempoSlider = new QSlider(Qt::Horizontal, this);
    m_tempoSlider->setRange(40, 208);
    m_tempoSlider->setValue(m_tempo);
    m_tempoSlider->setToolTip(tr("Adjust tempo"));
    
    m_tempoSpinBox = new QSpinBox(this);
    m_tempoSpinBox->setRange(40, 208);
    m_tempoSpinBox->setValue(m_tempo);
    m_tempoSpinBox->setSuffix(" BPM");
    m_tempoSpinBox->setToolTip(tr("Tempo in beats per minute"));
    
    m_metronomeCheckBox = new QCheckBox(tr("Metronome"), this);
    m_metronomeCheckBox->setChecked(m_metronomeEnabled);
    m_metronomeCheckBox->setToolTip(tr("Enable metronome during playback"));
    
    tempoLayout->addWidget(m_tempoSlider);
    tempoLayout->addWidget(m_tempoSpinBox);
    tempoLayout->addWidget(m_metronomeCheckBox);
    
    // MIDI Device Group
    QGroupBox* midiGroup = new QGroupBox(tr("MIDI Device"), this);
    QVBoxLayout* midiLayout = new QVBoxLayout(midiGroup);
    
    QHBoxLayout* deviceLayout = new QHBoxLayout();
    m_deviceComboBox = new QComboBox(this);
    m_deviceComboBox->setToolTip(tr("Select MIDI output device"));
    m_deviceComboBox->addItem(tr("No MIDI Devices Available"));
    m_deviceComboBox->setEnabled(false);
    
    m_refreshDevicesButton = new QPushButton(tr("Refresh"), this);
    m_refreshDevicesButton->setToolTip(tr("Refresh MIDI device list"));
    m_refreshDevicesButton->setIcon(QIcon::fromTheme("view-refresh"));
    
    deviceLayout->addWidget(m_deviceComboBox, 3);
    deviceLayout->addWidget(m_refreshDevicesButton, 1);
    
    QHBoxLayout* midiOptionsLayout = new QHBoxLayout();
    m_midiThroughCheckBox = new QCheckBox(tr("MIDI Through"), this);
    m_midiThroughCheckBox->setToolTip(tr("Enable MIDI input to output passthrough"));
    
    QLabel* latencyLabel = new QLabel(tr("Latency:"), this);
    m_latencySpinBox = new QSpinBox(this);
    m_latencySpinBox->setRange(1, 100);
    m_latencySpinBox->setValue(10);
    m_latencySpinBox->setSuffix(tr(" ms"));
    m_latencySpinBox->setToolTip(tr("MIDI processing latency"));
    
    midiOptionsLayout->addWidget(m_midiThroughCheckBox);
    midiOptionsLayout->addWidget(latencyLabel);
    midiOptionsLayout->addWidget(m_latencySpinBox);
    midiOptionsLayout->addStretch();
    
    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    
    midiLayout->addLayout(deviceLayout);
    midiLayout->addLayout(midiOptionsLayout);
    midiLayout->addWidget(m_statusLabel);
    
    // Add groups to main layout
    mainLayout->addWidget(playbackGroup);
    mainLayout->addWidget(tempoGroup);
    mainLayout->addWidget(midiGroup);
}

void TransportControls::connectSignals()
{
    // Transport controls
    connect(m_playButton, &QPushButton::clicked, this, &TransportControls::onPlayClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &TransportControls::onStopClicked);
    connect(m_recordButton, &QPushButton::clicked, this, &TransportControls::onRecordClicked);
    
    // Tempo controls
    connect(m_tempoSlider, &QSlider::valueChanged, this, &TransportControls::onTempoChanged);
    connect(m_tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &TransportControls::onTempoChanged);
    connect(m_metronomeCheckBox, &QCheckBox::toggled, this, &TransportControls::onMetronomeToggled);
    
    // MIDI device controls
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TransportControls::onDeviceChanged);
    connect(m_refreshDevicesButton, &QPushButton::clicked,
            this, &TransportControls::onRefreshDevicesClicked);
    connect(m_midiThroughCheckBox, &QCheckBox::toggled, this, &TransportControls::onMidiThroughToggled);
    connect(m_latencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TransportControls::onLatencyChanged);
}

void TransportControls::onPlayClicked()
{
    m_isPlaying = !m_isPlaying;
    updatePlaybackState();
    publishStateChange();
    
    if (m_isPlaying) {
        Q_EMIT playbackStarted();
    } else {
        Q_EMIT playbackStopped();
    }
}

void TransportControls::onStopClicked()
{
    m_isPlaying = false;
    m_isRecording = false;
    updatePlaybackState();
    publishStateChange();
    Q_EMIT playbackStopped();
}

void TransportControls::onRecordClicked()
{
    m_isRecording = m_recordButton->isChecked();
    updatePlaybackState();
    publishStateChange();
    
    if (m_isRecording) {
        Q_EMIT recordingStarted();
    }
}

void TransportControls::onTempoChanged(int value)
{
    // Sync slider and spinbox
    if (sender() == m_tempoSlider) {
        m_tempoSpinBox->setValue(value);
    } else if (sender() == m_tempoSpinBox) {
        m_tempoSlider->setValue(value);
    }
    
    m_tempo = value;
    publishStateChange();
    Q_EMIT tempoChanged(value);
}

void TransportControls::onMetronomeToggled(bool checked)
{
    m_metronomeEnabled = checked;
    publishStateChange();
    Q_EMIT metronomeToggled(checked);
}

void TransportControls::onDeviceChanged(int index)
{
    if (!m_midiAdapter) return;
    
    if (index >= 0) {
        // Close current device if open
        if (m_midiAdapter->isOpen()) {
            m_midiAdapter->close();
        }
        
        try {
            // Try to set and open the new device
            if (m_midiAdapter->setOutputDevice(index) && m_midiAdapter->open()) {
                m_statusLabel->setText(tr("Connected to %1").arg(m_deviceComboBox->currentText()));
                m_statusLabel->setStyleSheet("color: green;");
                publishStateChange();
                Q_EMIT midiDeviceSelected(index);
            } else {
                throw std::runtime_error("Failed to open MIDI device");
            }
        } catch (const std::exception& e) {
            handleMidiError(e.what());
        }
    }
}

void TransportControls::onRefreshDevicesClicked()
{
    refreshDeviceList();
}

void TransportControls::refreshDeviceList()
{
    if (!m_midiAdapter) {
        m_deviceComboBox->clear();
        m_deviceComboBox->addItem(tr("No MIDI Interface Available"));
        m_deviceComboBox->setEnabled(false);
        return;
    }
    
    m_deviceComboBox->clear();
    const auto outputs = m_midiAdapter->getAvailableOutputs();
    
    if (outputs.empty()) {
        m_deviceComboBox->addItem(tr("No MIDI Outputs Available"));
        m_deviceComboBox->setEnabled(false);
    } else {
        for (const auto& output : outputs) {
            m_deviceComboBox->addItem(QString::fromStdString(output));
        }
        m_deviceComboBox->setEnabled(true);
    }
}

void TransportControls::onMidiThroughToggled(bool enabled)
{
    if (!m_midiAdapter) return;
    
    try {
        m_midiAdapter->setMidiThrough(enabled);
        publishStateChange();
    } catch (const std::exception& e) {
        handleMidiError(e.what());
    }
}

void TransportControls::onLatencyChanged(int value)
{
    if (!m_midiAdapter) return;
    
    try {
        m_midiAdapter->setLatency(value);
        publishStateChange();
    } catch (const std::exception& e) {
        handleMidiError(e.what());
    }
}

void TransportControls::handleMidiError(const std::string& error)
{
    QString errorMsg = QString::fromStdString(error);
    m_statusLabel->setText(tr("Error: %1").arg(errorMsg));
    m_statusLabel->setStyleSheet("color: red;");
    Q_EMIT midiError(errorMsg);
}

void TransportControls::updatePlaybackState()
{
    m_playButton->setText(m_isPlaying ? tr("Pause") : tr("Play"));
    m_playButton->setIcon(QIcon::fromTheme(m_isPlaying ? "media-playback-pause" : "media-playback-start"));
    m_stopButton->setEnabled(m_isPlaying || m_isRecording);
    m_recordButton->setEnabled(!m_isPlaying);
}

void TransportControls::setPlaybackState(bool isPlaying)
{
    if (m_isPlaying != isPlaying) {
        m_isPlaying = isPlaying;
        updatePlaybackState();
    }
}

void TransportControls::setRecordingState(bool isRecording)
{
    if (m_isRecording != isRecording) {
        m_isRecording = isRecording;
        m_recordButton->setChecked(isRecording);
        updatePlaybackState();
    }
}

void TransportControls::setTempo(int tempo)
{
    if (m_tempo != tempo) {
        m_tempo = tempo;
        m_tempoSlider->setValue(tempo);
        m_tempoSpinBox->setValue(tempo);
    }
}

void TransportControls::setMetronomeEnabled(bool enabled)
{
    if (m_metronomeEnabled != enabled) {
        m_metronomeEnabled = enabled;
        m_metronomeCheckBox->setChecked(enabled);
    }
}

void TransportControls::publishStateChange()
{
    // Create playback state event
    music::events::GuiStateEvent::PlaybackState state{
        m_isPlaying,
        m_isRecording,
        m_tempo,
        m_metronomeEnabled
    };
    
    // Create MIDI device state event
    music::events::GuiStateEvent::MidiDeviceState deviceState{
        -1, // Input device not handled by transport controls
        m_deviceComboBox->currentIndex(),
        m_midiThroughCheckBox->isChecked(),
        m_latencySpinBox->value()
    };
    
    // Create and publish the state change events
    auto playbackEvent = music::events::GuiStateEvent::create(
        music::events::GuiStateEvent::StateType::PLAYBACK_STATE_CHANGE,
        state,
        "TransportControls"
    );
    
    auto deviceEvent = music::events::GuiStateEvent::create(
        music::events::GuiStateEvent::StateType::MIDI_DEVICE_CHANGE,
        deviceState,
        "TransportControls"
    );

    if (m_eventBus) {
        m_eventBus->publishAsync(std::move(playbackEvent));
        m_eventBus->publishAsync(std::move(deviceEvent));
    }
}

} // namespace presentation
} // namespace MusicTrainer