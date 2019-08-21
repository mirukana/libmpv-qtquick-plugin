TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(mpvquickplugin)
QT += quick

uri = QuickMpv

# Disable deprecated mpv APIs.
DEFINES += MPV_ENABLE_DEPRECATED=0

win32:!mingw {
    # You can download shinchiro's libmpv SDK (build from mpv's master branch) from:
    # https://sourceforge.net/projects/mpv-player-windows/files/libmpv/
    isEmpty(MPV_SDK_DIR) {
        error("Setup MPV_SDK_DIR first.")
    } else {
        MPV_LIB_DIR = $$MPV_SDK_DIR/lib
        contains(QMAKE_TARGET.arch, x86_64): MPV_LIB_DIR = $$join(MPV_LIB_DIR,,,64)
        INCLUDEPATH += $$MPV_SDK_DIR/include
        # How to generate .lib files from .def files for MSVC:
        # https://github.com/mpv-player/mpv/blob/master/DOCS/compile-windows.md#linking-libmpv-with-msvc-programs
        LIBS += -L$$MPV_LIB_DIR -lmpv
    }
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += mpv
}

CONFIG(shared, static|shared): VERSION = 1.0.0

win32: CONFIG(shared, static|shared) {
    QMAKE_TARGET_COMPANY = "wangwenx190"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License version 3"
    QMAKE_TARGET_DESCRIPTION = "libmpv warpper for Qt Quick. Can be used as a visual element in .qml files directly."
    QMAKE_TARGET_PRODUCT = "MpvQuickPlugin"
}

HEADERS += \
    mpvqthelper.hpp \
    mpvplayer.h \
    mpvquickplugin.h

SOURCES += \
    mpvplayer.cpp \
    mpvquickplugin.cpp

DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) "$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
