#ifndef EDIT_FIXTURE_DIALOG_H
#define EDIT_FIXTURE_DIALOG_H

#include <QDialog>
#include "fixturemanager/fixture.h"
#include <QMessageBox>

namespace Ui {
class EditFixtureDialog;
}

class EditFixtureDialog : public QDialog {
  Q_OBJECT

 public:
  explicit EditFixtureDialog(QWidget *parent = nullptr);

  explicit EditFixtureDialog(QWidget *parent = nullptr, list<Fixture> &fixtures = *new list<Fixture>(), std::vector<std::string> color_palettes = *new std::vector<std::string>());

  ~EditFixtureDialog() override;

  /**
  * @brief set_up_dialog_options Sets the blocked channels.
  * @param blocked_channels Channels that are allready occupied from other fixtures.
  */
  void set_up_dialog_options(std::list<int> _blocked_channels, std::string _own_channels, std::string _name, std::string _colors, int pos_in_group, std::string _type, std::string pos_on_stage, std::string moving_head_type, int modifier_pan, int modifier_tilt);

  /**
   * @brief get_fixture_options Gives you the choosen starting Point and type.
   * @param fixture_id Selected Fixture.
   * @param start_channel Start channel of the fixture.
   * @param type Type of the fixture.
   */
  void get_fixture_options(int & fixture_id, int & start_channel, QString & type, std::string &colors, int &position_in_group, std::string &position_on_stage, std::string &moving_head_type, int &modifier_pan, int &modifier_tilt);

  /**
   * @brief setup_for_edit Setup the dialog for choosing an fixture preset to be edited.
   */
  void setup_for_edit();

  /**
   * @brief edit_preset_choosen Gets you the choosen Fixture index.
   */
  int edit_preset_choosen();

  /**
   * @brief delete_fixture_preset Check if the choosen preset should be deleted.
   * @return True if the fixture should be deleted.
   */
  bool delete_fixture_preset();

 private slots:

      void on_add_fixture_dialog_clicked();

  void on_fixture_selection_currentRowChanged(int currentRow);

  void on_pB_delete_fixture_clicked();

  void update_position_in_group_status(QString current_type);
  void update_moving_head_position_status(QString current_fixture);

 private:
  Ui::EditFixtureDialog *ui;
  QStringList types;
  QStringList types_of_fixtures;
  QStringList names_of_fixtures;
  QStringList colors;
  std::list<int> blocked_channels;
  std::list<int> end_channels;
  bool is_delete;
  int max_channel = 512;

  /**
   * @brief is_allowed_start_channel Cheks if the start channel is not allready used.
   * @param start_channel Channel to be checked if it is blocked.
   * @return True if the channel is available.
   */
  bool is_allowed_start_channel(int start_channel);

  /**
   * @brief set_first_allowed_channel Sets the Spinbox for start channels to the first available.
   * @param current_row Row of the Fixture that is currently choosen. Default is the first Fixture.
   * @param first_start Prevents Messagebox from showing on the start of the Dialog.
   */
  void set_first_allowed_channel(int current_row = 0, bool first_start = false);
};

#endif // EDIT_FIXTURE_DIALOG_H
