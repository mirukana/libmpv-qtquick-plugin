#pragma once

#ifndef _MPVDECLARATIVEMODULE_H
#define _MPVDECLARATIVEMODULE_H

#include <QQmlExtensionPlugin>

class MpvDeclarativeModule : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif
