//
// Created by Johannes on 09.05.2019.
//

#include <libusb.h>
#include <lightshow/k_8062.h>
#include <lightshow/lightshow.h>
#include <iostream>
#include <QtMultimedia/QMediaPlayer>

#if defined(_WIN32) || defined(WIN32)
#include <mingw.thread.h>
#else
#include <thread>
#endif
#include <unistd.h>
#include <lightshow/analysis.h>
#include <QtCore/QFile>
#include <QtWidgets/QFileDialog>

bool lightshow_playing2 = true;

int main() {
try {
  K8062 my_device;
  if(my_device.start_device()) {
    std::cout << "Failed to open K8062, aborting..." << std::endl;
    return -1;
  }
  Lightshow lightshow_test_eight_lamps;
  lightshow_test_eight_lamps.set_sound_src("test/light/show");

  /*ValueChange vc1_on(0.0, 255);
  ValueChange vc1_off(1.0, 0);
  ValueChange vc2_on(1.0, 255);
  ValueChange vc2_off(2.0, 0);
  ValueChange vc3_on(2.0, 255);
  ValueChange vc3_off(3.0, 0);

  Channel channel1(1);
  Channel channel3(3);
  Channel channel4(4);
  Channel channel5(5);

  channel1.add_value_change(ValueChange(0.0, 255));
  channel1.add_value_change(ValueChange(3.0, 0));

  channel3.add_value_change(vc1_on);
  channel3.add_value_change(vc1_off);
  channel4.add_value_change(vc2_on);
  channel4.add_value_change(vc2_off);
  channel5.add_value_change(vc3_on);
  channel5.add_value_change(vc3_off);

  LightshowFixture fixture1(1);
  fixture1.add_channel(channel1);
  fixture1.add_channel(channel3);
  fixture1.add_channel(channel4);
  fixture1.add_channel(channel5);
  LightshowFixture fixture2(7);
  fixture2.add_channel(channel1);
  fixture2.add_channel(channel3);
  fixture2.add_channel(channel4);
  fixture2.add_channel(channel5);
  LightshowFixture fixture3(13);
  fixture3.add_channel(channel1);
  fixture3.add_channel(channel3);
  fixture3.add_channel(channel4);
  fixture3.add_channel(channel5);
  LightshowFixture fixture4(19);
  fixture4.add_channel(channel1);
  fixture4.add_channel(channel3);
  fixture4.add_channel(channel4);
  fixture4.add_channel(channel5);
  LightshowFixture fixture5(25);
  fixture5.add_channel(channel1);
  fixture5.add_channel(channel3);
  fixture5.add_channel(channel4);
  fixture5.add_channel(channel5);
  LightshowFixture fixture6(31);
  fixture6.add_channel(channel1);
  fixture6.add_channel(channel3);
  fixture6.add_channel(channel4);
  fixture6.add_channel(channel5);
  LightshowFixture fixture7(37);
  fixture7.add_channel(channel1);
  fixture7.add_channel(channel3);
  fixture7.add_channel(channel4);
  fixture7.add_channel(channel5);
  LightshowFixture fixture8(43);
  fixture8.add_channel(channel1);
  fixture8.add_channel(channel3);
  fixture8.add_channel(channel4);
  fixture8.add_channel(channel5);

  //lightshow_test_eight_lamps.add_multiple_fixtures({fixture1, fixture2, fixture3, fixture4, fixture5, fixture6, fixture7, fixture8});

  lightshow_test_eight_lamps.add_fixture(fixture1);
  lightshow_test_eight_lamps.add_fixture(fixture2);
  lightshow_test_eight_lamps.add_fixture(fixture3);
  lightshow_test_eight_lamps.add_fixture(fixture4);
  lightshow_test_eight_lamps.add_fixture(fixture5);
  lightshow_test_eight_lamps.add_fixture(fixture6);
  lightshow_test_eight_lamps.add_fixture(fixture7);
  lightshow_test_eight_lamps.add_fixture(fixture8);*/

  Analysis analysis;
  analysis.read_wav((char*)"../../../assets/Sabaton-7734.wav");
  analysis.stft();
  analysis.normalize();
  std::vector<data_pair> data_pairs_bass = analysis.peaks_per_band(50, 80); // bass
  std::vector<data_pair> data_pairs_snare = analysis.peaks_per_band(170, 220); // snare


  Lightshow lightshow_from_analysis;
  LightshowFixture my_fixture_1(1);
  LightshowFixture my_fixture_2(7);
  LightshowFixture my_fixture_3(13);
  LightshowFixture my_fixture_4(19);
  LightshowFixture my_fixture_5(25);
  LightshowFixture my_fixture_6(31);
  LightshowFixture my_fixture_7(37);
  LightshowFixture my_fixture_8(43);

  Channel my_channel_5(5);
  Channel my_channel_3(3);
  Channel my_channel_1(1);

  ValueChange my_value_change_dimmer_on(0.0, 255);
  my_channel_1.add_value_change(my_value_change_dimmer_on);
  ValueChange my_value_change_dimmer_off(540.0, 0);
  my_channel_1.add_value_change(my_value_change_dimmer_off);
  ValueChange my_value_change_1(0.0, 0);

  for(int i = 0; i < data_pairs_bass.size(); i++) {
    my_value_change_1.set_start(data_pairs_bass[i].time);
    my_value_change_1.set_value(data_pairs_bass[i].value);
    if(my_value_change_1.get_value() != my_channel_3.get_value_of_last_added_value_change())
      my_channel_3.add_value_change(my_value_change_1);
  }

  for(int i = 0; i < data_pairs_snare.size(); i++) {
    my_value_change_1.set_start(data_pairs_snare[i].time);
    my_value_change_1.set_value(data_pairs_snare[i].value);
    if(my_value_change_1.get_value() != my_channel_5.get_value_of_last_added_value_change())
      my_channel_5.add_value_change(my_value_change_1);
  }


  my_fixture_1.add_channel(my_channel_1);
  my_fixture_1.add_channel(my_channel_3);
  my_fixture_2.add_channel(my_channel_1);
  my_fixture_2.add_channel(my_channel_3);
  my_fixture_3.add_channel(my_channel_1);
  my_fixture_3.add_channel(my_channel_5);
  my_fixture_4.add_channel(my_channel_1);
  my_fixture_4.add_channel(my_channel_5);
  my_fixture_5.add_channel(my_channel_1);
  my_fixture_5.add_channel(my_channel_5);
  my_fixture_6.add_channel(my_channel_1);
  my_fixture_6.add_channel(my_channel_5);
  my_fixture_7.add_channel(my_channel_1);
  my_fixture_7.add_channel(my_channel_3);
  my_fixture_8.add_channel(my_channel_1);
  my_fixture_8.add_channel(my_channel_3);
  lightshow_from_analysis.add_fixture(my_fixture_1);
  lightshow_from_analysis.add_fixture(my_fixture_2);
  lightshow_from_analysis.add_fixture(my_fixture_3);
  lightshow_from_analysis.add_fixture(my_fixture_4);
  lightshow_from_analysis.add_fixture(my_fixture_5);
  lightshow_from_analysis.add_fixture(my_fixture_6);
  lightshow_from_analysis.add_fixture(my_fixture_7);
  lightshow_from_analysis.add_fixture(my_fixture_8);
  std::cout << "post read, pre play" << std::endl;
  lightshow_playing2 = true;
  QMediaPlayer *player = new QMediaPlayer;
  //QFile sound_file(QFileDialog::getOpenFileName(nullptr, QObject::tr("Load sound file"), nullptr, QObject::tr("WAV Files (*.wav)")));
  QFile *io = new QFile("C:\\Users\\Johannes\\Desktop\\Uni\\5_Raspi-to-light\\19ss-raspitolight1\\assets\\Sabaton-7734.wav", nullptr);
  if(io->open(QFile::ReadOnly)) {
    player->setMedia(QUrl::fromLocalFile("C:\\file.mp4"), io);
  } else { std::cout << "could not open sound file" << std::endl; }

  player->setVolume(30);
  player->play();
  lightshow_from_analysis.play(my_device, std::ref(lightshow_playing2));
  //lightshow_test_eight_lamps.play(my_device, std::ref(lightshow_playing2));



  my_device.stop_device();
} catch (const std::bad_alloc& e) {
  std::cout << "Allocation failed main: " << e.what() << '\n';
}
  return 0;
}
