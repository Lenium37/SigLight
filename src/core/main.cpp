#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <iostream>
#include <math.h>

#include <array>

#include <fftw3.h>
#include <sndfile.hh>
#include <logger.h>
#include <file_system_utils.h>

using namespace std;


int signalLaenge;
float *werte;
int hopSize;
int windowSize;
int sampleRate;

void wavAuslesen();
void STFT(float *,int,int,int);
FILE *out = fopen("../../analysis.txt","w");

int main() {
    fmt::format("test: {}", 1);
    Logger::info("test: {}", 42);
    Logger::debug("We should not see this");
    Logger::set_level(Logger::LevelEnum::debug);
    Logger::debug("We should see this");

  Logger::info(FileSystemUtils::get_file_name(R"(M:\workspace\19ss-raspitolight1\assets\Crazy-Frog-Axel-F.wav)"));
  Logger::info(FileSystemUtils::get_file_name(std::string(
      R"(M:\workspace\19ss-raspitolight1\assets\Crazy-Frog-Axel-F.wav)")));

  Logger::info(FileSystemUtils::get_file_name(R"(M:\workspace\19ss-raspitolight1\assets\Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav)"));
  Logger::info(FileSystemUtils::get_file_name(std::string(
      R"(M:\workspace\19ss-raspitolight1\assets\Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav)")));

    wavAuslesen();
    STFT(werte,signalLaenge,windowSize=1024,hopSize=1024);

    return 0;
}


void wavAuslesen(){

    SNDFILE *sf;
    SF_INFO info;
    int f,c;

    info.format = 0;																// Datei ˆffnen
    sf = sf_open("../../../assets/Sinus_f12_256.wav",SFM_READ,&info);
    if (sf == nullptr)
    {
        printf("Fehler\n");
        exit(-1);
    }

    f = info.frames;
    c = info.channels;
    sampleRate = info.samplerate;
    signalLaenge = f*c;

    werte = (float *) malloc(signalLaenge*sizeof(float));							// Speicherplatz freigeben
    sf_readf_float(sf,werte,signalLaenge);											// Werte von der Datei "sf" in ein Array namens "werte" speichern
    sf_close(sf);

}

void hamming(int windowLength, float *buffer) {

    for(int i = 0; i < windowLength; i++) {
        buffer[i] = 0.54 - (0.46 * cos( 2 * M_PI * (i / ((windowLength - 1) * 1.0))));
    }
}

void STFT(float *signal, int signalLength, int windowSize, int hopSize) {

    fftw_complex    *data, *fft_result, *ifft_result;
    fftw_plan       plan_forward, plan_backward;
    int             i;

    data        = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
    fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );
    ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * windowSize );

    plan_forward  = fftw_plan_dft_1d( windowSize, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

    // Create a hamming window of appropriate length
    float window[windowSize];
    hamming(windowSize, window);

    int chunkPosition = 0;

    int readIndex;

    // Should we stop reading in chunks?
    int bStop = 0;
    int numChunks = 0;

    // Process each chunk of the signal
    while(chunkPosition < signalLength && !bStop) {

        // Copy the chunk into our buffer
        for(i = 0; i < windowSize; i++) {

            readIndex = chunkPosition + i;

            if(readIndex < signalLength) {
                // Note the windowing!
                data[i][0] = signal[readIndex] * window[i];
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

        std::vector<float>result(windowSize/2);
        double max=0;
        int sample = 0;

        for (i = 0; i < windowSize/2 + 1; i++) {
            result[i] = sqrt(fft_result[i][0]*fft_result[i][0]+fft_result[i][1]*fft_result[i][1]);
            if (result[i] > max) {
                max = result[i];
                sample = i;
                //printf("sample: %d, result: %f\n", sample, max);
            }
        }

        double time_from = (double) ((double)( (numChunks) * (hopSize) ) / sampleRate) / 2;
        double time_to = (double) ((double) (((double) hopSize) + ((numChunks) * (hopSize))) / sampleRate) / 2;
        double freq_steps = (double)  ((double) sampleRate / 2) / ( (double) windowSize / 2);
        double freq_from = (double) (sample) * freq_steps ;
        double freq_to = (double) (sample + 1) * (freq_steps);

        printf("timerange: %f - %f // freqrange: %5.10f - %5.10f\n", time_from, time_to, freq_from, freq_to);

        chunkPosition += hopSize;
        numChunks++;
    }

    fftw_destroy_plan( plan_forward );
    fftw_free( data );
    fftw_free( fft_result );
    fftw_free( ifft_result );
}
