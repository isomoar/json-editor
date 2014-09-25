#ifndef TEXTEDIT_H
#define TEXTEDIT_H
#include <QPlainTextEdit>

class QCompleter;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    ~TextEdit();
    void setCompleter(QCompleter *c);
    void checkSyntax(bool);
    void checkSyntax();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void highlightAtPosition(int);
    int lineNumberAreaWidth();
    void cleanTextEdit();
    void cleanPreviousBraces();
    void setNeedToCheckSyntax(bool);

    //------------------------------error highlighting
    void createStructureForError( QList<QString>);
    void cleanExtraSelections(bool single = false);
    void checkErrorEditing();
    //------------------------------------------

    QCompleter *completer() const;
    QString currentObjectName;
    void clearCurrentObjectName();
    bool cursorUnderType;
    bool cursorUnderKey;

signals:
    void currentObjectChanged(QString);
    void currentObjectTypeChanged(QString);
    void completerKeysPressed();
    void schemaKeyChanged();
    void endOfCheckingCursorPositions();
    void currentObjectUndefined();

public slots:

private slots:
    void insertCompletion(const QString &completion);
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
    void checkCurrentObject();

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *event);
    void contextMenuEvent(QContextMenuEvent * );

private:
    QColor clearColor;
    QString textUnderCursor() const;
    void insertQuotes();
    void processKeys(QKeyEvent *e);
    void checkKeyUnderCursor();
    void highLightBraces();

    QCompleter *c;
    QList<QChar> braceChars;
    bool needToCheckSyntax;
    int tabStop;
    QString leftIndent;
    QString currentObjectType;
    QString schemaKeyValue;
    QWidget *lineNumberArea;
};


#endif // TEXTEDIT_H
