#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QObject>
#include "textedit.h"
class QString;
class Highlighter;
class QStandardItem;
class QStandardItemModel;
class QCompleter;
class QStringListModel;
class QJsonObject;

class FileController : public QObject
{
    Q_OBJECT
public:
    explicit FileController(QObject *parent = 0);
    ~FileController();

    void createDataCompleter(QString);
    void createSchemaCompleter();
    void setAbsoluteFileName(QString);
    TextEdit* textEdit;
    QString absoluteFileName;
    bool isSchema();
    void setIsSchema(bool);
    void setConnectionsToTextEdit();

signals:

public slots:
    void updateCompletionsForCurrentObject(QString);
    void updateCompletionsForSchema();
    void setCurrentObjectType(QString);
    void clearCompletions();

private:
    bool isSch;
    void createDataModel(QJsonObject, QStandardItem *);
    bool isSchemaValid(QString);
    void setCompletionsForObject(QStandardItem *, QString);
    QStandardItemModel* model;
    QCompleter* completer;
  \
    QString currentObjectType;
    QMap<QString, QStringListModel*> schemaModels;
};

#endif // FILECONTROLLER_H
