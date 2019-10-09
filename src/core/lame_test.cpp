//
// Created by Jan on 21.08.2019.
//
#include <lame.h>
#include <fstream>

int main() {


  // working wav to mp3
  int read, write;

  FILE *pcm = fopen("C:\\Users\\Johannes\\Desktop\\Uni\\5_Raspi-to-light\\257ers-Abrakadabra.wav", "rb");
  FILE *mp3 = fopen("C:\\Users\\Johannes\\Desktop\\Uni\\5_Raspi-to-light\\257ers-Abrakadabra.mp3", "wb");

  const int PCM_SIZE = 8192;
  const int MP3_SIZE = 8192;

  short int pcm_buffer[PCM_SIZE*2];
  unsigned char mp3_buffer[MP3_SIZE];

  lame_t lame = lame_init();
  lame_set_in_samplerate(lame, 44100);
  lame_set_VBR(lame, vbr_default);
  lame_init_params(lame);

  do {
    read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);
    if (read == 0)
      write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
    else
      write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
    fwrite(mp3_buffer, write, 1, mp3);
  } while (read != 0);

  lame_close(lame);
  fclose(mp3);
  fclose(pcm);


  // mp3 to wav something like this
  /*
  hip_t hip = hip_decode_init();
  hip_decode(hip, mp3_buffer, sizeof(unsigned char), )

  hip_decode_exit(hip);
  */

  return 0;
}