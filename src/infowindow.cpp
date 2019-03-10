#include "infowindow.h"
#include <QVBoxLayout>

using namespace qedytor;

InfoWindow::InfoWindow(QWidget *parent, syntaxhl::Repository *repository) : QDialog(parent)
{
    editor = new CodeEditor(repository, "", this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(editor);
    setLayout(mainLayout);
}

void InfoWindow::clear()
{
    editor->clear();
}

void InfoWindow::addLine(QString line)
{
    editor->appendPlainText(line);
}

void InfoWindow::describeProperties(const Properties &properties)
{
    clear();
    if (properties.isLocale)
        addLine("Locale");
    if (properties.isUtfBOM)
        addLine("UTF BOM");
    if (properties.isUtfDetected)
        addLine("Detected UTF8");
}
