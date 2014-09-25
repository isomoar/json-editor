#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include "textedit.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEdit *editor);

    virtual QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    TextEdit*codeEditor;

};

#endif // LINENUMBERAREA_H
