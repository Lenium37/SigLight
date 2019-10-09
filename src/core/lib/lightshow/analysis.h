//
// Created by Johannes on 12.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_ANALYSIS_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_ANALYSIS_H_

#include <vector>
#include <sndfile.h>
#include <fftw3.h>
#include <iostream>
#include <math.h>
#include <complex>

struct time_value_int {
  float time;
  int value;
};

struct time_value_float {
  float time;
  float value;
};

struct time_value_double {
  float time;
  double value;
};

struct complex_result {
  std::vector<double> real;
  std::vector<double> imag;
};

class Analysis {
 public:
  Analysis();
  ~Analysis();

  void read_wav(char *filepath);
  void stft();
  std::vector<time_value_int> peaks_per_band(int f1, int f2);
  std::vector<time_value_double> get_intensity_function_values();
  std::vector<time_value_int> get_intensity_average_for_next_segment(std::vector<double> beats, int beats_per_minute, double first_good_beat);
  std::vector<time_value_int> get_intensity_changes(std::vector<time_value_int> intensities, int threshold);

  // Metal threshold = 0.804
  // Film threshold 0.292
  // Avicii threshold = 0.815
  std::vector<double> get_bpm_old(float threshold);
  float get_intensity_mean();
  void normalize();
  float get_length_of_song();
  long long factorial(long long n);
  void binomial_weights(int ma_window_size);
  float average(std::vector<time_value_float> data);
  std::vector<time_value_double> moving_average(int left, int right, std::vector<time_value_double> data);
  std::vector<time_value_float> moving_average_delta(std::vector<time_value_float> data, float average);
  std::vector<time_value_double> get_turningpoints(std::vector<time_value_double> data);
  complex_result fft_impulse(std::vector<double> signal);
  int get_bpm();
  double get_first_beat();
  std::vector<double> get_all_beats(int bpm, double firstBeat);
  void set_resolution(int resolution);
  int get_samplerate();




 private:
  float *wav_values_multichannel;
  float *wav_values_mono;
  std::vector<double> wav_values_mono_snippet;
  int signal_length_multichannel;
  int signal_length_mono;
  int hop_size = 1102;
  int window_size = 8192;
  int samplerate;
  float *window;
  int *max_peaks;
  float fft_max_value = 0;
  float fft_min_value = 0;
  int bpm_seconds_to_check = 7;
  int bpm = 0;

  fftw_complex *m_data, *m_fft_result, *m_ifft_result;
  fftw_plan m_plan_forward, m_plan_backward;


  // stft results for each window
  std::vector<std::vector<double> > result;
  std::vector<std::vector<int> > normalized_result;
  std::vector<int> normalized_values = std::vector<int>((this->window_size/2)+1);
  std::vector<long long> ma_weights;


  SNDFILE *wav_file;
  SF_INFO wav_info;

  int frames;
  int channels;

};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_ANALYSIS_H_
