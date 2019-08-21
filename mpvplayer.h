#pragma once

#ifndef _MPVPLAYER_H
#define _MPVPLAYER_H

// Don't use any deprecated APIs from MPV.
#ifndef MPV_ENABLE_DEPRECATED
#define MPV_ENABLE_DEPRECATED 0
#endif

#include "mpvqthelper.hpp"
#include <QHash>
#include <QList>
#include <QQuickFramebufferObject>
#include <QUrl>
#include <QVariant>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvRenderer;

class MpvPlayer : public QQuickFramebufferObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MpvPlayer)

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(
        qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(MpvPlayer::PlaybackState playbackState READ playbackState WRITE
                   setPlaybackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MpvPlayer::MediaStatus mediaStatus READ mediaStatus NOTIFY
                   mediaStatusChanged)
    Q_PROPERTY(MpvPlayer::LogLevel logLevel READ logLevel WRITE setLogLevel
                   NOTIFY logLevelChanged)
    Q_PROPERTY(QString hwdec READ hwdec WRITE setHwdec NOTIFY hwdecChanged)
    Q_PROPERTY(QString mpvVersion READ mpvVersion)
    Q_PROPERTY(QString mpvConfiguration READ mpvConfiguration)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion)
    Q_PROPERTY(QString qtVersion READ qtVersion)
    Q_PROPERTY(int vid READ vid WRITE setVid NOTIFY vidChanged)
    Q_PROPERTY(int aid READ aid WRITE setAid NOTIFY aidChanged)
    Q_PROPERTY(int sid READ sid WRITE setSid NOTIFY sidChanged)
    Q_PROPERTY(int videoRotate READ videoRotate WRITE setVideoRotate NOTIFY
                   videoRotateChanged)
    Q_PROPERTY(qreal videoAspect READ videoAspect WRITE setVideoAspect NOTIFY
                   videoAspectChanged)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(bool deinterlace READ deinterlace WRITE setDeinterlace NOTIFY
                   deinterlaceChanged)
    Q_PROPERTY(bool audioExclusive READ audioExclusive WRITE setAudioExclusive
                   NOTIFY audioExclusiveChanged)
    Q_PROPERTY(QString audioFileAuto READ audioFileAuto WRITE setAudioFileAuto
                   NOTIFY audioFileAutoChanged)
    Q_PROPERTY(
        QString subAuto READ subAuto WRITE setSubAuto NOTIFY subAutoChanged)
    Q_PROPERTY(QString subCodepage READ subCodepage WRITE setSubCodepage NOTIFY
                   subCodepageChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)
    Q_PROPERTY(QString vo READ vo WRITE setVo NOTIFY voChanged)
    Q_PROPERTY(QString ao READ ao WRITE setAo NOTIFY aoChanged)
    Q_PROPERTY(QString screenshotFormat READ screenshotFormat WRITE
                   setScreenshotFormat NOTIFY screenshotFormatChanged)
    Q_PROPERTY(
        int screenshotPngCompression READ screenshotPngCompression WRITE
            setScreenshotPngCompression NOTIFY screenshotPngCompressionChanged)
    Q_PROPERTY(QString screenshotTemplate READ screenshotTemplate WRITE
                   setScreenshotTemplate NOTIFY screenshotTemplateChanged)
    Q_PROPERTY(QString screenshotDirectory READ screenshotDirectory WRITE
                   setScreenshotDirectory NOTIFY screenshotDirectoryChanged)
    Q_PROPERTY(
        QString profile READ profile WRITE setProfile NOTIFY profileChanged)
    Q_PROPERTY(bool hrSeek READ hrSeek WRITE setHrSeek NOTIFY hrSeekChanged)
    Q_PROPERTY(bool ytdl READ ytdl WRITE setYtdl NOTIFY ytdlChanged)
    Q_PROPERTY(bool loadScripts READ loadScripts WRITE setLoadScripts NOTIFY
                   loadScriptsChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(QString fileFormat READ fileFormat NOTIFY fileFormatChanged)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(qreal videoBitrate READ videoBitrate NOTIFY videoBitrateChanged)
    Q_PROPERTY(qreal audioBitrate READ audioBitrate NOTIFY audioBitrateChanged)
    Q_PROPERTY(MpvPlayer::AudioDevices audioDeviceList READ audioDeviceList
                   NOTIFY audioDeviceListChanged)
    Q_PROPERTY(
        bool screenshotTagColorspace READ screenshotTagColorspace WRITE
            setScreenshotTagColorspace NOTIFY screenshotTagColorspaceChanged)
    Q_PROPERTY(int screenshotJpegQuality READ screenshotJpegQuality WRITE
                   setScreenshotJpegQuality NOTIFY screenshotJpegQualityChanged)
    Q_PROPERTY(QString videoFormat READ videoFormat NOTIFY videoFormatChanged)
    Q_PROPERTY(MpvPlayer::MpvCallType mpvCallType READ mpvCallType WRITE
                   setMpvCallType NOTIFY mpvCallTypeChanged)
    Q_PROPERTY(MpvPlayer::MediaTracks mediaTracks READ mediaTracks NOTIFY
                   mediaTracksChanged)
    Q_PROPERTY(QStringList videoSuffixes READ videoSuffixes)
    Q_PROPERTY(QStringList audioSuffixes READ audioSuffixes)
    Q_PROPERTY(QStringList subtitleSuffixes READ subtitleSuffixes)
    Q_PROPERTY(
        MpvPlayer::Chapters chapters READ chapters NOTIFY chaptersChanged)

    friend class MpvRenderer;

public:
    enum PlaybackState { StoppedState, PlayingState, PausedState };
    Q_ENUM(PlaybackState)

    enum MediaStatus {
        UnknownMediaStatus,
        NoMedia,
        LoadingMedia,
        LoadedMedia,
        StalledMedia,
        BufferingMedia,
        BufferedMedia,
        EndOfMedia,
        InvalidMedia
    };
    Q_ENUM(MediaStatus)

    enum LogLevel {
        NoLog,
        DebugLevel,
        WarningLevel,
        CriticalLevel,
        FatalLevel,
        InfoLevel
    };
    Q_ENUM(LogLevel)

    enum MpvCallType { SynchronousCall, AsynchronousCall };
    Q_ENUM(MpvCallType)

    using SingleTrackInfo = QHash<QString, QVariant>;

    struct MediaTracks {
        int count;
        QList<SingleTrackInfo> videoChannels;
        QList<SingleTrackInfo> audioTracks;
        QList<SingleTrackInfo> subtitleStreams;
    };

    struct Chapters {
        int count;
        QList<SingleTrackInfo> chapters;
    };

    struct AudioDevices {
        int count;
        QList<SingleTrackInfo> devices;
    };

    explicit MpvPlayer(QQuickItem *parent = nullptr);
    ~MpvPlayer() override;

    static void on_update(void *ctx);
    Renderer *createRenderer() const override;

    // Current media's source in QUrl.
    QUrl source() const;
    // Currently played file, with path stripped. If this is an URL, try to undo
    // percent encoding as well. (The result is not necessarily correct, but
    // looks better for display purposes. Use the path property to get an
    // unmodified filename)
    QString fileName() const;
    // Video's picture size
    // video-out-params/dw = video-params/dw = dwidth
    // video-out-params/dh = video-params/dh = dheight
    QSize videoSize() const;
    // Playback state
    MpvPlayer::PlaybackState playbackState() const;
    // Media status
    MpvPlayer::MediaStatus mediaStatus() const;
    // Control verbosity directly for each module. The all module changes the
    // verbosity of all the modules. The verbosity changes from this option are
    // applied in order from left to right, and each item can override a
    // previous one.
    // --msg-level=<module1=level1,module2=level2,...>
    // Available levels: no, fatal, error, warn, info, status (default), v
    // (verbose messages), debug, trace (print all messages produced by mpv)
    MpvPlayer::LogLevel logLevel() const;
    // Duration of the current file in **SECONDS**, not milliseconds.
    qint64 duration() const;
    // Position in current file in **SECONDS**, not milliseconds.
    qint64 position() const;
    // Set the startup volume: --volume=<0-100>
    int volume() const;
    // Set startup audio mute status (default: no): --mute=<yes|no>
    bool mute() const;
    // Return whether it's generally possible to seek in the current file.
    bool seekable() const;
    // If the currently played file has a title tag, use that.
    // Otherwise, if the media type is DVD, return the volume ID of DVD.
    // Otherwise, return the filename property.
    QString mediaTitle() const;
    // Hardware video decoding API: --hwdec=<api>
    // <api> can be: no, auto, vdpau, vaapi, videotoolbox, dxva2, d3d11va,
    // mediacodec, mmal, cuda, nvdec, crystalhd, rkmpp
    QString hwdec() const;
    // The mpv version/copyright string
    QString mpvVersion() const;
    // The configuration arguments which were passed to the mpv build system
    QString mpvConfiguration() const;
    // The contents of the av_version_info() API call
    QString ffmpegVersion() const;
    // Qt version at run-time
    QString qtVersion() const;
    // Video channel: --vid=<ID|auto|no>
    int vid() const;
    // Audio track: --aid=<ID|auto|no>
    int aid() const;
    // Subtitle stream: --sid=<ID|auto|no>
    int sid() const;
    // Rotate the video clockwise, in degrees: --video-rotate=<0-359|no>
    // Use "video-out-params/rotate" to query this variable
    // video-out-params/rotate = video-params/rotate = video-rotate
    int videoRotate() const;
    // Video aspect ratio: --video-aspect=<ratio|no>
    // Eg: --video-aspect=4:3 or --video-aspect=1.3333
    // Eg: --video-aspect=16:9 or --video-aspect=1.7777
    // Use "video-out-params/aspect" to query this variable.
    // video-out-params/aspect = video-params/aspect = video-aspect
    qreal videoAspect() const;
    // Playback speed: --speed=<0.01-100>
    qreal speed() const;
    // Enable or disable interlacing (default: no): --deinterlace=<yes|no>
    bool deinterlace() const;
    // Enable exclusive output mode. In this mode, the system is usually locked
    // out, and only mpv will be able to output audio.
    // --audio-exclusive=<yes|no>
    bool audioExclusive() const;
    // Load additional audio files matching the video filename. The parameter
    // specifies how external audio files are matched.
    // --audio-file-auto=<no|exact|fuzzy|all>
    // no: Don't automatically load external audio files (default)
    // exact: Load the media filename with audio file extension
    // fuzzy: Load all audio files containing media filename
    // all: Load all audio files in the current and --audio-file-paths
    // directories
    QString audioFileAuto() const;
    // Load additional subtitle files matching the video filename. The parameter
    // specifies how external subtitle files are matched. exact is enabled by
    // default.
    // --sub-auto=<no|exact|fuzzy|all>
    // no, exact, fuzzy, all: same as --audio-file-auto.
    QString subAuto() const;
    // Subtitle codepage (default: auto): --sub-codepage=<codepage>
    // Eg: --sub-codepage=latin2
    // Eg: --sub-codepage=+cp1250
    QString subCodepage() const;
    // Specify a priority list of video output drivers to be used.
    // --vo=<driver1,driver2,...[,]>
    // drivers: xv, x11, vdpau, direct3d, gpu, sdl, vaapi, null, caca, tct,
    // image, libmpv, rpi, drm, mediacodec_embed
    QString vo() const;
    // Specify a priority list of audio output drivers to be used.
    // --ao=<driver1,driver2,...[,]>
    // drivers: alsa, oss, jack, coreaudio, coreaudio_exclusive, openal, pulse,
    // sdl, null, pcm, rsound, sndio, wasapi
    QString ao() const;
    // Set the image file type used for saving screenshots.
    // --screenshot-format=<png|jpg>
    QString screenshotFormat() const;
    // Tag screenshots with the appropriate colorspace.
    // --screenshot-tag-colorspace=<yes|no>
    bool screenshotTagColorspace() const;
    // --screenshot-png-compression=<0-9>
    // Set the PNG compression level. Higher means better compression. This will
    // affect the file size of the written screenshot file and the time it takes
    // to write a screenshot. Too high compression might occupy enough CPU time
    // to interrupt playback. The default is 7.
    int screenshotPngCompression() const;
    // --screenshot-jpeg-quality=<0-100>
    // Set the JPEG quality level. Higher means better quality. The default
    // is 90.
    int screenshotJpegQuality() const;
    // --screenshot-template=<template>
    // %F: Filename of the currently played video but strip the file extension,
    // including the dot.
    // %x: Directory path of the currently played video (empty if not on local
    // file system, eg: http://).
    // %p: Current playback time, in the same format as used in the OSD. The
    // result is a string of the form "HH:MM:SS".
    QString screenshotTemplate() const;
    // --screenshot-directory=<path>
    // Store screenshots in this directory. If the template filename is already
    // absolute, the directory is ignored. If the directory does not exist, it
    // is created on the first screenshot. If it is not a directory, an error is
    // generated when trying to write a screenshot.
    QString screenshotDirectory() const;
    // Preset configurations
    // You can apply profiles on start with the --profile=<name> option, or at
    // runtime with the apply-profile <name> command.
    QString profile() const;
    // --hr-seek=<no|absolute|yes>
    // Select when to use precise seeks that are not limited to keyframes
    // no: Never use precise seeks;
    // yes: Use precise seeks whenever possible;
    // absolute: Use precise seeks if the seek is to an absolute position in the
    // file, such as a chapter seek, but not for relative seeks like the default
    // behavior of arrow keys (default).
    bool hrSeek() const;
    // --ytdl=<yes|no>
    // Enable the youtube-dl hook-script. It will look at the input URL, and
    // will play the video located on the website
    bool ytdl() const;
    // --load-scripts=<yes|no>
    // Auto-load scripts from the scripts configuration subdirectory (usually
    // ~/.config/mpv/scripts/)
    bool loadScripts() const;
    // Full path of the currently played file. Usually this is exactly the same
    // string you pass on the mpv command line or the loadfile command, even if
    // it's a relative path. If you expect an absolute path, you will have to
    // determine it yourself
    QString path() const;
    // Symbolic name of the file format. In some cases, this is a
    // comma-separated list of format names, e.g. mp4 is mov,mp4,m4a,3gp,3g2,mj2
    // (the list may grow in the future for any format).
    QString fileFormat() const;
    // Length in bytes of the source file/stream.
    qint64 fileSize() const;
    // Video bitrate
    qreal videoBitrate() const;
    // Audio bitrate
    qreal audioBitrate() const;
    // Return the list of discovered audio devices.
    MpvPlayer::AudioDevices audioDeviceList() const;
    // Video format as string.
    QString videoFormat() const;
    // The call type of mpv client APIs.
    MpvPlayer::MpvCallType mpvCallType() const;
    // Video, audio and subtitle tracks.
    MpvPlayer::MediaTracks mediaTracks() const;
    // File types supported by mpv:
    // https://github.com/mpv-player/mpv/blob/master/player/external_files.c
    QStringList videoSuffixes() const {
        return QStringList()
            << QLatin1String("*.3g2") << QLatin1String("*.3ga")
            << QLatin1String("*.3gp") << QLatin1String("*.3gp2")
            << QLatin1String("*.3gpp") << QLatin1String("*.amv")
            << QLatin1String("*.asf") << QLatin1String("*.asx")
            << QLatin1String("*.avf") << QLatin1String("*.avi")
            << QLatin1String("*.bdm") << QLatin1String("*.bdmv")
            << QLatin1String("*.bik") << QLatin1String("*.clpi")
            << QLatin1String("*.cpi") << QLatin1String("*.dat")
            << QLatin1String("*.divx") << QLatin1String("*.drc")
            << QLatin1String("*.dv") << QLatin1String("*.dvr-ms")
            << QLatin1String("*.f4v") << QLatin1String("*.flv")
            << QLatin1String("*.gvi") << QLatin1String("*.gxf")
            << QLatin1String("*.hdmov") << QLatin1String("*.hlv")
            << QLatin1String("*.iso") << QLatin1String("*.letv")
            << QLatin1String("*.lrv") << QLatin1String("*.m1v")
            << QLatin1String("*.m2p") << QLatin1String("*.m2t")
            << QLatin1String("*.m2ts") << QLatin1String("*.m2v")
            << QLatin1String("*.m3u") << QLatin1String("*.m3u8")
            << QLatin1String("*.m4v") << QLatin1String("*.mkv")
            << QLatin1String("*.moov") << QLatin1String("*.mov")
            << QLatin1String("*.mp2") << QLatin1String("*.mp2v")
            << QLatin1String("*.mp4") << QLatin1String("*.mp4v")
            << QLatin1String("*.mpe") << QLatin1String("*.mpeg")
            << QLatin1String("*.mpeg1") << QLatin1String("*.mpeg2")
            << QLatin1String("*.mpeg4") << QLatin1String("*.mpg")
            << QLatin1String("*.mpl") << QLatin1String("*.mpls")
            << QLatin1String("*.mpv") << QLatin1String("*.mpv2")
            << QLatin1String("*.mqv") << QLatin1String("*.mts")
            << QLatin1String("*.mtv") << QLatin1String("*.mxf")
            << QLatin1String("*.mxg") << QLatin1String("*.nsv")
            << QLatin1String("*.nuv") << QLatin1String("*.ogm")
            << QLatin1String("*.ogv") << QLatin1String("*.ogx")
            << QLatin1String("*.ps") << QLatin1String("*.qt")
            << QLatin1String("*.qtvr") << QLatin1String("*.ram")
            << QLatin1String("*.rec") << QLatin1String("*.rm")
            << QLatin1String("*.rmj") << QLatin1String("*.rmm")
            << QLatin1String("*.rms") << QLatin1String("*.rmvb")
            << QLatin1String("*.rmx") << QLatin1String("*.rp")
            << QLatin1String("*.rpl") << QLatin1String("*.rv")
            << QLatin1String("*.rvx") << QLatin1String("*.thp")
            << QLatin1String("*.tod") << QLatin1String("*.tp")
            << QLatin1String("*.trp") << QLatin1String("*.ts")
            << QLatin1String("*.tts") << QLatin1String("*.txd")
            << QLatin1String("*.vcd") << QLatin1String("*.vdr")
            << QLatin1String("*.vob") << QLatin1String("*.vp8")
            << QLatin1String("*.vro") << QLatin1String("*.webm")
            << QLatin1String("*.wm") << QLatin1String("*.wmv")
            << QLatin1String("*.wtv") << QLatin1String("*.xesc")
            << QLatin1String("*.xspf");
    }
    QStringList audioSuffixes() const {
        return QStringList()
            << QLatin1String("*.mp3") << QLatin1String("*.aac")
            << QLatin1String("*.mka") << QLatin1String("*.dts")
            << QLatin1String("*.flac") << QLatin1String("*.ogg")
            << QLatin1String("*.m4a") << QLatin1String("*.ac3")
            << QLatin1String("*.opus") << QLatin1String("*.wav")
            << QLatin1String("*.wv");
    }
    QStringList subtitleSuffixes() const {
        return QStringList()
            << QLatin1String("*.utf") << QLatin1String("*.utf8")
            << QLatin1String("*.utf-8") << QLatin1String("*.idx")
            << QLatin1String("*.sub") << QLatin1String("*.srt")
            << QLatin1String("*.rt") << QLatin1String("*.ssa")
            << QLatin1String("*.ass") << QLatin1String("*.mks")
            << QLatin1String("*.vtt") << QLatin1String("*.sup")
            << QLatin1String("*.scc") << QLatin1String("*.smi");
    }
    // Chapter list
    MpvPlayer::Chapters chapters() const;

public slots:
    void open(const QUrl &url);
    void play();
    void play(const QUrl &url);
    void pause();
    void stop();
    // Seek: use absolute position.
    void seek(qint64 position);
    void setSource(const QUrl &source);
    void setMute(bool mute);
    void setPlaybackState(MpvPlayer::PlaybackState playbackState);
    void setLogLevel(MpvPlayer::LogLevel logLevel);
    void setPosition(qint64 position);
    void setVolume(int volume);
    void setHwdec(const QString &hwdec);
    void setVid(int vid);
    void setAid(int aid);
    void setSid(int sid);
    void setVideoRotate(int videoRotate);
    void setVideoAspect(qreal videoAspect);
    void setSpeed(qreal speed);
    void setDeinterlace(bool deinterlace);
    void setAudioExclusive(bool audioExclusive);
    void setAudioFileAuto(const QString &audioFileAuto);
    void setSubAuto(const QString &subAuto);
    void setSubCodepage(const QString &subCodepage);
    void setVo(const QString &vo);
    void setAo(const QString &ao);
    void setScreenshotFormat(const QString &screenshotFormat);
    void setScreenshotPngCompression(int screenshotPngCompression);
    void setScreenshotTemplate(const QString &screenshotTemplate);
    void setScreenshotDirectory(const QString &screenshotDirectory);
    void setProfile(const QString &profile);
    void setHrSeek(bool hrSeek);
    void setYtdl(bool ytdl);
    void setLoadScripts(bool loadScripts);
    void setScreenshotTagColorspace(bool screenshotTagColorspace);
    void setScreenshotJpegQuality(int screenshotJpegQuality);
    void setMpvCallType(MpvPlayer::MpvCallType mpvCallType);

protected slots:
    void handleMpvEvents();

private slots:
    void doUpdate();

    void mpvSendCommand(const QVariant &arguments);
    void mpvSetProperty(const QString &name, const QVariant &value);
    QVariant mpvGetProperty(const QString &name) const;
    void mpvObserveProperty(const QString &name);

private:
    void processMpvLogMessage(mpv_event_log_message *event);
    void processMpvPropertyChange(mpv_event_property *event);

    bool isLoaded() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void setMediaStatus(MpvPlayer::MediaStatus mediaStatus);

    // Should be called when MPV_EVENT_VIDEO_RECONFIG happens.
    // Never do anything expensive here.
    void videoReconfig();
    // Should be called when MPV_EVENT_AUDIO_RECONFIG happens.
    // Never do anything expensive here.
    void audioReconfig();

private:
    mpv::qt::Handle mpv;
    mpv_render_context *mpv_gl = nullptr;

    QUrl currentSource = QUrl();
    MpvPlayer::MediaStatus currentMediaStatus = MpvPlayer::MediaStatus::NoMedia;
    MpvPlayer::MpvCallType currentMpvCallType =
        MpvPlayer::MpvCallType::SynchronousCall;

    const QHash<QString, QString> properties = {
        {QLatin1String("dwidth"), QLatin1String("videoSizeChanged")},
        {QLatin1String("dheight"), QLatin1String("videoSizeChanged")},
        {QLatin1String("duration"), QLatin1String("durationChanged")},
        {QLatin1String("time-pos"), QLatin1String("positionChanged")},
        {QLatin1String("volume"), QLatin1String("volumeChanged")},
        {QLatin1String("mute"), QLatin1String("muteChanged")},
        {QLatin1String("seekable"), QLatin1String("seekableChanged")},
        {QLatin1String("hwdec-current"), QLatin1String("hwdecChanged")},
        {QLatin1String("vid"), QLatin1String("vidChanged")},
        {QLatin1String("aid"), QLatin1String("aidChanged")},
        {QLatin1String("sid"), QLatin1String("sidChanged")},
        {QLatin1String("video-rotate"), QLatin1String("videoRotateChanged")},
        {QLatin1String("video-aspect"), QLatin1String("videoAspectChanged")},
        {QLatin1String("speed"), QLatin1String("speedChanged")},
        {QLatin1String("deinterlace"), QLatin1String("deinterlaceChanged")},
        {QLatin1String("audio-exclusive"),
         QLatin1String("audioExclusiveChanged")},
        {QLatin1String("audio-file-auto"),
         QLatin1String("audioFileAutoChanged")},
        {QLatin1String("sub-auto"), QLatin1String("subAutoChanged")},
        {QLatin1String("sub-codepage"), QLatin1String("subCodepageChanged")},
        {QLatin1String("filename"), QLatin1String("fileNameChanged")},
        {QLatin1String("media-title"), QLatin1String("mediaTitleChanged")},
        {QLatin1String("vo"), QLatin1String("voChanged")},
        {QLatin1String("ao"), QLatin1String("aoChanged")},
        {QLatin1String("screenshot-format"),
         QLatin1String("screenshotFormatChanged")},
        {QLatin1String("screenshot-png-compression"),
         QLatin1String("screenshotPngCompressionChanged")},
        {QLatin1String("screenshot-template"),
         QLatin1String("screenshotTemplateChanged")},
        {QLatin1String("screenshot-directory"),
         QLatin1String("screenshotDirectoryChanged")},
        {QLatin1String("profile"), QLatin1String("profileChanged")},
        {QLatin1String("hr-seek"), QLatin1String("hrSeekChanged")},
        {QLatin1String("ytdl"), QLatin1String("ytdlChanged")},
        {QLatin1String("load-scripts"), QLatin1String("loadScriptsChanged")},
        {QLatin1String("path"), QLatin1String("pathChanged")},
        {QLatin1String("file-format"), QLatin1String("fileFormatChanged")},
        {QLatin1String("file-size"), QLatin1String("fileSizeChanged")},
        //{QLatin1String("video-bitrate"),
        // QLatin1String("videoBitrateChanged")},
        //{QLatin1String("audio-bitrate"),
        // QLatin1String("audioBitrateChanged")},
        {QLatin1String("audio-device-list"),
         QLatin1String("audioDeviceListChanged")},
        {QLatin1String("screenshot-tag-colorspace"),
         QLatin1String("screenshotTagColorspaceChanged")},
        {QLatin1String("screenshot-jpeg-quality"),
         QLatin1String("screenshotJpegQualityChanged")},
        {QLatin1String("video-format"), QLatin1String("videoFormatChanged")},
        {QLatin1String("pause"), QLatin1String("playbackStateChanged")},
        {QLatin1String("idle-active"), QLatin1String("playbackStateChanged")},
        {QLatin1String("track-list"), QLatin1String("mediaTracksChanged")},
        {QLatin1String("chapter-list"), QLatin1String("chaptersChanged")}};

signals:
    void onUpdate();
    void hasMpvEvents();

    void sourceChanged();
    void videoSizeChanged();
    void playbackStateChanged();
    void mediaStatusChanged();
    void logLevelChanged();
    void durationChanged();
    void positionChanged();
    void volumeChanged();
    void muteChanged();
    void seekableChanged();
    void hwdecChanged();
    void vidChanged();
    void aidChanged();
    void sidChanged();
    void videoRotateChanged();
    void videoAspectChanged();
    void speedChanged();
    void deinterlaceChanged();
    void audioExclusiveChanged();
    void audioFileAutoChanged();
    void subAutoChanged();
    void subCodepageChanged();
    void fileNameChanged();
    void mediaTitleChanged();
    void voChanged();
    void aoChanged();
    void screenshotFormatChanged();
    void screenshotPngCompressionChanged();
    void screenshotTemplateChanged();
    void screenshotDirectoryChanged();
    void profileChanged();
    void hrSeekChanged();
    void ytdlChanged();
    void loadScriptsChanged();
    void pathChanged();
    void fileFormatChanged();
    void fileSizeChanged();
    void videoBitrateChanged();
    void audioBitrateChanged();
    void audioDeviceListChanged();
    void screenshotTagColorspaceChanged();
    void screenshotJpegQualityChanged();
    void videoFormatChanged();
    void mpvCallTypeChanged();
    void mediaTracksChanged();
    void chaptersChanged();
};

Q_DECLARE_METATYPE(MpvPlayer::PlaybackState)
Q_DECLARE_METATYPE(MpvPlayer::MediaStatus)
Q_DECLARE_METATYPE(MpvPlayer::LogLevel)
Q_DECLARE_METATYPE(MpvPlayer::MpvCallType)
Q_DECLARE_METATYPE(MpvPlayer::MediaTracks)
Q_DECLARE_METATYPE(MpvPlayer::Chapters)
Q_DECLARE_METATYPE(MpvPlayer::AudioDevices)

#endif
