#ifndef CHANNEL_H
#define CHANNEL_H
#include <fixturemanager/function.h>
#include <list>
#include <QString>
#include <QStringList>

using namespace std;

class Channel_gui {
 public:
  Channel_gui(QString fixture_list);
  ~Channel_gui() {}

  /**
   * @brief get_functions
   * @return A List of the Functions.
   */
  QString get_functions();
  void add_function();
  string to_string();
  list<Function> get_function_list();
  bool check_for_global_dimmer();
 private:
  list<Function> functions;
};

#endif // CHANNEL_H
