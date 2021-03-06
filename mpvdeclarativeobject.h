#pragma once

#ifndef _MPVDECLARATIVEOBJECT_H
#define _MPVDECLARATIVEOBJECT_H

// Don't use any deprecated APIs from MPV.
#ifndef MPV_ENABLE_DEPRECATED
#define MPV_ENABLE_DEPRECATED 0
#endif

#include "mpvqthelper.hpp"
#include <QHash>
#include <QQuickFramebufferObject>
#include <QUrl>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvRenderer;

class MpvDeclarativeObject : public QQuickFramebufferObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MpvDeclarativeObject)

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(
        qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(
        MpvDeclarativeObject::PlaybackState playbackState READ playbackState
            WRITE setPlaybackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MpvDeclarativeObject::MediaStatus mediaStatus READ mediaStatus
                   NOTIFY mediaStatusChanged)
    Q_PROPERTY(MpvDeclarativeObject::LogLevel logLevel READ logLevel WRITE
                   setLogLevel NOTIFY logLevelChanged)
    Q_PROPERTY(QString hwdec READ hwdec WRITE setHwdec NOTIFY hwdecChanged)
    Q_PROPERTY(QString mpvVersion READ mpvVersion CONSTANT)
    Q_PROPERTY(QString mpvConfiguration READ mpvConfiguration CONSTANT)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
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
    Q_PROPERTY(MpvDeclarativeObject::AudioDevices audioDeviceList READ
                   audioDeviceList NOTIFY audioDeviceListChanged)
    Q_PROPERTY(
        bool screenshotTagColorspace READ screenshotTagColorspace WRITE
            setScreenshotTagColorspace NOTIFY screenshotTagColorspaceChanged)
    Q_PROPERTY(int screenshotJpegQuality READ screenshotJpegQuality WRITE
                   setScreenshotJpegQuality NOTIFY screenshotJpegQualityChanged)
    Q_PROPERTY(QString videoFormat READ videoFormat NOTIFY videoFormatChanged)
    Q_PROPERTY(MpvDeclarativeObject::MpvCallType mpvCallType READ mpvCallType
                   WRITE setMpvCallType NOTIFY mpvCallTypeChanged)
    Q_PROPERTY(MpvDeclarativeObject::MediaTracks mediaTracks READ mediaTracks
                   NOTIFY mediaTracksChanged)
    Q_PROPERTY(QStringList videoSuffixes READ videoSuffixes CONSTANT)
    Q_PROPERTY(QStringList audioSuffixes READ audioSuffixes CONSTANT)
    Q_PROPERTY(QStringList subtitleSuffixes READ subtitleSuffixes CONSTANT)
    Q_PROPERTY(MpvDeclarativeObject::Chapters chapters READ chapters NOTIFY
                   chaptersChanged)
    Q_PROPERTY(MpvDeclarativeObject::Metadata metadata READ metadata NOTIFY
                   metadataChanged)
    Q_PROPERTY(qreal avsync READ avsync NOTIFY avsyncChanged)
    Q_PROPERTY(int percentPos READ percentPos WRITE setPercentPos NOTIFY
                   percentPosChanged)
    Q_PROPERTY(
        qreal estimatedVfFps READ estimatedVfFps NOTIFY estimatedVfFpsChanged)

    friend class MpvRenderer;

    using SingleTrackInfo = QHash<QString, QVariant>;

public:
    enum class PlaybackState { Stopped, Playing, Paused };
    Q_ENUM(PlaybackState)

    enum class MediaStatus {
        Unknown,
        NoMedia,
        Loading,
        Loaded,
        Stalled,
        Buffering,
        Buffered,
        End,
        Invalid
    };
    Q_ENUM(MediaStatus)

    enum class LogLevel { Off, Debug, Warning, Critical, Fatal, Info };
    Q_ENUM(LogLevel)

    enum class MpvCallType { Synchronous, Asynchronous };
    Q_ENUM(MpvCallType)

    struct MediaTracks {
        QList<SingleTrackInfo> videoChannels;
        QList<SingleTrackInfo> audioTracks;
        QList<SingleTrackInfo> subtitleStreams;
    };

    using Chapters = QList<SingleTrackInfo>;

    using AudioDevices = QList<SingleTrackInfo>;

    using Metadata = SingleTrackInfo;

    explicit MpvDeclarativeObject(QQuickItem *parent = nullptr);
    ~MpvDeclarativeObject() override;

    static void on_update(void *ctx);
    [[nodiscard]] Renderer *createRenderer() const override;

    // Current media's source in QUrl.
    [[nodiscard]] QUrl source() const;
    // Currently played file, with path stripped. If this is an URL, try to undo
    // percent encoding as well. (The result is not necessarily correct, but
    // looks better for display purposes. Use the path property to get an
    // unmodified filename)
    [[nodiscard]] QString fileName() const;
    // Video's picture size
    // video-out-params/dw = video-params/dw = dwidth
    // video-out-params/dh = video-params/dh = dheight
    [[nodiscard]] QSize videoSize() const;
    // Playback state
    [[nodiscard]] MpvDeclarativeObject::PlaybackState playbackState() const;
    // Media status
    [[nodiscard]] MpvDeclarativeObject::MediaStatus mediaStatus() const;
    // Control verbosity directly for each module. The all module changes the
    // verbosity of all the modules. The verbosity changes from this option are
    // applied in order from left to right, and each item can override a
    // previous one.
    // --msg-level=<module1=level1,module2=level2,...>
    // Available levels: no, fatal, error, warn, info, status (default), v
    // (verbose messages), debug, trace (print all messages produced by mpv)
    [[nodiscard]] MpvDeclarativeObject::LogLevel logLevel() const;
    // Duration of the current file in **SECONDS**, not milliseconds.
    [[nodiscard]] qint64 duration() const;
    // Position in current file in **SECONDS**, not milliseconds.
    [[nodiscard]] qint64 position() const;
    // Set the startup volume: --volume=<0-100>
    [[nodiscard]] int volume() const;
    // Set startup audio mute status (default: no): --mute=<yes|no>
    [[nodiscard]] bool mute() const;
    // Return whether it's generally possible to seek in the current file.
    [[nodiscard]] bool seekable() const;
    // If the currently played file has a title tag, use that.
    // Otherwise, if the media type is DVD, return the volume ID of DVD.
    // Otherwise, return the filename property.
    [[nodiscard]] QString mediaTitle() const;
    // Hardware video decoding API: --hwdec=<api>
    // <api> can be: no, auto, vdpau, vaapi, videotoolbox, dxva2, d3d11va,
    // mediacodec, mmal, cuda, nvdec, crystalhd, rkmpp
    [[nodiscard]] QString hwdec() const;
    // The mpv version/copyright string
    [[nodiscard]] QString mpvVersion() const;
    // The configuration arguments which were passed to the mpv build system
    [[nodiscard]] QString mpvConfiguration() const;
    // The contents of the av_version_info() API call
    [[nodiscard]] QString ffmpegVersion() const;
    // Qt version at run-time
    [[nodiscard]] QString qtVersion() const;
    // Video channel: --vid=<ID|auto|no>
    [[nodiscard]] int vid() const;
    // Audio track: --aid=<ID|auto|no>
    [[nodiscard]] int aid() const;
    // Subtitle stream: --sid=<ID|auto|no>
    [[nodiscard]] int sid() const;
    // Rotate the video clockwise, in degrees: --video-rotate=<0-359|no>
    // Use "video-out-params/rotate" to query this variable
    // video-out-params/rotate = video-params/rotate = video-rotate
    [[nodiscard]] int videoRotate() const;
    // Video aspect ratio: --video-aspect=<ratio|no>
    // Eg: --video-aspect=4:3 or --video-aspect=1.3333
    // Eg: --video-aspect=16:9 or --video-aspect=1.7777
    // Use "video-out-params/aspect" to query this variable.
    // video-out-params/aspect = video-params/aspect = video-aspect
    [[nodiscard]] qreal videoAspect() const;
    // Playback speed: --speed=<0.01-100>
    [[nodiscard]] qreal speed() const;
    // Enable or disable interlacing (default: no): --deinterlace=<yes|no>
    [[nodiscard]] bool deinterlace() const;
    // Enable exclusive output mode. In this mode, the system is usually locked
    // out, and only mpv will be able to output audio.
    // --audio-exclusive=<yes|no>
    [[nodiscard]] bool audioExclusive() const;
    // Load additional audio files matching the video filename. The parameter
    // specifies how external audio files are matched.
    // --audio-file-auto=<no|exact|fuzzy|all>
    // no: Don't automatically load external audio files (default)
    // exact: Load the media filename with audio file extension
    // fuzzy: Load all audio files containing media filename
    // all: Load all audio files in the current and --audio-file-paths
    // directories
    [[nodiscard]] QString audioFileAuto() const;
    // Load additional subtitle files matching the video filename. The parameter
    // specifies how external subtitle files are matched. exact is enabled by
    // default.
    // --sub-auto=<no|exact|fuzzy|all>
    // no, exact, fuzzy, all: same as --audio-file-auto.
    [[nodiscard]] QString subAuto() const;
    // Subtitle codepage (default: auto): --sub-codepage=<codepage>
    // Eg: --sub-codepage=latin2
    // Eg: --sub-codepage=+cp1250
    [[nodiscard]] QString subCodepage() const;
    // Specify a priority list of video output drivers to be used.
    // --vo=<driver1,driver2,...[,]>
    // drivers: xv, x11, vdpau, direct3d, gpu, sdl, vaapi, null, caca, tct,
    // image, libmpv, drm, mediacodec_embed
    [[nodiscard]] QString vo() const;
    // Specify a priority list of audio output drivers to be used.
    // --ao=<driver1,driver2,...[,]>
    // drivers: alsa, oss, jack, coreaudio, coreaudio_exclusive, openal, pulse,
    // sdl, null, pcm, rsound, sndio, wasapi
    [[nodiscard]] QString ao() const;
    // Set the image file type used for saving screenshots.
    // --screenshot-format=<png|jpg>
    [[nodiscard]] QString screenshotFormat() const;
    // Tag screenshots with the appropriate colorspace.
    // --screenshot-tag-colorspace=<yes|no>
    [[nodiscard]] bool screenshotTagColorspace() const;
    // --screenshot-png-compression=<0-9>
    // Set the PNG compression level. Higher means better compression. This will
    // affect the file size of the written screenshot file and the time it takes
    // to write a screenshot. Too high compression might occupy enough CPU time
    // to interrupt playback. The default is 7.
    [[nodiscard]] int screenshotPngCompression() const;
    // --screenshot-jpeg-quality=<0-100>
    // Set the JPEG quality level. Higher means better quality. The default
    // is 90.
    [[nodiscard]] int screenshotJpegQuality() const;
    // --screenshot-template=<template>
    // %F: Filename of the currently played video but strip the file extension,
    // including the dot.
    // %x: Directory path of the currently played video (empty if not on local
    // file system, eg: http://).
    // %p: Current playback time, in the same format as used in the OSD. The
    // result is a string of the form "HH:MM:SS".
    [[nodiscard]] QString screenshotTemplate() const;
    // --screenshot-directory=<path>
    // Store screenshots in this directory. If the template filename is already
    // absolute, the directory is ignored. If the directory does not exist, it
    // is created on the first screenshot. If it is not a directory, an error is
    // generated when trying to write a screenshot.
    [[nodiscard]] QString screenshotDirectory() const;
    // Preset configurations
    // You can apply profiles on start with the --profile=<name> option, or at
    // runtime with the apply-profile <name> command.
    [[nodiscard]] QString profile() const;
    // --hr-seek=<no|absolute|yes>
    // Select when to use precise seeks that are not limited to keyframes
    // no: Never use precise seeks;
    // yes: Use precise seeks whenever possible;
    // absolute: Use precise seeks if the seek is to an absolute position in the
    // file, such as a chapter seek, but not for relative seeks like the default
    // behavior of arrow keys (default).
    [[nodiscard]] bool hrSeek() const;
    // --ytdl=<yes|no>
    // Enable the youtube-dl hook-script. It will look at the input URL, and
    // will play the video located on the website
    [[nodiscard]] bool ytdl() const;
    // --load-scripts=<yes|no>
    // Auto-load scripts from the scripts configuration subdirectory (usually
    // ~/.config/mpv/scripts/)
    [[nodiscard]] bool loadScripts() const;
    // Full path of the currently played file. Usually this is exactly the same
    // string you pass on the mpv command line or the loadfile command, even if
    // it's a relative path. If you expect an absolute path, you will have to
    // determine it yourself
    [[nodiscard]] QString path() const;
    // Symbolic name of the file format. In some cases, this is a
    // comma-separated list of format names, e.g. mp4 is mov,mp4,m4a,3gp,3g2,mj2
    // (the list may grow in the future for any format).
    [[nodiscard]] QString fileFormat() const;
    // Length in bytes of the source file/stream.
    [[nodiscard]] qint64 fileSize() const;
    // Video bitrate
    [[nodiscard]] qreal videoBitrate() const;
    // Audio bitrate
    [[nodiscard]] qreal audioBitrate() const;
    // Return the list of discovered audio devices.
    [[nodiscard]] MpvDeclarativeObject::AudioDevices audioDeviceList() const;
    // Video format as string.
    [[nodiscard]] QString videoFormat() const;
    // The call type of mpv client APIs.
    [[nodiscard]] MpvDeclarativeObject::MpvCallType mpvCallType() const;
    // Video, audio and subtitle tracks.
    [[nodiscard]] MpvDeclarativeObject::MediaTracks mediaTracks() const;
    // File types supported by mpv:
    // https://github.com/mpv-player/mpv/blob/master/player/external_files.c
    [[nodiscard]] QStringList videoSuffixes() const {
        return QStringList{QLatin1String("*.3g2"),   QLatin1String("*.3ga"),
                           QLatin1String("*.3gp"),   QLatin1String("*.3gp2"),
                           QLatin1String("*.3gpp"),  QLatin1String("*.amv"),
                           QLatin1String("*.asf"),   QLatin1String("*.asx"),
                           QLatin1String("*.avf"),   QLatin1String("*.avi"),
                           QLatin1String("*.bdm"),   QLatin1String("*.bdmv"),
                           QLatin1String("*.bik"),   QLatin1String("*.clpi"),
                           QLatin1String("*.cpi"),   QLatin1String("*.dat"),
                           QLatin1String("*.divx"),  QLatin1String("*.drc"),
                           QLatin1String("*.dv"),    QLatin1String("*.dvr-ms"),
                           QLatin1String("*.f4v"),   QLatin1String("*.flv"),
                           QLatin1String("*.gvi"),   QLatin1String("*.gxf"),
                           QLatin1String("*.hdmov"), QLatin1String("*.hlv"),
                           QLatin1String("*.iso"),   QLatin1String("*.letv"),
                           QLatin1String("*.lrv"),   QLatin1String("*.m1v"),
                           QLatin1String("*.m2p"),   QLatin1String("*.m2t"),
                           QLatin1String("*.m2ts"),  QLatin1String("*.m2v"),
                           QLatin1String("*.m3u"),   QLatin1String("*.m3u8"),
                           QLatin1String("*.m4v"),   QLatin1String("*.mkv"),
                           QLatin1String("*.moov"),  QLatin1String("*.mov"),
                           QLatin1String("*.mp2"),   QLatin1String("*.mp2v"),
                           QLatin1String("*.mp4"),   QLatin1String("*.mp4v"),
                           QLatin1String("*.mpe"),   QLatin1String("*.mpeg"),
                           QLatin1String("*.mpeg1"), QLatin1String("*.mpeg2"),
                           QLatin1String("*.mpeg4"), QLatin1String("*.mpg"),
                           QLatin1String("*.mpl"),   QLatin1String("*.mpls"),
                           QLatin1String("*.mpv"),   QLatin1String("*.mpv2"),
                           QLatin1String("*.mqv"),   QLatin1String("*.mts"),
                           QLatin1String("*.mtv"),   QLatin1String("*.mxf"),
                           QLatin1String("*.mxg"),   QLatin1String("*.nsv"),
                           QLatin1String("*.nuv"),   QLatin1String("*.ogm"),
                           QLatin1String("*.ogv"),   QLatin1String("*.ogx"),
                           QLatin1String("*.ps"),    QLatin1String("*.qt"),
                           QLatin1String("*.qtvr"),  QLatin1String("*.ram"),
                           QLatin1String("*.rec"),   QLatin1String("*.rm"),
                           QLatin1String("*.rmj"),   QLatin1String("*.rmm"),
                           QLatin1String("*.rms"),   QLatin1String("*.rmvb"),
                           QLatin1String("*.rmx"),   QLatin1String("*.rp"),
                           QLatin1String("*.rpl"),   QLatin1String("*.rv"),
                           QLatin1String("*.rvx"),   QLatin1String("*.thp"),
                           QLatin1String("*.tod"),   QLatin1String("*.tp"),
                           QLatin1String("*.trp"),   QLatin1String("*.ts"),
                           QLatin1String("*.tts"),   QLatin1String("*.txd"),
                           QLatin1String("*.vcd"),   QLatin1String("*.vdr"),
                           QLatin1String("*.vob"),   QLatin1String("*.vp8"),
                           QLatin1String("*.vro"),   QLatin1String("*.webm"),
                           QLatin1String("*.wm"),    QLatin1String("*.wmv"),
                           QLatin1String("*.wtv"),   QLatin1String("*.xesc"),
                           QLatin1String("*.xspf")};
    }
    [[nodiscard]] QStringList audioSuffixes() const {
        return QStringList{QLatin1String("*.mp3"),  QLatin1String("*.aac"),
                           QLatin1String("*.mka"),  QLatin1String("*.dts"),
                           QLatin1String("*.flac"), QLatin1String("*.ogg"),
                           QLatin1String("*.m4a"),  QLatin1String("*.ac3"),
                           QLatin1String("*.opus"), QLatin1String("*.wav"),
                           QLatin1String("*.wv")};
    }
    [[nodiscard]] QStringList subtitleSuffixes() const {
        return QStringList{QLatin1String("*.utf"),   QLatin1String("*.utf8"),
                           QLatin1String("*.utf-8"), QLatin1String("*.idx"),
                           QLatin1String("*.sub"),   QLatin1String("*.srt"),
                           QLatin1String("*.rt"),    QLatin1String("*.ssa"),
                           QLatin1String("*.ass"),   QLatin1String("*.mks"),
                           QLatin1String("*.vtt"),   QLatin1String("*.sup"),
                           QLatin1String("*.scc"),   QLatin1String("*.smi")};
    }
    // Chapter list
    [[nodiscard]] MpvDeclarativeObject::Chapters chapters() const;
    // Metadata map
    [[nodiscard]] MpvDeclarativeObject::Metadata metadata() const;
    // Last A/V synchronization difference. Unavailable if audio or video is
    // disabled.
    [[nodiscard]] qreal avsync() const;
    // Position in current file (0-100). The advantage over using this instead
    // of calculating it out of other properties is that it properly falls back
    // to estimating the playback position from the byte position, if the file
    // duration is not known.
    [[nodiscard]] int percentPos() const;
    // Estimated/measured FPS of the video filter chain output. (If no filters
    // are used, this corresponds to decoder output.) This uses the average of
    // the 10 past frame durations to calculate the FPS. It will be inaccurate
    // if frame-dropping is involved (such as when framedrop is explicitly
    // enabled, or after precise seeking). Files with imprecise timestamps (such
    // as Matroska) might lead to unstable results.
    [[nodiscard]] qreal estimatedVfFps() const;

    void setSource(const QUrl &source);
    void setMute(bool mute);
    void setPlaybackState(MpvDeclarativeObject::PlaybackState playbackState);
    void setLogLevel(MpvDeclarativeObject::LogLevel logLevel);
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
    void setMpvCallType(MpvDeclarativeObject::MpvCallType mpvCallType);
    void setPercentPos(int percentPos);

    Q_INVOKABLE bool open(const QUrl &url);
    Q_INVOKABLE bool play();
    Q_INVOKABLE bool play(const QUrl &url);
    Q_INVOKABLE bool pause();
    Q_INVOKABLE bool stop();
    Q_INVOKABLE bool seek(qint64 value, bool absolute = false,
                          bool percent = false);
    // Jump to an absolute position, in seconds. libmpv supports negative
    // position, which means jump from the end of the file, but I will not
    // implement it in a short period of time because I think it's useless.
    Q_INVOKABLE bool seekAbsolute(qint64 position);
    // Jump to a relative position, in seconds. If the offset is negative, then
    // the player will jump back.
    Q_INVOKABLE bool seekRelative(qint64 offset);
    // Jump to an absolute percent position (0-100). Although libmpv supports
    // relative percent, I will not implement it in a short period of time
    // because I don't think it is useful enough.
    Q_INVOKABLE bool seekPercent(int percent);
    Q_INVOKABLE bool screenshot();
    // According to mpv's manual, the file path must contain an extension
    // name, otherwise the behavior is arbitrary.
    Q_INVOKABLE bool screenshotToFile(const QString &filePath);

protected Q_SLOTS:
    void handleMpvEvents();

private Q_SLOTS:
    void doUpdate();

private:
    bool mpvSendCommand(const QVariant &arguments);
    bool mpvSetProperty(const QString &name, const QVariant &value);
    QVariant mpvGetProperty(const QString &name, bool *ok = nullptr) const;
    bool mpvObserveProperty(const QString &name);

    void processMpvLogMessage(mpv_event_log_message *event);
    void processMpvPropertyChange(mpv_event_property *event);

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] bool isPaused() const;
    [[nodiscard]] bool isStopped() const;

    void setMediaStatus(MpvDeclarativeObject::MediaStatus mediaStatus);

    // Should be called when MPV_EVENT_VIDEO_RECONFIG happens.
    // Never do anything expensive here.
    void videoReconfig();
    // Should be called when MPV_EVENT_AUDIO_RECONFIG happens.
    // Never do anything expensive here.
    void audioReconfig();

    void playbackStateChangeEvent();

private:
    mpv::qt::Handle mpv;
    mpv_render_context *mpv_gl = nullptr;

    QUrl currentSource = QUrl();
    MpvDeclarativeObject::MediaStatus currentMediaStatus =
        MpvDeclarativeObject::MediaStatus::NoMedia;
    MpvDeclarativeObject::MpvCallType currentMpvCallType =
        MpvDeclarativeObject::MpvCallType::Synchronous;

    const QHash<const char *, const char *> properties = {
        {"dwidth", "videoSizeChanged"},
        {"dheight", "videoSizeChanged"},
        {"duration", "durationChanged"},
        {"time-pos", "positionChanged"},
        {"volume", "volumeChanged"},
        {"mute", "muteChanged"},
        {"seekable", "seekableChanged"},
        {"hwdec", "hwdecChanged"},
        {"vid", "vidChanged"},
        {"aid", "aidChanged"},
        {"sid", "sidChanged"},
        {"video-rotate", "videoRotateChanged"},
        {"video-aspect", "videoAspectChanged"},
        {"speed", "speedChanged"},
        {"deinterlace", "deinterlaceChanged"},
        {"audio-exclusive", "audioExclusiveChanged"},
        {"audio-file-auto", "audioFileAutoChanged"},
        {"sub-auto", "subAutoChanged"},
        {"sub-codepage", "subCodepageChanged"},
        {"filename", "fileNameChanged"},
        {"media-title", "mediaTitleChanged"},
        {"vo", "voChanged"},
        {"ao", "aoChanged"},
        {"screenshot-format", "screenshotFormatChanged"},
        {"screenshot-png-compression", "screenshotPngCompressionChanged"},
        {"screenshot-template", "screenshotTemplateChanged"},
        {"screenshot-directory", "screenshotDirectoryChanged"},
        {"profile", "profileChanged"},
        {"hr-seek", "hrSeekChanged"},
        {"ytdl", "ytdlChanged"},
        {"load-scripts", "loadScriptsChanged"},
        {"path", "pathChanged"},
        {"file-format", "fileFormatChanged"},
        {"file-size", "fileSizeChanged"},
        {"video-bitrate", "videoBitrateChanged"},
        {"audio-bitrate", "audioBitrateChanged"},
        {"audio-device-list", "audioDeviceListChanged"},
        {"screenshot-tag-colorspace", "screenshotTagColorspaceChanged"},
        {"screenshot-jpeg-quality", "screenshotJpegQualityChanged"},
        {"video-format", "videoFormatChanged"},
        {"pause", "playbackStateChanged"},
        {"idle-active", "playbackStateChanged"},
        {"track-list", "mediaTracksChanged"},
        {"chapter-list", "chaptersChanged"},
        {"metadata", "metadataChanged"},
        {"avsync", "avsyncChanged"},
        {"percent-pos", "percentPosChanged"},
        {"estimated-vf-fps", "estimatedVfFpsChanged"}};

    // These properties are changing all the time during the playback process.
    // So we have to add them to the black list, otherwise we'll get huge
    // message floods.
    const QStringList propertyBlackList = {
        QLatin1String("time-pos"),      QLatin1String("playback-time"),
        QLatin1String("percent-pos"),   QLatin1String("video-bitrate"),
        QLatin1String("audio-bitrate"), QLatin1String("estimated-vf-fps"),
        QLatin1String("avsync")};

Q_SIGNALS:
    void onUpdate();
    void hasMpvEvents();
    void initFinished();

    void loaded();
    void playing();
    void paused();
    void stopped();

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
    void metadataChanged();
    void avsyncChanged();
    void percentPosChanged();
    void estimatedVfFpsChanged();
};

Q_DECLARE_METATYPE(MpvDeclarativeObject::MediaTracks)

#endif
