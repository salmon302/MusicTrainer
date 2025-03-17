#include "domain/state/SettingsState.h"
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <QDir>

namespace MusicTrainer::state {

SettingsState& SettingsState::instance() {
    static SettingsState instance;
    return instance;
}

SettingsState::SettingsState() : QObject(nullptr) {
    loadSettings();
}

void SettingsState::loadSettings() {
    QSettings settings;

    // Called on construction - no need to emit signals
    // as nothing is connected yet
}

void SettingsState::saveSettings() {
    QSettings settings;
    
    // MIDI settings
    settings.beginGroup("MIDI");
    settings.setValue("InputDevice", getMidiInputDevice());
    settings.setValue("OutputDevice", getMidiOutputDevice());
    settings.setValue("EnableThrough", getMidiThrough());
    settings.setValue("Latency", getMidiLatency());
    settings.endGroup();
    
    // Audio settings
    settings.beginGroup("Audio");
    settings.setValue("EnableMetronome", getMetronomeEnabled());
    settings.setValue("MetronomeVolume", getMetronomeVolume());
    settings.setValue("EnableSoundEffects", getSoundEffectsEnabled());
    settings.setValue("SoundEffectsVolume", getSoundEffectsVolume());
    settings.endGroup();
    
    // UI settings
    settings.beginGroup("UI");
    settings.setValue("Theme", getTheme());
    settings.setValue("ShowMeasureNumbers", getShowMeasureNumbers());
    settings.setValue("ShowKeySignature", getShowKeySignature());
    settings.setValue("ShowVoiceLabels", getShowVoiceLabels());
    settings.setValue("FontSize", getFontSize());
    settings.endGroup();
    
    // Viewport settings
    settings.beginGroup("Viewport");
    settings.setValue("Zoom", getViewportZoom());
    settings.setValue("PreserveOctaveExpansion", getViewportPreserveOctaveExpansion());
    settings.setValue("PitchBase", getViewportPitchBase());
    QPointF scrollPos = getViewportScrollPosition();
    settings.setValue("ScrollX", scrollPos.x());
    settings.setValue("ScrollY", scrollPos.y());
    settings.endGroup();
}

// MIDI Settings
QString SettingsState::getMidiInputDevice() const {
    QSettings settings;
    settings.beginGroup("MIDI");
    QString device = settings.value("InputDevice").toString();
    settings.endGroup();
    return device;
}

QString SettingsState::getMidiOutputDevice() const {
    QSettings settings;
    settings.beginGroup("MIDI");
    QString device = settings.value("OutputDevice").toString();
    settings.endGroup();
    return device;
}

bool SettingsState::getMidiThrough() const {
    QSettings settings;
    settings.beginGroup("MIDI");
    bool enabled = settings.value("EnableThrough", false).toBool();
    settings.endGroup();
    return enabled;
}

int SettingsState::getMidiLatency() const {
    QSettings settings;
    settings.beginGroup("MIDI");
    int latency = settings.value("Latency", 10).toInt();
    settings.endGroup();
    return latency;
}

// Audio Settings
bool SettingsState::getMetronomeEnabled() const {
    QSettings settings;
    settings.beginGroup("Audio");
    bool enabled = settings.value("EnableMetronome", true).toBool();
    settings.endGroup();
    return enabled;
}

int SettingsState::getMetronomeVolume() const {
    QSettings settings;
    settings.beginGroup("Audio");
    int volume = settings.value("MetronomeVolume", 80).toInt();
    settings.endGroup();
    return volume;
}

bool SettingsState::getSoundEffectsEnabled() const {
    QSettings settings;
    settings.beginGroup("Audio");
    bool enabled = settings.value("EnableSoundEffects", true).toBool();
    settings.endGroup();
    return enabled;
}

int SettingsState::getSoundEffectsVolume() const {
    QSettings settings;
    settings.beginGroup("Audio");
    int volume = settings.value("SoundEffectsVolume", 80).toInt();
    settings.endGroup();
    return volume;
}

// UI Settings
int SettingsState::getTheme() const {
    QSettings settings;
    settings.beginGroup("UI");
    int theme = settings.value("Theme", 0).toInt();
    settings.endGroup();
    return theme;
}

bool SettingsState::getShowMeasureNumbers() const {
    QSettings settings;
    settings.beginGroup("UI");
    bool show = settings.value("ShowMeasureNumbers", true).toBool();
    settings.endGroup();
    return show;
}

bool SettingsState::getShowKeySignature() const {
    QSettings settings;
    settings.beginGroup("UI");
    bool show = settings.value("ShowKeySignature", true).toBool();
    settings.endGroup();
    return show;
}

bool SettingsState::getShowVoiceLabels() const {
    QSettings settings;
    settings.beginGroup("UI");
    bool show = settings.value("ShowVoiceLabels", true).toBool();
    settings.endGroup();
    return show;
}

int SettingsState::getFontSize() const {
    QSettings settings;
    settings.beginGroup("UI");
    int size = settings.value("FontSize", 12).toInt();
    settings.endGroup();
    return size;
}

// Rule Settings
bool SettingsState::getRuleEnabled(const QString& ruleName) const {
    QSettings settings;
    settings.beginGroup("Rules");
    bool enabled = settings.value(ruleName, true).toBool();
    settings.endGroup();
    return enabled;
}

// Viewport Settings 
float SettingsState::getViewportZoom() const {
    QSettings settings;
    settings.beginGroup("Viewport");
    float zoom = settings.value("Zoom", 1.0f).toFloat();
    settings.endGroup();
    return zoom;
}

bool SettingsState::getViewportPreserveOctaveExpansion() const {
    QSettings settings;
    settings.beginGroup("Viewport");
    bool preserve = settings.value("PreserveOctaveExpansion", false).toBool();
    settings.endGroup();
    return preserve;
}

QPointF SettingsState::getViewportScrollPosition() const {
    QSettings settings;
    settings.beginGroup("Viewport");
    QPointF pos(
        settings.value("ScrollX", 0.0).toDouble(),
        settings.value("ScrollY", 0.0).toDouble()
    );
    settings.endGroup();
    return pos;
}

int SettingsState::getViewportPitchBase() const {
    QSettings settings;
    settings.beginGroup("Viewport");
    int pitch = settings.value("PitchBase", 60).toInt(); // Default to C4
    settings.endGroup();
    return pitch;
}

// Setters
void SettingsState::setMidiInputDevice(const QString& device) {
    QSettings settings;
    settings.beginGroup("MIDI");
    settings.setValue("InputDevice", device);
    settings.endGroup();
    Q_EMIT midiSettingsChanged();
}

void SettingsState::setMidiOutputDevice(const QString& device) {
    QSettings settings;
    settings.beginGroup("MIDI");
    settings.setValue("OutputDevice", device);
    settings.endGroup();
    Q_EMIT midiSettingsChanged();
}

void SettingsState::setMidiThrough(bool enabled) {
    QSettings settings;
    settings.beginGroup("MIDI");
    settings.setValue("EnableThrough", enabled);
    settings.endGroup();
    Q_EMIT midiSettingsChanged();
}

void SettingsState::setMidiLatency(int latency) {
    QSettings settings;
    settings.beginGroup("MIDI");
    settings.setValue("Latency", latency);
    settings.endGroup();
    Q_EMIT midiSettingsChanged();
}

void SettingsState::setMetronomeEnabled(bool enabled) {
    QSettings settings;
    settings.beginGroup("Audio");
    settings.setValue("EnableMetronome", enabled);
    settings.endGroup();
    Q_EMIT audioSettingsChanged();
}

void SettingsState::setMetronomeVolume(int volume) {
    QSettings settings;
    settings.beginGroup("Audio");
    settings.setValue("MetronomeVolume", volume);
    settings.endGroup();
    Q_EMIT audioSettingsChanged();
}

void SettingsState::setSoundEffectsEnabled(bool enabled) {
    QSettings settings;
    settings.beginGroup("Audio");
    settings.setValue("EnableSoundEffects", enabled);
    settings.endGroup();
    Q_EMIT audioSettingsChanged();
}

void SettingsState::setSoundEffectsVolume(int volume) {
    QSettings settings;
    settings.beginGroup("Audio");
    settings.setValue("SoundEffectsVolume", volume);
    settings.endGroup();
    Q_EMIT audioSettingsChanged();
}

void SettingsState::setTheme(int theme) {
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("Theme", theme);
    settings.endGroup();
    Q_EMIT uiSettingsChanged();
}

void SettingsState::setShowMeasureNumbers(bool show) {
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("ShowMeasureNumbers", show);
    settings.endGroup();
    Q_EMIT uiSettingsChanged();
}

void SettingsState::setShowKeySignature(bool show) {
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("ShowKeySignature", show);
    settings.endGroup();
    Q_EMIT uiSettingsChanged();
}

void SettingsState::setShowVoiceLabels(bool show) {
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("ShowVoiceLabels", show);
    settings.endGroup();
    Q_EMIT uiSettingsChanged();
}

void SettingsState::setFontSize(int size) {
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("FontSize", size);
    settings.endGroup();
    Q_EMIT uiSettingsChanged();
}

void SettingsState::setRuleEnabled(const QString& ruleName, bool enabled) {
    QSettings settings;
    settings.beginGroup("Rules");
    settings.setValue(ruleName, enabled);
    settings.endGroup();
    Q_EMIT ruleSettingsChanged();
}

void SettingsState::setViewportZoom(float zoom) {
    QSettings settings;
    settings.beginGroup("Viewport");
    settings.setValue("Zoom", zoom);
    settings.endGroup();
    Q_EMIT viewportSettingsChanged();
}

void SettingsState::setViewportPreserveOctaveExpansion(bool preserve) {
    QSettings settings;
    settings.beginGroup("Viewport");
    settings.setValue("PreserveOctaveExpansion", preserve);
    settings.endGroup();
    Q_EMIT viewportSettingsChanged();
}

void SettingsState::setViewportScrollPosition(const QPointF& pos) {
    QSettings settings;
    settings.beginGroup("Viewport");
    settings.setValue("ScrollX", pos.x());
    settings.setValue("ScrollY", pos.y());
    settings.endGroup();
    Q_EMIT viewportSettingsChanged();
}

void SettingsState::setViewportPitchBase(int pitch)
{
    QSettings settings;
    settings.beginGroup("Viewport");
    settings.setValue("PitchBase", qBound(0, pitch, 127));
    settings.endGroup();
    Q_EMIT viewportSettingsChanged();
}

void SettingsState::restoreDefaults() {
    QSettings settings;
    settings.clear();
    
    // Re-emit all signals since everything changed
    Q_EMIT midiSettingsChanged();
    Q_EMIT audioSettingsChanged();
    Q_EMIT uiSettingsChanged();
    Q_EMIT ruleSettingsChanged();
    Q_EMIT viewportSettingsChanged();
}

// State validation and management
bool SettingsState::validateSettings() const
{
    QSettings settings;

    // Check basic settings structure
    if (!settings.contains("MIDI") || 
        !settings.contains("Audio") || 
        !settings.contains("UI") || 
        !settings.contains("Rules") || 
        !settings.contains("Viewport")) {
        return false;
    }

    // Validate viewport settings ranges
    settings.beginGroup("Viewport");
    float zoom = settings.value("Zoom", 1.0f).toFloat();
    QPointF scrollPos = QPointF(
        settings.value("ScrollX", 0.0).toDouble(),
        settings.value("ScrollY", 0.0).toDouble()
    );
    int pitchBase = settings.value("PitchBase", 60).toInt(); // C4 default
    settings.endGroup();

    if (zoom < 0.1f || zoom > 10.0f ||
        scrollPos.x() < 0.0 || scrollPos.x() > 1000.0 ||
        scrollPos.y() < 0.0 || scrollPos.y() > 1000.0 ||
        pitchBase < 0 || pitchBase > 127) {
        return false;
    }

    // Validate UI settings ranges
    settings.beginGroup("UI");
    int fontSize = settings.value("FontSize", 12).toInt();
    int theme = settings.value("Theme", 0).toInt();
    settings.endGroup();

    if (fontSize < 8 || fontSize > 24 ||
        theme < 0 || theme > 2) {
        return false;
    }

    // Validate audio settings ranges
    settings.beginGroup("Audio");
    int metronomeVolume = settings.value("MetronomeVolume", 80).toInt();
    int sfxVolume = settings.value("SoundEffectsVolume", 80).toInt();
    settings.endGroup();

    if (metronomeVolume < 0 || metronomeVolume > 100 ||
        sfxVolume < 0 || sfxVolume > 100) {
        return false;
    }

    return true;
}

void SettingsState::saveState()
{
    QSettings settings;
    settings.sync();

    // Create backup of current settings
    QString backupPath = QDir::tempPath() + "/music_trainer_settings_backup.ini";
    backupState(backupPath);

    try {
        saveSettings();
        settings.sync();

        // Validate after save
        if (!validateSettings()) {
            qWarning() << "Settings validation failed after save, restoring from backup";
            restoreFromBackup(backupPath);
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to save settings:" << e.what();
        restoreFromBackup(backupPath);
    }
}

bool SettingsState::loadState()
{
    try {
        // Create backup before loading
        QString backupPath = QDir::tempPath() + "/music_trainer_settings_backup.ini";
        backupState(backupPath);

        loadSettings();

        // Validate after load
        if (!validateSettings()) {
            qWarning() << "Settings validation failed after load, restoring from backup";
            restoreFromBackup(backupPath);
            return false;
        }

        emitAllChanges();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Failed to load settings:" << e.what();
        restoreDefaults();
        return false;
    }
}

void SettingsState::backupState(const QString& backupPath)
{
    QSettings currentSettings;
    QSettings backup(backupPath, QSettings::IniFormat);

    // Copy all settings to backup
    QStringList groups = {"MIDI", "Audio", "UI", "Rules", "Viewport"};
    for (const QString& group : groups) {
        currentSettings.beginGroup(group);
        backup.beginGroup(group);
        
        for (const QString& key : currentSettings.childKeys()) {
            backup.setValue(key, currentSettings.value(key));
        }
        
        backup.endGroup();
        currentSettings.endGroup();
    }
    
    backup.sync();
}

bool SettingsState::restoreFromBackup(const QString& backupPath)
{
    if (!QFile::exists(backupPath)) {
        return false;
    }

    try {
        QSettings backup(backupPath, QSettings::IniFormat);
        QSettings settings;

        // Clear current settings
        settings.clear();

        // Copy all settings from backup
        QStringList groups = {"MIDI", "Audio", "UI", "Rules", "Viewport"};
        for (const QString& group : groups) {
            backup.beginGroup(group);
            settings.beginGroup(group);
            
            for (const QString& key : backup.childKeys()) {
                settings.setValue(key, backup.value(key));
            }
            
            settings.endGroup();
            backup.endGroup();
        }
        
        settings.sync();
        emitAllChanges();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Failed to restore from backup:" << e.what();
        return false;
    }
}

void SettingsState::emitAllChanges()
{
    Q_EMIT midiSettingsChanged();
    Q_EMIT audioSettingsChanged();
    Q_EMIT uiSettingsChanged();
    Q_EMIT ruleSettingsChanged();
    Q_EMIT viewportSettingsChanged();
    Q_EMIT stateRestored();
}

}