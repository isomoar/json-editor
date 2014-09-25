#include <QDebug>
#include <QCompleter>
#include <QStandardItemModel>
#include <QRegExp>
#include <QStringListModel>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCheckBox>

#include "filecontroller.h"
#include "completerpopupmodel.h"
#include "textedit.h"
#include "mainwindow.h"

FileController::FileController(QObject *parent) :
    QObject(parent)
{
    model = new QStandardItemModel(this);
    completer = new QCompleter();
    textEdit = new TextEdit();
    isSch = false;

    textEdit->setCompleter(completer);
    //create schema models
    QStringList list;
    list  << "type" << "id" << "properties" << "default" << "title" << "required"
          << "minProperties" << "maxProperties" << "dependencies" << "patternProperties"
          << "additionalProperties";
    schemaModels.insert("object", new QStringListModel(list));
    list.clear();
    list << "minLength" << "maxLength" << "pattern";
    schemaModels.insert("string", new QStringListModel(list));
    list.clear();
    list << "exclusiveMinimum" << "exclusiveMaximum" << "multipleOf"<< "maximum"<< "minimum";
    schemaModels.insert("number", new QStringListModel(list));
    schemaModels.insert("integer", new QStringListModel(list));
    list.clear();
    list << "additionalItems" << "items" << "maxItems" <<"minItems" <<  "uniqueItems";
    schemaModels.insert("array", new QStringListModel(list));
}

FileController::~FileController()
{
    delete completer;
    delete textEdit;
}

bool FileController::isSchema()
{
    return isSch;
}

void FileController::setIsSchema(bool b)
{
    isSch = b;
}

void FileController::setConnectionsToTextEdit()
{
    textEdit->disconnect();
    textEdit->clearCurrentObjectName();
    connect(textEdit, SIGNAL(currentObjectUndefined()), this, SLOT(clearCompletions()));

    if(!isSch) {
        connect(textEdit, SIGNAL(currentObjectChanged(QString)), this, SLOT(updateCompletionsForCurrentObject(QString)));
    }else {
        connect(textEdit, SIGNAL(currentObjectTypeChanged(QString)), this, SLOT(setCurrentObjectType(QString)));
        connect(textEdit, SIGNAL(endOfCheckingCursorPositions()), this, SLOT(updateCompletionsForSchema()));
        createSchemaCompleter();
    }
}

void FileController::setAbsoluteFileName(QString name)
{
    absoluteFileName = name;
}

//------------------------DATA COMPLETER---------------------------------

bool FileController::isSchemaValid(QString schema)
{
    QJsonDocument textDocument =  QJsonDocument::fromJson(schema.toUtf8());
    if(!textDocument.isObject())
        return 0;
    return 1;
}

void FileController::createDataCompleter(QString path)
{
    QFile schemaFile;
    schemaFile.setFileName(path);
    schemaFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString schemaContents = schemaFile.readAll();
    schemaContents.remove('\n');
    schemaContents.replace('\\', "\\\\");

    if(!isSchemaValid(schemaContents)) {
        qDebug() <<  "Error: invalid schema. Can't create document completer";
        return;
    }

    //start create new data model
    model->clear();
    completer->setModel(0);

    QJsonObject schemaObject1 = QJsonDocument::fromJson(schemaContents.toUtf8()).object();
    createDataModel(schemaObject1, model->invisibleRootItem());
}


void FileController::clearCompletions() {
    completer->setModel(0);
}

void FileController::createDataModel(QJsonObject schema, QStandardItem * parentItem)
{
    if(schema.contains("properties")) {
        QStringList properties = schema.value("properties").toObject().keys();
        QJsonObject propertyObject = schema.value("properties").toObject();

        for(int i=0; i<properties.size(); i++) {
            QStandardItem *item = new QStandardItem(properties.at(i));
            QJsonObject ob = propertyObject.value(properties.at(i)).toObject();

            QList<QStandardItem*> lst;
            QStandardItem *itemType = new QStandardItem();
            QString strType = "-";

            if(ob.contains("type")) {
                QString str = ob.value("type").toString();
                if(!str.isEmpty()) {
                    if(str=="boolean")
                        strType = "bool";
                    else if(str=="null")
                        strType = str;
                    else
                        strType = str.left(3);
                }else {
                    QJsonArray arr = ob.value("type").toArray();
                    if(!arr.isEmpty())
                        strType = QString("[%1..]").arg(arr.at(0).toString().left(3));
                    else
                        strType = "-";
                }
            }
            itemType->setText(strType);
            lst.append(item);
            lst.append(itemType);
            parentItem->appendRow(lst);

            if(strType=="obj" )
                createDataModel(ob, item);
        }
    }
}

void FileController::updateCompletionsForCurrentObject(QString name)
{
    setCompletionsForObject(model->invisibleRootItem(), name);
}

void FileController::setCompletionsForObject(QStandardItem *parent, QString objectName)
{
    QStringList list;
    QStringList listOfTypes;

    if(objectName=="") {
        if(parent->hasChildren()) {
            for(int i=0; i<parent->rowCount(); i++) {
                list << parent->child(i, 0)->text();
                listOfTypes << parent->child(i, 1)->text();
            }
        }else
            qDebug() <<   QString("No properties for \"%1\" object").arg(objectName);
    }
    else {
        QList<QStandardItem*> par = model->findItems(objectName, Qt::MatchRecursive, 0);
        if(!par.isEmpty()) {
            if(par.at(0)->hasChildren()) {
                for(int i=0; i<par.at(0)->rowCount(); i++) {
                   list <<  par.at(0)->child(i,0)->text();
                   listOfTypes << par.at(0)->child(i, 1)->text();
                }
            }else
              qDebug() <<  QString("No properties for \"%1\" object").arg(objectName);
        }else
              qDebug() <<QString("Object \"%1\" not declared in schema").arg(objectName);
    }

    CompleterPopupModel* cModel = new CompleterPopupModel(list.size(), 2);
    for(int i=0; i<list.size(); ++i) {
        QStandardItem* item = new QStandardItem(list.at(i));
        cModel->setItem(i, 0, item);
        QStandardItem* item1 = new QStandardItem(listOfTypes.at(i));
        cModel->setItem(i, 1, item1);
    }
    completer->setModel(cModel);
}

//--------------------------SCHEMA COMPLETER--------------------------

void FileController::createSchemaCompleter()
{
    completer->setModel(0);
    model->clear();
}

void FileController::setCurrentObjectType(QString ty)
{
    currentObjectType = ty;
}

void FileController::updateCompletionsForSchema()
{
    if(textEdit->cursorUnderType) {
        QStringList list;
        list << "object" << "number" << "string" <<"integer" <<  "array" << "boolean" << "null";
        QStringListModel* typeModel = new QStringListModel(list);
        completer->setModel(typeModel);

    }else if(textEdit->cursorUnderKey)
    {
        if(!currentObjectType.isEmpty()) {
            if(schemaModels.contains(currentObjectType))
                completer->setModel(schemaModels[currentObjectType]);
            else
                completer->setModel(0);
        }else
            completer->setModel(schemaModels["object"]);
    }else
        completer->setModel(0);
}



