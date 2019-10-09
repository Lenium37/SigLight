//
// Created by Jan on 06.06.2019.
//

#include "edit_result.h"
EditResult::EditResult(float start,
                       float an_end,
                       EditResult::ActionType action_type,
                       std::string fixture_name)
    : start(start), end(an_end), action_type(action_type), fixture_name(fixture_name) {}
