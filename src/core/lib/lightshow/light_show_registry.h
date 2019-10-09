//
// Created by Jan on 26.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHT_SHOW_REGISTRY_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHT_SHOW_REGISTRY_H_

#include <string>
#include <logger.h>
#include "lightshow/lightshow.h"
#include <logger.h>
#include <song.h>
#include <map>

class LightShowRegistry {
 public:
  void register_lightshow_file(Song *song, std::shared_ptr<Lightshow> lightshow, const std::string &path_to_lightshow_directory);
  void register_song(Song *song, const std::string &path_to_lightshow_directory);
  std::shared_ptr<Lightshow> get_lightshow(Song *song);
  void add_existing_song_and_lightshow(Song *song, const std::string &path_to_lightshow_directory);
  void renew_lightshow_for_song(Song *song, std::shared_ptr<Lightshow> lightshow, const std::string &path_to_lightshow_directory);
 private:
  std::map<const std::string, std::shared_ptr<Lightshow>> lightshows;
  std::map<const std::string, std::string> lightshow_files;
  void log_content();
  void write_lightshow(const std::string &lightshow_filename, std::shared_ptr<Lightshow> lightshow);
  std::shared_ptr<Lightshow> read_lightshow(const std::string file_path);
};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_LIGHT_SHOW_REGISTRY_H_
