//
// Created by Jan on 06.06.2019.
//

#include "light_show_combiner.h"
LightShowCombiner::LightShowCombiner(const GeneratedLightShow &generated_light_show,
                                     const std::vector<EditResult> &edit_results) : generated_light_show(
    generated_light_show), edit_results(edit_results) {}
