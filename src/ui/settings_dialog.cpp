// libraries
#include <QDebug>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QFileInfo>
#include <QMenu>

extern "C" {
    #include <appimage/appimage.h>
}

// local
#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include "shared.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui(new Ui::SettingsDialog),
                                                  settingsFile(getConfig(this)) {
    ui->setupUi(this);

    ui->applicationsDirLineEdit->setPlaceholderText(integratedAppImagesDestination().absolutePath());

    loadSettings();

// cosmetic changes in lite mode
#ifdef BUILD_LITE
    ui->daemonIsEnabledCheckBox->setChecked(true);
    ui->daemonIsEnabledCheckBox->setEnabled(false);

    ui->askMoveCheckBox->setChecked(false);
    ui->askMoveCheckBox->setEnabled(false);
#endif

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onDialogAccepted);
    connect(ui->chooseAppsDirToolButton, &QToolButton::released, this, &SettingsDialog::onChooseAppsDirClicked);
    connect(ui->additionalDirsAddButton, &QToolButton::released, this, &SettingsDialog::onAddDirectoryToWatchButtonClicked);
    connect(ui->additionalDirsRemoveButton, &QToolButton::released, this, &SettingsDialog::onRemoveDirectoryToWatchButtonClicked);
    connect(ui->additionalDirsListWidget, &QListWidget::itemActivated, this, &SettingsDialog::onDirectoryToWatchItemActivated);
    connect(ui->additionalDirsListWidget, &QListWidget::itemClicked, this, &SettingsDialog::onDirectoryToWatchItemActivated);

    // AppImage Manager connections
    connect(ui->refreshAppImagesButton, &QPushButton::clicked, this, &SettingsDialog::loadAppImages);
    connect(ui->appImageTable, &QTableWidget::customContextMenuRequested,
            this, &SettingsDialog::showAppImageContextMenu);

    // Set up AppImage table
    ui->appImageTable->setColumnWidth(0, 200);
    ui->appImageTable->setColumnWidth(1, 350);
    ui->appImageTable->setColumnWidth(2, 200);
    ui->appImageTable->horizontalHeader()->setStretchLastSection(true);

    // Load AppImages initially
    loadAppImages();

    QStringList availableFeatures;

#ifdef ENABLE_UPDATE_HELPER
    availableFeatures << "<span style='color: green;'>✔</span> " + tr("updater available for AppImages supporting AppImageUpdate");
#else
    availableFeatures << "<span style='color: red;'>🞬</span> " + tr("updater unavailable");
#endif

#ifdef BUILD_LITE
    availableFeatures << "<br /><br />"
                      << tr("<strong>Note: this is an AppImageLauncher Lite build, only supports a limited set of features</strong><br />"
                            "Please install the full version via the provided native packages to enjoy the full AppImageLauncher experience");
#endif

    ui->featuresLabel->setText(availableFeatures.join('\n'));

    // no matter what tab was selected when saving in Qt designer, we want to start up with the first tab
    ui->tabWidget->setCurrentWidget(ui->launcherTab);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::addDirectoryToWatchToListView(const QString& dirPath) {
    // empty paths are not permitted
    if (dirPath.isEmpty())
        return;

    const QDir dir(dirPath);

    // we don't want to redundantly add the main integration directory
    if (dir == integratedAppImagesDestination())
        return;

    QIcon icon;

    auto findIcon = [](const std::initializer_list<QString>& names) {
        for (const auto& i : names) {
            auto icon = QIcon::fromTheme(i, loadIconWithFallback(i));

            if (!icon.isNull())
                return icon;
        }

        return QIcon{};
    };

    if (dir.exists()) {
        icon = findIcon({"folder"});
    } else {
        // TODO: search for more meaningful icon, "remove" doesn't really show the directory is missing
        icon = findIcon({"remove"});
    }

    if (icon.isNull()) {
        qDebug() << "item icon unavailable, using fallback";
    }

    auto* item = new QListWidgetItem(icon, dirPath);
    ui->additionalDirsListWidget->addItem(item);
}

void SettingsDialog::loadSettings() {
    const auto daemonIsEnabled = settingsFile->value("AppImageLauncher/enable_daemon", "true").toBool();
    const auto askMoveChecked = settingsFile->value("AppImageLauncher/ask_to_move", "true").toBool();

    if (settingsFile) {
        ui->daemonIsEnabledCheckBox->setChecked(daemonIsEnabled);
        ui->askMoveCheckBox->setChecked(askMoveChecked);
        ui->applicationsDirLineEdit->setText(settingsFile->value("AppImageLauncher/destination").toString());

        const auto additionalDirsPath = settingsFile->value("appimagelauncherd/additional_directories_to_watch", "").toString();
        for (const auto& dirPath : additionalDirsPath.split(":")) {
            addDirectoryToWatchToListView(dirPath);
        }
    }
}

void SettingsDialog::onDialogAccepted() {
    saveSettings();
    toggleDaemon();
}

void SettingsDialog::saveSettings() {
    QStringList additionalDirsToWatch;

    {
        QListWidgetItem* currentItem;

        for (int i = 0; (currentItem = ui->additionalDirsListWidget->item(i)) != nullptr; ++i) {
            additionalDirsToWatch << currentItem->text();
        }
    }

    // temporary workaround to fill in the monitorMountedFilesystems with the same value it had in the old settings
    // this is supposed to support the option while hiding it in the settings
    int monitorMountedFilesystems = -1;
    {
        const auto oldSettings = getConfig();

        static constexpr auto oldKey = "appimagelauncherd/monitor_mounted_filesystems";

        // getConfig might return a null pointer if the config file doesn't exist
        // we have to handle this, obviously
        if (oldSettings != nullptr && oldSettings->contains(oldKey)) {
            const auto oldValue = oldSettings->value(oldKey).toBool();
            monitorMountedFilesystems = oldValue ? 1 : 0;
        }
    }

    createConfigFile(ui->askMoveCheckBox->isChecked(),
                     ui->applicationsDirLineEdit->text(),
                     ui->daemonIsEnabledCheckBox->isChecked(),
                     additionalDirsToWatch,
                     monitorMountedFilesystems);

    // reload settings
    loadSettings();
}

void SettingsDialog::toggleDaemon() {
    // assumes defaults if config doesn't exist or lacks the related key(s)
    if (settingsFile) {
        if (settingsFile->value("AppImageLauncher/enable_daemon", "true").toBool()) {
            system("systemctl --user enable  appimagelauncherd.service");
            // we want to actually restart the service to apply the new configuration
            system("systemctl --user restart appimagelauncherd.service");
        } else {
            system("systemctl --user disable appimagelauncherd.service");
            system("systemctl --user stop    appimagelauncherd.service");
        }
    }
}

void SettingsDialog::onChooseAppsDirClicked() {
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setWindowTitle(tr("Select Applications directory"));
    fileDialog.setDirectory(integratedAppImagesDestination().absolutePath());

    // Gtk+ >= 3 segfaults when trying to use the native dialog, therefore we need to enforce the Qt one
    // See #218 for more information
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        ui->applicationsDirLineEdit->setText(dirPath);
    }
}

void SettingsDialog::onAddDirectoryToWatchButtonClicked() {
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setWindowTitle(tr("Select additional directory to watch"));
    fileDialog.setDirectory(QStandardPaths::locate(QStandardPaths::HomeLocation, ".", QStandardPaths::LocateDirectory));

    // Gtk+ >= 3 segfaults when trying to use the native dialog, therefore we need to enforce the Qt one
    // See #218 for more information
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        addDirectoryToWatchToListView(dirPath);
    }
}

void SettingsDialog::onRemoveDirectoryToWatchButtonClicked() {
    auto* widget = ui->additionalDirsListWidget;

    auto* currentItem = widget->currentItem();

    if (currentItem == nullptr)
        return;

    const auto index = widget->row(currentItem);

    // after taking it, we have to delete it ourselves, Qt docs say
    auto deletedItem = widget->takeItem(index);
    delete deletedItem;

    // we should deactivate the remove button once the last item is gone
    if (widget->item(0) == nullptr) {
        ui->additionalDirsRemoveButton->setEnabled(false);
    }
}

void SettingsDialog::onDirectoryToWatchItemActivated(QListWidgetItem* item) {
    // we activate the button based on whether there's an item selected
    ui->additionalDirsRemoveButton->setEnabled(item != nullptr);
}

// AppImage Manager implementation

void SettingsDialog::loadAppImages()
{
    ui->appImageTable->setRowCount(0);

    QStringList appImages = getIntegratedAppImages();

    for (const QString& appImagePath : appImages) {
        QFileInfo fileInfo(appImagePath);
        QString name = fileInfo.fileName();
        QString launchArgs = readAppImageLaunchArgs(appImagePath);

        int row = ui->appImageTable->rowCount();
        ui->appImageTable->insertRow(row);

        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* pathItem = new QTableWidgetItem(appImagePath);
        QTableWidgetItem* argsItem = new QTableWidgetItem(launchArgs);

        // Store full path in user data
        nameItem->setData(Qt::UserRole, appImagePath);

        ui->appImageTable->setItem(row, 0, nameItem);
        ui->appImageTable->setItem(row, 1, pathItem);
        ui->appImageTable->setItem(row, 2, argsItem);
    }
}

QString SettingsDialog::getSelectedAppImagePath()
{
    auto selectedItems = ui->appImageTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    int row = selectedItems.first()->row();
    return ui->appImageTable->item(row, 1)->text();
}

void SettingsDialog::showAppImageContextMenu(const QPoint& pos)
{
    QTableWidgetItem* item = ui->appImageTable->itemAt(pos);
    if (!item) {
        return;
    }

    QMenu contextMenu(tr("Context menu"), this);

    QAction* runAction = contextMenu.addAction(tr("Run"));
    QAction* editArgsAction = contextMenu.addAction(tr("Edit Launch Arguments"));
    QAction* createShortcutAction = contextMenu.addAction(tr("Create Desktop Shortcut"));
    contextMenu.addSeparator();
    QAction* removeAction = contextMenu.addAction(tr("Remove"));

    QAction* selectedAction = contextMenu.exec(ui->appImageTable->viewport()->mapToGlobal(pos));

    if (selectedAction == runAction) {
        runSelectedAppImage();
    } else if (selectedAction == editArgsAction) {
        editAppImageLaunchArgs();
    } else if (selectedAction == createShortcutAction) {
        createAppImageDesktopShortcut();
    } else if (selectedAction == removeAction) {
        removeSelectedAppImage();
    }
}

void SettingsDialog::editAppImageLaunchArgs()
{
    QString appImagePath = getSelectedAppImagePath();
    if (appImagePath.isEmpty()) {
        return;
    }

    QString currentArgs = readAppImageLaunchArgs(appImagePath);

    bool ok;
    QString newArgs = QInputDialog::getText(this,
                                            tr("Edit Launch Arguments"),
                                            tr("Launch arguments for %1:").arg(QFileInfo(appImagePath).fileName()),
                                            QLineEdit::Normal,
                                            currentArgs,
                                            &ok);

    if (ok) {
        if (writeAppImageLaunchArgs(appImagePath, newArgs)) {
            QMessageBox::information(this, tr("Success"),
                                   tr("Launch arguments saved successfully."));
            loadAppImages();
        } else {
            QMessageBox::critical(this, tr("Error"),
                                tr("Failed to save launch arguments."));
        }
    }
}

void SettingsDialog::createAppImageDesktopShortcut()
{
    QString appImagePath = getSelectedAppImagePath();
    if (appImagePath.isEmpty()) {
        return;
    }

    // Get desktop path
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFileInfo appImageInfo(appImagePath);
    QString shortcutPath = desktopPath + "/" + appImageInfo.completeBaseName() + ".desktop";

    // Get desktop file from libappimage
    const char* desktopFilePath = appimage_registered_desktop_file_path(
        appImagePath.toStdString().c_str(), nullptr, false);

    if (!desktopFilePath || !QFile::exists(desktopFilePath)) {
        QMessageBox::critical(this, tr("Error"),
                            tr("Could not find desktop file for this AppImage."));
        return;
    }

    // Copy desktop file to desktop
    if (QFile::copy(desktopFilePath, shortcutPath)) {
        // Make it executable
        makeExecutable(shortcutPath);

        QMessageBox::information(this, tr("Success"),
                               tr("Desktop shortcut created successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"),
                            tr("Failed to create desktop shortcut."));
    }
}

void SettingsDialog::runSelectedAppImage()
{
    QString appImagePath = getSelectedAppImagePath();
    if (appImagePath.isEmpty()) {
        return;
    }

    QString launchArgs = readAppImageLaunchArgs(appImagePath);

    QStringList arguments;
    if (!launchArgs.isEmpty()) {
        // Simple split - doesn't handle quoted strings with spaces
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        arguments = launchArgs.split(' ', Qt::SkipEmptyParts);
#else
        arguments = launchArgs.split(' ', QString::SkipEmptyParts);
#endif
    }

    if (QProcess::startDetached(appImagePath, arguments)) {
        QMessageBox::information(this, tr("Success"),
                               tr("AppImage launched successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"),
                            tr("Failed to launch AppImage."));
    }
}

void SettingsDialog::removeSelectedAppImage()
{
    QString appImagePath = getSelectedAppImagePath();
    if (appImagePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(appImagePath);

    auto reply = QMessageBox::question(this, tr("Confirm Removal"),
                                      tr("Are you sure you want to remove %1?\nThis will unregister the AppImage and delete the file.")
                                      .arg(fileInfo.fileName()),
                                      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Unregister AppImage
        if (unregisterAppImage(appImagePath)) {
            // Delete the file
            if (QFile::remove(appImagePath)) {
                // Also remove config file if exists
                QString configPath = getAppImageConfigPath(appImagePath);
                QFile::remove(configPath);

                QMessageBox::information(this, tr("Success"),
                                       tr("AppImage removed successfully."));
                loadAppImages();
            } else {
                QMessageBox::critical(this, tr("Error"),
                                    tr("Failed to delete AppImage file."));
            }
        } else {
            QMessageBox::critical(this, tr("Error"),
                                tr("Failed to unregister AppImage."));
        }
    }
}

