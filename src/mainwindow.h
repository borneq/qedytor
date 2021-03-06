#ifndef TABDIALOG_H
#define TABDIALOG_H

#include "config.h"
#include "configdialog.h"
#include "tabcontrolable.h"
#include "searchdialog.h"
#include "infowindow.h"
#include <repository.h>
#include <QMainWindow>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QBitArray>
#include <QFileInfo>

namespace qedytor {
class MainWindow : public QMainWindow, public TabControlable
{
    Q_OBJECT
private:
    syntaxhl::Repository *repository;
    Config config;
    ConfigDialog *configDialog = nullptr;
    InfoWindow *infoWindow = nullptr;
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    void createMenu();
    bool tabControlPressed = false;
    bool eventFilter(QObject *watched, QEvent *event);
    bool openOrActivateFile(const QString& fileName);
    void addFilelistToMenu(QMenu* mruMenu, QList<ConfigItem*> &configMru, SortBy sortBy);
    QAction *newAction;
    QAction *openAction;
    QAction *closeAction;
    QAction *saveAction;
    QAction *saveasAction;
    QAction *exitAction;
    QMenu* mruMenu;
    QMenu* fileMenu;
    QMenu* windowMenu;
    QMenu* handyMenu;
    void closeTab(int index);
    void reReadRepository();
    SearchDialog searchDialog;
    QBitArray untitledNumbers;
    void closeEvent(QCloseEvent *event)override;
signals:
    void needToShow();
public slots:
    void handleMessage(const QString& message);
    void about();
    void removeObsoleteMru();
    void closeCurrentTab();
    void callSaveAs();
    void currentChanged(int index);
    void selectWord();
    void toLower();
    void toUpper();
    void insertDate();
    void insertTime();
    void find(bool bReplace);
    void findNext();
    void findPrev();
    void openConfigDialog();
    void openConfigDialogModal();
    void onDestroyConfigDialog();
    void onDestroyInfoWindow();
    void onChangedIni();
    void aboutToQuit();
    void properties();
    void showMenuWindow();
    void showMenuFile();
    void showMenuHandy();
public:
    QTabWidget *tabWidget;
    explicit MainWindow(QWidget *parent = nullptr);
    void setTab(int n) override;
    ~MainWindow();
};
}
#endif
