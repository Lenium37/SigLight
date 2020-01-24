//
// Created by Johannes on 23.01.2020.
//

#ifndef SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_
#define SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_

//#include <QtWidgets/QDialog>
#include <QtWidgets/QMainWindow>
#include "fixturechoosingdialog.h"
#include <QtWidgets/QTreeWidgetItem>
#include <fixturemanager/universe.h>
#include <song.h>
#include "edit_fixture_dialog.h"

namespace Ui {
class ChangeFixtures;
}

class ChangeFixtures : public QMainWindow {
 Q_OBJECT

 public:
  explicit ChangeFixtures(list<Fixture> _fixtures, std::vector<std::string> _color_palettes, std::list<Fixture> _fixture_presets, Song *_song, QWidget *parent = nullptr);
  ~ChangeFixtures() override;

  void setup_dialog();

 signals:
  void changed_fixtures_ready(Song *song, std::list<Fixture> fixtures);


 private:
  Ui::ChangeFixtures *ui;
  FixtureChoosingDialog *fcd;
  EditFixtureDialog *efd;
  std::list<Fixture> fixtures;
  std::list<Fixture> fixture_presets;
  Song *song;
  std::vector<std::string> color_palettes;
  Universe universes[5];
  list<QTreeWidgetItem> universe_tree;

  void add_universe(QString name);
  void add_fixture(QTreeWidgetItem *parent, Fixture _fixture, int start_channel, QString type, std::string _colors, int position_in_group, std::string position_on_stage, std::string moving_head_type, int modifier_pan, int modifier_tilt);
  void resize_fixture_list_columns();

 private slots:
  void on_add_fixture_button_clicked();
  void on_edit_fixture_clicked();
  void on_delete_fixture_button_clicked();
  void get_fixture_for_universe();
  void get_edited_fixture();
  void on_use_changed_fixtures_clicked();



};

#endif //SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_
