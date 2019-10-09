#ifndef PLAYER_EDIT_VIEW_H
#define PLAYER_EDIT_VIEW_H

#include <QGraphicsView>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QLabel>
#include <QGraphicsPixmapItem>


#include "multiple_tracks.h"
#include "lightshow/lightshow.h"

namespace Ui {
class Player_edit_view;
}

class Player_edit_view : public QGraphicsView {
 Q_OBJECT

 public:
  explicit Player_edit_view(QWidget *parent = nullptr);
  ~Player_edit_view();

  void initalize_default_scene();
  void initialize_player_edit_scene(bool current_playing_song, QString label, std::shared_ptr<Lightshow> lightshow);
  void initialize_editor_scene();
  void init_waiting_or_dmx_not_connected_scene(bool is_dmx_connected);
  void make_music_slider_move(qint64 position);
  void change_scene(bool _current_playing_song, QString label, std::shared_ptr<Lightshow> lightshow);
  void show_hide_grid();
  bool grid_status();

 protected slots:
  void slot_header_item_double_clicked(int x_pos);

 signals:
  void header_item_double_clicked(int x_pos);

 private:
  Multiple_tracks *tracks_scene;
  Multiple_tracks *current_scene;
  Music_slider *music_slider;

};

#endif // PLAYER_EDIT_VIEW_H
