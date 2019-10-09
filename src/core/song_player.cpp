//
// Created by Jan on 18.08.2019.
//

#include <iostream>
#include <song_player_factory.h>
#include <unistd.h>

int main() {
  auto song_player = SongPlayerFactory::create_song_player([](Song song) {
    std::cout
        << fmt::format("Song changed callback: {}", song.get_file_path())
        << std::endl;
    }, [](SongPlayer::PlayerStatus status) {
    std::cout
        << fmt::format("PlayerStatusChanged callback: {}", status == SongPlayer::PlayerStatus::PLAYING ? "playing" : "not playing")
        << std::endl;
  },
  [](long long int position) {
    std::cout << fmt::format("position_changed_callback: {}", position) << std::endl;
  });

  song_player->set_current_song(Song(R"(/home/pi/19ss-raspitolight1/assets/Kasalla-Pirate.wav)"));
  song_player->play();
  
  
  
  std::cout << "start sleeping" << std::endl;
  usleep(6*1000000);
  std::cout << "stop sleeping" << std::endl;
  
  std::cout << "pause playback" << std::endl;
  song_player->pause();
  
  std::cout << "start sleeping" << std::endl;
  usleep(1*1000000);
  std::cout << "stop sleeping, play song to end" << std::endl;
  song_player->play();
  
  while (song_player->get_player_status() == SongPlayer::PlayerStatus::PLAYING) {
      
  }
  return 0;
}
