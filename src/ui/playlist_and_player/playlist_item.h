#ifndef PLAYLIST_ITEM_H
#define PLAYLIST_ITEM_H

#include <QObject>
#include "song.h"

class Playlist_item : public QObject
{
public:
    Playlist_item(Song *items_song);
    ~Playlist_item();

    Song* get_song();
    bool get_lightshow_status();
    void toggle_lightshow_status();
    void set_lightshow_status(bool status);

protected slots:

signals:
    void lighshow_status_changed(Playlist_item *item);


private:
    Song *my_song;
    bool is_lightshow_ready_to_play;
};

#endif // PLAYLIST_ITEM_H
