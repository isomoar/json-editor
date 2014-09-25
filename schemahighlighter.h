#ifndef SCHEMAHIGHLIGHTER_H
#define SCHEMAHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SchemaHighlighter: public QSyntaxHighlighter
{
public:
   SchemaHighlighter(QTextDocument *parent = 0);
private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> rules;
protected:
    void highlightBlock(const QString &text);
};

#endif // SCHEMAHIGHLIGHTER_H
