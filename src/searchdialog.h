#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QTextDocument>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();
     QString initial;
     QString textToFind;
     QString textToReplace;
     QTextDocument::FindFlag flags;
     bool bReplace;
     bool bReplaceAll;
     bool isRegular;
     int exec() override;
private slots:
     void find(bool bAll);
     void findReplace();
     void replaceAll();
     void cancel();
     void setReplace();
private:
    Ui::SearchDialog *ui;
};

#endif // SEARCHDIALOG_H
