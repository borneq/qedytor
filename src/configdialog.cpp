#include "configdialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QInputDialog>

using namespace qedytor;

QString ConfigDialog::browse(QString defPath)
{
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), defPath));

    if (directory.isEmpty())
        return "";
    else
        return directory;
}

void ConfigDialog::load(Config *config)
{
    syntaxPath->setText(config->pathToSyntax);
    themesPath->setText(config->pathToThemes);
}

void ConfigDialog::save(Config *config)
{
    config->pathToSyntax = syntaxPath->text();
    config->pathToThemes = themesPath->text();
    config->saveToFile();
    emit changedIni();
}

ConfigDialog::~ConfigDialog()
{
}


void ConfigDialog::addRecord()
{
    if (tabList->currentIndex()==0)
    {
        QString path = QInputDialog::getText(this, "Editor", "Get path", QLineEdit::Normal);
        QFileInfo fi(path);
        ConfigEditor *ce = new ConfigEditor;
        ce->name = fi.fileName();
        ce->path = fi.absoluteFilePath();
        config->otherEditors.append(ce);
        listEditors->update();
    }
    else if (tabList->currentIndex()==1)
    {
        QString path = QInputDialog::getText(this, "Handy", "Get path", QLineEdit::Normal);
        QFileInfo fi(path);
        ConfigFile *cf = new ConfigFile;
        cf->path = fi.absoluteFilePath();
        config->handy.append(cf);
        listHandy->update();
    }
    else if (tabList->currentIndex()==2)
    {
        QString path = QInputDialog::getText(this, "Mru", "Get path", QLineEdit::Normal);
        QFileInfo fi(path);
        ConfigFile *cf = new ConfigFile;
        cf->path = fi.absoluteFilePath();
        config->mru.append(cf);
        listMru->update();
    }
}

void ConfigDialog::deleteRecord()
{
    if (tabList->currentIndex()==0)
    {
        QItemSelectionModel *select = listEditors->selectionModel();
        if (!select->hasSelection())
            QMessageBox::warning(nullptr, "Warning", "Select row(s)",
                                       QMessageBox::Ok);
        else {
            QModelIndexList list = select->selectedRows();
            for (int i = list.count()-1; i>=0; i--)
            {
                QModelIndex index = list.at(i);
                config->otherEditors.removeAt(index.row());
            }
            listEditors->update();
        }
    }
    else if (tabList->currentIndex()==1)
    {
        QItemSelectionModel *select = listHandy->selectionModel();
        if (!select->hasSelection())
            QMessageBox::warning(nullptr, "Warning", "Select row(s)",
                                       QMessageBox::Ok);
        else {
            QModelIndexList list = select->selectedRows();
            for (int i = list.count()-1; i>=0; i--)
            {
                QModelIndex index = list.at(i);
                config->handy.removeAt(index.row());
            }
            listHandy->update();
        }
    }
    else if (tabList->currentIndex()==2)
    {
        QItemSelectionModel *select = listMru->selectionModel();
        if (!select->hasSelection())
            QMessageBox::warning(nullptr, "Warning", "Select row(s)",
                                       QMessageBox::Ok);
        else {
            QModelIndexList list = select->selectedRows();
            for (int i = list.count()-1; i>=0; i--)
            {
                QModelIndex index = list.at(i);
                config->mru.removeAt(index.row());
            }
            listMru->update();
        }
    }
}

void ConfigDialog::editRecord()
{
    QMessageBox::warning(nullptr, "Warning", "Edit: Not implemented yet",
                               QMessageBox::Ok);
}

ConfigDialog::ConfigDialog(Config *config, QWidget *parent): config(config), QDialog(parent)
{
    listEditors = new ConfigTableView(&config->otherEditors, &config->configEditorHeaders, this);
    listHandy = new ConfigTableView(&config->handy, &config->configFilesHeaders, this);
    listMru = new ConfigTableView(&config->mru, &config->configFilesHeaders, this);
    QGridLayout *mainLayout = new QGridLayout(this);

    mainLayout->addWidget(new QLabel(tr("Config file:")), 0, 0);
    mainLayout->addWidget(new QLabel(config->getConfigPath()), 0, 1, 1,3);

    tabList = new QTabWidget(this);
    tabList->addTab(listEditors,"Editors");
    tabList->addTab(listHandy,"Handy");
    tabList->addTab(listMru,"Mru");
    mainLayout->addWidget(tabList, 1, 0, 1,4);

    QPushButton *addButton = new QPushButton(tr("&Add"), this);
    QPushButton *editButton = new QPushButton(tr("&Edit"), this);
    QPushButton *deleteButton = new QPushButton(tr("&Delete"), this);
    mainLayout->addWidget(addButton, 2, 1);
    mainLayout->addWidget(editButton, 2, 2);
    mainLayout->addWidget(deleteButton, 2, 3);
    connect(addButton,  SIGNAL(clicked()), this, SLOT(addRecord()));
    connect(editButton,  SIGNAL(clicked()), this, SLOT(editRecord()));
    connect(deleteButton,  SIGNAL(clicked()), this, SLOT(deleteRecord()));

    syntaxPath = new QLineEdit(this);
    themesPath = new QLineEdit(this);
    configRef = config;
    load(config);
    QPushButton *browseSyntaxButton = new QPushButton(tr("&Browse..."), this);
    QPushButton *browseThemeButton = new QPushButton(tr("&Browse..."), this);

    connect(browseSyntaxButton,  &QAbstractButton::clicked, this, [this, config]
    {
        QString pathToset = "";
        if (syntaxPath->text().isEmpty())
        {
            if (themesPath->text().isEmpty())
                pathToset = browse(config->getConfigPath());
            else
                pathToset = browse(themesPath->text());
        }
        else
            pathToset = browse(syntaxPath->text());
        syntaxPath->setText(pathToset);
    });
    connect(browseThemeButton,  &QAbstractButton::clicked, this, [this, config]
    {
        QString pathToset = "";
        if (themesPath->text().isEmpty())
        {
            if (syntaxPath->text().isEmpty())
                pathToset = browse(config->getConfigPath());
            else
                pathToset = browse(syntaxPath->text());
        }
        else
            pathToset = browse(themesPath->text());
        themesPath->setText(pathToset);
    });
    QPushButton *btnOk = new QPushButton(tr("OK"), this);
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(okSlot()));
    QPushButton *btnCancel = new QPushButton(tr("Cancel"), this);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QPushButton *btnApply = new QPushButton(tr("Apply"), this);
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));

    mainLayout->addWidget(new QLabel(tr("Syntax:")), 3, 0);
    mainLayout->addWidget(syntaxPath, 3, 1, 1, 2);
    mainLayout->addWidget(browseSyntaxButton, 3, 3);

    mainLayout->addWidget(new QLabel(tr("Themes:")), 4, 0);
    mainLayout->addWidget(themesPath, 4, 1, 1, 2);
    mainLayout->addWidget(browseThemeButton, 4, 3);

    mainLayout->addWidget(btnOk, 5, 1);
    mainLayout->addWidget(btnCancel, 5, 2);
    mainLayout->addWidget(btnApply, 5, 3);
}

void ConfigDialog::apply()
{
    if (checkCorrectness())
    {
        save(configRef);
    }
}

void ConfigDialog::okSlot()
{
    if (checkCorrectness())
    {
        save(configRef);
        close();
    }
}

void ConfigDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        close();
    }
}

bool ConfigDialog::checkCorrectness()
{
    if (syntaxPath->text().isEmpty())
    {
        QMessageBox::warning(nullptr, "Warning","Syntax path is not set",
                                   QMessageBox::Ok);
        return false;
    }
    if (themesPath->text().isEmpty())
    {
        QMessageBox::warning(nullptr, "Warning","Themes path is not set",
                                   QMessageBox::Ok);
        return false;
    }
    QDir syntaxDir(syntaxPath->text());
    QFileInfo fiKateIndex(syntaxDir.filePath("index.katesyntax.json"));
    if (!fiKateIndex.exists())
    {
        QMessageBox::warning(nullptr, "Warning","Not found index.katesyntax.json in syntax path",
                                   QMessageBox::Ok);
        return false;
    }
    QDir themesDir(syntaxPath->text());
    QFileInfo fiDefaultTheme(syntaxDir.filePath("default.theme"));
    if (!fiKateIndex.exists())
    {
        QMessageBox::warning(nullptr, "Warning","Not found default.theme in themes path",
                                   QMessageBox::Ok);
        return false;
    }
    return true;
}
