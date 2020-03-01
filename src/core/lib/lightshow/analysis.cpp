//
// Created by Steven Drewers on 2019-05-05.
//

#include <logger.h>
#include "analysis.h"

void Analysis::read_wav(char *filepath) {

    wav_info.format = 0;

    wav_file = sf_open(filepath, SFM_READ, &wav_info);

    if (wav_file == NULL) {
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
    for (int i = 0; i < signal_length_multichannel; i += channels) {

        // add values for each channel and get arithmetic middle by (c1+...+cn)/n
        float combined_channel_value = 0;
        for (int n = 0; n < channels; n++) {
            combined_channel_value += wav_values_multichannel[i + n];
        }
        wav_values_mono[i / channels] += (combined_channel_value / channels);
        if (combined_channel_value / channels > max_sample_value && i / channels > signal_length_mono / 2 &&
            i / channels < signal_length_mono - (4 * 44100)) {
            max_sample_value = combined_channel_value / channels;
            max_sample = i / channels;
        }

    }

    //printf("time: %d, value: %f\n", max_sample/44100, max_sample_value);

    /** TODO:
     * HIER MÜSST MAN FÜNF MARKANTE SEKUNDEN AUS DEM SONG SUCHEN,
     * DIE HOHE ENERGIE IM TIEFEN FREQUENZBEREICH HABEN
     * MOMENTAN WIRD DIE HÄLFTE DES SONGS GENOMMEN, DAS GEHT IN ÜBER 90% DER FÄLLE KLAR
     * EVTL.
    */
    if (signal_length_mono > 15 * samplerate) {
        float wert = 0;
        //printf("from: %d, to: %d\n", (max_sample - (bpm_seconds_to_check*44100/2))/44100, (max_sample + (bpm_seconds_to_check*44100/2))/44100);
        for (int i = max_sample - (bpm_seconds_to_check * 44100 / 2);
             i < max_sample + (bpm_seconds_to_check * 44100 / 2); i++) {
            wert = fabsf((wav_values_mono[i + 1] - wav_values_mono[i]));
            wav_values_mono_snippet.push_back(wert);
            //printf("wert: %f", wert);
        }
    }

    free(wav_values_multichannel);

}

void Analysis::stft() {

    float normalization_factor = (2.0 * 1.63) / samplerate;

    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;

    data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);
    fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);
    ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);

    plan_forward = fftw_plan_dft_1d(window_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

    // construct window
    float *buffer;
    window = (float *) malloc(window_size * sizeof(float));
    for (int i = 0; i < window_size; i++) {
        window[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((window_size - 1) * 1.0))));
    }

    int chunk_position = 0;

    int read_index;

    // Should we stop reading in chunks?
    int b_stop = 0;
    int num_chunks = 0;

    // go for it
    while (chunk_position < signal_length_mono && !b_stop) {

        for (int i = 0; i < window_size; i++) {
            read_index = chunk_position + i;

            if (read_index < signal_length_mono) {
                data[i][0] = wav_values_mono[read_index] * window[i];
                data[i][1] = 0.0;
            } else {
                data[i][0] = 0.0;
                data[i][1] = 0.0;
                b_stop = 1;
            }
        }
        fftw_execute(plan_forward);
        std::vector<double> window_values = std::vector<double>(window_size / 2);
        for (int i = 0; i < window_values.size(); i++) {

            //window_values[i] = 10*log10f(((fft_result[i][0]*fft_result[i][0])+(fft_result[i][1]*fft_result[i][1]))*normalization_factor);
            window_values[i] = 10 *
                               log10f(((fft_result[i][0] * fft_result[i][0]) + (fft_result[i][1] * fft_result[i][1])) *
                                      ((fft_result[i][0] * fft_result[i][0]) + (fft_result[i][1] * fft_result[i][1])) *
                                      normalization_factor);


            if (window_values[i] > fft_max_value) {
                fft_max_value = window_values[i];
            }

        }
        //result.push_back(window_values);
        result.emplace_back(window_values);
        chunk_position += hop_size;
        num_chunks++;

    }

    Logger::debug("result.size(): {}", result.size());

    //std::cout << "result[0].size(): " << result[0].size() << std::endl;

    fftw_destroy_plan(plan_forward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);

}

std::vector<float> Analysis::get_onset_timestamps_energy_difference(float onset_value){

  std::vector<float> onset_timestamps;

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


  // this works good for hardstyle and some other stuff, semi good for the rest
  /*float f1_sample = ((float) window_size_onsets / 44100) * 1;
  float f2_sample = ((float) window_size_onsets / 44100) * 10000;
  std::cout << "normalized_result.size(): " << normalized_result.size() << std::endl;
  std::cout << "signal_length_mono: " << signal_length_mono << std::endl;
  for (int i = 0; i < normalized_result.size(); i++){
    int this_value = 0;
    int this_sample = 0;
    int this_block = 0;
    float this_freq = 0;
    for(int y = 0; y < window_size_onsets; y++)
      audioFrame[y] = 0;
    for (int n = f1_sample, k = 0; n <= f2_sample ; n++, k++){
      audioFrame[k] = (float) normalized_result[i][n];
    }
    gist2.processAudioFrame (audioFrame, window_size_onsets);
    float ed = gist2.energyDifference();
    float x = i;
    float time = x * window_size_onsets / 44100 / 1.85796852603;
    onsets.push_back({time, ed});
  }*/


  // ??
  /*int f1 = 100;
  int f2 = 200;
  int co_f1 = f1/(samplerate/window_size_onsets);
  int co_f2 = f2/(samplerate/window_size_onsets);
  for(int i = 0; i < co_f1; i++);
  for(int i = co_f2; i < window_size_onsets; i++);*/




  // works good for rocky stuff and clear electronic bass
  for(int i = 0; i < signal_length_mono - window_size_onsets; i = i + window_size_onsets / 2) {
    for(int j = i, k = 0; k < window_size_onsets; j++, k++) {
      audioFrame[k] = wav_values_mono[j];
      //std::cout << audioFrame[k] << std::endl;
    }


    gist2.processAudioFrame (audioFrame, window_size_onsets);
    float ed = gist2.energyDifference();
    float i_float = i;
    float time = i_float/44100;

    //if(ed > 0)
    //std::cout << "ed(" << time << "): " << ed << std::endl;

    onsets.push_back({time, ed});
  }

  // look for max peak
  float max_ed_value = 0.0;
  for(int i = 0; i < onsets.size(); i++) {
    if(onsets[i].value > max_ed_value)
      max_ed_value = onsets[i].value;
  }

  // look for all peaks, then take mean
  float all_peaks = 0.0;
  int peak_counter = 0;
  for(int i = 0; i < onsets.size(); i++) {
    if(i > 0 && i < onsets.size()-1) {
      if (onsets[i].value > onsets[i - 1].value && onsets[i].value > onsets[i + 1].value && onsets[i].value > max_ed_value * 0.25) {
        all_peaks += onsets[i].value;
        peak_counter++;
      }
    }
  }
  float mean_of_all_peaks = all_peaks / peak_counter;

  // OLD VALUES
  // rocky = 5.5
  // metal (HSB Voice of the Voiceless, double bass) = 1.7
  // metal/hard rock (Sabaton 7734) = 4.5
  // somewhat allgemeingültig = 5
  //min_value_onset = min_value_onset / onsets.size() * 5;

  // NEW VALUES for whole frequency onset detection
  // 9 average
  // 11:
    // Was Ich Liebe: okay, eher zu wenige
    // Madsen Keiner: zu viele
    // Callejon Ein Kompliment: zu viele
    // HSB Voice Of The Voiceless: gut, ein paar mehr wären okay
  // 12:
    // HSB Voice Of The Voiceless: gut!
    // Sabaton 7734: okay, ein paar zu viele
    // Powerwolf Armati Stigoi: gut, beim leisen Beginn ein paar zu viele
    // SOAD Prison Song: schlecht, viel zu viele
    // Architects Doomsday: sehr geil!
  // -> maybe 11.5-12.0 is fine for double bass songs?


  //min_value_onset = max_ed_value * 0.315;
  std::cout << "using onset_value: " << onset_value << std::endl;
  min_value_onset = (max_ed_value + mean_of_all_peaks + mean_of_all_peaks) / onset_value; // 7.5 zu wenig, 10 zu viel, 9 maybe
  std::cout << "mean_of_all_peaks: " << mean_of_all_peaks << std::endl;
  std::cout << "max_ed_value: " << max_ed_value << std::endl;
  std::cout << "min_value_onset: " << min_value_onset << std::endl;
  std::cout << "old min_value_onset: " << max_ed_value * 0.315 << std::endl;
  float threshold_reset = 0.0f;

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

    if(onset.value <= threshold_reset) {
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

std::vector<float> Analysis::get_onset_timestamps_frequencies(float f_start, float f_end) {

  // BAND PASS FREQUENCIES
  //float f_start = 0;
  //float f_end = 22050;
  //f_start = 0;
  //f_end = 22050;

  int window_size_onsets = 2048;
  Gist<float> gist2(window_size_onsets, 44100);
  float audioFrame[window_size_onsets];
  float window_function[window_size_onsets];
  int numSamplesMinus1 = window_size_onsets - 1;
  std::vector<float> onsets;
  std::vector<time_value_float> spectral_flux;
  std::vector<fluxes> spectral_fluxes;
  std::vector<fluxes> thresholds;
  std::vector<float> magnitude_spectrum;
  std::vector<float> last_magnitude_spectrum;
  std::vector<time_value_float> threshold_function_values;
  float max_ed, max_sd, max_sdhwr, max_csd, max_hfc, max_l2nh, max_l1nh, max_l2h, max_l1h = -9999999;


  // PREPARE CSV FILES WITH HEADERS FOR ALGORITHM COMPARISON
  /*FILE *fp_spectral_flux = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/spectral_flux.csv",
                                      "w");
  fprintf(fp_spectral_flux, "time, ed, sd, sdhwr, csd, hfc, l2nohwr, l1nohwr, l2hwr, l1hwr\n");

  FILE *fp_thresholds = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/thresholds.csv", "w");
  fprintf(fp_thresholds, "time, ed, sd, sdhwr, csd, hfc, l2nohwr, l1nohwr, l2hwr, l1hwr\n");

  FILE *fp_threshold_function = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/threshold.csv",
                                           "w");
  fprintf(fp_threshold_function, "time, value\n");

  FILE *fp_onsets = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/onsets.csv", "w");
  fprintf(fp_onsets, "time, dummyvalue\n");

  FILE *fp_average_flux = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/average_flux.csv",
                                     "w");
  fprintf(fp_average_flux, "time, average\n");

  FILE *fp_average_threshold = std::fopen(
      "/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/average_threshold.csv", "w");
  fprintf(fp_average_threshold, "time, average\n");*/

  // GENERATE HAMMING WINDOW FOR AUDIOFRAME (TIME-DOMAIN SMOOTHING)
  for (int i = 0; i < window_size_onsets; i++) {
    window_function[i] = (float) (0.5 *
        (float) (1.0 - cos(2.0 * (float) M_PI * ((float) i / (float) numSamplesMinus1))));
  }

  // LOOP OVER COMPLETE SIGNAL WITH WINDOW_SIZE_ONSETS AND A HOPSIZE OF 50%
  for (int i = 0; i < signal_length_mono - window_size_onsets; i = i + window_size_onsets / 2) {

    // PREPARE FRAME AND MULTIPLY WITH HAMMING WINDOW FOR ONSET DETECTION
    for (int j = i, k = 0; k < window_size_onsets; j++, k++) {
      audioFrame[k] = wav_values_mono[j] * window_function[k];
    }
    // GIST PROCESS FOR FFT, MAGNITUDE SPECTRUM ETC. - THIS IS NECESSARY
    gist2.processAudioFrame(audioFrame, window_size_onsets);

    // GET GIST MAGNITUDE SPECTRUM (SEEMS TO BE DFT COEFFICIENTS RMS)
    // ALSO DELETE COEFFICIENTS FOR FREQUENCIES OUT OF RANGE OF [F_START, F_END]
    magnitude_spectrum = gist2.getMagnitudeSpectrum();

    for (int k = 0; k < magnitude_spectrum.size(); k++) {
      float f_current = (44100 / window_size_onsets) * k;
      if (f_current <= f_start || f_current >= f_end) {
        magnitude_spectrum[k] = 0.0;
      }
    }

    // HACK TO ALSO SAVE LAST MAGNITUDE SPECTRUM TEMPORARY, WE NEED THIS TO GET THE DELTA FOR SPECTRAL FLUX
    if (i == 0) {
      for (int k = 0; k <= magnitude_spectrum.size(); k++) {
        last_magnitude_spectrum.push_back(magnitude_spectrum[k]);
      }
    }

    // CALCULATE SPECTRAL FLUXES INCLUDED BY GIST
    float ed = gist2.energyDifference();
    float sd = gist2.spectralDifference();
    float sdhwr = gist2.spectralDifferenceHWR();
    float csd = gist2.complexSpectralDifference();
    float hfc = gist2.highFrequencyContent();

    float i_float = i;
    float time = i_float / 44100;

    // CALCULATE SPECTRAL FLUXES INCLUDED BY ESSENTIA
    float flux_l2_no_hwr = 0.0;
    float flux_l1_no_hwr = 0.0;
    float flux_l2_hwr = 0.0;
    float flux_l1_hwr = 0.0;

    for (int m = 0; m < magnitude_spectrum.size(); m++) {

      float flux_value_l2_no_hwr = (magnitude_spectrum[m] - last_magnitude_spectrum[m]) *
          (magnitude_spectrum[m] - last_magnitude_spectrum[m]);
      flux_l2_no_hwr += flux_value_l2_no_hwr * flux_value_l2_no_hwr;

      float flux_value_l1_no_hwr = abs((magnitude_spectrum[m] - last_magnitude_spectrum[m]));
      flux_l1_no_hwr += flux_value_l1_no_hwr;

      float flux_value_l2_hwr = (magnitude_spectrum[m] - last_magnitude_spectrum[m]) *
          (magnitude_spectrum[m] - last_magnitude_spectrum[m]);
      flux_l2_hwr += flux_value_l2_hwr < 0 ? 0 : flux_value_l2_hwr * flux_value_l2_hwr;

      float flux_value_l1_hwr = (magnitude_spectrum[m] - last_magnitude_spectrum[m]);
      //std::cout << flux_value_l1_hwr << std::endl;
      flux_l1_hwr += flux_value_l1_hwr < 0 ? 0 : flux_value_l1_hwr;

    }
    flux_l2_no_hwr = sqrt(flux_l2_no_hwr);
    flux_l2_hwr = sqrt(flux_l2_hwr);



    // CHOOSE A SPECTRAL FLUX TO USE FOR ONSET DETECTION (SEE LINE 251 FOR THEIR NAMES)
    spectral_flux.push_back({time, csd});
    if (ed > max_ed) {
      max_ed = ed;
    }
    if (sd > max_sd) {
      max_sd = sd;
    }
    if (sdhwr > max_sdhwr) {
      max_sdhwr = sdhwr;
    }
    if (csd > max_csd) {
      max_csd = csd;
    }
    if (hfc > max_hfc) {
      max_hfc = hfc;
    }
    if (flux_l2_no_hwr > max_l2nh) {
      max_l2nh = flux_l2_no_hwr;
    }
    if (flux_l1_no_hwr > max_l1nh) {
      max_l1nh = flux_l1_no_hwr;
    }
    if (flux_l2_hwr > max_l2h) {
      max_l2h = flux_l2_hwr;
    }
    if (flux_l1_hwr > max_l1h) {
      max_l1h = flux_l1_hwr;
    }
    spectral_fluxes.push_back(
        {time, ed, sd, sdhwr, csd, hfc, flux_l2_no_hwr, flux_l1_no_hwr, flux_l2_hwr, flux_l1_hwr}
    );

    // SAVE CURRENT SPECTRAL FLUX AS LAST TEMPORAL FLUX FOR COMPARISON
    last_magnitude_spectrum.clear();

    // COPY THIS MAGNITUDE_SPECTRUM AS OLD MAGNITUDE SPECTRUM
    for (int k = 0; k < magnitude_spectrum.size(); k++) {
      last_magnitude_spectrum.push_back(magnitude_spectrum[k]);
    }
  }

  for (int j = 0; j < spectral_fluxes.size(); j++) {
    spectral_fluxes[j].ed /= max_ed;
    spectral_fluxes[j].sd /= max_sd;
    spectral_fluxes[j].sdhwr /= max_sdhwr;
    spectral_fluxes[j].csd /= max_csd;
    spectral_fluxes[j].hfc /= max_hfc;
    spectral_fluxes[j].l2nh /= max_l2nh;
    spectral_fluxes[j].l1nh /= max_l1nh;
    spectral_fluxes[j].l2h /= max_l2h;
    spectral_fluxes[j].l1h /= max_l1h;
    //fprintf(fp_spectral_flux, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", spectral_fluxes[j].time, spectral_fluxes[j].ed, spectral_fluxes[j].sd, spectral_fluxes[j].sdhwr, spectral_fluxes[j].csd, spectral_fluxes[j].hfc,
      //      spectral_fluxes[j].l2nh, spectral_fluxes[j].l1nh, spectral_fluxes[j].l2h, spectral_fluxes[j].l1h);
  }

  //fclose(fp_spectral_flux);

  // PRINT ALL THE SPECTRAL FLUXES TO A CSV FILE FOR COMPARISON


  // GENERATE MOVING AVERAGE OF THE CHOSEN SPECTRAL FLUX - THIS IS USED FOR THRESHOLDING AKA FINDING REAL ONSETS!

  // CALCULATE HOW MANY FLUXES WILL BE USED FOR MOVING AVERAGE (LEFT/RIGHT)
  int fluxes = 0.5 / (44100 / window_size_onsets * 0.001);

  // SET A MULTIPLIER TO MOVE UP THE THRESHOLD FUNCTION VALUES AND FIND REAL ONSETS
  // THIS VALUE CAN BE TWEAKED!!! (1.5 IS THE VALUE USED IN THE ONSET DETECTION TUTORIAL)
  // ANYTHING BETWEEN ~1.3 AND ~1.9 MIGHT YIELD GOOD RESULTS
  float multiplier = 1;

  // ZERO PADDING FOR MOVING AVERAGE

  float delta = 0;
  for (int f = 0; f < spectral_flux.size(); f++) {
    delta += (spectral_flux[f].value / spectral_flux.size());
  }
  // SIMPLE MOVING AVERAGE LOOP OVER ALL SPECTRAL FLUX VALUES
  for (int i = 0; i < spectral_flux.size() - (fluxes/2); i++) {
    int start = fmax(0, i - (fluxes / 2) + 1);
    int end = fmin(spectral_flux.size() - 1, i + (fluxes / 2) + 1);
    float mean = delta * multiplier;
    float mean_current = 0;

    for (int j = start; j <= end; j++) {
      mean_current = spectral_flux[j].value / (end - start);
      mean = mean + mean_current;
    }
    //mean /= (float)(end - start);

    float this_time = spectral_flux[i + (fluxes / 2)].time;
    //mean *= multiplier;
    threshold_function_values.push_back({this_time, mean});
    //fprintf(fp_threshold_function, "%f, %f\n", threshold_function_values[i].time,
      //      threshold_function_values[i].value);
  }
  //fclose(fp_threshold_function);

  // GRAPH STUFF //
  // GO THROUGH ALL FLUXES AND GENERATE ALL THRESHOLDS FOR GRAPH COMPARISON

  float multiplier_ed = 1;
  float multiplier_sd = 1;
  float multiplier_sdhwr = 1;
  float multiplier_hfc = 1;
  float multiplier_csd = 1;
  float multiplier_l2nh = 1;
  float multiplier_l1nh = 1;
  float multiplier_l2h = 1;
  float multiplier_l1h = 1;

  float delta_ed = 0;
  float delta_sd = 0;
  float delta_sdhwr = 0;
  float delta_csd = 0;
  float delta_hfc = 0;
  float delta_l2nh = 0;
  float delta_l1nh = 0;
  float delta_l2h = 0;
  float delta_l1h = 0;

  for (int g = 0; g < spectral_fluxes.size(); g++) {
    delta_ed += (spectral_fluxes[g].ed / spectral_fluxes.size());
    delta_sd += (spectral_fluxes[g].sd / spectral_fluxes.size());
    delta_sdhwr += (spectral_fluxes[g].sdhwr / spectral_fluxes.size());
    delta_csd += (spectral_fluxes[g].csd / spectral_fluxes.size());
    delta_hfc += (spectral_fluxes[g].hfc / spectral_fluxes.size());
    delta_l2nh += (spectral_fluxes[g].l2nh / spectral_fluxes.size());
    delta_l1nh += (spectral_fluxes[g].l1nh / spectral_fluxes.size());
    delta_l2h += (spectral_fluxes[g].l2h / spectral_fluxes.size());
    delta_l1h += (spectral_fluxes[g].l1h / spectral_fluxes.size());
  }
  for (int i = 0; i < spectral_fluxes.size() - (fluxes/2); i++) {
    int start = fmax(0, i - (fluxes / 2) + 1);
    int end = fmin(spectral_fluxes.size() - 1, i + (fluxes / 2) + 1);
    float mean_ed = delta_ed * multiplier_ed;
    float mean_sd = delta_sd * multiplier_sd;
    float mean_sdhwr = delta_sdhwr * multiplier_sdhwr;
    float mean_csd = delta_csd * multiplier_csd;
    float mean_hfc = delta_hfc * multiplier_hfc;
    float mean_l2nh = delta_l2nh * multiplier_l2nh;
    float mean_l1nh = delta_l1nh * multiplier_l1nh;
    float mean_l2h = delta_l2h * multiplier_l2h;
    float mean_l1h = delta_l1h * multiplier_l1h;

    float mean_current_ed = 0;
    float mean_current_sd = 0;
    float mean_current_sdhwr = 0;
    float mean_current_csd = 0;
    float mean_current_hfc = 0;
    float mean_current_l2nh = 0;
    float mean_current_l1nh = 0;
    float mean_current_l2h = 0;
    float mean_current_l1h = 0;


    for (int j = start; j <= end; j++) {
      mean_current_ed = spectral_fluxes[j].ed / (end - start);
      mean_current_sd = spectral_fluxes[j].sd / (end - start);
      mean_current_sdhwr = spectral_fluxes[j].sdhwr / (end - start);
      mean_current_csd = spectral_fluxes[j].csd / (end - start);
      mean_current_hfc = spectral_fluxes[j].hfc / (end - start);
      mean_current_l2nh = spectral_fluxes[j].l2nh / (end - start);
      mean_current_l1nh = spectral_fluxes[j].l1nh / (end - start);
      mean_current_l2h = spectral_fluxes[j].l2h / (end - start);
      mean_current_l1h = spectral_fluxes[j].l1h / (end - start);
      mean_ed = mean_ed + mean_current_ed;
      mean_sd = mean_sd + mean_current_sd;
      mean_sdhwr = mean_sdhwr + mean_current_sdhwr;
      mean_csd = mean_csd + mean_current_csd;
      mean_hfc = mean_hfc + mean_current_hfc;
      mean_l2nh = mean_l2nh + mean_current_l2nh;
      mean_l1nh = mean_l1nh + mean_current_l1nh;
      mean_l2h = mean_l2h + mean_current_l2h;
      mean_l1h = mean_l1h + mean_current_l1h;
    }
    //mean /= (float)(end - start);

    float this_time = spectral_fluxes[i + (fluxes / 2)].time;
    //mean *= multiplier;
    thresholds.push_back(
        {this_time, mean_ed, mean_sd, mean_sdhwr, mean_csd, mean_hfc, mean_l2nh, mean_l1nh, mean_l2h,
         mean_l1h});
    //fprintf(fp_thresholds, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", thresholds[i].time, thresholds[i].ed,
      //      thresholds[i].sd, thresholds[i].sdhwr, thresholds[i].csd, thresholds[i].hfc, thresholds[i].l2nh,
        //    thresholds[i].l1nh, thresholds[i].l2h, thresholds[i].l1h);

  }
  //fclose(fp_thresholds);


  // DAS ULTIMATIVE MITTEL AUS NEUN ONSET DETECTION ALGORITHMS MIT GEWICHTUNGSMÖGLICHKEIT ZUM TUNEN

  float w1 = 4.0 / 9.0; // energy difference
  float w2 = 0.0 / 9.0; // spectral difference
  float w3 = 1.0 / 9.0; // spectral difference hwr
  float w4 = 0.0 / 9.0; // complex spectral difference
  float w5 = 0.0 / 9.0; // high frequency content
  float w6 = 1.0 / 9.0; // l2 no hwr
  float w7 = 0.0 / 9.0; // l1 no hwr
  float w8 = 2.0 / 9.0; // l2 hwr
  float w9 = 1.0 / 9.0; // l1 hwr

  std::vector<time_value_float> average_flux, average_threshold;

  for (int i = 0; i < spectral_fluxes.size() - (fluxes/2); i++) {
    average_flux.push_back({spectral_fluxes[i].time, (
        (spectral_fluxes[i].ed * w1) +
            (spectral_fluxes[i].sd * w2) +
            (spectral_fluxes[i].sdhwr * w3) +
            (spectral_fluxes[i].csd * w4) +
            (spectral_fluxes[i].hfc * w5) +
            (spectral_fluxes[i].l2nh * w6) +
            (spectral_fluxes[i].l1nh * w7) +
            (spectral_fluxes[i].l2h * w8) +
            (spectral_fluxes[i].l1h * w9)
    )});
    //fprintf(fp_average_flux, "%f, %f\n", average_flux[i].time, average_flux[i].value);
    float current_threshold = (
        (thresholds[i].ed * w1) +
            (thresholds[i].sd * w2) +
            (thresholds[i].sdhwr * w3) +
            (thresholds[i].csd * w4) +
            (thresholds[i].hfc * w5) +
            (thresholds[i].l2nh * w6) +
            (thresholds[i].l1nh * w7) +
            (thresholds[i].l2h * w8) +
            (thresholds[i].l1h * w9)
    ) * 0.5;
    average_threshold.push_back({thresholds[i].time, current_threshold});

    //fprintf(fp_average_threshold, "%f, %f\n", average_threshold[i].time, average_threshold[i].value);

  }




  // ADD FLUX TIMES TO ONSETS IF THEYRE ABOVE THRESHOLD AND IF THEY'RE A PEAK (GREATER THAN LAST AND NEXT VALUE)
  /*
  for (int i = 1; i < spectral_flux.size() - 2; i++) {
      if (spectral_flux[i].value > threshold_function_values[i].value &&
          spectral_flux[i].value > spectral_flux[i + 1].value &&
          spectral_flux[i].value > spectral_flux[i - 1].value) {
          onsets.push_back({spectral_flux[i].time});
          fprintf(fp_onsets, "%f, %d\n", spectral_flux[i].time, 1);
      }
  }
  fclose(fp_onsets);
   */

  // ADD FLUX TIMES TO ONSETS IF THEYRE ABOVE THRESHOLD AND IF THEY'RE A PEAK (GREATER THAN LAST AND NEXT VALUE) AND IF THERE HAS NOT BEEN ANOTHER ONSET IN THE LAST 20 MILLISECONDS
  for (int i = 1; i < spectral_flux.size() - 2; i++) {
    if (average_flux[i].value > average_threshold[i].value &&
        average_flux[i].value > average_flux[i + 1].value &&
        average_flux[i].value > average_flux[i - 1].value) {

      if (!onsets.empty() && (average_flux[i].time - onsets.back() >= 0.025)){
        onsets.push_back({average_flux[i].time});
        //fprintf(fp_onsets, "%f, %d\n", average_flux[i].time, 1);
      } else if (onsets.empty()){
        onsets.push_back({average_flux[i].time});
        //fprintf(fp_onsets, "%f, %d\n", average_flux[i].time, 1);
      } else {

      }


    }
  }
  //fclose(fp_onsets);

  return onsets;

}



std::vector<time_value_double> Analysis::get_intensity_function_values() {

    int block_size = 44100;
    int segment_duration = 60 / 169 * 16;
    int jump_size = block_size / 3;

    std::vector<time_value_double> intensities = std::vector<time_value_double>();
    float average_window_value = 0;
    float normalization_factor = (2.0 * 1.63) / samplerate;

    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;

    data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);
    fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);
    ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);

    plan_forward = fftw_plan_dft_1d(block_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

    // construct window
    float *buffer;
    window = (float *) malloc(block_size * sizeof(float));
    for (int i = 0; i < block_size; i++) {
        window[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((block_size - 1) * 1.0))));
    }

    int chunk_position = 0;

    int read_index;

    // Should we stop reading in chunks?
    int b_stop = 0;
    int num_chunks = 0;



    // go for it
    while (chunk_position < signal_length_mono && !b_stop) {

        for (int i = 0; i < block_size; i++) {
            read_index = chunk_position + i;

            if (read_index < signal_length_mono) {
                data[i][0] = wav_values_mono[read_index] * window[i];
                data[i][1] = 0.0;
            } else {
                data[i][0] = 0.0;
                data[i][1] = 0.0;
                b_stop = 1;
            }
        }
        fftw_execute(plan_forward);
        std::vector<double> window_values = std::vector<double>(block_size / 2);
        for (int i = 0; i < window_values.size(); i++) {

            window_values[i] = (
                    (
                            (
                                    (fft_result[i][0] * fft_result[i][0])
                                    +
                                    (fft_result[i][1] * fft_result[i][1])
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

            if (f < 1) {
                weight = 0;
            } else if (f >= 1 && f <= 150) {
                weight = 8;
            } else if (f > 150 && f <= 2000) {
                weight = 16;
            } else if (f > 2000 && f <= 6000) {
                weight = 8;
            } else if (f > 6000 && f <= 16000) {
                weight = 0;
            } else if (f > 16000) {
                weight = 0;
            }

            average_window_value = average_window_value + (window_values[i] * weight);


        }

        average_window_value /= ((float) block_size / 2);
        time_value_double current = {0.0, 0.0};
        current.time = ((num_chunks * jump_size) + (block_size / 2.0)) / samplerate;
        current.value = average_window_value;

        intensities.push_back(current);
        chunk_position += jump_size;
        num_chunks++;

    }

    fftw_destroy_plan(plan_forward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);

    return intensities;

}

std::vector<time_value_float> Analysis::get_segments() {

  bool FILEPRINT = false;
  auto start = std::chrono::system_clock::now();

  std::vector<time_value_float> segments;

  // BIN AND HOPSIZE BASED ON MEINARD MÜLLERS RECOMMENDATION IN "FUNDAMENTALS OF USIC PROCESSING"
  int bin_size = 2205; // 0.05 seconds, 20Hz
  int hop_size = 2205; // no overlap

  Gist<float> gist2(bin_size, samplerate);
  std::vector<std::vector<float>> window;
  std::vector<std::vector<float>> ssm;
  std::vector<float> bin;
  bin.reserve(bin_size);
  std::vector<float> coeffs;

  // filter kernel variables
  int L = 75;
  int N = (L*2)+1;

  // ##############################
  // ###### 1. GET AUDIO BIN ######
  // ##############################

  auto start_audiobin = std::chrono::system_clock::now();

  for (int i = 0; i < (signal_length_mono - bin_size); i += hop_size){

    for(int j = 0; j < bin_size; j++){
      int index = i + j;
      //std::cout << "index " << index << ": " << wav_values_mono[index] << std::endl;
      bin.push_back(wav_values_mono[index]);
    }
    gist2.processAudioFrame(bin);

    // ########################################
    // ###### 2. GET THE FEATURE VECTORS ######
    // ###### AND PUT THEM INTO WINDOW ########
    // ########################################

    // USE MAGNITUDE SPECTRUM
    //coeffs = gist2.getMagnitudeSpectrum();

    // USE MFCC
    coeffs = gist2.getMelFrequencyCepstralCoefficients();

    window.push_back(coeffs);
    coeffs.clear();
    bin.clear();
  }

  auto end_audiobin = std::chrono::system_clock::now();


  // MAYBE WE WILL NEED THIS LATER
  // HOW TO GET THE TIMES
  // time of window[n] = (((n * hop_size) + (bin_size / 2)) / sample_rate)

  // #################################
  // ###### 3. COMPUTE DISTANCE ######
  // #################################

  // THIS HAS TO BE OPTIMIZED!!!
  // ONE HALF OF MATRIX DOESNT NEED TO BE CALCULATED BECAUSE [m][n] = [n][m]
  // EVERYTHING ABOVE OR BELOW L DOESN'T NEED TO BE CALCULATED
  // ZERO PADDING MUST BE APPLIED!

  std::vector<float> similarity;

  float ssm_max = 0;
  float ssm_min = 0;
  float ssm_count = 0;
  float ssm_middle = 0;
  float ssm_sum = 0;

  float distance = 0;

  auto start_distance = std::chrono::system_clock::now();

  for (int m = 0; m < window.size(); m++){
    similarity.clear();
    for (int n = 0; n < window.size(); n++){

      if (n <= m+L && n >= m-L) {

        // BEFORE CALCULATING DISTANCE, CHECK IF THERE IS A MIRRORED VALUE BECAUSE [m][n] = [n][m]
        /*
        if (ssm[n].size() > m && ssm[m].size() > n) {
            ssm[m][n] = ssm[n][m];
            continue;
        }
        else {
        }
         */

        // #################################################################
        // ###################### 3.1 COSINE DISTANCE ######################
        // #### d_cos  =  m*n / |m|*|n|  =  m*n / sqrt(m*m) * sqrt(n*n) ####
        // #################################################################

        float p_mn = 0;
        for (int j = 0; j < window[m].size(); j++) {
          p_mn += window[m][j] * window[n][j];
        }
        float p_mm = 0;
        for (int j = 0; j < window[m].size(); j++) {
          p_mm += window[m][j] * window[m][j];
        }
        float p_nn = 0;
        for (int j = 0; j < window[n].size(); j++) {
          p_nn += window[n][j] * window[n][j];
        }
        float cosine_distance = p_mn / (sqrt(p_mm) * sqrt(p_nn));


        // USE COSINE DISTANCE
        distance = cosine_distance;
      }
      else {
        distance = 0.0;
      }

      // USE OTHER DISTANCE
      //float distance = other_distance;

      similarity.push_back(distance);

      // SUM UP DISTANCES
      ssm_sum += distance;

      // COUNT DISTANCES
      ssm_count += 1;

      // GET MIN AND MAX DISTANCE
      if (distance < ssm_min){
        ssm_min = distance;
      }
      else if (distance > ssm_max){
        ssm_max = distance;
      }
      else {}

    }

    // GET MIDDLE OF DISTANCES
    ssm_middle = ssm_sum / ssm_count;

    // ####################
    // ## 4. MAKE MATRIX ##
    // ####################

    ssm.push_back(similarity);


  }


  auto end_distance = std::chrono::system_clock::now();


  // PRINT MIN, MAX AND MIDDLE
  std::cout << "SSM_MIN: " << ssm_min << " SSM_MAX: " << ssm_max << std::endl;
  std::cout << "SSM_MIDDLE: " << ssm_middle <<  std::endl;

  // ###############################################
  // ## 4.1 PUT MATRIX IN A CSV FILE FOR PLOTTING ##
  // ###############################################
  auto start_file_ssm = std::chrono::system_clock::now();

  if (FILEPRINT == true) {
    FILE *fp_ssm = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm.csv", "w");

    for (int m = 0; m < ssm.size(); m++) {
      for (int n = 0; n < ssm.size(); n++) {
        fprintf(fp_ssm, "%f,", ssm[m][n]);
      }
      fprintf(fp_ssm, "\n");
    }

    fclose(fp_ssm);

  }
  auto end_file_ssm = std::chrono::system_clock::now();



  // ###################################
  // ## 5. NOVELTY BASED SEGMENTATION ##
  // ###################################

  // ####################################################
  // ## 5.1 CREATE RADIAL GAUSSIAN CHECKERBOARD KERNEL ##
  // ####################################################

  std::vector<std::vector<float>> cb_kernel;
  std::vector<float> cb_kernel_line;
  float cb_value;
  std::vector<std::vector<float>> gaussian_kernel;
  std::vector<float> gaussian_kernel_line;
  float gaussian_value;
  float epsilon = 0.5;
  std::vector<std::vector<float>> kernel;
  std::vector<float> kernel_line;
  float value;

  auto start_kernel = std::chrono::system_clock::now();

  // CREATE CHECKERBOARD KERNEL
  for (int m = 1; m <= N; m++){
    for (int n = 1; n <= N; n++){
      if ((m <= L && n <= L) || (m >= (L+2) && n >= (L+2))){
        cb_value = 1;
      }
      else if (m == (L+1) || n == (L+1)) {
        cb_value = 0;
      }
      else {
        cb_value = -1;
      }
      cb_kernel_line.push_back(cb_value);
    }
    cb_kernel.push_back(cb_kernel_line);
    cb_kernel_line.clear();
  }

  if (FILEPRINT == true) {
    FILE *fp_cb_kernel = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/cb_kernel.csv", "w");

    for (int m = 0; m < cb_kernel.size(); m++) {
      for (int n = 0; n < cb_kernel.size(); n++) {
        fprintf(fp_cb_kernel, "%f,", cb_kernel[m][n]);
      }
      fprintf(fp_cb_kernel, "\n");
    }


    fclose(fp_cb_kernel);
  }

  // CREATE RADIAL GAUSSIAN KERNEL
  float increment = 2.0/N;
  for (float s = (-1.0 + (increment/2)); s < 1.0; s+=increment){
    for (float t = (-1.0 + (increment/2)); t < 1.0; t+=increment){
      gaussian_value = exp(-1 * pow(epsilon, 2) * ( pow(s, 2) + pow(t, 2) ));
      gaussian_kernel_line.push_back(gaussian_value);
    }
    gaussian_kernel.push_back(gaussian_kernel_line);
    gaussian_kernel_line.clear();
  }
  if (FILEPRINT == true) {
    FILE *fp_gaussian_kernel = std::fopen(
        "/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/gaussian_kernel.csv", "w");

    for (int m = 0; m < gaussian_kernel.size(); m++) {
      for (int n = 0; n < gaussian_kernel.size(); n++) {
        fprintf(fp_gaussian_kernel, "%f,", gaussian_kernel[m][n]);
      }
      fprintf(fp_gaussian_kernel, "\n");
    }

    fclose(fp_gaussian_kernel);
  }

  // ADD THEM UP ELEMENTWISE!
  for (int m = 0; m < N; m++){
    for (int n = 0; n < N; n++){
      value = cb_kernel[m][n] * gaussian_kernel[m][n];
      kernel_line.push_back(value);
    }
    kernel.push_back(kernel_line);
    kernel_line.clear();
  }
  if (FILEPRINT == true) {
    FILE *fp_kernel = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/kernel.csv", "w");

    for (int m = 0; m < kernel.size(); m++) {
      for (int n = 0; n < kernel.size(); n++) {
        fprintf(fp_kernel, "%f,", kernel[m][n]);
      }
      fprintf(fp_kernel, "\n");
    }

    fclose(fp_kernel);
  }

  auto end_kernel = std::chrono::system_clock::now();

  // 1  1  0 -1 -1
  // 1  1  0 -1 -1
  // 0  0  0  0  0
  // -1 -1 0  1  1
  // -1 -1 0  1  1





  // ####################################
  // ## 5.1 KERNEL MIT SSM VERHEIRATEN ##
  // ####################################

  // LAUT BUCH: DELTA_KERNEL(n) = SUM(K(k,l)*S(n+k,n+l)
  // LAUT BUCH MÜSSTE DAS ALLES IRGENDWIE SO GEHEN...
  std::vector<time_value_float> novelty_function;
  float novelty_value = 0;

  auto start_filter = std::chrono::system_clock::now();


  for (int n = 0; n < (ssm.size()); n++){
    novelty_value = 0;
    // [n,n] müsste der punkt sein, um den herum wir uns alles ansehen.... also auf der diagonalen liegen
    for (int k = 0; k < N; k++){
      // dann über k....
      for (int l = 0; l < N; l++){
        // dann checken wie groß n gerade so ist, evtl. müssen wir gar nicht rechnen, weil zero padding

        if ((n-L) < 0 || ((n-L)+k) >= ssm.size() || ((n-L)+l) >= ssm.size()){
          novelty_value += 0;
        } else {
          float k_val = kernel[k][l];
          float ssm_val = ssm[(n-L)+k][(n-L)+l];
          novelty_value += (k_val * ssm_val);
        }

      }
    }
    float novelty_time = n*0.05;
    novelty_function.push_back({novelty_time, novelty_value});
  }

  auto end_filter = std::chrono::system_clock::now();

  auto start_file_novelty = std::chrono::system_clock::now();
  auto end_file_novelty = std::chrono::system_clock::now();

  float novelty_middle = 0;
  for (int i = 0; i < novelty_function.size(); i++){
    novelty_middle += novelty_function[i].value;
  }
  novelty_middle /= novelty_function.size();
  novelty_middle *= 1.1;
  std::cout << "NOVELTY MIDDLE: " << novelty_middle << std::endl;

  std::cout << "--- SEGMENT CHANGES ---" << std::endl;
  for (int i = 1; i < novelty_function.size()-1; i++){
    if (novelty_function[i].value >= novelty_middle
        && novelty_function[i-1].value < novelty_function[i].value
        && novelty_function[i+1].value < novelty_function[i].value ){
      segments.push_back({novelty_function[i].time, novelty_function[i].value});
      std::cout << "time: " << novelty_function[i].time << std::endl;
    }
  }

  bool multiple_peaks = false;
  std::vector<int> indexes_of_multiple_peaks;
  if(!segments.empty()) {
    for (int i = 0; i < segments.size() - 1; i++) {

      if ((abs(segments[i + 1].time - segments[i].time)) < 2.0 && multiple_peaks) {
        indexes_of_multiple_peaks.push_back(i);
      }

      if ((abs(segments[i + 1].time - segments[i].time)) < 2.0 && !multiple_peaks) {
        multiple_peaks = true;
        indexes_of_multiple_peaks.push_back(i);
      }

      if (((abs(segments[i + 1].time - segments[i].time)) >= 2.0 || i == segments.size() - 2) && multiple_peaks) {
        multiple_peaks = false;
        float max_value_of_multiple_peaks = 0;
        int index_of_max_value_of_multiple_peaks = 0;
        // find out which index has the highest value
        for (int i = 0; i < indexes_of_multiple_peaks.size(); i++) {
          if (segments[indexes_of_multiple_peaks[i]].value >= max_value_of_multiple_peaks) {
            max_value_of_multiple_peaks = segments[indexes_of_multiple_peaks[i]].value;
            index_of_max_value_of_multiple_peaks = indexes_of_multiple_peaks[i];
          }
        }
        // remove the index of the value we want to keep
        indexes_of_multiple_peaks.erase(std::remove(indexes_of_multiple_peaks.begin(),
                                                    indexes_of_multiple_peaks.end(),
                                                    max_value_of_multiple_peaks), indexes_of_multiple_peaks.end());

        for (int j = indexes_of_multiple_peaks.size() - 1; j >= 0; j--) {
          std::cout << "to be erased: " << segments[indexes_of_multiple_peaks[j]].time << std::endl;
          for (int k = 0; k < segments.size(); k++) {
            if (indexes_of_multiple_peaks[j] == k) {
              segments.erase(segments.begin() + k);
            }
          }
        }
        i = 0;
        indexes_of_multiple_peaks.clear();
      }
    }
  }
  std::cout << "--- SEGMENTS JOHANNES STYLE ---" << std::endl;
  for (auto tvf:segments){
    std::cout << "time: " << tvf.time << std::endl;
  }




  if (FILEPRINT == true) {
    FILE *fp_novelty = std::fopen("/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty.csv", "w");

    for (int i = 0; i < novelty_function.size(); i++) {
      fprintf(fp_novelty, "%f, %f\n", novelty_function[i].time, novelty_function[i].value);
    }
    fclose(fp_novelty);
  }


  // #########################
  // ## 6. CLUSTER SEGMENTS ##
  // #########################

  // PRINT COMPUTATION TIME
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::chrono::duration<double> elapsed_seconds_audiobin = end_audiobin-start_audiobin;
  std::chrono::duration<double> elapsed_seconds_distance = end_distance-start_distance;
  std::chrono::duration<double> elapsed_seconds_file_ssm = end_file_ssm-start_file_ssm;
  std::chrono::duration<double> elapsed_seconds_kernel = end_kernel-start_kernel;
  std::chrono::duration<double> elapsed_seconds_filter = end_filter-start_filter;
  std::chrono::duration<double> elapsed_seconds_file_novelty = end_file_novelty-start_file_novelty;


  std::time_t end_time = std::chrono::system_clock::to_time_t(end);

  std::cout << "finished segmentation at " << std::ctime(&end_time)
            << "elapsed time: " << elapsed_seconds.count() << "s\n"
            << "elapsed_seconds_audiobin: " << elapsed_seconds_audiobin.count() << "s\n"
            << "elapsed_seconds_distance: " << elapsed_seconds_distance.count() << "s\n"
            << "elapsed_seconds_file_ssm: " << elapsed_seconds_file_ssm.count() << "s\n"
            << "elapsed_seconds_kernel: " << elapsed_seconds_kernel.count() << "s\n"
            << "elapsed_seconds_filter: " << elapsed_seconds_filter.count() << "s\n"
            << "elapsed_seconds_file_novelty: " << elapsed_seconds_file_novelty.count() << "s\n"
      ;


  // RETURN SEGMENTS

  return segments;
}

std::vector<time_value_int>
Analysis::get_intensity_average_for_next_segment(std::vector<double> beats, int beats_per_minute,
                                                 double first_good_beat) {

    // TODO: 4 beats, also 1 takt in die zukunft schauen um average intensität für diesen takt zu berechnen
    int block_size = (60 * 4 * samplerate) / beats_per_minute;
    // TODO: alle 4 beats, also jeden takt average intensity des kommenden takts bestimmen
    int jump_size = (60 * 4 * samplerate) / beats_per_minute;
    while (first_good_beat > jump_size) {
        first_good_beat -= jump_size;
    }
    int chunk_position = first_good_beat;

    std::vector<time_value_double> intensities = std::vector<time_value_double>();
    std::vector<time_value_int> intensities_normalized = std::vector<time_value_int>();
    float normalization_factor = (2.0 * 1.63) / samplerate;

    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;

    data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);
    fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);
    ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * block_size);

    plan_forward = fftw_plan_dft_1d(block_size, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

    // construct window
    float *buffer;
    window = (float *) malloc(block_size * sizeof(float));
    for (int i = 0; i < block_size; i++) {
        window[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((block_size - 1) * 1.0))));
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

        for (int i = 0; i < block_size; i++) {
            read_index = chunk_position + i;

            if (read_index < signal_length_mono) {
                data[i][0] = wav_values_mono[read_index] * window[i];
                data[i][1] = 0.0;
            } else {
                data[i][0] = 0.0;
                data[i][1] = 0.0;
                b_stop = 1;
            }
        }
        fftw_execute(plan_forward);
        std::vector<double> window_values = std::vector<double>(block_size / 2);
        for (int i = 0; i < window_values.size(); i++) {

            window_values[i] = sqrt(
                    (
                            (
                                    (fft_result[i][0] * fft_result[i][0])
                                    +
                                    (fft_result[i][1] * fft_result[i][1])
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

            if (f < 1) {
                weight = 0;
            } else if (f >= 1 && f <= 150) {
                weight = 2;
            } else if (f > 150 && f <= 2000) {
                weight = 1.5;
            } else if (f > 2000 && f <= 6000) {
                weight = 0.5;
            } else if (f > 6000 && f <= 16000) {
                weight = 0;
            } else if (f > 16000) {
                weight = 0;
            }

            //Logger::info("window_values[i]: {}", window_values[i]);
            average_window_value += (window_values[i] * weight);

        }

        average_window_value /= ((float) block_size / 2);

        time_value_double current = {0.0, 0.0};
        current.time = ((double) (num_chunks * jump_size) / samplerate) + first_good_beat / 44100;
        current.value = average_window_value;
        if (average_window_value > max_intensity) {
            max_intensity = average_window_value;
        }
        intensities.push_back(current);
        chunk_position += jump_size;
        num_chunks++;

        //Logger::info("average_window_value: {}", average_window_value);
    }

    fftw_destroy_plan(plan_forward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);


    for (int i = 0; i < intensities.size(); i++) {
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

std::vector<time_value_int> Analysis::get_intensity_changes(std::vector<time_value_int> intensities, int threshold) {

    std::vector<time_value_int> intensity_changes;
    intensity_changes.push_back({intensities[0].time, intensities[0].value});
    for (int i = 0; i < intensities.size(); i++) {
        int a = intensity_changes.size() - 1;
        int percent_now = intensities[i].value;
        if (percent_now > intensity_changes[a].value + threshold ||
            percent_now < intensity_changes[a].value - threshold) {
            //intensities[i].value = cut_now;
            intensity_changes.push_back({intensities[i].time, intensities[i].value});
        }
    }
    return intensity_changes;
}

float Analysis::get_intensity_mean() {

    float intensity = 0;

    // eine fft über den gesamten song
    int n = signal_length_mono;
    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;

    data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);
    fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);
    ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * window_size);

    plan_forward = fftw_plan_dft_1d(n, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < n; i++) {

        data[i][0] = wav_values_mono[i];
        data[i][1] = 0.0;

    }

    fftw_execute(plan_forward);

    // energiewerte aufsummieren und mitteln
    float sum = 0;
    for (int i = 0; i < n / 2; i++) {
        sum = ((fft_result[i][0] * fft_result[i][0]) + (fft_result[i][1] * fft_result[i][1])) / sqrtf((float) 2);
    }
    sum = sum / n;
    // alles wieder freigeben
    fftw_destroy_plan(plan_forward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);
    return intensity;
};

std::vector<time_value_int> Analysis::peaks_per_band(int f1, int f2) {

    std::vector<time_value_int> data = std::vector<time_value_int>();

    float f1_sample = ((float) window_size / samplerate) * f1;
    float f2_sample = ((float) window_size / samplerate) * f2;
    float freq_step = (float) samplerate / window_size;

    for (int i = 0; i < normalized_result.size(); i++) {
        int this_value = 0;
        int this_sample = 0;
        int this_block = 0;
        float this_freq = 0;
        for (int n = f1_sample; n <= f2_sample; n++) {

            if (normalized_result[i][n] >= this_value) {
                this_sample = n;
                this_block = i;
                this_value = normalized_result[i][n];
            }

        }

        float sample_time = (this_block * hop_size + ((float) window_size / 2)) / samplerate;
        time_value_int tvalue = {0.0, 0};
        tvalue.time = sample_time;
        tvalue.value = this_value;

        data.push_back(tvalue);
    }


    return data;
}

void Analysis::normalize() {

    for (int i = 0; i < result.size(); i++) {
        for (int n = 0; n < window_size / 2; n++) {

            if (result[i][n] > 0) {
                normalized_values[n] = (result[i][n]) / (fft_max_value) * 255;
            } else {
                normalized_values[n] = 0;
            }

        }
        normalized_result.push_back(normalized_values);
    }

    Logger::debug("normalized_result.size(): {}", normalized_result.size());

}

float Analysis::get_length_of_song() {
    float t = signal_length_mono / this->samplerate;
    //std::cout << t << std::endl;
    return t;
}

long long Analysis::factorial(long long n) {
    return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void Analysis::binomial_weights(int ma_window_size) {

    // precalculate weights for given window size
    //
    // (ma_window_size over current_position (here = i))
    //
    //  n over k = n! / ( k! * (n - k)! )


    long long current_weight = 0;
    long long fac_window_size = factorial(ma_window_size);

    for (int i = 0; i <= ma_window_size; i++) {

        current_weight = fac_window_size / (factorial(i) * factorial(ma_window_size - i));

        ma_weights.push_back(current_weight);
    }
    Logger::debug("ma_weights.size(): {}", ma_weights.size());

}

float Analysis::average(std::vector<time_value_float> data) {
    float mw = 0;
    for (int i = 0; i < data.size(); i++) {
        mw += data.at(i).value / data.size();
    }
    return mw;
}

std::vector<time_value_double> Analysis::moving_average(int left, int right, std::vector<time_value_double> data) {
    int ma_window_size = left + right + 1;
    // calculate weights ONCE
    binomial_weights(ma_window_size);

    //Moving Average weighted and centered
    std::vector<time_value_double> data2;
    time_value_double temp = {0.0, 0.0};
    float zeit;

    long long d = pow(2, ma_window_size);

    for (int i = 0; i <= data.size() - ma_window_size; i++) {
        double ds = 0;
        zeit = data.at(i).time;
        int z = i - left;
        if (z < 0) {
            z = 0;
        }

        for (int a = 0; a < ma_window_size; a++) {
            ds += data.at(z + a).value * ma_weights[a];
        }

        temp = {zeit, ds / d};

        data2.push_back(temp);
    }
    return data2;
}

std::vector<time_value_float> Analysis::moving_average_delta(std::vector<time_value_float> data, float average) {
    std::vector<time_value_float> ma_result;
    for (int i = 0; i < data.size(); i++) {
        ma_result.push_back({data.at(i).time, fabsf(data.at(i).value - average)});
    }
    return ma_result;
}

std::vector<time_value_double> Analysis::get_turningpoints(std::vector<time_value_double> data) {
    std::vector<time_value_double> tp_result;
    for (int i = 1; i < data.size() - 2; i++) {
        double delta0 = fabs(data.at(i).value - data.at(i - 1).value);
        double delta1 = fabs(data.at(i + 1).value - data.at(i).value);
        double delta2 = fabs(data.at(i + 2).value - data.at(i + 1).value);
        if ((delta1 > delta2 && delta1 > delta0 && delta1 > 0)) {
            // CALCULATE VALUE
            tp_result.push_back({data.at(i).time, data.at(i).value});
        }
    }
    return tp_result;
}

complex_result Analysis::fft_impulse(std::vector<double> signal) {

    int signalLength = bpm_seconds_to_check * 44100;

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

    if (signal_length_mono < 15 * samplerate)
        return -1;

    int signalLength = bpm_seconds_to_check * 44100;

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
            } else {
                impulsee = 0;
            }
            if (n >= 4) {
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
                if (z == 0) {
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

double Analysis::get_first_beat() {

    int windowSize = 1024;
    int hopSize = 1024;
    int bands = 1;
    int samplesPerBand = (1024 / 2) / bands;
    int signalLength = signal_length_mono;

    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;

    data = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * windowSize);
    fft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * windowSize);
    ifft_result = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * windowSize);

    plan_forward = fftw_plan_dft_1d(windowSize, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);

    // Create a hamming window of appropriate length

    int chunkPosition = 0;
    double maxResult = 0;

    int readIndex;

    // Should we stop reading in chunks?
    int bStop = 0;
    int numChunks = 0;

    std::vector<double> energySubband = std::vector<double>((signalLength / windowSize + 1) * bands);
    std::vector<double> energySubbandAvg = std::vector<double>((signalLength / windowSize + 1) * bands);
    std::vector<double> first_beat_result = std::vector<double>(windowSize);
    std::vector<double> time;


    // Process each chunk of the signal
    while (chunkPosition < signalLength && !bStop) {

        // Copy the chunk into our buffer
        for (int i = 0; i < windowSize; i++) {

            readIndex = chunkPosition + i;

            if (readIndex < signalLength) {
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
        fftw_execute(plan_forward);


        double scale = (1 - 0) / log(1.0 + 16);
        double upper_bound = 0 + log(2 + 1) * scale;

        for (int i = 0; i < windowSize * upper_bound; i++) {
            first_beat_result[i] = sqrt(fft_result[i][0] * fft_result[i][0] + fft_result[i][1] * fft_result[i][1]);
        }


        for (int i = 0; i < bands; i++) {
            for (int j = 0; j < windowSize * upper_bound; j++) {
                energySubband[i + (numChunks * bands)] = energySubband[i + (numChunks * bands)] +
                                                         (first_beat_result[j + (i * samplesPerBand)] *
                                                          first_beat_result[j + (i * samplesPerBand)]);
            }
            energySubband[i + (numChunks * bands)] = energySubband[i + (numChunks * bands)] /
                                                     samplesPerBand;                        //pow(energySubband[i+(numChunks*bands)],2);
            if (energySubband[i + (numChunks * bands)] > maxResult) maxResult = energySubband[i + (numChunks * bands)];
        }
        chunkPosition += hopSize;
        numChunks++;
    }


    for (int z = 0; z < numChunks - 43; z++) {
        for (int i = 0; i < 1; i++) { // welches Band
            for (int j = 0; j < 43; j++) {
                energySubbandAvg[i + z * bands] =
                        energySubbandAvg[i + z * bands] + energySubband[i + (j * bands) + (z * bands)];
            }
            energySubbandAvg[i + z * bands] = energySubbandAvg[i + z * bands] / 43;
            if (energySubband[i + z * bands] > 1.5f * energySubbandAvg[i + z * bands] &&
                energySubband[i + z * bands] > 0.5f * maxResult) {
                time.push_back((windowSize * z) - 512);
                z = z + 5;
                break;
            }
        }
    }
    Logger::debug("time.size(): {}", time.size());

    double timeTemp = time[0];

    fftw_destroy_plan(plan_forward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);

    return timeTemp;
};

std::vector<double> Analysis::get_all_beats(int beats_per_minute, double firstBeat) {


    std::vector<double> times;

    double timeTemp = firstBeat;
    //printf("Firstbeat %f\n",timeTemp);
    for (int i = 0; i < ((firstBeat * beats_per_minute) / (samplerate * 60)) - 1; i++) {
        timeTemp = timeTemp - (60 * samplerate / beats_per_minute);
        //printf("TimeTemp %f\n",timeTemp);
        times.insert(times.begin(), timeTemp);
    }

    timeTemp = firstBeat;
    times.push_back(firstBeat);
    for (int i = 0; i < ((((signal_length_mono - firstBeat) / samplerate) / 60) * beats_per_minute) - 1; i++) {
        timeTemp = timeTemp + (60 * samplerate / beats_per_minute);
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
    if (resolution == 40)
        this->hop_size = 1102;
    else if (resolution == 20)
        this->hop_size = 2205;
    else if (resolution == 10)
        this->hop_size = 4410;
    else if (resolution == 5)
        this->hop_size = 8820;

    Logger::debug("Set resolution of Analysis to: {}, hop size now: {}", resolution, this->hop_size);
}

int Analysis::get_samplerate() {
    return this->samplerate;
}

int Analysis::get_spectral_flux() {

    return 0;
}