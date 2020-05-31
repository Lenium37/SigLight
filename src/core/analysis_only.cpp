//
// Created by Steven Drewers on 19.01.20.
//

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <iostream>
#include <math.h>
#include <libusb.h>

#include <array>

#include <fftw3.h>
#include <sndfile.hh>
#include <logger.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <file_system_utils.h>
#include <lightshow/lightshow.h>
#include <filesystem>
#include <lightshow/analysis.h>

// copy behaviour of "SONG ADDED"!

int main() {
    int resolution = 40;

    std::string song = "/Users/sd/CLionProjects/Sound-to-Light-2.0/assets/doku/the_kinks_you_really_got_me.wav";

    std::shared_ptr<Lightshow> lightshow = std::make_shared<Lightshow>();
    lightshow->set_resolution(resolution);

    lightshow->set_sound_src(song);
    //lightshow->prepare_analysis_for_song((char*)file_path.c_str());
    lightshow->prepare_analysis_for_song((char*)song.c_str(), false, false, false, false, 0, 9, 30, 65, 160, 150, 300, 175);

    system("python /Users/sd/CLionProjects/Sound-to-Light-2.0/CSV/plot_novelty.py --kernel --mfcc --chroma --stft");
    // --kernel --mfcc --chroma --stft --rhythm --energybars

    return 0;
}