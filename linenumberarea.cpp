#include "linenumberarea.h"

LineNumberArea::LineNumberArea(TextEdit *editor) : QWidget(editor)
{
    codeEditor = editor;
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}
