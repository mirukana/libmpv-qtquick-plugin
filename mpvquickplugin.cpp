#include "mpvquickplugin.h"
#include "mpvplayer.h"

void MpvQuickPlugin::registerTypes(const char *uri) {
    Q_ASSERT(uri == QLatin1String("wangwenx190.QuickMpv"));
    qmlRegisterType<MpvPlayer>(uri, 1, 0, "MpvPlayer");
}
