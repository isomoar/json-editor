#include "structurer.h"
#include <QRegExp>
#include <QDebug>
#include <QStringList>

Structurer::Structurer()
{
    mainObject = new Key();
    mainObject->type = "null";
}

bool Structurer::isEmpty()
{
    if(mainObject->type == "null")
        return 1;
    return 0;
}

void Structurer::print(Key* k)
{
    if(k->children.isEmpty())
        return;
    qDebug() << k->name << " 's children: ";

    for(int i=0; i<k->children.size(); ++i)
    {
        qDebug() << k->children.at(i)->name;
    }
    for(int i=0; i<k->children.size(); ++i)
        print(k->children.at(i));

}

int Structurer::getPositionForPath(QString path)
{
    path.remove(0,1);
    QStringList list = path.split("/");

    Key* curKey = mainObject;

    int index = 0;
    bool found = false;

    while(!found)
    {
        for(int i=0; i<curKey->children.size(); ++i)
        {
            if(curKey->children.at(i)->name==list.at(index))
            {
                curKey = curKey->children.at(i);

                if(index==list.size()-1) //if it last element in path
                {
                    found = true;
                    return curKey->pos;

                }else if(curKey->children.isEmpty())
                {
                    found = true;
                    return -1;
                 //   qDebug() << "Empty child" <<  curKey->name;
                }else
                    index++;
                break;
            }else if(i==curKey->children.size()-1)
                return -1;
        }


    }
    return -1;
}

void Structurer::createTree(QString text)
{
    QRegExp reg("([\"]((?:[^\"])*)[\"]\\s*:\\s*.)");

    QString leftText;
    int index = 0;
    int length = 0;

    mainObject->parent = 0;
    mainObject->name = "";
    mainObject->type = "object";

    Key* currentKey = mainObject;

    for(int i=0; i<text.size(); ++i)
    {
        leftText.append(text.at(i));
        int indexCur = reg.indexIn(leftText, index + length);


        if(indexCur>0)
        {
            QChar ch = reg.cap(1).at(reg.cap(1).length()-1);

            if(ch==' ' || ch=='\n')
                continue;
            else
            {
                Key* k = new Key();
                k->pos = i;
                k->name = reg.cap(2);
                k->parent = currentKey;
                currentKey->children.append(k);


                if(ch=='{')
                {
                    k->type = "object";
                    currentKey = k;

                }else if(ch=='[')
                {
                    k->type = "array";
                    currentKey = k;

                }else
                   k->type = "other";

                length = reg.cap(1).length()+1 + indexCur;
            }
        }else if(text.at(i)=='{')
        {
            if(currentKey->type=="array")
            {
                Key* k = new Key();
                k->pos = i;
                k->type = "object";
                k->parent = currentKey;
                currentKey->children.append(k);
                int size = currentKey->children.size()-1;
                k->name = QString("%1").arg(size);
                currentKey = k;
            }
        }

        if(text.at(i)==']' || text.at(i)=='}')
        {
            if(currentKey!=mainObject)
                 currentKey = currentKey->parent;
        }
    }

}
