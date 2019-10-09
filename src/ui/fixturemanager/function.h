#ifndef FUNCTION_H
#define FUNCTION_H
#include <QString>
#include <QStringList>
#include <string>

using namespace std;

class Function {
 public:
  Function(QString data);
  ~Function() {}
  QString to_string();
  QString to_string_view();
  bool is_gloabl_dimmer();

 private:
  /**
   * @brief name Name of the function. Also contains the way it works.
   */
  QString name;
  /**
   * @brief start_value Startvalue of the function.
   */
  int start_value;
  /**
   * @brief end_value Endvalue of the function.
   */
  int end_value;
};

#endif // FUNCTION_H
