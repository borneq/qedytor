#include "searchdialog.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

using namespace qedytor;

QComboBox *SearchDialog::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}

void SearchDialog::find()
{
    textToFind = textComboBox->lineEdit()->text();
    QDialog::accept();
}

void SearchDialog::cancel()
{
    QDialog::reject();
}

SearchDialog::SearchDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Find Files"));
    QPushButton *findButton = new QPushButton(tr("&Find"), this);
    findButton->setDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"), this);
    connect(findButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    textComboBox = createComboBox("");
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(textComboBox, 0, 1, 1, 2);
    mainLayout->addWidget(findButton, 1, 2);
    mainLayout->addWidget(cancelButton, 1, 3);
    textComboBox->setFocus();
}
