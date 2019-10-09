#ifndef MULTIPLE_TRACKS_H
#define MULTIPLE_TRACKS_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QLabel>
#include <QUrl>

#include "header_mult_tracks.h"
#include "music_slider.h"
#include "lightshow_fixture_properties.h"
#include "standard_lightshow.h"
#include "logger.h"
#include "change_lightshow_dialog.h"
#include "lightshow/lightshow.h"
#include "song_label.h"

class Multiple_tracks : public QGraphicsScene
{
    Q_OBJECT

public:
    Multiple_tracks(QString label, std::shared_ptr<Lightshow> _lightshow);

    Music_slider *music_slider;

    SongLabel *song_name_label;

    void move_music_slider(qint64 time_pos);

    void initialize_player_edit_view(bool current_playing_song);

    void change_grid_status();
    bool grid_status();

    void load_timestamps_fixture_groups();



//protected slots:
    /* Changing Lightshows has been shut down, because of time reasons
    void slot_track_double_clicked(Standard_lightshow *clicked_item);
    */
    /* Changing Position in the song got shut down
    void slot_header_double_clicked(double x_pos);
    */

//signals:
    /* Changing Lightshows has been shut down, because of time reasons
    void track_double_clicked();
    */
    /* Changing Position in the song got shut down
    void header_double_clicked(double x_pos);
    */

private:
    void update_tracks_dividers();

    QList <QGraphicsItem *> horizontal_dividers;
    QList <QGraphicsItem *> vertical_dividers;
    Header_mult_tracks *header;
    Lightshow_fixture_properties *fixture;
    Standard_lightshow *standard_lightshow;
    //Change_lightshow_dialog *change_dialog;
    int header_width;
    int fixture_count;
    qint64 song_length;
    int scene_width;
    int scene_height;
    bool show_grid;
    std::shared_ptr<Lightshow> my_lightshow;
    int bass_fixture_count;
    int middle_fixture_count;
    int high_fixture_count;
    int ambient_fixture_count;
    std::vector<float> time_on_off_bass;
    std::vector<float> time_on_off_middle;
    std::vector<float> time_on_off_high;
    std::vector<float> time_on_off_ambient;

};

#endif // MULTIPLE_TRACKS_H
