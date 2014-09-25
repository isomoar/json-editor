#include "syntaxanalyser.h"
#include <QString>
#include <QChar>
#include <QDebug>
#include <QStringList>
#include <QRegExp>

SyntaxAnalyser::SyntaxAnalyser()
{

}

int SyntaxAnalyser::beginError()
{
    if(descPositions.isEmpty())
        return 0;
    else if(positionOfDescriptor==0)
        return 0;
    else
    {
        if(positionOfDescriptor>=descPositions.size())
            return descPositions.last();

        else
            return descPositions.at(positionOfDescriptor);

    }
}

int SyntaxAnalyser::endError()
{
    if(positionOfDescriptor==0) {
        return descPositions.at(0) + list.first().length();
    }
    else if(positionOfDescriptor>=descPositions.size())
        return descPositions.last() + list.last().length();
    else
    {   if(list.size()<=positionOfDescriptor)
            return descPositions.last() + list.last().length();
        else
            return descPositions.at(positionOfDescriptor) + list.at(positionOfDescriptor).length();
    }

}

void SyntaxAnalyser::getDescriptors(QString text)
{
    QString res;
    bool inQuotes = 0;

    for(int i=0; i<text.length(); i++) {
        if(text.at(i)== '"'){
            if(inQuotes)
                inQuotes = 0;
            else
                inQuotes = 1;

            descPositions.append(i);
            res.append(" \" ");

        }else if(text.at(i) == '{' || text.at(i) == '}' || text.at(i) == '[' || text.at(i) == ']'  ||text.at(i) == ','  || text.at(i) == ':')
        {
            if(inQuotes)
            {
                if(res.at(res.length()-1)==' ')
                    descPositions.append(i);

                res.append(text.at(i));
            }
            else
            {
                res.append(QString(" %1 ").arg(text.at(i)));
                descPositions.append(i);
            }
        }else if(text.at(i) == '\n' ||text.at(i) == '\t')
            continue;
        else if( text.at(i) == ' ')
        {
            if(inQuotes)
                res.append('_');
            else if(i>0 && !res.isEmpty())
                res.append(" ");
            else
                continue;
        }
        else
        {
            if(res.isEmpty()) {
                descPositions.append(i);
                res.append(text.at(i));
                continue;
            }
            if(res.at(res.length()-1)==' ')
                descPositions.append(i);
            res.append(text.at(i));
        }

    }

     list = res.split(" ", QString::SkipEmptyParts);

     QRegExp exp("[-0-9.]+");

    for(int i=0; i<list.size(); i++)
    {
        if(list.at(i) == "{")
            descriptors.append(1);
        else if(list.at(i) == "}")
            descriptors.append(2);
        else if(list.at(i) == ",")
            descriptors.append(3);
        else if(list.at(i) == ":")
            descriptors.append(4);
        else if(list.at(i) == "\"")
            descriptors.append(5);
        else if(list.at(i) == "[")
            descriptors.append(8);
        else if(list.at(i) == "]")
            descriptors.append(9);
        else if(list.at(i) == "true")
            descriptors.append(10);
        else if(list.at(i) == "false")
            descriptors.append(11);
        else if(exp.exactMatch(list.at(i)))
        {
            if(i>0 && list.at(i-1) == "\"")
                descriptors.append(6);
            else
                descriptors.append(7);
        }
        else
            descriptors.append(6);
    }
}

bool SyntaxAnalyser::case1000()
{
    switch(descriptors.at(0))
    {
        case 5:
        case 7:
        case 8:
        case 1:
            magazin.pop_back();
            magazin << 1006;
            return 1;
            break;
        default:
            return 0;
    }
}

bool SyntaxAnalyser::case1001()
{
    switch(descriptors.at(0))
    {
    case 1:
        descriptors.removeFirst();
        positionOfDescriptor++;
        magazin.pop_back();
        magazin << 2 << 1002;
        return 1;
        break;
    default:
        return 0;
    }

}

bool SyntaxAnalyser::case1002()
{
    switch(descriptors.at(0))
    {
    case 5:
        magazin.pop_back();
        magazin << 1011  << 1003;
        return 1;
        break;
    case 2:
        magazin.pop_back();
        return 1;
        break;
    default:
         return 0;
    }
}


bool SyntaxAnalyser::case1003()
{
    switch(descriptors.at(0))
    {
    case 5:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 1006 << 4 << 5 << 1005;
        positionOfDescriptor++;
        return 1;
        break;
    default:
         return 0;
    }
}

bool SyntaxAnalyser::case1005()
{
    switch(descriptors.at(0))
    {
    case 6:
        descriptors.removeFirst();
        magazin.pop_back();
        positionOfDescriptor++;
        return 1;
        break;
    case 5:
            magazin.pop_back();
        return 1;
        break;

    default:
         return 0;
    }
}

bool SyntaxAnalyser::case1006()
{
    switch(descriptors.at(0))
    {
    case 5:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 5 << 1005;
        positionOfDescriptor++;
        return 1;
        break;
    case 7:
        descriptors.removeFirst();
        magazin.pop_back();
        positionOfDescriptor++;
        return 1;
        break;
    case 8:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 9 << 1007;
        positionOfDescriptor++;
        return 1;
        break;
    case 1:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 2 << 1002;
        positionOfDescriptor++;
        return 1;
        break;
    case 10:
        descriptors.removeFirst();
        magazin.pop_back();
        positionOfDescriptor++;
        return 1;
        break;
    case 11:
        descriptors.removeFirst();
        magazin.pop_back();
        positionOfDescriptor++;
        return 1;
        break;
    default:
         return 0;
    }
}

bool SyntaxAnalyser::case1007()
{
    switch(descriptors.at(0))
    {
    case 9:
        magazin.pop_back();
        return 1;
        break;
    case 5:
    case 7:
    case 8:
    case 1:
        magazin.pop_back();
        magazin << 1012 << 1008;
        return 1;
        break;
    default:
         return 0;
    }
}

bool SyntaxAnalyser::case1008()
{
    switch(descriptors.at(0))
    {
    case 5:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 5 << 1005;
        positionOfDescriptor++;
        return 1;
        break;
    case 7:
        descriptors.removeFirst();
        magazin.pop_back();
        positionOfDescriptor++;
        return 1;
        break;
    case 8:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 9 << 1007;
        positionOfDescriptor++;
        return 1;
        break;
    case 1:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 2 << 1002;
        positionOfDescriptor++;
        return 1;
        break;
    default:
         return 0;
    }
}

bool SyntaxAnalyser::case1011()
{
    switch(descriptors.at(0))
    {
    case 3:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 1011 << 1003;
        positionOfDescriptor++;
        return 1;
        break;
    case 2:
        magazin.pop_back();
        return 1;
        break;
    default:
         return 0;
    }
}


bool SyntaxAnalyser::case1012()
{
    switch(descriptors.at(0))
    {
    case 3:
        descriptors.removeFirst();
        magazin.pop_back();
        magazin << 1012 << 1008;
        positionOfDescriptor++;
        return 1;
        break;
    case 9:
        magazin.pop_back();
        return 1;
        break;
    default:
         return 0;
    }
}


bool SyntaxAnalyser::isValid(QString text)
{
    magazin.push_back(-1);
    magazin.push_back(1000);
    positionOfDescriptor = 0;
    errorList = "";

    if(text.simplified().isEmpty()) {
        return 1;
    }
    else
        getDescriptors(text);

     while(magazin.last()!=-1)
    {
        switch(magazin.last())
        {
        case 1000:
            if(!case1000())
            {
               //errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1001:
            if(!case1001())
            {
                //errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1002:
            if(!case1002())
            {
                //errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
         case 1003:
            if(!case1003())
            {
                // errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1005:
            if(!case1005())
            {
               // errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1006:
            if(!case1006())
            {
               //  errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1007:
            if(!case1007())
            {
               // errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1012:
            if(!case1012())
            {
               //  errorList= QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1008:
            if(!case1008()){
             //   errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        case 1011:
            if(!case1011())
            {
              // errorList = QString("Error at position number %1").arg( positionOfDescriptor);
                return 0;
            }
            break;
        default:
            if(descriptors.isEmpty() && magazin.last()!=-1)
                return 0;

            if(magazin.last() == descriptors.at(0))
            {
                magazin.pop_back();
                descriptors.removeFirst();
                positionOfDescriptor++;
            }else
            {
                //errorList = QString("Expected %1 not %2 in position number %3").arg(designations[magazin.last()]).arg(designations[descriptors.at(0)]).arg(positionOfDescriptor);
                return 0;
            }
            break;
        }
    }

     if(!descriptors.isEmpty())
     {
         errorList = QString("JSON object must be the only object");
        return 0;
     }
    // if(inComment)
     //   return 0;

     return 1;

}
