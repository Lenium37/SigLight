#include "player_edit_view.h"


#define DEFAULT_SCENE_WIDTH 1000
#define DEFAULT_SCENE_HEIGHT 1000

Player_edit_view::Player_edit_view(QWidget *parent) :
        QGraphicsView(parent)
{
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

Player_edit_view::~Player_edit_view()
{
}

void Player_edit_view::initalize_default_scene()
{
    QLabel *default_scene_label = new QLabel();
    default_scene_label->setText("Please add a Song to the Playlist to use the Lightshow-edit and -play-mode.\n You can do so with the button in the Toolbar which looks like this:");

    QGraphicsPixmapItem *default_scene_pixmap = new QGraphicsPixmapItem();
    default_scene_pixmap->setPixmap(QPixmap(":/icons_png/png/audio.png"));
    default_scene_pixmap->setPos(0, 50);

    QGraphicsScene *default_scene = new QGraphicsScene();
    default_scene->addWidget(default_scene_label);
    default_scene->addItem(default_scene_pixmap);
    default_scene->setSceneRect(0, 0, 500, 500);
    setScene(default_scene);
}

void Player_edit_view::initialize_player_edit_scene(bool current_playing_song, QString label, std::shared_ptr<Lightshow> lightshow)
{
    Logger::info("player_edit_view::init_player_edit_scene");
    //Logger::debug(lightshow->get_value_changes_bass()[0].value);
    /*Logger::debug(lightshow.get()->get_value_changes_bass()[1].time);
    Logger::debug(lightshow.get()->get_value_changes_bass()[1].value);
    Logger::debug(lightshow.get()->get_value_changes_bass()[2].time);
    Logger::debug(lightshow.get()->get_value_changes_bass()[2].value);*/
    tracks_scene = new Multiple_tracks(label, lightshow);
    tracks_scene->initialize_player_edit_view(current_playing_song);
    setScene(tracks_scene);
    this->verticalScrollBar()->setSliderPosition(1);
    this->horizontalScrollBar()->setSliderPosition(1);
    current_scene = tracks_scene;
}

void Player_edit_view::init_waiting_or_dmx_not_connected_scene(bool is_dmx_connected)
{
    QLabel *scene_label = new QLabel();
    if (is_dmx_connected){
        scene_label->setText("Please wait until the lightshow is ready!");
    }
    else {
        scene_label->setText("Please connect the DMX-interface to use all funtions of the Rasberry to Light programm!");
    }

    QGraphicsScene *waiting_scene = new QGraphicsScene();
    waiting_scene->addWidget(scene_label);
    waiting_scene->setSceneRect(0, 0, 500, 500);
    setScene(waiting_scene);
}

void Player_edit_view::make_music_slider_move(qint64 milliseconds)
{
    current_scene->move_music_slider(milliseconds);
}

void Player_edit_view::change_scene(bool _current_playing_song, QString label ,std::shared_ptr<Lightshow> lightshow)
{
    Logger::trace("player_edit_view::change_scene");
    initialize_player_edit_scene(_current_playing_song, label, lightshow);
}

void Player_edit_view::show_hide_grid()
{
    if(current_scene != nullptr){
        current_scene->change_grid_status();
    }
}

bool Player_edit_view::grid_status()
{
    return current_scene->grid_status();
}

void Player_edit_view::slot_header_item_double_clicked(int x_pos)
{
    emit header_item_double_clicked(x_pos);
}
