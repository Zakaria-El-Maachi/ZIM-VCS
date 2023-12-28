#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTabWidget>
#include <QTableWidget>
#include "CLICode/VersionControlSystem.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_initBtn_clicked();

    void on_removeRepoBtn_clicked();

    void on_selectBtn_clicked();

    void on_addBtn_clicked();

    void on_removeFileBtn_clicked();

    void on_commitBtn_clicked();

    void on_openBtn_clicked();

    void on_refreshBtn_clicked();

private:
    Ui::MainWindow *ui;
    QListWidget *repos;
    QTabWidget *tabs;
    QTableWidget *files;
    void displayError(const QString &message);
    QString getCurrentRepo();
    void updateRepoSelection(QListWidgetItem *currentItem);
    void updateStatusList();
    void addFileToStatusList(const QString &baseFolderPath, const QString &filename);

};
#endif // MAINWINDOW_H
