#ifndef STRUCTURER_H
#define STRUCTURER_H
#include <QString>
#include <QList>

class Structurer
{
    struct Key{
        int pos;
        QString name;
        QString type;
        Key* parent;
        QList<Key*> children;
    };
    Key* mainObject;

public:
    Structurer();
    void createTree(QString);
    void print(Key*);
    int getPositionForPath(QString);
    bool isEmpty();

};

#endif // STRUCTURER_H
