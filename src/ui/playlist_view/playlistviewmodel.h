#ifndef PLAYLISTVIEWMODEL_H
#define PLAYLISTVIEWMODEL_H

#include <QStandardItemModel>

const int COLS = 5;

class PlaylistViewModel : public QStandardItemModel
{
    Q_OBJECT
public:
    PlaylistViewModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QStringList mimeTypes() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    void appendRow(const QList<QStandardItem*> &items);
    bool removeRow(int row);
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    int row_count;
};

#endif // PLAYLISTVIEWMODEL_H
