/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QStatusBar>
#include <QLabel>
#include "repository.h"
#include "properties.h"
#include "codeeditorsidebar.h"
#include "config.h"

#define STATUSBAR 1

namespace syntaxhl {
    class SyntaxHighlighter;
}

namespace qedytor {
enum class UtfKind {Bom, detected, locale, error};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
private:
    friend class CodeEditorSidebar;
    void setTheme(const syntaxhl::Theme &theme);
    int sidebarWidth() const;
    void sidebarPaintEvent(QPaintEvent *event);
    void updateBarsGeometry();
    void updateSidebarArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void updateStatusBar();

    QObject *getMainWindow();

    QTextBlock blockAtPosition(int y) const;
    bool isFoldable(const QTextBlock &block) const;
    bool isFolded(const QTextBlock &block) const;
    void toggleFold(const QTextBlock &block);
    static int countNonAscii(QByteArray &content);

    syntaxhl::Repository *m_repositoryRef;
    CodeEditorSidebar *m_sideBar;

    bool eventFilter(QObject *watched, QEvent *event);
    std::string een_password;
#if STATUSBAR
    QStatusBar *m_statusBar;
    QLabel *statusLabel0;
    QLabel *statusLabel1;
    QLabel *statusLabel2;
#endif
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
public slots:
    void onTextChanged();
public:
    syntaxhl::SyntaxHighlighter *highlighter;
    UtfKind loadUtfKind;
    UtfKind saveUtfKind;
    bool showLineNumbers = false;
    explicit CodeEditor(syntaxhl::Repository *repository,
                        QString fileName, QWidget *parent = nullptr);
    ~CodeEditor() override;
    QString fileName;
    qint64 lastEditTime = 0;
    void openFile(const QString &fileName);
    void saveFile(const QString& fileName, bool isSaveAs);
    bool findNext(QString textToFind, QTextDocument::FindFlags flags, bool findRegular);
    bool replaceNext(QString textToFind, QString textToReplace, QTextDocument::FindFlags flags, bool findRegular);
    Properties getProperties();
    void setCorrectCursorPosition(Config &config);
};
}
#endif // CODEEDITOR_H
