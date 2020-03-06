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
#include "Gist.h"
#include "Chromagram.h"
//#include <essentia.h>
//#include <algorithmfactory.h>
//#include <algorithm.h>


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

struct fluxes {
  float time;
  float ed;
  float sd;
  float sdhwr;
  float csd;
  float hfc;
  float l2nh;
  float l1nh;
  float l2h;
  float l1h;
};

class Analysis {
 public:
  Analysis();
  ~Analysis();

  std::vector<float> get_onset_timestamps_energy_difference(float onset_value);
  std::vector<float> get_onset_timestamps_frequencies(float f_start, float f_end);
  void read_wav(char *filepath);
  void stft();
  std::vector<time_value_int> peaks_per_band(int f1, int f2);
  std::vector<time_value_double> get_intensity_function_values();
  std::vector<time_value_int> get_intensity_average_for_next_segment(std::vector<double> beats, int beats_per_minute, double first_good_beat);
  std::vector<time_value_int> get_intensity_changes(std::vector<time_value_int> intensities, int threshold);

  std::vector<std::vector<float> > get_stmfcc(float *signal_values, int bin_size, int hop_size);
  std::vector<std::vector<float> > get_chromagram(float *signal_values, int bin_size, int hop_size, int song_samplerate);
  std::vector<std::vector<float> > get_spectrogram(std::vector<float> audio, int bin_size, int hop_size);
  float get_cosine_distance(std::vector<float> m, std::vector<float> n);
  float get_exponential_cosine_distance(std::vector<float> m, std::vector<float> n);
  std::vector<std::vector<float> > get_self_similarity_matrix(std::vector<std::vector<float>> window, int distance_formula);
  std::vector<std::vector<float> > get_filter_kernel(int song_bpm, int bin_size, int song_samplerate);
  std::vector<time_value_float> get_novelty_function(std::vector<std::vector<float> > ssm, std::vector<std::vector<float> > kernel, int cut_seconds_end, int bin_size, int song_samplerate, int N);
  std::vector<time_value_float> get_extrema(std::vector<time_value_float> novelty_function);
  float get_middle_tvf(std::vector<time_value_float> v);
  float get_variance_tvf(std::vector<time_value_float> v, float middle);
  float get_standard_deviation_tvf(std::vector<time_value_float> v, float variance);
  std::vector<time_value_float> filter_extrema(std::vector<time_value_float> extrema, float middle, float middle_factor, float variance, float standard_deviation, int bpm, bool filter_by_bars);
  void make_csv_timeseries_tvf(std::vector<time_value_float> v, char *directory, char *filename );
  void make_csv_matrix_f(std::vector<std::vector<float>> v, char *directory, char *filename );

  std::vector<time_value_float> get_segments();

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
  void set_bpm(int _bpm);
  double get_first_beat();
  std::vector<double> get_all_beats(int bpm, double firstBeat);
  void set_resolution(int resolution);
  int get_samplerate();
  int get_spectral_flux();

  ////////////////////////////////
  // START ///////////////////////
  // CUSTOM ESSENTIA EXTRACTORS //
  ////////////////////////////////

  //int onset_detection(char* songname);

  //int get_onset_times(char* songname);

  ////////////////////////////////
  // END /////////////////////////
  // CUSTOM ESSENTIA EXTRACTORS //
  ////////////////////////////////



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
