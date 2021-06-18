TEMPLATE = lib
CONFIG += static
TARGET = harbour-lib
QT += dbus
QT-= gui

# For HarbourSystemTime
CONFIG += link_pkgconfig
PKGCONFIG += libglibutil

VERSION = 1.0

greaterThan(QT_MAJOR_VERSION, 4) {
QT += qml quick
} else {
QT += declarative
}

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-psabi

CONFIG(debug, debug|release) {
  DEFINES += HARBOUR_DEBUG=1
}

SOURCES += \
    src/HarbourBase32.cpp \
    src/HarbourClipboard.cpp \
    src/HarbourDisplayBlanking.cpp \
    src/HarbourJson.cpp \
    src/HarbourLib.cpp \
    src/HarbourMce.cpp \
    src/HarbourMediaPlugin.cpp \
    src/HarbourOrganizeListModel.cpp \
    src/HarbourPolicyPlugin.cpp \
    src/HarbourProcessState.cpp \
    src/HarbourSelectionListModel.cpp \
    src/HarbourSigChildHandler.cpp \
    src/HarbourSingleImageProvider.cpp \
    src/HarbourSystem.cpp \
    src/HarbourSystemInfo.cpp \
    src/HarbourSystemState.cpp \
    src/HarbourSystemTime.cpp \
    src/HarbourTask.cpp \
    src/HarbourTemporaryFile.cpp \
    src/HarbourTransferMethodInfo.cpp \
    src/HarbourTransferMethodsModel.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
SOURCES += \
    src/HarbourImageProvider.cpp \
    src/HarbourPluginLoader.cpp \
    src/HarbourTheme.cpp
}

INCLUDEPATH += include
PUBLIC_HEADERS += \
    include/HarbourBase32.h \
    include/HarbourClipboard.h \
    include/HarbourDebug.h \
    include/HarbourDisplayBlanking.h \
    include/HarbourJson.h \
    include/HarbourLib.h \
    include/HarbourMediaPlugin.h \
    include/HarbourOrganizeListModel.h \
    include/HarbourPolicyPlugin.h \
    include/HarbourProcessState.h \
    include/HarbourSelectionListModel.h \
    include/HarbourSigChildHandler.h \
    include/HarbourSingleImageProvider.h \
    include/HarbourSystem.h \
    include/HarbourSystemInfo.h \
    include/HarbourSystemState.h \
    include/HarbourSystemTime.h \
    include/HarbourTask.h \
    include/HarbourTemporaryFile.h \
    include/HarbourTransferMethodInfo.h \
    include/HarbourTransferMethodsModel.h

greaterThan(QT_MAJOR_VERSION, 4) {
PUBLIC_HEADERS += \
    include/HarbourImageProvider.h \
    include/HarbourPluginLoader.h \
    include/HarbourTheme.h
OTHER_FILES += qml/*.qml
}

HEADERS += \
  $$PUBLIC_HEADERS \
  src/HarbourMce.h

OTHER_FILES += \
  LICENSE \
  README \
  rpm/harbour-lib-devel.spec

headers.path = $$INSTALL_ROOT$$PREFIX/include/$$TARGET
headers.files = $$PUBLIC_HEADERS
INSTALLS += headers

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

pkgconfig.files = $$TARGET.pc
pkgconfig.path = $$target.path/pkgconfig-qt5
QMAKE_PKGCONFIG_DESCRIPTION = Utility library for Sailfish harbour apps
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
CONFIG += create_pc create_prl no_install_prl
INSTALLS += pkgconfig
