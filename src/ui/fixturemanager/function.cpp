#include "function.h"

Function::Function(QString data) {
  QStringList splitted_data = data.split("~");
  name = splitted_data.at(0);
  start_value = splitted_data.at(1).toInt();
  end_value = splitted_data.at(2).toInt();
}

QString Function::to_string() {
  return name + "~" + QString::number(start_value) + "~" + QString::number(end_value);
}

QString Function::to_string_view() {
    return "" + name + ((name.size() > 16)? "\t": "\t\t") + QString::number(start_value) + "-" + QString::number(end_value);
}

bool Function::is_gloabl_dimmer()
{
 return name.contains("master dimmer", Qt::CaseInsensitive);
}
