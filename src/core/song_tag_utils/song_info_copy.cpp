//
// Created by Jan Honsbrok on 03.09.19.
//

#include "song_info_utils.h"

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << "Copy from " << argv[1] << " to " << argv[2] << std::endl;
    SongInfoUtils::copy_song_information(argv[1], argv[2]);
  } else {
    std::cerr << "No songs provided!" << std::endl;
  }
  return 0;
}