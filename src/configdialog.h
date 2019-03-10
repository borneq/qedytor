#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableView>
#include "config.h"
#include "tablemodel.h"
#include "configtableview.h"

namespace qedytor {
class ConfigDialog : public QDialog
{
    Q_OBJECT
private:
    Config *config;
    void keyPressEvent(QKeyEvent *event);
    QLineEdit *syntaxPath;
    QLineEdit *themesPath;
    void load(Config *config);
    void save(Config *config);
    Config *configRef;
    bool checkCorrectness();
    //TableModel *modelEditors;
    //TableModel *modelMru;
    //TableModel *modelHandy;
    QTabWidget *tabList;
    ConfigTableView *listEditors;
    ConfigTableView *listHandy;
    ConfigTableView *listMru;
signals:
    void changedIni();
public slots:
    QString browse(QString defPath);
    void apply();
    void okSlot();
    void addRecord();
    void deleteRecord();
    void editRecord();
public:
    ConfigDialog(Config *config, QWidget *parent);
    ~ConfigDialog();
};
}
#endif // CONFIGDIALOG_H
