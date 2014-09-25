#include "schemahighlighter.h"
#include <QRegExp>
#include <QDebug>
#include <QTextCharFormat>

SchemaHighlighter::SchemaHighlighter(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    
    rule.pattern = QRegExp ("(\"[^\"]*\")");
    rule.format.setForeground(Qt::darkBlue);
    rule.format.setFontWeight(QFont::Bold);
    
    rules.append(rule);
    
    rule.pattern = QRegExp (":+(?:[: []*)(\"[^\"]*\")");
    rule.format.setForeground(Qt::darkCyan);
    rule.format.setFontWeight(QFont::Normal);
    rules.append(rule);
    
    rule.pattern = QRegExp ("\\,(?:[ ]*)\"([^\"]*)\"(?:\\s*)");
    rule.format.setForeground(Qt::darkCyan);
    rules.append(rule);
    
    rule.pattern = QRegExp("([0-9.]+)(?!.*\")");
    rule.format.setForeground(Qt::darkGreen);
    rules.append(rule);
    
}

void SchemaHighlighter::highlightBlock(const QString &text)
{
    //object
    foreach (const HighlightingRule &rule, rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        
        while (index >= 0) {
            index = expression.pos(1);
            int length = expression.cap(1).length();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    
    
    
    
}
