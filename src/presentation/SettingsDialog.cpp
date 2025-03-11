#include "presentation/SettingsDialog.h"
#include "domain/ports/MidiAdapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QApplication>

namespace MusicTrainer {
namespace presentation {

SettingsDialog::SettingsDialog(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget* parent)
    : QDialog(parent)
    , m_midiAdapter(std::move(midiAdapter))
{
    setupUi();
    setupConnections();
    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUi()
{
    setWindowTitle(tr("Settings"));
    resize(600, 500);
    
    auto* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    // MIDI Tab
    auto* midiTab = new QWidget(this);
    auto* midiLayout = new QVBoxLayout(midiTab);
    
    auto* midiDevicesGroup = new QGroupBox(tr("MIDI Devices"), this);
    auto* midiDevicesLayout = new QVBoxLayout(midiDevicesGroup);
    
    auto* inputLayout = new QHBoxLayout();
    auto* inputLabel = new QLabel(tr("Input Device:"), this);
    m_midiInputDeviceComboBox = new QComboBox(this);
    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(m_midiInputDeviceComboBox, 1);
    
    auto* outputLayout = new QHBoxLayout();
    auto* outputLabel = new QLabel(tr("Output Device:"), this);
    m_midiOutputDeviceComboBox = new QComboBox(this);
    outputLayout->addWidget(outputLabel);
    outputLayout->addWidget(m_midiOutputDeviceComboBox, 1);
    
    m_refreshMidiDevicesButton = new QPushButton(tr("Refresh Devices"), this);
    
    midiDevicesLayout->addLayout(inputLayout);
    midiDevicesLayout->addLayout(outputLayout);
    midiDevicesLayout->addWidget(m_refreshMidiDevicesButton);
    
    auto* midiOptionsGroup = new QGroupBox(tr("MIDI Options"), this);
    auto* midiOptionsLayout = new QVBoxLayout(midiOptionsGroup);
    
    m_enableMidiThroughCheckBox = new QCheckBox(tr("Enable MIDI Through"), this);
    
    auto* latencyLayout = new QHBoxLayout();
    auto* latencyLabel = new QLabel(tr("Input Latency (ms):"), this);
    m_midiLatencySpinBox = new QSpinBox(this);
    m_midiLatencySpinBox->setRange(0, 100);
    m_midiLatencySpinBox->setValue(10);
    latencyLayout->addWidget(latencyLabel);
    latencyLayout->addWidget(m_midiLatencySpinBox);
    latencyLayout->addStretch();
    
    midiOptionsLayout->addWidget(m_enableMidiThroughCheckBox);
    midiOptionsLayout->addLayout(latencyLayout);
    
    midiLayout->addWidget(midiDevicesGroup);
    midiLayout->addWidget(midiOptionsGroup);
    midiLayout->addStretch();
    
    // Audio Tab
    auto* audioTab = new QWidget(this);
    auto* audioLayout = new QVBoxLayout(audioTab);
    
    auto* metronomeGroup = new QGroupBox(tr("Metronome"), this);
    auto* metronomeLayout = new QVBoxLayout(metronomeGroup);
    
    m_enableMetronomeCheckBox = new QCheckBox(tr("Enable Metronome"), this);
    
    auto* metronomeVolumeLayout = new QHBoxLayout();
    auto* metronomeVolumeLabel = new QLabel(tr("Volume:"), this);
    m_metronomeVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_metronomeVolumeSlider->setRange(0, 100);
    m_metronomeVolumeSlider->setValue(80);
    metronomeVolumeLayout->addWidget(metronomeVolumeLabel);
    metronomeVolumeLayout->addWidget(m_metronomeVolumeSlider);
    
    metronomeLayout->addWidget(m_enableMetronomeCheckBox);
    metronomeLayout->addLayout(metronomeVolumeLayout);
    
    auto* soundEffectsGroup = new QGroupBox(tr("Sound Effects"), this);
    auto* soundEffectsLayout = new QVBoxLayout(soundEffectsGroup);
    
    m_enableSoundEffectsCheckBox = new QCheckBox(tr("Enable Sound Effects"), this);
    
    auto* soundEffectsVolumeLayout = new QHBoxLayout();
    auto* soundEffectsVolumeLabel = new QLabel(tr("Volume:"), this);
    m_soundEffectsVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_soundEffectsVolumeSlider->setRange(0, 100);
    m_soundEffectsVolumeSlider->setValue(80);
    soundEffectsVolumeLayout->addWidget(soundEffectsVolumeLabel);
    soundEffectsVolumeLayout->addWidget(m_soundEffectsVolumeSlider);
    
    soundEffectsLayout->addWidget(m_enableSoundEffectsCheckBox);
    soundEffectsLayout->addLayout(soundEffectsVolumeLayout);
    
    audioLayout->addWidget(metronomeGroup);
    audioLayout->addWidget(soundEffectsGroup);
    audioLayout->addStretch();
    
    // UI Tab
    auto* uiTab = new QWidget(this);
    auto* uiLayout = new QVBoxLayout(uiTab);
    
    auto* themeLayout = new QHBoxLayout();
    auto* themeLabel = new QLabel(tr("Theme:"), this);
    m_themeComboBox = new QComboBox(this);
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(m_themeComboBox, 1);
    
    auto* displayGroup = new QGroupBox(tr("Display Options"), this);
    auto* displayLayout = new QVBoxLayout(displayGroup);
    
    m_showMeasureNumbersCheckBox = new QCheckBox(tr("Show Measure Numbers"), this);
    m_showKeySignatureCheckBox = new QCheckBox(tr("Show Key Signature"), this);
    m_showVoiceLabelsCheckBox = new QCheckBox(tr("Show Voice Labels"), this);
    
    auto* fontSizeLayout = new QHBoxLayout();
    auto* fontSizeLabel = new QLabel(tr("Font Size:"), this);
    m_fontSizeSpinBox = new QSpinBox(this);
    m_fontSizeSpinBox->setRange(8, 24);
    m_fontSizeSpinBox->setValue(12);
    fontSizeLayout->addWidget(fontSizeLabel);
    fontSizeLayout->addWidget(m_fontSizeSpinBox);
    fontSizeLayout->addStretch();
    
    displayLayout->addWidget(m_showMeasureNumbersCheckBox);
    displayLayout->addWidget(m_showKeySignatureCheckBox);
    displayLayout->addWidget(m_showVoiceLabelsCheckBox);
    displayLayout->addLayout(fontSizeLayout);
    
    uiLayout->addLayout(themeLayout);
    uiLayout->addWidget(displayGroup);
    uiLayout->addStretch();
    
    // Rules Tab
    auto* rulesTab = new QWidget(this);
    auto* rulesContainerLayout = new QHBoxLayout(rulesTab);
    
    auto* ruleSetGroup = new QGroupBox(tr("Rule Sets"), this);
    auto* ruleSetLayout = new QVBoxLayout(ruleSetGroup);
    m_ruleSetListWidget = new QListWidget(this);
    ruleSetLayout->addWidget(m_ruleSetListWidget);
    
    auto* rulesGroup = new QGroupBox(tr("Rules"), this);
    auto* rulesListLayout = new QVBoxLayout(rulesGroup);
    m_rulesListWidget = new QListWidget(this);
    rulesListLayout->addWidget(m_rulesListWidget);
    
    rulesContainerLayout->addWidget(ruleSetGroup);
    rulesContainerLayout->addWidget(rulesGroup);
    
    // Add tabs to tab widget
    m_tabWidget->addTab(midiTab, tr("MIDI"));
    m_tabWidget->addTab(audioTab, tr("Audio"));
    m_tabWidget->addTab(uiTab, tr("UI"));
    m_tabWidget->addTab(rulesTab, tr("Rules"));
    
    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    
    m_restoreDefaultsButton = new QPushButton(tr("Restore Defaults"), this);
    m_saveButton = new QPushButton(tr("Save"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    buttonLayout->addWidget(m_restoreDefaultsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // Add widgets to main layout
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    
    // Populate data
    populateMidiDevices();
    populateThemes();
    populateRuleSets();
}

void SettingsDialog::setupConnections()
{
    // MIDI connections
    connect(m_midiInputDeviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onMidiInputDeviceChanged);
            
    connect(m_midiOutputDeviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onMidiOutputDeviceChanged);
            
    connect(m_refreshMidiDevicesButton, &QPushButton::clicked,
            this, &SettingsDialog::onRefreshMidiDevicesClicked);
            
    // UI connections
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
            
    // Rules connections
    connect(m_ruleSetListWidget, &QListWidget::currentRowChanged,
            this, &SettingsDialog::onRuleSetSelectionChanged);
            
    // Connect rule items to toggle slot
    connect(m_rulesListWidget, &QListWidget::itemChanged,
            this, [this](QListWidgetItem* item) {
                if (item) {
                    onRuleToggled(item->checkState() == Qt::Checked);
                }
            });
            
    // Button connections
    connect(m_saveButton, &QPushButton::clicked,
            this, &SettingsDialog::onSaveClicked);
            
    connect(m_cancelButton, &QPushButton::clicked,
            this, &SettingsDialog::onCancelClicked);
            
    connect(m_restoreDefaultsButton, &QPushButton::clicked,
            this, &SettingsDialog::onRestoreDefaultsClicked);
}

void SettingsDialog::populateMidiDevices()
{
    // Clear existing items
    m_midiInputDeviceComboBox->clear();
    m_midiOutputDeviceComboBox->clear();
    
    if (!m_midiAdapter) {
        m_midiInputDeviceComboBox->addItem(tr("No MIDI Interface Available"));
        m_midiOutputDeviceComboBox->addItem(tr("No MIDI Interface Available"));
        m_midiInputDeviceComboBox->setEnabled(false);
        m_midiOutputDeviceComboBox->setEnabled(false);
        return;
    }
    
    // Get available MIDI input devices
    const auto inputs = m_midiAdapter->getAvailableInputs();
    if (inputs.empty()) {
        m_midiInputDeviceComboBox->addItem(tr("No MIDI Input Devices Available"));
        m_midiInputDeviceComboBox->setEnabled(false);
    } else {
        for (const auto& input : inputs) {
            m_midiInputDeviceComboBox->addItem(QString::fromStdString(input));
        }
        m_midiInputDeviceComboBox->setEnabled(true);
    }
    
    // Get available MIDI output devices
    const auto outputs = m_midiAdapter->getAvailableOutputs();
    if (outputs.empty()) {
        m_midiOutputDeviceComboBox->addItem(tr("No MIDI Output Devices Available"));
        m_midiOutputDeviceComboBox->setEnabled(false);
    } else {
        for (const auto& output : outputs) {
            m_midiOutputDeviceComboBox->addItem(QString::fromStdString(output));
        }
        m_midiOutputDeviceComboBox->setEnabled(true);
    }
}

void SettingsDialog::populateThemes()
{
    m_themeComboBox->clear();
    m_themeComboBox->addItem(tr("System Default"));
    m_themeComboBox->addItem(tr("Light"));
    m_themeComboBox->addItem(tr("Dark"));
    m_themeComboBox->addItem(tr("High Contrast"));
}

void SettingsDialog::populateRuleSets()
{
    // Clear existing items
    m_ruleSetListWidget->clear();
    m_rulesListWidget->clear();
    
    // Add placeholder rule sets
    m_ruleSetListWidget->addItem(tr("First Species Rules"));
    m_ruleSetListWidget->addItem(tr("Second Species Rules"));
    m_ruleSetListWidget->addItem(tr("Third Species Rules"));
    m_ruleSetListWidget->addItem(tr("Fourth Species Rules"));
    m_ruleSetListWidget->addItem(tr("Fifth Species Rules"));
    m_ruleSetListWidget->addItem(tr("Free Counterpoint Rules"));
    
    // Select first rule set by default
    if (m_ruleSetListWidget->count() > 0) {
        m_ruleSetListWidget->setCurrentRow(0);
        onRuleSetSelectionChanged();
    }
}

void SettingsDialog::onMidiInputDeviceChanged(int index)
{
    if (!m_midiAdapter || index < 0 || m_midiInputDeviceComboBox->count() <= 0) {
        return;
    }
    
    QString deviceName = m_midiInputDeviceComboBox->itemText(index);
    qDebug() << "Selected MIDI input device:" << deviceName;
    
    // Close the current connection
    if (m_midiAdapter->isOpen()) {
        m_midiAdapter->close();
    }
    
    // TODO: In a real implementation, we would need to store the selected device index
    // and reopen the MIDI adapter with the new device. For now, we'll just log it.
    
    // Reopen the connection
    if (!m_midiAdapter->open()) {
        QMessageBox::warning(this, tr("MIDI Connection Error"),
            tr("Failed to open MIDI connection to %1. Please check your MIDI setup.").arg(deviceName));
    }
}

void SettingsDialog::onMidiOutputDeviceChanged(int index)
{
    if (!m_midiAdapter || index < 0 || m_midiOutputDeviceComboBox->count() <= 0) {
        return;
    }
    
    QString deviceName = m_midiOutputDeviceComboBox->itemText(index);
    qDebug() << "Selected MIDI output device:" << deviceName;
    
    // Close the current connection
    if (m_midiAdapter->isOpen()) {
        m_midiAdapter->close();
    }
    
    // TODO: In a real implementation, we would need to store the selected device index
    // and reopen the MIDI adapter with the new device. For now, we'll just log it.
    
    // Reopen the connection
    if (!m_midiAdapter->open()) {
        QMessageBox::warning(this, tr("MIDI Connection Error"),
            tr("Failed to open MIDI connection to %1. Please check your MIDI setup.").arg(deviceName));
    }
}

void SettingsDialog::onRefreshMidiDevicesClicked()
{
    populateMidiDevices();
}

void SettingsDialog::onThemeChanged(int index)
{
    QString themeName = m_themeComboBox->itemText(index);
    qDebug() << "Selected theme:" << themeName;
    
    // Apply the theme immediately
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("Theme", index);
    settings.endGroup();
    
    // Apply the theme to the application
    if (themeName == tr("Light")) {
        qApp->setStyle("fusion");
        QPalette lightPalette;
        qApp->setPalette(lightPalette);
    } else if (themeName == tr("Dark")) {
        qApp->setStyle("fusion");
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);
    } else if (themeName == tr("High Contrast")) {
        qApp->setStyle("fusion");
        QPalette contrastPalette;
        contrastPalette.setColor(QPalette::Window, Qt::black);
        contrastPalette.setColor(QPalette::WindowText, Qt::white);
        contrastPalette.setColor(QPalette::Base, Qt::black);
        contrastPalette.setColor(QPalette::AlternateBase, Qt::black);
        contrastPalette.setColor(QPalette::ToolTipBase, Qt::white);
        contrastPalette.setColor(QPalette::ToolTipText, Qt::white);
        contrastPalette.setColor(QPalette::Text, Qt::white);
        contrastPalette.setColor(QPalette::Button, Qt::black);
        contrastPalette.setColor(QPalette::ButtonText, Qt::white);
        contrastPalette.setColor(QPalette::BrightText, Qt::white);
        contrastPalette.setColor(QPalette::Link, QColor(255, 255, 0));
        contrastPalette.setColor(QPalette::Highlight, QColor(255, 255, 0));
        contrastPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(contrastPalette);
    } else {
        // System Default
        qApp->setStyle("");
        qApp->setPalette(qApp->style()->standardPalette());
    }
    
    // Emit a signal to notify that the theme has changed
    Q_EMIT themeChanged(index);
}

void SettingsDialog::onRuleSetSelectionChanged()
{
    m_rulesListWidget->clear();
    
    int currentRow = m_ruleSetListWidget->currentRow();
    if (currentRow < 0) {
        return;
    }
    
    QString ruleSetName = m_ruleSetListWidget->item(currentRow)->text();
    
    // Populate rules based on selected rule set
    if (ruleSetName == tr("First Species Rules")) {
        // Create rule items with default checked state
        QSettings settings;
        settings.beginGroup("Rules");
        
        auto* item1 = new QListWidgetItem(tr("Consonant Intervals Only"), m_rulesListWidget);
        item1->setFlags(item1->flags() | Qt::ItemIsUserCheckable);
        item1->setCheckState(settings.value(item1->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        auto* item2 = new QListWidgetItem(tr("No Parallel Fifths"), m_rulesListWidget);
        item2->setFlags(item2->flags() | Qt::ItemIsUserCheckable);
        item2->setCheckState(settings.value(item2->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        auto* item3 = new QListWidgetItem(tr("No Parallel Octaves"), m_rulesListWidget);
        item3->setFlags(item3->flags() | Qt::ItemIsUserCheckable);
        item3->setCheckState(settings.value(item3->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        auto* item4 = new QListWidgetItem(tr("Approach Perfect Consonances by Contrary Motion"), m_rulesListWidget);
        item4->setFlags(item4->flags() | Qt::ItemIsUserCheckable);
        item4->setCheckState(settings.value(item4->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        settings.endGroup();
    } else if (ruleSetName == tr("Second Species Rules")) {
        QSettings settings;
        settings.beginGroup("Rules");
        
        auto* item1 = new QListWidgetItem(tr("Strong Beats Must Be Consonant"), m_rulesListWidget);
        item1->setFlags(item1->flags() | Qt::ItemIsUserCheckable);
        item1->setCheckState(settings.value(item1->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        auto* item2 = new QListWidgetItem(tr("Weak Beats May Be Dissonant If Passing"), m_rulesListWidget);
        item2->setFlags(item2->flags() | Qt::ItemIsUserCheckable);
        item2->setCheckState(settings.value(item2->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        auto* item3 = new QListWidgetItem(tr("No Parallel Fifths on Strong Beats"), m_rulesListWidget);
        item3->setFlags(item3->flags() | Qt::ItemIsUserCheckable);
        item3->setCheckState(settings.value(item3->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        settings.endGroup();
    } else {
        // Add placeholder rules for other rule sets
        QSettings settings;
        settings.beginGroup("Rules");
        
        auto* item = new QListWidgetItem(tr("Placeholder Rule"), m_rulesListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(settings.value(item->text(), true).toBool() ? Qt::Checked : Qt::Unchecked);
        
        settings.endGroup();
    }
}

void SettingsDialog::onRuleToggled(bool checked)
{
    // Get the current item since we can't use qobject_cast with QListWidgetItem
    auto* item = m_rulesListWidget->currentItem();
    if (!item) {
        return;
    }
    
    QString ruleName = item->text();
    qDebug() << "Rule toggled:" << ruleName << "Enabled:" << checked;
    
    // Store the rule state in settings
    QSettings settings;
    settings.beginGroup("Rules");
    settings.setValue(ruleName, checked);
    settings.endGroup();
    
    // Update the UI to reflect the change
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    
    // Emit a signal to notify that rules have changed
    Q_EMIT rulesChanged();
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    // MIDI settings
    settings.beginGroup("MIDI");
    settings.setValue("InputDevice", m_midiInputDeviceComboBox->currentText());
    settings.setValue("OutputDevice", m_midiOutputDeviceComboBox->currentText());
    settings.setValue("EnableThrough", m_enableMidiThroughCheckBox->isChecked());
    settings.setValue("Latency", m_midiLatencySpinBox->value());
    settings.endGroup();
    
    // Audio settings
    settings.beginGroup("Audio");
    settings.setValue("EnableMetronome", m_enableMetronomeCheckBox->isChecked());
    settings.setValue("MetronomeVolume", m_metronomeVolumeSlider->value());
    settings.setValue("EnableSoundEffects", m_enableSoundEffectsCheckBox->isChecked());
    settings.setValue("SoundEffectsVolume", m_soundEffectsVolumeSlider->value());
    settings.endGroup();
    
    // UI settings
    settings.beginGroup("UI");
    settings.setValue("Theme", m_themeComboBox->currentIndex());
    settings.setValue("ShowMeasureNumbers", m_showMeasureNumbersCheckBox->isChecked());
    settings.setValue("ShowKeySignature", m_showKeySignatureCheckBox->isChecked());
    settings.setValue("ShowVoiceLabels", m_showVoiceLabelsCheckBox->isChecked());
    settings.setValue("FontSize", m_fontSizeSpinBox->value());
    settings.endGroup();
    
    // Rule settings
    settings.beginGroup("Rules");
    for (int i = 0; i < m_rulesListWidget->count(); ++i) {
        QListWidgetItem* item = m_rulesListWidget->item(i);
        if (item) {
            QString ruleName = item->text();
            bool enabled = item->checkState() == Qt::Checked;
            settings.setValue(ruleName, enabled);
        }
    }
    settings.endGroup();
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    // MIDI settings
    settings.beginGroup("MIDI");
    QString inputDevice = settings.value("InputDevice").toString();
    QString outputDevice = settings.value("OutputDevice").toString();
    bool enableThrough = settings.value("EnableThrough", false).toBool();
    int latency = settings.value("Latency", 10).toInt();
    settings.endGroup();
    
    // Find and select the saved devices in the combo boxes
    int inputIndex = m_midiInputDeviceComboBox->findText(inputDevice);
    if (inputIndex >= 0) {
        m_midiInputDeviceComboBox->setCurrentIndex(inputIndex);
    }
    
    int outputIndex = m_midiOutputDeviceComboBox->findText(outputDevice);
    if (outputIndex >= 0) {
        m_midiOutputDeviceComboBox->setCurrentIndex(outputIndex);
    }
    
    m_enableMidiThroughCheckBox->setChecked(enableThrough);
    m_midiLatencySpinBox->setValue(latency);
    
    // Audio settings
    settings.beginGroup("Audio");
    bool enableMetronome = settings.value("EnableMetronome", true).toBool();
    int metronomeVolume = settings.value("MetronomeVolume", 80).toInt();
    bool enableSoundEffects = settings.value("EnableSoundEffects", true).toBool();
    int soundEffectsVolume = settings.value("SoundEffectsVolume", 80).toInt();
    settings.endGroup();
    
    m_enableMetronomeCheckBox->setChecked(enableMetronome);
    m_metronomeVolumeSlider->setValue(metronomeVolume);
    m_enableSoundEffectsCheckBox->setChecked(enableSoundEffects);
    m_soundEffectsVolumeSlider->setValue(soundEffectsVolume);
    
    // UI settings
    settings.beginGroup("UI");
    int theme = settings.value("Theme", 0).toInt();
    bool showMeasureNumbers = settings.value("ShowMeasureNumbers", true).toBool();
    bool showKeySignature = settings.value("ShowKeySignature", true).toBool();
    bool showVoiceLabels = settings.value("ShowVoiceLabels", true).toBool();
    int fontSize = settings.value("FontSize", 12).toInt();
    settings.endGroup();
    
    m_themeComboBox->setCurrentIndex(theme);
    m_showMeasureNumbersCheckBox->setChecked(showMeasureNumbers);
    m_showKeySignatureCheckBox->setChecked(showKeySignature);
    m_showVoiceLabelsCheckBox->setChecked(showVoiceLabels);
    m_fontSizeSpinBox->setValue(fontSize);
    
    // Rule settings
    settings.beginGroup("Rules");
    
    // Apply saved rule states to the current rule set
    for (int i = 0; i < m_rulesListWidget->count(); ++i) {
        QListWidgetItem* item = m_rulesListWidget->item(i);
        if (item) {
            QString ruleName = item->text();
            bool enabled = settings.value(ruleName, true).toBool(); // Default to enabled
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
        }
    }
    settings.endGroup();
}

void SettingsDialog::onSaveClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}

void SettingsDialog::onRestoreDefaultsClicked()
{
    // Confirm with user
    auto result = QMessageBox::question(this, tr("Restore Defaults"),
                                       tr("Are you sure you want to restore all settings to their default values?"),
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (result != QMessageBox::Yes) {
        return;
    }
    
    // Reset MIDI settings
    m_enableMidiThroughCheckBox->setChecked(false);
    m_midiLatencySpinBox->setValue(10);
    
    // Reset audio settings
    m_enableMetronomeCheckBox->setChecked(true);
    m_metronomeVolumeSlider->setValue(80);
    m_enableSoundEffectsCheckBox->setChecked(true);
    m_soundEffectsVolumeSlider->setValue(80);
    
    // Reset UI settings
    m_themeComboBox->setCurrentIndex(0);
    m_showMeasureNumbersCheckBox->setChecked(true);
    m_showKeySignatureCheckBox->setChecked(true);
    m_showVoiceLabelsCheckBox->setChecked(true);
    m_fontSizeSpinBox->setValue(12);
    
    // Reset rule settings
    populateRuleSets();
}

} // namespace presentation
} // namespace MusicTrainer