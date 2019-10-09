#include "multiple_tracks.h"
#include "header_mult_tracks.h"



#define VIEW_DEFAULT_WIDTH 5000
#define VIEW_DEFAULT_HEIGHT 5000
#define FIXTURE_HEIGHT 100
#define FIXTURE_WIDTH  250
#define HALF_SECOND_WIDTH 25

Multiple_tracks::Multiple_tracks(QString label, std::shared_ptr<Lightshow> _lightshow) : QGraphicsScene()
  , fixture_count(_lightshow->get_fixtures().size())
  , song_length((_lightshow->get_length() - 3) / _lightshow->get_resolution())
  , show_grid(false)
  , my_lightshow(_lightshow)
  , bass_fixture_count(_lightshow->get_fixtures_bass().size())
  , middle_fixture_count(_lightshow->get_fixtures_middle().size())
  , high_fixture_count(_lightshow->get_fixtures_high().size())
  , ambient_fixture_count(_lightshow->get_fixtures_ambient().size())
{
    /*Logger::info(bass_fixture_count);
    Logger::info(middle_fixture_count);
    Logger::info(high_fixture_count);
    Logger::info(ambient_fixture_count);*/

    // 98, 110, 120, 255
    //this->setBackgroundBrush(QBrush(QColor(178, 190, 200, 255)));

    scene_width = (song_length * 2) * HALF_SECOND_WIDTH + FIXTURE_WIDTH + 1;
    scene_height = VIEW_DEFAULT_HEIGHT;

    this->setSceneRect(0, 0, scene_width, scene_height);

    Logger::info("load_timestamps_fixture_groups");
    this->load_timestamps_fixture_groups();

    song_name_label = new SongLabel(250, 34, label);
    song_name_label->setPos(0, 0);
    addItem(song_name_label);


    header_width = VIEW_DEFAULT_WIDTH;
    header = new Header_mult_tracks(song_length);
    header->setPos(251, 0);
    addItem(header);
}

void Multiple_tracks::move_music_slider(qint64 milliseconds)
{
    qint64 new_pos = 251 + (milliseconds / 20);
    music_slider->setX(new_pos);

}

void Multiple_tracks::update_tracks_dividers()
{   

    QGraphicsItem *item = this->addRect(0, 35,
                                        this->width(), 1,
                                        QPen(QColor(87, 87, 87, 255)),
                                        QBrush(QColor(87, 87, 87, 255)));
    item->setZValue(1);

    QGraphicsItem *item1 = this->addRect(249, 37,
                                         1, fixture_count * FIXTURE_HEIGHT + fixture_count,
                                         QPen(QColor(87, 87, 87, 255)),
                                         QBrush(QColor(87, 87, 87, 255)));
    item1->setZValue(1);

    for (int j = 0; j < fixture_count; j++){
        int ypos = 137 + j * FIXTURE_HEIGHT + j;

        QGraphicsItem *item2 = this->addRect(0, ypos,
                                               this->width(), 1,
                                               QPen(QColor(87, 87, 87, 255)),
                                               QBrush(QColor(87, 87, 87, 255)));
        item->setZValue(1);
        horizontal_dividers.append(item2);
    }

    if (show_grid){
        for (int i = 276; i <= this->width(); i += 25){
            QGraphicsItem *item3 = this->addRect(i, 37, 0, fixture_count * FIXTURE_HEIGHT + fixture_count,
                                                      QPen(QColor(190, 190, 190, 255)),
                                                      QBrush(QColor(190, 190, 190, 255)));
            item1->setZValue(0);
            vertical_dividers.append(item3);
        }
    }
    else {
        if(vertical_dividers.count() != 0){
            for (int i = 0; i < vertical_dividers.count(); i++){
                this->removeItem(vertical_dividers[i]);
            }
            vertical_dividers.clear();
        }
    }
}

void Multiple_tracks::initialize_player_edit_view(bool current_playing_song)
{

    //Logger::debug("Multiple_tracks::init_player_edit_view");
    /*Logger::debug(my_lightshow.get()->get_value_changes_bass()[0].value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass()[1].time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass()[1].value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass()[2].time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass()[2].value);*/
    if (bass_fixture_count != 0){
        //Logger::debug("in bass fixture");
        for (int i = 0; i < bass_fixture_count; i++){
            fixture = new Lightshow_fixture_properties("Bass " + QString::number(i + 1), FIXTURE_WIDTH, FIXTURE_HEIGHT);
            fixture->setPos(0, 37 + i * FIXTURE_HEIGHT + i);
            addItem(fixture);

            //Logger::debug("for schleife");
            //Logger::debug(time_on_off_bass.size());
            for(int j = 0; j < time_on_off_bass.size(); j += 2) {
                //Logger::debug("bass objects");
                int start_pos = (time_on_off_bass[j] * 2) * HALF_SECOND_WIDTH;
                int end_pos = (time_on_off_bass[j + 1] * 2) * HALF_SECOND_WIDTH;
                standard_lightshow = new Standard_lightshow(end_pos - start_pos, FIXTURE_HEIGHT);
                standard_lightshow->setPos(251 + start_pos, 37 + i * FIXTURE_HEIGHT + i);
                standard_lightshow->setZValue(0);
                //Logger::debug(start_pos);
                //Logger::debug(end_pos);
                addItem(standard_lightshow);
            }
        }
    }

    if (middle_fixture_count != 0){
        for (int i = 0; i < middle_fixture_count; i++){
            fixture = new Lightshow_fixture_properties("Middle " + QString::number(i + 1), FIXTURE_WIDTH, FIXTURE_HEIGHT);
            fixture->setPos(0, 37 + (i + bass_fixture_count)  * FIXTURE_HEIGHT + (i + bass_fixture_count));
            addItem(fixture);

            for (int j = 0; j < time_on_off_middle.size(); j += 2){
                int start_pos = (time_on_off_middle[j] * 2) * HALF_SECOND_WIDTH;
                int end_pos = (time_on_off_middle[j + 1] * 2) * HALF_SECOND_WIDTH;
                standard_lightshow = new Standard_lightshow(end_pos - start_pos, FIXTURE_HEIGHT);
                standard_lightshow->setPos(251 + start_pos, 37 + (i + bass_fixture_count) * FIXTURE_HEIGHT + (i + bass_fixture_count));
                standard_lightshow->setZValue(0);
                addItem(standard_lightshow);
            }
        }
    }

    if (high_fixture_count != 0){
        for (int i = 0; i < high_fixture_count; i++){
            fixture = new Lightshow_fixture_properties("High " + QString::number(i + 1), FIXTURE_WIDTH, FIXTURE_HEIGHT);
            fixture->setPos(0, 37 + (i + bass_fixture_count + middle_fixture_count) * FIXTURE_HEIGHT + (i + bass_fixture_count + middle_fixture_count));
            addItem(fixture);

            for (int j = 0; j < time_on_off_high.size(); j += 2){
                int start_pos = (time_on_off_high[j] * 2) * HALF_SECOND_WIDTH;
                int end_pos = (time_on_off_high[j + 1] * 2) * HALF_SECOND_WIDTH;
                standard_lightshow = new Standard_lightshow(end_pos - start_pos, FIXTURE_HEIGHT);
                standard_lightshow->setPos(251 + start_pos, 37 + (i + bass_fixture_count + middle_fixture_count) * FIXTURE_HEIGHT + (i + bass_fixture_count + middle_fixture_count));
                standard_lightshow->setZValue(0);
                addItem(standard_lightshow);
            }
        }
    }

    if (ambient_fixture_count != 0){
        for (int i = 0; i < ambient_fixture_count; i++){
            fixture = new Lightshow_fixture_properties("Ambient " + QString::number(i + 1), FIXTURE_WIDTH, FIXTURE_HEIGHT);
            fixture->setPos(0, 37 + (i + bass_fixture_count + middle_fixture_count + high_fixture_count)  * FIXTURE_HEIGHT + (i + bass_fixture_count + middle_fixture_count + high_fixture_count));
            addItem(fixture);

            //int start_pos = (time_on_off_ambient[j] * 2) * HALF_SECOND_WIDTH;
            int start_pos = 0;
            //int end_pos = (time_on_off_ambient[j + 1] * 2) * HALF_SECOND_WIDTH;
            int end_pos = scene_width - 75;
            standard_lightshow = new Standard_lightshow(end_pos - start_pos, FIXTURE_HEIGHT);
            standard_lightshow->setPos(251 + start_pos, 37 + (i + bass_fixture_count + middle_fixture_count + high_fixture_count) * FIXTURE_HEIGHT + (i + bass_fixture_count + middle_fixture_count + high_fixture_count));
            standard_lightshow->setZValue(0);
            addItem(standard_lightshow);
        }
    }

    // Changing Lightshows has been shut down, because of time reasons
    // connect(standard_lightshow, &Standard_lightshow::item_double_clicked, this, &Multiple_tracks::slot_track_double_clicked);

    if (current_playing_song){
           music_slider = new Music_slider((fixture_count * FIXTURE_HEIGHT) + fixture_count + 37);
           music_slider->setPos(251, 0);
           music_slider->setZValue(999);
           addItem(music_slider);
       }

    if(bass_fixture_count != 0 || middle_fixture_count != 0 || high_fixture_count != 0 || ambient_fixture_count != 0)
        update_tracks_dividers();
}

void Multiple_tracks::change_grid_status()
{
    if(show_grid == false)
        this->show_grid = true;
    else
        this->show_grid = false;

    update_tracks_dividers();
}

bool Multiple_tracks::grid_status()
{
    return this->show_grid;
}

void Multiple_tracks::load_timestamps_fixture_groups()
{
    Logger::trace("in load_timestamps_fixture_groups");
    std::vector<time_value_int> v = my_lightshow.get()->get_value_changes_bass();
    //Logger::info("Multiple_tracks::load_timestamps_fixture_groups");
    Logger::info(my_lightshow->get_sound_src());
    //Logger::info(my_lightshow.get()->get_value_changes_bass().size());
    //Logger::info(my_lightshow->get_value_changes_middle().size());
    //Logger::info(my_lightshow->get_value_changes_high().size());


    Logger::debug("Multiple_tracks::load_timestamps_fixture_groups");
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(0).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(0).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(1).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(1).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(2).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(2).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(3).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(3).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(4).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(4).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(5).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(5).value);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(6).time);
    Logger::debug(my_lightshow.get()->get_value_changes_bass().at(6).value);

    for (std::vector<time_value_int>::iterator it = v.begin(); it != v.end(); ++it){
      if(it->value == 0)
        time_on_off_bass.push_back(it->time);
    }


    v = my_lightshow.get()->get_value_changes_middle();
    for (std::vector<time_value_int>::iterator it = v.begin(); it != v.end(); ++it){
        if(it->value == 0)
            time_on_off_middle.push_back(it->time);
    }

    v = my_lightshow.get()->get_value_changes_high();
    for (std::vector<time_value_int>::iterator it = v.begin(); it != v.end(); ++it){
        if(it->value == 0)
            time_on_off_high.push_back(it->time);
    }
}

/*
void Multiple_tracks::slot_track_double_clicked(Standard_lightshow *clicked_item)
{
    change_dialog = new Change_lightshow_dialog();
    change_dialog->exec();
}
*/

/*
void Multiple_tracks::slot_header_double_clicked(double x_pos)
{
    this->music_slider->setX(250 + x_pos);
    emit header_double_clicked(x_pos);
}
*/
