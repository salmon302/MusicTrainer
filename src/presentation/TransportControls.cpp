#include "presentation/TransportControls.h"
#include "domain/ports/MidiAdapter.h"
#include "domain/ports/MidiPort.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QIcon>

namespace MusicTrainer {
namespace presentation {

TransportControls::TransportControls(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget* parent)
    : QWidget(parent)
    , m_midiAdapter(std::move(midiAdapter))
{
    setupUi();
    setupConnections();
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
    
    playbackLayout->addWidget(m_playButton);
    playbackLayout->addWidget(m_stopButton);
    playbackLayout->addWidget(m_recordButton);
    
    // Tempo Group
    QGroupBox* tempoGroup = new QGroupBox(tr("Tempo"), this);
    QHBoxLayout* tempoLayout = new QHBoxLayout(tempoGroup);
    
    m_tempoSlider = new QSlider(Qt::Horizontal, this);
    m_tempoSlider->setRange(40, 240);
    m_tempoSlider->setValue(m_currentTempo);
    m_tempoSlider->setToolTip(tr("Adjust tempo"));
    
    m_tempoSpinBox = new QSpinBox(this);
    m_tempoSpinBox->setRange(40, 240);
    m_tempoSpinBox->setValue(m_currentTempo);
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
    QHBoxLayout* midiLayout = new QHBoxLayout(midiGroup);
    
    m_deviceComboBox = new QComboBox(this);
    m_deviceComboBox->setToolTip(tr("Select MIDI output device"));
    m_deviceComboBox->addItem(tr("No MIDI Devices Available"));
    m_deviceComboBox->setEnabled(false);
    
    m_refreshDevicesButton = new QPushButton(tr("Refresh"), this);
    m_refreshDevicesButton->setToolTip(tr("Refresh MIDI device list"));
    m_refreshDevicesButton->setIcon(QIcon::fromTheme("view-refresh"));
    
    midiLayout->addWidget(m_deviceComboBox, 3);
    midiLayout->addWidget(m_refreshDevicesButton, 1);
    
    // Add groups to main layout
    mainLayout->addWidget(playbackGroup);
    mainLayout->addWidget(tempoGroup);
    mainLayout->addWidget(midiGroup);
    
    // Set the main layout
    setLayout(mainLayout);
    
    // Initial device refresh if MIDI adapter is available
    if (m_midiAdapter) {
        refreshDeviceList();
    }
}

void TransportControls::setupConnections()
{
    // Transport button connections
    connect(m_playButton, &QPushButton::clicked, this, &TransportControls::onPlayClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &TransportControls::onStopClicked);
    connect(m_recordButton, &QPushButton::clicked, this, &TransportControls::onRecordClicked);
    
    // Tempo control connections
    connect(m_tempoSlider, &QSlider::valueChanged, m_tempoSpinBox, &QSpinBox::setValue);
    connect(m_tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_tempoSlider, &QSlider::setValue);
    connect(m_tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TransportControls::onTempoChanged);
    
    // Metronome connection
    connect(m_metronomeCheckBox, &QCheckBox::toggled, this, &TransportControls::onMetronomeToggled);
    
    // MIDI device connections
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TransportControls::onDeviceSelectionChanged);
    connect(m_refreshDevicesButton, &QPushButton::clicked, this, &TransportControls::refreshDeviceList);
}

void TransportControls::onPlayClicked()
{
    m_isPlaying = true;
    m_playButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_recordButton->setEnabled(false);
    
    emit playbackStarted();
}

void TransportControls::onStopClicked()
{
    m_isPlaying = false;
    m_isRecording = false;
    m_playButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_recordButton->setEnabled(true);
    
    emit playbackStopped();
}

void TransportControls::onRecordClicked()
{
    m_isRecording = true;
    m_isPlaying = true;
    m_playButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_recordButton->setEnabled(false);
    
    emit recordingStarted();
}

void TransportControls::onTempoChanged(int bpm)
{
    m_currentTempo = bpm;
    emit tempoChanged(bpm);
}

void TransportControls::onMetronomeToggled(bool checked)
{
    m_metronomeEnabled = checked;
    emit metronomeToggled(checked);
}

void TransportControls::onDeviceSelectionChanged(int index)
{
    if (index >= 0) {
        m_selectedDeviceIndex = index;
        emit midiDeviceSelected(index);
    }
}

void TransportControls::refreshDeviceList()
{
    // Clear the combo box
    m_deviceComboBox->clear();
    
    if (!m_midiAdapter) {
        m_deviceComboBox->addItem(tr("No MIDI Interface Available"));
        m_deviceComboBox->setEnabled(false);
        return;
    }
    
    // Get available MIDI output devices
    const auto devices = m_midiAdapter->getAvailableOutputs();
    
    if (devices.empty()) {
        m_deviceComboBox->addItem(tr("No MIDI Devices Available"));
        m_deviceComboBox->setEnabled(false);
        return;
    }
    
    // Add devices to the combo box
    for (const auto& device : devices) {
        m_deviceComboBox->addItem(QString::fromStdString(device));
    }
    
    m_deviceComboBox->setEnabled(true);
    
    // Try to select previously selected device
    if (m_selectedDeviceIndex >= 0 && m_selectedDeviceIndex < m_deviceComboBox->count()) {
        m_deviceComboBox->setCurrentIndex(m_selectedDeviceIndex);
    } else {
        // Default to first device
        m_deviceComboBox->setCurrentIndex(0);
        m_selectedDeviceIndex = 0;
    }
}

} // namespace presentation
} // namespace MusicTrainer