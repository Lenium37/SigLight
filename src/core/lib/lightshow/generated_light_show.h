//
// Created by Jan on 06.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_GENERATED_LIGHT_SHOW_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_GENERATED_LIGHT_SHOW_H_

#include <vector>
#include <cstdint>
#include <string>
#include <map>

typedef std::vector<std::vector<std::uint8_t>> FixtureChannelValues;

class GeneratedLightShow {
 public:
  std::map<std::string, FixtureChannelValues> all_fixture_values;
};

#endif //RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_GENERATED_LIGHT_SHOW_H_
