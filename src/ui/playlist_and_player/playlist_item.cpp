#include "playlist_item.h"

Playlist_item::Playlist_item(Song *items_song)
    : QObject()
    , my_song(items_song)
    , is_lightshow_ready_to_play(false)
{
}

Playlist_item::~Playlist_item()
{
}

Song *Playlist_item::get_song()
{
    return this->my_song;
}

void Playlist_item::set_lightshow_status(bool status) {
  this->is_lightshow_ready_to_play = status;
  //emit this->lighshow_status_changed(this);
}

bool Playlist_item::get_lightshow_status() {
  return this->is_lightshow_ready_to_play;
}


void Playlist_item::toggle_lightshow_status()
{
    if(is_lightshow_ready_to_play == false)
      this->is_lightshow_ready_to_play = true;
    else
      this->is_lightshow_ready_to_play = false;

    //emit this->lighshow_status_changed(this);
}
