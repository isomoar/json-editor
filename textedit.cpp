#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QTableView>
#include <QHeaderView>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QTextBlock>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QBrush>
#include <QStringListModel>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextBlockFormat>
#include <QClipboard>
#include <QMenu>
#include <QPalette>

#include "textedit.h"
#include "linenumberarea.h"
#include "syntaxhighlightening/syntaxanalyser.h"
#include "structurer.h"


TextEdit::TextEdit(QWidget *parent)
: QPlainTextEdit(parent), c(0)
{
    QFont font;
    font.setFamily("MENLO");
    font.setPointSize(11);
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    tabStop = 4; //width of tab in spaces
    QFontMetrics metrics(font);
    setTabStopWidth(16);

    currentObjectName = "undefined"; // key name which value is object
    needToCheckSyntax = 0; // if json file highlight JSON syntax errors
    cursorUnderType = 0; // if cursor after "type" key (only in schema)

    braceChars << '{' << '}' << '[' << ']';
    lineNumberArea = new LineNumberArea(this);
    updateLineNumberAreaWidth(0);

    QColor textColor(230,230,230);

    QPalette pal;
    pal.setColor(QPalette::Text, textColor);
    pal.setBrush(QPalette::HighlightedText, textColor);
    pal.setBrush(QPalette::Highlight, QColor(85, 85, 85));
    setPalette(pal);

    clearColor.setRgb(45,45,45);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
}

TextEdit::~TextEdit()
{
}

void TextEdit::processKeys(QKeyEvent *e)
{
    QTextCursor tc = textCursor();
    QString text = toPlainText();

    switch (e->key()) {
    case Qt::Key_Return: //Enter pressed
    {
        //process autoindents when Enter pressed
        //check left indent

        QString textInCurrentBlock = tc.block().text();
        QRegExp reg("\\s*");
        int symbolsBeginPosition = reg.indexIn(textInCurrentBlock);
        leftIndent = reg.cap(0);

        //if current block is empty clear all spaces and tabs
        if(!textInCurrentBlock.isEmpty()) {
            if(reg.exactMatch(textInCurrentBlock) && !text.trimmed().isEmpty() && text.trimmed().at(0)=='{') {
                tc.select(QTextCursor::BlockUnderCursor);
                tc.removeSelectedText();
                tc.insertText("\n");
            }
        }

        //if cursor between {<this>}
        int pos = tc.position();
        QString text = toPlainText();

        if(text.size()>tc.position()) {
            if( (text.at(pos)==Qt::Key_BraceRight && text.at(pos-1)==Qt::Key_BraceLeft)
                    || (text.at(pos)==Qt::Key_BracketRight && text.at(pos-1)==Qt::Key_BracketLeft)) {
                QString str = QString("\n%1\t\n%2").arg(leftIndent).arg(leftIndent);

                tc.deleteChar();
                tc.insertText("\n\n"+leftIndent+text.at(pos));
                tc.movePosition(QTextCursor::Up);
                leftIndent.append("\t");
                tc.insertText(leftIndent);
                setTextCursor(tc);
            }else {
                QPlainTextEdit::keyPressEvent(e);
                tc.insertText(leftIndent);
                tc.movePosition(QTextCursor::EndOfBlock);
            }
        }else {
            QPlainTextEdit::keyPressEvent(e);
            tc.insertText(leftIndent);
            tc.movePosition(QTextCursor::EndOfBlock);
        }
    }
        break;

    case Qt::Key_BraceLeft: // {
    case Qt::Key_BracketLeft: // [
    {
        QTextCursor tc = textCursor();
        QString text = toPlainText();
        QChar chNext = ' ';
        QChar rightBrace;

        if(e->text()=="{")
            rightBrace = '}';
        else
            rightBrace = ']';

        if(text.size()>tc.position())
            chNext =  text.at(tc.position());

        if(chNext==' ' ||chNext=='\t'|| chNext=='\n'||chNext==rightBrace || chNext==',') {
            tc.insertText(e->text() + rightBrace);
            tc.movePosition(QTextCursor::Left);
            setTextCursor(tc);
        }else {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
        break;

    case Qt::Key_BraceRight: // }
    case Qt::Key_BracketRight: // ]
    {
        QTextCursor tc = textCursor();
        QString text = toPlainText();
        QChar chNext = ' ';

        if(text.size()>tc.position())
            chNext =  text.at(tc.position());

        if(chNext==e->text()) {
            tc.movePosition(QTextCursor::Right);
            setTextCursor(tc);
        }
        else {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
        break;

    case Qt::Key_QuoteDbl: // "
    {
        if(tc.atStart() || tc.atEnd()) {
            insertQuotes();
            break;
        }
        if(text.at(tc.position())=='"'){
            tc.movePosition(QTextCursor::Right);
            setTextCursor(tc);
            break;
        }
        if(!tc.atEnd()) {
            QChar chLeft = text.at(tc.position()-1);
            QChar chRight = text.at(tc.position());
            QRegExp reg("[\\s{}[]]");
            if(reg.exactMatch(chLeft) && reg.exactMatch(chRight))
                insertQuotes();
            else
                QPlainTextEdit::keyPressEvent(e);
        }
    }
        break;

    case Qt::Key_Backspace: //<=
    {
        if(tc.atStart() && !tc.hasSelection())
            break;

        if(tc.hasSelection()) {
            QPlainTextEdit::keyPressEvent(e);
            break;
        }
        if(text.size()<=tc.position()) {
            QPlainTextEdit::keyPressEvent(e);
            return;
        }

        if((text.at(tc.position())=='"' && text.at(tc.position()-1)=='"')
                || (text.at(tc.position())=='}' && text.at(tc.position()-1)=='{')
                || (text.at(tc.position())==']' && text.at(tc.position()-1)=='[')) {
            tc.deleteChar();
            QPlainTextEdit::keyPressEvent(e);
            return;
        }
        QPlainTextEdit::keyPressEvent(e);
    }
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
        break;
    }

    checkSyntax();
    checkCurrentObject();
    highLightBraces();
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    QTextCursor tc = textCursor();
    Qt::KeyboardModifiers keyModifiers = QApplication::keyboardModifiers ();

    bool isShift = keyModifiers.testFlag(Qt::ShiftModifier);
    bool isCtrl = keyModifiers.testFlag(Qt::ControlModifier);

    if (isShift && isCtrl)
        return;

    if((e->modifiers() & Qt::ControlModifier) && e->key()==Qt::Key_Z)
    {
        qDebug() << "Ctrl+z";
        return;
    }

    if((e->modifiers() & Qt::ControlModifier) && e->key()==Qt::Key_V)
    {
        paste();
        return;
    }

    if((e->modifiers() & Qt::ControlModifier) && e->key()==Qt::Key_C)
    {
        return;
    }

    if((e->modifiers() & Qt::ControlModifier) && e->key()==Qt::Key_F)
        return;

    if((e->modifiers() & Qt::ControlModifier) && e->key()==Qt::Key_W)
        return;

    if(e->key()==Qt::Key_Control)
        return;

    if (c && c->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        default:
            break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E

    if(isShortcut)
        emit completerKeysPressed();

    if (!c || !isShortcut)
       processKeys(e);

    checkErrorEditing();
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;

    QString completionPrefix = textUnderCursor();
    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1)) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }

    //draw completer
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->sizeHintForColumn(1)+
                c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

void TextEdit::clearCurrentObjectName()
{
    currentObjectName = "undefined";
}

void TextEdit::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu *menu = new QMenu(this);
    QAction* copyAct = new QAction(tr("Копировать"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    QAction* pasteAct = new QAction(tr("Вставить"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct , SIGNAL(triggered()), this, SLOT(paste()));

    QAction* cutAct = new QAction(tr("Вырезать"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    QAction* selectAllAct = new QAction(tr("Выделить все"), this);
    selectAllAct->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

     menu->addAction(copyAct);
     menu->addAction(pasteAct);
     menu->addAction(cutAct);
     menu->addAction(selectAllAct);
     menu->exec(event->globalPos());
}

void TextEdit::setNeedToCheckSyntax(bool val)
{
    needToCheckSyntax = val;
}

void TextEdit::insertQuotes()
{
    QTextCursor tc = textCursor();
    tc.insertText("\"\"");
    tc.movePosition(QTextCursor::Left);
    setTextCursor(tc);
}

void TextEdit::checkSyntax(bool b)
{
    needToCheckSyntax = b;
}

void TextEdit::mousePressEvent(QMouseEvent *e)
{
     QPlainTextEdit::mousePressEvent(e);
     checkSyntax();
     checkCurrentObject();
     highLightBraces();
}

void TextEdit::checkKeyUnderCursor()
{
    QTextCursor tc = textCursor();
    QString text = tc.block().text().trimmed();
    QRegExp reg("\"type\"\\s*:\\s*[\"a-zA-Z]*");

    cursorUnderType  = 0;
    cursorUnderKey = 0;

    if(reg.exactMatch(text))  {
        cursorUnderType = 1;
        return;
    }

    text = tc.block().text().trimmed();

    if(text.isEmpty())  {
        cursorUnderKey = 1;
    }else {
        int quot = 0;

        for(int i=0; i<text.length(); ++i) {
            if(text.at(i)=='"')
                quot++;
        }

        if(quot % 2 !=0 || quot==0) {
            cursorUnderKey = 1;
        } else {
            if(tc.positionInBlock() < tc.block().text().length())
                cursorUnderKey = 1;
            else
                cursorUnderKey = 0;
        }
    }
}

void TextEdit::checkCurrentObject()
{
    QTextCursor tc = textCursor();
    QString text = toPlainText();

    if(text.trimmed().isEmpty()) {
        currentObjectName = "undefined";
        emit currentObjectUndefined();
        return;
    }

    QString textCursorLeft = text.left(tc.position());
    QString textCursorRight = text.right(text.length()-tc.position());
    QString result;

    checkKeyUnderCursor();

    int braces = 0;

    //start search current object type...

    for(int i=textCursorLeft.length()-1; i>=0; --i) {
        if(textCursorLeft.at(i)=='}') {
            braces++;
            result.prepend(textCursorLeft.at(i));
        }
        else if(textCursorLeft.at(i)=='{') {
            if(braces==0) {
                textCursorLeft = textCursorLeft.left(i);
                break;
            } else {
                braces--;
                result.prepend(textCursorLeft.at(i));
            }
        }else {
            result.prepend(textCursorLeft.at(i));
        }
    }

    for(int i=0; i<textCursorRight.length(); ++i) {
        if(textCursorRight.at(i)=='{') {
            result.append(textCursorRight.at(i));
            braces++;
        } else if(textCursorRight.at(i)=='}') {
            if(braces==0) {
                break;
            } else {
                result.append(textCursorRight.at(i));
                braces--;
            }
        }else
            result.append(textCursorRight.at(i));
    }

    QRegExp ex("\"([^\"]*)\"\\s*:\\s*");
    int index = ex.indexIn(textCursorLeft);
    QStringList list;

    while(index>0) {
        list<<ex.cap(1);
        int length = ex.cap(1).length()+1;
        index = ex.indexIn(textCursorLeft, index + length);
    }

    QString current = "";
    if(!list.isEmpty())
        current = list.last();

    if(current!=currentObjectName) {
        currentObjectName = current;
        emit currentObjectChanged(current);
    }

    QRegExp exp("\"type\"\\s*:\\s*\"([^\"]*)(?![^\\{]*\\})");
    index = exp.indexIn(result);

    QString typeName = exp.cap(1);

    if(typeName.isEmpty())
        emit currentObjectTypeChanged("");

    if(typeName!=currentObjectType) {
        currentObjectType=typeName;
        emit currentObjectTypeChanged(typeName);
    }
    emit endOfCheckingCursorPositions();
}

//
//-----------------------FORMAT-------------------------------
//
// validation errors

void TextEdit::createStructureForError(QList<QString> pathList)
{
    Structurer structTree;
    if(structTree.isEmpty())
        structTree.createTree(toPlainText());

    QStringList list;
    int  pos = 0;
    for(int i=0; i<pathList.size(); i++)
    {
        pos = structTree.getPositionForPath(pathList.at(i));
        highlightAtPosition(pos);
    }
}

void TextEdit::highlightAtPosition(int pos)
{
    QList<QTextEdit::ExtraSelection> es = extraSelections();
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(204,0,0);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        QTextCursor tc = textCursor();
        tc.setPosition(pos);

        selection.cursor = tc;
        selection.cursor.clearSelection();
        es.append(selection);
    }
    setExtraSelections(es);
}

void TextEdit::cleanExtraSelections(bool single)
{
    QTextEdit::ExtraSelection selection;
    QList<QTextEdit::ExtraSelection> newExtraList;
    QList<QTextEdit::ExtraSelection> currentExtraList = extraSelections();
    QColor lineColor = QColor(clearColor);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    if(single) {
        selection.cursor = textCursor();
        currentExtraList.append(selection);
        selection.cursor.clearSelection();
        setExtraSelections(currentExtraList);
    }else  {
        setExtraSelections(newExtraList);
    }
}

void TextEdit::checkErrorEditing()
{
    QTextCursor tc = textCursor();
    QList<QTextEdit::ExtraSelection> es = extraSelections();

    for(int i=0; i<es.size(); ++i) {
        if(es.at(i).cursor.blockNumber()==tc.blockNumber()) {
            cleanExtraSelections(true);
            break;
        }
    }
}

void TextEdit::highLightBraces()
{
    for(int i=0 ; i< braceChars.size(); i+=2) {
        QTextCursor tc = textCursor();
        if(toPlainText().isEmpty())
            return;
        QChar left = braceChars.at(i);
        QChar right = braceChars.at(i+1);
        QString text = toPlainText();
        QTextCharFormat fmt;
        fmt.setBackground(QColor(245,184,0));
        fmt.setForeground(QColor(Qt::black));

        int braces = 0;

        bool wasModif = document()->isModified();
        if(!tc.atEnd()) {
            if(text.at(tc.position())==left) {
                if(tc.position()+1>=text.size())
                    return;
                for(int i=tc.position()+1; i<text.length(); ++i) {
                    if(text.at(i)==right) {
                        if(braces==0) {
                            tc.setPosition(tc.position(), QTextCursor::MoveAnchor);
                            tc.setPosition(tc.position()+1, QTextCursor::KeepAnchor);
                            tc.setCharFormat(fmt);
                            if(!wasModif)
                                document()->setModified(false);
                            tc.setPosition(i, QTextCursor::MoveAnchor);
                            tc.setPosition(i+1, QTextCursor::KeepAnchor);
                            tc.setCharFormat(fmt);
                            if(!wasModif)
                                document()->setModified(false);
                            break;
                        }else braces--;
                    }else if(text.at(i)==left)
                        braces++;
                }
            }else if(!tc.atStart()) {
                if(text.at(tc.position()-1)==right) {
                    if(tc.position()-2<0)
                        return;
                    for(int i=tc.position()-2; i>=0; --i) {
                        if(text.at(i)==left) {
                            if(braces==0) {
                                tc.setPosition(tc.position(), QTextCursor::MoveAnchor);
                                tc.setPosition(tc.position()-1, QTextCursor::KeepAnchor);
                                tc.setCharFormat(fmt);
                                if(!wasModif)
                                    document()->setModified(false);
                                tc.setPosition(i, QTextCursor::MoveAnchor);
                                tc.setPosition(i+1, QTextCursor::KeepAnchor);
                                tc.setCharFormat(fmt);
                                if(!wasModif)
                                    document()->setModified(false);

                                break;
                            }else
                                braces--;
                        }else if(text.at(i)==right)
                            braces++;
                    }
                }
            }
        }else {
            if(text.at(tc.position()-1)==right) {
                if(tc.position()-2<0)
                    return;
                for(int i=tc.position()-2; i>=0; --i) {
                    if(text.at(i)==left) {
                        if(braces==0) {
                            tc.setPosition(tc.position(), QTextCursor::MoveAnchor);
                            tc.setPosition(tc.position()-1, QTextCursor::KeepAnchor);
                            tc.setCharFormat(fmt);
                            if(!wasModif)
                                document()->setModified(false);
                            tc.setPosition(i, QTextCursor::MoveAnchor);
                            tc.setPosition(i+1, QTextCursor::KeepAnchor);
                            tc.setCharFormat(fmt);
                            if(!wasModif)
                                document()->setModified(false);
                            break;
                        }else
                            braces--;
                    }else if(text.at(i)==right)
                        braces++;
                }
            }
        }
    }
}

void TextEdit::cleanTextEdit()
{
    QTextCharFormat fmtClean;
    fmtClean.setBackground(clearColor);
    QTextCursor cursor = textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.setPosition(toPlainText().length(), QTextCursor::KeepAnchor);
    bool wasModif = document()->isModified();
    cursor.setCharFormat(fmtClean);
    if(!wasModif)
        document()->setModified(false);
}

void TextEdit::checkSyntax()
{
    if(!needToCheckSyntax)
        return;
    cleanTextEdit();
    QString text = this->toPlainText();
    SyntaxAnalyser s;
    QTextCharFormat fmtError;
    fmtError.setBackground(QColor(255, 5 ,68)); //set format of the errors

    if(!s.isValid(text)) {
        QTextCursor cursor = textCursor();
        cursor.setPosition(s.beginError(), QTextCursor::MoveAnchor);
        cursor.setPosition(s.endError(), QTextCursor::KeepAnchor);
        bool wasModif = document()->isModified();
        cursor.setCharFormat(fmtError);
        if(!wasModif)
            document()->setModified(false);
    }
}

//-------------------COMPLETER WORK-------------------------------

void TextEdit::setCompleter(QCompleter *completer)
{
    //check if completer already exists
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);

    //start create view for completer and style it
    QTableView* tv = new QTableView();
    c->setMaxVisibleItems(7);
    tv->horizontalHeader()->hide();
    tv->verticalHeader()->hide();
    tv->setShowGrid(false);

    tv->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);

    //set rows width
    QHeaderView *verticalHeader = tv->verticalHeader();
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(20);

    tv->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents);

    //set view to completer
    c->setPopup(tv);
    QItemSelectionModel* sm = new QItemSelectionModel(c->completionModel());
    c->popup()->setSelectionModel(sm);
    sm->select(c->completionModel()->index(1,0), QItemSelectionModel::Select);

    QObject::connect(c, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

QCompleter *TextEdit::completer() const
{
    return c;
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void TextEdit::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;

    QTextCursor tc = textCursor();
    QString text = toPlainText();

    if(tc.atStart()) {
        tc.insertText(QString("\"%1\"").arg(completion));
        return;
    }

    QList<QChar> separators;
    separators << ' ' << '\t' << '\n';

    if(separators.contains(text.at(tc.position()-1))) {
        tc.insertText(QString("\"%1\"").arg(completion));
        return;
    }

    int wordStartPosition = 0;
    int wordEndPosition = 0;
    int currentPosition = tc.position();

    //find left and right positions to get word
    while(!tc.atStart() && !separators.contains( text.at(tc.position()-1))) {
        tc.movePosition(QTextCursor::Left);
    }
    wordStartPosition = tc.position();
    tc.setPosition(currentPosition);
    while(!tc.atEnd() && !separators.contains( text.at(tc.position()))) {
        tc.movePosition(QTextCursor::Right);
    }
    wordEndPosition = tc.position();

    //remove typed word
    tc.setPosition(wordStartPosition, QTextCursor::MoveAnchor);
    tc.setPosition(wordEndPosition, QTextCursor::KeepAnchor);
    tc.removeSelectedText();

    //replace typed word to completion with quotes
    QString word = text.mid(wordStartPosition, wordEndPosition-wordStartPosition).trimmed();
    tc.insertText(QString("\"%1\"").arg(completion));
}

QString TextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    QString text = toPlainText();

    if(tc.atStart())
        return "";

    QList<QChar> separators;
    separators << ' ' << '\t' << '\n' << '"';

    if(separators.contains(text.at(tc.position()-1))) {
        return "";
    }

    int wordStartPosition = 0;
    int wordEndPosition = 0;
    int currentPosition = tc.position();

     //find left and right positions to get word
    while(!tc.atStart() && !separators.contains( text.at(tc.position()-1))) {
        tc.movePosition(QTextCursor::Left);
    }
    wordStartPosition = tc.position();
    tc.setPosition(currentPosition);

    while(!tc.atEnd() && !separators.contains( text.at(tc.position()))) {
        tc.movePosition(QTextCursor::Right);
    }
    wordEndPosition = tc.position();

    QString word = text.mid(wordStartPosition, wordEndPosition-wordStartPosition).trimmed();
    QRegExp reg("\"*([^\"]*)\"*");
    int index = reg.indexIn(word);
    return reg.cap(1);
}

//--------------------------LINE NUMBER AREA ------------------------------
//from Qt examples

int TextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void TextEdit::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), clearColor); //background
    QTextCursor tc = textCursor();

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            if(tc.blockNumber() == number.toInt()-1) {
                 painter.setPen(QColor(180,180,180)); //number current
                 painter.drawText(0, top, lineNumberArea->width()-3, fontMetrics().height(),
                              Qt::AlignRight, number);
                // painter.drawLine(0, top,10,top);
            }
            else {
                painter.setPen(QColor(120, 120, 120)); //number
             //   painter.drawLine(lineNumberArea->width(), top,lineNumberArea->width(),top+fontMetrics().height());
                painter.drawText(0, top, lineNumberArea->width()-3, fontMetrics().height(),
                             Qt::AlignRight, number);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
