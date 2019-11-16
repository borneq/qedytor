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
    ui->comboBox->lineEdit()->setFocus();
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

int SearchDialog::exec()
{
    if (initial!="")
    {
        QLineEdit *edit = ui->comboBox->lineEdit();
        edit->setText(initial);
    }
    return QDialog::exec();
}
void SearchDialog::find()
{
    QLineEdit *edit = ui->comboBox->lineEdit();
    textToFind = edit->text();
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
