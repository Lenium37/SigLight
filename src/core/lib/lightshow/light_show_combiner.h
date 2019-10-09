//
// Created by Jan on 06.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_COMBINER_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_COMBINER_H_

#include <memory>
#include "generated_light_show.h"
#include "edit_result.h"

class LightShowCombiner {
 public:
  LightShowCombiner(const GeneratedLightShow &generated_light_show, const std::vector<EditResult> &edit_results);
 private:
  const GeneratedLightShow &generated_light_show;
  const std::vector<EditResult> &edit_results;
};

#endif //RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_LIGHT_SHOW_COMBINER_H_
