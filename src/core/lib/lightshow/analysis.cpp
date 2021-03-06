//
// Created by Steven Drewers on 2019-05-05.
//

#include <logger.h>
#include <map>
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

std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> Analysis::get_onset_timestamps_energy_difference(float onset_value, int onset_bass_lower_frequency, int onset_bass_upper_frequency, int onset_bass_threshold, int onset_snare_lower_frequency, int onset_snare_upper_frequency, int onset_snare_threshold) {

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


  std::cout << "result.size(): " << result.size() << std::endl;
  std::cout << "window_size: " << window_size << std::endl;
//  std::cout << "wav_values_mono.size(): " << wav_values_mono.size() << std::endl;
  std::cout << "signal_length_mono - window_size_onsets: " << signal_length_mono - window_size_onsets << std::endl;
  std::cout << "window_size_onsets: " << window_size_onsets << std::endl;

    // works good for rocky stuff and clear electronic bass
    for (int i = 0; i < signal_length_mono - window_size_onsets; i = i + window_size_onsets / 2) {
        for (int j = i, k = 0; k < window_size_onsets; j++, k++) {
            audioFrame[k] = wav_values_mono[j];
            //std::cout << audioFrame[k] << std::endl;
        }


        gist2.processAudioFrame(audioFrame, window_size_onsets);
        float ed = gist2.energyDifference();
        float i_float = i;
        float time = i_float / 44100;

        //if(ed > 0)
        //std::cout << "ed(" << time << "): " << ed << std::endl;

        onsets.push_back({time, ed});
    }

    // look for max peak
    float max_ed_value = 0.0;
    for (int i = 0; i < onsets.size(); i++) {
        if (onsets[i].value > max_ed_value)
            max_ed_value = onsets[i].value;
    }

    // look for all peaks, then take mean
    float all_peaks = 0.0;
    int peak_counter = 0;
    for (int i = 0; i < onsets.size(); i++) {
        if (i > 0 && i < onsets.size() - 1) {
            if (onsets[i].value > onsets[i - 1].value && onsets[i].value > onsets[i + 1].value &&
                onsets[i].value > max_ed_value * 0.25) {
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
    min_value_onset =
            (max_ed_value + mean_of_all_peaks + mean_of_all_peaks) / onset_value; // 7.5 zu wenig, 10 zu viel, 9 maybe
    std::cout << "mean_of_all_peaks: " << mean_of_all_peaks << std::endl;
    std::cout << "max_ed_value: " << max_ed_value << std::endl;
    std::cout << "min_value_onset: " << min_value_onset << std::endl;
    std::cout << "old min_value_onset: " << max_ed_value * 0.315 << std::endl;
    float threshold_reset = 0.0f;

    for (time_value_float onset: onsets) {
        //std::cout << "ed(" << onset.time << "): " << onset.value << std::endl;
        if (onset.value > min_value_onset)
            onset_found = true;

        if (onset_found) {
            if (onset.value < last_value && !already_added_this_onset) {
                onset_timestamps.emplace_back(last_time);
                already_added_this_onset = true;
            }
        }

        if (onset.value <= threshold_reset) {
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





  // ####################### testing bass / snare detection

//  std::vector<time_value_int> peaks_bass = this->peaks_per_band(40, 65);
  std::vector<time_value_int> peaks_bass = this->peaks_per_band(onset_bass_lower_frequency, onset_bass_upper_frequency);
//  std::vector<time_value_int> peaks_bass_attack = this->peaks_per_band(3000, 4000);
//  std::vector<time_value_int> peaks_snare = this->peaks_per_band(200, 300);
  std::vector<time_value_int> peaks_snare = this->peaks_per_band(onset_snare_lower_frequency, onset_snare_upper_frequency);

//  for(int i = 0; i < peaks_bass.size(); i++) {
//    std::cout << peaks_bass[i].time << " bass: " << peaks_bass[i].value << " snare: " << peaks_snare[i].value << std::endl;
//  }

//  std::vector<time_value_int> onset_bass_values;
//  std::vector<time_value_int> onset_snare_values;
  std::vector<float> bass_onsets;
  std::vector<float> snare_onsets;

  std::vector<freq_value> frequency_values;
  std::vector<int> frequencies;



  for(float &timestamp : onset_timestamps) {

//    for(int freq = 1; freq < 500 / 25; freq++) {
//      std::vector<time_value_int> peaks_freq = this->peaks_per_band((freq - 1) * 25 + 1, freq * 25);
//
//      // find nearest bass value
//      auto j = min_element(begin(peaks_freq), end(peaks_freq), [=] (time_value_int x, time_value_int y)
//      {
//        return abs(x.time - timestamp) < abs(y.time - timestamp);
//      });
//
//      int value_freq = peaks_freq[std::distance(begin(peaks_freq), j)].value;
//
////      std::cout << "freq: " << (freq - 1) * 25 + 1 << "  value: " << value_freq << std::endl;
//
//      frequency_values.push_back({freq, value_freq});
////      frequencies.push_back(freq);
////      frequencies.push_back(value_freq);
//
//    }
//
//    std::cout << frequency_values[2].value << std::endl;
//    std::cout << frequency_values[3].value << std::endl;
//    std::cout << frequency_values[4].value << std::endl;
//    std::cout << std::endl;
//    if(frequency_values[2].value >= 150)
//      if(frequency_values[4].value > frequency_values[3].value)
//        snare_onsets.push_back(timestamp);
//      else
//        bass_onsets.push_back(timestamp);
//    else
//      snare_onsets.push_back(timestamp);
//
//    frequency_values.clear();
//
//    int maxElementIndex = std::max_element(frequencies.begin(), frequencies.end()) - frequencies.begin();
//
////    std::cout << "peak at onset in frequency: " << frequencies[maxElementIndex] << " with value: " << frequency_values[maxElementIndex].value << std::endl;
////    std::cout << "peak at onset in frequency: " << frequency_values[maxElementIndex].frequency << " with value: " << frequency_values[maxElementIndex].value << std::endl;






    // find nearest bass value
    auto j = min_element(begin(peaks_bass), end(peaks_bass), [=] (time_value_int x, time_value_int y)
    {
      return abs(x.time - timestamp) < abs(y.time - timestamp);
    });
//    onset_bass_values.push_back(peaks_bass[std::distance(begin(peaks_bass), j)]);
//    float time_bass = peaks_bass[std::distance(begin(peaks_bass), j)].time;
    int value_bass = peaks_bass[std::distance(begin(peaks_bass), j)].value;
    int value_bass_prev = value_bass;
    int value_bass_next = value_bass;
    int value_bass_next2 = value_bass;
    if (std::distance(begin(peaks_bass), j) > 1) {
//      value_bass = peaks_bass[std::distance(begin(peaks_bass), j)].value - peaks_bass[std::distance(begin(peaks_bass), j) - 1].value;
      value_bass_prev = peaks_bass[std::distance(begin(peaks_bass), j) - 1].value;
    }
    if(std::distance(begin(peaks_bass), j) < peaks_bass.size() - 2) {
      value_bass_next = peaks_bass[std::distance(begin(peaks_bass), j) + 1].value;
      value_bass_next2 = peaks_bass[std::distance(begin(peaks_bass), j) + 2].value;
    }


//    // find nearest bass attack
//    auto j2 = min_element(begin(peaks_bass_attack), end(peaks_bass_attack), [=] (time_value_int x, time_value_int y)
//    {
//      return abs(x.time - timestamp) < abs(y.time - timestamp);
//    });
//    int value_bass_attack = peaks_bass_attack[std::distance(begin(peaks_bass_attack), j2)].value;
//    if (std::distance(begin(peaks_bass), j) > 1)
//      value_bass_attack = peaks_bass[std::distance(begin(peaks_bass), j)].value - peaks_bass[std::distance(begin(peaks_bass), j-1)].value;



    // find nearest snare value
    auto k = min_element(begin(peaks_snare), end(peaks_snare), [=] (time_value_int x, time_value_int y)
    {
      return abs(x.time - timestamp) < abs(y.time - timestamp);
    });
    int value_snare = peaks_snare[std::distance(begin(peaks_snare), k)].value;
    int value_snare_prev = value_snare;
    int value_snare_next = value_snare;
    int value_snare_next2 = value_snare;

    if (std::distance(begin(peaks_snare), k) > 1)
      value_snare_prev = peaks_snare[std::distance(begin(peaks_snare), k)].value - peaks_snare[std::distance(begin(peaks_snare), k)-1].value;

    if(std::distance(begin(peaks_snare), k) < peaks_snare.size() - 2) {
      value_snare_next = peaks_snare[std::distance(begin(peaks_snare), k) + 1].value;
      value_snare_next2 = peaks_snare[std::distance(begin(peaks_snare), k) + 2].value;
    }

//    std::cout << timestamp << "   value_bass: " << value_bass << " value_bass_attack: " << value_bass_attack << "     value_snare: " << value_snare << std::endl;
//    std::cout << timestamp << "    value_bass_prev: " << value_bass_prev << "    value_bass: " << value_bass << "    value_bass_next: " << value_bass_next << "    value_bass_next2: " << value_bass_next2 << "      value_snare: " << value_snare << std::endl;

//    std::cout << timestamp << "    value_bass_prev: " << value_bass_prev << "    value_bass: " << value_bass << "    value_bass_next: " << value_bass_next << "    value_bass_next2: " << value_bass_next2 << std::endl;
//    std::cout << timestamp << "    value_snare_prev: " << value_snare_prev << "    value_snare: " << value_snare << "    value_snare_next: " << value_snare_next << "    value_snare_next2: " << value_snare_next2 << std::endl;

//    std::cout << timestamp << "   value_bass_next2: " << value_bass_next2 << "     value_snare_next2: " << value_snare_next2 << std::endl;

//    if(value_snare >= value_bass)
//    if(value_bass < 3)
//    if(value_bass_next2 <= value_snare)
//      snare_onsets.push_back(timestamp);
//    else
//      bass_onsets.push_back(timestamp);




    if(value_bass_next2 > onset_bass_threshold) {
      bass_onsets.push_back(timestamp);
    }
    if(value_snare_next2 >= onset_snare_threshold) {
      snare_onsets.push_back(timestamp);
    }


  }





    return std::make_tuple(onset_timestamps, bass_onsets, snare_onsets);
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
    for (int i = 0; i < spectral_flux.size() - (fluxes / 2); i++) {
        int start = fmax(0, i - (fluxes / 2) + 1);
        int end = fmin(spectral_flux.size() - 1, i + (fluxes / 2) + 1);
        float mean = delta * multiplier;
        float mean_current = 0;

        for (int j = start; j <= end; j++) {
            mean_current = spectral_flux[j].value / (end - start);
            mean = mean + mean_current;
        }
        //mean /= (float)(end - start);,m,m,m

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
    for (int i = 0; i < spectral_fluxes.size() - (fluxes / 2); i++) {
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

    for (int i = 0; i < spectral_fluxes.size() - (fluxes / 2); i++) {
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

            if (!onsets.empty() && (average_flux[i].time - onsets.back() >= 0.025)) {
                onsets.push_back({average_flux[i].time});
                //fprintf(fp_onsets, "%f, %d\n", average_flux[i].time, 1);
            } else if (onsets.empty()) {
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

std::vector<std::vector<float>> Analysis::get_stmfcc(float *signal_values, int bin_size, int hop_size, int offset, int cut_start, int cut_end) {

    std::vector<std::vector<float>> stmfcc;
    std::vector<float> bin, coeffs;
    Gist<float> gist2(bin_size, samplerate);

    for (int i = offset; i < (signal_length_mono - bin_size); i += hop_size) {

        for (int j = 0; j < bin_size; j++) {
            int index = i + j;
            //std::cout << "index " << index << ": " << wav_values_mono[index] << std::endl;
            bin.push_back(signal_values[index]);
        }
        gist2.processAudioFrame(bin);

        // USE MFCC
        coeffs = gist2.getMelFrequencySpectrum();

        std::vector<float>::const_iterator first = coeffs.begin() + (cut_start - 1);
        std::vector<float>::const_iterator last = coeffs.begin() + (cut_end - 1);
        std::vector<float> coeffs_cut(first, last);

        stmfcc.push_back(coeffs_cut);
        coeffs_cut.clear();
        coeffs.clear();
        bin.clear();
    }


    return stmfcc;
}

std::vector<std::vector<float>>
Analysis::get_chromagram(float *signal_values, int bin_size, int hop_size, int song_samplerate, int offset) {

    std::vector<std::vector<float>> chromagram;
    std::vector<double> bin, chromagram_line;
    std::vector<float> chromagram_line_f;
    Chromagram chroma(bin_size, song_samplerate);

    for (int i = offset; i < (signal_length_mono - bin_size); i += hop_size) {
        for (int j = 0; j < bin_size; j++) {
            int index = i + j;
            bin.push_back((double) signal_values[index]);
        }
        chroma.processAudioFrame(bin);

        if (chroma.isReady()) {
            chromagram_line = chroma.getChromagram();
            for (auto x:chromagram_line) {
                chromagram_line_f.push_back((float) x);
            }
            chromagram.push_back(chromagram_line_f);
        }
        chromagram_line.clear();
        chromagram_line_f.clear();
        bin.clear();
    }


    return chromagram;
}

std::vector<std::vector<float>>
Analysis::get_spectrogram(float *signal_values, int bin_size, int hop_size, int offset) {
    std::vector<std::vector<float>> spectrogram;

    std::vector<float> bin, coeffs, coeffs_half;
    Gist<float> gist2(bin_size, samplerate);

    for (int i = offset; i < (signal_length_mono - bin_size); i += hop_size) {

        for (int j = 0; j < bin_size; j++) {
            int index = i + j;
            //std::cout << "index " << index << ": " << wav_values_mono[index] << std::endl;
            bin.push_back(signal_values[index]);
        }
        gist2.processAudioFrame(bin);

        coeffs = gist2.getMagnitudeSpectrum();

        for (int j = 0; j < coeffs.size() / 2; j++) {
            coeffs_half.push_back(coeffs[j]);
        }

        spectrogram.push_back(coeffs_half);
        coeffs.clear();
        coeffs_half.clear();
        bin.clear();
    }

    return spectrogram;
}

std::vector<std::vector<float>>
Analysis::get_rhythmogram(float *signal_values, int bin_size, int hop_size, int offset) {
    std::vector<std::vector<float>> rhythmogram;
    return rhythmogram;
}

float Analysis::get_cosine_distance(const std::vector<float> &m, const std::vector<float> &n) {

    float cosine_distance = 0;

    float p_mn = 0;
    float p_mm = 0;
    float p_nn = 0;
    for (int j = 0; j < m.size(); j++) {
        p_mn += m[j] * n[j];
        p_mm += m[j] * m[j];
        p_nn += n[j] * n[j];
    }

    cosine_distance = p_mn / (sqrt(p_mm) * sqrt(p_nn));

    return cosine_distance;

}

float Analysis::get_exponential_cosine_distance(const std::vector<float> &m, const std::vector<float> &n) {

    float exponential_cosine_distance = 0;

    float p_mn = 0;
    float p_mm = 0;
    float p_nn = 0;
    for (int j = 0; j < m.size(); j++) {
        p_mn += m[j] * n[j];
        p_mm += m[j] * m[j];
        p_nn += n[j] * n[j];
    }

    exponential_cosine_distance = exp((p_mn / (sqrt(p_mm) * sqrt(p_nn))) - 1);

    return exponential_cosine_distance;
}

std::vector<std::vector<float>>
Analysis::get_self_similarity_matrix(const std::vector<std::vector<float>> &f_window, int distance_formula) {

    std::vector<std::vector<float>> ssm;
    std::vector<float> similarity;
    //ssm.resize(window.size());
    //similarity.reserve(window.size());
    float distance = 0;

    for (int m = 0; m < f_window.size(); m++) {
        similarity.clear();
        for (int n = 0; n < f_window.size(); n++) {

            // #################################################################
            // ###################### 3.1 COSINE DISTANCE ######################
            // #### d_cos  =  m*n / |m|*|n|  =  m*n / sqrt(m*m) * sqrt(n*n) ####
            // #################################################################
            // ################## EXPONENTIAL COSINE DISTANCE ##################
            // ######################## exp(d_cos - 1) #########################

            if (distance_formula == 1) {
                distance = get_exponential_cosine_distance(f_window[m], f_window[n]);
            } else {
                distance = get_cosine_distance(f_window[m], f_window[n]);
            }
            similarity.push_back(distance);
            //similarity.emplace_back(distance);
        }

        ssm.push_back(similarity);
        //ssm[m] = similarity;
    }
    //std::cout << "ssm.size(): " << ssm.size() << std::endl;
    //std::cout << "similarity.size(): " << similarity.size() << std::endl;
    return ssm;
}

std::vector<std::vector<float>> Analysis::get_filter_kernel(int song_bpm, int bin_size, int song_samplerate, int bars) {

    std::vector<std::vector<float>> kernel;
    std::vector<float> kernel_line;
    float kernel_value = 0;


    std::vector<std::vector<float>> gaussian_kernel;
    std::vector<float> gaussian_kernel_line;
    float gaussian_kernel_value = 0;

    std::vector<std::vector<float>> cb_kernel;
    std::vector<float> cb_kernel_line;
    float cb_kernel_value = 0;

    float kernel_time = (float) (bars * 4) * ((float) 60.0 / (float) song_bpm);
    float L_f = (kernel_time / (float) 2.0) / ((float) bin_size / (float) song_samplerate);
    int L = L_f;
    int N = 2 * L + 1;
    std::cout << "L: " << L << std::endl;

    double var = 0.5;
    double epsilon = sqrt(0.5) / ((double) L * var);

    // CHECKERBOARD KERNEL
    for (int m = 1; m <= N; m++) {
        for (int n = 1; n <= N; n++) {
            if ((m <= L && n <= L) || (m >= (L + 2) && n >= (L + 2))) {
                cb_kernel_value = 1;
            } else if (m == (L + 1) || n == (L + 1)) {
                cb_kernel_value = 0;
            } else {
                cb_kernel_value = -1;
            }
            cb_kernel_line.push_back(cb_kernel_value);
        }
        cb_kernel.push_back(cb_kernel_line);
        cb_kernel_line.clear();
    }

    // RADIAL GAUSSIAN KERNEL
    for (double s = -L; s <= L; s += 1.0) {
        for (double t = -L; t <= L; t += 1.0) {
            gaussian_kernel_value = exp(-1.0 * pow(epsilon, 2.0) * (pow(s, 2.0) + pow(t, 2.0)));
            gaussian_kernel_value /= (4.0 * pow(L, 2.0));
            gaussian_kernel_line.push_back((float) gaussian_kernel_value);
        }
        gaussian_kernel.push_back(gaussian_kernel_line);
        gaussian_kernel_line.clear();
    }

    // VERHEIRATEN
    for (int m = 0; m < N; m++) {
        for (int n = 0; n < N; n++) {
            kernel_value = cb_kernel[m][n] * gaussian_kernel[m][n];
            kernel_line.push_back(kernel_value);
        }
        kernel.push_back(kernel_line);
        kernel_line.clear();
    }

    return kernel;
}

std::vector<time_value_float>
Analysis::get_novelty_function(std::vector<std::vector<float>> ssm, std::vector<std::vector<float>> kernel,
                               int cut_seconds_end, int bin_size, int song_samplerate, int N, int offset) {
    std::vector<time_value_float> novelty_function;
    float novelty_value;
    int L = (N - 1) / 2;

    float cut = ((float) song_samplerate / (float) bin_size) * cut_seconds_end;
    for (int n = 0; n < ssm.size() - cut; n++) {
        novelty_value = 0;
        // [n,n] müsste der punkt sein, um den herum wir uns alles ansehen.... also auf der diagonalen liegen
        for (int k = 0; k < N; k++) {
            // dann über k....
            for (int l = 0; l < N; l++) {
                // dann checken wie groß n gerade so ist, evtl. müssen wir gar nicht rechnen, weil zero padding

                if ((n - L) < 0 || ((n - L) + k) >= ssm.size() || ((n - L) + l) >= ssm.size()) {
                    novelty_value += 0;
                } else {
                    float k_val = kernel[k][l];
                    float ssm_val = ssm[(n - L) + k][(n - L) + l];
                    novelty_value += (k_val * ssm_val);
                }

            }
        }
        float novelty_time = ((float) n * ((float) bin_size / (float) song_samplerate)) +
                             (((float) bin_size / (float) 2.0) / (float) song_samplerate) +
                             ((float) offset / song_samplerate);
        novelty_function.push_back({novelty_time, novelty_value});
    }

    return novelty_function;
}

std::vector<std::vector<float>> Analysis::norm_euclidian(std::vector<std::vector<float>> v, float threshold){
    float norm = 0;

    for (int i = 0; i < v.size(); i++){
        for (int j = 0; j < v[i].size(); j++){
            norm += pow(v[i][j], 2);
        }
        norm = sqrt(norm);
        for (int j = 0; j < v[i].size(); j++){
            if (v[i][j] > threshold){
                v[i][j] /= norm;
            }
        }
    }

    return v;
}
std::vector<std::vector<float>> Analysis::norm_absolute(std::vector<std::vector<float>> v, float threshold){
    float norm = 0;

    for (int i = 0; i < v.size(); i++){
        for (int j = 0; j < v[i].size(); j++){
            norm += abs(v[i][j]);
        }
        norm = sqrt(norm);
        for (int j = 0; j < v[i].size(); j++){
            if (v[i][j] > threshold){
                v[i][j] /= norm;
            }
        }
    }

    return v;
}
std::vector<std::vector<float>> Analysis::norm_manhattan(std::vector<std::vector<float>> v, float threshold){
    float norm = -INFINITY;

    for (int i = 0; i < v.size(); i++){
        norm = -INFINITY;
        for (int j = 0; j < v[i].size(); j++){
            if (v[i][j] > norm){
                norm = v[i][j];
            }
        }
        for (int j = 0; j < v[i].size(); j++){
            if (v[i][j] > threshold){
                v[i][j] /= norm;
            }
        }
    }

    return v;
}
std::vector<std::vector<float>> Analysis::norm_mean_variance(std::vector<std::vector<float>> v, float threshold){
    float norm = 0;
    float dev = 0;
    float var = 0;
    std::vector<std::vector<float>> normed;

    for (int i = 0; i < v.size(); i++){
        var = 0;
        dev = 0;
        for (int j = 0; j < v[i].size(); j++){
            dev += v[i][j];
        }
        dev /= v[i].size();
        for (int j = 0; j < v[i].size(); j++){
            var += pow((v[i][j] - dev),2);
        }
        var /= (v[i].size() - 1);
        var = sqrt(var);

        for (int j = 0; j < v[i].size(); j++){
            if (v[i][j] > threshold){
                v[i][j] = (v[i][j] - dev) / var;
            }
            else {
            }
        }
    }

    return v;
}

std::vector<time_value_float>
Analysis::get_combined_novelty_function(std::vector<time_value_float> mfcc, std::vector<time_value_float> chroma,
                                        std::vector<time_value_float> stft, std::vector<time_value_float> rhythm,
                                        float mfcc_co, float chroma_co, float stft_co, float rhythm_co, bool FILEPRINT,
                                        char const *directory) {

    std::vector<time_value_float> combined_novelty_function;

    float mfcc_max = 0;
    float chroma_max = 0;
    float stft_max = 0;
    float rhythm_max = 0;
    int size = 0;
    float combined_factor = 0;

    if (!mfcc.empty())
        size = mfcc.size();
    else if (!chroma.empty())
        size = chroma.size();
    else if (!stft.empty())
        size = stft.size();
    else if (!rhythm.empty())
        size = rhythm.size();


    if (mfcc_co > 0) {
        combined_factor += mfcc_co;
    }
    if (chroma_co > 0) {
        combined_factor += chroma_co;
    }
    if (stft_co > 0) {
        combined_factor += stft_co;
    }
    if (rhythm_co > 0) {
        combined_factor += rhythm_co;
    }

    for (int i = 0; i < size; i++) {
        if (!mfcc.empty())
            if (mfcc[i].value > mfcc_max)
                mfcc_max = mfcc[i].value;
        if (!chroma.empty())
            if (chroma[i].value > chroma_max)
                chroma_max = chroma[i].value;
        if (!stft.empty())
            if (stft[i].value > stft_max)
                stft_max = stft[i].value;
        if (!rhythm.empty())
            if (rhythm[i].value > rhythm_max)
                rhythm_max = rhythm[i].value;
    }

    for (int i = 0; i < size; i++) {
        if (!mfcc.empty())
            mfcc[i].value /= mfcc_max;
        if (!chroma.empty())
            chroma[i].value /= chroma_max;
        if (!stft.empty())
            stft[i].value /= stft_max;
        if (!rhythm.empty())
            rhythm[i].value /= rhythm_max;
    }

    if (FILEPRINT == true) {
        make_csv_timeseries_tvf(mfcc, directory, "novelty_function_mfcc");
        make_csv_timeseries_tvf(chroma, directory, "novelty_function_chroma");
        make_csv_timeseries_tvf(stft, directory, "novelty_function_stft");
        make_csv_timeseries_tvf(rhythm, directory, "novelty_function_rhythm");
    }

    // START PEAK SELECT ALGORITHM
    std::map<int, int> number_of_maxima_at_timestamp;
    std::map<int, int>::iterator it;
    std::vector<time_value_float> v_max;

    if (mfcc.size() > size)
        size = mfcc.size();
    if (chroma.size() > size)
        size = chroma.size();
    if (stft.size() > size)
        size = stft.size();
    if (rhythm.size() > size)
        size = rhythm.size();
    /*
    for (int i = 1; i < size - 1; i++) {
        if (mfcc.size() == size) {
            if (mfcc[i].value > mfcc[i - 1].value && mfcc[i].value > mfcc[i + 1].value) {
                it = number_of_maxima_at_timestamp.find(i);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i - 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i - 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i + 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i + 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
            }
        }

        if (chroma.size() == size) {
            if (chroma[i].value > chroma[i - 1].value && chroma[i].value > chroma[i + 1].value) {
                it = number_of_maxima_at_timestamp.find(i);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i - 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i - 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i + 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i + 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
            }
        }

        if (stft.size() == size) {
            if (stft[i].value > stft[i - 1].value && stft[i].value > stft[i + 1].value) {
                it = number_of_maxima_at_timestamp.find(i);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i - 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i - 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i + 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i + 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
            }
        }

        if (rhythm.size() == size) {
            if (rhythm[i].value > rhythm[i - 1].value && rhythm[i].value > rhythm[i + 1].value) {
                it = number_of_maxima_at_timestamp.find(i);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i - 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i - 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
                it = number_of_maxima_at_timestamp.find(i + 1);
                if (it != number_of_maxima_at_timestamp.end()) { // found
                    it->second++;
                    //std::cout << "increasing maxima" << std::endl;
                } else { // not found
                    number_of_maxima_at_timestamp.insert(std::pair<int, int>(i + 1, 1));
                    //std::cout << "adding maxima" << std::endl;
                }
            }
        }
    }
    */
    int number_of_maxima_needed = 1;

    /*
    if ((!mfcc.empty() && chroma.empty() && stft.empty() && rhythm.empty()) ||
        (mfcc.empty() && !chroma.empty() && stft.empty() && rhythm.empty()) ||
        (mfcc.empty() && chroma.empty() && !stft.empty() && rhythm.empty()) ||
        (mfcc.empty() && chroma.empty() && stft.empty() && !rhythm.empty())) {
        number_of_maxima_needed = 0;
    }
     */

    /*
    for (auto const &max: number_of_maxima_at_timestamp) {
        std::cout << "at: " << max.first << " there are " << max.second << " maxima" << std::endl;
        if (max.second > number_of_maxima_needed) { // found maximum at same timestamp in at least 2 vectors
            float max_value = 0;
            float timestamp_of_max_value = 0;
            if (max.first == 0 || max.first == size - 1) {
                if (mfcc.size() == size && mfcc[max.first].value > max_value) {
                    max_value = mfcc[max.first].value;
                    timestamp_of_max_value = mfcc[max.first].time;
                }
            } else {
                if (mfcc.size() == size) {
                    if (mfcc[max.first].value > max_value) {
                        max_value = mfcc[max.first].value;
                        timestamp_of_max_value = mfcc[max.first].time;
                    }
                    if (mfcc[max.first - 1].value > max_value) {
                        max_value = mfcc[max.first - 1].value;
                        timestamp_of_max_value = mfcc[max.first - 1].time;
                    }
                    if (mfcc[max.first + 1].value > max_value) {
                        max_value = mfcc[max.first + 1].value;
                        timestamp_of_max_value = mfcc[max.first + 1].time;
                    }
                }
            }
            if (max.first == 0 || max.first == size - 1) {
                if (chroma.size() == size && chroma[max.first].value > max_value) {
                    max_value = chroma[max.first].value;
                    timestamp_of_max_value = chroma[max.first].time;
                }
            } else {
                if (chroma.size() == size) {
                    if (chroma[max.first].value > max_value) {
                        max_value = chroma[max.first].value;
                        timestamp_of_max_value = chroma[max.first].time;
                    }
                    if (chroma[max.first - 1].value > max_value) {
                        max_value = chroma[max.first - 1].value;
                        timestamp_of_max_value = chroma[max.first - 1].time;
                    }
                    if (chroma[max.first + 1].value > max_value) {
                        max_value = chroma[max.first + 1].value;
                        timestamp_of_max_value = chroma[max.first + 1].time;
                    }
                }
            }
            if (max.first == 0 || max.first == size - 1) {
                if (stft.size() == size && stft[max.first].value > max_value) {
                    max_value = stft[max.first].value;
                    timestamp_of_max_value = stft[max.first].time;
                }
            } else {
                if (stft.size() == size) {
                    if (stft[max.first].value > max_value) {
                        max_value = stft[max.first].value;
                        timestamp_of_max_value = stft[max.first].time;
                    }
                    if (stft[max.first - 1].value > max_value) {
                        max_value = stft[max.first - 1].value;
                        timestamp_of_max_value = stft[max.first - 1].time;
                    }
                    if (stft[max.first + 1].value > max_value) {
                        max_value = stft[max.first + 1].value;
                        timestamp_of_max_value = stft[max.first + 1].time;
                    }
                }
            }
            if (max.first == 0 || max.first == size - 1) {
                if (rhythm.size() == size && rhythm[max.first].value > max_value) {
                    max_value = rhythm[max.first].value;
                    timestamp_of_max_value = rhythm[max.first].time;
                }
            } else {
                if (rhythm.size() == size) {
                    if (rhythm[max.first].value > max_value) {
                        max_value = rhythm[max.first].value;
                        timestamp_of_max_value = rhythm[max.first].time;
                    }
                    if (rhythm[max.first - 1].value > max_value) {
                        max_value = rhythm[max.first - 1].value;
                        timestamp_of_max_value = rhythm[max.first - 1].time;
                    }
                    if (rhythm[max.first + 1].value > max_value) {
                        max_value = rhythm[max.first + 1].value;
                        timestamp_of_max_value = rhythm[max.first + 1].time;
                    }
                }
            }
            v_max.push_back({timestamp_of_max_value, max_value});
        }
    }
     */

    /*
     std::map<int, int>::iterator it2 = number_of_maxima_at_timestamp.begin();
    it2.

    for(int j = 1; j < number_of_maxima_at_timestamp.size() - 1; j++) {
      std::pair<int, int> current = number_of_maxima_at_timestamp[j];
      auto const & previous = number_of_maxima_at_timestamp[j - 1];
      auto const & next = number_of_maxima_at_timestamp[j + 1];

      std::cout << "at: " << number_of_maxima_at_timestamp[j].first << " there are " << number_of_maxima_at_timestamp[j].second << " maxima" << std::endl;
      if(number_of_maxima_at_timestamp[j].second > 1) { // found maximum at same timestamp in at least 2 vectors
        float max_value = 0;
        float timestamp_of_max_value = 0;
        if(mfcc.size() == size && mfcc[number_of_maxima_at_timestamp[j].first].value > max_value) {
          max_value = mfcc[number_of_maxima_at_timestamp[j].first].value;
          timestamp_of_max_value = mfcc[number_of_maxima_at_timestamp[j].first].time;
        }
        if(chroma.size() == size && chroma[number_of_maxima_at_timestamp[j].first].value > max_value) {
          max_value = chroma[number_of_maxima_at_timestamp[j].first].value;
          timestamp_of_max_value = chroma[number_of_maxima_at_timestamp[j].first].time;
        }
        if(stft.size() == size && stft[number_of_maxima_at_timestamp[j].first].value > max_value) {
          max_value = stft[number_of_maxima_at_timestamp[j].first].value;
          timestamp_of_max_value = stft[number_of_maxima_at_timestamp[j].first].time;
        }
        if(rhythm.size() == size && rhythm[number_of_maxima_at_timestamp[j].first].value > max_value) {
          max_value = rhythm[number_of_maxima_at_timestamp[j].first].value;
          timestamp_of_max_value = rhythm[number_of_maxima_at_timestamp[j].first].time;
        }
        v_number_of_maxima_at_timestamp[j].push_back({timestamp_of_max_value, max_value});
      }
    }*/
    /*
    for(auto const & max: v_max) {
        std::cout << "at: " << max.time << " the maximum maxima value is " << max.value << std::endl;
    }
     */

    // END PEAK SELECT ALGORITHM



    // std::cout << "i: " << i << " // time: " << time << std::endl;


    for(int i = 0; i < size; i++){

        float time = 0.0;

        if (!mfcc.empty()) {
            time = mfcc[i].time;
        }
        if (!chroma.empty()) {
            time = chroma[i].time;
        }
        if (!stft.empty()) {
            time = stft[i].time;
        }
        if (!rhythm.empty()) {
            time = rhythm[i].time;
        }


        float value = 0.0;

        if(!mfcc.empty())
            value += mfcc[i].value * mfcc_co;
        if(!chroma.empty())
            value += chroma[i].value * chroma_co;
        if(!stft.empty())
            value += stft[i].value * stft_co;
        if(!rhythm.empty())
            value += rhythm[i].value * rhythm_co;

        value = sqrtf(value / combined_factor);

        combined_novelty_function.push_back({time, value});
    }

    return combined_novelty_function;
    //return v_max;

}

std::vector<time_value_float> Analysis::get_extrema(std::vector<time_value_float> novelty_function) {

    std::vector<time_value_float> extrema;

    bool first_extremum = true;

    for (int i = 1; i < novelty_function.size() - 1; i++) {

        if ((novelty_function[i - 1].value < novelty_function[i].value
        && novelty_function[i + 1].value < novelty_function[i].value)
        ||
        (novelty_function[i - 1].value > novelty_function[i].value
        && novelty_function[i + 1].value > novelty_function[i].value)
        ){
            // WE GOT A MAXIMUM
            if (first_extremum) {
                first_extremum = false;
            } else {
                extrema.push_back({novelty_function[i].time, (novelty_function[i].value)});
            }
        }
    }

    auto start_file_novelty = std::chrono::system_clock::now();
    // WHERE IS THIS?
    auto end_file_novelty = std::chrono::system_clock::now();

    // DELETE FIRST AND LAST EXTREMUM FROM EXTREMA
    extrema.pop_back();

    return extrema; // RETURN ONLY HIGHS!
}

float Analysis::get_middle_tvf(std::vector<time_value_float> v) {
    float v_m = 0;

    if (!v.empty()) {
        for (auto x:v) {
            v_m += x.value;
        }
        v_m /= (float) v.size();
    }

    return v_m;
}

float Analysis::get_variance_tvf(std::vector<time_value_float> v, float middle) {
    float v_var = 0;

    for (int i = 0; i < v.size(); i++) {
        v_var += pow((v[i].value - middle), 2.0);
    }
    if (!v.empty())
        v_var /= (float) v.size();

    return v_var;
}

float Analysis::get_standard_deviation_tvf(std::vector<time_value_float> v, float variance) {
    float v_dev = 0;
    if (variance > 0)
        v_dev = sqrt(variance);

    return v_dev;
}

std::vector<time_value_float>

Analysis::filter_extrema(std::vector<time_value_float> extrema, float middle, float middle_factor, float variance,
                         float standard_deviation, int bpm, bool filter_by_bars, int bars_c, float knn_upper_mean, float factor) {
    std::vector<time_value_float> segments;

    for (int i = 1; i < extrema.size()-1; i++) {
        if (extrema[i].value > extrema[i-1].value) {
            float threshold = ((extrema[i + 1].value + extrema[i - 1].value) / 2) + (knn_upper_mean * factor);
            if (extrema[i].value > threshold) {
                segments.push_back({extrema[i].time, extrema[i].value});
            }
        }
        else {}
    }



    // FILTER SEGMENTS THAT ARE LESS THAN "bars" APART
    if (filter_by_bars == true) {
        bool multiple_peaks = false;
        std::vector<int> indexes_of_multiple_peaks;

        float bars_time = (float) bars_c * ((float) 4.0 * 60.0 / (float) bpm);

        if (!segments.empty()) {
            for (int i = 0; i < segments.size() - 1; i++) {

                if ((abs(segments[i + 1].time - segments[i].time)) < bars_time * 4 && multiple_peaks) {
                    indexes_of_multiple_peaks.push_back(i);
                }

                if ((abs(segments[i + 1].time - segments[i].time)) < bars_time * 4 && !multiple_peaks) {
                    multiple_peaks = true;
                    indexes_of_multiple_peaks.push_back(i);
                }

                if (((abs(segments[i + 1].time - segments[i].time)) >= bars_time * 4 || i == segments.size() - 2) &&
                    multiple_peaks) {
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
                    indexes_of_multiple_peaks.erase(
                            std::remove(indexes_of_multiple_peaks.begin(), indexes_of_multiple_peaks.end(),
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
    }

    return segments;
}

void Analysis::make_csv_timeseries_tvf(std::vector<time_value_float> v, char const *directory, char const *filename) {
    std::string str_path;
    str_path += directory;
    str_path += filename;
    str_path += ".csv";
    const char *filepath = str_path.c_str();

    FILE *fp = std::fopen(filepath, "w");

    for (int i = 0; i < v.size(); i++) {
        fprintf(fp, "%f, %f\n", v[i].time, v[i].value);
    }
    fclose(fp);
    return;
}

void Analysis::make_csv_matrix_f(std::vector<std::vector<float>> v, char const *directory, char const *filename) {
    std::string str_path;
    str_path += directory;
    str_path += filename;
    str_path += ".csv";
    const char *filepath = str_path.c_str();

    FILE *fp = std::fopen(filepath, "w");

    for (int m = 0; m < v.size(); m++) {
        for (int n = 0; n < v.size(); n++) {
            fprintf(fp, "%f,", v[m][n]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return;
}

float mean(std::vector<float> v){

    float mean = 0;
    if (!v.empty()) {
        for (float x:v) {
            mean += x;
        }
        mean /= v.size();
        return mean;
    }
    else{
        return 0;
    }

}

float Analysis::knn_upper_mean(std::vector<time_value_float> values){

    // PSEUDO CODE
    std::vector<float> a;
    std::vector<float> b;

    float mean_a = 0;
    float mean_b = 0;
    bool threshold = false;
    float e = 0.1;

    while(!threshold){
        for (auto v:values){

            float delta_a = abs(mean(a) - v.value);
            float delta_b = abs(mean(b) - v.value);
            if (delta_a > delta_b || delta_a == delta_b){
                a.push_back(v.value);
            }
            else if(delta_a < delta_b){
                b.push_back(v.value);
            }
            else {
                // there SHOULD be no else... but NaN and stuff are creepy and unpredictable... :)
            }
            //std::cout << "mean_a: " << mean(a) << " // mean_b: " << mean(b) << std::endl;
            if (delta_a >= e && delta_b >= e){
                threshold = true;
            }
        }
        // TODO: DAS HIER MACHT NOCH KEINEN SINN!!!


        // MACH'S EINFACH 20 mal...?
    }

    if (mean(a) >= mean(b))
        return mean(a);
    else {
        return mean(b);
    }

}

std::vector<std::vector<float>> Analysis::smoothing_median(std::vector<std::vector<float>> &v, int L){
    std::vector<std::vector<float>> vs;
    std::vector<float> vm;

    vs.resize(v.size());

    for (int i = 0; i < v.size(); i++){
        vs[i].resize(v[i].size());
    }


    //es wird mit einem offset nach links und rechts über jeden iten wert JEDES feature vektors geschleift

    //        {v-1  v0  v1  v2  v3}
    // dim_0    0   1   2   3   0
    // dim_1    0   4   5   6   0
    // dim_2    0   7   8   9   0

    //

    // neu_0        1   2   2
    // neu_1        4   5   5
    // neu_2        7   8   8
    //
    // L = 3 // N = 1 MEDIAN

    // median_filter w über komplett dim_0 mit offset links und rechts also L = (N*2)+1
    int N = (L-1)/2;
    float median = 0;
    if (!v.empty()){
        for (int i = 0; i < v[0].size(); i++){
            for (int j = 0; j < v.size(); j++){

                for(int k = j-N; k <= j+N; k++){
                    if (k < 0 || k >= v.size()){
                        vm.push_back(0);
                    }
                    else {
                        vm.push_back(v[k][i]);
                    }
                }
                std::sort (vm.begin(), vm.end());

                int it = 0;
                if ((vm.size() % 2) > 0){
                    it = (int)((vm.size() - 1) / 2);
                    vs[j][i] = vm[it];
                }
                else {
                    it = vm.size() / 2;
                    vs[j][i] = ( (vm[it-1] + vm[it]) / 2);
                }
                vm.clear();
                vm.shrink_to_fit();
            }
        }
    }

    return vs;
};

float Analysis::get_average_energy_for_segment(float start, float end, float *vals, int sr, bool last){
    std::cout << "start: " << start << " // end: " << end << std::endl;
    // DO SIMPLE RMS SHIT

    float av_energy = 0;
    float energy;
    int sample_start = int(start * (float) sr);


    if (last) {
        std::vector<float> vals_nonzero;
        int zero_counter = 0;
        for (int i = signal_length_mono-1; i >= sample_start; i--) {
            //std::cout << vals[i] << std::endl;
            if (vals[i] < 0.01) {
                zero_counter++;
            } else {
                break;
            }
        }

        end = end - ((float) zero_counter / (float) samplerate);
        std::cout << "ZEROS: " << zero_counter << std::endl;
        std::cout << "end cut: " << end << std::endl;
    }

    int sample_end = int(end * (float) sr);
    int n = sample_end - sample_start;

    //TODO: check if sample calc is needed. this depends on vals

    for (int i = sample_start; i < sample_end; i++){

        // calc energy for sample here
        // (sum sample squares, take sqrt)

        energy = pow(vals[i], 2);

        av_energy += energy;

    }

    av_energy = sqrt(av_energy/n);

    return av_energy;
}

std::vector<time_value_float> Analysis::get_segments(std::vector<float> _custom_segments) {


    bool FILEPRINT = false; // print csv files for plotting?

    // TODO: PRINT MULTIPLE OUTPUTS WITH VARYING PARAMETERS FOR EVERY SONG!
    // CSV/MADSEN_KEINER/01/...
    char const *directory = "/Users/sd/CLionProjects/Sound-to-Light-2.0/CSV/"; // DIRECTORY FOR FILEPRINTS

    // TEMPORAL ALIGNMENT AND TIME-DISTANCE BASED THRESHOLDING OF EXTREMA
    bool filter_by_bars = true; // filter extrema that are too close?
    int bars_c = 1; // left/right distance for filtering extrema that are too close // TODO: EIGENTLICH 1
    float bars_bin = 1; // width of audio bin in bars
    int filter_bars = 2; // filter kernel width in bars
    float cut_seconds_end = 2.5; // HOW MANY SECONDS TO CUT AT THE END

    // KNN ALGORITHM FOR EXTREMA THRESHOLDING
    float knn_factor = 0.5;
    float norm_threshold = 0.00001;

    // MEDIAN SMOOTHING
    bool smooth_median = true;
    int smoothing_filter_length = 5;

    // NOVELTY COMBINATION WITH WEIGHTS (ALSO ON OFF SWITCH, =0 for OFF, >0 for ON)
    // youreallygotme 8,1,4
    // madsen 2/1/4
    float mfcc_factor = 8; // 1
    float chroma_factor = 1; // 0.5
    float stft_factor = 4; // 4
    float rhythm_factor = 0; // 0

    int offset = get_first_beat();

    // TODO: REVERT!!!!
    // TODO: DON'T GO OVER samplerate/2 BECAUSE OF CHROMAGRAM... DUH...
    float bpm_bin = bars_bin * (60.0 / this->bpm) * this->samplerate;
    while (bpm_bin > 44100) {
      bpm_bin /= 2;
    }

    std::cout << "Sampling Frequency: " << bpm_bin / this->samplerate << " // BPM based bin size: " << bpm_bin
              << std::endl;
    std::cout << "New bin size: " << bpm_bin << std::endl;

    auto start_segmentation = std::chrono::system_clock::now();

    int bin_size = (int) bpm_bin; // 44100 = 1 second, 1Hz
    std::cout << "bin_size: " << bin_size << std::endl;
    int hop_size = (int) bin_size; // no overlap, don't do overlap, all timestamps will be broken.

    // MIDDLE THRESHOLDING (CURRENTLY OFF)
    float middle_factor = 1; // 1
    float novelty_factor = 0.5;
    float deviation_factor = 4;
    float cut_seconds_start = 0;

    std::vector<time_value_float> segments;


  if(_custom_segments.empty()) {
    // weighting for influence on novelty function

    // ###################################
    // ###### 1. GET AUDIO FEATURES ######
    // ####### CURRENTLY MFCC ONLY #######
    // ###################################

    auto start_features = std::chrono::system_clock::now();

    std::vector<std::vector<float>> window_mfcc;
    std::vector<std::vector<float>> window_chroma;
    std::vector<std::vector<float>> window_stft;
    std::vector<std::vector<float>> window_rhythm;

    if (mfcc_factor > 0) {
      window_mfcc = get_stmfcc(wav_values_mono, bin_size, hop_size, offset, 4, 14);
      window_mfcc = norm_euclidian(window_mfcc, norm_threshold);
      if (smooth_median)
        window_mfcc = smoothing_median(window_mfcc, smoothing_filter_length);
    }
    if (chroma_factor > 0) {
      window_chroma = get_chromagram(wav_values_mono, bin_size, hop_size, samplerate, offset);
      window_chroma = norm_euclidian(window_chroma, norm_threshold);
      if (smooth_median)
        window_chroma = smoothing_median(window_chroma, smoothing_filter_length);
    }
    if (stft_factor > 0) {
      window_stft = get_spectrogram(wav_values_mono, bin_size, hop_size, offset);
      window_stft = norm_euclidian(window_stft, norm_threshold);
      if (smooth_median)
        window_stft = smoothing_median(window_stft, smoothing_filter_length);
    }

    if (rhythm_factor > 0) {
      window_rhythm = get_rhythmogram(wav_values_mono, bin_size, hop_size, offset);
      window_rhythm = norm_euclidian(window_rhythm, norm_threshold);
    }
    auto end_features = std::chrono::system_clock::now();

    // #################################
    // ########## 3. MAKE SSM ##########
    // #################################

    auto start_ssm = std::chrono::system_clock::now();

    std::vector<std::vector<float>> ssm_mfcc;
    std::vector<std::vector<float>> ssm_chroma;
    std::vector<std::vector<float>> ssm_stft;
    std::vector<std::vector<float>> ssm_rhythm;

    //auto start_ssm1 = std::chrono::system_clock::now();
    if (mfcc_factor > 0) {
      //std::cout << "getting ssm for mfcc" << std::endl;
      //std::cout << "window_mfcc.size(): " << window_mfcc.size() << std::endl;
      //std::cout << "window_mfcc[0].size(): " << window_mfcc[0].size() << std::endl;
      //std::cout << "window_mfcc[0][0]: " << window_mfcc[0][0] << std::endl;
      ssm_mfcc = get_self_similarity_matrix(window_mfcc, 0);
    }

    //auto end_ssm1 = std::chrono::system_clock::now();

    //auto start_ssm2 = std::chrono::system_clock::now();
    if (chroma_factor > 0) {
      //std::cout << "getting ssm for chroma" << std::endl;
      //std::cout << "window_chroma.size(): " << window_chroma.size() << std::endl;
      //std::cout << "window_chroma[0].size(): " << window_chroma[0].size() << std::endl;
      //std::cout << "window_chroma[0][0]: " << window_chroma[0][0] << std::endl;
      ssm_chroma = get_self_similarity_matrix(window_chroma, 0);
    }

    //auto end_ssm2 = std::chrono::system_clock::now();

    //auto start_ssm3 = std::chrono::system_clock::now();
    if (stft_factor > 0) {
      //std::cout << "getting ssm for stft" << std::endl;
      //std::cout << "window_stft.size(): " << window_stft.size() << std::endl;
      //std::cout << "window_stft[0].size(): " << window_stft[0].size() << std::endl;
      //std::cout << "window_stft[0][0]: " << window_stft[0][0] << std::endl;
      ssm_stft = get_self_similarity_matrix(window_stft, 0);
    }

    //auto end_ssm3 = std::chrono::system_clock::now();

    //auto start_ssm4 = std::chrono::system_clock::now();
    if (rhythm_factor > 0) {
      //std::cout << "getting ssm for rhythm" << std::endl;
      ssm_rhythm = get_self_similarity_matrix(window_rhythm, 0);
    }

    //auto end_ssm4 = std::chrono::system_clock::now();

    auto end_ssm = std::chrono::system_clock::now();

    // ####################################################
    // ## 5.1 CREATE RADIAL GAUSSIAN CHECKERBOARD KERNEL ##
    // ####################################################

    auto start_kernel = std::chrono::system_clock::now();

    std::vector<std::vector<float>> kernel = get_filter_kernel(this->bpm, bin_size, samplerate, filter_bars);

    auto end_kernel = std::chrono::system_clock::now();

    std::cout << "kernel durch" << std::endl;


    // ###################################
    // ### 5.1 CREATE NOVELTY FUNCTION ###
    // ###################################

    auto start_filter = std::chrono::system_clock::now();

    std::vector<time_value_float> novelty_function_mfcc;
    std::vector<time_value_float> novelty_function_chroma;
    std::vector<time_value_float> novelty_function_stft;
    std::vector<time_value_float> novelty_function_rhythm;

    if (mfcc_factor > 0)
      novelty_function_mfcc = get_novelty_function(ssm_mfcc, kernel, cut_seconds_end, bin_size,
                                                   samplerate, kernel[0].size(), offset);
    if (chroma_factor > 0)
      novelty_function_chroma = get_novelty_function(ssm_chroma, kernel, cut_seconds_end, bin_size,
                                                     samplerate, kernel[0].size(), offset);
    if (stft_factor > 0)
      novelty_function_stft = get_novelty_function(ssm_stft, kernel, cut_seconds_end, bin_size,
                                                   samplerate, kernel[0].size(), offset);
    if (rhythm_factor > 0)
      novelty_function_rhythm = get_novelty_function(ssm_rhythm, kernel, cut_seconds_end, bin_size,
                                                     samplerate, kernel[0].size(), offset);

    std::vector<time_value_float> novelty_function_combined = get_combined_novelty_function(novelty_function_mfcc,
                                                                                            novelty_function_chroma,
                                                                                            novelty_function_stft,
                                                                                            novelty_function_rhythm,
                                                                                            mfcc_factor, chroma_factor,
                                                                                            stft_factor, rhythm_factor,
                                                                                            FILEPRINT, directory);

    auto end_filter = std::chrono::system_clock::now();

    std::cout << "get_novelty alle durch" << std::endl;

    // ########################
    // ### 5.2 FIND EXTREMA ###
    // ########################

    auto start_extrema = std::chrono::system_clock::now();

    std::vector<time_value_float> extrema = get_extrema(novelty_function_combined);
    //std::vector<time_value_float> extrema = novelty_function_combined;

    float novelty_middle = get_middle_tvf(novelty_function_combined);
    float novelty_variance = get_variance_tvf(novelty_function_combined, novelty_middle);
    float novelty_deviation = get_variance_tvf(novelty_function_combined, novelty_variance);

    float extrema_middle = get_middle_tvf(extrema);
    float extrema_variance = get_variance_tvf(extrema, extrema_middle);
    float extrema_deviation = get_standard_deviation_tvf(extrema, extrema_variance);

    float novelty_knn_upper_mean = knn_upper_mean(novelty_function_combined);

    segments = filter_extrema(extrema,
                                                            extrema_middle,
                                                            middle_factor,
                                                            extrema_variance,

        /*extrema_deviation, this->bpm, filter_by_bars, bars_c);

        auto comp1 = [] ( const time_value_float& tvf1, const time_value_float& tvf2 ) {return tvf1.time == tvf2.time;};
        auto last = std::unique(segments.begin(), segments.end(),comp1);
        segments.erase(last, segments.end());*/


        //segments.erase( unique( segments.begin(), segments.end() ), segments.end() );
                                                            extrema_deviation,
                                                            this->bpm,
                                                            filter_by_bars,
                                                            bars_c,
                                                            novelty_knn_upper_mean,
                                                            knn_factor);
    auto end_extrema = std::chrono::system_clock::now();


    // ######################################
    // ### 5.3. GET AVERAGE SEGMENT ENERGY ##
    // ######################################


  } else {
    for(int i = 0; i < _custom_segments.size(); i++)
      if(_custom_segments[i] > 0)
        segments.push_back({_custom_segments[i], 1});
  }


    // for foo fighters - the pretender
//    segments.clear();
//    segments.shrink_to_fit();
//    segments.push_back({32, 1});
//    segments.push_back({58.5, 1});
//    segments.push_back({70, 1});
//    segments.push_back({83.5, 1});
//    segments.push_back({106, 1});
//    segments.push_back({120, 1});
//    segments.push_back({133.5, 1});
//    segments.push_back({159, 1});
//    segments.push_back({197.6, 1});
//    segments.push_back({208.3, 1});
//    segments.push_back({255.5, 1});


  // for Rammstein - Pussy
//  segments.clear();
//  segments.shrink_to_fit();
//  segments.push_back({7, 1});
//  segments.push_back({35.8, 1});
//  segments.push_back({50, 1});
//  segments.push_back({64, 1});
//  segments.push_back({71, 1});
//  segments.push_back({99.5, 1});
//  segments.push_back({114, 1});
//  segments.push_back({128, 1});
//  segments.push_back({135.2, 1});
//  segments.push_back({163.5, 1});
//  segments.push_back({177.8, 1});
//  segments.push_back({192.8, 1});
//  segments.push_back({206, 1});


  // for Red Hot Chilli Peppers - Californication
//  segments.clear();
//  segments.shrink_to_fit();
//  segments.push_back({64, 1});
//  segments.push_back({93.5, 1});
//  segments.push_back({103, 1});
//  segments.push_back({153, 1});
//  segments.push_back({182.8, 1});
//  segments.push_back({202.6, 1});
//  segments.push_back({242.3, 1});
//  segments.push_back({267, 1});
//  segments.push_back({296, 1});


  // for Basshunter - All I Ever Wanted
//  segments.clear();
//  segments.shrink_to_fit();
//  segments.push_back({22, 1});
//  segments.push_back({35, 1});
//  segments.push_back({49, 1});
//  segments.push_back({57, 1});
//  segments.push_back({83.5, 1});
//  segments.push_back({115, 1});
//  segments.push_back({141.5, 1});
//  segments.push_back({149.8, 1});
//  segments.push_back({176, 1});





    float seg_start;
    float seg_end;
    std::vector<time_value_float> energy_segments;
    for (int i = 0; i <= segments.size(); i++){
        if (i == 0){
            seg_start = 0;
            seg_end = segments[i].time;
            energy_segments.push_back({0.0 ,get_average_energy_for_segment(seg_start, seg_end, wav_values_mono, samplerate, false)});
        }
        else if (i == segments.size()){
            seg_start = segments[i-1].time;
            seg_end = (float) ((signal_length_mono) / samplerate);
            energy_segments.push_back({segments[i-1].time ,get_average_energy_for_segment(seg_start, seg_end, wav_values_mono, samplerate, true)});
        }
        else {
            seg_start = segments[i-1].time;
            seg_end = segments[i].time;
            energy_segments.push_back({segments[i-1].time ,get_average_energy_for_segment(seg_start, seg_end, wav_values_mono, samplerate, false)});
        }


    }

    float max_energy = 0;
    for (int i = 0; i < energy_segments.size(); i++){
        if (energy_segments[i].value > max_energy){
            max_energy = energy_segments[i].value;
        }
    }
    float min_energy = INFINITY;
    for (int i = 0; i < energy_segments.size(); i++){
        if (energy_segments[i].value < min_energy && energy_segments[i].value != 0){
            min_energy = energy_segments[i].value;
        }
    }

    for (int i = 0; i < energy_segments.size(); i++){
        energy_segments[i].value = ((energy_segments[i].value - min_energy) / (max_energy - min_energy) );
    }

    // #######################
    // ### 6. PRINT TO CSV ###
    // #######################

//    if (FILEPRINT == true) {
//        make_csv_matrix_f(kernel, directory, "kernel");
//        if (mfcc_factor > 0)
//            make_csv_matrix_f(ssm_mfcc, directory, "ssm_mfcc");
//        if (chroma_factor > 0)
//            make_csv_matrix_f(ssm_chroma, directory, "ssm_chroma");
//        if (stft_factor > 0)
//            make_csv_matrix_f(ssm_stft, directory, "ssm_stft");
//        if (rhythm_factor > 0)
//            make_csv_matrix_f(ssm_rhythm, directory, "ssm_rhythm");
//        //make_csv_timeseries_tvf(novelty_function_mfcc, directory, "novelty_function_mfcc");
//        //make_csv_timeseries_tvf(novelty_function_chroma, directory, "novelty_function_chroma");
//        //make_csv_timeseries_tvf(novelty_function_stft, directory, "novelty_function_stft");
//        //make_csv_timeseries_tvf(novelty_function_rhythm, directory, "novelty_function_rhythm");
//
//        make_csv_timeseries_tvf(novelty_function_combined, directory, "novelty_function_combined");
//        make_csv_timeseries_tvf(extrema, directory, "extrema");
//        make_csv_timeseries_tvf(segments, directory, "segments");
//        make_csv_timeseries_tvf(energy_segments, directory, "energy_segments");
//    }


    // ################################
    // ### TODO: 7. CLUSTER SEGMENTS ##
    // ################################


    // #################################
    // ### 8. PRINT CALCULATION TIMES ##
    // #################################

//    auto end_segmentation = std::chrono::system_clock::now();
//    std::chrono::duration<double> elapsed_seconds = end_segmentation - start_segmentation;
//    std::chrono::duration<double> elapsed_seconds_features = end_features - start_features;
//    std::chrono::duration<double> elapsed_seconds_ssm = end_ssm - start_ssm;
//    //std::chrono::duration<double> elapsed_seconds_ssm1 = end_ssm1 - start_ssm1;
//    //std::chrono::duration<double> elapsed_seconds_ssm2 = end_ssm2 - start_ssm2;
//    //std::chrono::duration<double> elapsed_seconds_ssm3 = end_ssm3 - start_ssm3;
//    //std::chrono::duration<double> elapsed_seconds_ssm4 = end_ssm4 - start_ssm4;
//    std::chrono::duration<double> elapsed_seconds_kernel = end_kernel - start_kernel;
//    std::chrono::duration<double> elapsed_seconds_filter = end_filter - start_filter;
//    std::chrono::duration<double> elapsed_seconds_extrema = end_extrema - start_extrema;

//    std::time_t end_time = std::chrono::system_clock::to_time_t(end_segmentation);
//
//    std::cout << "finished segmentation at " << std::ctime(&end_time)
//              << "elapsed time: " << elapsed_seconds.count() << "s\n"
//              << "FEATURE EXTRACTION: " << elapsed_seconds_features.count() << "s\n"
//              << "SSM CALCULATION: " << elapsed_seconds_ssm.count() << "s\n"
//              //<< "SSM CALCULATION1: " << elapsed_seconds_ssm1.count() << "s\n"
//              //<< "SSM CALCULATION2: " << elapsed_seconds_ssm2.count() << "s\n"
//              //<< "SSM CALCULATION3: " << elapsed_seconds_ssm3.count() << "s\n"
//              //<< "SSM CALCULATION4: " << elapsed_seconds_ssm4.count() << "s\n"
//              << "KERNEL CALCULATION: " << elapsed_seconds_kernel.count() << "s\n"
//              << "NOVELTY CALCULATION: " << elapsed_seconds_filter.count() << "s\n"
//              << "SEGMENTS CALCULATION: " << elapsed_seconds_extrema.count() << "s\n";

    // PRINT SEGMENTS TO CONSOLE FOR EASY AND FAST CHECKING
    //std::cout << "--- SEGMENTS FOUND ---" << std::endl;
    /*
    for (auto tvf:segments) {
        std::cout << "time: " << tvf.time << std::endl;
    }
    */


    // PRINT ALL THE COOL STUFF
    // PRINT ALL PARAMETERS
//    if (FILEPRINT){
//
//        std::string str_path;
//        str_path += directory;
//        str_path += "params.csv";
//        const char *filepath = str_path.c_str();
//
//        FILE *fp = std::fopen(filepath, "w");
//
//        fprintf(fp, "\n\n### GENERAL - SIGNAL INFORMATION ###\n");
//        fprintf(fp, "SIGNAL LENGTH: %d SAMPLES\n", sizeof(wav_values_mono));
//        fprintf(fp, "SAMPLE RATE: %d SAMPLES PER SECOND\n", samplerate);
//
//        fprintf(fp, "\n\n### GENERAL - SET PARAMATERS ###\n");
//        fprintf(fp, "KERNEL BARS: %d BARS\n", filter_bars);
//        fprintf(fp, "BIN BARS: %d BARS\n", bars_bin);
//        fprintf(fp, "CUT SECONDS (END): %f s\n", cut_seconds_end);
//        if (filter_by_bars)
//            fprintf(fp, "FILTER BY BARS: %d BARS\n", bars_c);
//        else
//            fprintf(fp, "FILTER BY BARS: FALSE\n");
//
//        fprintf(fp, "\n\n### FEATURE EXTRACTION - CALCULATED PARAMETERS ###\n");
//        fprintf(fp, "BPM: %d BEATS/m\n", this->bpm);
//        fprintf(fp, "FIRST BEAT (OFFSET): %d samples\n", offset);
//        fprintf(fp, "BIN SIZE: %f SAMPLES\n", bpm_bin);
//        fprintf(fp, "HOP SIZE: %d SAMPLES\n", hop_size);
//        fprintf(fp, "FEATURE SAMPLING FREQUENCY: %f Hz\n", (bpm_bin / samplerate));
//
//        fprintf(fp, "\n\n### NOVELTY — SET PARAMETERS ###\n");
//        fprintf(fp, "NOVELTY WEIGHTING FACTOR (MFCC): %f \n", mfcc_factor);
//        fprintf(fp, "NOVELTY WEIGHTING FACTOR (CHROMA): %f \n", chroma_factor);
//        fprintf(fp, "NOVELTY WEIGHTING FACTOR (STFT): %f \n", stft_factor);
//        fprintf(fp, "NOVELTY WEIGHTING FACTOR (RHYTHM): %f \n", rhythm_factor);
//
//        fprintf(fp, "\n\n### NOVELTY — CALCULATED PARAMETERS ###\n");
//        fprintf(fp, "ARITHMETIC MIDDLE (NOVELTY): %f \n", novelty_middle);
//        fprintf(fp, "VARIANCE (NOVELTY): %f \n", novelty_variance);
//        fprintf(fp, "STANDARD DEVIATION (NOVELTY): %f \n", novelty_deviation);
//        fprintf(fp, "ARITHMETIC MIDDLE (EXTREMA): %f \n", extrema_middle);
//        fprintf(fp, "VARIANCE (EXTREMA): %f \n", extrema_variance);
//        fprintf(fp, "STANDARD DEVIATION (EXTREMA): %f \n", extrema_deviation);
//        //fprintf(fp, "KNN - BIGGEST SET (NOVELTY): %f \n", novelty_knn_big_middle);
//        //fprintf(fp, "KNN SMALLEST SET (NOVELTY): %f \n", novelty_knn_small_set);
//
//        fprintf(fp, "\n\n### ELAPSED TIME ###\n");
//        fprintf(fp, "SEGMENTATION OVERALL: %f s\n", elapsed_seconds.count());
//        fprintf(fp, "FEATURE EXTRACTION: %f s\n", elapsed_seconds_features.count());
//        fprintf(fp, "SELF SIMILARITY CALCULATION: %f s\n", elapsed_seconds_ssm.count());
//        fprintf(fp, "KERNEL CALCULATION: %f s\n", elapsed_seconds_kernel.count());
//        fprintf(fp, "NOVELTY CALCULATION: %f s\n", elapsed_seconds_filter.count());
//        fprintf(fp, "EXTREMA CALCULATION AND FILTERING: %f s\n", elapsed_seconds_extrema.count());
//
//        fclose(fp);
//    }

    // FINALLY RETURN SEGMENTS OR ENERGY SEGMENTS
    //return segments;
    return energy_segments;


}

std::vector<time_value_int>
Analysis::get_intensity_average_for_next_segment(std::vector<double> beats, int beats_per_minute,
                                                 double first_good_beat) {

    int block_size = (60 * 4 * samplerate) / beats_per_minute;
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
            /*
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
    this->bpm = finalBPM;

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

    this->bpm = beats_per_minute;
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

void Analysis::set_bpm(int _bpm) {
    this->bpm = _bpm;
}


double Analysis::generate_seed_for_song() {

  int middle_of_song = this->signal_length_mono / 2;
  int seed_area_size = 2048;
//  std::cout << "signal_length_mono: " << signal_length_mono << std::endl;

  double seed = 0;
  for(int i = middle_of_song; i < middle_of_song + seed_area_size - 1; i++) {
//    std::cout << "hi " << this->wav_values_mono[i] * this->wav_values_mono[i] << std::endl;
    seed += this->wav_values_mono[i] * this->wav_values_mono[i];
  }
//  std::cout << "seed after adding: " << seed << std::endl;
  seed = sqrt(seed);
//  std::cout << "seed after sqrt: " << seed << std::endl;

  return seed;
}