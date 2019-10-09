#include "channel_gui.h"

Channel_gui::Channel_gui(QString function_list) {
  QStringList functions_data = function_list.split("|");
  for (int i = 0; i < functions_data.size(); i++) {
    functions.push_back(Function(functions_data.at(i)));
  }
}

QString Channel_gui::get_functions() {
  QString functions_text = "";
  for (int i = 0; i < functions.size(); i++) {
    if (!functions_text.isEmpty()) {
      functions_text.append("|");
    }
    functions_text.append((*std::next(functions.begin(), i)).to_string_view());
  }
  return functions_text;
}

string Channel_gui::to_string() {
  string functions_text = "";
  for (int i = 0; i < functions.size(); i++) {
    if (!functions_text.empty()) {
      functions_text.append("|");
    }
    functions_text.append((*std::next(functions.begin(), i)).to_string().toStdString());
  }
  return functions_text;
}

list<Function> Channel_gui::get_function_list()
{
    return functions;
}

bool Channel_gui::check_for_global_dimmer()
{
    bool has_global_dimmer = false;
    for(Function function : functions) {
        if(function.is_gloabl_dimmer() == true) {
            has_global_dimmer = true;
            break;
        }
    }
    return has_global_dimmer;
}
