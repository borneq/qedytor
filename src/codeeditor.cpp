#include "codeeditor.h"
#include "een.h"

#include <definition.h>
#include <foldingregion.h>
#include <syntaxhighlighter.h>
#include <theme.h>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextBlock>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QRegExp>
#include <QMessageBox>
#include <QTextCodec>
#include <QDateTime>
#include <QInputDialog>
#include <QLineEdit>
#include <QScrollBar>
#include "tabcontrolable.h"
#include "edytorexception.h"
#include "tabwindow.h"


using namespace qedytor;

CodeEditor::CodeEditor(syntaxhl::Repository *repository,QString name, QWidget *parent):  QPlainTextEdit(parent),
    m_repositoryRef(repository),
    highlighter(new syntaxhl::SyntaxHighlighter(document())),
    m_sideBar(new CodeEditorSidebar(this))
#if STATUSBAR
  ,m_statusBar(new QStatusBar(this))
#endif
{
    this->fileName = name;
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    setTheme((palette().color(QPalette::Base).lightness() < 128)
        ? m_repositoryRef->defaultTheme(syntaxhl::Repository::DarkTheme)
        : m_repositoryRef->defaultTheme(syntaxhl::Repository::LightTheme));

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateBarsGeometry);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateSidebarArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::updateStatusBar);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::updateStatusBar);
#if STATUSBAR
    m_statusBar->setStyleSheet("border-top: 1px solid lightgray");
    m_statusBar->setSizeGripEnabled(false);
    statusLabel1 = new QLabel(this);
    statusLabel1->setMinimumWidth(100);
    m_statusBar->addWidget(statusLabel1);
    statusLabel2 = new QLabel(this);
    statusLabel2->setMinimumWidth(100);
    m_statusBar->addWidget(statusLabel2);
    statusLabel0 = new QLabel(this);
    statusLabel0->setMinimumWidth(10);
    m_statusBar->addWidget(statusLabel0, 1);
#endif
    updateBarsGeometry();
    highlightCurrentLine();
    installEventFilter(this);
}

CodeEditor::~CodeEditor()
{
}

int CodeEditor::countNonAscii(QByteArray &content)
{
    int count = 0;
    for (int i=0; i<content.size(); i++)
        if (content.at(i)>127)
            count++;
    return count;
}

void CodeEditor::setCorrectCursorPosition(Config &config)
{
    if (fileName!="")
    {
        ConfigFile *existed = config.cfFindInHandyOrMru(fileName);
        if (existed)
        {
            QTextBlock text_block = document()->findBlockByNumber(existed->row);
            QTextCursor text_cursor = textCursor();
            text_cursor.setPosition(text_block.position()+existed->col);
            setTextCursor(text_cursor);
        }
    }
}

void CodeEditor::openFile(const QString& fileName)
{
    clear();
    const auto def = m_repositoryRef->definitionForFileName(fileName);
    //if (def.foldingEnabled())
    highlighter->setDefinition(def);

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << fileName << ":" << f.errorString();
        return;
    }

    QByteArray content = f.readAll();
    QRegExp rx("*.een");
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (rx.exactMatch(fileName))
    {
        Een een;
        QString password = QInputDialog::getText(this, "Crypted", "Get password", QLineEdit::Password);
        try {
            content = een.decrypt(content, password.toStdString());
            een_password = password.toStdString();
        } catch (EdytorException &ex) {
            QMessageBox::critical(nullptr, "Error",ex.what(), QMessageBox::Ok);
            content.clear();
            een_password = "";
        }

    }

    QString string;
    QTextCodec *codecBOM = QTextCodec::codecForUtfText(content.mid(0,4), QTextCodec::codecForName("System"));
    if (!codecBOM || codecBOM->name()=="System")
    {
        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        string = codec->toUnicode(content, content.size(), &state);
        int count = countNonAscii(content);
        if (state.invalidChars > count/10)
        {
            QTextCodec *codecLocale = QTextCodec::codecForLocale();
            if (codecLocale)
            {
                string = codecLocale->toUnicode(content);
                loadUtfKind = UtfKind::locale;
            }
            else
            {
                string = "";
                loadUtfKind = UtfKind::error;
            }
        }
        else loadUtfKind = UtfKind::detected;
    }
    else
    {
        string = codecBOM->toUnicode(content);
        loadUtfKind = UtfKind::Bom;
    }
    saveUtfKind = loadUtfKind;
    setPlainText(string);
#ifdef _WIN32
    QFont font("Courier New");
#else
    QFont font("Bitstream Vera Sans Mono");
#endif
    font.setPointSize(10);
    setFont(font);
}

void CodeEditor::saveFile(const QString& fileName)
{
    QTextCodec *codec;
    if (saveUtfKind==UtfKind::locale)
    {
        codec = QTextCodec::codecForLocale();
    }
    else {
        codec = QTextCodec::codecForName("UTF-8");
    }
    QTextCodec::ConverterState state;
    QString text = toPlainText();
    QByteArray content = codec->fromUnicode(text);
    if (state.invalidChars>0)
    {
        QMessageBox::warning(nullptr, "Warning","Can't convert from Unicode",
                                   QMessageBox::Ok);
        return;
    }
    if (saveUtfKind==UtfKind::locale && loadUtfKind!=UtfKind::locale)
    {
        QString againUTF = codec->toUnicode(content);
        if (againUTF!=text)
        {
            QMessageBox::warning(nullptr, "Warning","Can't convert from Unicode",
                                       QMessageBox::Ok);
            return;
        }
    }
    QRegExp rx("*.een");
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (rx.exactMatch(fileName))
    {
        Een een;
        content = een.encrypt(content, een_password);
        QFile f(fileName);
        if (!f.open(QFile::WriteOnly)) {
            qWarning() << "Failed to open" << fileName << ":" << f.errorString();
            return;
        }
        f.write(content);
        document()->setModified(false);
        updateStatusBar();
    }
    else {
        QFile f(fileName);
        if (!f.open(QFile::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << fileName << ":" << f.errorString();
            return;
        }
        if (saveUtfKind==UtfKind::Bom)
        {
            QByteArray bom =  QByteArrayLiteral("\xef\xbb\xbf");
            f.write(bom);
        }
        f.write(content);
        document()->setModified(false);
        updateStatusBar();
    }
    loadUtfKind = saveUtfKind;
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    QAction *action;

    action = new QAction(tr("Word wrap"), this);
    action->setCheckable(true);
    action->setChecked(wordWrapMode()!=QTextOption::NoWrap);
    connect(action, &QAction::changed, this,
            [this,action]() {
                    action->isChecked()?
                    this->setWordWrapMode(QTextOption::WordWrap):
                    this->setWordWrapMode(QTextOption::NoWrap);
                });
    action->setEnabled(true);
    menu->addAction(action);

    action = new QAction(tr("Line numbers"), this);
    action->setCheckable(true);
    action->setChecked(showLineNumbers);
    connect(action, &QAction::changed, this,
            [this,action]() {
                    showLineNumbers = action->isChecked();
                    updateBarsGeometry();
                });
    action->setEnabled(true);
    menu->addAction(action);

    QAction *actionUtf = new QAction(tr("Save as UTF"), this);
    actionUtf->setCheckable(true);
    actionUtf->setChecked(saveUtfKind!=UtfKind::locale);
    actionUtf->setEnabled(true);
    menu->addAction(actionUtf);

    QAction *actionBOM = new QAction(tr("Save UTF with BOM"), this);
    actionBOM->setCheckable(true);
    actionBOM->setChecked(saveUtfKind==UtfKind::Bom);
    actionBOM->setEnabled(true);
    menu->addAction(actionBOM);

    connect(actionUtf, &QAction::changed, this,
            [this ,actionUtf, actionBOM]() {
                  if (actionUtf->isChecked())
                  {
                      if (actionBOM->isChecked())
                          saveUtfKind = UtfKind::Bom;
                      else
                          saveUtfKind = UtfKind::detected;
                  }
                  else
                      saveUtfKind = UtfKind::locale;
                });

    connect(actionBOM, &QAction::changed, this,
            [this ,actionUtf, actionBOM]() {
                  if (actionUtf->isChecked())
                  {
                      if (actionBOM->isChecked())
                          saveUtfKind = UtfKind::Bom;
                      else
                          saveUtfKind = UtfKind::detected;
                  }
                  else
                      saveUtfKind = UtfKind::locale;
                });

    action = new QAction(tr("&Undo"), this);
    action->setShortcut(QKeySequence(QKeySequence::Undo));
    connect(action, SIGNAL(triggered()), this, SLOT(undo()));
    action->setEnabled(document()->isUndoAvailable());
    menu->addAction(action);

    action = new QAction(tr("&Redo"), this);
    action->setShortcut(QKeySequence(QKeySequence::Redo));
    connect(action, SIGNAL(triggered()), this, SLOT(redo()));
    action->setEnabled(document()->isRedoAvailable());
    menu->addAction(action);

    QMenu *selectionMenu = menu->addMenu(tr("Selection"));

    action = new QAction(tr("&Copy"), this);
    action->setShortcut(QKeySequence(QKeySequence::Copy));
    connect(action, SIGNAL(triggered()), this, SLOT(copy()));
    action->setEnabled(textCursor().hasSelection());
    selectionMenu->addAction(action);

    action = new QAction(tr("&Paste"), this);
    action->setShortcut(QKeySequence(QKeySequence::Paste));
    connect(action, SIGNAL(triggered()), this, SLOT(paste()));
    action->setEnabled(canPaste());
    selectionMenu->addAction(action);

    action = new QAction(tr("&Delete"), this);
    action->setShortcut(QKeySequence(QKeySequence::Delete));
    connect(action, &QAction::triggered, this,
            [this]() {
                    this->insertPlainText("");  //tricky because not delete slot
                });
    action->setEnabled(textCursor().hasSelection());
    selectionMenu->addAction(action);

    selectionMenu->addSeparator();

    action = new QAction(tr("Select &All"), this);
    action->setShortcut(QKeySequence(QKeySequence::SelectAll));
    connect(action, SIGNAL(triggered()), this, SLOT(selectAll()));
    action->setEnabled(!document()->isEmpty());
    selectionMenu->addAction(action);

    // syntax selection
    auto hlActionGroup = new QActionGroup(menu);
    hlActionGroup->setExclusive(true);
    auto hlGroupMenu = menu->addMenu(QStringLiteral("Syntax"));
    QMenu *hlSubMenu = hlGroupMenu;
    QString currentGroup;

    foreach (const auto &def, m_repositoryRef->definitions()) {
        if (def.isHidden())
            continue;
        if (currentGroup != def.section()) {
            currentGroup = def.section();
            hlSubMenu = hlGroupMenu->addMenu(def.translatedSection());
        }

        Q_ASSERT(hlSubMenu);
        auto action = hlSubMenu->addAction(def.translatedName());
        action->setCheckable(true);
        action->setData(def.name());
        hlActionGroup->addAction(action);
        if (def.name() == highlighter->definition().name())
            action->setChecked(true);
    }

    connect(hlActionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        const auto defName = action->data().toString();
        const auto def = m_repositoryRef->definitionForName(defName);
        highlighter->setDefinition(def);
    });

    // theme selection
    auto themeGroup = new QActionGroup(menu);
    themeGroup->setExclusive(true);
    auto themeMenu = menu->addMenu(QStringLiteral("Theme"));
    foreach (const auto &theme, m_repositoryRef->themes()) {
        auto action = themeMenu->addAction(theme.translatedName());
        action->setCheckable(true);
        action->setData(theme.name());
        themeGroup->addAction(action);
        if (theme.name() == highlighter->theme().name())
            action->setChecked(true);
    }
    connect(themeGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        const auto themeName = action->data().toString();
        const auto theme = m_repositoryRef->theme(themeName);
        setTheme(theme);
    });
    menu->exec(event->globalPos());
    delete menu;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updateBarsGeometry();
}

void CodeEditor::setTheme(const syntaxhl::Theme &theme)
{
    auto pal = qApp->palette();
    if (theme.isValid()) {
        pal.setColor(QPalette::Base, theme.editorColor(syntaxhl::Theme::BackgroundColor));
        pal.setColor(QPalette::Text, theme.textColor(syntaxhl::Theme::Normal));
        pal.setColor(QPalette::Highlight, theme.editorColor(syntaxhl::Theme::TextSelection));
    }
    setPalette(pal);

    highlighter->setTheme(theme);
    highlighter->rehighlight();
    highlightCurrentLine();
}

int CodeEditor::sidebarWidth() const
{
    if (showLineNumbers)
    {
        int digits = 1;
        auto count = blockCount();
        while (count >= 10) {
            ++digits;
            count /= 10;
        }
        return 4 + fontMetrics().width(QLatin1Char('9')) * digits + fontMetrics().lineSpacing();
    }
    else
        return fontMetrics().lineSpacing();
}

void CodeEditor::sidebarPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_sideBar);
    painter.fillRect(event->rect(), highlighter->theme().editorColor(syntaxhl::Theme::IconBorder));

    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    const int currentBlockNumber = textCursor().blockNumber();

    const auto foldingMarkerSize = fontMetrics().lineSpacing();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const auto number = QString::number(blockNumber + 1);
            painter.setPen(highlighter->theme().editorColor(
                (blockNumber == currentBlockNumber) ? syntaxhl::Theme::CurrentLineNumber
                                                    : syntaxhl::Theme::LineNumbers));
            painter.drawText(0, top, m_sideBar->width() - 2 - foldingMarkerSize, fontMetrics().height(), Qt::AlignRight, number);
        }

        // folding marker
        if (block.isVisible() && isFoldable(block)) {
            QPolygonF polygon;
            if (isFolded(block)) {
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.25);
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.75);
                polygon << QPointF(foldingMarkerSize * 0.8, foldingMarkerSize * 0.5);
            } else {
                polygon << QPointF(foldingMarkerSize * 0.25, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.75, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.5, foldingMarkerSize * 0.8);
            }
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(highlighter->theme().editorColor(syntaxhl::Theme::CodeFolding)));
            painter.translate(m_sideBar->width() - foldingMarkerSize, top);
            painter.drawPolygon(polygon);
            painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::updateBarsGeometry()
{
#if STATUSBAR
    const int StatusH = 20;
#else
    const int StatusH = 0;
#endif
    setViewportMargins(sidebarWidth(), 0, 0, StatusH);
    const auto r = contentsRect();
    m_sideBar->setGeometry(QRect(r.left(), r.top(), sidebarWidth(), r.height()));
#if STATUSBAR
    if (verticalScrollBar()->isVisible())
        m_statusBar->setGeometry(QRect(r.left(), r.top()+r.height()-StatusH, r.width()-verticalScrollBar()->width(), StatusH));
    else
        m_statusBar->setGeometry(QRect(r.left(), r.top()+r.height()-StatusH, r.width(), StatusH));
#endif
}

void CodeEditor::updateSidebarArea(const QRect& rect, int dy)
{
    if (dy)
        m_sideBar->scroll(0, dy);
    else
        m_sideBar->update(0, rect.y(), m_sideBar->width(), rect.height());
}

void CodeEditor::highlightCurrentLine()
{
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(highlighter->theme().editorColor(syntaxhl::Theme::CurrentLine));
    selection.format.setBackground(lineColor);

    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

QObject *CodeEditor::getMainWindow()
{
    QObject *result = this;
    while(result)
    {
        if (dynamic_cast<TabWindow*>(result) != nullptr)
            return result;
        result = result->parent();
    }
    return result;
}

void CodeEditor::updateStatusBar()
{
    const QTextBlock block = textCursor().block();
    const int relativePos = textCursor().position() - block.position();
    QTabWidget* tabWidget = ((TabWindow*)(getMainWindow()))->tabWidget;
    if (document()->isModified())
        tabWidget->tabBar()->setTabTextColor(tabWidget->currentIndex(), Qt::red);
    else
        tabWidget->tabBar()->setTabTextColor(tabWidget->currentIndex(), Qt::black);

#if STATUSBAR
    statusLabel1->setText(QString::number(block.blockNumber()+1)+":"+QString::number(relativePos+1));
    if (document()->isModified())
        statusLabel2->setText("*");
    else
        statusLabel2->setText("");
#endif
}

QTextBlock CodeEditor::blockAtPosition(int y) const
{
    auto block = firstVisibleBlock();
    if (!block.isValid())
        return QTextBlock();

    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    do {
        if (top <= y && y <= bottom)
            return block;
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    } while (block.isValid());
    return QTextBlock();
}

bool CodeEditor::isFoldable(const QTextBlock &block) const
{
    return highlighter->startsFoldingRegion(block);
}

bool CodeEditor::isFolded(const QTextBlock &block) const
{
    if (!block.isValid())
        return false;
    const auto nextBlock = block.next();
    if (!nextBlock.isValid())
        return false;
    return !nextBlock.isVisible();
}

void CodeEditor::toggleFold(const QTextBlock &startBlock)
{
    // we also want to fold the last line of the region, therefore the ".next()"
    const auto endBlock = highlighter->findFoldingRegionEnd(startBlock).next();

    if (isFolded(startBlock)) {
        // unfold
        auto block = startBlock.next();
        while (block.isValid() && !block.isVisible()) {
            block.setVisible(true);
            block.setLineCount(block.layout()->lineCount());
            block = block.next();
        }

    } else {
        // fold
        auto block = startBlock.next();
        while (block.isValid() && block != endBlock) {
            block.setVisible(false);
            block.setLineCount(0);
            block = block.next();
        }
    }

    // redraw document
    document()->markContentsDirty(startBlock.position(), endBlock.position() - startBlock.position() + 1);

    // update scrollbars
    emit document()->documentLayout()->documentSizeChanged(document()->documentLayout()->documentSize());
}

bool CodeEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key()>=Qt::Key_0 && keyEvent->key()<=Qt::Key_9 && keyEvent->modifiers()==Qt::AltModifier)
        {
            TabControlable *controlable =  dynamic_cast<TabControlable *>(parent()->parent()->parent()->parent());
            int n = keyEvent->key()>=Qt::Key_1?keyEvent->key()-Qt::Key_1:9;
            controlable->setTab(n);
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
    }
    return false;
}

void CodeEditor::findNext(QString textToFind, QTextDocument::FindFlags flags)
{
    QTextCursor cursor(document());
    cursor = document()->find(textToFind, textCursor(), flags);
    if (cursor.isNull())
        QMessageBox::warning(nullptr, "Warning","Can't find ["+textToFind+"]",
                                   QMessageBox::Ok);
    else
        setTextCursor(cursor);
}

Properties CodeEditor::getProperties()
{
    Properties properties;
    if (loadUtfKind==UtfKind::Bom)
        properties.isUtfBOM = true;
    else if (loadUtfKind==UtfKind::detected)
        properties.isUtfDetected = true;
    else if (loadUtfKind==UtfKind::locale)
        properties.isLocale = true;
    return properties;
}

void CodeEditor::onTextChanged()
{
    QDateTime current = QDateTime::currentDateTime();
    lastEditTime = current.toMSecsSinceEpoch() ;
}

