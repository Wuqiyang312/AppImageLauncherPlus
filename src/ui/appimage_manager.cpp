#include "appimage_manager.h"
#include "ui_appimage_manager.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

extern "C" {
    #include <appimage/appimage.h>
}

#include "../shared/shared.h"

AppImageManager::AppImageManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppImageManager)
{
    ui->setupUi(this);

    // Set up table
    ui->appImageTable->setColumnWidth(0, 200);
    ui->appImageTable->setColumnWidth(1, 350);
    ui->appImageTable->setColumnWidth(2, 200);
    ui->appImageTable->horizontalHeader()->setStretchLastSection(true);

    // Connect signals
    connect(ui->refreshButton, &QPushButton::clicked, this, &AppImageManager::loadAppImages);
    connect(ui->appImageTable, &QTableWidget::customContextMenuRequested,
            this, &AppImageManager::showContextMenu);

    // Load AppImages
    loadAppImages();
}

AppImageManager::~AppImageManager()
{
    delete ui;
}

void AppImageManager::loadAppImages()
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

QString AppImageManager::getSelectedAppImagePath()
{
    auto selectedItems = ui->appImageTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    int row = selectedItems.first()->row();
    return ui->appImageTable->item(row, 1)->text();
}

void AppImageManager::showContextMenu(const QPoint& pos)
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
        runAppImage();
    } else if (selectedAction == editArgsAction) {
        editLaunchArgs();
    } else if (selectedAction == createShortcutAction) {
        createDesktopShortcut();
    } else if (selectedAction == removeAction) {
        removeAppImage();
    }
}

void AppImageManager::editLaunchArgs()
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

void AppImageManager::createDesktopShortcut()
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

void AppImageManager::runAppImage()
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

void AppImageManager::removeAppImage()
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
