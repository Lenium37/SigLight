//
// Created by Johannes on 22.10.2019.
//

#include <QtWidgets/QHBoxLayout>
#include <QtCore/QFile>
#include <iostream>
#include <QtCore/QXmlStreamReader>
#include <tinyxml2/tinyxml2.h>
#include "lightshow_window.h"
#include <QtWidgets/qapplication.h>
#include <QtMultimedia/qmediaplayer.h>
#include <unistd.h>

LightshowWindow::LightshowWindow() {

}

LightshowWindow::~LightshowWindow() {

}

int LightshowWindow::init() {
  auto* fixture_layout = new QHBoxLayout;
  //QString file_string = "C:\\Users\\Johannes\\Music\\Raspberry-to-Light\\xml-lightshows\\257ers-Sexdisko.xml";
  //QString file_string = "C:\\Users\\Johannes\\Music\\Raspberry-to-Light\\xml-lightshows\\Fun.-Some-Nights.xml";
  //QString file_string = "C:\\Users\\Johannes\\Music\\Raspberry-to-Light\\xml-lightshows\\Kasalla-Pirate.xml";
  QString file_string = "C:\\Users\\Johannes\\Music\\Raspberry-to-Light\\xml-lightshows\\Drum-loop-124-bpm.xml";

  // Setting Stylesheet
  QFile xml_file(file_string);
  if(!xml_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cout << "Error: Could not read XML file." << std::endl;
    return -1;
  }

  QXmlStreamReader reader;
  reader.setDevice(&xml_file);
  int resolution = 0;
  int length = 0;
  int channel_count = 0;
  QString path_to_wav;
  this->channel_index_bass = -1;
  this->channel_index_mid = -1;
  this->channel_index_high = -1;


  tinyxml2::XMLDocument lightshow_xml;
  lightshow_xml.LoadFile(file_string.toStdString().c_str());
  tinyxml2::XMLElement *lightshow_element = lightshow_xml.FirstChildElement("lightshow");

  // parameter setzen
  if (lightshow_element == nullptr){ std::cout << "Error: No lightshow element." << std::endl; return -1; }
  else {
    path_to_wav = lightshow_element->Attribute("src");
    length = std::stoi(lightshow_element->Attribute("length"));
    resolution = std::stoi(lightshow_element->Attribute("res"));
    tinyxml2::XMLElement *fixture_element = lightshow_element->FirstChildElement("fixture");
    while (fixture_element != nullptr) {
      tinyxml2::XMLElement *channel_element = fixture_element->FirstChildElement("channel");
      while(channel_element != nullptr) {
        channel_count++;
        channel_element = channel_element->NextSiblingElement("channel");
      }
      fixture_element = fixture_element->NextSiblingElement("fixture");
    }
  }

  std::vector<std::vector<std::uint8_t>> test(length, std::vector<std::uint8_t>(channel_count, 0.0));
  this->all_channel_values = test;
  std::cout << "src: " << path_to_wav.toStdString() << "\nlength: " << length << "\nresolution: " << resolution <<  "\nchannel count: " << channel_count << std::endl;

  // value changes auslesen
  tinyxml2::XMLElement *fixture_element = lightshow_element->FirstChildElement("fixture");
  while (fixture_element != nullptr) {
    int fixture_start_channel = std::stoi(fixture_element->Attribute("start_channel"));
    std::string fixture_type = fixture_element->Attribute("type");
    if(fixture_type == "bass")
      channel_index_bass = fixture_start_channel - 1;
    else if(fixture_type == "mid")
      channel_index_mid = fixture_start_channel - 1;
    else if(fixture_type == "high")
      channel_index_high = fixture_start_channel - 1;

    tinyxml2::XMLElement *channel_element = fixture_element->FirstChildElement("channel");
    while(channel_element != nullptr) {
      tinyxml2::XMLElement *vc_element = channel_element->FirstChildElement("vc");
      while(vc_element != nullptr) {
        float time = std::stof(vc_element->Attribute("t"));
        all_channel_values[time * resolution][fixture_start_channel + std::stoi(channel_element->Attribute("channel")) - 2] = std::stoi(vc_element->GetText());
        vc_element = vc_element->NextSiblingElement("vc");
      }
      channel_element = channel_element->NextSiblingElement("channel");
    }
    fixture_element = fixture_element->NextSiblingElement("fixture");
  }
  std::cout << "channel_index_bass: " << channel_index_bass << std::endl;
  std::cout << "channel_index_mid: " << channel_index_mid << std::endl;
  std::cout << "channel_index_high: " << channel_index_high << std::endl;
  std::cout << std::to_string(all_channel_values[110][0]) << std::endl;

  this->fix_bass = new QLabel();
  fix_bass->setObjectName("bass");
  fix_bass->setMinimumWidth(100);
  fix_bass->setMinimumHeight(100);
  fix_bass->setProperty("alpha", "0");
  fix_bass->setStyle(QApplication::style());

  this->fix_mid = new QLabel();
  fix_mid->setObjectName("mid");
  fix_mid->setMinimumWidth(100);
  fix_mid->setMinimumHeight(100);
  fix_mid->setProperty("alpha", "0");
  fix_mid->setStyle(QApplication::style());

  this->fix_high = new QLabel();
  fix_high->setObjectName("high");
  fix_high->setMinimumWidth(100);
  fix_high->setMinimumHeight(100);
  fix_high->setProperty("alpha", "0");
  fix_high->setStyle(QApplication::style());

  fixture_layout->addWidget(fix_bass);
  fixture_layout->addWidget(fix_mid);
  fixture_layout->addWidget(fix_high);

  QWidget* fixture_widget = new QWidget();
  fixture_widget->setLayout(fixture_layout);
  this->setCentralWidget(fixture_widget);
  this->show();

  auto* player = new QMediaPlayer();
  player->setNotifyInterval(75);
  connect(player, &QMediaPlayer::positionChanged, this, &LightshowWindow::update_fixtures);
  player->setMedia(QMediaContent(QUrl::fromLocalFile(path_to_wav)));
  player->setVolume(40);
  player->play();

}

void LightshowWindow::update_fixtures(qint64 timestamp) {
  //std::cout << std::to_string(timestamp) << std::endl;
  if(channel_index_bass >= 0)
    fix_bass->setProperty("alpha", QString::fromStdString(std::to_string(all_channel_values[timestamp/25][channel_index_bass])));

  if(channel_index_mid >= 0)
    fix_mid->setProperty("alpha", QString::fromStdString(std::to_string(all_channel_values[timestamp/25][channel_index_mid])));

  if(channel_index_high >= 0)
    fix_high->setProperty("alpha", QString::fromStdString(std::to_string(all_channel_values[timestamp/25][channel_index_high])));

  fix_bass->setStyle(QApplication::style());
  fix_mid->setStyle(QApplication::style());
  fix_high->setStyle(QApplication::style());
}