#include "mpvplayer.h"
#include <QDebug>
#include <QFileInfo>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QStandardPaths>

namespace {

void wakeup(void *ctx) {
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    QMetaObject::invokeMethod(static_cast<MpvPlayer *>(ctx), "hasMpvEvents",
                              Qt::QueuedConnection);
}

void on_mpv_redraw(void *ctx) { MpvPlayer::on_update(ctx); }

void *get_proc_address_mpv(void *ctx, const char *name) {
    Q_UNUSED(ctx)

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (glctx == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

} // namespace

class MpvRenderer : public QQuickFramebufferObject::Renderer {
public:
    MpvRenderer(MpvPlayer *mpvPlayer) : m_mpvPlayer(mpvPlayer) {
        Q_ASSERT(m_mpvPlayer != nullptr);
    }

    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject *
    createFramebufferObject(const QSize &size) override {
        // init mpv_gl:
        if (m_mpvPlayer->mpv_gl == nullptr) {
            mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr,
                                                  nullptr};
            mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE,
                 const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                {MPV_RENDER_PARAM_INVALID, nullptr}};

            const int initResult = mpv_render_context_create(
                &m_mpvPlayer->mpv_gl, m_mpvPlayer->mpv, params);
            Q_ASSERT(initResult >= 0);
            mpv_render_context_set_update_callback(m_mpvPlayer->mpv_gl,
                                                   on_mpv_redraw, m_mpvPlayer);
        }

        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render() override {
        m_mpvPlayer->window()->resetOpenGLState();

        QOpenGLFramebufferObject *fbo = framebufferObject();
        mpv_opengl_fbo mpfbo{0, 0, 0, 0};
        mpfbo.fbo = static_cast<int>(fbo->handle());
        mpfbo.w = fbo->width();
        mpfbo.h = fbo->height();
        int flip_y{0};

        mpv_render_param params[] = {
            // Specify the default framebuffer (0) as target. This will
            // render onto the entire screen. If you want to show the video
            // in a smaller rectangle or apply fancy transformations, you'll
            // need to render into a separate FBO and draw it manually.
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            // Flip rendering (needed due to flipped GL coordinate system).
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}};
        // See render_gl.h on what OpenGL environment mpv expects, and
        // other API details.
        mpv_render_context_render(m_mpvPlayer->mpv_gl, params);

        m_mpvPlayer->window()->resetOpenGLState();
    }

private:
    MpvPlayer *m_mpvPlayer = nullptr;
};

MpvPlayer::MpvPlayer(QQuickItem *parent) : QQuickFramebufferObject(parent) {
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    Q_ASSERT(mpv != nullptr);

    mpvSetProperty(QLatin1String("input-default-bindings"), false);
    mpvSetProperty(QLatin1String("input-vo-keyboard"), false);
    mpvSetProperty(QLatin1String("input-cursor"), false);
    mpvSetProperty(QLatin1String("cursor-autohide"), false);

    setScreenshotFormat(QLatin1String("png"));
    setScreenshotPngCompression(9);
    setScreenshotTagColorspace(true);
    setScreenshotTemplate(QLatin1String("quickplayer-screenshot-%F-%p"));
    setScreenshotDirectory(
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    setHwdec(QLatin1String("auto"));
    setHrSeek(true);
    setYtdl(true);
    setLoadScripts(true);
    setLogLevel(LogLevel::DebugLevel);

    QHash<QString, QString>::const_iterator iterator = properties.constBegin();
    while (iterator != properties.constEnd()) {
        mpvObserveProperty(iterator.key());
        ++iterator;
    }

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &MpvPlayer::hasMpvEvents, this, &MpvPlayer::handleMpvEvents,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    const int mpvInitResult = mpv_initialize(mpv);
    Q_ASSERT(mpvInitResult >= 0);

    connect(this, &MpvPlayer::onUpdate, this, &MpvPlayer::doUpdate,
            Qt::QueuedConnection);
}

MpvPlayer::~MpvPlayer() {
    // only initialized if something got drawn
    if (mpv_gl != nullptr) {
        mpv_render_context_free(mpv_gl);
    }
    // We don't need to destroy mpv handle in our own because we are using
    // mpv::qt::Handle, which is a shared pointer.
    // mpv_terminate_destroy(mpv);
}

void MpvPlayer::on_update(void *ctx) {
    emit static_cast<MpvPlayer *>(ctx)->onUpdate();
}

// connected to onUpdate() signal makes sure it runs on the GUI thread
void MpvPlayer::doUpdate() { update(); }

void MpvPlayer::processMpvLogMessage(mpv_event_log_message *event) {
    const QString logMessage = QStringLiteral("[libmpv] %1: %2")
                                   .arg(QString::fromUtf8(event->prefix))
                                   .arg(QString::fromUtf8(event->text));
    switch (event->log_level) {
    case MPV_LOG_LEVEL_V:
    case MPV_LOG_LEVEL_DEBUG:
    case MPV_LOG_LEVEL_TRACE:
        qDebug().noquote() << logMessage;
        break;
    case MPV_LOG_LEVEL_WARN:
        qWarning().noquote() << logMessage;
        break;
    case MPV_LOG_LEVEL_ERROR:
        qCritical().noquote() << logMessage;
        break;
    case MPV_LOG_LEVEL_FATAL:
        qFatal("%ls", qUtf16Printable(logMessage));
        break;
    case MPV_LOG_LEVEL_INFO:
        qInfo().noquote() << logMessage;
        break;
    default:
        qDebug().noquote() << logMessage;
        break;
    }
}

void MpvPlayer::processMpvPropertyChange(mpv_event_property *event) {
    const QString eventName = QString::fromUtf8(event->name);
    if ((eventName != QLatin1String("time-pos")) &&
        (eventName != QLatin1String("playback-time"))) {
        qDebug().noquote() << QLatin1String(
                                  "[libmpv] Property changed from mpv:")
                           << eventName;
    }
    if (properties.contains(eventName)) {
        const QString signalName = properties.value(eventName);
        if (!signalName.isEmpty()) {
            QMetaObject::invokeMethod(this, qPrintable(signalName));
        }
    }
}

bool MpvPlayer::isLoaded() const {
    return ((mediaStatus() == MediaStatus::LoadedMedia) ||
            (mediaStatus() == MediaStatus::BufferingMedia) ||
            (mediaStatus() == MediaStatus::BufferedMedia));
}

bool MpvPlayer::isPlaying() const {
    return playbackState() == PlaybackState::PlayingState;
}

bool MpvPlayer::isPaused() const {
    return playbackState() == PlaybackState::PausedState;
}

bool MpvPlayer::isStopped() const {
    return playbackState() == PlaybackState::StoppedState;
}

void MpvPlayer::setMediaStatus(MpvPlayer::MediaStatus mediaStatus) {
    if (this->mediaStatus() == mediaStatus) {
        return;
    }
    currentMediaStatus = mediaStatus;
    emit mediaStatusChanged();
}

void MpvPlayer::videoReconfig() { emit videoSizeChanged(); }

void MpvPlayer::audioReconfig() {}

void MpvPlayer::mpvSendCommand(const QVariant &arguments) {
    if (arguments.isNull() || !arguments.isValid()) {
        return;
    }
    qDebug().noquote() << QLatin1String("Sending a command to mpv:")
                       << arguments;
    int errorCode = 0;
    if (mpvCallType() == MpvCallType::AsynchronousCall) {
        errorCode = mpv::qt::command_async(mpv, arguments, 0);
    } else {
        errorCode = mpv::qt::get_error(mpv::qt::command(mpv, arguments));
    }
    if (errorCode < 0) {
        qWarning().noquote()
            << QLatin1String("Failed to execute a command for mpv:")
            << arguments;
    }
}

void MpvPlayer::mpvSetProperty(const QString &name, const QVariant &value) {
    if (name.isEmpty() || value.isNull() || !value.isValid()) {
        return;
    }
    qDebug().noquote() << QLatin1String("Setting a property for mpv:") << name
                       << QLatin1String("to:") << value;
    int errorCode = 0;
    if (mpvCallType() == MpvCallType::AsynchronousCall) {
        errorCode = mpv::qt::set_property_async(mpv, name, value, 0);
    } else {
        errorCode = mpv::qt::get_error(mpv::qt::set_property(mpv, name, value));
    }
    if (errorCode < 0) {
        qWarning().noquote()
            << QLatin1String("Failed to set a property for mpv:") << name;
    }
}

QVariant MpvPlayer::mpvGetProperty(const QString &name) const {
    if (name.isEmpty()) {
        return QVariant();
    }
    const QVariant result = mpv::qt::get_property(mpv, name);
    if (result.isNull() || !result.isValid()) {
        qWarning().noquote()
            << QLatin1String("Failed to query a property from mpv:") << name;
    } else {
        /*if ((name != QLatin1String("time-pos")) &&
            (name != QLatin1String("duration"))) {
            qDebug().noquote() << QLatin1String("Querying a property from mpv:")
                               << name << QLatin1String("result:") << result;
        }*/
    }
    return result;
}

void MpvPlayer::mpvObserveProperty(const QString &name) {
    if (name.isEmpty()) {
        return;
    }
    qDebug().noquote() << QLatin1String("Observing a property from mpv:")
                       << name;
    const int errorCode = mpv_observe_property(
        mpv, 0, name.toUtf8().constData(), MPV_FORMAT_NONE);
    if (errorCode < 0) {
        qWarning().noquote()
            << QLatin1String("Failed to observe a property from mpv:") << name;
    }
}

QQuickFramebufferObject::Renderer *MpvPlayer::createRenderer() const {
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvPlayer *>(this));
}

QUrl MpvPlayer::source() const { return isStopped() ? QUrl() : currentSource; }

QString MpvPlayer::fileName() const {
    return isStopped() ? QString()
                       : mpvGetProperty(QLatin1String("filename")).toString();
}

QSize MpvPlayer::videoSize() const {
    if (isStopped()) {
        return QSize();
    }
    QSize size(
        qMax(mpvGetProperty(QLatin1String("video-out-params/dw")).toInt(), 0),
        qMax(mpvGetProperty(QLatin1String("video-out-params/dh")).toInt(), 0));
    const int rotate = videoRotate();
    if ((rotate == 90) || (rotate == 270)) {
        size.transpose();
    }
    return size;
}

MpvPlayer::PlaybackState MpvPlayer::playbackState() const {
    const bool stopped = mpvGetProperty(QLatin1String("idle-active")).toBool();
    const bool paused = mpvGetProperty(QLatin1String("pause")).toBool();
    return stopped
        ? PlaybackState::StoppedState
        : (paused ? PlaybackState::PausedState : PlaybackState::PlayingState);
}

MpvPlayer::MediaStatus MpvPlayer::mediaStatus() const {
    return currentMediaStatus;
}

MpvPlayer::LogLevel MpvPlayer::logLevel() const {
    const QString level = mpvGetProperty(QLatin1String("msg-level")).toString();
    if (level.isEmpty() || (level == QLatin1String("no")) ||
        (level == QLatin1String("off"))) {
        return LogLevel::NoLog;
    }
    const QString actualLevel =
        level.right(level.length() - level.lastIndexOf(QLatin1Char('=')) - 1);
    if (actualLevel.isEmpty() || (actualLevel == QLatin1String("no")) ||
        (actualLevel == QLatin1String("off"))) {
        return LogLevel::NoLog;
    }
    if ((actualLevel == QLatin1String("v")) ||
        (actualLevel == QLatin1String("debug")) ||
        (actualLevel == QLatin1String("trace"))) {
        return LogLevel::DebugLevel;
    }
    if (actualLevel == QLatin1String("warn")) {
        return LogLevel::WarningLevel;
    }
    if (actualLevel == QLatin1String("error")) {
        return LogLevel::CriticalLevel;
    }
    if (actualLevel == QLatin1String("fatal")) {
        return LogLevel::FatalLevel;
    }
    if (actualLevel == QLatin1String("info")) {
        return LogLevel::InfoLevel;
    }
    return LogLevel::DebugLevel;
}

qint64 MpvPlayer::duration() const {
    return isStopped()
        ? 0
        : qMax(mpvGetProperty(QLatin1String("duration")).toLongLong(),
               qint64(0));
}

qint64 MpvPlayer::position() const {
    return isStopped()
        ? 0
        : qMin(qMax(mpvGetProperty(QLatin1String("time-pos")).toLongLong(),
                    qint64(0)),
               duration());
}

int MpvPlayer::volume() const {
    return qMin(qMax(mpvGetProperty(QLatin1String("volume")).toInt(), 0), 100);
}

bool MpvPlayer::mute() const {
    return mpvGetProperty(QLatin1String("mute")).toBool();
}

bool MpvPlayer::seekable() const {
    return isStopped() ? false
                       : mpvGetProperty(QLatin1String("seekable")).toBool();
}

QString MpvPlayer::mediaTitle() const {
    return isStopped()
        ? QString()
        : mpvGetProperty(QLatin1String("media-title")).toString();
}

QString MpvPlayer::hwdec() const {
    // Querying "hwdec" itself will return empty string.
    return mpvGetProperty(QLatin1String("hwdec-current")).toString();
}

QString MpvPlayer::mpvVersion() const {
    return mpvGetProperty(QLatin1String("mpv-version")).toString();
}

QString MpvPlayer::mpvConfiguration() const {
    return mpvGetProperty(QLatin1String("mpv-configuration")).toString();
}

QString MpvPlayer::ffmpegVersion() const {
    return mpvGetProperty(QLatin1String("ffmpeg-version")).toString();
}

QString MpvPlayer::qtVersion() const {
    // qVersion(): run-time Qt version
    // QT_VERSION_STR: Qt version against which the application is compiled
    return qVersion();
}

int MpvPlayer::vid() const {
    return isStopped() ? 0 : mpvGetProperty(QLatin1String("vid")).toInt();
}

int MpvPlayer::aid() const {
    return isStopped() ? 0 : mpvGetProperty(QLatin1String("aid")).toInt();
}

int MpvPlayer::sid() const {
    return isStopped() ? 0 : mpvGetProperty(QLatin1String("sid")).toInt();
}

int MpvPlayer::videoRotate() const {
    return isStopped()
        ? 0
        : qMin((qMax(mpvGetProperty(QLatin1String("video-out-params/rotate"))
                         .toInt(),
                     0) +
                360) %
                   360,
               359);
}

qreal MpvPlayer::videoAspect() const {
    return isStopped()
        ? 1.7777
        : qMax(
              mpvGetProperty(QLatin1String("video-out-params/aspect")).toReal(),
              0.0);
}

qreal MpvPlayer::speed() const {
    return qMax(mpvGetProperty(QLatin1String("speed")).toReal(), 0.0);
}

bool MpvPlayer::deinterlace() const {
    return mpvGetProperty(QLatin1String("deinterlace")).toBool();
}

bool MpvPlayer::audioExclusive() const {
    return mpvGetProperty(QLatin1String("audio-exclusive")).toBool();
}

QString MpvPlayer::audioFileAuto() const {
    return mpvGetProperty(QLatin1String("audio-file-auto")).toString();
}

QString MpvPlayer::subAuto() const {
    return mpvGetProperty(QLatin1String("sub-auto")).toString();
}

QString MpvPlayer::subCodepage() const {
    QString codePage = mpvGetProperty(QLatin1String("sub-codepage")).toString();
    if (codePage.startsWith(QLatin1Char('+'))) {
        codePage.remove(0, 1);
    }
    return codePage;
}

QString MpvPlayer::vo() const {
    return mpvGetProperty(QLatin1String("vo")).toString();
}

QString MpvPlayer::ao() const {
    return mpvGetProperty(QLatin1String("ao")).toString();
}

QString MpvPlayer::screenshotFormat() const {
    return mpvGetProperty(QLatin1String("screenshot-format")).toString();
}

bool MpvPlayer::screenshotTagColorspace() const {
    return mpvGetProperty(QLatin1String("screenshot-tag-colorspace")).toBool();
}

int MpvPlayer::screenshotPngCompression() const {
    return qMin(
        qMax(
            mpvGetProperty(QLatin1String("screenshot-png-compression")).toInt(),
            0),
        9);
}

int MpvPlayer::screenshotJpegQuality() const {
    return qMin(
        qMax(mpvGetProperty(QLatin1String("screenshot-jpeg-quality")).toInt(),
             0),
        100);
}

QString MpvPlayer::screenshotTemplate() const {
    return mpvGetProperty(QLatin1String("screenshot-template")).toString();
}

QString MpvPlayer::screenshotDirectory() const {
    return mpvGetProperty(QLatin1String("screenshot-directory")).toString();
}

QString MpvPlayer::profile() const { return QLatin1String("gpu-hq"); }

bool MpvPlayer::hrSeek() const {
    return mpvGetProperty(QLatin1String("hr-seek")).toBool();
}

bool MpvPlayer::ytdl() const {
    return mpvGetProperty(QLatin1String("ytdl")).toBool();
}

bool MpvPlayer::loadScripts() const {
    return mpvGetProperty(QLatin1String("load-scripts")).toBool();
}

QString MpvPlayer::path() const {
    return isStopped() ? QString()
                       : mpvGetProperty(QLatin1String("path")).toString();
}

QString MpvPlayer::fileFormat() const {
    return isStopped()
        ? QString()
        : mpvGetProperty(QLatin1String("file-format")).toString();
}

qint64 MpvPlayer::fileSize() const {
    return isStopped()
        ? 0
        : qMax(mpvGetProperty(QLatin1String("file-size")).toLongLong(),
               qint64(0));
}

qreal MpvPlayer::videoBitrate() const {
    return isStopped()
        ? 0.0
        : qMax(mpvGetProperty(QLatin1String("video-bitrate")).toReal(), 0.0);
}

qreal MpvPlayer::audioBitrate() const {
    return isStopped()
        ? 0.0
        : qMax(mpvGetProperty(QLatin1String("audio-bitrate")).toReal(), 0.0);
}

MpvPlayer::AudioDevices MpvPlayer::audioDeviceList() const {
    AudioDevices audioDevices;
    int deviceCount = 0;
    QVariantList deviceList =
        mpvGetProperty(QLatin1String("audio-device-list")).toList();
    for (const auto &device : deviceList) {
        const auto &deviceInfo = device.toMap();
        SingleTrackInfo singleTrackInfo;
        singleTrackInfo[QLatin1String("name")] =
            deviceInfo[QLatin1String("name")];
        singleTrackInfo[QLatin1String("description")] =
            deviceInfo[QLatin1String("description")];
        audioDevices.devices.append(singleTrackInfo);
        ++deviceCount;
    }
    audioDevices.count = deviceCount;
    return audioDevices;
}

QString MpvPlayer::videoFormat() const {
    return isStopped()
        ? QString()
        : mpvGetProperty(QLatin1String("video-format")).toString();
}

MpvPlayer::MpvCallType MpvPlayer::mpvCallType() const {
    return currentMpvCallType;
}

MpvPlayer::MediaTracks MpvPlayer::mediaTracks() const {
    MediaTracks mediaTracks;
    int trackCount = 0;
    QVariantList trackList =
        mpvGetProperty(QLatin1String("track-list")).toList();
    for (const auto &track : trackList) {
        const auto &trackInfo = track.toMap();
        if ((trackInfo[QLatin1String("type")] != QLatin1String("video")) &&
            (trackInfo[QLatin1String("type")] != QLatin1String("audio")) &&
            (trackInfo[QLatin1String("type")] != QLatin1String("sub"))) {
            continue;
        }
        SingleTrackInfo singleTrackInfo;
        singleTrackInfo[QLatin1String("id")] = trackInfo[QLatin1String("id")];
        singleTrackInfo[QLatin1String("type")] =
            trackInfo[QLatin1String("type")];
        singleTrackInfo[QLatin1String("src-id")] =
            trackInfo[QLatin1String("src-id")];
        if (trackInfo[QLatin1String("title")].toString().isEmpty()) {
            if (trackInfo[QLatin1String("lang")].toString() !=
                QLatin1String("und")) {
                singleTrackInfo[QLatin1String("title")] =
                    trackInfo[QLatin1String("lang")];
            } else if (!trackInfo[QLatin1String("external")].toBool()) {
                singleTrackInfo[QLatin1String("title")] =
                    QLatin1String("[internal]");
            } else {
                singleTrackInfo[QLatin1String("title")] =
                    QLatin1String("[untitled]");
            }
        } else {
            singleTrackInfo[QLatin1String("title")] =
                trackInfo[QLatin1String("title")];
        }
        singleTrackInfo[QLatin1String("lang")] =
            trackInfo[QLatin1String("lang")];
        singleTrackInfo[QLatin1String("default")] =
            trackInfo[QLatin1String("default")];
        singleTrackInfo[QLatin1String("forced")] =
            trackInfo[QLatin1String("forced")];
        singleTrackInfo[QLatin1String("codec")] =
            trackInfo[QLatin1String("codec")];
        singleTrackInfo[QLatin1String("external")] =
            trackInfo[QLatin1String("external")];
        singleTrackInfo[QLatin1String("external-filename")] =
            trackInfo[QLatin1String("external-filename")];
        singleTrackInfo[QLatin1String("selected")] =
            trackInfo[QLatin1String("selected")];
        singleTrackInfo[QLatin1String("decoder-desc")] =
            trackInfo[QLatin1String("decoder-desc")];
        if (trackInfo[QLatin1String("type")] == QLatin1String("video")) {
            singleTrackInfo[QLatin1String("albumart")] =
                trackInfo[QLatin1String("albumart")];
            singleTrackInfo[QLatin1String("demux-w")] =
                trackInfo[QLatin1String("demux-w")];
            singleTrackInfo[QLatin1String("demux-h")] =
                trackInfo[QLatin1String("demux-h")];
            singleTrackInfo[QLatin1String("demux-fps")] =
                trackInfo[QLatin1String("demux-fps")];
            mediaTracks.videoChannels.append(singleTrackInfo);
        } else if (trackInfo[QLatin1String("type")] == QLatin1String("audio")) {
            singleTrackInfo[QLatin1String("demux-channel-count")] =
                trackInfo[QLatin1String("demux-channel-count")];
            singleTrackInfo[QLatin1String("demux-channels")] =
                trackInfo[QLatin1String("demux-channels")];
            singleTrackInfo[QLatin1String("demux-samplerate")] =
                trackInfo[QLatin1String("demux-samplerate")];
            mediaTracks.audioTracks.append(singleTrackInfo);
        } else if (trackInfo[QLatin1String("type")] == QLatin1String("sub")) {
            mediaTracks.subtitleStreams.append(singleTrackInfo);
        }
        ++trackCount;
    }
    mediaTracks.count = trackCount;
    return mediaTracks;
}

MpvPlayer::Chapters MpvPlayer::chapters() const {
    Chapters chapters;
    int chapterCount = 0;
    QVariantList chapterList =
        mpvGetProperty(QLatin1String("chapter-list")).toList();
    for (const auto &chapter : chapterList) {
        const auto &chapterInfo = chapter.toMap();
        SingleTrackInfo singleTrackInfo;
        singleTrackInfo[QLatin1String("title")] =
            chapterInfo[QLatin1String("title")];
        singleTrackInfo[QLatin1String("time")] =
            chapterInfo[QLatin1String("time")];
        chapters.chapters.append(singleTrackInfo);
        ++chapterCount;
    }
    chapters.count = chapterCount;
    return chapters;
}

void MpvPlayer::open(const QUrl &url) {
    if (!url.isValid()) {
        return;
    }
    if (url != currentSource) {
        setSource(url);
    }
    play();
}

void MpvPlayer::play() {
    if (!isPaused() || !currentSource.isValid()) {
        return;
    }
    mpvSetProperty(QLatin1String("pause"), false);
}

void MpvPlayer::play(const QUrl &url) {
    if (!url.isValid()) {
        return;
    }
    if ((url == currentSource) && !isPlaying()) {
        play();
    } else {
        open(url);
    }
}

void MpvPlayer::pause() {
    if (!isPlaying()) {
        return;
    }
    mpvSetProperty(QLatin1String("pause"), true);
}

void MpvPlayer::stop() {
    if (isStopped()) {
        return;
    }
    mpvSendCommand(QVariantList{QLatin1String("stop")});
}

void MpvPlayer::seek(qint64 position) {
    if (isStopped()) {
        return;
    }
    mpvSendCommand(QVariantList{QLatin1String("seek"),
                                qMin(qMax(position, qint64(0)), duration()),
                                QLatin1String("absolute")});
}

void MpvPlayer::setSource(const QUrl &source) {
    if (!source.isValid() || (source == currentSource)) {
        return;
    }
    currentSource = source;
    mpvSendCommand(QVariantList{QLatin1String("loadfile"),
                                source.isLocalFile() ? source.toLocalFile()
                                                     : source.url()});
    emit sourceChanged();
}

void MpvPlayer::setMute(bool mute) {
    mpvSetProperty(QLatin1String("mute"), mute);
}

void MpvPlayer::setPlaybackState(MpvPlayer::PlaybackState playbackState) {
    if (isStopped() || (this->playbackState() == playbackState)) {
        return;
    }
    switch (playbackState) {
    case PlaybackState::StoppedState:
        stop();
        break;
    case PlaybackState::PausedState:
        pause();
        break;
    case PlaybackState::PlayingState:
        play();
        break;
    }
    emit playbackStateChanged();
}

void MpvPlayer::setLogLevel(MpvPlayer::LogLevel logLevel) {
    if (logLevel == this->logLevel()) {
        return;
    }
    QString level = QLatin1String("debug");
    switch (logLevel) {
    case MpvPlayer::NoLog:
        level = QLatin1String("no");
        break;
    case MpvPlayer::DebugLevel:
        // libmpv's log level: v (verbose) < debug < trace (print all messages)
        // Use "v" to avoid noisy message floods.
        level = QLatin1String("v");
        break;
    case MpvPlayer::WarningLevel:
        level = QLatin1String("warn");
        break;
    case MpvPlayer::CriticalLevel:
        level = QLatin1String("error");
        break;
    case MpvPlayer::FatalLevel:
        level = QLatin1String("fatal");
        break;
    case MpvPlayer::InfoLevel:
        level = QLatin1String("info");
        break;
    }
    mpvSetProperty(QLatin1String("terminal"), level != QLatin1String("no"));
    mpvSetProperty(QLatin1String("msg-level"),
                   QStringLiteral("all=%1").arg(level));
    mpv_request_log_messages(mpv, level.toUtf8().constData());
    emit logLevelChanged();
}

void MpvPlayer::setPosition(qint64 position) {
    if (isStopped()) {
        return;
    }
    seek(qMin(qMax(position, qint64(0)), duration()));
}

void MpvPlayer::setVolume(int volume) {
    mpvSetProperty(QLatin1String("volume"), qMin(qMax(volume, 0), 100));
}

void MpvPlayer::setHwdec(const QString &hwdec) {
    if (hwdec.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("hwdec"), hwdec);
}

void MpvPlayer::setVid(int vid) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("vid"), qMax(vid, 0));
}

void MpvPlayer::setAid(int aid) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("aid"), qMax(aid, 0));
}

void MpvPlayer::setSid(int sid) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("sid"), qMax(sid, 0));
}

void MpvPlayer::setVideoRotate(int videoRotate) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("video-rotate"),
                   qMin(qMax(videoRotate, 0), 359));
}

void MpvPlayer::setVideoAspect(qreal videoAspect) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("video-aspect"), qMax(videoAspect, 0.0));
}

void MpvPlayer::setSpeed(qreal speed) {
    if (isStopped()) {
        return;
    }
    mpvSetProperty(QLatin1String("speed"), qMax(speed, 0.0));
}

void MpvPlayer::setDeinterlace(bool deinterlace) {
    mpvSetProperty(QLatin1String("deinterlace"), deinterlace);
}

void MpvPlayer::setAudioExclusive(bool audioExclusive) {
    mpvSetProperty(QLatin1String("audio-exclusive"), audioExclusive);
}

void MpvPlayer::setAudioFileAuto(const QString &audioFileAuto) {
    if (audioFileAuto.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("audio-file-auto"), audioFileAuto);
}

void MpvPlayer::setSubAuto(const QString &subAuto) {
    if (subAuto.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("sub-auto"), subAuto);
}

void MpvPlayer::setSubCodepage(const QString &subCodepage) {
    if (subCodepage.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("sub-codepage"),
                   subCodepage.startsWith(QLatin1Char('+'))
                       ? subCodepage
                       : QLatin1Char('+') + subCodepage);
}

void MpvPlayer::setVo(const QString &vo) {
    if (vo.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("vo"), vo);
}

void MpvPlayer::setAo(const QString &ao) {
    if (ao.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("ao"), ao);
}

void MpvPlayer::setScreenshotFormat(const QString &screenshotFormat) {
    if (screenshotFormat.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("screenshot-format"), screenshotFormat);
}

void MpvPlayer::setScreenshotPngCompression(int screenshotPngCompression) {
    mpvSetProperty(QLatin1String("screenshot-png-compression"),
                   qMin(qMax(screenshotPngCompression, 0), 9));
}

void MpvPlayer::setScreenshotTemplate(const QString &screenshotTemplate) {
    if (screenshotTemplate.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("screenshot-template"), screenshotTemplate);
}

void MpvPlayer::setScreenshotDirectory(const QString &screenshotDirectory) {
    if (screenshotDirectory.isEmpty()) {
        return;
    }
    mpvSetProperty(QLatin1String("screenshot-directory"), screenshotDirectory);
}

void MpvPlayer::setProfile(const QString &profile) {
    if (profile.isEmpty()) {
        return;
    }
    mpvSendCommand(QVariantList{QLatin1String("apply-profile"), profile});
}

void MpvPlayer::setHrSeek(bool hrSeek) {
    mpvSetProperty(QLatin1String("hr-seek"), hrSeek);
}

void MpvPlayer::setYtdl(bool ytdl) {
    mpvSetProperty(QLatin1String("ytdl"), ytdl);
}

void MpvPlayer::setLoadScripts(bool loadScripts) {
    mpvSetProperty(QLatin1String("load-scripts"), loadScripts);
}

void MpvPlayer::setScreenshotTagColorspace(bool screenshotTagColorspace) {
    mpvSetProperty(QLatin1String("screenshot-tag-colorspace"),
                   screenshotTagColorspace);
}

void MpvPlayer::setScreenshotJpegQuality(int screenshotJpegQuality) {
    mpvSetProperty(QLatin1String("screenshot-jpeg-quality"),
                   qMin(qMax(screenshotJpegQuality, 0), 100));
}

void MpvPlayer::setMpvCallType(MpvPlayer::MpvCallType mpvCallType) {
    if (this->mpvCallType() == mpvCallType) {
        return;
    }
    currentMpvCallType = mpvCallType;
    emit mpvCallTypeChanged();
}

void MpvPlayer::handleMpvEvents() {
    // Process all events, until the event queue is empty.
    while (mpv != nullptr) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        // Nothing happened. Happens on timeouts or sporadic wakeups.
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        bool shouldOutput = true;
        switch (event->event_id) {
        // Happens when the player quits. The player enters a state where it
        // tries to disconnect all clients. Most requests to the player will
        // fail, and the client should react to this and quit with
        // mpv_destroy() as soon as possible.
        case MPV_EVENT_SHUTDOWN:
            break;
        // See mpv_request_log_messages().
        case MPV_EVENT_LOG_MESSAGE:
            processMpvLogMessage(
                static_cast<mpv_event_log_message *>(event->data));
            shouldOutput = false;
            break;
        // Reply to a mpv_get_property_async() request.
        // See also mpv_event and mpv_event_property.
        case MPV_EVENT_GET_PROPERTY_REPLY:
            shouldOutput = false;
            break;
        // Reply to a mpv_set_property_async() request.
        // (Unlike MPV_EVENT_GET_PROPERTY, mpv_event_property is not used.)
        case MPV_EVENT_SET_PROPERTY_REPLY:
            shouldOutput = false;
            break;
        // Reply to a mpv_command_async() or mpv_command_node_async() request.
        // See also mpv_event and mpv_event_command.
        case MPV_EVENT_COMMAND_REPLY:
            shouldOutput = false;
            break;
        // Notification before playback start of a file (before the file is
        // loaded).
        case MPV_EVENT_START_FILE:
            setMediaStatus(MediaStatus::LoadingMedia);
            break;
        // Notification after playback end (after the file was unloaded).
        // See also mpv_event and mpv_event_end_file.
        case MPV_EVENT_END_FILE:
            setMediaStatus(MediaStatus::EndOfMedia);
            emit playbackStateChanged();
            break;
        // Notification when the file has been loaded (headers were read
        // etc.), and decoding starts.
        case MPV_EVENT_FILE_LOADED:
            setMediaStatus(MediaStatus::LoadedMedia);
            emit playbackStateChanged();
            break;
        // Idle mode was entered. In this mode, no file is played, and the
        // playback core waits for new commands. (The command line player
        // normally quits instead of entering idle mode, unless --idle was
        // specified. If mpv was started with mpv_create(), idle mode is enabled
        // by default.)
        case MPV_EVENT_IDLE:
            emit playbackStateChanged();
            break;
        // Sent every time after a video frame is displayed. Note that
        // currently, this will be sent in lower frequency if there is no video,
        // or playback is paused - but that will be removed in the future, and
        // it will be restricted to video frames only.
        case MPV_EVENT_TICK:
            shouldOutput = false;
            break;
        // Triggered by the script-message input command. The command uses the
        // first argument of the command as client name (see mpv_client_name())
        // to dispatch the message, and passes along all arguments starting from
        // the second argument as strings.
        // See also mpv_event and mpv_event_client_message.
        case MPV_EVENT_CLIENT_MESSAGE:
            break;
        // Happens after video changed in some way. This can happen on
        // resolution changes, pixel format changes, or video filter changes.
        // The event is sent after the video filters and the VO are
        // reconfigured. Applications embedding a mpv window should listen to
        // this event in order to resize the window if needed.
        // Note that this event can happen sporadically, and you should check
        // yourself whether the video parameters really changed before doing
        // something expensive.
        case MPV_EVENT_VIDEO_RECONFIG:
            videoReconfig();
            break;
        // Similar to MPV_EVENT_VIDEO_RECONFIG. This is relatively
        // uninteresting, because there is no such thing as audio output
        // embedding.
        case MPV_EVENT_AUDIO_RECONFIG:
            audioReconfig();
            break;
        // Happens when a seek was initiated. Playback stops. Usually it will
        // resume with MPV_EVENT_PLAYBACK_RESTART as soon as the seek is
        // finished.
        case MPV_EVENT_SEEK:
            break;
        // There was a discontinuity of some sort (like a seek), and playback
        // was reinitialized. Usually happens after seeking, or ordered chapter
        // segment switches. The main purpose is allowing the client to detect
        // when a seek request is finished.
        case MPV_EVENT_PLAYBACK_RESTART:
            break;
        // Event sent due to mpv_observe_property().
        // See also mpv_event and mpv_event_property.
        case MPV_EVENT_PROPERTY_CHANGE:
            processMpvPropertyChange(
                static_cast<mpv_event_property *>(event->data));
            shouldOutput = false;
            break;
        // Happens if the internal per-mpv_handle ringbuffer overflows, and at
        // least 1 event had to be dropped. This can happen if the client
        // doesn't read the event queue quickly enough with mpv_wait_event(), or
        // if the client makes a very large number of asynchronous calls at
        // once.
        // Event delivery will continue normally once this event was returned
        // (this forces the client to empty the queue completely).
        case MPV_EVENT_QUEUE_OVERFLOW:
            break;
        // Triggered if a hook handler was registered with mpv_hook_add(), and
        // the hook is invoked. If you receive this, you must handle it, and
        // continue the hook with mpv_hook_continue().
        // See also mpv_event and mpv_event_hook.
        case MPV_EVENT_HOOK:
            break;
        default:
            break;
        }
        if (shouldOutput) {
            qDebug().noquote()
                << QLatin1String("[libmpv] Event received from mpv:")
                << QString::fromUtf8(mpv_event_name(event->event_id));
        }
    }
}
