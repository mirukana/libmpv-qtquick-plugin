#pragma once

#ifndef _MPVQUICKPLUGIN_H
#define _MPVQUICKPLUGIN_H

#include <QQmlExtensionPlugin>

class MpvQuickPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif
