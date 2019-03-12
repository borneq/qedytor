#include <QtWidgets>
#include "tabwindow.h"
#include "codeeditor.h"
#include "edytorexception.h"
#include <syntaxhighlighter.h>
#include <definition.h>
#include <iostream>
#include <QKeyEvent>
#include <QTextDocument>
#include <synexception.h>

using namespace qedytor;

TabWindow::TabWindow(QWidget *parent)
    : QMainWindow(parent)
{
    centralWidget = new QWidget;
    bool configOk = false;
    try {
        config.readFromFile();
        configOk = true;
    } catch (EdytorException &ex) {
        QMessageBox::warning(nullptr, "Warning",ex.what(),
                                   QMessageBox::Ok);
    }
    tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);
    createMenu();
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    setWindowTitle(tr("Main Window"));
    if (!configOk)
        openConfigDialogModal();
    restoreGeometry(config.geometry);
    reReadRepository();
    connect(tabWidget, &QTabWidget::tabCloseRequested, this,
       [this](int index) {
           closeTab(index);
       });
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        currentChanged(index);
    });
    installEventFilter(this);
    untitledNumbers.resize(32);
}

void TabWindow::reReadRepository()
{
    repository = nullptr;
    try {
        repository = new syntaxhl::Repository(config.pathToSyntax, config.pathToThemes);
    } catch (syntaxhl::SynException &e) {
        delete repository;
        QMessageBox::warning(nullptr, "Warning", e.what(),
                                        QMessageBox::Ok);
        repository = new syntaxhl::Repository("", "");
    }
}

void TabWindow::handleMessage(const QString& message)
{
    showNormal();//bring window to top on OSX
    raise();//bring window from minimized state on OSX
    activateWindow();//bring window to front/unminimize on windows
    QStringList arguments = message.split("\n");
    for(int i=0; i<arguments.size(); i++)
        if (arguments[i]!="")
            openOrActivateFile(arguments[i]);
}

void TabWindow::currentChanged(int index)
{
    if (tabWidget->currentWidget())
        setWindowTitle(((CodeEditor*)(tabWidget->currentWidget()))->fileName);
}

TabWindow::~TabWindow()
{
    delete repository;
}

bool TabWindow::openOrActivateFile(const QString& aFilePath)
{
    QString title;
    CodeEditor* newEditor;
    QFileInfo fi(aFilePath);
    QString filePath = fi.absoluteFilePath();
    if (filePath=="")
    {
        int size = untitledNumbers.size();
        int n;
        for (int i=0; i<size; i++)
            if (untitledNumbers.at(i)==false)
            {
               n = i;
               break;
            }
            else
            {
                n = size;
                untitledNumbers.resize(size+32);
            }
        untitledNumbers.setBit(n);
        newEditor = new CodeEditor(repository, filePath, this);
        title = "Untitled"+ QString::number(n+1);
    }
    else
    {
        title = fi.fileName();
        for (int i=0; i<tabWidget->count(); i++)
        {
            CodeEditor *editor = qobject_cast<CodeEditor *>(tabWidget->widget(i));
            if (editor->fileName == filePath)
            {
                editor->openFile(filePath);
                tabWidget->setCurrentIndex(i);
                return false;
            }
        }
        newEditor = new CodeEditor(repository, filePath, this);
        newEditor->openFile(filePath);
    }
    tabWidget->addTab(newEditor, title);
    tabWidget->setCurrentWidget(newEditor);
    newEditor->setFocus();
    connect(newEditor, &QPlainTextEdit::textChanged, newEditor, &CodeEditor::onTextChanged);
    return true;
}

void TabWindow::addFilelistToMenu(QMenu* menu, QList<ConfigItem*> &configlist)
{
    auto mruGroup = new QActionGroup(menu);
    mruGroup->setExclusive(true);
    for (int i=0; i<configlist.size(); i++)
    {
        auto action = menu->addAction(configlist[i]->path);
        action->setData(configlist[i]->path);
        mruGroup->addAction(action);
    }
    connect(mruGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        openOrActivateFile(action->data().toString());
    });
}

void TabWindow::closeCurrentTab()
{
    if (tabWidget->currentWidget())
        closeTab(tabWidget->currentIndex());
}

void TabWindow::insertDate()
{
    if (tabWidget->currentWidget())
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->currentWidget());
        QDate today = QDate::currentDate();
        editor->insertPlainText(today.toString("yyyy-MM-dd dddd"));
    }
}

void TabWindow::insertTime()
{
    if (tabWidget->currentWidget())
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->currentWidget());
        QTime now = QTime::currentTime();
        editor->insertPlainText(now.toString(" hh:mm"));
    }
}

void TabWindow::find()
{
    if (tabWidget->currentWidget())
    {
        int result = searchDialog.exec();
        if (result == QDialog::Accepted)
        {
            dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->findNext(searchDialog.textToFind, 0);
        }
    }
}

void TabWindow::findNext()
{
    if (tabWidget->currentWidget())
    {
       dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->findNext(searchDialog.textToFind, 0);
    }
}

void TabWindow::findPrev()
{
    if (tabWidget->currentWidget())
    {
       dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->findNext(searchDialog.textToFind, QTextDocument::FindBackward);
    }
}

void TabWindow::properties()
{
    Properties properties;
    if (tabWidget->currentWidget())
    {
       properties = dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->getProperties();
    }
    if (!infoWindow)
    {
        infoWindow = new InfoWindow(this, repository);
        infoWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(infoWindow, SIGNAL(destroyed()), this, SLOT(onDestroyInfoWindow()));
        infoWindow->show();
    }
    else
        infoWindow->activateWindow();
    infoWindow->describeProperties(properties);
}

void TabWindow::openConfigDialog()
{
    if (!configDialog)
    {
        configDialog = new ConfigDialog(&config, this);
        configDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(configDialog, SIGNAL(destroyed()), this, SLOT(onDestroyConfigDialog()));
        connect(configDialog, SIGNAL(changedIni()), this, SLOT(onChangedIni()));
        configDialog->show();
    }
    else
        configDialog->activateWindow();
}

void TabWindow::openConfigDialogModal()
{
    ConfigDialog *configDialog = new ConfigDialog(&config, this);
    configDialog->exec();
    delete configDialog;
}

void TabWindow::onDestroyConfigDialog()
{
    configDialog = nullptr;
}

void TabWindow::onDestroyInfoWindow()
{
    infoWindow = nullptr;
}

void TabWindow::onChangedIni()
{
    reReadRepository();
    menuBar()->clear();
    createMenu();
}

void TabWindow::createMenu()
{
    QAction *newAction = new QAction(tr("New"), this);
    newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    QAction *openAction = new QAction(tr("Open file..."), this);
    QAction *saveAction = new QAction(tr("Save"), this);
    saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    QAction *exitAction = new QAction(tr("Exit"), this);
    QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);
    QAction *closeAction = new QAction("Close", this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
    QAction *configAction = new QAction("Config", this);
    QAction *insertDateAction = new QAction("Insert &date", this);
    QAction *insertTimeAction = new QAction("Insert &time", this);
    QAction *findAction = new QAction("&Find...", this);
    findAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    QAction *findNextAction = new QAction("Find &next", this);
    findNextAction->setShortcut(QKeySequence(Qt::Key_F3));
    QAction *findPrevAction = new QAction("Find &prev", this);
    findPrevAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3));
    QAction *propertiesAction = new QAction("Properties", this);

    connect(newAction, &QAction::triggered, this, [this]() {
        openOrActivateFile("");
    });
    connect(openAction, &QAction::triggered, this, [this]() {
        const auto fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Open File"));
        if (!fileName.isEmpty())
           openOrActivateFile(fileName);
    });
    connect(saveAction, &QAction::triggered, this, [this]() {
        if (tabWidget->currentWidget())
        {
            CodeEditor *editor = dynamic_cast<CodeEditor *>(tabWidget->currentWidget());
            if (!editor->fileName.isEmpty())
                editor->saveFile(editor->fileName);
        }
    });
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
    connect(configAction, SIGNAL(triggered()), this, SLOT(openConfigDialog()));
    connect(insertDateAction, SIGNAL(triggered()), this, SLOT(insertDate()));
    connect(insertTimeAction, SIGNAL(triggered()), this, SLOT(insertTime()));
    connect(findAction, SIGNAL(triggered()), this, SLOT(find()));
    connect(findNextAction, SIGNAL(triggered()), this, SLOT(findNext()));
    connect(findPrevAction, SIGNAL(triggered()), this, SLOT(findPrev()));
    connect(propertiesAction, SIGNAL(triggered()), this, SLOT(properties()));

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    QMenu* searchMenu = menuBar()->addMenu(tr("&Search"));
    searchMenu->addAction(findAction);
    searchMenu->addAction(findNextAction);
    searchMenu->addAction(findPrevAction);
    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(configAction);
    toolsMenu->addAction(insertDateAction);
    toolsMenu->addAction(insertTimeAction);
    toolsMenu->addAction(propertiesAction);

    mruMenu = menuBar()->addMenu(tr("&Mru"));
    addFilelistToMenu(mruMenu, config.mru);
    mruMenu->addSeparator();
    auto removeObsoleteAction = mruMenu->addAction("Remove obsolete");
    connect(removeObsoleteAction, SIGNAL(triggered()), this, SLOT(removeObsoleteMru()));

    QMenu *menuOthers = menuBar()->addMenu(tr("&Other editors"));
    auto othersGroup = new QActionGroup(menuOthers);
    othersGroup->setExclusive(true);
    for (int i=0; i<config.otherEditors.size(); i++)
    {
        auto action = menuOthers->addAction(config.otherEditors[i]->name);
        action->setData(config.otherEditors[i]->path);
        othersGroup->addAction(action);
    }
    connect(othersGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        QProcess process;
        QStringList args;
        if (tabWidget->currentWidget())
            args.append(((CodeEditor*)(tabWidget->currentWidget()))->fileName);
        process.startDetached(action->data().toString(), args);
    });

    windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(showMenuWindow()));
    handyMenu = menuBar()->addMenu(tr("&Handy"));
    addFilelistToMenu(handyMenu, config.handy);

    QMenu* helpMenu = menuBar()->addMenu(tr("&About"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void TabWindow::showMenuWindow()
{
    windowMenu->clear();
    QActionGroup *windowGroup = new QActionGroup(windowMenu);
    windowGroup->setExclusive(true);
    for (int i=0; i<tabWidget->count(); i++)
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->widget(i));
        QChar umpersanded;
        if (i<9)
            umpersanded = '1'+i;
        else if (i==9)
            umpersanded = '0';
        else if (i<10+26)
            umpersanded = 'a'+(i-10);
        auto action = windowMenu->addAction("&"+QString(umpersanded)+" "+editor->fileName);
        action->setData(i);
        windowGroup->addAction(action);
    }
    connect(windowGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        tabWidget->setCurrentIndex(action->data().toInt());
    });
}

bool TabWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key()==Qt::Key_Escape)
        {
            if (tabWidget->currentWidget())
                tabWidget->currentWidget()->setFocus();

        }
        else if (keyEvent->key()>=Qt::Key_0 && keyEvent->key()<=Qt::Key_9)
        {
            insertDate();
        }
        if (keyEvent->key()==Qt::Key_Tab && keyEvent->modifiers() == Qt::ControlModifier)
        {
            tabControlPressed = true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (tabControlPressed /*&& keyEvent->key()==Qt::Key_Control*/)
        {
            tabWidget->tabBar()->moveTab(tabWidget->currentIndex(), 0);
            tabControlPressed = false;
        }
    }
    return false;
}

void TabWindow::closeTab(int index)
{
    QWidget *tab = tabWidget->widget(index);
    CodeEditor* editor = dynamic_cast<CodeEditor*>(tab);
    ConfigFile *configFile = new ConfigFile;
    ConfigFile *existed = nullptr;
    if (editor->fileName!="")
    {
        int n = config.findInHandy(editor->fileName);
        if (n>=0)
        {
            existed = dynamic_cast<ConfigFile*>(config.handy[n]);
        }
        else {
            int n = config.findInMru(editor->fileName);
            if (n>=0)
                existed = dynamic_cast<ConfigFile*>(config.mru[n]);
        }
    }
    if (existed)
        *configFile = *existed;
    configFile->path = editor->fileName;
    configFile->row = editor->textCursor().blockNumber();
    configFile->col = editor->textCursor().positionInBlock();
    QDateTime current = QDateTime::currentDateTime();
    configFile->closingTime = current.toMSecsSinceEpoch() ;
    if (editor->lastEditTime>0)
        configFile->lastEditTime = editor->lastEditTime;
    configFile->wordWrap = editor->wordWrapMode()!=QTextOption::NoWrap;
    configFile->syntax = editor->highlighter->definition().name();
    if (editor->fileName!="")
        config.addClosedFile(configFile);
    else
    {
        QString numStr = tabWidget->tabText(index).mid(8);//"Untitled"+number
        int n = numStr.toInt();
        untitledNumbers.clearBit(n-1);
    }
    disconnect( tab, 0, 0, 0 );
    tab->close();
    delete tab;
    tab = nullptr;
    setWindowTitle("");

    mruMenu->clear();
    addFilelistToMenu(mruMenu, config.mru);
    mruMenu->addSeparator();
    auto removeObsoleteAction = mruMenu->addAction("Remove obsolete");
    connect(removeObsoleteAction, SIGNAL(triggered()), this, SLOT(removeObsoleteMru()));

    handyMenu->clear();
    addFilelistToMenu(handyMenu, config.handy);
}


void TabWindow::about()
{
    QMessageBox::information(nullptr, "About","QEdytor 1.0", QMessageBox::Ok);
}

void TabWindow::removeObsoleteMru()
{
    QList<ConfigItem*> newList;
    for (int i=0; i<config.mru.size(); i++)
    {
        QFileInfo check_file(config.mru[i]->path);
        if (check_file.exists() && check_file.isFile())
            newList.append(config.mru[i]);
    }
    config.mru = newList;
    mruMenu->clear();
    addFilelistToMenu(mruMenu, config.mru);
    mruMenu->addSeparator();
    auto removeObsoleteAction = mruMenu->addAction("Remove obsolete");
    connect(removeObsoleteAction, SIGNAL(triggered()), this, SLOT(removeObsoleteMru()));
}

void TabWindow::setTab(int n)
{
    if (n<tabWidget->count())
        tabWidget->setCurrentIndex(n);
}

void TabWindow::aboutToQuit()
{
    config.geometry = saveGeometry();
    config.saveToFile();
}
