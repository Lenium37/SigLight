//
// Created by Steven Drewers on 2019-05-05.
//

#include <logger.h>
#include "analysis.h"

// TODO: REMOVE DUPLICATE CODE
// TODO: REMOVE OTHER GARBAGE
// TODO: TIDY CODE
// TODO: COMMENT CODE// TODO: TIDY CODE
// TODO: MAKE CODE COMPLY TO AGREED UPON FLOWCHART (I/O)
// TODO: MAKE THE WHOLE THING WORK

void Analysis::read_wav(char *filepath){

  wav_info.format = 0;

  wav_file = sf_open(filepath, SFM_READ, &wav_info);

  if (wav_file == NULL)
  {
    printf("Datei kann nicht gelesen werden...\n");
    exit(69);
  }

  frames = wav_info.frames;
  channels = wav_info.channels;
  samplerate = wav_info.samplerate;
  signal_length_multichannel = frames * channels;
  signal_length_mono = frames;

  wav_values_multichannel = (float *) malloc(2 * signal_length_multichannel * sizeof(float));
  wav_values_mono = (float *) malloc(signal_length_mono * sizeof(float));


  sf_readf_float(wav_file, wav_values_multichannel, signal_length_multichannel);

  sf_close(wav_file);

  // convert to mono
  int max_sample = 0;
  float max_sample_value = 0;
  for (int i = 0; i < signal_length_multichannel; i+=channels ){

    // add values for each channel and get arithmetic middle by (c1+...+cn)/n
    float combined_channel_value = 0;
    for (int n = 0; n < channels; n++){
      combined_channel_value += wav_values_multichannel[i+n];
    }
    wav_values_mono[i/channels] += (combined_channel_value / channels);
    if (combined_channel_value/channels > max_sample_value && i/channels > signal_length_mono/2 && i/channels < signal_length_mono - (4*44100)){
      max_sample_value = combined_channel_value/channels;
      max_sample = i/channels;
    }

  }

  //printf("time: %d, value: %f\n", max_sample/44100, max_sample_value);

  /** TODO:
   * HIER MÜSST MAN FÜNF MARKANTE SEKUNDEN AUS DEM SONG SUCHEN,
   * DIE HOHE ENERGIE IM TIEFEN FREQUENZBEREICH HABEN
   * MOMENTAN WIRD DIE HÄLFTE DES SONGS GENOMMEN, DAS GEHT IN ÜBER 90% DER FÄLLE KLAR
   * EVTL.
  */
  if(signal_length_mono > 15 * samplerate) {
    float wert = 0;
    //printf("from: %d, to: %d\n", (max_sample - (bpm_seconds_to_check*44100/2))/44100, (max_sample + (bpm_seconds_to_check*44100/2))/44100);
    for (int i = max_sample - (bpm_seconds_to_check*44100/2); i < max_sample + (bpm_seconds_to_check*44100/2); i++) {
      wert = fabsf((wav_values_mono[i + 1] - wav_values_mono[i]));
      wav_values_mono_snippet.push_back(wert);
      //printf("wert: %f", wert);
    }
  }

  free(wav_values_multichannel);

}

void Analysis::stft(){

  float normalization_factor = (2.0 * 1.63) / samplerate;

  fftw_complex    *data, *fft_result, *ifft_result;
  fftw_plan       plan_forward, plan_backward;

  data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );
  fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );
  ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );

  plan_forward  = fftw_plan_dft_1d( window_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

  // construct window
  float *buffer;
  window = (float *) malloc(window_size * sizeof(float));
  for(int i = 0; i < window_size; i++) {
    window[i] = 0.54 - (0.46 * cos( 2 * M_PI * (i / ((window_size - 1) * 1.0))));
  }

  int chunk_position = 0;

  int read_index;

  // Should we stop reading in chunks?
  int b_stop = 0;
  int num_chunks = 0;

  // go for it
  while (chunk_position < signal_length_mono && !b_stop){

    for (int i = 0; i < window_size; i++){
      read_index = chunk_position + i;

      if (read_index < signal_length_mono){
        data[i][0] = wav_values_mono[read_index] * window[i];
        data[i][1] = 0.0;
      }
      else {
        data[i][0] = 0.0;
        data[i][1] = 0.0;
        b_stop = 1;
      }
    }
    fftw_execute(plan_forward);
    std::vector<double> window_values = std::vector<double>(window_size/2);
    for (int i = 0; i < window_values.size(); i++){

      //window_values[i] = 10*log10f(((fft_result[i][0]*fft_result[i][0])+(fft_result[i][1]*fft_result[i][1]))*normalization_factor);
      window_values[i] = 10*log10f(((fft_result[i][0]*fft_result[i][0])+(fft_result[i][1]*fft_result[i][1]))*((fft_result[i][0]*fft_result[i][0])+(fft_result[i][1]*fft_result[i][1]))*normalization_factor);


      if (window_values[i] > fft_max_value){
        fft_max_value = window_values[i];
      }

    }
    //result.push_back(window_values);
    result.emplace_back(window_values);
    chunk_position += hop_size;
    num_chunks++;

  }

  Logger::debug("result.size(): {}", result.size());

  std::cout << "result[0].size(): " << result[0].size() << std::endl;

  fftw_destroy_plan( plan_forward );
  fftw_free( data );
  fftw_free( fft_result );
  fftw_free( ifft_result );

}

std::vector<float> Analysis::get_onset_timestamps(){

  std::vector<float> onset_timestamps;

  //Gist<double> gist(4096, 44100);
  /*for(int i = 0; i < result.size(); i ++) {
    gist.processAudioFrame (result[i]);
    float hfc = gist.highFrequencyContent();
    std::cout << "hfc(" << i << "):" << hfc << std::endl;
  }*/
  /*for(int i = 0; i < result.size(); i++) {
    gist.processAudioFrame (result[i]);
    float ed = gist.energyDifference();
    //std::cout << "ed(" << i << "): " << ed << std::endl;
  }*/

  int window_size_onsets = 2048;
  Gist<float> gist2(window_size_onsets, 44100);
  float audioFrame[window_size_onsets];
  float last_value = 0;
  float last_time = 0;
  bool onset_found = false;
  bool already_added_this_onset = false;
  std::vector<time_value_float> onsets;
  onsets.resize(signal_length_mono / window_size_onsets / 2);
  float min_value_onset = 0;

  for(int i = 0; i < signal_length_mono - window_size_onsets; i = i + window_size_onsets / 2) {
    for(int j = i, k = 0; k < window_size_onsets; j++, k++)
      audioFrame[k] = wav_values_mono[j];

    gist2.processAudioFrame (audioFrame, window_size_onsets);
    float ed = gist2.energyDifference();
    float x = i;
    float time = x/44100;

    //if(ed > 0)
      //std::cout << "ed(" << time << "): " << ed << std::endl;

    onsets.push_back({time, ed});
  }

  for(int i = 0; i < onsets.size(); i++) {
      min_value_onset += onsets[i].value;
  }
  min_value_onset = min_value_onset / onsets.size() * 6;
  float threshold_reset = min_value_onset * 0.2;

  for(time_value_float onset: onsets) {
    //std::cout << "ed(" << onset.time << "): " << onset.value << std::endl;
    if(onset.value > min_value_onset)
      onset_found = true;

    if(onset_found) {
      if(onset.value < last_value && !already_added_this_onset) {
        onset_timestamps.emplace_back(last_time);
        already_added_this_onset = true;
      }
    }

    if(onset.value < threshold_reset) {
      onset_found = false;
      already_added_this_onset = false;
    }

    last_value = onset.value;
    last_time = onset.time;
  }

  std::cout << "min_value_onset: " << min_value_onset << std::endl;
  std::cout << "threshold_reset: " << threshold_reset << std::endl;
  //for(float ts: onset_timestamps)
    //std::cout << ts << std::endl;

  return onset_timestamps;
}

std::vector<time_value_double> Analysis::get_intensity_function_values(){

  int block_size = 44100;
  int segment_duration = 60/169*16;
  int jump_size =  block_size/3;

  std::vector<time_value_double> intensities = std::vector<time_value_double>();
  float average_window_value = 0;
  float normalization_factor = (2.0 * 1.63) / samplerate;

  fftw_complex    *data, *fft_result, *ifft_result;
  fftw_plan       plan_forward, plan_backward;

  data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );
  fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );
  ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );

  plan_forward  = fftw_plan_dft_1d( block_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

  // construct window
  float *buffer;
  window = (float *) malloc(block_size * sizeof(float));
  for(int i = 0; i < block_size; i++) {
    window[i] = 0.54 - (0.46 * cos( 2 * M_PI * (i / ((block_size - 1) * 1.0))));
  }

  int chunk_position = 0;

  int read_index;

  // Should we stop reading in chunks?
  int b_stop = 0;
  int num_chunks = 0;



  // go for it
  while (chunk_position < signal_length_mono && !b_stop){

    for (int i = 0; i < block_size; i++){
      read_index = chunk_position + i;

      if (read_index < signal_length_mono){
        data[i][0] = wav_values_mono[read_index] * window[i];
        data[i][1] = 0.0;
      }
      else {
        data[i][0] = 0.0;
        data[i][1] = 0.0;
        b_stop = 1;
      }
    }
    fftw_execute(plan_forward);
    std::vector<double> window_values = std::vector<double>(block_size/2);
    for (int i = 0; i < window_values.size(); i++){

      window_values[i] = (
          (
              (
                  (fft_result[i][0]*fft_result[i][0])
                      +
                          (fft_result[i][1]*fft_result[i][1])
              )
          )
              *
                  normalization_factor
      );
      // weighting!!!
      // make frequencies from i
      // f = i * block_size / samplerate
      float f = (float) i * samplerate / block_size;
      float weight = 1;
      // unomment for weighting based on frequency!
      // TODO: find useful weighting!

      if (f < 1){
        weight = 0;
      }
      else if (f >= 1 && f <= 150){
        weight = 8;
      }
      else if (f > 150 && f <= 2000){
        weight = 16;
      }
      else if (f > 2000 && f <= 6000){
        weight = 8;
      }
      else if (f > 6000 && f <= 16000 ){
        weight = 0;
      }
      else if (f > 16000){
        weight = 0;
      }

      average_window_value = average_window_value + (window_values[i]*weight);


    }

    average_window_value /= ((float) block_size/2);
    time_value_double current = {0.0, 0.0};
    current.time = ((num_chunks*jump_size)+(block_size/2.0))/samplerate;
    current.value = average_window_value;

    intensities.push_back(current);
    chunk_position += jump_size;
    num_chunks++;

  }

  fftw_destroy_plan( plan_forward );
  fftw_free( data );
  fftw_free( fft_result );
  fftw_free( ifft_result );

  return intensities;

};

std::vector<time_value_int> Analysis::get_intensity_average_for_next_segment(std::vector<double> beats, int beats_per_minute, double first_good_beat){

  // TODO: 4 beats, also 1 takt in die zukunft schauen um average intensität für diesen takt zu berechnen
  int block_size = (60 * 4 * samplerate) / beats_per_minute;
  // TODO: alle 4 beats, also jeden takt average intensity des kommenden takts bestimmen
  int jump_size = (60 * 4 * samplerate) / beats_per_minute;
  while (first_good_beat > jump_size){
    first_good_beat -= jump_size;
  }
  int chunk_position = first_good_beat;

  std::vector<time_value_double> intensities = std::vector<time_value_double>();
  std::vector<time_value_int> intensities_normalized = std::vector<time_value_int>();
  float normalization_factor = (2.0 * 1.63) / samplerate;

  fftw_complex    *data, *fft_result, *ifft_result;
  fftw_plan       plan_forward, plan_backward;

  data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );
  fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );
  ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * block_size );

  plan_forward  = fftw_plan_dft_1d( block_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

  // construct window
  float *buffer;
  window = (float *) malloc(block_size * sizeof(float));
  for(int i = 0; i < block_size; i++) {
    window[i] = 0.54 - (0.46 * cos( 2 * M_PI * (i / ((block_size - 1) * 1.0))));
  }

  int read_index;

  // Should we stop reading in chunks?
  int b_stop = 0;
  int num_chunks = 0;

  // go for it
  double max_intensity = 0;

  Logger::debug("");
  Logger::debug("#####   Alle Variablen, die in der while Relevanz haben:");
  Logger::debug("chunk_position: {}", chunk_position);
  Logger::debug("first_good_beat: {}", first_good_beat);
  Logger::debug("signal_length_mono: {}", signal_length_mono);
  Logger::debug("b_stop: {}", b_stop);
  Logger::debug("block_size: {}", block_size);
  Logger::debug("window[0]: {}", window[0]);
  Logger::debug("bpm: {}", beats_per_minute);
  Logger::debug("normalization_factor: {}", normalization_factor);
  Logger::debug("#####");
  Logger::debug("");

  while (chunk_position < signal_length_mono && !b_stop) {
    float average_window_value = 0;

    for (int i = 0; i < block_size; i++){
      read_index = chunk_position + i;

      if (read_index < signal_length_mono){
        data[i][0] = wav_values_mono[read_index] * window[i];
        data[i][1] = 0.0;
      }
      else {
        data[i][0] = 0.0;
        data[i][1] = 0.0;
        b_stop = 1;
      }
    }
    fftw_execute(plan_forward);
    std::vector<double> window_values = std::vector<double>(block_size/2);
    for (int i = 0; i < window_values.size(); i++){

      window_values[i] = sqrt(
          (
              (
                  (fft_result[i][0]*fft_result[i][0])
                      +
                          (fft_result[i][1]*fft_result[i][1])
              )
          )
              *
                  normalization_factor
      );
      // weighting!!!
      // make frequencies from i
      // f = i * block_size / samplerate
      float f = (float) i * samplerate / block_size;
      float weight = 1;
      // unomment for weighting based on frequency!
      /* TODO: gewichtung der einzelnen frequenzanteile zur berechnung der average intensity für einen takt
       * evtl. kann man das noch basierend auf gesamtem spektrum automatisch festlegen.
       * geht sicherlich eleganter...
       * hier werden freuenz von 1Hz bis 6kHz berücksichtigt
       * 1-150Hz mit Gewichtung 2 (Hier liegt Bass) weight=2
       * 151-2000 (Hier liegt der ganze wichtige Rest) weight = 1.5
       * 2001-6000 (Hier liegt auch noch Kram, aber nicht mehr so wichtig = weight = 0.5
       * */

      if (f < 1){
        weight = 0;
      }
      else if (f >= 1 && f <= 150){
        weight = 2;
      }
      else if (f > 150 && f <= 2000){
        weight = 1.5;
      }
      else if (f > 2000 && f <= 6000){
        weight = 0.5;
      }
      else if (f > 6000 && f <= 16000 ){
        weight = 0;
      }
      else if (f > 16000){
        weight = 0;
      }

      //Logger::info("window_values[i]: {}", window_values[i]);
      average_window_value += (window_values[i]*weight);

    }

    average_window_value /= ((float) block_size/2);

    time_value_double current = {0.0, 0.0};
    current.time = ((double)(num_chunks*jump_size)/samplerate)+first_good_beat/44100;
    current.value = average_window_value;
    if (average_window_value > max_intensity){
      max_intensity = average_window_value;
    }
    intensities.push_back(current);
    chunk_position += jump_size;
    num_chunks++;

    //Logger::info("average_window_value: {}", average_window_value);
  }

  fftw_destroy_plan( plan_forward );
  fftw_free( data );
  fftw_free( fft_result );
  fftw_free( ifft_result );


  for(int i = 0; i < intensities.size(); i++){
    double percent = intensities[i].value / max_intensity;
    int cut = 100 * percent;
    intensities_normalized.push_back({intensities[i].time, cut});
  }



  // normalize all values [0-1] with ma_intensity




  // go through intensities and set intensity levels
  /*
  std::vector<time_value_double> averages;
  double mom_average = 0;
  for (int i = 0; i < intensities.size(); i+=16){
    mom_average = 0;
    for (int n = 0; n < 16; n++){
      mom_average += intensities[i+n].value;
    }
    averages.push_back({intensities[i].time, mom_average});
  }
  */
  /*
  double intensity_max = 0;
  double intensity_average = 0;
  double intensity_min = 9999;

  for(int i = 0; i < intensities.size(); i++){
    intensities[i].time = beats[i]/44100;

    intensity_average += intensities[i].value / intensities.size();
    if (intensities[i].value > intensity_max){
      intensity_max = intensities[i].value;
    }
    if (intensities[i].value > 0 && intensities[i].value < intensity_min){
      intensity_min = intensities[i].value;
    }

  }

  double int_2 = (intensity_average + intensity_max / 1.2);
  double int_1 = (intensity_average + intensity_min) / 2;
   */
/*
  for(int i = 0; i < intensities.size(); i++){
    if (intensities[i].value >= int_2){
      intensities[i].value = 2;
    }
    else if (intensities[i].value >= int_1 && intensities[i].value < int_2){
      intensities[i].value = 1;
    }
    else {
      intensities[i].value = 0;
    }
  }
*/
  return intensities_normalized;

};

std::vector<time_value_int> Analysis::get_intensity_changes(std::vector<time_value_int> intensities, int threshold){

  std::vector<time_value_int> intensity_changes;
  intensity_changes.push_back({intensities[0].time, intensities[0].value});
  for (int i = 0; i < intensities.size(); i++){
    int a = intensity_changes.size()-1;
    int percent_now = intensities[i].value;
    if(percent_now > intensity_changes[a].value+threshold || percent_now < intensity_changes[a].value-threshold){
      //intensities[i].value = cut_now;
      intensity_changes.push_back({intensities[i].time, intensities[i].value});
    }
  }
  return intensity_changes;
}

float Analysis::get_intensity_mean(){

  float intensity = 0;

  // eine fft über den gesamten song
  int n = signal_length_mono;
  fftw_complex    *data, *fft_result, *ifft_result;
  fftw_plan       plan_forward, plan_backward;

  data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );
  fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );
  ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * window_size );

  plan_forward  = fftw_plan_dft_1d(n, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

  for (int i = 0; i < n; i++){

    data[i][0] = wav_values_mono[i];
    data[i][1] = 0.0;

  }

  fftw_execute(plan_forward);

  // energiewerte aufsummieren und mitteln
  float sum = 0;
  for (int i = 0; i < n/2; i++) {
    sum = ((fft_result[i][0] * fft_result[i][0]) + (fft_result[i][1] * fft_result[i][1]))/sqrtf((float)2);
  }
  sum = sum/n;
  // alles wieder freigeben
  fftw_destroy_plan( plan_forward );
  fftw_free( data );
  fftw_free( fft_result );
  fftw_free( ifft_result );
  return intensity;
};

std::vector<time_value_int> Analysis::peaks_per_band(int f1, int f2){

  std::vector<time_value_int> data = std::vector<time_value_int>();

  float f1_sample = ((float) window_size / samplerate) * f1;
  float f2_sample = ((float) window_size / samplerate) * f2;
  float freq_step = (float) samplerate / window_size;

  for (int i = 0; i < normalized_result.size(); i++){
    int this_value = 0;
    int this_sample = 0;
    int this_block = 0;
    float this_freq = 0;
    for (int n = f1_sample; n <= f2_sample ; n++){

      if (normalized_result[i][n] >= this_value) {
        this_sample = n;
        this_block = i;
        this_value = normalized_result[i][n];
      }

    }

    float sample_time = (this_block*hop_size + ((float)window_size/2)) / samplerate;
    time_value_int tvalue = {0.0, 0};
    tvalue.time = sample_time;
    tvalue.value = this_value;

    data.push_back(tvalue);
  }


  return data;
}

void Analysis::normalize(){

  for (int i = 0; i < result.size(); i++){
    for (int n = 0; n < window_size/2; n++){

      if (result[i][n] > 0){
        normalized_values[n] = (result[i][n])/(fft_max_value)*255;
      }
      else {
        normalized_values[n] = 0;
      }

    }
    normalized_result.push_back(normalized_values);
  }

  Logger::debug("normalized_result.size(): {}", normalized_result.size());

}

float Analysis::get_length_of_song() {
  float t = signal_length_mono/this->samplerate;
  //std::cout << t << std::endl;
  return t;
}

long long Analysis::factorial(long long n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void Analysis::binomial_weights(int ma_window_size){

  // precalculate weights for given window size
  //
  // (ma_window_size over current_position (here = i))
  //
  //  n over k = n! / ( k! * (n - k)! )


  long long current_weight = 0;
  long long fac_window_size = factorial(ma_window_size);

  for (int i = 0; i <= ma_window_size; i++){

    current_weight = fac_window_size / ( factorial(i) * factorial( ma_window_size - i) );

    ma_weights.push_back(current_weight);
  }
  Logger::debug("ma_weights.size(): {}", ma_weights.size());

}

float Analysis::average(std::vector<time_value_float> data){
  float mw = 0;
  for(int i = 0; i < data.size(); i++)
  {
    mw += data.at(i).value / data.size();
  }
  return mw;
}

std::vector<time_value_double> Analysis::moving_average(int left, int right, std::vector<time_value_double> data){
  int ma_window_size = left + right + 1;
  // calculate weights ONCE
  binomial_weights(ma_window_size);

  //Moving Average weighted and centered
  std::vector<time_value_double> data2;
  time_value_double temp = {0.0, 0.0};
  float zeit;

  long long d = pow(2,ma_window_size);
  int rounder = round(ma_window_size % 2);


  for (int i = 0 ; i <= data.size() - ma_window_size; i++)
  {
    double ds = 0;
    zeit = data.at(i).time;
    int z = i - left;
    if (z < 0){
      z = 0;
    }

    for (int a = 0; a < ma_window_size; a++) {
      ds += data.at(z + a).value  * ma_weights[a];
    }

    temp = {zeit, ds/d};
    data2.push_back(temp);
  }
  return data2;
}

std::vector<time_value_float> Analysis::moving_average_delta(std::vector<time_value_float> data, float average){
  std::vector<time_value_float> ma_result;
  for (int i = 0; i < data.size(); i++){
    ma_result.push_back({data.at(i).time, fabsf(data.at(i).value - average)});
  }
  return ma_result;
}

std::vector<time_value_double> Analysis::get_turningpoints(std::vector<time_value_double> data){
  std::vector<time_value_double> tp_result;
  for (int i = 1; i < data.size()-2; i++){
    double delta0 = fabs(data.at(i).value - data.at(i-1).value);
    double delta1 = fabs(data.at(i+1).value - data.at(i).value);
    double delta2 = fabs(data.at(i+2).value - data.at(i+1).value);
    if((delta1 > delta2 && delta1 > delta0 && delta1 > 0)){
      // CALCULATE VALUE
      tp_result.push_back({data.at(i).time, data.at(i).value});
    }
  }
  return tp_result;
}

complex_result Analysis::fft_impulse(std::vector<double> signal) {

  int signalLength = bpm_seconds_to_check*44100;

  //fftw_complex *data, *fft_result, *ifft_result;
  //fftw_plan plan_forward, plan_backward;
  int i = 0;

  //data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  //fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  //ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);

  struct complex_result result;
  result.real.reserve(signalLength);
  result.imag.reserve(signalLength);

  m_plan_forward = fftw_plan_dft_1d(signalLength, m_data, m_fft_result, FFTW_FORWARD, FFTW_ESTIMATE);


  // Process each chunk of the signal

  for (i = 0; i < signalLength; i++) {

    m_data[i][0] = signal[i];
    m_data[i][1] = signal[i];

  }
  // Perform the FFT on our chunk
  fftw_execute(m_plan_forward);

  for (i = 0; i < signalLength; i++) {
    //result.real.push_back(fft_result[i][0]);
    //result.imag.push_back(fft_result[i][1]);
    result.real.emplace_back(m_fft_result[i][0]);
    result.imag.emplace_back(m_fft_result[i][1]);
  }
  Logger::debug("result.real.size(): {}", result.real.size());
  Logger::debug("result.imag.size(): {}", result.imag.size());

  //fftw_destroy_plan(plan_forward);
  //fftw_free(data);
  //fftw_free(fft_result);
  //fftw_free(ifft_result);

  return result;
}

int Analysis::get_bpm() {

  if(signal_length_mono < 15 * samplerate)
    return -1;

  int signalLength = bpm_seconds_to_check*44100;

  fftw_complex *data, *fft_result, *ifft_result;
  fftw_plan plan_forward, plan_backward;

  data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);

  plan_forward = fftw_plan_dft_1d(signalLength, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

  std::vector<double> resultRE;
  std::vector<double> resultIM;
  resultRE.reserve(signalLength);
  resultIM.reserve(signalLength);
  std::vector<double> EBPMcs;
  std::vector<double> EBPMmax;

  for (int i = 0; i < signalLength; i++) {
    data[i][0] = wav_values_mono_snippet[i];
    data[i][1] = wav_values_mono_snippet[i];

  }

  fftw_execute(plan_forward);
  double re;
  double im;
  for (int i = 0; i < signalLength; i++) {
    re = fft_result[i][0];
    im = fft_result[i][1];
    resultRE.push_back(re);
    resultIM.push_back(im);
  }
  Logger::debug("resultRE.size(): {}", resultRE.size());
  Logger::debug("resultIM.size(): {}", resultIM.size());


  double Ti;
  int n;

  int baender = 16;
  int bpmSchritt = 1;
  int bpmObergrenze = 181;
  int bpmUntergrenze = 70;
  std::vector<double> impulse;
  impulse.reserve(signalLength);

  m_data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  m_fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);
  m_ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * signalLength);

  for (int BPM = bpmUntergrenze; BPM <= bpmObergrenze; BPM += bpmSchritt) {

    double start = 0;
    double end = 1;

    double scale = (end - start) / log(1.0 + baender);
    double lower_bound = start;

    Ti = (60.0 / BPM) * samplerate;
    n = 0;
    double impulsee;
    impulse.clear();
    for (int k = 0; k < signalLength; k += 1) {
      if (fmod(k, Ti) < 1) {
        impulsee = 1;
        n++;
      }
      else {
        impulsee = 0;
      }
      if (n >= 4){
        impulsee = 0;
      }

      impulse.push_back(impulsee);
    }

    complex_result impulseFFT = fft_impulse(impulse);

    for (int z = 0; z < 1; z++) { // 1 = nur erstes band | baender = alle baender


      double upper_bound = start + log(2 + z) * scale;
      double energyValue = 0;
      for (int i = (signalLength / 180) * lower_bound; i < (signalLength / 180) * upper_bound;
           i += 1) {   // Frequenzbereich ..signalLength:=44100 Hertz...beispiel 44100 / 8 := signalLength/2^8
        energyValue = (double) energyValue + sqrt(
            ((resultRE[i] * impulseFFT.real[i] - resultIM[i] * impulseFFT.imag[i])
                * (resultRE[i] * impulseFFT.real[i] - resultIM[i] * impulseFFT.imag[i]))
                + ((resultRE[i] * impulseFFT.imag[i] + resultIM[i] * impulseFFT.real[i])
                    * (resultRE[i] * impulseFFT.imag[i] + resultIM[i] * impulseFFT.real[i])));

      }
      lower_bound = upper_bound;
      EBPMcs.push_back(energyValue);


    }
  }

  fftw_destroy_plan(m_plan_forward);
  fftw_free(m_data);
  fftw_free(m_fft_result);
  fftw_free(m_ifft_result);

  Logger::debug("impulse.size(): {}", impulse.size());
  Logger::debug("EBPMcs.size(): {}", EBPMcs.size());

  int finalBPM = 0;
  for (int z = 0; z < 1; z++) { // 1 = nur erstes band | baender = alle baender
    double maxWert = 0;
    for (int i = 0; i < bpmObergrenze - bpmUntergrenze + 1; i++) {

      if (EBPMcs[i * 1 + z] > maxWert) { // 1 = nur erstes band | baender = alle baender
        maxWert = EBPMcs[i * 1 + z];
        if (z == 0){
          finalBPM = (i * bpmSchritt) + bpmUntergrenze;
        }
      }

    }

  }

  fftw_destroy_plan(plan_forward);
  fftw_free(data);
  fftw_free(fft_result);
  fftw_free(ifft_result);

  //std::cout << "BPM: " << finalBPM << std::endl;

  return finalBPM;
}

double Analysis::get_first_beat(){

  int windowSize = 1024;
  int hopSize = 1024;
  int bands = 1;
  int samplesPerBand = (1024/2)/bands;
  int signalLength = signal_length_mono;

  fftw_complex    *data, *fft_result, *ifft_result;
  fftw_plan       plan_forward, plan_backward;

  data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
  fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
  ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );

  plan_forward  = fftw_plan_dft_1d( windowSize, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

  // Create a hamming window of appropriate length

  int chunkPosition = 0;
  double maxResult=0;

  int readIndex;

  // Should we stop reading in chunks?
  int bStop = 0;
  int numChunks = 0;

  std::vector<double> energySubband = std::vector<double>((signalLength/windowSize +1)*bands);
  std::vector<double> energySubbandAvg = std::vector<double>((signalLength/windowSize +1)*bands);
  std::vector<double> first_beat_result = std::vector<double>(windowSize);
  std::vector<double> time;


  // Process each chunk of the signal
  while(chunkPosition < signalLength && !bStop) {

    // Copy the chunk into our buffer
    for(int i = 0; i < windowSize; i++) {

      readIndex = chunkPosition + i;

      if(readIndex < signalLength) {
        // Note the windowing!
        data[i][0] = wav_values_mono[readIndex];
        data[i][1] = 0.0;

      } else {
        // we have read beyond the signal, so zero-pad it!
        data[i][0] = 0.0;
        data[i][1] = 0.0;
        bStop = 1;
      }



    }
    // Perform the FFT on our chunk
    fftw_execute( plan_forward );



    double scale = (1-0) / log(1.0+16);
    double upper_bound=0 + log(2+1)*scale;

    for (int i = 0; i < windowSize*upper_bound ; i++) {
      first_beat_result[i] = sqrt(fft_result[i][0]*fft_result[i][0]+fft_result[i][1]*fft_result[i][1]);
    }



    for (int i = 0; i < bands; i++){
      for(int j = 0; j < windowSize*upper_bound; j++)
      {
        energySubband[i+(numChunks*bands)] = energySubband[i+(numChunks*bands)] + (first_beat_result[j+(i*samplesPerBand)] * first_beat_result[j+(i*samplesPerBand)]);
      }
      energySubband[i+(numChunks*bands)] = energySubband[i+(numChunks*bands)] / samplesPerBand; 						//pow(energySubband[i+(numChunks*bands)],2);
      if(energySubband[i+(numChunks*bands)] > maxResult) maxResult = energySubband[i+(numChunks*bands)];
    }
    chunkPosition += hopSize;
    numChunks++;
  }


  for(int z=0; z < numChunks-43;z++){
    for(int i=0; i < 1; i++){ // welches Band
      for(int j=0; j<43;j++)
      {
        energySubbandAvg[i+z*bands] = energySubbandAvg[i+z*bands] + energySubband[i+(j*bands)+(z*bands)];
      }
      energySubbandAvg[i+z*bands] = energySubbandAvg[i+z*bands]/43;
      if(energySubband[i+z*bands] > 1.5f*energySubbandAvg[i+z*bands] && energySubband[i+z*bands]> 0.5f * maxResult ){
        time.push_back((windowSize*z)-512);
        z=z+5;
        break;
      }
    }
  }
  Logger::debug("time.size(): {}", time.size());

  double timeTemp = time[0];

  fftw_destroy_plan( plan_forward );
  fftw_free( data );
  fftw_free( fft_result );
  fftw_free( ifft_result );

  return timeTemp;
};

std::vector<double> Analysis::get_all_beats(int beats_per_minute, double firstBeat){


  std::vector<double> times;

  double timeTemp=firstBeat;
  //printf("Firstbeat %f\n",timeTemp);
  for(int i = 0 ; i < ((firstBeat*beats_per_minute) / (samplerate * 60))-1; i++){
    timeTemp=timeTemp-(60*samplerate/beats_per_minute);
    //printf("TimeTemp %f\n",timeTemp);
    times.insert(times.begin(),timeTemp);
  }

  timeTemp=firstBeat;
  times.push_back(firstBeat);
  for(int i = 0; i < ((((signal_length_mono-firstBeat)/samplerate)/60)*beats_per_minute)-1; i++){
    timeTemp = timeTemp+(60*samplerate/beats_per_minute);
    times.push_back(timeTemp);
    //fprintf(out,"Wert %f\n",times[i]);
  }
  Logger::debug("times.size(): {}", times.size());

  return times;
};


Analysis::Analysis() {

}
Analysis::~Analysis() {

}
void Analysis::set_resolution(int resolution) {
  if(resolution == 40)
    this->hop_size = 1102;
  else if(resolution == 20)
    this->hop_size = 2205;
  else if(resolution == 10)
    this->hop_size = 4410;
  else if(resolution == 5)
    this->hop_size = 8820;

  Logger::debug("Set resolution of Analysis to: {}, hop size now: {}", resolution, this->hop_size);
}

int Analysis::get_samplerate() {
  return this->samplerate;
}