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
import wangwenx190.QuickMpv 1.0

MpvPlayer {
    id: mpvPlayer

    hrSeek: true
    loadScripts: true
    ytdl: true
    screenshotFormat: "png"
    logLevel: MpvPlayer.DebugLevel
    volume: 85

    onPositionChanged: // do something
    onDurationChanged: // do something
    onVideoSizeChanged: // do something
    onPlaybackStateChanged: // do something
}
```
For more information, please refer to [*mpvplayer.h*](/mpvplayer.h).

Note: Qt will load the qml plugins automatically if you have installed them into their correct locations, you don't need to load them manually.

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
   Note: For more log levels, please refer to [*mpvplayer.h*](/mpvplayer.h).
4. Why Qt says failed to create EGL context ... etc ?

   ANGLE only supports OpenGL version <= 3.1

   Desktop OpenGL doesn't have this limit.

## License
[GNU General Public License version 3](/LICENSE.md)
