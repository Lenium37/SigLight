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

#include <array>

#include <fftw3.h>
#include <sndfile.hh>
#include <logger.h>
#include <file_system_utils.h>
#include <lightshow/lightshow.h>

// copy beahaviour of "SONG ADDED"!

int main() {

    int resolution = 40;
    std::string file_path = "/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/assets/test_drumloop.wav";

    std::shared_ptr<Lightshow> lightshow_from_analysis = std::make_shared<Lightshow>();
    lightshow_from_analysis->set_resolution(resolution);

    lightshow_from_analysis->set_sound_src(file_path);
    lightshow_from_analysis->prepare_analysis_for_song((char*)file_path.c_str());


    //system("python /Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/plot.py");
    //system("python /Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/plot_av.py");

    return 0;
}