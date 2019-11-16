#include <definition.h>
#include <iostream>
#include <QtWidgets>
#include <QKeyEvent>
#include <QTextDocument>
#include <QStatusBar>
#include <synexception.h>
#include <syntaxhighlighter.h>
#include "mainwindow.h"
#include "codeeditor.h"
#include "edytorexception.h"

using namespace qedytor;

MainWindow::MainWindow(QWidget *parent)
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

void MainWindow::reReadRepository()
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

void MainWindow::handleMessage(const QString& message)
{
    showNormal();//bring window to top on OSX
    raise();//bring window from minimized state on OSX
    activateWindow();//bring window to front/unminimize on windows
    QStringList arguments = message.split("\n");
    for(int i=0; i<arguments.size(); i++)
        if (arguments[i]!="")
            openOrActivateFile(arguments[i]);
}

void MainWindow::currentChanged(int index)
{
    if (tabWidget->currentWidget())
        setWindowTitle(((CodeEditor*)(tabWidget->currentWidget()))->fileName);
}

MainWindow::~MainWindow()
{
    delete repository;
}

bool MainWindow::openOrActivateFile(const QString& aFilePath)
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
                QTextCursor text_cursor = editor->textCursor();
                int row = text_cursor.blockNumber();
                int col = text_cursor.positionInBlock();
                editor->openFile(filePath);
                tabWidget->setCurrentIndex(i);
                text_cursor = editor->textCursor();
                QTextBlock text_block = editor->document()->findBlockByNumber(row);
                text_cursor.setPosition(text_block.position()+col);
                editor->setTextCursor(text_cursor);
                editor->setFocus();
                return false;
            }
        }
        newEditor = new CodeEditor(repository, filePath, this);
        newEditor->openFile(filePath);
        int n = config.findInMru(filePath);
        if (n>=0)
            config.removeMru(n);
    }
    tabWidget->addTab(newEditor, title);
    tabWidget->setCurrentWidget(newEditor);
    newEditor->setCorrectCursorPosition(config);
    newEditor->setFocus();
    connect(newEditor, &QPlainTextEdit::textChanged, newEditor, &CodeEditor::onTextChanged);
    return true;
}

bool lessThanLastEditTime(const ConfigItem *it1, const ConfigItem *it2)
{
    ConfigFile *f1 = (ConfigFile*)it1;
    ConfigFile *f2 = (ConfigFile*)it2;
    if (f1->lastEditTime==f2->lastEditTime)
        return f1->path < f2->path;
    else
        return f1->lastEditTime > f2->lastEditTime;
}

bool lessThanClosingTime(const ConfigItem *it1, const ConfigItem *it2)
{
    ConfigFile *f1 = (ConfigFile*)it1;
    ConfigFile *f2 = (ConfigFile*)it2;
    if (f1->closingTime==f2->closingTime)
        return f1->path < f2->path;
    else
        return f1->closingTime > f2->closingTime;
}

bool lessThanPath(const ConfigItem *it1, const ConfigItem *it2)
{
    ConfigFile *f1 = (ConfigFile*)it1;
    ConfigFile *f2 = (ConfigFile*)it2;
    return f1->path < f2->path;
}

bool lessThanName(const ConfigItem *it1, const ConfigItem *it2)
{
    ConfigFile *f1 = (ConfigFile*)it1;
    ConfigFile *f2 = (ConfigFile*)it2;
    if (f1->name == f2->name)
        return f1->path < f2->path;
    else
        return f1->name < f2->name;
}

void MainWindow::addFilelistToMenu(QMenu* menu, QList<ConfigItem*> &configlist, SortBy sortBy)
{
    QList<ConfigItem*> sortedList = configlist;
    switch (sortBy)
    {
        case(SortBy::name): std::sort(sortedList.begin(), sortedList.end(), lessThanName); break;
        case(SortBy::path): std::sort(sortedList.begin(), sortedList.end(), lessThanPath); break;
        case(SortBy::closingTime): std::sort(sortedList.begin(), sortedList.end(), lessThanClosingTime); break;
        case(SortBy::lastEditTime): std::sort(sortedList.begin(), sortedList.end(), lessThanLastEditTime); break;
    }
    auto mruGroup = new QActionGroup(menu);
    mruGroup->setExclusive(true);
    for (int i=0; i<sortedList.size(); i++)
    {
        auto action = menu->addAction(sortedList[i]->path);
        action->setData(sortedList[i]->path);
        mruGroup->addAction(action);
    }
    connect(mruGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        openOrActivateFile(action->data().toString());
    });
}

void MainWindow::closeCurrentTab()
{
    if (tabWidget->currentWidget())
        closeTab(tabWidget->currentIndex());
}

void MainWindow::insertDate()
{
    if (tabWidget->currentWidget())
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->currentWidget());
        QDate today = QDate::currentDate();
        editor->insertPlainText(today.toString("yyyy-MM-dd dddd"));
    }
}

void MainWindow::insertTime()
{
    if (tabWidget->currentWidget())
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->currentWidget());
        QTime now = QTime::currentTime();
        editor->insertPlainText(now.toString(" hh:mm"));
    }
}

void MainWindow::find()
{
    if (tabWidget->currentWidget())
    {
        CodeEditor* editor = dynamic_cast<CodeEditor*>(tabWidget->currentWidget());
        QString selected = editor->textCursor().selectedText();
        QChar endl(8233);
        if (selected!="" && !selected.contains(endl))
        {
            searchDialog.initial = selected;
        }
        int result = searchDialog.exec();
        if (result == QDialog::Accepted)
        {
            editor->findNext(searchDialog.textToFind, searchDialog.flags, searchDialog.isRegular);
        }
    }
}

void MainWindow::findNext()
{
    if (tabWidget->currentWidget())
    {
       dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->findNext(searchDialog.textToFind, QTextDocument::FindFlag((int)searchDialog.flags & ~(int)QTextDocument::FindBackward), searchDialog.isRegular);
    }
}

void MainWindow::findPrev()
{
    if (tabWidget->currentWidget())
    {
       dynamic_cast<CodeEditor*>(tabWidget->currentWidget())->findNext(searchDialog.textToFind, QTextDocument::FindFlag((int)searchDialog.flags | (int)QTextDocument::FindBackward), searchDialog.isRegular);
    }
}

void MainWindow::properties()
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

void MainWindow::openConfigDialog()
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

void MainWindow::openConfigDialogModal()
{
    ConfigDialog *configDialog = new ConfigDialog(&config, this);
    configDialog->exec();
    delete configDialog;
}

void MainWindow::onDestroyConfigDialog()
{
    configDialog = nullptr;
}

void MainWindow::onDestroyInfoWindow()
{
    infoWindow = nullptr;
}

void MainWindow::onChangedIni()
{
    reReadRepository();
    menuBar()->clear();
    createMenu();
}

void MainWindow::callSaveAs()
{
    if (tabWidget->currentWidget())
    {
        const auto fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Save as File"));
        if (!fileName.isEmpty())
        {
            CodeEditor *editor = dynamic_cast<CodeEditor *>(tabWidget->currentWidget());
            QFileInfo fi(fileName);
            editor->fileName = fi.absoluteFilePath();
            editor->saveFile(fileName, true);
            tabWidget->setTabText(tabWidget->currentIndex(), fi.fileName());
        }
    }
}

void MainWindow::createMenu()
{
    newAction = new QAction(tr("New"), this);
    newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    openAction = new QAction(tr("Open file..."), this);
    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    saveasAction = new QAction(tr("save &As"), this);
    exitAction = new QAction(tr("Exit"), this);
    QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);
    closeAction = new QAction("Close", this);
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
                editor->saveFile(editor->fileName, false);
            else
                callSaveAs();
        }
    });
    connect(saveasAction, &QAction::triggered, this, [this]() {
        callSaveAs();
    });
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
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

    fileMenu = menuBar()->addMenu(tr("&File"));
    connect(fileMenu, SIGNAL(aboutToShow()), this, SLOT(showMenuFile()));
    QMenu* searchMenu = menuBar()->addMenu(tr("&Search"));
    searchMenu->addAction(findAction);
    searchMenu->addAction(findNextAction);
    searchMenu->addAction(findPrevAction);
    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(configAction);
    toolsMenu->addAction(insertDateAction);
    toolsMenu->addAction(insertTimeAction);
    toolsMenu->addAction(propertiesAction);

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
    connect(handyMenu, SIGNAL(aboutToShow()), this, SLOT(showMenuHandy()));

    QMenu* helpMenu = menuBar()->addMenu(tr("&About"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
    QKeyEvent event1(QEvent::KeyPress, Qt::Key_F, Qt::AltModifier, "f");
    QApplication::sendEvent(menuBar(), &event1);
    QKeyEvent event2(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier, "");
    QApplication::sendEvent(menuBar(), &event2);
}

void MainWindow::showMenuWindow()
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

void MainWindow::showMenuFile()
{
    fileMenu->clear();
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    QMenu *mruMenu = fileMenu->addMenu(tr("&Recent files"));
    addFilelistToMenu(mruMenu, config.mru, SortBy::closingTime);
    mruMenu->addSeparator();
    auto removeObsoleteAction = mruMenu->addAction("Remove obsolete");
    connect(removeObsoleteAction, SIGNAL(triggered()), this, SLOT(removeObsoleteMru()));
    QMenu *mruNamesMenu = fileMenu->addMenu(tr("&According to name"));
    addFilelistToMenu(mruNamesMenu, config.mru, SortBy::name);
    QMenu *mruPathesMenu = fileMenu->addMenu(tr("&According to path"));
    addFilelistToMenu(mruPathesMenu, config.mru, SortBy::path);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveasAction);
    fileMenu->addAction(closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
}

void MainWindow::showMenuHandy()
{
    handyMenu->clear();
    QMenu *lastClosedMenu = handyMenu->addMenu(tr("&Last closed"));
    addFilelistToMenu(lastClosedMenu, config.handy, SortBy::closingTime);
    QMenu *lastEditedMenu = handyMenu->addMenu(tr("&Last edited"));
    addFilelistToMenu(lastEditedMenu, config.handy, SortBy::lastEditTime);
    QMenu *namesMenu = handyMenu->addMenu(tr("&Names"));
    addFilelistToMenu(namesMenu, config.handy, SortBy::name);
    QMenu *pathesMenu = handyMenu->addMenu(tr("&Pathes"));
    addFilelistToMenu(pathesMenu, config.handy, SortBy::path);
    QMenu *unsortedMenu = handyMenu->addMenu(tr("&Unsorted"));
    addFilelistToMenu(unsortedMenu, config.handy, SortBy::original);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
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

void MainWindow::closeTab(int index)
{
    QWidget *tab = tabWidget->widget(index);
    CodeEditor* editor = dynamic_cast<CodeEditor*>(tab);
    if (editor->document()->isModified())
    {
        QMessageBox::warning(nullptr, "Warning", "Text not saved!",
                                   QMessageBox::Ok);
        return;
    }
    ConfigFile *configFile = new ConfigFile;
    ConfigFile *existed = config.cfFindInHandyOrMru(editor->fileName);
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
    config.saveToFile();
}


void MainWindow::about()
{
    QMessageBox::information(nullptr, "About","QEdytor {>1.0}", QMessageBox::Ok);
}

void MainWindow::removeObsoleteMru()
{
    QList<ConfigItem*> newList;
    for (int i=0; i<config.mru.size(); i++)
    {
        QFileInfo check_file(config.mru[i]->path);
        if (check_file.exists() && check_file.isFile())
            newList.append(config.mru[i]);
    }
    config.mru = newList;
}

void MainWindow::setTab(int n)
{
    if (n<tabWidget->count())
        tabWidget->setCurrentIndex(n);
}

void MainWindow::aboutToQuit()
{
    config.geometry = saveGeometry();
    config.saveToFile();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i=0; i<tabWidget->count(); i++)
    {
        CodeEditor *editor = qobject_cast<CodeEditor *>(tabWidget->widget(i));
        if (editor->document()->isModified())
        {
            tabWidget->setCurrentIndex(i);
            QMessageBox::warning(nullptr, "Warning", "Text not saved!",
                                       QMessageBox::Ok);
            editor->setFocus();
            event->ignore();
            return;
        }
    }
    event->accept();
}
