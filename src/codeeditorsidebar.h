#ifndef CODEEDITORSIDEBAR_H
#define CODEEDITORSIDEBAR_H

#include <QWidget>

namespace qedytor {
class CodeEditor;

class CodeEditorSidebar : public QWidget
{
    Q_OBJECT
public:
    explicit CodeEditorSidebar(CodeEditor *editor);
    QSize sizeHint() const Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    CodeEditor *m_codeEditor;
};
}
#endif // CODEEDITORSIDEBAR_H
