TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(mpvwrapperplugin)
QT += quick

# Qt's QML plugins should be relocatable
CONFIG += relative_qt_rpath

shared {
    win32: DLLDESTDIR = bin
    else: DESTDIR = bin
} else {
    DESTDIR = lib
}

contains(QMAKE_TARGET.arch, x86_64) {
    win32: shared: DLLDESTDIR = $$join(DLLDESTDIR,,,64)
    else: DESTDIR = $$join(DESTDIR,,,64)
}

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

win32: shared {
    VERSION = 1.0.0.0
    QMAKE_TARGET_PRODUCT = "MpvDeclarativeWrapper"
    QMAKE_TARGET_DESCRIPTION = "libmpv wrapper for Qt Quick"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License version 3"
    CONFIG += skip_target_version_ext
}

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
    plugins.qmltypes

uri = wangwenx190.QuickMpv

# Insert the plugins URI into its meta data to enable usage
# of static plugins in QtDeclarative:
QMAKE_MOC_OPTIONS += -Muri=$$replace(uri, "/", ".")

static: CONFIG += builtin_resources
else: CONFIG += install_qml_files

builtin_resources: RESOURCES += mpvdeclarativewrapper.qrc

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    win32: shared: TARGET_DIR = $$OUT_PWD/$$DLLDESTDIR
    else: TARGET_DIR = $$OUT_PWD/$$DESTDIR
    copy_qmldir.target = $$TARGET_DIR/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) "$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)"
    copy_qmltypes.target = $$TARGET_DIR/plugins.qmltypes
    copy_qmltypes.depends = $$_PRO_FILE_PWD_/plugins.qmltypes
    copy_qmltypes.commands = $(COPY_FILE) "$$replace(copy_qmltypes.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qmltypes.target, /, $$QMAKE_DIR_SEP)"
    QMAKE_EXTRA_TARGETS += \
        copy_qmldir \
        copy_qmltypes
    PRE_TARGETDEPS += \
        $$copy_qmldir.target \
        $$copy_qmltypes.target
    install_qml_files {
        copy_qml.target = $$TARGET_DIR/MpvPlayer.qml
        copy_qml.depends = $$_PRO_FILE_PWD_/MpvPlayer.qml
        copy_qml.commands = $(COPY_FILE) "$$replace(copy_qml.depends, /, $$QMAKE_DIR_SEP)" "$$replace(copy_qml.target, /, $$QMAKE_DIR_SEP)"
        QMAKE_EXTRA_TARGETS += copy_qml
        PRE_TARGETDEPS += $$copy_qml.target
    }
}

qmldir.files = qmldir
qmltypes.files = plugins.qmltypes
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \., /)
qmldir.path = $$installPath
qmltypes.path = $$installPath
target.path = $$installPath
INSTALLS += \
    target \
    qmldir \
    qmltypes
install_qml_files {
    qml.files = MpvPlayer.qml
    qml.path = $$installPath
    INSTALLS += qml
}

# plugins.qmltypes is used by Qt Creator for syntax highlighting and the QML code model.  It needs
# to be regenerated whenever the QML elements exported by the plugin change.  This cannot be done
# automatically at compile time because qmlplugindump does not support some QML features and it may
# not be possible when cross-compiling.
#
# To regenerate run 'make qmltypes' which will update the plugins.qmltypes file in the source
# directory.  Then review and commit the changes made to plugins.qmltypes.
#
!cross_compile: qtHaveModule(widgets) {
    # qtPrepareTool() must be called outside a build pass, as it protects
    # against concurrent wrapper creation by omitting it during build passes.
    # However, creating the actual targets is reserved to the build passes.
    qtPrepareTool(QMLPLUGINDUMP, qmlplugindump)
    build_pass|!debug_and_release {
        load(resolve_target)
        qmltypes.target = qmltypes
        qmltypes.commands = $$QMLPLUGINDUMP -nonrelocatable $$uri 1.0 > $$_PRO_FILE_PWD_/plugins.qmltypes
        qmltypes.depends = $$QMAKE_RESOLVED_TARGET
    } else {
        #qmltypes.CONFIG += recursive
    }
    QMAKE_EXTRA_TARGETS += qmltypes
}
