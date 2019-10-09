//
// Created by Jan on 06.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_ANALYSER_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_ANALYSER_H_

#include <string>
#include <memory>
#include "analysis_result.h"

class SongAnalyser {
 public:
  SongAnalyser(const std::string &song_path);
  void analyse(AnalysisResult &analysis_result);
 private:
  std::string songPath;
};

#endif //RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_ANALYSER_H_
