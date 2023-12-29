#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QFileInfo"
#include "QMessageBox"
#include "CLICode/VersionControlSystem.h"
#include <filesystem>
#include <iostream>
#include "CLICode/AuthenticationSystem.h"

// Constructor for MainWindow class
MainWindow::MainWindow(QWidget *parent)
    // Setup UI and initialize class members
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    repos = ui->reposList; // List of repositories
    tabs = ui->tabWidget;   // Tabs in the UI
    files = ui->statusList;  // List showing file status
    QCoreApplication::setApplicationName( QString("ZIM Version Control System") );
    setWindowTitle( QCoreApplication::applicationName() );  // Set window title

}

// Destructor for MainWindow class
MainWindow::~MainWindow()
{
    delete ui;
}

// Slot function for initializing a new repository
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

// Slot function to remove a repository from the list
void MainWindow::on_removeRepoBtn_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "C://");

    if (dirPath.isEmpty()) {
        displayError("No Folder has been selected");
    } else {
        QDir dir(dirPath);

        // Prepare file paths for version_control.csv and version.txt
        QString versionControlPath = dir.filePath("version_control.csv");
        QString versionPath = dir.filePath("version.txt");
        QString historyPath = dir.filePath("history");  // History is a directory

        // Check if the repository files exist
        if (QFile::exists(versionControlPath)) {
            bool removed = true;

            // Remove version_control.csv
            if (!QFile::remove(versionControlPath)) {
                displayError("Error removing version_control.csv");
                removed = false;
            }

            // Remove version.txt
            if (!QFile::remove(versionPath)) {
                displayError("Error removing version.txt");
                removed = false;
            }

            // Remove the history directory
            if (QDir(historyPath).exists()) {
                QDir historyDir(historyPath);
                historyDir.removeRecursively();  // Remove all contents and the directory itself
            }

            // If all items removed successfully, update the UI accordingly
            if (removed) {
                delete repos->currentItem();
                ui->repoName->setText("No Repository Selected");
                ui->statusList->setRowCount(0);
            }

        } else {
            displayError("This folder is not a repository");
        }
    }
}

// Slot function to select a repository from the list
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
            QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Select Files"), baseFolderPath, "All files (*.*)");

            if (filenames.isEmpty()) {
                displayError("No files have been selected");
            } else {
                VersionControlSystem fileVcs(baseFolderPath.toStdString());
                QStringList reservedNames = { "version_control.csv", "version.txt", "history" };

                for (const QString &filename : filenames) {
                    // Skip if the file is a reserved name
                    if (reservedNames.contains(QFileInfo(filename).fileName())) continue;

                    fileVcs.add(filename.toStdString());
                    addFileToStatusList(baseFolderPath, filename);
                }
            }
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}


void MainWindow::on_addFolderBtn_clicked()
{
    try {
        if (QListWidgetItem *currentItem = repos->currentItem()) {
            QString baseFolderPath = getCurrentRepo();
            QString directoryName = QFileDialog::getExistingDirectory(this, tr("Select Directory"), baseFolderPath);

            if (directoryName.isEmpty()) {
                displayError("No directory has been selected");
            } else {
                QString dirNameOnly = QFileInfo(directoryName).fileName();
                // Skip if the directory is reserved
                if (dirNameOnly == "history") {
                    displayError("Reserved directory cannot be added");
                    return;
                }

                VersionControlSystem fileVcs(baseFolderPath.toStdString());
                fileVcs.addDirectory(directoryName.toStdString());
                addFileToStatusList(baseFolderPath, directoryName);
            }
        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
}



void MainWindow::on_addAllBtn_clicked()
{
    try {
        if (QListWidgetItem *currentItem = repos->currentItem()) {
            QString baseFolderPath = getCurrentRepo();
            QDir baseFolderDir(baseFolderPath);
            VersionControlSystem fileVcs(baseFolderPath.toStdString());
            QStringList reservedNames = { "version_control.csv", "version.txt", "history" };

            QFileInfoList entries = baseFolderDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo &entry : entries) {
                QString path = entry.absoluteFilePath();
                // Skip if the item is reserved
                if (reservedNames.contains(entry.fileName())) continue;

                if (entry.isDir()) {
                    fileVcs.addDirectory(path.toStdString());
                } else if (entry.isFile()) {
                    fileVcs.add(path.toStdString());
                }
                addFileToStatusList(baseFolderPath, path);
            }

        } else {
            throw std::runtime_error("No Repository to add files from.");
        }
    } catch (const std::exception& e) {
        displayError(e.what());
    }
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
            updateRepoSelection(repos->currentItem());
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


void MainWindow::on_rollbackBtn_clicked()
{
    try{
        if (QListWidgetItem *currentItem = repos->currentItem()){
            QString baseFolderPath = getCurrentRepo();
            VersionControlSystem fileVcs(baseFolderPath.toStdString());
            fileVcs.rollback(ui->rollback->text().toInt());
            updateRepoSelection(currentItem);
        } else {
            throw std::runtime_error("No Repository to rollback from.");
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
    VersionControlSystem vcs(fullPath.toStdString());
    ui->repoName->setText(lastPart + " : Version " + QString::number(vcs.getVersion()));
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


