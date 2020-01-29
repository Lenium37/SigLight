#include "edit_fixture_dialog.h"
#include "ui_edit_fixture_dialog.h"
#include <QtCore>
#include <iostream>
#include <sstream>
#include <logger.h>

EditFixtureDialog::EditFixtureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditFixtureDialog) {
  ui->setupUi(this);
  connect(ui->add_fixture_dialog, SIGNAL(finished(int)), this, SLOT(on_add_fixture_dialog_clicked()));
  ui->pB_delete_fixture->setVisible(false);
  this->setWindowTitle("Edit Fixture");
  EditFixtureDialog::is_delete = false;
}

EditFixtureDialog::EditFixtureDialog(QWidget *parent, list<Fixture> &fixtures, std::vector<std::string> color_palettes) :
    QDialog(parent),
    ui(new Ui::EditFixtureDialog) {
  ui->setupUi(this);
  types << "Ambient" << "Bass" << "Mid" << "High" << "color_change" << "flash" << "flash_reverse" << "blink" << "group_one_after_another" << "group_one_after_another_blink" << "group_one_after_another_back_and_forth" << "group_one_after_another_back_and_forth_blink" << "group_two_after_another" << "group_alternate_odd_even" << "group_random_flashes" << "strobe_if_many_onsets";
  for(std::string _colors: color_palettes)
    colors << QString::fromStdString(_colors);
  auto list_size = static_cast<double>(fixtures.size());
  for (int i = 0; i < list_size; i++) {
    ui->fixture_selection->addItem(QString::fromStdString((std::next(fixtures.begin(), i))->get_name()));
    types_of_fixtures << QString::fromStdString(std::next(fixtures.begin(), i)->get_type());
    names_of_fixtures << QString::fromStdString(std::next(fixtures.begin(), i)->get_name());
    end_channels.push_back(std::next(fixtures.begin(), i)->get_channel_count());
  }

  ui->cB_timestamps->addItem("None");
  ui->cB_timestamps->addItem("Onsets");
  //ui->cB_timestamps->addItem("Onsets bass");
  //ui->cB_timestamps->addItem("Onsets snare");
  ui->cB_timestamps->addItem("Beats 1/2/3/4");
  ui->cB_timestamps->addItem("Beats 2/4");
  ui->cB_timestamps->addItem("Beats 1/3");
  ui->cB_timestamps->addItem("Beats 1");
  ui->cB_timestamps->addItem("Beats 2");
  ui->cB_timestamps->addItem("Beats 3");
  ui->cB_timestamps->addItem("Beats 4");
  ui->cB_timestamps->addItem("Beats 1 every other bar");
  ui->cB_timestamps->addItem("Beats 1/2/3/4 action");
  ui->cB_timestamps->addItem("Beats 2/4 action");
  ui->cB_timestamps->addItem("Beats 1/3 action");
  ui->cB_timestamps->addItem("Beats 1 action");
  ui->cB_timestamps->addItem("Beats 2 action");
  ui->cB_timestamps->addItem("Beats 3 action");
  ui->cB_timestamps->addItem("Beats 4 action");
  ui->cB_timestamps->addItem("Beats 1 every other bar action");

  ui->cB_moving_head_position->addItem("Center");
  ui->cB_moving_head_position->addItem("Left");
  ui->cB_moving_head_position->addItem("Right");

  ui->cB_moving_head_type->addItem("Nothing");
  ui->cB_moving_head_type->addItem("Continuous 8");
  ui->cB_moving_head_type->addItem("Continuous Circle");
  ui->cB_moving_head_type->addItem("Continuous Line");
  ui->cB_moving_head_type->addItem("Backlight, drop on action");

  ui->fixture_selection->setCurrentRow(0);
  ui->cB_type->addItems(types);
  ui->cB_colors->addItems(colors);
  ui->sB_start_channel->setRange(1, max_channel);
  ui->sB_position_inside_group->setRange(1, 32);
  ui->sB_position_inside_mv_group->setRange(1, 32);
  ui->sB_modifier_pan->setRange(-360, 360);
  ui->sB_modifier_tilt->setRange(-180, 180);
  ui->pB_delete_fixture->setVisible(false);
  this->setWindowTitle("Edit Fixture");
  EditFixtureDialog::is_delete = false;

  connect(ui->cB_type, SIGNAL(currentTextChanged(QString)), this, SLOT(update_position_in_group_status(QString)));
  this->update_position_in_group_status(ui->cB_type->currentText());
}

EditFixtureDialog::~EditFixtureDialog() {
  delete ui;
}

void EditFixtureDialog::set_up_dialog_options(std::list<int> _blocked_channels, std::string _own_channels, std::string _name, std::string _colors, int pos_in_group, std::string _type, std::string pos_on_stage, std::string moving_head_type, int modifier_pan, int modifier_tilt, std::string timestamps_type)
{
  for(int i = 0; i < names_of_fixtures.size(); i++) {
    if(names_of_fixtures[i] == QString::fromStdString(_name))
      ui->fixture_selection->setCurrentRow(i);
  }

  //ui->lE_fixture_name->setText(QString::fromStdString(_name));
  ui->l_fixture_name->setText(QString::fromStdString(_name));

  Logger::debug("_own_channels: {}", _own_channels);


  int own_start_channel = QString::fromStdString(_own_channels).split(" ")[0].toInt();
  int own_end_channel = own_start_channel;
  if(QString::fromStdString(_own_channels).split(" ").size() > 1)
    own_end_channel = QString::fromStdString(_own_channels).split(" ")[2].toInt();


  ui->sB_start_channel->setValue(own_start_channel);

  _blocked_channels.remove(own_start_channel);
  _blocked_channels.remove(own_end_channel);

  int index_colors = ui->cB_colors->findText(QString::fromStdString(_colors));
  if ( index_colors != -1 ) { // -1 for not found
    ui->cB_colors->setCurrentIndex(index_colors);
  }

  int index_timestamps = ui->cB_timestamps->findText(QString::fromStdString(timestamps_type));
  if ( index_timestamps != -1 ) { // -1 for not found
    ui->cB_timestamps->setCurrentIndex(index_timestamps);
  }

  ui->sB_position_inside_group->setValue(pos_in_group);

  int index_type = ui->cB_type->findText(QString::fromStdString(_type));
  if ( index_type != -1 ) { // -1 for not found
    ui->cB_type->setCurrentIndex(index_type);
  }

  ui->sB_start_channel->setValue(own_start_channel);


  int index_pos_on_stage = ui->cB_moving_head_position->findText(QString::fromStdString(pos_on_stage));
  if ( index_pos_on_stage != -1 ) { // -1 for not found
    ui->cB_moving_head_position->setCurrentIndex(index_pos_on_stage);
  }

  int index_moving_head_type = ui->cB_moving_head_type->findText(QString::fromStdString(moving_head_type));
  if ( index_moving_head_type != -1 ) { // -1 for not found
    ui->cB_moving_head_type->setCurrentIndex(index_moving_head_type);
  } else {
    ui->cB_moving_head_type->setCurrentIndex(0);
  }

  ui->sB_modifier_pan->setValue(modifier_pan);
  ui->sB_modifier_tilt->setValue(modifier_tilt);

  for(int c: _blocked_channels)
    Logger::debug("blocked channel: {}", c);

  this->blocked_channels = _blocked_channels;
  //set_first_allowed_channel(0, true);
}

void EditFixtureDialog::get_fixture_options(int &fixture_id, int &start_channel, QString &type, std::string &colors, int &position_in_group, std::string &position_on_stage, std::string &moving_head_type, int &modifier_pan, int &modifier_tilt, std::string &timestamps_type)
{
  fixture_id = ui->fixture_selection->currentRow();
  start_channel = ui->sB_start_channel->value();
  type = ui->cB_type->currentText();
  position_in_group = ui->sB_position_inside_group->value();
  colors = ui->cB_colors->currentText().toStdString();
  position_on_stage = ui->cB_moving_head_position->currentText().toStdString();
  moving_head_type = ui->cB_moving_head_type->currentText().toStdString();
  modifier_pan = ui->sB_modifier_pan->value();
  modifier_tilt = ui->sB_modifier_tilt->value();
  timestamps_type = ui->cB_timestamps->currentText().toStdString();
}

void EditFixtureDialog::setup_for_edit()
{
  ui->l_type->setVisible(false);
  ui->l_start_channel->setVisible(false);
  ui->cB_type->setVisible(false);
  ui->sB_start_channel->setVisible(false);
  ui->pB_delete_fixture->setVisible(true);
  ui->add_fixture_dialog->setText("Edit");
}

int EditFixtureDialog::edit_preset_choosen() {
  return ui->fixture_selection->currentRow();
}

bool EditFixtureDialog::delete_fixture_preset()
{
  return is_delete;
}

void EditFixtureDialog::on_add_fixture_dialog_clicked() {
  if(is_allowed_start_channel(ui->sB_start_channel->value())){
    accept();
  } else {
    ui->sB_start_channel->setStyleSheet("border: 1px solid red");
  }
}

bool EditFixtureDialog::is_allowed_start_channel(int start_channel)
{
  bool is_allowed = true;
  if(((start_channel + *std::next(end_channels.begin(), ui->fixture_selection->currentRow())) - 1) > max_channel) {
    is_allowed = false;
  } else {
    for (int i = 0; i < blocked_channels.size(); i=i+2) {
      if(start_channel >= *std::next(blocked_channels.begin(), i)
          && start_channel <= *std::next(blocked_channels.begin(), i +1)) {
        is_allowed = false;
        break;
      }
    }
  }
  return is_allowed;
}

void EditFixtureDialog::set_first_allowed_channel(int current_Row, bool first_start)
{
  int end_value = *std::next(end_channels.begin(), current_Row);
  bool channels_left = false;
  bool channel_range_blocked = true;
  int calculated_end_value;
  for(int i = 1; i < max_channel; i++) {
    if(is_allowed_start_channel(i)) {
      calculated_end_value = i + end_value;
      for(int j = i; j < calculated_end_value; j++) {
        if((channel_range_blocked = !is_allowed_start_channel(j))) {
          break;
        } else if (j > ui->sB_start_channel->maximum()) {
          channel_range_blocked = true;
          break;
        }
      }
      if(!channel_range_blocked) {
        ui->sB_start_channel->setValue(i);
        channels_left = true;
        break;
      }
    }
  }
  if(!channels_left && !first_start) {
    QMessageBox::question(this, "No free Channels left", "There are no more free Channels left for this Fixture.", QMessageBox::Ok);
  }
  ui->add_fixture_dialog->setEnabled(channels_left);
}

void EditFixtureDialog::on_fixture_selection_currentRowChanged(int currentRow)
{
  //ui->cB_type->setCurrentIndex(ui->cB_type->findText(types_of_fixtures.at(currentRow)));
  //set_first_allowed_channel(currentRow);
  ui->l_fixture_name->setText(names_of_fixtures.at(currentRow));
  this->update_moving_head_position_status(names_of_fixtures.at(currentRow));
}

void EditFixtureDialog::on_pB_delete_fixture_clicked()
{
  EditFixtureDialog::is_delete = true;
  accept();
}

void EditFixtureDialog::update_position_in_group_status(QString current_type) {
  //std::cout << current_type.toStdString() << std::endl;
  current_type = current_type.toLower();
  if(current_type == "group_one_after_another"
      || current_type == "group_one_after_another_back_and_forth"
      || current_type == "group_one_after_another_back_and_forth_blink"
      || current_type == "group_two_after_another"
      || current_type == "group_alternate_odd_even"
      || current_type == "group_one_after_another_blink"
      || current_type == "group_random_flashes")
    ui->sB_position_inside_group->setEnabled(true);
  else {
    ui->sB_position_inside_group->setEnabled(false);
    ui->sB_position_inside_group->setValue(0);
  }

  if(current_type == "color_change"
      || current_type == "flash"
      || current_type == "flash_reverse"
      || current_type == "blink"
      || current_type == "group_one_after_another"
      || current_type == "group_one_after_another_blink"
      || current_type == "group_one_after_another_back_and_forth"
      || current_type == "group_one_after_another_back_and_forth_blink"
      || current_type == "group_two_after_another"
      || current_type == "group_alternate_odd_even"
      || current_type == "group_random_flashes") {
    ui->cB_timestamps->setEnabled(true);
  } else {
    ui->cB_timestamps->setEnabled(false);
    ui->cB_timestamps->setCurrentIndex(0);
  }
}

void EditFixtureDialog::update_moving_head_position_status(QString current_fixture) {
  if(current_fixture == "JBLED A7 (S8)"
  || current_fixture == "JBLED P4 (M1)"
  || current_fixture == "JBLED Sparx 7 (M3)") {
    ui->cB_moving_head_position->setEnabled(true);
    ui->cB_moving_head_type->setEnabled(true);
    ui->sB_modifier_pan->setEnabled(true);
    ui->sB_modifier_tilt->setEnabled(true);
  } else {
    ui->cB_moving_head_position->setCurrentIndex(0);
    ui->cB_moving_head_position->setEnabled(false);
    ui->cB_moving_head_type->setCurrentIndex(0);
    ui->cB_moving_head_type->setEnabled(false);
    ui->sB_modifier_pan->setEnabled(false);
    ui->sB_modifier_pan->setValue(0);
    ui->sB_modifier_tilt->setEnabled(false);
    ui->sB_modifier_tilt->setValue(0);
  }

  if(current_fixture == "SGM X-5 (1CH)"
      || current_fixture == "SGM X-5 (3CH)"
      || current_fixture == "SGM X-5 (4CH)") {
    ui->cB_type->setEnabled(false);
    int index_white = ui->cB_colors->findText("W");
    if ( index_white != -1 ) { // -1 for not found
      ui->cB_colors->setCurrentIndex(index_white);
    }
    ui->cB_colors->setEnabled(false);
  } else {
    ui->cB_type->setEnabled(true);
    ui->cB_colors->setEnabled(true);
  }
}