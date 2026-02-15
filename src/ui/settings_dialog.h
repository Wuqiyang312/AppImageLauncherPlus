#pragma once

// system
#include <memory>

// libraries
#include <QDialog>
#include <QListWidgetItem>
#include <QSettings>


namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    ~SettingsDialog() override;

protected slots:
    void onChooseAppsDirClicked();
    void onAddDirectoryToWatchButtonClicked();
    void onRemoveDirectoryToWatchButtonClicked();
    void onDirectoryToWatchItemActivated(QListWidgetItem* item);

    void onDialogAccepted();

    // AppImage Manager slots
    void loadAppImages();
    void showAppImageContextMenu(const QPoint& pos);
    void editAppImageLaunchArgs();
    void createAppImageDesktopShortcut();
    void runSelectedAppImage();
    void removeSelectedAppImage();

private:
    void loadSettings();

    void saveSettings();

    void toggleDaemon();

    void addDirectoryToWatchToListView(const QString& dirPath);

    // AppImage Manager helper
    QString getSelectedAppImagePath();

    Ui::SettingsDialog* ui;
    QSettings* settingsFile;
};
