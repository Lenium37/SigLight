//
// Created by Jan on 06.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_EDIT_RESULT_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_EDIT_RESULT_H_

#include <string>
struct EditResult {

 public:
  enum class ActionType {
    COLOR_CHANGE // etc. add what you need
  };
  EditResult(float start,
             float an_end,
             EditResult::ActionType action_type,
             std::string fixture_name);
  float start, end;
  ActionType action_type;
  std::string fixture_name;
};

#endif //RASPITOLIGHT_SRC_CORE_LIB_LIGHTSHOW_EDIT_RESULT_H_
