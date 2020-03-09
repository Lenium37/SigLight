#include "playlist.h"

#include <sys/stat.h>
#include <song_info_utils.h>

Playlist::Playlist() {
  m3u_file_name = "current_rtl_playlist.m3u";
  index_current_song = 0;
}

void Playlist::add_playlist_item(Song *song) {
  Playlist_item *pls_item = new Playlist_item(song);

  const std::string song_path = pls_item->get_song()->get_file_path();
  const SongInfo info = SongInfoUtils::get_song_info(song_path);
  pls_item->get_song()->set_title(info.title);
  pls_item->get_song()->set_artist(info.artist);

  playlist.push_back(pls_item);
}

void Playlist::delete_song_from_playlist(int index) {
  Logger::trace("Playlist::delete_song_from_playlist");
  Logger::debug(index);
  playlist.erase(playlist.begin() + index);
}

void Playlist::move_songs_to(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index)
{
    if(old_first_row_index < new_first_row_index){
        int k = new_first_row_index;
        for(int i = old_first_row_index; i <= old_last_row_index; i++){
            playlist.insert(playlist.begin() + k, this->playlist_item_at(i));
            k++;
        }

        for(int i = old_first_row_index; i <= old_last_row_index; i++){
            playlist.erase(playlist.begin() + i);
        }
    } else {
        int k = new_first_row_index;
        for(int i = old_first_row_index - 1; i <= old_last_row_index - 1; i++){
            playlist.insert(playlist.begin() + k, this->playlist_item_at(i));
            k++;
        }

        for(int i = old_first_row_index; i <= old_last_row_index; i++){
            playlist.erase(playlist.begin() + i);
        }
    }
}

Playlist_item *Playlist::find_playlist_item_by_name(std::string file_name) {
  int i = find_index_of_playlist_item_by_name(file_name);

  if (i != -1)
    return playlist_item_at(i);
  else
    return nullptr;
}

int Playlist::find_index_of_playlist_item_by_name(std::string file_name)
{
    int temp = -1;
    for(int i = 0; std::size_t(i) < playlist.size(); i++){
        if(file_name == playlist_item_at(i)->get_song()->get_song_name()){
            temp = i;
        }
    }
    return temp;
}

void Playlist::set_directory(std::string _directory_path) {
  this->directory_path = _directory_path;
}

void Playlist::set_m3u_file_name(std::string _m3u_file_name) {
  this->m3u_file_name = _m3u_file_name;
}

std::string Playlist::name_of_m3u_file() {
  return this->m3u_file_name;
}

void Playlist::delete_current_playlist() {

}

bool Playlist::read_m3u_file(std::string file_to_read) {

  std::cout << "debug00" << std::endl;
  bool success = false;
  std::cout << "debug000" << std::endl;
  std::ifstream input_file(directory_path + m3u_file_name, std::ifstream::in);
  std::cout << "debug0000" << std::endl;
  /*if (file_to_read.empty()) {
    std::string s = directory_path + m3u_file_name;
    input_file.open(s);
  } else {
    try {
      file_to_read = QUrl(QString::fromStdString(file_to_read)).path().toStdString().substr(1);
      input_file.open(file_to_read);
    } catch (...) {
      Logger::error("Something went wrong while opening the playlist!");
    }
  }
  std::string read_data;

  if (input_file) {
    int i = 1;
    std::string url;
    int sec = 0000;
    std::string file_name;
    std::string song_title;
    std::string song_artist;
    std::getline(input_file, read_data);
    bool useless_line = false;
    while (std::getline(input_file, read_data)) {

      if (read_data[0] == '#') {
        if (read_data[4] == 'I') {
          std::string string_sec;
          size_t pos_comma = read_data.find_first_of(",");
          string_sec = read_data.substr(8, pos_comma - 8);
          sec = std::stoi(string_sec);
          size_t pos_minus = read_data.find_first_of("-");
          song_artist = read_data.substr(pos_comma + 1, pos_minus - pos_comma - 1);
          size_t pos_last_dot = read_data.find_last_of('.');
          song_title = read_data.substr(pos_minus + 1, pos_last_dot - pos_minus - 1);
        } else {
          useless_line = true;
        }

      } else {
        url = read_data;
        size_t pos_last_slash = read_data.find_last_of("/");
        file_name = read_data.substr(pos_last_slash + 1);
      }

      if (!useless_line) {
        if (i % 2 == 0 && i != 0) {
          const QUrl qurl = QUrl(QString::fromStdString(url));
          std::string file_path;
          const bool is_file_url = qurl.scheme() == "file";

          if (is_file_url) {
            file_path = url.substr(8);
          } else {
            file_path = url;
          }

          const bool exists = ghc::filesystem::exists(file_path);
          if (!exists) {
            // check for relative path
            std::string music_folder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation).toStdString();
            auto absolute_path = ghc::filesystem::path(music_folder).append(file_path);
            if (ghc::filesystem::exists(absolute_path)) {
              file_path = absolute_path.string();
            }
          } else {
            Song *song = new Song(file_path, sec);
            song->set_title(song_title);
            song->set_artist(song_artist);
            auto *pls_item = new Playlist_item(song);
            playlist.push_back(pls_item);
            success = true;
          }
        }
        i++;
      } else {
        useless_line = false;
      }
    }
  }*/

  input_file.close();

  if (file_to_read.empty() && success == true) {
    emit current_rtl_m3u_read_and_inserted(0, this->playlist_length() - 1);
  }

  // TODO it is crashing here somehow?!
  std::cout << "before return" << std::endl;

  return success;
}

bool Playlist::write_m3u_file() {
  bool temp;
  std::ofstream output_file;
  std::string s = directory_path + m3u_file_name;
  output_file.open(s);

  output_file << "#EXTM3U" << std::endl;

  if (!output_file) {
    temp = false;
  } else {
    temp = true;
    for (int i = 0; i < playlist.size(); i++) {
      if (playlist_item_at(i)->get_song()->get_title() != "" && playlist_item_at(i)->get_song()->get_artist() != "") {
        output_file << "#EXTINF:" << playlist.at(i)->get_song()->get_duration() << ","
                    << playlist.at(i)->get_song()->get_artist() << "-" << playlist.at(i)->get_song()->get_title()
                    << std::endl;
      } else {
        output_file << "#EXTINF:" << playlist.at(i)->get_song()->get_duration() << ","
                    << playlist.at(i)->get_song()->get_song_name() << std::endl;
      }

      output_file
          //<< QUrl::fromLocalFile(QString::fromStdString(playlist.at(std::size_t(i))->get_song()->get_file_path())).toString().toStdString()
          << playlist.at(i)->get_song()->get_file_path()
          << std::endl;
    }
  }
  output_file.close();
  return temp;
}

Playlist_item* Playlist::playlist_item_at(int index)
{
    return playlist.at(std::size_t(index));
}

Song *Playlist::next_song()
{
    if (std::size_t(index_current_song) == playlist.size() - 1){
        index_current_song = 0;
    }
    else {
        index_current_song += 1;
    }
    emit current_media_index_changed(index_current_song);
    return playlist.at(std::size_t(index_current_song))->get_song();
}

Song *Playlist::previous_song()
{
    if (index_current_song == 0){
        index_current_song = playlist.size() - 1;
    }
    else {
        index_current_song -= 1;
    }
    emit current_media_index_changed(index_current_song);
    return playlist.at(std::size_t(index_current_song))->get_song();
}

int Playlist::playlist_length() {
  return playlist.size();
}

Playlist_item *Playlist::get_current_song() {
  return this->playlist_item_at(index_current_song);
}

void Playlist::slot_lightshow_of_pls_item_ready(std::string song_name) {
  find_playlist_item_by_name(song_name)->toggle_lightshow_status();
}

void Playlist::slot_title_or_artist_has_changed(int playlist_index, bool string_is_title, std::string title_or_artist) {
  if (string_is_title)
    this->playlist_item_at(playlist_index)->get_song()->set_title(title_or_artist);
  else
    this->playlist_item_at(playlist_index)->get_song()->set_artist(title_or_artist);
}

bool Playlist::file_exists(const std::string &filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

void Playlist::slot_lightshow_for_song_is_ready(Song *song) {
  this->find_playlist_item_by_name(song->get_song_name())->set_lightshow_status(true);
  Logger::info("Lightshow for Song {} is ready.", song->get_song_name());
}

void Playlist::set_index_current_song(int index_current_song) {
  this->index_current_song = index_current_song;
  emit current_media_index_changed(index_current_song);
}
