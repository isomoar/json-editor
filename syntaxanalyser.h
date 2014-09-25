#ifndef SYNTAXANALYSER_H
#define SYNTAXANALYSER_H
#include <QList>
#include <QString>
#include <QMap>
#include <QStringList>

class SyntaxAnalyser
{
private:
    QMap<int, QString> designations;
public:
    SyntaxAnalyser();
    QList<int> magazin;

    bool isValid(QString text);

    QList<int> descriptors;
    QList<int> descPositions;
    void getDescriptors(QString);
    int beginError();
    int endError();
    
    int positionOfDescriptor;
    QStringList list;
    int inComment;

    bool case1000();
    bool case1001();
    bool case1002();
    bool case1011();
    bool case1003();
    bool case1005();
    bool case1006();
    bool case1007();
    bool case1012();
    bool case1008();
    QString errorList;

};

#endif // SYNTAXANALYSER_H
