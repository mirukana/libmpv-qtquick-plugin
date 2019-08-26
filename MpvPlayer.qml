import QtQuick 2.13
import wangwenx190.QuickMpv 1.0

/*!
    \qmltype MpvPlayer
    \inherits Item
    \brief A convenience type for playing a specified media content.

    \c MpvPlayer is a libmpv wrapper for Qt Quick. It can be embeded
    into any Qt Quick GUI applications easily.

    \qml
    MpvPlayer {
        id: mpvPlayer

        width : 800
        height : 600

        source: "video.avi"

        MouseArea {
            anchors.fill: parent
            onClicked: mpvPlayer.play()
        }

        focus: true
        Keys.onSpacePressed: mpvPlayer.isPlaying() ? mpvPlayer.pause() : mpvPlayer.play()
        Keys.onLeftPressed: mpvPlayer.seek(-5)
        Keys.onRightPressed: mpvPlayer.seek(5)
    }
    \endqml

    \c MpvPlayer supports almost every property that libmpv supports.
*/

Item {
    id: mpvPlayer

    /*!
        \qmlproperty url MpvPlayer::source

        This property holds the source URL of the media.

        Playback will start immediately once the \l source property changed.
    */
    property alias source: mpvObject.source
    property alias videoSize: mpvObject.videoSize
    property alias duration: mpvObject.duration
    property alias position: mpvObject.position
    property alias volume: mpvObject.volume
    property alias mute: mpvObject.mute
    property alias seekable: mpvObject.seekable
    property alias playbackState: mpvObject.playbackState
    property alias mediaStatus: mpvObject.mediaStatus
    property alias logLevel: mpvObject.logLevel
    property alias hwdec: mpvObject.hwdec
    property alias mpvVersion: mpvObject.mpvVersion
    property alias mpvConfiguration: mpvObject.mpvConfiguration
    property alias ffmpegVersion: mpvObject.ffmpegVersion
    property alias qtVersion: mpvObject.qtVersion
    property alias vid: mpvObject.vid
    property alias aid: mpvObject.aid
    property alias sid: mpvObject.sid
    property alias videoRotate: mpvObject.videoRotate
    property alias videoAspect: mpvObject.videoAspect
    property alias speed: mpvObject.speed
    property alias deinterlace: mpvObject.deinterlace
    property alias audioExclusive: mpvObject.audioExclusive
    property alias audioFileAuto: mpvObject.audioFileAuto
    property alias subAuto: mpvObject.subAuto
    property alias subCodepage: mpvObject.subCodepage
    property alias fileName: mpvObject.fileName
    property alias mediaTitle: mpvObject.mediaTitle
    property alias vo: mpvObject.vo
    property alias ao: mpvObject.ao
    property alias screenshotFormat: mpvObject.screenshotFormat
    property alias screenshotPngCompression: mpvObject.screenshotPngCompression
    property alias screenshotTemplate: mpvObject.screenshotTemplate
    property alias screenshotDirectory: mpvObject.screenshotDirectory
    property alias profile: mpvObject.profile
    property alias hrSeek: mpvObject.hrSeek
    property alias ytdl: mpvObject.ytdl
    property alias loadScripts: mpvObject.loadScripts
    property alias path: mpvObject.path
    property alias fileFormat: mpvObject.fileFormat
    property alias fileSize: mpvObject.fileSize
    property alias videoBitrate: mpvObject.videoBitrate
    property alias audioBitrate: mpvObject.audioBitrate
    property alias audioDeviceList: mpvObject.audioDeviceList
    property alias screenshotTagColorspace: mpvObject.screenshotTagColorspace
    property alias screenshotJpegQuality: mpvObject.screenshotJpegQuality
    property alias videoFormat: mpvObject.videoFormat
    property alias mpvCallType: mpvObject.mpvCallType
    property alias mediaTracks: mpvObject.mediaTracks
    property alias videoSuffixes: mpvObject.videoSuffixes
    property alias audioSuffixes: mpvObject.audioSuffixes
    property alias subtitleSuffixes: mpvObject.subtitleSuffixes
    property alias chapters: mpvObject.chapters
    property alias metadata: mpvObject.metadata
    property alias avsync: mpvObject.avsync
    property alias percentPos: mpvObject.percentPos
    property alias estimatedVfFps: mpvObject.estimatedVfFps

    signal initFinished
    signal playing
    signal paused
    signal stopped

    function open(url) {
        mpvObject.open(url);
    }
    function play() {
        mpvObject.play();
    }
    function pause() {
        mpvObject.pause();
    }
    function stop() {
        mpvObject.stop();
    }
    function seek(offset) {
        mpvObject.seek(offset);
    }
    function seekAbsolute(position) {
        mpvObject.seekAbsolute(position);
    }
    function seekRelative(offset) {
        mpvObject.seekRelative(offset);
    }
    function seekPercent(percent) {
        mpvObject.seekPercent(percent);
    }
    function screenshot() {
        mpvObject.screenshot();
    }
    function screenshotToFile(path) {
        mpvObject.screenshotToFile(path);
    }
    function isPlaying() {
        return mpvObject.playbackState === MpvObject.PlayingState;
    }
    function isPaused() {
        return mpvObject.playbackState === MpvObject.PausedState;
    }
    function isStopped() {
        return mpvObject.playbackState === MpvObject.StoppedState;
    }

    MpvObject {
        id: mpvObject
        anchors.fill: mpvPlayer
        onPlaybackStateChanged: {
            switch (mpvObject.playbackState) {
            case MpvObject.PlayingState:
                mpvPlayer.playing();
                break;
            case MpvObject.PausedState:
                mpvPlayer.paused();
                break;
            case MpvObject.StoppedState:
                mpvPlayer.stopped();
                break;
            }
        }
        onInitializationFinished: mpvPlayer.initFinished()
        Component.onCompleted: mpvObject.setInitializationState(false, false, true)
    }
}
