//
// Created by Jan on 20.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_SONG_H_
#define RASPITOLIGHT_SRC_CORE_LIB_SONG_H_

#include <string>
#include <ostream>
class Song {
 public:
  Song();

  explicit Song(const std::string &file_path);
  const std::string get_file_path() const;
  void set_file_path(std::string _file_path);
  explicit Song(const std::string &file_path, const int duration_in_sec);
  const std::string get_song_name();
  const int get_duration();

  friend std::ostream &operator<<(std::ostream &os, const Song &song);
  bool operator==(const Song &rhs) const;
  bool operator!=(const Song &rhs) const;
  bool is_empty();

  std::string get_formatted_duration();
  std::string get_title();
  void set_title(std::string title);
  std::string get_artist();
  void set_artist(std::string artist);

 private:
  std::string file_path;
  int duration_in_sec;
  std::string formatted_duration;
  std::string artist;
  std::string title;

};

#endif //RASPITOLIGHT_SRC_CORE_LIB_SONG_H_
