#include <QDebug>
#include <QtWidgets>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextCharFormat>

#include "searchpanel.h"
#include "textedit.h"
#include "mainwindow.h"

SearchPanel::SearchPanel(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    currentPositionIndex = -1;
    connect(searchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(searchMatches(QString)));
}

void SearchPanel::updateSearch()
{
    if(currentPositionIndex==-1)
        return;
    searchMatches(searchLineEdit->text());
}

void SearchPanel::searchMatches(QString searchString)
{
    //get current textedit
    MainWindow* mw = qobject_cast<MainWindow*>(parent());
    textEdit = mw->currentTextEdit();

    //clear previous matches
    positions.clear();
    str = searchString;

    textEdit->cleanTextEdit();
    if(searchString.trimmed().isEmpty())
        return;

    QTextCursor highlightCursor(textEdit->document()); //hihglights matches
    QTextCursor cursor(textEdit->document());

    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat; //highlght current match
    colorFormat.setBackground(QColor(51,102,153));

    //start find matches in blocks loop...
    cursor.beginEditBlock();
            while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
//                if(regCheckBox->isChecked())
//                    highlightCursor = fp->textEdit->document()->find(searchString, highlightCursor, QTextDocument::FindCaseSensitively);
//                else
                    highlightCursor = textEdit->document()->find(searchString, highlightCursor);

                if (!highlightCursor.isNull()) {
                    positions.push_back(highlightCursor.position());
                    highlightCursor.mergeCharFormat(colorFormat);
                }
            }
    cursor.endEditBlock();
}

void SearchPanel::drawCurrent()
{
    searchMatches(str);
    if(positions.isEmpty())
        return;

    QTextCursor tc = textEdit->textCursor();
    tc.setPosition(positions.at(currentPositionIndex)-str.length(), QTextCursor::MoveAnchor);
    tc.setPosition(positions.at(currentPositionIndex), QTextCursor::KeepAnchor);

    QTextCharFormat fmt;
    fmt.setBackground(QColor(51,102,153));
    fmt.setTextOutline(QPen(QColor(204,102,0)));
    tc.setCharFormat(fmt);

    tc.setPosition(positions.at(currentPositionIndex));
    textEdit->setTextCursor(tc);
}

void SearchPanel::on_nextButton_clicked()
{
    if(positions.size()-1==currentPositionIndex)
        currentPositionIndex = 0;
    else
       currentPositionIndex++;
    drawCurrent();
}

void SearchPanel::on_previousButton_clicked()
{
    if(currentPositionIndex==-1 || currentPositionIndex==0)
        currentPositionIndex = positions.size()-1;
    else
        currentPositionIndex--;
    drawCurrent();
}

void SearchPanel::on_replaceButton_clicked()
{
    if(currentPositionIndex==-1)
        return;
    if(replaceLineEdit->text().isEmpty() || str.isEmpty())
        return;
    QTextCursor tc = textEdit->textCursor();
    tc.setPosition(positions.at(currentPositionIndex)-str.length(), QTextCursor::MoveAnchor);
    tc.setPosition(positions.at(currentPositionIndex), QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText(replaceLineEdit->text());
}

void SearchPanel::on_replaceAllButton_clicked()
{
    if(currentPositionIndex==-1)
        return;
    if(replaceLineEdit->text().isEmpty() || str.isEmpty())
        return;
    currentPositionIndex = 0;

    while(currentPositionIndex<positions.size()) {
        on_replaceButton_clicked();
        currentPositionIndex++;
    }
}

void SearchPanel::hideEvent(QHideEvent * event)
{
//    QTextCharFormat fmtClean;
//    fmtClean.setBackground(Qt::white);

//    QTextCursor cursor = fp->textEdit->textCursor();
//    cursor.setPosition(0, QTextCursor::MoveAnchor);
//    cursor.setPosition(fp->textEdit->toPlainText().length(), QTextCursor::KeepAnchor);
//    cursor.setCharFormat(fmtClean);
}

void SearchPanel::showEvent(QShowEvent * event)
{
    positions.clear();
    currentPositionIndex = -1;
    searchLineEdit->setFocus(Qt::OtherFocusReason);
}

