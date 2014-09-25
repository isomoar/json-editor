#include "completerpopupmodel.h"

CompleterPopupModel::CompleterPopupModel(QObject *parent) :
    QStandardItemModel(parent)
{}


CompleterPopupModel::CompleterPopupModel(int rows, int columns, QObject *parent) : QStandardItemModel(parent)
{
    n_rows = rows;
    n_columns = columns;
}

QVariant CompleterPopupModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role==Qt::BackgroundColorRole) {
        return QColor(230,230,230);
    }
    if(role==Qt::FontRole) {
        QFont font;
        font.setFamily("MENLO");
        if(index.column()==1)
            font.setPointSize(9);
        return font;
    }

    if(role==Qt::SizeHintRole && index.column()==1)
        return QSize(45, 10);

//    if(role==Qt::SizeHintRole) {
//        return QSize(100,40);
//    }

    if(index.column()==1 && role == Qt::TextColorRole)
        return QColor(Qt::gray);

     return  QStandardItemModel::data(index, role);
}

QVariant CompleterPopupModel::headerData(int, Qt::Orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    return "1";
}

int CompleterPopupModel::rowCount(const QModelIndex &) const
{
    return n_rows;
}

int CompleterPopupModel::columnCount(const QModelIndex &) const
{
    return n_columns;
}
