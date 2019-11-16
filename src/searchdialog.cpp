#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    connect(ui->OkButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(ui->CancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}
void SearchDialog::find()
{
    textToFind = "a";
    QDialog::accept();
}

void SearchDialog::cancel()
{
    QDialog::reject();
}
