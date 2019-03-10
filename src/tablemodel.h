#pragma once

#include <QString>
#include <QAbstractTableModel>
#include "configitem.h"

namespace qedytor {
class TableModel : public QAbstractTableModel
{
private:
  QList<ConfigItem*> variantableList;
  QStringList headerList;
public:
  explicit TableModel(QObject *parent = nullptr);
  virtual ~TableModel() override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  void setList(QList<ConfigItem*> &variantableList, QStringList &headerList);
};
}
