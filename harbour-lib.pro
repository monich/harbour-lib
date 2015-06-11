TEMPLATE = lib
CONFIG += static
TARGET = harbour-lib
QT += dbus qml
QT -= gui

VERSION = 1.0

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

CONFIG(debug, debug|release) {
  DEFINES += HARBOUR_DEBUG=1
}

SOURCES += \
    src/HarbourLib.cpp \
    src/HarbourSystemState.cpp

INCLUDEPATH += include
PUBLIC_HEADERS += \
    include/HarbourDebug.h \
    include/HarbourLib.h \
    include/HarbourSystemState.h

HEADERS += \
  $$PUBLIC_HEADERS

OTHER_FILES += rpm/harbour-lib-devel.spec

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
