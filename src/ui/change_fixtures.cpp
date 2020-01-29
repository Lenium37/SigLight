//
// Created by Johannes on 23.01.2020.
//

#include <iostream>
#include "change_fixtures.h"
#include "ui_change_fixtures.h"

ChangeFixtures::ChangeFixtures(list<Fixture> _fixtures, std::vector<std::string> _color_palettes, std::list<Fixture> _fixture_presets, QUrl _song_url, int _user_bpm, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::ChangeFixtures) {
    ui->setupUi(this);
    this->song = nullptr;
    this->fixtures = _fixtures;
    this->fixture_presets = _fixture_presets;
    this->color_palettes = _color_palettes;
    this->song_url = _song_url;
    this->user_bpm = _user_bpm;

    this->setWindowIcon(QIcon(":/icons_svg/svg/rtl_icon.svg"));
    std::string window_title = "Change Fixtures for " + _song_url.fileName().toStdString();
    this->setWindowTitle(QString::fromStdString(window_title));

    ui->sB_user_bpm->setRange(0, 250);
    ui->sB_user_bpm->setValue(this->user_bpm);

    ui->fixture_list->setStyleSheet("background-color: rgb(238, 238, 236);");

    // Set options for the fixturelist.
    ui->fixture_list->setSelectionMode(QAbstractItemView::SingleSelection);

    // Sets the layout for the fixturelist.
    ui->fixture_list->setColumnCount(10);

    ui->fixture_list->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

    ui->fixture_list->setContextMenuPolicy(Qt::CustomContextMenu);
    // Prepares a list for the headers of the list.
    QStringList headers;
    headers.append("Fixtures");
    headers.append("Channel");
    headers.append("Colors");
    headers.append("Timestamps");
    headers.append("# in group");
    headers.append("Pos. on stage");
    headers.append("Mov. Head type");
    headers.append("# in MH group");
    headers.append("Modifier pan");
    headers.append("Modifier tilt");
    ui->fixture_list->setHeaderLabels(headers);

    add_universe("Universe1");

    for(Fixture f : this->fixtures) {
      add_fixture(&universe_tree.back(), f, f.get_start_channel(), QString::fromStdString(f.get_type()), f.get_colors(), f.get_position_in_group(), f.get_position_on_stage(), f.get_moving_head_type(), f.get_modifier_pan(), f.get_modifier_tilt(), f.get_timestamps_type(), f.get_position_in_mh_group());
    }

    //connect(this, SIGNAL(changed_fixtures_ready(song, fixtures)), this->parent(), &MainWindow::changed_fixtures_for_lightshow_ready);
}

ChangeFixtures::~ChangeFixtures() {
  delete ui;
}

void ChangeFixtures::setup_dialog() {

}

void ChangeFixtures::on_add_fixture_button_clicked() {
  std::cout << "add clicked" << std::endl;
  if (universes[0].get_fixture_count() >= 512) {
    QMessageBox::question(this, "Too many fixtures",
                          "Reached maximum amount of Fixtures. Only 512 Devices allowed.",
                          QMessageBox::Ok);
  } else {
    this->fcd = new FixtureChoosingDialog(this, this->fixture_presets, this->color_palettes);
    this->fcd->set_up_dialog_options(universes[0].get_blocked_adress_range());
    connect(this->fcd, SIGNAL(accepted()), this, SLOT(get_fixture_for_universe()));
    this->fcd->exec();
  }
}

void ChangeFixtures::on_edit_fixture_clicked() {
  std::cout << "edit clicked" << std::endl;
  // Only delete if an Item was clicked.
  if (!ui->fixture_list->selectedItems().empty()) {
    // Toplevelitems should not have a description.
    if (ui->fixture_list->currentItem()) {
      if (ui->fixture_list->currentItem()->parent()) {
        if (ui->fixture_list->currentItem()->parent()->parent()) {
          this->efd = new EditFixtureDialog(this, this->fixture_presets, this->color_palettes);
          std::string modifier_pan_s = ui->fixture_list->currentItem()->text(8).toStdString();
          std::string modifier_tilt_s = ui->fixture_list->currentItem()->text(9).toStdString();
          std::string timestamps_type = ui->fixture_list->currentItem()->text(3).toStdString();
          int modifier_pan = 0;
          int modifier_tilt = 0;
          if(modifier_pan_s.size() > 1)
            modifier_pan = std::stoi(modifier_pan_s.erase(modifier_pan_s.size()-1));
          if(modifier_tilt_s.size() > 1)
            modifier_tilt = std::stoi(modifier_tilt_s.erase(modifier_tilt_s.size()-1));


          this->efd->set_up_dialog_options(universes[0].get_blocked_adress_range(),
                                           ui->fixture_list->currentItem()->text(1).toStdString(),
                                           ui->fixture_list->currentItem()->text(0).toStdString(),
                                           ui->fixture_list->currentItem()->text(2).toStdString(),
                                           ui->fixture_list->currentItem()->text(4).toInt(),
                                           ui->fixture_list->currentItem()->parent()->text(0).toStdString(),
                                           ui->fixture_list->currentItem()->text(5).toStdString(),
                                           ui->fixture_list->currentItem()->text(6).toStdString(),
                                           modifier_pan,
                                           modifier_tilt,
                                           timestamps_type,
                                           ui->fixture_list->currentItem()->text(7).toInt());
          connect(this->efd, SIGNAL(accepted()), this, SLOT(get_edited_fixture()));
          this->efd->exec();
        }
      }
    }
  }
}

void ChangeFixtures::get_edited_fixture() {
  int fixture_index = 0;
  int start_channel = 0;
  QString type;
  std::string colors;
  int position_in_group = 0;
  std::string position_on_stage;
  std::string moving_head_type;
  int modifier_pan;
  int modifier_tilt;
  std::string timestamps_type;
  int position_in_mh_group = 0;

  this->efd->get_fixture_options(fixture_index, start_channel, type, colors, position_in_group, position_on_stage, moving_head_type, modifier_pan, modifier_tilt, timestamps_type, position_in_mh_group);


  //std::cout << "new type of fixture: " << type.toStdString() << std::endl;


  QList<QTreeWidgetItem *> type_items = ui->fixture_list->findItems(type, Qt::MatchExactly | Qt::MatchRecursive, 0);
  QTreeWidgetItem *type_item;
  if (type_items.size() == 0) {
    type_item = new QTreeWidgetItem();
    type_item->setText(0, type);
    type_item->setText(1, "");
    type_item->setText(2, "");
    type_item->setText(3, "");
    type_item->setText(4, "");
    type_item->setText(5, "");
    type_item->setText(6, "");
    type_item->setText(7, "");
    type_item->setText(8, "");
    type_item->setText(9, "");
    type_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
    std::cout << "debug22 " << type_item->text(0).toStdString()<< std::endl;

    ui->fixture_list->topLevelItem(0)->addChild(type_item);
    //parent->addChild(type_item);
    std::cout << "debug23" << std::endl;
  } else {
    type_item = type_items.back();
  }
  std::cout << "debug3" << std::endl;
  Fixture fix = *(std::next(fixtures.begin(), fixture_index));
  fix.set_start_channel(start_channel);
  fix.set_type(type.toStdString());
  if(!colors.empty())
    fix.set_colors(colors);
  else
    fix.set_colors("Y");

  fix.set_position_in_group(position_in_group);
  fix.set_position_on_stage(position_on_stage);
  fix.set_moving_head_type(moving_head_type);
  fix.set_modifier_pan(modifier_pan);
  fix.set_modifier_tilt(modifier_tilt);
  fix.set_timestamps_type(timestamps_type);
  fix.set_position_in_mh_group(position_in_mh_group);

  auto *new_item = new QTreeWidgetItem(type_item);
  new_item->setFlags(
      Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

  new_item->setText(0, QString::fromStdString(fix.get_name()));
  new_item->setText(1, QString::fromStdString(std::to_string(fix.get_start_channel()) + " - " +
      std::to_string(fix.get_last_channel())));
  new_item->setIcon(0,
                    QIcon(
                        ":icons_svg/svg/" + QString::fromStdString(fix.get_icon()) + ".svg"));
  new_item->setText(2, QString::fromStdString(fix.get_colors()));
  new_item->setText(3, QString::fromStdString(fix.get_timestamps_type()));
  new_item->setText(4, QString::fromStdString(std::to_string(fix.get_position_in_group())));
  new_item->setText(5, QString::fromStdString(fix.get_position_on_stage()));
  if(fix.get_moving_head_type() != "Nothing")
    new_item->setText(6, QString::fromStdString(fix.get_moving_head_type()));
  else new_item->setText(6, "");
  new_item->setText(7, QString::fromStdString(std::to_string(fix.get_position_in_mh_group())));
  if(fix.get_modifier_pan())
    new_item->setText(8, QString::fromStdString(std::to_string(fix.get_modifier_pan()) + "°"));
  else new_item->setText(8, "");
  if(fix.get_modifier_tilt())
    new_item->setText(9, QString::fromStdString(std::to_string(fix.get_modifier_tilt()) + "°"));
  else new_item->setText(9, "");


  if(type_item && new_item) {
    type_item->addChild(new_item);
    type_item->setExpanded(true);
  }

  std::cout << "ui->fixture_list->currentItem()->text(1).split(" ")[0]: " << ui->fixture_list->currentItem()->text(1).split(" ")[0].toStdString() << std::endl;
  int cur_item = universes[0].get_fixtureid_by_startchannel(ui->fixture_list->currentItem()->text(1).split(" ")[0].toInt());
  // Delete the fixture from the Dataside.
  universes[0].remove_fixture(cur_item);

  QTreeWidgetItem *item = ui->fixture_list->currentItem();
  QTreeWidgetItem *parent = item->parent();
  if (item && parent) {
    //item = parent->takeChild(parent->indexOfChild(item));
    parent->removeChild(item);
  }

  universes[0].add_fixture(fix);


  //fixtures_changed = true;

  if (parent && parent->childCount() > 0) parent->setExpanded(true);


  for(int i = 0; i < universe_tree.back().childCount(); i++) {
    if(universe_tree.back().child(i)->childCount() == 0)
      universe_tree.back().removeChild(universe_tree.back().child(i));
  }
  this->resize_fixture_list_columns();
}

void ChangeFixtures::on_delete_fixture_button_clicked() {
  std::cout << "delete clicked" << std::endl;
  // Only delete if an Item was clicked.
  if (!ui->fixture_list->selectedItems().empty()) {
    bool delete_items = false;
    // Toplevelitems should not have a description.
    if (ui->fixture_list->currentItem()->parent()) {
      if (ui->fixture_list->currentItem()->parent() == ui->fixture_list->topLevelItem(0)) {
        QMessageBox::StandardButton answer = QMessageBox::warning(this,
                                                                  "Deleting a type!",
                                                                  "You are about to delete a complete type and its Fixtures. \n Delete the type and all its Fixtures?",
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (answer == QMessageBox::Yes) {
          std::list<Fixture> type_fixtures = universes[0]
              .get_fixture_by_type(ui->fixture_list->currentItem()->text(0).toStdString());
          for (int i = 0; i < type_fixtures.size(); i++) {
            universes[0].remove_fixture(
                universes[0].get_fixtureid_by_startchannel(
                    (*std::next(type_fixtures.begin(), i)).get_start_channel()));
            ui->fixture_list->currentItem()->takeChild(i);
          }
          delete_items = true;
        }
      } else {
        int cur_item =
            universes[0].get_fixtureid_by_startchannel(ui->fixture_list->currentItem()->text(1).split(" ")[0].toInt());
        // Delete the fixture from the Dataside.
        universes[0].remove_fixture(cur_item);
        delete_items = true;
      }
      if (delete_items) {
        QTreeWidgetItem *item = ui->fixture_list->currentItem();
        QTreeWidgetItem *parent = item->parent();
        if (item) {
          parent->takeChild(parent->indexOfChild(item));
        }
      }
    }
    if (delete_items) {
      this->resize_fixture_list_columns();
      //fixtures_changed = true;
    }
  }
  for(int i = 0; i < universe_tree.back().childCount(); i++) {
    if(universe_tree.back().child(i)->childCount() == 0)
      universe_tree.back().removeChild(universe_tree.back().child(i));
  }
  this->resize_fixture_list_columns();
}

void ChangeFixtures::add_universe(QString name) {
  auto *itm = new QTreeWidgetItem();
  itm->setIcon(0, QIcon(":icons_svg/svg/group.svg"));
  universes[0] = *new Universe(name.toUtf8().constData());
  itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
  universe_tree.push_back(*itm);
  (&universe_tree.back())->setText(0, QString::fromStdString(universes[0].get_name()));
  ui->fixture_list->addTopLevelItem((&universe_tree.back()));
}

void ChangeFixtures::add_fixture(QTreeWidgetItem *parent, Fixture _fixture, int start_channel, QString type, std::string _colors, int position_in_group, std::string position_on_stage, std::string moving_head_type, int modifier_pan, int modifier_tilt, std::string timestamps_type, int position_in_mh_group) {
  auto *itm = new QTreeWidgetItem();
  _fixture.set_start_channel(start_channel);
  _fixture.set_type(type.toStdString());
  if(!_colors.empty())
    _fixture.set_colors(_colors);
  else
    _fixture.set_colors("Y");

  _fixture.set_position_in_group(position_in_group);
  _fixture.set_position_on_stage(position_on_stage);
  _fixture.set_moving_head_type(moving_head_type);
  _fixture.set_modifier_pan(modifier_pan);
  _fixture.set_modifier_tilt(modifier_tilt);
  _fixture.set_timestamps_type(timestamps_type);
  _fixture.set_position_in_mh_group(position_in_mh_group);

  universes[0].add_fixture(_fixture);

  itm->setText(0, QString::fromStdString(universes[0].get_fixtures().back().get_name()));
  itm->setText(1, QString::fromStdString(
      std::to_string(universes[0].get_fixtures().back().get_start_channel()) + " - " +
          std::to_string(universes[0].get_fixtures().back().get_last_channel())));
  itm->setIcon(0,
               QIcon(
                   ":icons_svg/svg/" + QString::fromStdString(universes[0].get_fixtures().back().get_icon()) + ".svg"));
  /*QList<QTreeWidgetItem *> type_items = ui->fixture_list->findItems(QString::fromStdString(_fixture.get_type()),
                                                                    Qt::MatchExactly | Qt::MatchRecursive,
                                                                    0);*/
  itm->setText(2, QString::fromStdString(universes[0].get_fixtures().back().get_colors()));
  itm->setText(3, QString::fromStdString(universes[0].get_fixtures().back().get_timestamps_type()));
  itm->setText(4, QString::fromStdString(std::to_string(universes[0].get_fixtures().back().get_position_in_group())));
  itm->setText(5, QString::fromStdString(universes[0].get_fixtures().back().get_position_on_stage()));
  if(universes[0].get_fixtures().back().get_moving_head_type() != "Nothing")
    itm->setText(6, QString::fromStdString(universes[0].get_fixtures().back().get_moving_head_type()));
  else itm->setText(6, "");
  itm->setText(7, QString::fromStdString(std::to_string(universes[0].get_fixtures().back().get_position_in_mh_group())));
  if(universes[0].get_fixtures().back().get_modifier_pan())
    itm->setText(8, QString::fromStdString(std::to_string(universes[0].get_fixtures().back().get_modifier_pan()) + "°"));
  else itm->setText(8, "");
  if(universes[0].get_fixtures().back().get_modifier_tilt())
    itm->setText(9, QString::fromStdString(std::to_string(universes[0].get_fixtures().back().get_modifier_tilt()) + "°"));
  else itm->setText(9, "");

  QList<QTreeWidgetItem *> type_items = ui->fixture_list->findItems(type, Qt::MatchExactly | Qt::MatchRecursive, 0);

  QTreeWidgetItem *type_item;
  if (type_items.size() == 0) {
    type_item = new QTreeWidgetItem();
    type_item->setText(0, type);
    type_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
    //std::cout << "parent: " << parent->text(0).toStdString() << std::endl;
    //std::cout << "debug22 " << type_item->text(0).toStdString()<< std::endl;

    parent->addChild(type_item);
    //std::cout << "debug23" << std::endl;
  } else {
    type_item = type_items.back();
  }
  //std::cout << "debug3" << std::endl;
  itm->setFlags(
      Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
  type_item->addChild(itm);
  type_item->setExpanded(true);

  if (parent->childCount() > 0) parent->setExpanded(true);
  //ui->fixture_list->resizeColumnToContents(0);
  this->resize_fixture_list_columns();

  //delete itm;
  //delete type_item;
}

void ChangeFixtures::resize_fixture_list_columns() {
  for(int i = 0; i < this->ui->fixture_list->columnCount(); i++) {
    this->ui->fixture_list->resizeColumnToContents(i);
  }
}

void ChangeFixtures::get_fixture_for_universe() {
  int fixture_index = 0;
  int start_channel = 0;
  QString type;
  std::string colors;
  int position_in_group = 0;
  std::string position_on_stage;
  std::string moving_head_type;
  int modifier_pan;
  int modifier_tilt;
  std::string timestamps_type;
  int position_in_mh_group = 0;

  this->fcd->get_fixture_options(fixture_index, start_channel, type, colors, position_in_group, position_on_stage, moving_head_type, modifier_pan, modifier_tilt, timestamps_type, position_in_mh_group);
  add_fixture((&universe_tree.back()), *(std::next(fixture_presets.begin(), fixture_index)), start_channel, type, colors, position_in_group, position_on_stage, moving_head_type, modifier_pan, modifier_tilt, timestamps_type, position_in_mh_group);
  //fixtures_changed = true;
}

void ChangeFixtures::on_use_changed_fixtures_clicked() {
  std::cout << this->song << std::endl;
  if(this->song)
    std::cout << this->song->get_song_name() << std::endl;

  if(this->song)
    emit changed_fixtures_of_existing_lightshow(this->song, this->universes[0].get_fixtures(), ui->sB_user_bpm->value());
  else
    emit changed_fixtures_ready(song_url, this->universes[0].get_fixtures(), ui->sB_user_bpm->value());
}

void ChangeFixtures::set_song(Song* _song) {
  this->song = _song;
}

Song* ChangeFixtures::get_song() {
  return this->song;
}
