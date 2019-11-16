#include <qlineedit.h>
#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    connect(ui->OkButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(ui->CancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->cbReplace, SIGNAL(clicked()), this, SLOT(setReplace()));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setReplace()
{
    bReplace = ui->cbReplace->checkState()== Qt::CheckState::Checked;
    if (bReplace) {
        ui->OkButton->setText("&Replace");
        ui->ReplaceAllButton->setVisible(true);
    }
    else {
        ui->OkButton->setText("&Find");
        ui->ReplaceAllButton->setVisible(false);
    }
}

int SearchDialog::exec()
{
    QLineEdit *edit = ui->comboBox->lineEdit();
    if (initial!="")
    {
        edit->setText(initial);
    }
    ui->cbReplace->setCheckState(bReplace?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    setReplace();
    if (bReplace)
    {
        ui->comboBoxR->lineEdit()->setFocus();
    }
    else {
        if (edit->text()=="")
            edit->setFocus();
    }
    return QDialog::exec();
}

void SearchDialog::find()
{
    ui->OkButton->setFocus();
    QLineEdit *edit = ui->comboBox->lineEdit();
    textToFind = edit->text();
    QLineEdit *editR = ui->comboBoxR->lineEdit();
    textToReplace = editR->text();
    unsigned f = 0;
    if (ui->cbCaseSensitive->checkState()==Qt::CheckState::Checked)
        f |= QTextDocument::FindFlag::FindCaseSensitively;
    if (ui->cbWholeWords->checkState()==Qt::CheckState::Checked)
        f |= QTextDocument::FindFlag::FindWholeWords;
    if (ui->cbBackwards->checkState()==Qt::CheckState::Checked)
        f |= QTextDocument::FindFlag::FindBackward;
    flags = QTextDocument::FindFlag(f);
    isRegular = ui->cbRegular->checkState()==Qt::CheckState::Checked;
    initial = textToFind;
    QDialog::accept();
}

void SearchDialog::cancel()
{
    QLineEdit *edit = ui->comboBox->lineEdit();
    initial = edit->text();
    QDialog::reject();
}
