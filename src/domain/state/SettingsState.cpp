#include "domain/state/SettingsState.h"
#include <QSettings>
#include <QApplication>
#include <QDebug>

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

void SettingsState::restoreDefaults() {
    QSettings settings;
    settings.clear();
    
    // Re-emit all signals since everything changed
    Q_EMIT midiSettingsChanged();
    Q_EMIT audioSettingsChanged();
    Q_EMIT uiSettingsChanged();
    Q_EMIT ruleSettingsChanged();
}