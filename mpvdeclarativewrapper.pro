TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(mpvwrapperplugin)
QT += quick

CONFIG(shared, static|shared) {
    win32: DLLDESTDIR = bin
    else: DESTDIR = bin
} else: CONFIG(static, static|shared) {
    DESTDIR = lib
}

contains(QMAKE_TARGET.arch, x86_64) {
    win32: CONFIG(shared, static|shared): DLLDESTDIR = $$join(DLLDESTDIR,,,64)
    else: DESTDIR = $$join(DESTDIR,,,64)
}

uri = wangwenx190.QuickMpv

# Disable deprecated mpv APIs.
DEFINES += MPV_ENABLE_DEPRECATED=0

win32:!mingw {
    # You can download shinchiro's libmpv SDK (build from mpv's master branch) from:
    # https://sourceforge.net/projects/mpv-player-windows/files/libmpv/
    isEmpty(MPV_SDK_DIR) {
        error("You have to setup \"MPV_SDK_DIR\" in \"user.conf\" first!")
    } else {
        MPV_BIN_DIR = $$MPV_SDK_DIR/bin
        MPV_LIB_DIR = $$MPV_SDK_DIR/lib
        contains(QMAKE_TARGET.arch, x86_64) {
            MPV_BIN_DIR = $$join(MPV_BIN_DIR,,,64)
            MPV_LIB_DIR = $$join(MPV_LIB_DIR,,,64)
        }
        INCLUDEPATH += $$MPV_SDK_DIR/include
        # How to generate .lib files from .def files for MSVC:
        # https://github.com/mpv-player/mpv/blob/master/DOCS/compile-windows.md#linking-libmpv-with-msvc-programs
        LIBS += -L$$MPV_SDK_DIR -L$$MPV_LIB_DIR -lmpv
        libmpv.path = $$[QT_INSTALL_BINS]
        libmpv.files = \
            $$MPV_SDK_DIR/mpv-1.dll \
            $$MPV_BIN_DIR/mpv.dll \
            $$MPV_BIN_DIR/d3dcompiler_43.dll \
            $$MPV_BIN_DIR/youtube-dl.exe
        INSTALLS += libmpv
    }
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += mpv
}

win32: CONFIG(shared, static|shared): RC_FILE = mpvdeclarativewrapper.rc
else: unix: CONFIG(shared, static|shared): VERSION = 1.0.0

HEADERS += \
    mpvqthelper.hpp \
    mpvdeclarativeobject.h \
    mpvdeclarativewrapper.h

SOURCES += \
    mpvdeclarativeobject.cpp \
    mpvdeclarativewrapper.cpp

DISTFILES += \
    MpvPlayer.qml \
    qmldir \
    mpvwrapperplugin.qmltypes

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    win32: CONFIG(shared, static|shared): TARGET_DIR = $$OUT_PWD/$$DLLDESTDIR
    else: TARGET_DIR = $$OUT_PWD/$$DESTDIR
    copy_qml.target = $$TARGET_DIR/MpvPlayer.qml
    copy_qml.depends = $$_PRO_FILE_PWD_/MpvPlayer.qml
    copy_qml.commands = $(COPY_FILE) "$$replace(copy_qml.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qml.target, /, $$QMAKE_DIR_SEP)"
    copy_qmldir.target = $$TARGET_DIR/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) "$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)"
    copy_qmltypes.target = $$TARGET_DIR/mpvwrapperplugin.qmltypes
    copy_qmltypes.depends = $$_PRO_FILE_PWD_/mpvwrapperplugin.qmltypes
    copy_qmltypes.commands = $(COPY_FILE) "$$replace(copy_qmltypes.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmltypes.target, /, $$QMAKE_DIR_SEP)"
    QMAKE_EXTRA_TARGETS += \
        copy_qml \
        copy_qmldir \
        copy_qmltypes
    PRE_TARGETDEPS += \
        $$copy_qml.target \
        $$copy_qmldir.target \
        $$copy_qmltypes.target
}

qml.files = MpvPlayer.qml
qmldir.files = qmldir
qmltypes.files = mpvwrapperplugin.qmltypes
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \., /)
qml.path = $$installPath
qmldir.path = $$installPath
qmltypes.path = $$installPath
target.path = $$installPath
INSTALLS += \
    target \
    qml \
    qmldir \
    qmltypes
