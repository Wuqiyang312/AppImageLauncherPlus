#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QMenu>

namespace Ui {
    class AppImageManager;
}

class AppImageManager : public QDialog {
    Q_OBJECT

public:
    explicit AppImageManager(QWidget *parent = nullptr);
    ~AppImageManager();

private slots:
    void loadAppImages();
    void showContextMenu(const QPoint& pos);
    void editLaunchArgs();
    void createDesktopShortcut();
    void runAppImage();
    void removeAppImage();

private:
    Ui::AppImageManager *ui;
    QString getSelectedAppImagePath();
};
