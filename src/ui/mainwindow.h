#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <iterator>
#include <list>
#include <QTreeWidget>
#include <string>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QToolBar>
#include <QCloseEvent>

#include "player_edit_view/player_edit_view.h"
#include "fixturechoosingdialog.h"
#include "fixturemanager/universe.h"
#include "fixturemanager/fixture.h"
#include "volume_button.h"
#include "playlist_view/playlist_view.h"
#include "createfixturedialog.h"
#include "playlist_and_player/music_player.h"
#include "lightshow/light_show_registry.h"
#include "lightshow_player.h"
#include <tinyxml2.h>
#include <QShortcut>
#include <dmx_device_k_8062.h>
#include <dmx_device_eurolite_pro.h>
#include "lightshow_generator.h"
#include "edit_fixture_dialog.h"
#include "change_fixtures.h"

using namespace std;

namespace Ui {
    class MainWindow;
}

struct ls_generating_parameter
{
    bool is_regenerate;
    Song *song;
    std::list<Fixture> fixtures;
    int user_bpm;
    float onset_value;
};


class MainWindow : public QMainWindow
{

Q_OBJECT

 protected:
  enum class DmxDeviceConnected {
    NOT_FOUND = -1,
    VELLEMAN_K8062 = 0,
    EUROLITE_PRO = 1
  };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    /**
     * @brief universe_tree The Fixtures for the Tree Widget.
     * This is just for the Visual representation.
     */
    list<QTreeWidgetItem> universe_tree;

    /**
     * @brief fixtures The Basic Fixtureobject, that can be added to a universe.
     */
    list<Fixture> fixtures;

    /**
     * @brief universes Contains Fixtures for logic operations.
     * In here are Fixtures for a Lightshow. These Fixtures contain Information that
     * is used by other interfaces.
     */
    Universe universes[5];
    void change_fixtures_of_existing_song();
    void regenerate_lightshow_with_default_fixtures();

    signals:
    void lightshow_for_song_is_ready(Song *song);

private slots:
    void changed_fixtures_for_lightshow_ready(QUrl url, std::list<Fixture> _fixtures, int user_bpm, float onset_value);
    void changed_fixtures_for_existing_lightshow_ready(Song* song, std::list<Fixture> _fixtures, int user_bpm, float onset_value);
    void right_click_on_playlist_item(QPersistentModelIndex index);

    /**
     * @brief on_delete_fixture_button_clicked Slot that deletes a Fixture.
     * The Fixture will be deleted in the visual and the logic part.
     */
    void on_delete_fixture_button_clicked();

    /**
     * @brief on_add_fixture_button_clicked Adds a Fixture to the View and the Logic.
     */
    void on_add_fixture_button_clicked();

    /**
     * @brief on_fixture_list_itemSelectionChanged Sets the fixture_details for the selected Item.
     * If a Fixture was clicked or selected by Keyboard the "right side", the fixture detail will
     * display its corresponding Information.
     * A Univers will leave the details blank.
     */
    void on_fixture_list_itemSelectionChanged();

    void on_action_fixture_management_triggered();
    //void on_action_player_view_triggered();
    void on_action_switch_play_pause_triggered();
    void on_action_stop_triggered();
    //void on_go_to_player_button_clicked();
    void on_go_to_fixtures_button_clicked();
    void on_exit_client_button_clicked();
    void on_action_add_song_to_player_triggered();

    /**
     * @brief get_fixture_from_dialog Pulls the Information from the create dialog.
     */
    void get_fixture_from_dialog();

    void on_action_next_song_triggered();

    void on_action_previous_song_triggered();

    void on_action_playlist_view_triggered();
    void add_player_edit_mode(int start, int end);
    void add_to_playlist_view(int start, int end);
    void throw_error_dialog(QString error_message);
    void change_song_in_combobox_availibility(int index);

    /**
     * @brief ShowContextMenu Shows the custom Menu.
     * @param pos Position of the Menu.
     */
    void ShowContextMenu(const QPoint & pos);

    /**
     * @brief set_fixture_type Sets the given type for the given Fixture.
     * @param type_position Type for the Fixture and Index of the Fixture.
     */
    void set_fixture_type(QObject * type_position);

    /**
     * @brief on_edit_fixture_clicked The Dialog with the selected Fixture will be calles.
     */
    void on_edit_fixture_clicked();

    /**
     * @brief get_fixture_for_universe Get the function and details from the Fixture choosing dialog.
     */
    void get_fixture_for_universe();

    /**
     * @brief get_edited_fixture Gets the edited Fixture from the Dialog.
     */
    void get_edited_fixture();

    /**
     * @brief edit_preset opens up the create fixture dialog in preset edit mode.
     */
    void open_edit_preset();

    /**
     * @brief edit_preset Gets the Fixturepreset from the Dialog and edits it.
     */
    void edit_preset();

    void on_action_activate_grid_triggered();

    void on_actionLade_Fixtures_triggered();

    void on_action_open_current_dmx_device_triggered();

    void closeEvent(QCloseEvent *event) override;

    void slot_song_title_or_artist_changed_update_combobox(int playlist_index);

    /**
     * @brief rowsInserted A Fixture is dropped on an Type. The Type of that Fixture gets changed.
     * @param parent Model of the Type, the Fixture is Droped on.
     * @param start Position in the Type the Fixture is droped on.
     * @param end Position of the Type the Fixture is droped on, because only singledrag is allowed.
     */
    void rowsInserted(const QModelIndex &parent, int start, int end);

    void fixture_item_dropped(QDropEvent* event);

    void on_actionExit_triggered();

    void on_actionFullscreen_triggered();

    void on_actionMaximized_triggered();

    void on_actionMinimize_triggered();

    void on_actionAmbient_Fixtures_always_on_changed();

    void on_actionAlways_generate_Lightshows_changed();

    void on_action_remove_song_from_playlist_triggered();
    void slot_shortcut_f11();

    void on_go_to_playlist_button_clicked();
    void slot_order_playlist_changed(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index);

    void on_action_safe_playlist_triggered();

    void on_action_switch_to_dark_mode_triggered();

    void on_action_regenerate_lightshows_triggered();
    void on_action_ignite_discharge_lamps_triggered();
    void on_action_turn_off_discharge_lamps_triggered();

private:
    Ui::MainWindow *ui;

    /**
     * @brief fixture_objects_file_name The Filename of the Fixtureobjects.
     */
    QString fixture_objects_file_name = "fixture_objects.xml";

    /**
     * @brief fixture_list_file_name The Filename of the current Fixturelist.
     */
    QString fixture_list_file_name = "fixture_list.xml";

    MusicPlayer *player;

    Player_edit_view  *player_edit_view;
    Playlist_view *playlist_view;
    Volume_button *volume_button;
    QString current_song_and_duration;
    QString song_duration;
    QLabel *label_current_song_and_duration;
    CreateFixtureDialog * create_dialog;
    FixtureChoosingDialog * fcd;
    EditFixtureDialog * efd;
    ChangeFixtures * change_fixtures_dialog;
    QComboBox* combobox_edit_chosen_song;
    QStandardItemModel* combobox_model;
    bool default_is_active;
    QDialog *error_dialog;
    QVBoxLayout *error_dialog_layout;
    QLabel *error_dialog_label;
    QPushButton *error_dialog_button;

    LightShowRegistry lightShowRegistry;
    std::shared_ptr<Lightshow> read_lightshow;
    void check_which_dmx_device_is_connected();
    DmxDeviceConnected dmx_device_connected = DmxDeviceConnected ::NOT_FOUND;
    DmxDeviceK8062 dmx_device_k8062;
    DmxDeviceEurolitePro dmx_device_eurolite_pro;
    DmxDevice& get_current_dmx_device();
    LightshowPlayer *lightshow_player;
    int lightshow_resolution = 40; // 40 = DMX update every 25ms.    20 = DMX update every 50ms.    10 = DMX update every 100ms.
    LightshowGenerator lightshow_generator;

    QString sig_light_path = "";
    std::string lightshows_directory_path = "";
    std::string songs_directory_path = "";

    std::list<ls_generating_parameter> lightshows_to_generate_for;
    bool ls_generating_thread_is_alive;
    bool is_in_dark_mode;

    std::vector<std::string> color_palettes{"auto", "R", "G", "B", "C", "LG", "P", "Y", "O", "W", "R/Y/O", "B/C/W", "G/LG/C", "R/C/LG/B/P", "B/LG/C/R/G", "G/Y/R/W/B", "B/LG/P/C/R/G/Y/W", "LG/P/C/R/G/Y/W/B", "P/C/R/G/Y/W/B/LG", "C/R/G/Y/W/B/LG/P", "R/G/Y/W/B/LG/P/C", "G/Y/W/B/LG/P/C/R", "Y/W/B/LG/P/C/R/G", "W/B/LG/P/C/R/G/Y"};

    bool fixtures_changed = false;
    /**
     * @brief ambient_light_allways_on True if the Ambient Light should be allways on.
     * The user can choose if the ambient lights should be allways on.
     * True - The ambient Light is allways on.
     * False - It is not.
     */
    bool ambient_fixtures_always_on;
    /**
     * @brief always_generate_lightshows True if Lightshows should always be generated.
     * The user can choose if lightshows should always be generated, also if no DMX device is connected.
     * True - Lightshows always get generated.
     * False - Lightshows only get generated if a DMX device is connected.
     */
    bool always_generate_lightshows;

    /**
     * @brief choosen_preset Choosen Preset for editing.
     */
    int choosen_preset_index;

    QShortcut *key_f11;

    /**
     * @brief add_universe Adds a Universe to the View and array.
     * @param name Name of the Universe.
     * @param description Description of the Universe.
     *
     * Creats a Universe to handle Fixtures.
     */
    void add_universe(QString name);

    /**
     * @brief add_fixture Add Fixture to the View and Universe.
     * @param parent Toplevel Item representing its Universe.
     * @param _fixture The Fixture to added.
     * @param start_channel The start channel of the Fixture.
     * @param type Type of the Fixutre.
     */
    void add_fixture(QTreeWidgetItem *parent, Fixture _fixture, int start_channel, QString type, std::string _colors, int position_in_group, std::string position_on_stage, std::string moving_head_type, int modifier_pan, int modifier_tilt, std::string timestamps_type, int position_inside_mh_group, bool invert_tilt, int amplitude_pan, int amplitude_tilt);

    /**
     * @brief create_fixtures Creates the preset Fixtures or loads them if the Fixturefile exists.
     *
     * Creates some known Fixtures. If the Fixtureobjectsfile exists it will instead load the fixtures from that file.
     */
    void create_fixtures();

    /**
     * @brief create_new_fixture Creates a new Fixture and saves it.
     * @param name Name of the Fixture.
     * @param type Type of the Fixture.
     * @param description Description of the Fixture.
     * @param channels Channels with the functions of the Fixture.
     * @param icon_identifyer Icon identifyer of the Fixture.
     * @param start_channel Set the start channel of a Fixture.
     */
    void create_new_fixture(std::string name, std::string type, std::string description, QStringList channels,
                            std::string icon_identifyer, std::string colors, int position_in_group, std::string position_on_stage = "Left", std::string moving_head_type = "Nothing", int modifier_pan = 0, int modifier_tilt = 0, std::string timestamps_type = "onsets", int start_channel = 0, int position_in_mh_group = 0, bool invert_tilt = false, int amplitude_pan = 0, int amplitude_tilt = 0);

    /**
     * @brief init Initializes some settings of the UI and sets some Presets.
     * Sets Settings for the UI and creates a first Universe. Also Creates the known Fixtures.
     */
    void init();
    void init_toolbar();
    void init_status_bar();
    void init_connects();
    void update_label_song_position_and_duration(qint64 position);
    void start_to_play_lightshow();
    void play_next_song();
    void hide_edit_tools();
    void show_edit_tools();
    void change_player_edit_view(int index);
    void read_own_m3u_on_startup();

    /**
     * @brief save_fixture_objects_to_xml Saves the Fixture objects to an XML file.
     * @param is_preset If the object to save is a preset true .Defaut true.
     */
    void save_fixture_objects_to_xml(bool is_preset = true);

    /**
     * @brief load_fixture_objects_from_xml Loads Fixtures from an XML file.
     * @param is_preset If the object to save is a preset true .Defaut true.
     */
    void load_fixture_objects_from_xml(bool is_preset = true, QString * filename = nullptr);

    /**
     * @brief xml_has_no_error Checks for XML Errors and displays them.
     * @param error XLM Error
     * @return true if no errors. False if there are errors.
     */
    bool xml_has_no_error(tinyxml2::XMLError error);

    /**
     * @brief update_fixture_list
     */
    void update_fixture_list();

    /**
     * @brief get_home Cheks for the home folder on the curent OS.
     * @return Gives you the Homepath.
     */
    QString get_home();

    /**
     * @brief has_fixture_changed After Fixture changes ask if the Lightshow should be recreated and create it if wanted.
     */
    bool has_fixture_changed();

    void generate_lightshow(Song *song, std::list<Fixture> _fixtures, int user_bpm, float onset_value);
    void regenerate_lightshow(Song *song, std::list<Fixture> _fixtures, int user_bpm, float onset_value);
    void queue_for_generating_light_show();
    void start_thread_for_generating_queue();
    void init_shortcuts();
    void resize_fixture_list_columns();

    std::vector<int> get_all_pan_tilt_channels();
    std::vector<std::uint8_t> get_all_pan_tilt_channels_with_default_value();
    std::vector<std::uint8_t> get_control_channels_with_ignite_value();
    std::vector<std::uint8_t> get_control_channels_with_turn_off_value();

    QStringList lighting_types = (QStringList() << "auto_beats" << "group_auto_beats" << "auto_onsets" << "group_auto_onsets" << "Ambient" << "Bass" << "Mid" << "High" << "color_change" << "flash" << "flash_reverse" << "blink" << "pulse" << "group_one_after_another" << "group_one_after_another_fade" << "group_one_after_another_fade_reverse" << "group_one_after_another_fade_single" << "group_one_after_another_fade_single_reverse" << "group_one_after_another_blink" << "group_one_after_another_back_and_forth" << "group_one_after_another_back_and_forth_blink" << "group_two_after_another" << "group_alternate_odd_even" << "group_random_flashes" << "strobe_if_many_onsets");
};

#endif // MAINWINDOW_H
