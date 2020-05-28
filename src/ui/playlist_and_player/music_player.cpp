#include "music_player.h"
#include "mp_3_to_wav_converter.h"
#include <qdebug.h>

MusicPlayer::MusicPlayer()
    : QObject() {
  song_player = SongPlayerFactory::create_song_player([](Song song) {
    Logger::info("Song changed to: {}", song.get_file_path());
  }, [](SongPlayer::PlayerStatus status) {
    Logger::info("PlayerStatus: {}", status == SongPlayer::PlayerStatus::PLAYING ? "playing" : "not playing");
  }, [this](long long int position) {
    emit position_changed(position);
  });

  // todo support positionChanged signal connect(q_media_player, &QMediaPlayer::positionChanged, this, &MusicPlayer::position_changed);

  media_is_playing = false;
  // set player volume to 50 -> and display it in the volume qlcdwindow as default
  // todo what about volume? q_media_player->setVolume(50);

  // notifyInterval is set to 20ms so the multi_tracks cursor moves smoothly

  // todo what about notify interval? q_media_player->setNotifyInterval(20);
  this->song_player.get()->set_notify_interval(20);

  pls = new Playlist(); // todo make unique ptr
  this->current_playlist_index = 0;
  connect(this->pls, &Playlist::current_media_index_changed, this, &MusicPlayer::slot_current_playlist_index_changed);
  connect(this,
          &MusicPlayer::title_or_artist_of_song_has_changed,
          this->pls,
          &Playlist::slot_title_or_artist_has_changed);
  connect(this->pls,
          &Playlist::current_rtl_m3u_read_and_inserted,
          this,
          &MusicPlayer::slot_media_inserted_into_pls_on_startup);
  connect(this, &MusicPlayer::lightshow_for_song_is_ready, this->pls, &Playlist::slot_lightshow_for_song_is_ready);
}

void MusicPlayer::set_songs_directory_path(std::string _songs_directory_path) {
  this->songs_directory_path = _songs_directory_path;
}

QString MusicPlayer::get_formatted_time() const {
  long long int time_milli_seconds = this->song_player->current_song_duration();
  return format_time(time_milli_seconds);
}
QString MusicPlayer::format_time(qint64 time_milli_seconds) const {
  qint64 centiseconds = time_milli_seconds / 10;
  qint64 seconds = time_milli_seconds / 1000;
  const qint64 minutes = seconds / 60;
  seconds -= minutes * 60;
  centiseconds -= seconds * 100;
  return QStringLiteral("%1:%2:%3")
      .arg(minutes, 2, 10, QLatin1Char('0'))
      .arg(seconds, 2, 10, QLatin1Char('0'))
      .arg(centiseconds, 2, 10, QLatin1Char('0'));
}

Playlist_item *MusicPlayer::get_playlist_media_at(int index) {
  if (index < this->pls->playlist_length())
    return this->pls->playlist_item_at(index);
  else
    return nullptr;
}

Song * MusicPlayer::add_to_playlist(QUrl url) {
  const bool mp3_converter_is_avaible = Mp3ToWavConverter::is_avaible();
  int start_media_inserted_into_playlist = pls->playlist_length();
  Song * _ls_for_song;

  if (!is_playlist(url)) {
    std::string new_file_url = change_and_copy_file(url);
    Logger::debug(new_file_url);

    Song *song = new Song(new_file_url);
    pls->add_playlist_item(song);
    _ls_for_song = song;
  } else {
    int first_index_playlist_insertion = this->pls->playlist_length();
    bool read_successful = this->pls->read_m3u_file(url.toString().toStdString(), this->songs_directory_path);
    int last_index_playlist_insertion = this->pls->playlist_length() - 1;

    if (read_successful) {
      for (int i = first_index_playlist_insertion; i <= last_index_playlist_insertion; i++) {
        const std::string source = pls->playlist_item_at(i)->get_song()->get_file_path();
        const ghc::filesystem::path file_extension = ghc::filesystem::path(source).extension();
        if (file_extension == ".wav" || (file_extension == ".mp3" && mp3_converter_is_avaible)) {
          QUrl u = QUrl::fromLocalFile(QString::fromStdString(source));
          Logger::debug(u.toString().toStdString());

          std::string new_file_url = change_and_copy_file(u);
          pls->playlist_item_at(i)->get_song()->set_file_path(new_file_url);
          Logger::info(source);
          _ls_for_song = pls->playlist_item_at(i)->get_song();
        } else {
          Logger::error("Skipping file {}, converter is not avaible", source);
        }
      }
    }
  }

  // todo if songplayer has no song, set the song
  /*if (q_media_player->mediaStatus() == QMediaPlayer::NoMedia) {
    q_media_player->setMedia(QUrl(song->get_file_path().c_str()));
  }*/
  Song current_song = song_player->get_current_song();
  if (current_song.is_empty()) {
    song_player->set_current_song(Song(pls->playlist_item_at(0)->get_song()->get_file_path())); // todo better copying / dont take song pointer => edit -> has to be with a pointer from pls for m3u readability
  }

  int end_media_inserted_into_playlist = pls->playlist_length() - 1;
  if (end_media_inserted_into_playlist != -1)
      emit media_inserted_into_playlist(start_media_inserted_into_playlist, end_media_inserted_into_playlist);

  return _ls_for_song;
}

static inline void replace_substring(std::string &str, const std::string &from, const std::string &to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
}

const std::string MusicPlayer::change_and_copy_file(const QUrl _file_url) {
  std::string new_file_name = _file_url.fileName().toStdString();
  replace_substring(new_file_name, "ä", "ae");
  replace_substring(new_file_name, "ö", "oe");
  replace_substring(new_file_name, "ü", "ue");

  std::string copied_file_url = this->songs_directory_path + new_file_name;
  std::replace(copied_file_url.begin(), copied_file_url.end(), '\\', '/');
  if (ghc::filesystem::path(_file_url.fileName().toStdString()).extension() == ".wav") {
    copy_wav(_file_url, copied_file_url);
  } else {
    copied_file_url = ghc::filesystem::path(copied_file_url).replace_extension(".wav").string();
    convert_to_wav(_file_url.toLocalFile().toStdString(), copied_file_url);
  }

  return copied_file_url;
}
void MusicPlayer::copy_wav(const QUrl &_file_url, const std::string &copied_file_url) const {
  Logger::debug("Copy wav from {} to {}", _file_url.toLocalFile().toStdString(), copied_file_url);
  if (QFile::exists(QString::fromStdString(copied_file_url))) {
    Logger::debug("Destination sound file exists, removing...");
    QFile::remove(QString::fromStdString(copied_file_url));
  }
  Logger::debug("Copying of song file completed: {}",
                QFile::copy(_file_url.toLocalFile(), QString::fromStdString(copied_file_url)));
}

bool MusicPlayer::is_playlist(const QUrl &playlist_path) {
  if (!playlist_path.isLocalFile())
    return false;
  const QFileInfo fileInfo(playlist_path.toLocalFile());
  return fileInfo.exists() && !fileInfo.suffix().compare(QLatin1String("m3u"), Qt::CaseInsensitive);
}

void MusicPlayer::next_song() {
  Logger::debug("next_song");
  song_player->set_current_song(Song(pls->next_song()->get_file_path()));
}

void MusicPlayer::previous_song() {
  song_player->set_current_song(Song(pls->previous_song()->get_file_path()));
}

void MusicPlayer::play_song() {
  song_player->play();
  media_is_playing = true;
}

void MusicPlayer::pause_song() {
  song_player->pause();
  media_is_playing = false;
}

void MusicPlayer::stop_song() {
  song_player->stop();
  media_is_playing = false;
}

int MusicPlayer::playlist_index() {
  return this->current_playlist_index;
}

void MusicPlayer::set_playlist_index(int index) {
  this->current_playlist_index = index;
  pls->set_index_current_song(index);
}

bool MusicPlayer::is_media_playing() {
  return media_is_playing;
}

void MusicPlayer::set_media_to(int index) {
  pls->set_index_current_song(index);
  if (index != -1) {
    song_player->set_current_song(Song(pls->playlist_item_at(index)->get_song()->get_file_path()));
    this->current_playlist_index = index;
  }

  if (media_is_playing)
    this->play_song();
}

void MusicPlayer::save_playlist(std::string directory_to_save_playlist) {
  pls->set_directory(directory_to_save_playlist);
  pls->write_m3u_file();
}

void MusicPlayer::read_own_m_3_u_on_startup(std::string standard_directory_to_read_playlist) {
  pls->set_directory(standard_directory_to_read_playlist);
  pls->read_m3u_file("", this->songs_directory_path);
}

Song *MusicPlayer::get_current_song() {
  return pls->get_current_song()->get_song();
}

void MusicPlayer::slot_current_playlist_index_changed(int _current_playlist_index) {
  current_playlist_index = _current_playlist_index;
}

void MusicPlayer::slot_media_inserted_into_pls_on_startup(int start_index, int end_index) {
  this->set_media_to(start_index);
  emit media_inserted_into_playlist_on_startup(start_index, end_index);
}

void MusicPlayer::slot_title_or_artist_of_song_has_changed(int pls_index, bool is_title, std::string title_or_artist) {
  emit title_or_artist_of_song_has_changed(pls_index, is_title, title_or_artist);
}

void MusicPlayer::slot_lightshow_for_song_is_ready(Song *song) {
  emit lightshow_for_song_is_ready(song);
}

void MusicPlayer::slot_order_playlist_changed(int old_first_row_index, int old_last_row_index, int new_first_row_index, int new_last_row_index)
{
    this->pls->move_songs_to(old_first_row_index, old_last_row_index, new_first_row_index, new_last_row_index);
}

int MusicPlayer::get_playlist_length() {
  return this->pls->playlist_length();
}
int MusicPlayer::volume() {
  return song_player->get_volume();
}
void MusicPlayer::set_volume(int volume) {
  song_player->set_volume(volume);
}

int MusicPlayer::playlist_index_for(Song *song) {
  return this->pls->find_index_of_playlist_item_by_name(song->get_song_name());
}

void MusicPlayer::delete_song_from_playlist(int index) {
  pls->delete_song_from_playlist(index);
}

void MusicPlayer::convert_to_wav(std::string from, std::string to) {
  Logger::debug("convert_to_wav from {} to {}", from, to);
  Mp3ToWavConverter::convert_to_mp3(from, to);
}

double MusicPlayer::get_song_time() {
  return song_player->get_song_time();
}
