//
// Created by Jan Honsbrok on 03.09.19.
//


#include <iostream>
#include "song_info_utils.h"

int main(int argc, char *argv[]) {
  if (argc > 1) {
    const SongInfo song_info = SongInfoUtils::get_song_info(argv[1]);
    std::cout << "Path: " << argv[1] << std::endl;
    std::cout << "title: " << song_info.title << std::endl;
    std::cout << "artist: " << song_info.artist << std::endl;
    std::cout << "album: " << song_info.album << std::endl;

  } else {
    std::cerr << "No song provided!" << std::endl;
  }
  return 0;
}