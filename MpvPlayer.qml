import QtQuick 2.13
import wangwenx190.QuickMpv 1.0

Item {
    id: mpvPlayer

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

    signal paused
    signal stopped
    signal playing

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
    function seek(pos) {
        mpvObject.seek(pos);
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
    }
}
