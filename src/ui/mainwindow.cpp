#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lightshow_generator.h"
#include <libusb.h>
#include <iostream>
#include <unistd.h>
#include <lightshow/analysis.h>
#include <QDateTime>

#include <file_system_utils_qt.h>
#include <logger.h>
#include <song.h>
// for copying song files
#include <fstream>
#include "mp_3_to_wav_converter.h"

#define DEFAULT_STATUSBAR_OUTPUT "No media is playing!     00:00:00 / 00:00:00"
#define DEFAULT_COMBOBOX_OUTPUT "Default Playerview!"
#define ERROR_MESSAGE_NO_SONG "Please add a song to the Playlist!"
#define ERROR_MESSAGE_NO_FIXTURES "Please add a Fixture first!"

bool lightshow_playing = false;
bool lightshow_paused = true;

#if defined(_WIN32) || defined(WIN32)
#include <mingw.thread.h>
#else
#include <thread>
#endif

std::vector<LightshowFixture> create_multiple_fixtures(std::string type, int amount) {
  std::vector<LightshowFixture> fixtures;
  if (type.compare("Cameo Flat RGB 10") == 0) {
    for (int i = 0; i < amount; i++) {
      fixtures.push_back(LightshowFixture(type, (i * 6) + 1, 6, "mid"));
    }
  } else std::cout << "Fixture type unknown." << std::endl;
  return fixtures;
}

void test_xml_and_dmx_output(MainWindow *window) {

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->stackedWidget->setCurrentIndex(0);
  ui->mainToolBa->hide();
  ui->menuBar->hide();
  ui->statusBar->hide();
  ui->fixture_list->setStyleSheet("background-color: rgb(238, 238, 236);");
  ui->fixture_details->setStyleSheet("background-color: rgb(238, 238, 236);");
  this->is_in_dark_mode = false;

  this->setWindowIcon(QIcon(":/icons_svg/svg/rtl_icon.svg"));
  ui->label->setPixmap(QPixmap(":/icons_png/png/rtl_icon_squared.png").scaledToWidth(300));
  player = new MusicPlayer();

  player_edit_view = new Player_edit_view();

  // default view
  player_edit_view->initalize_default_scene();
  default_is_active = true;

  playlist_view = new Playlist_view(this);

  ui->stackedWidget->insertWidget(1, player_edit_view);
  ui->stackedWidget->insertWidget(2, playlist_view);

  init_toolbar();
  init_status_bar();
  init_shortcuts();
  init_connects();
  init();
  this->read_own_m3u_on_startup();
}

void MainWindow::init() {
  this->setWindowState(Qt::WindowFullScreen);
  rtl_path = get_home();
  if (rtl_path.endsWith("/")) {
    rtl_path = rtl_path + "Raspberry-to-Light/";
    lightshows_directory_path = rtl_path.toStdString() + "xml-lightshows/";
    songs_directory_path = rtl_path.toStdString() + "songs/";
  } else {
    rtl_path = rtl_path + "Music\\Raspberry-to-Light\\";
    lightshows_directory_path = rtl_path.toStdString() + "xml-lightshows\\";
    songs_directory_path = rtl_path.toStdString() + "songs\\";
  }
  QDir dir;
  if (!dir.exists(rtl_path)) {
    dir.mkdir(rtl_path);
  }
  if (!dir.exists(QString::fromStdString(this->lightshows_directory_path))) {
    dir.mkdir(QString::fromStdString(this->lightshows_directory_path));
  }
  if (!dir.exists(QString::fromStdString(this->songs_directory_path))) {
    dir.mkdir(QString::fromStdString(this->songs_directory_path));
  }
  // Set options for the fixturelist.
  ui->fixture_list->setSelectionMode(QAbstractItemView::SingleSelection);

  // Sets the layout for the fixturelist.
  ui->fixture_list->setColumnCount(2);

  ui->fixture_list->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);

  ui->fixture_list->setContextMenuPolicy(Qt::CustomContextMenu);
  // Prepares a list for the headers of the list.
  QStringList headers;
  headers.append("Fixtures");
  headers.append("Channel");
  ui->fixture_list->setHeaderLabels(headers);
  // Create the Fixtureobjects.
  create_fixtures();
  // Adds the first Univers.
  add_universe("Universe1", "USB-DMX-Interface");
  load_fixture_objects_from_xml(false);
  this->setWindowTitle("Raspberry To Light");
  this->check_which_dmx_device_is_connected();
  // claim device interface
  if(get_current_dmx_device().is_connected())
    get_current_dmx_device().start_device();
  this->lightshow_player = new LightshowPlayer(get_current_dmx_device());
  //this->lightshow_player = new LightshowPlayer(get_current_dmx_device());
  this->lightshow_resolution = 40;

  ls_generating_thread_is_alive = false;
  player->set_songs_directory_path(this->songs_directory_path);

}

void MainWindow::init_toolbar() {
  ui->mainToolBa->toggleViewAction()->setEnabled(false);
  ui->mainToolBa->setFloatable(false);
  ui->mainToolBa->setMovable(false);

  volume_button = new Volume_button(this);
  volume_button->set_volume(player->volume());

  ui->mainToolBa->insertWidget(ui->action_activate_grid, volume_button);

  QWidget *spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->mainToolBa->addWidget(spacer);

  combobox_edit_chosen_song = new QComboBox;
  combobox_model = new QStandardItemModel(combobox_edit_chosen_song);
  combobox_edit_chosen_song->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  combobox_edit_chosen_song->setInsertPolicy(QComboBox::NoInsert);
  combobox_edit_chosen_song->addItem(DEFAULT_COMBOBOX_OUTPUT);
  combobox_edit_chosen_song->setMaxVisibleItems(10);
  combobox_edit_chosen_song->setToolTip("Choose the lightshow to display in the playerview");
  ui->mainToolBa->addWidget(combobox_edit_chosen_song);
  ui->mainToolBa->setToolButtonStyle(Qt::ToolButtonFollowStyle);
  hide_edit_tools();
}

void MainWindow::init_status_bar() {
  label_current_song_and_duration = new QLabel(this);
  label_current_song_and_duration->setText(DEFAULT_STATUSBAR_OUTPUT);
  ui->statusBar->addWidget(label_current_song_and_duration);
}

void MainWindow::init_connects() {
  connect(ui->fixture_list, SIGNAL(selectionChanged(
                                       const QItemSelection &, const QItemSelection &)), this,
          SLOT(on_fixture_list_itemSelectionChanged()));

  connect(playlist_view, &QTableView::doubleClicked, [this](const QModelIndex &index) {
    if (get_current_dmx_device().is_connected()) {
      get_current_dmx_device().stop_device();
      lightshow_playing = false;
      lightshow_paused = true;
    }
    player->set_media_to(index.row());
  });

  connect(this->player, &MusicPlayer::position_changed, this, &MainWindow::update_label_song_position_and_duration);
  connect(this->player, &MusicPlayer::title_or_artist_of_song_has_changed, this, &MainWindow::play_next_song);
  connect(this->player, &MusicPlayer::media_inserted_into_playlist, this, &MainWindow::add_player_edit_mode);
  connect(this->player,
          &MusicPlayer::media_inserted_into_playlist_on_startup,
          this,
          &MainWindow::add_player_edit_mode);
  /* Out of Order -> still here so it can posibly be used in the future
  connect(this->playlist_view,
          &Playlist_view::song_title_or_artist_has_changed,
          this->player,
          &MusicPlayer::slot_title_or_artist_of_song_has_changed);
  connect(this->playlist_view,
          &Playlist_view::song_title_or_artist_has_changed,
          this,
          &MainWindow::slot_song_title_or_artist_changed_update_combobox);
  */
  connect(this->playlist_view,
          &Playlist_view::playlist_order_has_been_changed,
          this,
          &MainWindow::slot_order_playlist_changed);
  connect(this,
          &MainWindow::lightshow_for_song_is_ready,
          this->player,
          &MusicPlayer::slot_lightshow_for_song_is_ready);
  connect(this, &MainWindow::lightshow_for_song_is_ready,
          this->playlist_view,
          [=](Song *song) { playlist_view->slot_song_lightshow_state_has_changed(this->player->playlist_index_for(song)); });
  connect(this, &MainWindow::lightshow_for_song_is_ready,
          this,
          [=](Song *song) { if(this->player->playlist_index_for(song) == combobox_edit_chosen_song->currentIndex())
                                this->change_player_edit_view(this->player->playlist_index_for(song)); });

  // Changing Position in the song has been shut down, because of time reasons
  //connect(player_edit_view, &Player_edit_view::header_item_double_clicked, player, &MusicPlayer::slot_header_item_double_clicked);

  connect(volume_button, &Volume_button::volume_changed, player, &MusicPlayer::set_volume);

  connect(ui->fixture_list, SIGNAL(customContextMenuRequested(
                                       const QPoint &)),
          this, SLOT(ShowContextMenu(
                         const QPoint &)));
  // Handle drop of a Fixture.
  connect(ui->fixture_list->model(), SIGNAL(rowsInserted(
                                                const QModelIndex &, int, int)), this, SLOT(rowsInserted(
                                                                                                const QModelIndex &, int, int)));
  connect(key_f11, SIGNAL(activated()), this, SLOT(slot_shortcut_f11()));
}

void MainWindow::init_shortcuts() {
  key_f11 = new QShortcut(this);
  key_f11->setKey(Qt::Key_F11);
}

MainWindow::~MainWindow() {
  delete ui;
}

/**
 * Creates an Univers and adds it to the universlist.
 * @brief MainWindow::addUniverse
 * @param name name of the univers.
 * @param description Description wich kind of univers this is.
 */
void MainWindow::add_universe(QString name, QString description) {
  auto *itm = new QTreeWidgetItem();
  itm->setIcon(0, QIcon(":icons_svg/svg/group.svg"));
  universes[0] = *new Universe(name.toUtf8().constData(), description.toUtf8().constData());
  itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
  universe_tree.push_back(*itm);
  (&universe_tree.back())->setText(0, QString::fromStdString(universes[0].get_name()));
  (&universe_tree.back())->setText(1, QString::fromStdString(universes[0].get_description()));
  ui->fixture_list->addTopLevelItem((&universe_tree.back()));
  //delete itm;
}

void MainWindow::add_fixture(QTreeWidgetItem *parent, Fixture _fixture, int start_channel, QString type) {
  auto *itm = new QTreeWidgetItem();
  _fixture.set_start_channel(start_channel);
  _fixture.set_type(type.toStdString());
  universes[0].add_fixture(_fixture);

  itm->setText(0, QString::fromStdString(universes[0].get_fixtures().back().get_name()));
  itm->setText(1, QString::fromStdString(
      std::to_string(universes[0].get_fixtures().back().get_start_channel()) + " - " +
          std::to_string(universes[0].get_fixtures().back().get_last_channel())));
  itm->setIcon(0,
               QIcon(
                   ":icons_svg/svg/" + QString::fromStdString(universes[0].get_fixtures().back().get_icon()) + ".svg"));
  QList<QTreeWidgetItem *> type_items = ui->fixture_list->findItems(QString::fromStdString(_fixture.get_type()),
                                                                    Qt::MatchExactly | Qt::MatchRecursive,
                                                                    0);
  QTreeWidgetItem *type_item;
  if (type_items.size() == 0) {
    type_item = new QTreeWidgetItem();
    type_item->setText(0, QString::fromStdString(_fixture.get_type()));
    parent->addChild(type_item);
    type_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
  } else {
    type_item = type_items.back();
  }
  itm->setFlags(
      Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
  type_item->addChild(itm);
  type_item->setExpanded(true);

  if (parent->childCount() > 0) parent->setExpanded(true);
  ui->fixture_list->resizeColumnToContents(0);
  //delete itm;
  //delete type_item;
}

void MainWindow::create_fixtures() {
  // setup the file for Fixture objects.
  QFile fixture_objects_file(rtl_path + fixture_objects_file_name);
  QFileInfo datei(fixture_objects_file);

  // If the file is older than the 28.05.2019 it has to be deleted.
  if (datei.created().date() < (*(new QDate(2019, 6, 20)))) {
    fixture_objects_file.remove();
  }
  if (fixture_objects_file.exists()) {
    load_fixture_objects_from_xml();
  } else {
    QStringList channels;

    channels << "Master dimmer (0-100%)~0~255" << "Strobo~0~255" << "Red (0-100%)~0~225" << "Green (0-100%)~0~225"
             << "Blue (0-100%)~0~225"
             << "Blackout/colour mix CH3 - CH5~0~4|Red~5~15|Green~16~26|Blue~27~37|Yellow~38~48|Magenta~49~59|Cyan~60~70|White~71~80|Color change (rate)~81~150|Color blending~151~220";

    create_new_fixture("Cameo Flat RGB 10",
                       "Bass",
                       "DMX-Funktionen: Colour Fade, Colour Jump, Master Dimmer, RGB, Sound Control, Strobe",
                       channels,
                       "lamp");
    channels.clear();

    channels << "Aus~0~5|Output (5-95%)~6~249|Max Output (100%)~250~255";
    create_new_fixture("ANTARI Z-1200 MKII",
                       "Ambient",
                       "Ausstossmenge über DMX kontrollierbar",
                       channels,
                       "smoke");
    channels.clear();

    channels << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "Red (0-100%)~0~255"
             << "Green (0-100%)~0~255" << "Blue (0-100%)~0~255" << "White (0-100%)~0~255" << "Strobo~0~255"
             << "Master dimmer (0-100%)~0~255";
    create_new_fixture("Helios 7",
                       "Mid",
                       "DMX-Funktionen: Pan, Tilt, Farben, noch viel mehr",
                       channels,
                       "lamp");
    channels.clear();

    channels << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1"
             << "Colors Red~13~25|Blue~26~38|Yellow~39~51|Green~52~64|Pink~65~77" << "NULL~0~1" << "NULL~0~1"
             << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1" << "NULL~0~1"
             << "NULL~0~1" << "Master dimmer (0-100%)~0~255";
    create_new_fixture("Cobalt Plus Spot 5R",
                       "High",
                       "DMX-Funktionen: Pan, Tilt, Farben, noch viel mehr",
                       channels,
                       "lamp");
    channels.clear();

    channels << "Red (0-100%)~0~255" << "Green (0-100%)~0~255" << "Blue (0-100%)~0~255" << "Weiß (0-100%)~0~255"
             << "Amber (0-100%)~0~255";
    create_new_fixture("Varytec PAD7 seventy",
                       "Mid",
                       "DMX-Funktionen: Farben",
                       channels,
                       "lamp");
    channels.clear();

    channels << "Master dimmer (0-100%)~0~255" << "Strobo~0~255" << "Red (0-100%)~0~225" << "Green (0-100%)~0~225"
             << "Blue (0-100%)~0~225" << "White (0-100%)~0~225"
             << "Blackout/colour mix CH3 - CH5~0~4|Red~5~15|Green~16~26|Blue~27~37|Yellow~38~48|Magenta~49~59|Cyan~60~70|White~71~80|Color change (rate)~81~150|Color blending~151~220";
    create_new_fixture("TOURSPOT PRO",
                       "High",
                       "DMX-Funktionen: Farben",
                       channels,
                       "lamp");
    channels.clear();

    channels << "Blackout/colour mix CH3 - CH5~0~4|Red~5~15|Green~16~26|Blue~27~37|Yellow~38~48|Magenta~49~59|Cyan~60~70|White~71~80|Color change (rate)~81~150|Color blending~151~220"
            << "Master dimmer (0-100%)~0~255" << "Strobo~0~255" << "Red (0-100%)~0~225" << "Green (0-100%)~0~225" << "Blue (0-100%)~0~225";
    create_new_fixture("BAR TRI-LED",
                       "Ambient",
                       "DMX-Funktionen: Farben",
                       channels,
                       "lamp");

#if defined(_WIN32) || defined(WIN32)
    fixture_objects_file.open(QFile::ReadWrite);
    fixture_objects_file.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileBirthTime);
    fixture_objects_file.close();
#endif
  }
}

/**
 * Creats a new Fixture.
 * @brief MainWindow::createnewFixture
 * @param _name
 * @param _type
 * @param _description
 * @param _channel_count
 */
void MainWindow::create_new_fixture(string _name,
                                    string _type,
                                    string _description,
                                    QStringList _channels,
                                    std::string _icon,
                                    int start_channel) {
  Fixture temp;
  temp.set_name(_name);
  temp.set_type(_type);
  temp.set_description(_description);
  temp.set_channels(_channels);
  temp.set_icon(_icon);
  if (start_channel == 0) {
    fixtures.push_back(temp);
    save_fixture_objects_to_xml();
  } else {
    add_fixture(&universe_tree.back(), temp, start_channel, QString::fromStdString(temp.get_type()));
  }
}

void MainWindow::update_label_song_position_and_duration(qint64 position) {
  QString current_song_name;
  if (player->get_playlist_media_at(player->playlist_index())->get_song()->get_title() != ""
      && player->get_playlist_media_at(player->playlist_index())->get_song()->get_artist() != "")
    current_song_name =
        QString::fromStdString(player->get_playlist_media_at(player->playlist_index())->get_song()->get_artist())
            + " - "
            + QString::fromStdString(player->get_playlist_media_at(player->playlist_index())->get_song()->get_title());
  else
    current_song_name =
        QString::fromStdString(player->get_playlist_media_at(player->playlist_index())->get_song()->get_song_name());
  QString current_song_position = player->format_time(position);
  song_duration = player->get_formatted_time();
  current_song_and_duration = current_song_position + " / " + song_duration;
  label_current_song_and_duration->setText(current_song_name + "     " + current_song_and_duration);

  if (current_song_position == song_duration && position != 0) {
    player->next_song();
    disconnect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);

    /*QString wanted_file_name =
        QString::fromStdString(player->get_playlist_media_at(combobox_edit_chosen_song->currentIndex())->get_song()->get_song_name());*/

    if (get_current_dmx_device().is_connected()) {
      get_current_dmx_device().stop_device();
      lightshow_playing = false;
      lightshow_paused = true;
      Logger::debug(player->get_current_song()->get_song_name());
      if (player->is_media_playing())
        this->start_to_play_lightshow();
    } else if (player->is_media_playing()) {
      player->play_song();
    }
  }

}

void MainWindow::on_add_fixture_button_clicked() {
  if (universes[0].get_fixture_count() >= 512) {
    QMessageBox::question(this, "Zu viele Geräte",
                          "Reached maximum amount of Fixtures. Only 512 Devices allowed.",
                          QMessageBox::Ok);
  } else {
    fcd = new FixtureChoosingDialog(this, fixtures);
    fcd->set_up_dialog_options(universes[0].get_blocked_adress_range());
    connect(fcd, SIGNAL(accepted()), this, SLOT(get_fixture_for_universe()));
    fcd->exec();
  }
}

void MainWindow::get_fixture_for_universe() {
  // Adds the chosen Fixture from the dialog.
  int fixture_index = 0;
  int start_channel = 0;
  QString type;
  fcd->get_fixture_options(fixture_index, start_channel, type);
  add_fixture((&universe_tree.back()), *(std::next(fixtures.begin(), fixture_index)), start_channel, type);
  save_fixture_objects_to_xml(false);
  fixtures_changed = true;
}

void MainWindow::on_fixture_list_itemSelectionChanged() {
  // Toplevelitems should not have a description.
  if (ui->fixture_list->currentItem()->parent() && ui->fixture_list->currentItem()->parent()->parent()) {
    ui->fixture_details->setPlainText(QString::fromStdString(
        universes[0].get_fixture(universes[0].get_fixtureid_by_startchannel(
            ui->fixture_list->currentItem()->text(1).split(" ")[0]
                .toInt())).get_description_view()));
  } else {
    ui->fixture_details->setPlainText("");
  }
}

/**
 * Changes the window to the fixture managemenet view
 *
 * @brief MainWindow::on_action_fixture_management_triggered
 */

void MainWindow::on_action_fixture_management_triggered() {
  ui->stackedWidget->setCurrentIndex(3);
  hide_edit_tools();
}


/**
 * Changes the window to the player view
 *
 * @brief MainWindow::on_action_player_view_triggered
 */
void MainWindow::on_action_player_view_triggered() {
  has_fixture_changed();
  ui->stackedWidget->setCurrentIndex(1);
  show_edit_tools();
}

void MainWindow::start_to_play_lightshow() {
  Logger::info("Start to play Lightshow");
  usleep(625 * this->lightshow_resolution + 1);

  read_lightshow = lightShowRegistry.get_lightshow(player->get_current_song());
  if (read_lightshow != nullptr) {
#ifndef __arm__
    disconnect(this->player, &MusicPlayer::position_changed, this->lightshow_player, &LightshowPlayer::send_new_dmx_data);
    connect(this->player,
            &MusicPlayer::position_changed,
            this->lightshow_player,
            &LightshowPlayer::send_new_dmx_data,
            Qt::QueuedConnection);
    Logger::info("connected positionChanged to lightshow_player");
#endif
    lightshow_playing = true;
    lightshow_paused = false;
    try {
      std::thread t(&LightshowPlayer::play_lightshow,
                    lightshow_player,
                    read_lightshow.get(),
                    std::ref(get_current_dmx_device()),
                    std::ref(lightshow_playing),
                    std::ref(lightshow_paused),
                    player);
      t.detach();
    } catch (const std::bad_alloc &e) {
      Logger::error("Allocation failed mainwindow: {}", e.what());
    }
  } else {
    Logger::info("read_lightshow is nullptr, only playing Audio");
#ifndef __arm__
    disconnect(this->player,
               &MusicPlayer::position_changed,
               this->lightshow_player,
               &LightshowPlayer::send_new_dmx_data);
    Logger::info("disconnected positionChanged to lightshow_player");
#endif
    player->play_song();
  }

}

void MainWindow::on_action_switch_play_pause_triggered() {
  if (default_is_active == false) {
    Logger::debug("on_action_switch_play_pause_triggered");


    //if(player->state() == QMediaPlayer::PausedState | player->state() == QMediaPlayer::StoppedState){
    if (!player->is_media_playing()) {

      if (get_current_dmx_device().is_connected()) {
        get_current_dmx_device().start_device();
        if (lightshow_playing) {
          lightshow_paused = false;
          player->play_song();
          Logger::info("resumed lightshow");
          ui->action_switch_play_pause->setIcon(QIcon(":/icons_svg/svg/iconfinder_004_-_Pause_2949873.svg"));
        } else {
          this->start_to_play_lightshow();

          ui->action_switch_play_pause->setIcon(QIcon(":/icons_svg/svg/iconfinder_004_-_Pause_2949873.svg"));
        }
      } else {
        player->play_song();
        ui->action_switch_play_pause->setIcon(QIcon(":/icons_svg/svg/iconfinder_004_-_Pause_2949873.svg"));
      }
    } else {
      Logger::info("paused lightshow");
      lightshow_paused = true;
      player->pause_song();
      ui->action_switch_play_pause->setIcon(QIcon(":/icons_svg/svg/iconfinder_001_-_play_2949892.svg"));
    }
  } else {
    this->throw_error_dialog(ERROR_MESSAGE_NO_SONG);
  }
}

void MainWindow::on_action_stop_triggered() {
  ui->action_switch_play_pause->setIcon(QIcon(":/icons_svg/svg/iconfinder_001_-_play_2949892.svg"));
  lightshow_playing = false;
  get_current_dmx_device().turn_off_all_channels();
  player->stop_song();
  usleep(2 * 625 * this->lightshow_resolution
             + 1); // sleep extra to make sure everything really is turned off! 50ms if resolution is 40
  get_current_dmx_device().stop_device();
}

void MainWindow::on_delete_fixture_button_clicked() {
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
      save_fixture_objects_to_xml(false);
      ui->fixture_list->resizeColumnToContents(0);
      fixtures_changed = true;
    }
  }
}

void MainWindow::on_create_fixture_button_clicked() {
  create_dialog = new CreateFixtureDialog();
  connect(create_dialog, SIGNAL(accepted()), this, SLOT(get_fixture_from_dialog()));
  create_dialog->exec();

}

void MainWindow::get_fixture_from_dialog() {
  std::string name;
  std::string type;
  std::string description;
  QStringList channels;
  std::string icon;
  create_dialog->create_fixture_dialog_finished(name, type, description, channels, icon);
  if (name.empty() || type.empty() || description.empty() || channels.contains("")) {

  } else {
    create_new_fixture(name, type, description, channels, icon);
  }
}

void MainWindow::on_go_to_player_button_clicked() {
  on_action_player_view_triggered();
  ui->menuBar->show();
  ui->mainToolBa->show();
  ui->statusBar->show();
}

void MainWindow::on_go_to_fixtures_button_clicked() {
  on_action_fixture_management_triggered();
  ui->menuBar->show();
  ui->mainToolBa->show();
  ui->statusBar->show();
}

void MainWindow::on_exit_client_button_clicked() {
  this->close();
}

void MainWindow::on_action_add_song_to_player_triggered() {

  if (universes[0].get_fixture_count() != 0) {
    QFileDialog file_dialog(this);
    file_dialog.setWindowTitle(tr("Open Files"));
    QStringList supported_name_filters;
    supported_name_filters << "Audio files (*.wav)"
                           << "Playlist files (*.m3u)";
      if(Mp3ToWavConverter::is_avaible()){
        supported_name_filters << "Audio files (*.mp3)";
      }
    file_dialog.setNameFilters(supported_name_filters);
    file_dialog.setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog.setFileMode(QFileDialog::ExistingFiles);
    file_dialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0,
                                                                                                    QDir::homePath()));
    if (file_dialog.exec() == QDialog::Accepted) {
        std::vector<Song*> temp = player->add_to_playlist(file_dialog.selectedUrls().toVector().toStdVector()) ;

        for(Song *song : temp){
            lightshows_to_generate_for.push_back({false, song});
        }

        this->start_thread_for_generating_queue();
      }
  } else {
    throw_error_dialog(ERROR_MESSAGE_NO_FIXTURES);
  }

  if (ui->stackedWidget->currentIndex() == 1)
    show_edit_tools();
}

void MainWindow::start_thread_for_generating_queue(){
    if(ls_generating_thread_is_alive == false){
        std::thread t(&MainWindow::queue_for_generating_light_show, this);
        t.detach();
    }
}

void MainWindow::queue_for_generating_light_show(){
    ls_generating_thread_is_alive = true;
    Logger::trace("MainWindow::queue_for_generating_light_show");

    for(ls_generating_parameter lightshow_parameter : lightshows_to_generate_for){
        if(!lightshow_parameter.is_regenerate)
            generate_lightshow(lightshow_parameter.song);
        else
            regenerate_lightshow(lightshow_parameter.song);
    }
    ls_generating_thread_is_alive = false;
    lightshows_to_generate_for.clear();
    Logger::trace("ENDE => MainWindow::queue_for_generating_light_show");
}


void MainWindow::generate_lightshow(Song *song) {
  //Logger::info("K8062 connected: {}", dmx_device_k8062.is_connected());
  std::shared_ptr<Lightshow>
      generated_lightshow = LightshowGenerator::generate(this->lightshow_resolution, song, universes[0].get_fixtures());

  lightShowRegistry.register_lightshow_file(song, generated_lightshow, this->lightshows_directory_path);
  if(player->playlist_index_for(song) != -1)
    emit lightshow_for_song_is_ready(song);
}

void MainWindow::regenerate_lightshow(Song *song) {
  //Logger::info("K8062 connected: {}", dmx_device_k8062.is_connected());
  std::shared_ptr<Lightshow> regenerated_lightshow =
      LightshowGenerator::generate(this->lightshow_resolution, song, universes[0].get_fixtures());

  Logger::debug("lightshow length: {}", regenerated_lightshow->get_length());

  lightShowRegistry.renew_lightshow_for_song(song, regenerated_lightshow, this->lightshows_directory_path);
  if(player->playlist_index_for(song) != -1)
    emit lightshow_for_song_is_ready(song);
}

void MainWindow::on_action_lightshow_test_triggered() {
  test_xml_and_dmx_output(this);
}

void MainWindow::on_action_next_song_triggered() {
  has_fixture_changed();

  player->next_song();

  disconnect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);

  if (player->get_playlist_media_at(combobox_edit_chosen_song->currentIndex())->get_lightshow_status()) {
    Song *song = player->get_playlist_media_at(combobox_edit_chosen_song->currentIndex())->get_song();
    QString label = QString::fromStdString(song->get_artist()) + " - " + QString::fromStdString(song->get_title());
    if (player->playlist_index() != combobox_edit_chosen_song->currentIndex()){
      player_edit_view->change_scene(false, label, lightShowRegistry.get_lightshow(song));
    }
    else {
        player_edit_view->change_scene(true, label, lightShowRegistry.get_lightshow(song));
        connect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);
    }
  } else{
     player_edit_view->init_waiting_or_dmx_not_connected_scene(true);
  }

  if (get_current_dmx_device().is_connected()) {
    lightshow_playing = false;
    lightshow_paused = true;
    get_current_dmx_device().turn_off_all_channels();
    get_current_dmx_device().stop_device();
    Logger::debug(player->get_current_song()->get_file_path());
    if (player->is_media_playing())
      this->start_to_play_lightshow();
  } else if (player->is_media_playing()) {
    player->play_song();
  }
}

void MainWindow::on_action_previous_song_triggered() {
  has_fixture_changed();
  player->previous_song();

  disconnect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);

  if (player->get_playlist_media_at(combobox_edit_chosen_song->currentIndex())->get_lightshow_status()) {
    Song *song = player->get_playlist_media_at(combobox_edit_chosen_song->currentIndex())->get_song();
    QString label = QString::fromStdString(song->get_artist()) + " - " + QString::fromStdString(song->get_title());
    if (player->playlist_index() != combobox_edit_chosen_song->currentIndex()){
        player_edit_view->change_scene(false, label, lightShowRegistry.get_lightshow(song));
    }
    else {
        player_edit_view->change_scene(true, label, lightShowRegistry.get_lightshow(song));
        connect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);
    }
  } else {
    player_edit_view->init_waiting_or_dmx_not_connected_scene(true);
  }

  if (get_current_dmx_device().is_connected()) {
    lightshow_playing = false;
    lightshow_paused = true;
    get_current_dmx_device().turn_off_all_channels();
    get_current_dmx_device().stop_device();
    if (player->is_media_playing())
      this->start_to_play_lightshow();
  } else if (player->is_media_playing()) {
    player->play_song();
  }
}

void MainWindow::on_action_playlist_view_triggered() {
  has_fixture_changed();
  ui->stackedWidget->setCurrentIndex(2);
  hide_edit_tools();
}

void MainWindow::play_next_song() {
  //has_fixture_changed();
  if (player->is_media_playing()) {
    Logger::info("fade to next song");
    if (get_current_dmx_device().is_connected()) {

      player->pause_song();
      lightshow_playing = false;
      lightshow_paused = true;
      usleep(625 * this->lightshow_resolution);
      this->start_to_play_lightshow();
    }
  }
}

void MainWindow::hide_edit_tools() {
  ui->action_activate_grid->setVisible(false);
  ui->mainToolBa->actions().last()->setVisible(false);
}

void MainWindow::show_edit_tools() {
  ui->action_activate_grid->setVisible(true);
  ui->mainToolBa->actions().last()->setVisible(true);
}

void MainWindow::change_player_edit_view(int index) {
  has_fixture_changed();
  disconnect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);

  Song *song = player->get_playlist_media_at(index)->get_song();
  QString label = QString::fromStdString(song->get_artist()) + " - " + QString::fromStdString(song->get_title());

  if (player->get_playlist_media_at(index)->get_lightshow_status()) {
    if (player->get_current_song()->get_file_path()
        == player->get_playlist_media_at(index)->get_song()->get_file_path()) {
      std::shared_ptr<Lightshow> ls = lightShowRegistry.get_lightshow(song);

      if(ls){
        player_edit_view->change_scene(true, label, ls);
        Logger::info("Mainwindow::change_player_edit_view");
        Logger::info(ls.get()->get_sound_src());
        //Logger::info(ls->get_value_changes_bass().at(0).time);
        //Logger::info(ls->get_value_changes_bass().at(0).value);
        connect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);
      }
    } else {
        std::shared_ptr<Lightshow> ls = lightShowRegistry.get_lightshow(song);
        if(ls){
          player_edit_view->change_scene(false, label, ls);
        }
    }
  } else {
    player_edit_view->init_waiting_or_dmx_not_connected_scene(true);
  }
}

void MainWindow::read_own_m3u_on_startup() {
  if (universes[0].get_fixture_count() != 0)
    player->read_own_m_3_u_on_startup(rtl_path.toStdString());

  int i = 0;
  while (player->get_playlist_media_at(i) != nullptr) {
    Song *song = player->get_playlist_media_at(i)->get_song();
    Logger::debug(song->get_song_name());
    Logger::info("Read from m3u: {}", song->get_song_name());
    this->lightShowRegistry.register_song(song, this->lightshows_directory_path);
    const shared_ptr<Lightshow>
        lightshow = lightShowRegistry.get_lightshow(song);
    
    if(!lightshow) {
      Logger::debug("Lightshoe for Song from m3u does not exist");
      lightshows_to_generate_for.push_back({true, song});
      start_thread_for_generating_queue();
    } else {
      Logger::debug("Song and Lightshow from m3u do exist");
      emit lightshow_for_song_is_ready(song);
      //player->get_playlist_media_at(i)->set_lightshow_status(true);
    }
    
    i++;
  }

}

void MainWindow::add_player_edit_mode(int start, int end) {
  if (universes[0].get_fixture_count() != 0) {
    if (default_is_active) {
      combobox_edit_chosen_song->removeItem(0);
      default_is_active = false;
    }


    QString temp;
    for (int index = start; index <= end; index++) {
      if (player->get_playlist_media_at(index)->get_song()->get_title() != ""
          && player->get_playlist_media_at(index)->get_song()->get_artist() != "") {
        temp = QString::fromStdString(player->get_playlist_media_at(index)->get_song()->get_artist() + " - "
                                          + player->get_playlist_media_at(index)->get_song()->get_title());
      } else {
        temp = QString::fromStdString(player->get_playlist_media_at(index)->get_song()->get_song_name());
      }

      combobox_edit_chosen_song->addItem(temp);
      //this->change_player_edit_view(combobox_edit_chosen_song->count() - 1);

      if (index == player->playlist_index()) {
        if (player->get_playlist_media_at(index)->get_lightshow_status()) {
          Song *song = player->get_playlist_media_at(index)->get_song();
          std::shared_ptr<Lightshow> ls = lightShowRegistry.get_lightshow(song);
          Logger::trace("Mainwindow::add_player_edit_view");
          Logger::debug(ls->get_value_changes_bass()[0].time);
          Logger::debug(ls->get_value_changes_bass()[0].value);
          Logger::debug(ls->get_value_changes_bass()[1].time);
          Logger::debug(ls->get_value_changes_bass()[1].value);
          Logger::debug(ls->get_value_changes_bass()[2].time);
          Logger::debug(ls->get_value_changes_bass()[2].value);

          player_edit_view->initialize_player_edit_scene(true, temp, ls);
          connect(player, &MusicPlayer::position_changed, player_edit_view, &Player_edit_view::make_music_slider_move);
        } else {
          player_edit_view->init_waiting_or_dmx_not_connected_scene(true);
        }
      }
    }

    add_to_playlist_view(start, end);

    // connect has to be initialized here because the QComboBox has no List to display earlier
    connect(combobox_edit_chosen_song,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) { this->change_player_edit_view(index); });
  } else {
    this->throw_error_dialog(ERROR_MESSAGE_NO_FIXTURES);
  }
}

void MainWindow::add_to_playlist_view(int start, int end) {
  for (int index = start; index <= end; index++) {
    playlist_view->add_to_playlist_view(player->get_playlist_media_at(index));
  }
}

void MainWindow::save_fixture_objects_to_xml(bool is_preset) {
  std::list<Fixture> saved_fixtures;
  if (is_preset) {
    saved_fixtures = fixtures;
  } else {
    saved_fixtures = universes[0].get_fixtures();
  }
  tinyxml2::XMLDocument fixture_objects;
  tinyxml2::XMLNode *toplevel_node = fixture_objects.NewElement("Fixtures");

  fixture_objects.InsertFirstChild(toplevel_node);
  for (auto fixture : saved_fixtures) {
    tinyxml2::XMLElement *fixture_object = fixture_objects.NewElement("Fixture");
    toplevel_node->InsertEndChild(fixture_object);

    tinyxml2::XMLElement *xml_name = fixture_objects.NewElement("name");
    xml_name->SetText(fixture.get_name().c_str());
    fixture_object->InsertEndChild(xml_name);

    tinyxml2::XMLElement *xml_type = fixture_objects.NewElement("type");
    xml_type->SetText(fixture.get_type().c_str());
    fixture_object->InsertEndChild(xml_type);

    tinyxml2::XMLElement *xml_description = fixture_objects.NewElement("description");
    xml_description->SetText(fixture.get_description().c_str());
    fixture_object->InsertEndChild(xml_description);

    if (!is_preset) {
      tinyxml2::XMLElement *xml_start_channel = fixture_objects.NewElement("startchannel");
      xml_start_channel->SetText(fixture.get_start_channel());
      fixture_object->InsertEndChild(xml_start_channel);
    }
    for (auto channel : fixture.get_channels()) {
      tinyxml2::XMLElement *xml_channel = fixture_objects.NewElement("channel");
      fixture_object->InsertEndChild(xml_channel);

      auto functions = QString::fromStdString(channel.to_string()).split("|");
      for (auto function : functions) {
        tinyxml2::XMLElement *xml_function = fixture_objects.NewElement("function");
        xml_channel->InsertEndChild(xml_function);
        auto function_detail = function.split("~");
        if (function_detail.size() == 3) {
          tinyxml2::XMLElement *xml_function_name = fixture_objects.NewElement("function_name");
          xml_function_name->SetText(function_detail.at(0).toLocal8Bit().data());
          xml_function->InsertEndChild(xml_function_name);

          tinyxml2::XMLElement *xml_function_from = fixture_objects.NewElement("from");
          xml_function_from->SetText(function_detail.at(1).toLocal8Bit().data());
          xml_function->InsertEndChild(xml_function_from);

          tinyxml2::XMLElement *xml_function_to = fixture_objects.NewElement("to");
          xml_function_to->SetText(function_detail.at(2).toLocal8Bit().data());
          xml_function->InsertEndChild(xml_function_to);
        }
      }
    }
    tinyxml2::XMLElement *xml_icon = fixture_objects.NewElement("icon");
    xml_icon->SetText(fixture.get_icon().c_str());
    fixture_object->InsertEndChild(xml_icon);
  }
  std::string fixture_objects_path = rtl_path.toLocal8Bit().toStdString();
  if (is_preset) {
    // Ein Fixture wird geschrieben.
    fixture_objects_path = fixture_objects_path + fixture_objects_file_name.toLocal8Bit().toStdString();
  } else {
    fixture_objects_path = fixture_objects_path + fixture_list_file_name.toLocal8Bit().toStdString();
  }
  fixture_objects.SaveFile(fixture_objects_path.c_str());
}

void MainWindow::load_fixture_objects_from_xml(bool is_preset, QString *filename) {
  std::string fixture_name;
  std::string fixture_type;
  std::string fixture_description;
  std::string fixture_icon;
  std::string functions;
  std::string function_name;
  std::string function_from;
  std::string function_to;
  QStringList channels;
  int start_channel = 0;

  tinyxml2::XMLDocument fixture_objects;
  tinyxml2::XMLError error;
  if (filename != nullptr) {
    error = fixture_objects.LoadFile(filename->toLocal8Bit().data());
  } else if (is_preset) {
    std::string path = rtl_path.toLocal8Bit().toStdString() + fixture_objects_file_name.toLocal8Bit().toStdString();
    error = fixture_objects.LoadFile(path.c_str());
  } else {
    std::string path = rtl_path.toLocal8Bit().toStdString() + fixture_list_file_name.toLocal8Bit().toStdString();
    error = fixture_objects.LoadFile(path.c_str());
  }
  if (xml_has_no_error(error)) {
    tinyxml2::XMLNode *toplevel_node = fixture_objects.FirstChild();
    if (toplevel_node == nullptr) { Logger::error("Error!"); }
    else {
      tinyxml2::XMLElement *fixture = toplevel_node->FirstChildElement("Fixture");
      while (fixture != nullptr) {
        tinyxml2::XMLElement *name = fixture->FirstChildElement("name");
        fixture_name = name->GetText();

        tinyxml2::XMLElement *type = fixture->FirstChildElement("type");
        fixture_type = type->GetText();

        tinyxml2::XMLElement *description = fixture->FirstChildElement("description");
        fixture_description = description->GetText();

        if (!is_preset) {
          tinyxml2::XMLElement *start_channel_xml = fixture->FirstChildElement("startchannel");
          start_channel = std::atoi(start_channel_xml->GetText());
        }

        tinyxml2::XMLElement *icon = fixture->FirstChildElement("icon");
        fixture_icon = icon->GetText();

        tinyxml2::XMLElement *channels_xml = fixture->FirstChildElement("channel");
        while (channels_xml != nullptr) {
          tinyxml2::XMLElement *functions_xml = channels_xml->FirstChildElement("function");
          while (functions_xml != nullptr) {
            tinyxml2::XMLElement *fname = functions_xml->FirstChildElement("function_name");
            function_name = fname->GetText();

            tinyxml2::XMLElement *from = functions_xml->FirstChildElement("from");
            function_from = from->GetText();

            tinyxml2::XMLElement *to = functions_xml->FirstChildElement("to");
            function_to = to->GetText();

            if (!functions.empty()) { functions.append("|"); }
            functions.append(function_name + "~" + function_from + "~" + function_to);
            functions_xml = functions_xml->NextSiblingElement("function");
          }
          channels << QString::fromStdString(functions);
          channels_xml = channels_xml->NextSiblingElement("channel");
          functions.clear();
        }
        fixture = fixture->NextSiblingElement("Fixture");
        create_new_fixture(fixture_name, fixture_type, fixture_description, channels, fixture_icon, start_channel);
        channels.clear();
      }
    }
  } else {
    fixture_objects.PrintError();
  }
}

void MainWindow::throw_error_dialog(QString error_message) {
  error_dialog = new QDialog(this);
  error_dialog_layout = new QVBoxLayout(error_dialog);
  error_dialog_label = new QLabel();
  error_dialog_label->setText(error_message);
  error_dialog_layout->addWidget(error_dialog_label);
  error_dialog_button = new QPushButton("&Okay!", error_dialog);
  error_dialog_layout->addWidget(error_dialog_button);
  connect(error_dialog_button, &QPushButton::clicked, error_dialog, &QDialog::close);
  error_dialog->exec();
}

void MainWindow::change_song_in_combobox_availibility(int index) {
  const QStandardItemModel *model = dynamic_cast< QStandardItemModel * >(combobox_edit_chosen_song->model());
  QStandardItem *item = model->item(index);
  item->setEnabled(false);
}

void MainWindow::ShowContextMenu(const QPoint &pos) {
  int cur_item = 0;
  // If no Item was selected this returns false.
  if (ui->fixture_list->itemAt(pos) != nullptr) {
    cur_item = ui->fixture_list->currentIndex().row();
  }

  QMenu contextMenu(tr("Context menu"), this);
  QAction remove("Remove", this);

  QMenu *type_menu = contextMenu.addMenu("Type");
  QAction *type_action = type_menu->addAction("Ambient");
  QAction *type_bass = type_menu->addAction("Bass");
  //QAction * type_action = type_menu->addAction("Action");
  QAction *type_middle = type_menu->addAction("Mid");
  QAction *type_high = type_menu->addAction("High");

  QSignalMapper *signalMapper = new QSignalMapper(this);
  QString type_pos = "Bass";
  type_pos.append(cur_item);
  signalMapper->setMapping(type_bass, type_pos);

  connect(type_bass, SIGNAL(triggered()), signalMapper, SLOT(map()));

  connect(signalMapper, SIGNAL(mapped(QObject * )), this, SLOT(set_fixture_type(QObject * )));
  contextMenu.addAction(&remove);

  contextMenu.exec(mapToGlobal(pos));
}

void MainWindow::set_fixture_type(QObject *type_pos) {

}

bool MainWindow::xml_has_no_error(tinyxml2::XMLError error) {
  if (error != tinyxml2::XML_SUCCESS) {
    printf("Error: %i\n", error);
    return false;
  } else {
    return true;
  }
}

void MainWindow::update_fixture_list() {
  QStringList types = (QStringList() /*<< "Action"*/ << "Ambient" << "Bass" << "Mid" << "High");

  for (auto type : types) {
    QList<QTreeWidgetItem *> type_items = ui->fixture_list->findItems(QString::fromStdString(type.toStdString()),
                                                                      Qt::MatchExactly | Qt::MatchRecursive,
                                                                      0);
    if (type_items.size() != 0) {
      for (int i = 0; i < type_items.size(); i++) {
        (*type_items.at(i)).takeChildren();
        std::list<Fixture> type_fixtures = universes[0].get_fixture_by_type((type.toStdString()));
        for (int j = 0; j < type_fixtures.size(); j++) {
          Fixture current_type_fixture = *std::next(type_fixtures.begin(), j);
          QTreeWidgetItem *item = new QTreeWidgetItem();
          item->setText(0, QString::fromStdString(current_type_fixture.get_name()));
          item->setText(1, QString::number(current_type_fixture.get_start_channel())
              + " - " + QString::number(current_type_fixture.get_last_channel()));
          item->setIcon(0,
                        QIcon(
                            ":icons_svg/svg/" + QString::fromStdString(current_type_fixture.get_icon()) + ".svg"));
          type_items.at(i)->addChild(item);
        }
      }
    }
  }
}

QString MainWindow::get_home() {
  QString home_path;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  home_path = strcat(getenv("HOMEDRIVE"), getenv("HOMEPATH"));
  home_path = home_path + "\\";
#else
  home_path = getenv("HOME");
  home_path = home_path + "/";
#endif
  return home_path;
}

bool MainWindow::has_fixture_changed() {
  bool redo_lightshow = false;

  if (fixtures_changed && player->get_playlist_length() > 0) {
    QMessageBox::StandardButton fixture_changed_button = QMessageBox::question(this, "Redo Lightshow?",
                                                   tr("The Fixtures have changed.\nDo You want to create the Lightshow again?"),
                                                   QMessageBox::No | QMessageBox::Yes);
    fixtures_changed = false;

    if (fixture_changed_button == QMessageBox::Yes) {
      redo_lightshow = true;
      Logger::info("renewing {} lightshows", player->get_playlist_length());
      for (int i = 0; i < player->get_playlist_length(); i++) {
        Song *song = player->get_playlist_media_at(i)->get_song();
        Logger::info("renewing lightshow for song {}", song->get_song_name());

        lightshows_to_generate_for.push_back({true, song});
        start_thread_for_generating_queue();
      }

    }
  }
  return redo_lightshow;
}

void MainWindow::on_edit_fixture_clicked() {
  // Only delete if an Item was clicked.
  if (!ui->fixture_list->selectedItems().empty()) {
    // Toplevelitems should not have a description.
    if (ui->fixture_list->currentItem()->parent()) {
      create_dialog = new CreateFixtureDialog();
      create_dialog->edit_fixture_dialog(universes[0].get_fixture(universes[0].get_fixtureid_by_startchannel(
          ui->fixture_list->currentItem()->text(1).split(" ")[0]
              .toInt())),
                                         universes[0].get_blocked_adress_range(), false);
      connect(create_dialog, SIGNAL(accepted()), this, SLOT(get_edited_fixture()));
      create_dialog->exec();
    }
  }
}

void MainWindow::get_edited_fixture() {
  Fixture edited_fixture;
  create_dialog->edit_fixture_finished(edited_fixture);
  universes[0].set_fixture(
      edited_fixture,
      universes[0].get_fixtureid_by_startchannel(
          ui->fixture_list->currentItem()->text(1).split(" ")[0].toInt()));
  update_fixture_list();
  on_fixture_list_itemSelectionChanged();
  save_fixture_objects_to_xml(false);
  ui->fixture_list->resizeColumnToContents(0);
  fixtures_changed = true;
}

void MainWindow::on_actionFixture_Presets_bearbeiten_triggered() {
  fcd = new FixtureChoosingDialog(this, fixtures);
  fcd->setup_for_edit();
  connect(fcd, SIGNAL(accepted()), this, SLOT(open_edit_preset()));
  fcd->exec();
}

void MainWindow::open_edit_preset() {
  choosen_preset_index = fcd->edit_preset_choosen();
  if (fcd->delete_fixture_preset()) {
    fixtures.erase(std::next(fixtures.begin(), choosen_preset_index));
    save_fixture_objects_to_xml();
  } else {
    create_dialog = new CreateFixtureDialog();
    create_dialog->edit_fixture_dialog(*std::next(fixtures.begin(), choosen_preset_index),
                                       universes[0].get_blocked_adress_range(),
                                       true);
    connect(create_dialog, SIGNAL(accepted()), this, SLOT(edit_preset()));
    create_dialog->exec();
  }
}

void MainWindow::edit_preset() {

  Fixture edited_preset;
  create_dialog->edit_fixture_finished(edited_preset);
  auto tmp_preset = fixtures.erase(std::next(fixtures.begin(), choosen_preset_index));
  fixtures.insert(tmp_preset, edited_preset);
  save_fixture_objects_to_xml();
}

void MainWindow::on_action_activate_grid_triggered() {
  player_edit_view->show_hide_grid();
}

void MainWindow::on_actionLade_Presetdatei_triggered() {
  QString filename = QFileDialog::getOpenFileName(this, "Open Presetfile", QDir::homePath(), ("XML: (*.xml)"));
  fixtures.clear();
  load_fixture_objects_from_xml(true, &filename);
  save_fixture_objects_to_xml();
}

void MainWindow::on_actionLade_Fixtures_triggered() {
  ui->fixture_list->clearSelection();
  QString filename = QFileDialog::getOpenFileName(this, "Open Fixturelist", QDir::homePath(), ("XML: (*.xml)"));
  universes[0].empty_fixtures();
  universe_tree.back().takeChildren();
  load_fixture_objects_from_xml(false, &filename);
  save_fixture_objects_to_xml(false);
  has_fixture_changed();
}

void MainWindow::rowsInserted(const QModelIndex &parent, int start, int end) {

  if (ui->fixture_list->currentItem() != nullptr && ui->fixture_list->currentItem() != ui->fixture_list->topLevelItem(0)) {
    int fixture_id = universes[0].get_fixtureid_by_startchannel(
        universe_tree.begin()->child((parent.row()))->
            child(start)->text(1).split(" ")[0].toInt());
    Fixture temp = universes[0].get_fixture(fixture_id);
    temp.set_type(universe_tree.begin()->child((parent.row()))->text(0).toStdString());
    universes[0].set_fixture(temp, fixture_id);
    fixtures_changed = true;
    save_fixture_objects_to_xml(false);
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(this->ui->stackedWidget->currentIndex() != 0){
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Saving?",
                                                             tr("Do you want to safe your playlist?\n"),
                                                             QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                             QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes) {
            if (resBtn == QMessageBox::Cancel)
                event->ignore();
            else
                event->accept();
        } else {
            player->save_playlist(rtl_path.toStdString());
            event->accept();
        }

        if (get_current_dmx_device().is_connected()) {
            //dmx_device_k8062.start_device();
            get_current_dmx_device().start_daemon_thread();
            lightshow_playing = false;
            usleep(625 * this->lightshow_resolution
               + 1); // sleep extra on close event to make sure everything really is turned off! 25ms if resolution = 40
            get_current_dmx_device().turn_off_all_channels();
            usleep(625 * this->lightshow_resolution
               + 1); // sleep extra on close event to make sure everything really is turned off! 25ms if resolution = 40
            get_current_dmx_device().stop_device();
        }
    }
    else {
        event->accept();
    }
}

void MainWindow::slot_song_title_or_artist_changed_update_combobox(int playlist_index) {
  std::string shown_in_combobox;
  if (player->get_playlist_media_at(playlist_index)->get_song()->get_title() != ""
      && player->get_playlist_media_at(playlist_index)->get_song()->get_artist() != "") {
    shown_in_combobox = player->get_playlist_media_at(playlist_index)->get_song()->get_artist() + " - "
        + player->get_playlist_media_at(playlist_index)->get_song()->get_title();
    this->combobox_edit_chosen_song->removeItem(playlist_index);
    this->combobox_edit_chosen_song->insertItem(playlist_index, QString::fromStdString(shown_in_combobox));
  }
}

void MainWindow::on_actionExit_triggered() {
  this->close();
}

void MainWindow::on_actionFullscreen_triggered() {
  this->setWindowState(Qt::WindowFullScreen);
}

void MainWindow::on_actionMaximized_triggered() {
  this->setWindowState(Qt::WindowMaximized);
}

void MainWindow::on_actionMinimize_triggered() {
  this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::on_actionAmbient_Fixtures_always_on_changed() {
  this->ambient_fixtures_always_on = ui->actionAmbient_Fixtures_always_on->isChecked();
  Logger::info("ambient_fixtures_always_on: {}", this->ambient_fixtures_always_on);
}

void MainWindow::on_actionAlways_generate_Lightshows_changed() {
  this->always_generate_lightshows = ui->actionAlways_generate_Lightshows->isChecked();
  Logger::info("always_generate_lightshows: {}", this->always_generate_lightshows);
}

void MainWindow::on_action_remove_song_from_playlist_triggered()
{
    /** !!!Löschen ist aktuell noch verbugt!!! **/

    Logger::trace("MainWindow::on_action_remove_song_from_playlist_triggered");
    if(ui->stackedWidget->currentIndex() != 2){
        ui->stackedWidget->setCurrentIndex(2);
    }
    else{
        int index_to_delete = playlist_view->delete_current_selected_song();
        Logger::trace("MainWindow::on_action_remove_song_from_playlist_triggered");
        Logger::trace(index_to_delete);

        if(this->player->playlist_index() == index_to_delete){
            /* if the user wants to delete the current playing song
             * play the next available song || in case it is the last song in the playlist
             * -> stop playing and then delete the song */

            // Case 1: current playing is not the last song in the playlist
            if(this->player->get_playlist_media_at(index_to_delete + 1) != nullptr){
                this->on_action_next_song_triggered();
                if(this->player->get_playlist_length() == 2)
                    this->player->set_playlist_index(0);
                else
                    this->player->set_playlist_index(this->player->playlist_index() - 1);
            }

            // Case 2: current playing song is not the first and the last song in the playlist
            if(index_to_delete != 0 && this->player->get_playlist_media_at(index_to_delete + 1) == nullptr){
                this->on_action_next_song_triggered();
                this->player->set_playlist_index(0);
            }

            // Case 3: current playing song is both the first and the last song in the playlist
            if(index_to_delete == 0 && this->player->get_playlist_media_at(index_to_delete + 1) == nullptr){
                QMessageBox warning;
                warning.setText("Please add another song before deleting the last!");
                warning.setWindowTitle("Warning!");
                warning.exec();
            }
        }


        this->player->delete_song_from_playlist(index_to_delete);
    }
}

void MainWindow::slot_shortcut_f11() {
  if (this->isFullScreen()) {
    this->setWindowState(Qt::WindowMaximized);
  } else {
    this->setWindowState(Qt::WindowFullScreen);
  }
}

void MainWindow::on_go_to_playlist_button_clicked()
{
    on_action_playlist_view_triggered();
    ui->menuBar->show();
    ui->mainToolBa->show();
    ui->statusBar->show();
}

void MainWindow::slot_order_playlist_changed(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index)
{
    QStringList s;
    int k = new_first_row_index;
    for(int i = old_first_row_index; i <= old_last_row_index; i++){
        s.append(this->combobox_edit_chosen_song->itemText(i));
        this->combobox_edit_chosen_song->insertItem(k, s.last());
        k++;
    }

    for(int i = old_last_row_index; i >= old_first_row_index; i--){
        this->combobox_edit_chosen_song->removeItem(i);
    }
    this->change_player_edit_view(0);
    player->slot_order_playlist_changed(old_first_row_index, old_last_row_index, new_first_row_index, new_last_row_index);
}

void MainWindow::on_action_safe_playlist_triggered()
{
    /* Save a playlist does not work properly
     * until then this action has been disabled*/
    QFileDialog file_dialog(this);
    file_dialog.setWindowTitle(tr("Choose file path"));
    file_dialog.setFileMode(QFileDialog::Directory);
    //file_dialog.setAcceptMode(QFileDialog::AcceptSave);
    file_dialog.setFileMode(QFileDialog::DirectoryOnly);
    if(file_dialog.exec() == QDialog::Accepted){
        std::string directory = file_dialog.directory().absolutePath().toStdString() + "/";
        player->save_playlist(directory);
    }
}

void MainWindow::on_action_switch_to_dark_mode_triggered()
{
    if(is_in_dark_mode){
        this->setStyleSheet("all: initial;");
        ui->fixture_list->setStyleSheet("background-color: rgb(238, 238, 236);");
        ui->fixture_details->setStyleSheet("background-color: rgb(238, 238, 236);");
        is_in_dark_mode = false;
    }
    else {
        ui->fixture_list->setStyleSheet("all: initial;");
        ui->fixture_details->setStyleSheet("all: initial;");

        /* got the darkmode from a stackoverflow and github:
         * 1) https://stackoverflow.com/questions/48256772/dark-theme-for-in-qt-widgets
         * 2) https://github.com/ColinDuquesnoy/QDarkStyleSheet
         */
        QFile f(":/dark-stylesheet/stylesheet/style.qss");
        if (!f.exists())
        {
            Logger::info("Unable to set stylesheet, file not found");
        }
        else
        {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            this->setStyleSheet(ts.readAll());
        }
        is_in_dark_mode = true;
    }
}


DmxDevice& MainWindow::get_current_dmx_device() {
  if(dmx_device_connected == DmxDeviceConnected::EUROLITE_PRO) {
    //Logger::debug("current dmx device: EUROLITE");
    return this->dmx_device_eurolite_pro;
  } else if(dmx_device_connected == DmxDeviceConnected::VELLEMAN_K8062) {
    //Logger::debug("current dmx device: K8062");
    return this->dmx_device_k8062;
  }
  else { // use k8062 pro as default, no idea how to do this better. K8062 is needed as default because else it won't work (probably timing problem on start?)
    //Logger::debug("No dmx device found, using default");
    return this->dmx_device_k8062;
  }
}

void MainWindow::check_which_dmx_device_is_connected() {
  int status = 0;
  libusb_context *ctx = nullptr;
  status = libusb_init(&ctx);
  if (status != LIBUSB_SUCCESS)
    Logger::error("failed to init libusb");

  // list all devices
  libusb_device **devs;
  libusb_device *dev;
  int k = 0;
  libusb_get_device_list(nullptr, &devs);
  while ((dev = devs[k++]) != nullptr) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      Logger::error("failed to read device descriptor");
    }

    //Logger::debug("idVendor: {}, idProduct: {}", desc.idVendor, desc.idProduct);
    if (desc.idVendor == FTDI_VENDOR_ID && desc.idProduct == FTDI_PRODUCT_ID) {
      Logger::debug("Using Eurolite Pro DMX Device");
      this->dmx_device_connected = DmxDeviceConnected::EUROLITE_PRO;
      return; // sobald Eurolite gefunden wurde nicht weitersuchen. Koennte ja ein schlechtes Velleman gefunden werden.
    } else if(desc.idVendor == K_8062_VENDOR_ID && desc.idProduct == K_8062_PRODUCT_ID) {
      Logger::debug("Using Velleman K8062 DMX Device");
      this->dmx_device_connected = DmxDeviceConnected::VELLEMAN_K8062;
    }

  }
  if(this->dmx_device_connected == DmxDeviceConnected::NOT_FOUND)
    Logger::debug("No connected DMX Device found!");
}
