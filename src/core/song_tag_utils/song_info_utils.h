//
// Created by Jan Honsbrok on 03.09.19.
//

#ifndef RASPITOLIGHT_SRC_CORE_SONG_TAG_UTILS_SONG_INFO_UTILS_H_
#define RASPITOLIGHT_SRC_CORE_SONG_TAG_UTILS_SONG_INFO_UTILS_H_

// Windows needs this
#define TAGLIB_STATIC

#include <string>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <utility>
#include <ghc/filesystem.hpp>

struct SongInfo {
 public:
  SongInfo(std::string title, std::string artist, std::string album);
  std::string title;
  std::string artist;
  std::string album;
  bool is_empty();
};

class SongInfoUtils {
 public:
  static SongInfo get_song_info(const std::string &song_path);
  static void write_song_information(const std::string &to, const SongInfo &song_info);
  static void copy_song_information(const std::string &from, const std::string &to);

 private:
  static std::string song_title_from_path(const std::string &song_path);
};

#endif //RASPITOLIGHT_SRC_CORE_SONG_TAG_UTILS_SONG_INFO_UTILS_H_
