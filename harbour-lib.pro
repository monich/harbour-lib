TEMPLATE = lib
CONFIG += static
TARGET = harbour-lib
QT += dbus
QT -= gui

VERSION = 1.0

greaterThan(QT_MAJOR_VERSION, 4) {
QT += qml
} else {
QT += declarative
}

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-psabi

CONFIG(debug, debug|release) {
  DEFINES += HARBOUR_DEBUG=1
}

SOURCES += \
    src/HarbourJson.cpp \
    src/HarbourLib.cpp \
    src/HarbourPluginLoader.cpp \
    src/HarbourSigChildHandler.cpp \
    src/HarbourSystemState.cpp \
    src/HarbourTask.cpp \
    src/HarbourTransferMethodInfo.cpp \
    src/HarbourTransferMethodsModel.cpp

INCLUDEPATH += include
PUBLIC_HEADERS += \
    include/HarbourDebug.h \
    include/HarbourJson.h \
    include/HarbourLib.h \
    include/HarbourPluginLoader.h \
    include/HarbourSigChildHandler.h \
    include/HarbourSystemState.h \
    include/HarbourTask.h \
    include/HarbourTransferMethodInfo.h \
    include/HarbourTransferMethodsModel.h

HEADERS += \
  $$PUBLIC_HEADERS

OTHER_FILES += \
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
