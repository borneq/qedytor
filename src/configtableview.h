#pragma once
#include <QWidget>
#include <QTableView>
#include "tablemodel.h"

namespace qedytor {
class ConfigTableView : public QWidget
{
    QTableView *tableView;
    TableModel model;
    QList<ConfigItem*> *variantableList;
    QStringList *headerList;
public:
    explicit ConfigTableView(QList<ConfigItem*> *variantableList, QStringList *headerList, QWidget *parent);
    ~ConfigTableView() override;
    void update();
    QItemSelectionModel* selectionModel();
};
}
