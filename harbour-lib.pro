TEMPLATE = lib
CONFIG += static
TARGET = harbour-lib
QT += dbus
QT-= gui

CONFIG += link_pkgconfig
PKGCONFIG += libglibutil libqrencode

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
    src/HarbourBase45.cpp \
    src/HarbourBattery.cpp \
    src/HarbourClipboard.cpp \
    src/HarbourColorEditorModel.cpp \
    src/HarbourDisplayBlanking.cpp \
    src/HarbourJson.cpp \
    src/HarbourLib.cpp \
    src/HarbourMce.cpp \
    src/HarbourObject.cpp \
    src/HarbourOrganizeListModel.cpp \
    src/HarbourProcessState.cpp \
    src/HarbourProtoBuf.cpp \
    src/HarbourQrCodeGenerator.cpp \
    src/HarbourQrCodeImageProvider.cpp \
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
    src/HarbourTransferMethodsModel.cpp \
    src/HarbourUtil.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
SOURCES += \
    src/HarbourImageProvider.cpp \
    src/HarbourTheme.cpp
}

INCLUDEPATH += include
PUBLIC_HEADERS += \
    include/HarbourBase32.h \
    include/HarbourBase45.h \
    include/HarbourBattery.h \
    include/HarbourClipboard.h \
    include/HarbourColorEditorModel.h \
    include/HarbourDebug.h \
    include/HarbourDisplayBlanking.h \
    include/HarbourJson.h \
    include/HarbourLib.h \
    include/HarbourObject.h \
    include/HarbourOrganizeListModel.h \
    include/HarbourProcessState.h \
    include/HarbourProtoBuf.h \
    include/HarbourQrCodeGenerator.h \
    include/HarbourQrCodeImageProvider.h \
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
    include/HarbourTransferMethodsModel.h \
    include/HarbourUtil.h

greaterThan(QT_MAJOR_VERSION, 4) {
PUBLIC_HEADERS += \
    include/HarbourImageProvider.h \
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
