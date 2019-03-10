#include "tablemodel.h"

using namespace qedytor;

TableModel::TableModel(QObject *parent): QAbstractTableModel(parent)
{

}

TableModel::~TableModel()
{

}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation);
  if (role != Qt::DisplayRole)
      return QVariant();
  if (orientation==Qt::Orientation::Horizontal)
  {
      return QVariant(headerList[section]);
  }
  else
    return QString::number(section);
}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
  if(hasIndex(row, column, parent) == false) {
      return QModelIndex();
  }
  return createIndex(row, column, nullptr);
}


int TableModel::rowCount(const QModelIndex &parent) const
{
    return variantableList.count();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  return headerList.count();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
      return QVariant();

    if(role != Qt::DisplayRole)
      return QVariant();

    return variantableList[index.row()]->getData(index.column());
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TableModel::setList(QList<ConfigItem*> &variantableList, QStringList &headerList)
{
    beginResetModel();
    this->variantableList = variantableList;
    this->headerList = headerList;
    endResetModel();
}
