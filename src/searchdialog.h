#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();
     QString textToFind;
private slots:
    void find();
    void cancel();
private:
    Ui::SearchDialog *ui;
};

#endif // SEARCHDIALOG_H
