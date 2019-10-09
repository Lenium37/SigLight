//
// Created by Jan Honsbrok on 16.09.19.
//

#include "mp_3_to_wav_converter.h"

#include <iostream>
#include <song_info_utils.h>

void Mp3ToWavConverter::convert_to_mp3(const std::string &from, const std::string &to) {
  const std::basic_string<char, std::char_traits<char>, std::allocator<char>>
      &command = "lame --decode \"" + from + "\" \"" + to + "\"";
  const int result = system(command.c_str());

  const bool ok = result == 0;
  if (!ok) {
    std::cerr << "Please make sure lame is on your $PATH" << std::endl;
    return;
  }

  SongInfoUtils::copy_song_information(from, to);;
}
bool Mp3ToWavConverter::is_avaible() {
  return system("lame --version") == 0;
}
