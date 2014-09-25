#include "highlighter.h"
#include <QRegExp>
#include <QDebug>
#include <QTextCharFormat>

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
   HighlightingRule rule;
    //numbers
   rule.pattern = QRegExp("([-0-9.]+)(?!([^\"]*\"[\\s]*\\:))");
   rule.format.setForeground(Qt::darkRed);
   rules.append(rule);


   //middle
   rule.pattern = QRegExp ("(?:[ ]*\\,[ ]*)(\"[^\"]*\")");
   rule.format.setForeground(Qt::darkGreen);
   rules.append(rule);

   //last
   rule.pattern = QRegExp ("(\"[^\"]*\")(?:\\s*\\])");
   rule.format.setForeground(Qt::darkGreen);
   rules.append(rule);


   //string:
   rule.pattern = QRegExp ("(\"[^\"]*\")\\s*\\:");
   rule.format.setForeground(Qt::darkBlue);
   rules.append(rule);

   rule.pattern = QRegExp (":+(?:[: []*)(\"[^\"]*\")");
   rule.format.setForeground(Qt::darkGreen);
   rules.append(rule);

//   commentFormat.setForeground(Qt::darkGray);
//   commentStartExpression = QRegExp("/\\*");
//   commentEndExpression = QRegExp("\\*/");
    
}

void Highlighter::highlightBlock(const QString &text)
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

//    setCurrentBlockState(0);

//    int startIndex = 0;
//    if (previousBlockState() != 1)
//        startIndex = commentStartExpression.indexIn(text);

//    while (startIndex >= 0) {

//        int endIndex = commentEndExpression.indexIn(text, startIndex);
//        int commentLength;
//        if (endIndex == -1) {
//            setCurrentBlockState(1);
//            commentLength = text.length() - startIndex;
//        } else {
//            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
//        }
//        setFormat(startIndex, commentLength, commentFormat);
//        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
//    }


}
