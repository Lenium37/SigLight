//
// Created by Jan on 20.06.2019.
//


#include "song.h"
#include "logger.h"
#include <ghc/filesystem.hpp>

Song::Song(const std::string &file_path) : file_path(file_path) {}
const std::string Song::get_file_path() const {
    return file_path;
}

void Song::set_file_path(std::string _file_path)
{
    this->file_path = _file_path;
}
bool Song::operator==(const Song &rhs) const {
  return file_path == rhs.file_path;
}
bool Song::operator!=(const Song &rhs) const {
  return !(rhs == *this);
}

Song::Song(const std::string &file_path, const int duration_in_sec)
    : file_path(file_path), duration_in_sec(duration_in_sec) {
  Logger::debug("new Song with fila path: {}", file_path);
  int minutes = duration_in_sec / 60;
  int secs = duration_in_sec % 60;
  this->formatted_duration = std::to_string(minutes) + ":" + std::to_string(secs);
}

const std::string Song::get_song_name() {
  return ghc::filesystem::path(file_path).filename().string();
}

std::ostream &operator<<(std::ostream &os, const Song &song) {
  os << "file_path: " << song.file_path;
  return os;
}

Song::Song() {
  file_path = "";
}

bool Song::is_empty() {
  return file_path.empty();
}

const int Song::get_duration() {
  return this->duration_in_sec;
}

std::string Song::get_formatted_duration() {
  return this->formatted_duration;
}

void Song::set_title(std::string title) {
  this->title = title;
}

std::string Song::get_title() {
  return this->title;
}

void Song::set_artist(std::string artist) {
  this->artist = artist;
}

std::string Song::get_artist() {
  return this->artist;
}

