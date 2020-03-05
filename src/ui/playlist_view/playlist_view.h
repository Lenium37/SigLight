#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#define STANDARD_COLUMN_WIDTH 150

#include <QMouseEvent>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>

#include "playlist_and_player/playlist_item.h"
#include "song.h"
#include "not_editable_delegate.h"
#include "playlistviewmodel.h"
#include "playlistviewstyle.h"


class Playlist_view : public QTableView
{
    Q_OBJECT

public:
   Playlist_view(QWidget *parent = nullptr);

    void add_to_playlist_view(Playlist_item *playlist_item);
    int delete_current_selected_song();
    void reset_every_lightshow_status();
    void reset_lightshow_status(int index);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:
    void slot_song_lightshow_state_has_changed(int index_in_playlist);

protected slots:
    /* Out of Order -> still here so it can posibly be used in the future
    void slot_item_has_been_changed(QModelIndex modelindex_top_left);*/
    void slot_rows_inserted(const QModelIndex &parent, int first, int last);
    void slot_rows_removed(const QModelIndex &parent, int first, int last);
    void slot_rows_have_been_moved(const QModelIndex &topLeft);


signals:
    /* Out of Order -> still here so it can posibly be used in the future
    void song_title_or_artist_has_changed(int playlist_index, bool string_is_title, std::string title_or_artist);*/
    void playlist_order_has_been_changed(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index);
    void activated(QPersistentModelIndex index);

private:
    PlaylistViewModel *playlist_view_model;
    int rows_inserted_first_index;
    int rows_inserted_last_index;
    int rows_removed_first_index;
    int rows_removed_last_index;
    bool is_drag_drop;
};

#endif // PLAYLIST_VIEW_H
