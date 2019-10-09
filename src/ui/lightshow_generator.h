//
// Created by Jan on 06.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_GENERATOR_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_GENERATOR_H_

#include <memory>
#include "lightshow/analysis_result.h"
#include "lightshow/lightshow_fixture.h"
#include "lightshow/generated_light_show.h"
#include "lightshow/lightshow.h"
#include <song.h>
#include <fixturemanager/fixture.h>

class LightshowGenerator {
 public:
  LightshowGenerator();
  static std::shared_ptr<Lightshow> generate(int resolution, Song *song, std::list<Fixture> fixtures);
 private:
  std::shared_ptr<AnalysisResult> analysis_result;
  //std::vector<LightshowFixture> fixtures;
};

#endif //RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_GENERATOR_H_
