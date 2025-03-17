#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>
#include <QPointF>

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

    // Viewport Settings
    float getViewportZoom() const;
    bool getViewportPreserveOctaveExpansion() const;
    QPointF getViewportScrollPosition() const;
    int getViewportPitchBase() const;

public Q_SLOTS:
    // MIDI Settings
    void setMidiInputDevice(const QString& device);
    void setMidiOutputDevice(const QString& device);
    void setMidiThrough(bool enabled);
    void setMidiLatency(int latency);

    // Audio Settings
    void setMetronomeEnabled(bool enabled);
    void setMetronomeVolume(int volume);
    void setSoundEffectsEnabled(bool enabled);
    void setSoundEffectsVolume(int volume);

    // UI Settings
    void setTheme(int theme);
    void setShowMeasureNumbers(bool show);
    void setShowKeySignature(bool show);
    void setShowVoiceLabels(bool show);
    void setFontSize(int size);

    // Rule Settings
    void setRuleEnabled(const QString& ruleName, bool enabled);

    // Viewport Settings
    void setViewportZoom(float zoom);
    void setViewportPreserveOctaveExpansion(bool preserve);
    void setViewportScrollPosition(const QPointF& pos);
    void setViewportPitchBase(int pitch);
    
    // State Management
    void restoreDefaults();
    void saveState();
    bool loadState();
    void backupState(const QString& backupPath);
    bool restoreFromBackup(const QString& backupPath);

Q_SIGNALS:
    void midiSettingsChanged();
    void audioSettingsChanged();
    void uiSettingsChanged();
    void ruleSettingsChanged();
    void viewportSettingsChanged();
    void stateRestored();

private:
    SettingsState();
    void loadSettings();
    void saveSettings();
    bool validateSettings() const;
    void emitAllChanges();
};

} // namespace MusicTrainer::state