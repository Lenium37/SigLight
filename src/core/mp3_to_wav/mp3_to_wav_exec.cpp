//
// Created by Jan Honsbrok on 16.09.19.
//

#include <song_info_utils.h>
#include "mp_3_to_wav_converter.h"
int main() {
  Mp3ToWavConverter::convert_to_mp3("/Users/Jan/Music/test/1-01 Arbeit ist out.mp3",
                                    "/Users/Jan/Music/test/from_mp3_.wav");
  return 0;
}