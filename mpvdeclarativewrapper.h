#pragma once

#ifndef _MPVDECLARATIVEWRAPPER_H
#define _MPVDECLARATIVEWRAPPER_H

#include <QQmlExtensionPlugin>

class MpvDeclarativeWrapper : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif
