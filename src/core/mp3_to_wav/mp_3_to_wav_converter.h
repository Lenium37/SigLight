//
// Created by Jan Honsbrok on 16.09.19.
//

#ifndef RASPITOLIGHT_SRC_CORE_MP3_TO_WAV_MP_3_TO_WAV_CONVERTER_H_
#define RASPITOLIGHT_SRC_CORE_MP3_TO_WAV_MP_3_TO_WAV_CONVERTER_H_

#include <string>

class Mp3ToWavConverter {
 public:
  static void convert_to_mp3(const std::string &from, const std::string &to);
  static bool is_avaible();
};

#endif //RASPITOLIGHT_SRC_CORE_MP3_TO_WAV_MP_3_TO_WAV_CONVERTER_H_
