# libmpv-qtquick-plugin
libmpv wrapper for Qt Quick. Can be used as a visual element in .qml files directly.

## Features
- Can be easily used in Qt Quick applications.
- Supports almost every property that libmpv supports.

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
    logLevel: MpvPlayer.DebugLevel
    volume: 85 // 0-100

    onPositionChanged: // do something
    onDurationChanged: // do something
    onVideoSizeChanged: // do something
    onPlaybackStateChanged: // do something
    onMediaStatusChanged: // do something
}
```
Notes
- `duration`, `position` and `seek` use **SECONDS** instead of milliseconds.
- `seek` uses absolute position, not relative offset.
- You can use `mpvPlayer.open(url)` to load and play *url* directly, they are equivalent to `mpvPlayer.source = url` + `mpvPlayer.play()`.
- You can also use `mpvPlayer.play()` to resume a paused playback, `mpvPlayer.pause()` to pause a playing playback, `mpvPlayer.stop()` to stop a loaded playback and `mpvPlayer.seek()` to jump to a different position.
- To get the current playback state, use `mpvPlayer.isPlaying()`, `mpvPlayer.isPaused()` and `mpvPlayer.isStopped()`.
- Qt will load the qml plugins automatically if you have installed them into their correct locations, you don't need to load them manually.

For more information, please refer to [*MpvPlayer.qml*](/MpvPlayer.qml) and [*mpvdeclarativeobject.h*](/mpvdeclarativeobject.h).

## Compilation
1. Checkout source code:
   ```bash
   git clone https://github.com/wangwenx190/libmpv-qtquick-plugin.git
   ```
   Note: Please remember to install *Git* yourself.
2. Create a directory for building:

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
2. Build and Install:

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
   Note: Replace "jom" with "nmake" if you don't have *JOM*.

## FAQ
1. Why another window appears instead of rendering in my own application?

   Because the `MpvPlayer` class is quite huge, the initialization of it is kind of slow. Please make sure it is completely initialized before using it to play media contents.
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
       logLevel: MpvPlayer.DebugLevel // type: enum
       // ...
   }
   ```
   Note: For more log levels, please refer to [*mpvdeclarativeobject.h*](/mpvdeclarativeobject.h).
4. Why Qt says failed to create EGL context ... etc ?

   ANGLE only supports OpenGL version <= 3.1

   Desktop OpenGL doesn't have this limit.

## License
[GNU General Public License version 3](/LICENSE.md)
