QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CLICode/AuthenticationSystem.cpp \
    CLICode/FileHandler.cpp \
    CLICode/Repository.cpp \
    CLICode/Utils.cpp \
    CLICode/VersionControlSystem.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CLICode/AuthenticationSystem.h \
    CLICode/FileHandler.h \
    CLICode/Repository.h \
    CLICode/Utils.h \
    CLICode/VersionControlSystem.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += "C:/Program Files/OpenSSL-Win64/include" \
               "C:/msys64/mingw64/include"

LIBS += -L"C:/Program Files/OpenSSL-Win64/lib" \
        -llibcrypto \
        -LC:/msys64/mingw64/lib \
        -lminizip

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = ZIM_VCS.ico

RESOURCES += \
    application.qrc
