#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QComboBox>

namespace qedytor {
class SearchDialog : public QDialog
{
    Q_OBJECT
    QComboBox *createComboBox(const QString &text);
    QComboBox *textComboBox;
private slots:
    void cancel();
    void find();
public:
    QString textToFind;
    explicit SearchDialog(QWidget *parent = nullptr);

signals:

public slots:
};
}
#endif // SEARCHDIALOG_H
