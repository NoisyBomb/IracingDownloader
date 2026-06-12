QT += core gui network webenginewidgets webchannel
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    core/authmanager.cpp \
    core/iracingpathresolver.cpp \
    core/setupdownloader.cpp \
    core/setupinstaller.cpp \
    core/setupmanager.cpp \
    providers/gridandgoprovider.cpp \
    registry/carregistry.cpp \
    registry/trackregistry.cpp \
    ui/mainwindow.cpp

HEADERS += \
    models/car.h \
    models/track.h \
    models/setup.h \
    models/iracingweek.h \
    core/authmanager.h \
    core/iracingpathresolver.h \
    core/setupdownloader.h \
    core/setupinstaller.h \
    core/setupmanager.h \
    providers/gridandgoprovider.h \
    registry/carregistry.h \
    registry/trackregistry.h \
    ui/mainwindow.h

FORMS += \
    ui/mainwindow.ui

RESOURCES += \
    resources/resources.qrc

RC_ICONS = resources/icon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
