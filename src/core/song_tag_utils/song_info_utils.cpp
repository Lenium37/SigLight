//
// Created by Jan Honsbrok on 03.09.19.
//

#include <cassert>
#include "song_info_utils.h"

SongInfo SongInfoUtils::get_song_info(const std::string &song_path) {
  TagLib::FileRef f(song_path.c_str());

  if (!f.isNull() && f.tag()) {
    TagLib::Tag *tag = f.tag();
    SongInfo info = SongInfo(tag->title().to8Bit(), tag->artist().to8Bit(), tag->album().to8Bit());
    if (!info.is_empty()) {
      return info;
    }
  }
  return {song_title_from_path(song_path), "", ""};
}
std::string SongInfoUtils::song_title_from_path(const std::string &song_path) {
  return ghc::filesystem::path(song_path).filename().replace_extension("");
}
void SongInfoUtils::write_song_information(const std::string &to, const SongInfo &song_info) {
  TagLib::FileRef g(to.c_str());

  g.tag()->setTitle(song_info.title);
  g.tag()->setArtist(song_info.artist);
  g.tag()->setAlbum(song_info.album);

  assert(g.save());
}
void SongInfoUtils::copy_song_information(const std::string &from, const std::string &to) {
  SongInfo song_info = get_song_info(from);
  write_song_information(to, song_info);
}

SongInfo::SongInfo(std::string title, std::string artist, std::string album)
    : title(std::move(title)), artist(std::move(artist)), album(std::move(album)) {}

bool SongInfo::is_empty() {
  return title.empty() && artist.empty() && album.empty();
}
