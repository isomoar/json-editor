#ifndef COMPLETERPOPUPMODEL_H
#define COMPLETERPOPUPMODEL_H

#include <QStandardItemModel>

class CompleterPopupModel : public QStandardItemModel
{
    Q_OBJECT
public:
   CompleterPopupModel(QObject *parent = 0);
   CompleterPopupModel(int rows, int columns, QObject * parent = 0);
signals:
private:
    int n_rows;
    int n_columns;
public slots:
public:
     QVariant data(const QModelIndex &index, int role) const;
     virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
};

#endif // COMPLETERPOPUPMODEL_H
