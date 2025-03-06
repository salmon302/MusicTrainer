#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <memory>

namespace MusicTrainer {
namespace ports {
    class MidiAdapter;
}
namespace presentation {

/**
 * @brief Dialog for application settings
 * 
 * This dialog allows users to configure MIDI settings, audio preferences,
 * UI options, and rule sets for validation.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(std::shared_ptr<ports::MidiAdapter> midiAdapter, QWidget* parent = nullptr);
    ~SettingsDialog() override;

signals:
    /**
     * @brief Signal emitted when rule settings are changed
     *
     * This signal is emitted when a rule is enabled or disabled,
     * allowing other components to update their validation accordingly.
     */
    void rulesChanged();
    
    /**
     * @brief Signal emitted when the theme is changed
     *
     * This signal is emitted when the user selects a different theme,
     * allowing other components to update their appearance accordingly.
     *
     * @param index The index of the selected theme
     */
    void themeChanged(int index);

private slots:
    void onMidiInputDeviceChanged(int index);
    void onMidiOutputDeviceChanged(int index);
    void onRefreshMidiDevicesClicked();
    void onThemeChanged(int index);
    void onRuleSetSelectionChanged();
    void onRuleToggled(bool checked);
    void onSaveClicked();
    void onCancelClicked();
    void onRestoreDefaultsClicked();

private:
    void setupUi();
    void setupConnections();
    void populateMidiDevices();
    void populateThemes();
    void populateRuleSets();
    void saveSettings();
    void loadSettings();

    // UI Components
    QTabWidget* m_tabWidget{nullptr};

    // MIDI Tab
    QComboBox* m_midiInputDeviceComboBox{nullptr};
    QComboBox* m_midiOutputDeviceComboBox{nullptr};
    QPushButton* m_refreshMidiDevicesButton{nullptr};
    QCheckBox* m_enableMidiThroughCheckBox{nullptr};
    QSpinBox* m_midiLatencySpinBox{nullptr};

    // Audio Tab
    QCheckBox* m_enableMetronomeCheckBox{nullptr};
    QSlider* m_metronomeVolumeSlider{nullptr};
    QCheckBox* m_enableSoundEffectsCheckBox{nullptr};
    QSlider* m_soundEffectsVolumeSlider{nullptr};

    // UI Tab
    QComboBox* m_themeComboBox{nullptr};
    QCheckBox* m_showMeasureNumbersCheckBox{nullptr};
    QCheckBox* m_showKeySignatureCheckBox{nullptr};
    QCheckBox* m_showVoiceLabelsCheckBox{nullptr};
    QSpinBox* m_fontSizeSpinBox{nullptr};

    // Rules Tab
    QListWidget* m_ruleSetListWidget{nullptr};
    QListWidget* m_rulesListWidget{nullptr};

    // Dialog buttons
    QPushButton* m_saveButton{nullptr};
    QPushButton* m_cancelButton{nullptr};
    QPushButton* m_restoreDefaultsButton{nullptr};

    // Data
    std::shared_ptr<ports::MidiAdapter> m_midiAdapter;
};

} // namespace presentation
} // namespace MusicTrainer