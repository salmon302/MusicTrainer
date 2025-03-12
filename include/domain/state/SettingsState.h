#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

namespace MusicTrainer::state {

/**
 * @brief Manages application-wide settings state
 */
class SettingsState : public QObject {
    Q_OBJECT

public:
    static SettingsState& instance();

    // MIDI Settings
    QString getMidiInputDevice() const;
    QString getMidiOutputDevice() const;
    bool getMidiThrough() const;
    int getMidiLatency() const;

    // Audio Settings
    bool getMetronomeEnabled() const;
    int getMetronomeVolume() const;
    bool getSoundEffectsEnabled() const;
    int getSoundEffectsVolume() const;

    // UI Settings
    int getTheme() const;
    bool getShowMeasureNumbers() const;
    bool getShowKeySignature() const;
    bool getShowVoiceLabels() const;
    int getFontSize() const;

    // Rule Settings
    bool getRuleEnabled(const QString& ruleName) const;

public Q_SLOTS:
    void setMidiInputDevice(const QString& device);
    void setMidiOutputDevice(const QString& device);
    void setMidiThrough(bool enabled);
    void setMidiLatency(int latency);
    void setMetronomeEnabled(bool enabled);
    void setMetronomeVolume(int volume);
    void setSoundEffectsEnabled(bool enabled);
    void setSoundEffectsVolume(int volume);
    void setTheme(int theme);
    void setShowMeasureNumbers(bool show);
    void setShowKeySignature(bool show);
    void setShowVoiceLabels(bool show);
    void setFontSize(int size);
    void setRuleEnabled(const QString& ruleName, bool enabled);
    void restoreDefaults();

Q_SIGNALS:
    void midiSettingsChanged();
    void audioSettingsChanged();
    void uiSettingsChanged();
    void ruleSettingsChanged();

private:
    SettingsState();
    void loadSettings();
    void saveSettings();
};

} // namespace MusicTrainer::state