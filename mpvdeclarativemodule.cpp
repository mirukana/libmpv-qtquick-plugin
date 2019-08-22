#include "mpvdeclarativemodule.h"
#include "mpvdeclarativeobject.h"

void MpvDeclarativeModule::registerTypes(const char *uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("wangwenx190.QuickMpv"));
    qmlRegisterType<MpvDeclarativeObject>(uri, 1, 0, "MpvObject");
}
