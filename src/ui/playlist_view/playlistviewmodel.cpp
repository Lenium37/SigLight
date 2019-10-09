#include "playlistviewmodel.h"


#include <logger.h>

PlaylistViewModel::PlaylistViewModel(QObject *parent) : QStandardItemModel(parent)
{
    row_count = 0;
}

int PlaylistViewModel::rowCount(const QModelIndex & /*parent*/) const
{
   return row_count;
}

int PlaylistViewModel::columnCount(const QModelIndex & /*parent*/) const
{
    return COLS;
}


QVariant PlaylistViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole){
        if(orientation == Qt::Horizontal){
            switch(section) {
            case 0:
                return QString("Title");
            case 1:
                return QString("Artist");
            case 2:
                return QString("Length");
            case 3:
                return QString("File name");
            case 4:
                return QString("Lightshowstatus");
            } 
        }
        if(orientation == Qt::Vertical){
            return QStandardItemModel::headerData(section, orientation, role).toInt() - 1;
        }
    }
    return QVariant();
}


QStringList PlaylistViewModel::mimeTypes() const
{
    return QStringList{"application/x-qabstractitemmodeldatalist", "application/x-qstandarditemmodeldatalist"};
}

bool PlaylistViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Logger::trace("PlaylistViewModel::dropMimeData");
    Q_UNUSED(column);
    return QStandardItemModel::dropMimeData(data, action, row, 0, parent);
}

void PlaylistViewModel::appendRow(const QList<QStandardItem *> &items)
{
    QStandardItemModel::appendRow(items);
    row_count++;
}

bool PlaylistViewModel::removeRow(int row)
{
    bool successful = QAbstractItemModel::removeRow(row);
    if(successful)
        row_count--;
    return successful;
}

Qt::ItemFlags PlaylistViewModel::flags(const QModelIndex &index) const
{
    return (QStandardItemModel::flags(index) & ~Qt::ItemIsEditable);
}

