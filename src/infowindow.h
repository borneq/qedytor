#ifndef INFOWINDOW_H
#define INFOWINDOW_H
#include <QDialog>
#include "codeeditor.h"
#include <repository.h>

namespace qedytor {
class InfoWindow : public QDialog
{
    Q_OBJECT
    CodeEditor* editor;
public:
    explicit InfoWindow(QWidget *parent, syntaxhl::Repository *repository);
    void clear();
    void addLine(QString line);
    void describeProperties(const Properties &properties);
signals:

public slots:
};
}
#endif // INFOWINDOW_H
