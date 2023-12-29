#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "CLICode/VersionControlSystem.h"
#include <filesystem>
#include <iostream>
#include "CLICode/AuthenticationSystem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    repos = ui->reposList;
    tabs = ui->tabWidget;
    files = ui->statusList;
    QCoreApplication::setApplicationName( QString("ZIM Version Control System") );
    setWindowTitle( QCoreApplication::applicationName() );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_initBtn_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "C://");

    if (folderPath.isEmpty()) {
        displayError("No Folder has been selected");
    } else {
        VersionControlSystem fileVcs(folderPath.toStdString());
        fileVcs.init();
        repos->addItem(new QListWidgetItem(folderPath));
    }
}

void MainWindow::on_removeRepoBtn_clicked()
{
    delete repos->currentItem();
}

void MainWindow::on_selectBtn_clicked()
{
    try {
        if (QListWidgetItem *currentItem = repos->currentItem()) {
            updateRepoSelection(currentItem);
            tabs->setCurrentIndex(1);
        } else {
            throw std::runtime_error("No item selected in the list.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}

void MainWindow::on_addBtn_clicked()
{
    try {
        if (QListWidgetItem *currentItem = repos->currentItem()) {
            QString baseFolderPath = getCurrentRepo();
            QString filename = QFileDialog::getOpenFileName(this, tr("Select File"), baseFolderPath, "All files (*.*)");

            if (filename.isEmpty()) {
                displayError("No File has been selected");
            } else {
                VersionControlSystem fileVcs(baseFolderPath.toStdString());
                fileVcs.add(filename.toStdString());
                addFileToStatusList(baseFolderPath, filename);
            }
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}

// Utility Functions
void MainWindow::displayError(const QString &message) {
    QMessageBox::information(this, tr("Selection Error"), message);
}

QString MainWindow::getCurrentRepo() {
    return repos->currentItem()->text();
}

void MainWindow::updateRepoSelection(QListWidgetItem *currentItem) {
    QString fullPath = currentItem->text();
    QFileInfo fileInfo(fullPath);
    QString lastPart = fileInfo.fileName();
    ui->repoName->setText(lastPart);
    updateStatusList();
}

void MainWindow::updateStatusList() {
    files->setRowCount(0);
    QString baseFolderPath = getCurrentRepo();
    QDir baseFolder(baseFolderPath);
    VersionControlSystem fileVcs(baseFolderPath.toStdString());
    std::vector<std::string> fileNames = fileVcs.getFiles();
    std::cout << fileNames.size() << std::endl;
    std::vector<bool> status = fileVcs.status();
    for (int i = 0; i < fileNames.size(); i++) {
        int row = files->rowCount();
        files->insertRow(row);
        files->setItem(row, 0, new QTableWidgetItem(baseFolder.relativeFilePath(QString::fromStdString(fileNames[i]))));
        files->setItem(row, 1, new QTableWidgetItem((status[i])?"Modified":"Up to Date"));
    }
}

void MainWindow::addFileToStatusList(const QString &baseFolderPath, const QString &filename) {
    QDir baseFolder(baseFolderPath);
    QString relativePath = baseFolder.relativeFilePath(filename);

    int row = files->rowCount();
    files->insertRow(row);
    files->setItem(row, 0, new QTableWidgetItem(relativePath));
    files->setItem(row, 1, new QTableWidgetItem("New"));
}

void MainWindow::on_removeFileBtn_clicked()
{
    try{
        if (QListWidgetItem *currentRepo = repos->currentItem()){
            QTableWidgetItem *currentItem = files->currentItem();

            if (currentItem) {
                int row = currentItem->row();
                QString baseFolderPath = getCurrentRepo();

                QString filename = files->item(row, 0)->text();

                QString filePath = baseFolderPath + '/' + filename;
                // Call Repository's untrackFile method to untrack the file
                Repository repo = Repository(baseFolderPath.toStdString());
                repo.untrackFile(filePath.toStdString());
                files->removeRow(row); // Remove the row from the table
            } else {
                displayError("No item selected to remove.");
            }
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}


void MainWindow::on_commitBtn_clicked()
{
    try{
        if (QListWidgetItem *currentItem = repos->currentItem()){
            QString baseFolderPath = getCurrentRepo();
            VersionControlSystem fileVcs(baseFolderPath.toStdString());
            fileVcs.commit();
            for (int row = 0; row < files->rowCount(); row++) {
                QTableWidgetItem *fileStatus = files->item(row, 1);
                fileStatus->setText("Up to Date");
            }
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}


void MainWindow::on_openBtn_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Existing Repository"), "C://");

    if (folderPath.isEmpty()) {
        displayError("No Folder has been selected");
    } else if (! std::filesystem::exists(folderPath.toStdString() + "/version_control.csv")) {
        displayError("This folder is not a repository");
    } else {
        VersionControlSystem fileVcs(folderPath.toStdString());
        repos->addItem(new QListWidgetItem(folderPath));
    }
}


void MainWindow::on_refreshBtn_clicked()
{

    try{
        if (QListWidgetItem *currentItem = repos->currentItem()){
            QString baseFolderPath = getCurrentRepo();
            VersionControlSystem fileVcs(baseFolderPath.toStdString());
            fileVcs.refresh();
            updateStatusList();
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}


void MainWindow::on_signin_clicked()
{
    AuthenticationSystem auth;



    // Input from user
    QString user, pass;
    user = ui->username->text();
    pass = ui->password->text();

    if (auth.authenticateUser(user.toStdString(), pass.toStdString())) {
        ui->stackedWidget->setCurrentWidget(ui->repoPage);
    } else {
        displayError("Wrong Credentials.");
    }

}

