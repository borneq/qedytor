#pragma once
#include <QVariant>

namespace qedytor {
struct ConfigItem
{
    QString path;
    QString name;
    virtual ~ConfigItem() {}
    virtual QVariant getData(int col) = 0;
};
}
