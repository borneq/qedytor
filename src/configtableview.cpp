#include <QStackedLayout>
#include "configtableview.h"

using namespace qedytor;
ConfigTableView::ConfigTableView(QList<ConfigItem*> *variantableList, QStringList *headerList, QWidget *parent):
    QWidget(parent), variantableList(variantableList), headerList(headerList)
{
    model.setList(*variantableList, *headerList);
    QStackedLayout *layout = new QStackedLayout(this);
    tableView = new QTableView(this);
    tableView->setModel(&model);
    tableView->setColumnWidth(0,200);
    tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(tableView);
    setLayout(layout);
}

ConfigTableView::~ConfigTableView()
{}

void ConfigTableView::update()
{
    model.setList(*variantableList, *headerList);
}

QItemSelectionModel* ConfigTableView::selectionModel()
{
    return tableView->selectionModel();
}
