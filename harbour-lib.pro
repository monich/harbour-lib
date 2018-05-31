TEMPLATE = lib
CONFIG += static
TARGET = harbour-lib
QT += dbus
QT-= gui

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
    src/HarbourDisplayBlanking.cpp \
    src/HarbourJson.cpp \
    src/HarbourLib.cpp \
    src/HarbourMce.cpp \
    src/HarbourSigChildHandler.cpp \
    src/HarbourSystemState.cpp \
    src/HarbourTask.cpp \
    src/HarbourTemporaryFile.cpp \
    src/HarbourTransferMethodInfo.cpp \
    src/HarbourTransferMethodsModel.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
SOURCES += \
    src/HarbourImageProvider.cpp \
    src/HarbourPluginLoader.cpp
}

INCLUDEPATH += include
PUBLIC_HEADERS += \
    include/HarbourDebug.h \
    include/HarbourDisplayBlanking.h \
    include/HarbourJson.h \
    include/HarbourLib.h \
    include/HarbourSigChildHandler.h \
    include/HarbourSystemState.h \
    include/HarbourTask.h \
    include/HarbourTemporaryFile.h \
    include/HarbourTransferMethodInfo.h \
    include/HarbourTransferMethodsModel.h

greaterThan(QT_MAJOR_VERSION, 4) {
PUBLIC_HEADERS += \
    include/HarbourImageProvider.h \
    include/HarbourPluginLoader.h
}

HEADERS += \
  $$PUBLIC_HEADERS \
  src/HarbourMce.h

OTHER_FILES += \
  LICENSE \
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
