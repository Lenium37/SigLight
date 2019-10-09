#ifndef CREATEFIXTUREDIALOG_H
#define CREATEFIXTUREDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <list>
#include <QLayout>
#include <QSignalMapper>
#include <QScrollArea>
#include "fixturemanager/fixture.h"

namespace Ui {
class CreateFixtureDialog;
}

class CreateFixtureDialog : public QDialog {
 Q_OBJECT

 public:
  explicit CreateFixtureDialog(QWidget *parent = nullptr);
  ~CreateFixtureDialog() override;
  void create_fixture_dialog_finished(std::string &, std::string &, std::string &, QStringList &, std::string &);

  /**
   * @brief edit_fixture_dialog Set up the Dialog for Editing
   * @param edit_fixture The Fixture to be added.
   * @param blocked_channels The channels that are allready blocked by Fixtures.
   * @param edit_preset Set true if the Fixture is a preset.
   */
  void edit_fixture_dialog(Fixture edit_fixture, std::list<int> blocked_channels, bool edit_preset);

  /**
   * @brief edit_fixture_finished Sets the new Fixture
   * @param edited_fixture Fixture to be added.
   */
  void edit_fixture_finished(Fixture &edited_fixture);

 private slots:
  void on_buttonBox_accepted();

  void on_buttonBox_rejected();

  void on_pB_add_channel_clicked();

  void on_pB_add_function_generated_clicked(QObject *);

private:
  Ui::CreateFixtureDialog *ui;
  int channel_count = 0;
  int last_channel = 0;
  int max_channel = 512;
  QStringList icons;
  QStringList types;
  std::list<int> blocked_channels;
  int original_start_channel;
  bool is_edit_view = false;

  /**
   * @brief is_allowed_start_channel Cheks if the start channel is not allready used.
   * @return True if the channel is available.
   */
  bool is_allowed_start_channel();
};

#endif // CREATEFIXTUREDIALOG_H
