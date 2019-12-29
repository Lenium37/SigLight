#include "playlist_view.h"

 #include "logger.h"

Playlist_view::Playlist_view(QWidget *parent)
    : QTableView(parent)

{
    horizontalHeader()->setStretchLastSection(true);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setAlternatingRowColors(true);

    this->setDropIndicatorShown(true);
    this->setDragDropOverwriteMode(false);
    this->setDragEnabled(true);
    this->viewport()->setAcceptDrops(true);
    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setStyle(new PlaylistViewStyle());

    /*setColumnWidth(0, 10);
    setColumnWidth(1, STANDARD_COLUMN_WIDTH);
    setColumnWidth(2, STANDARD_COLUMN_WIDTH);
    setColumnWidth(3, STANDARD_COLUMN_WIDTH);
    setColumnWidth(4, STANDARD_COLUMN_WIDTH);
    setColumnWidth(5, STANDARD_COLUMN_WIDTH);*/

    //this->resizeColumnsToContents();


    /* Out of Order -> still here so it can posibly be used in the future
     * these delegates were used so only 2 colums would be editable
    this->setItemDelegateForColumn(0, new Not_editable_delegate(this));
    this->setItemDelegateForColumn(3, new Not_editable_delegate(this));
    this->setItemDelegateForColumn(4, new Not_editable_delegate(this));
    this->setItemDelegateForColumn(5, new Not_editable_delegate(this));
    */

    playlist_view_model = new PlaylistViewModel(this);

    setModel(playlist_view_model);

    /* Out of Order -> still here so it can posibly be used in the future
    connect(this->model(), &QAbstractItemModel::dataChanged, this, &Playlist_view::slot_item_has_been_changed);*/
    connect(this->model(), &QAbstractItemModel::rowsRemoved, this, &Playlist_view::slot_rows_removed);
    connect(this->model(), &QAbstractItemModel::rowsInserted, this, &Playlist_view::slot_rows_inserted);
    connect(this->model(), &QAbstractItemModel::dataChanged, this, &Playlist_view::slot_rows_have_been_moved);

    is_drag_drop = false;
}

void Playlist_view::add_to_playlist_view(Playlist_item *playlist_item)
{
    QList<QStandardItem *> items;
    items.append(new QStandardItem(QString::fromStdString(playlist_item->get_song()->get_title())));
    items.last()->setDropEnabled(false);
    items.last()->setDragEnabled(true);
    items.append(new QStandardItem(QString::fromStdString(playlist_item->get_song()->get_artist())));
    items.last()->setDropEnabled(false);
    items.last()->setDragEnabled(true);
    items.append(new QStandardItem(QString::fromStdString(playlist_item->get_song()->get_formatted_duration())));
    items.last()->setDropEnabled(false);
    items.last()->setDragEnabled(true);
    items.append(new QStandardItem(QString::fromStdString(playlist_item->get_song()->get_song_name())));
    items.last()->setDropEnabled(false);
    items.last()->setDragEnabled(true);
    items.append(new QStandardItem());
    items.last()->setDropEnabled(false);
    items.last()->setDragEnabled(true);

    if(playlist_item->get_lightshow_status()) {
        items.last()->setData(QIcon(":/icons_svg/svg/iconfinder_014_-_Volume_2949882.svg"), Qt::DecorationRole);
    }
    else {
        items.last()->setData(QIcon(":/icons_svg/svg/uncheck.svg"), Qt::DecorationRole);
    }

    playlist_view_model->appendRow(items);
}

void Playlist_view::reset_every_lightshow_status() {
  for(int i = 0; i < this->playlist_view_model->rowCount(); i++)
    playlist_view_model->item(i, 4)->setData(QIcon(":/icons_svg/svg/uncheck.svg"), Qt::DecorationRole);
}

int Playlist_view::delete_current_selected_song()
{
    Logger::trace("Playlist_view::delete_current_selected_song");
    int row_to_delete = this->currentIndex().row();
    playlist_view_model->removeRow(row_to_delete);
    this->setCurrentIndex(this->indexAt(QPoint(row_to_delete, 0)));
    return row_to_delete;
}

/* Out of Order -> still here so it can posibly be used in the future
 *
 * was the ability to edit tile and artist -> got shutdown for now
void Playlist_view::slot_item_has_been_changed(QModelIndex modelindex_top_left)
{       
    std::string title_or_artist;
    if(modelindex_top_left.column() == 1){
        title_or_artist = modelindex_top_left.data().toString().toStdString();
        emit song_title_or_artist_has_changed(modelindex_top_left.row(), true, title_or_artist);
    }
    else {
        title_or_artist = modelindex_top_left.data().toString().toStdString();
        emit song_title_or_artist_has_changed(modelindex_top_left.row(), false, title_or_artist);
    }
}
*/
void Playlist_view::slot_rows_inserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    this->rows_inserted_first_index = first;
    this->rows_inserted_last_index = last;
}

void Playlist_view::slot_rows_removed(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    this->rows_removed_first_index = first;
    this->rows_removed_last_index = last;
    if(is_drag_drop){
        emit this->playlist_order_has_been_changed(rows_removed_first_index, rows_removed_last_index, rows_inserted_first_index, rows_inserted_last_index);
        is_drag_drop = false;
    }
}

void Playlist_view::slot_rows_have_been_moved(const QModelIndex &topLeft)
{
    if(topLeft.column() == 0){
        is_drag_drop = true;
    }
}

void Playlist_view::slot_song_lightshow_state_has_changed(int index_in_playlist)
{
    playlist_view_model->item(index_in_playlist, 4)->setData(QIcon(":/icons_svg/svg/apply.svg"), Qt::DecorationRole);
}
