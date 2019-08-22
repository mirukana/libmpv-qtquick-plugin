# libmpv-qtquick-plugin
libmpv wrapper for Qt Quick. Can be used as a visual element in .qml files directly.

## Features
- Can be easily embeded into any Qt Quick applications.
- Support almost every property that libmpv supports.
- Support both shared and static building and loading.

## TODO
- Support more properties from libmpv.
- Support *CMake + Ninja* build system.

## Usage
Once you have installed this plugin successfully, you can use it like any other normal visual elements of Qt Quick in .qml files:
```qml
import QtQuick.Dialogs 1.3
import wangwenx190.QuickMpv 1.0

FileDialog {
    id: fileDialog

    title: qsTr("Please select a media file.")
    folder: shortcuts.movies
    nameFilters: [qsTr("Video files (%1)").arg(mpvPlayer.videoSuffixes.join(' ')), qsTr("Audio files (%1)").arg(mpvPlayer.audioSuffixes.join(' ')), qsTr("All files (*)")

    onAccepted: mpvPlayer.source = fileDialog.fileUrl
}

MpvPlayer {
    id: mpvPlayer

    source: "file:///D:/Videos/test.mkv" // playback will start immediately once the source url is changed
    hrSeek: true
    loadScripts: true
    ytdl: true
    screenshotFormat: "png" // "jpg" or "png"
    logLevel: MpvObject.DebugLevel
    volume: 85 // 0-100

    onPositionChanged: // do something
    onDurationChanged: // do something
    onVideoSizeChanged: // do something
    onPlaybackStateChanged: // do something
    onMediaStatusChanged: // do something
}
```
Notes
- `mpvPlayer.duration`, `mpvPlayer.position` and `mpvPlayer.seek(position)` use **SECONDS** instead of milliseconds.
- `mpvPlayer.seek(position)` uses absolute position, not relative offset.
- You can use `mpvPlayer.open(url)` to load and play *url* directly, it is equivalent to `mpvPlayer.source = url` + `mpvPlayer.play()`.
- You can also use `mpvPlayer.play()` to resume a paused playback, `mpvPlayer.pause()` to pause a playing playback, `mpvPlayer.stop()` to stop a loaded playback and `mpvPlayer.seek(position)` to jump to a different position.
- To get the current playback state, use `mpvPlayer.isPlaying()`, `mpvPlayer.isPaused()` and `mpvPlayer.isStopped()`.
- Qt will load the qml plugins automatically if you have installed them into their correct locations, you don't need to load them manually.

For more information, please refer to [*MpvPlayer.qml*](/MpvPlayer.qml) and [*mpvdeclarativeobject.h*](/mpvdeclarativeobject.h).

## Compilation
1. Checkout source code:
   ```bash
   git clone https://github.com/wangwenx190/libmpv-qtquick-plugin.git
   ```
   Note: Please remember to install *Git* yourself. Windows users can download it from: https://git-scm.com/downloads
2. Setup libmpv SDK:

   For Linux developers, you just need to install `libmpv-dev` (or something like that, depending on your Linux distro). No more things to do. It's that easy.

   However, if you are using Windows, things are a little different. You can download *shinchiro*'s package from https://sourceforge.net/projects/mpv-player-windows/files/libmpv/ , the **mpv.lib** needed by MSVC should be generated manually, you can refer to https://github.com/mpv-player/mpv/blob/master/DOCS/compile-windows.md#linking-libmpv-with-msvc-programs for more information. Once everything is ready, then write the following things to a text file named **user.conf** and save it to this repository's directory:
   ```text
   # You should replace the "D:/code/mpv-sdk" with your own path.
   # Better to use "/" instead of "\", even on Windows platform.
   isEmpty(MPV_SDK_DIR): MPV_SDK_DIR = D:/code/mpv-sdk
   ```
3. Create a directory for building:

   Linux:
   ```bash
   mkdir build
   cd build
   ```
   Windows (cmd):
   ```bat
   md build
   cd build
   ```
   Windows (PowerShell):
   ```ps
   New-Item -Path "build" -ItemType "Directory"
   Set-Location -Path "build"
   ```
4. Build and Install:

   Linux:
   ```bash
   qmake
   make
   make install
   ```
   Windows:
   ```bat
   jom qmake_all
   jom
   jom install
   ```
   Note: Replace "jom" with "nmake" if you don't have *JOM*. Qt's official website to download *JOM*: http://download.qt.io/official_releases/jom/

## FAQ
1. Why another window appears instead of rendering in my own application?

   Because the C++ backend class is quite huge, the initialization of it is kind of slow. Please make sure `MpvPlayer` is completely initialized before using it to play any media contents.
2. Why is the CPU usage very high (>=10%) ?

   You need to enable **hardware decoding** manually:
   ```qml
   import wangwenx190.QuickMpv 1.0

   MpvPlayer {
       // ...
       hwdec: "auto" // type: string
       // ...
   }
   ```
3. How to set the log level of libmpv?

    ```qml
   import wangwenx190.QuickMpv 1.0

   MpvPlayer {
       // ...
       logLevel: MpvObject.DebugLevel // type: enum
       // ...
   }
   ```
   Note: For more log levels, please refer to [*mpvdeclarativeobject.h*](/mpvdeclarativeobject.h).
4. Why Qt says failed to create EGL context ... etc ?

   ANGLE only supports OpenGL version <= 3.1

   Desktop OpenGL doesn't have this limit.

## License
[GNU General Public License version 3](/LICENSE.md)
