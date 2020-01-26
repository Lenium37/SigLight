//
// Created by Jan on 26.05.2019.
//


#include <ghc/filesystem.hpp>
#include <tinyxml2.h>
#include "light_show_registry.h"


std::shared_ptr<Lightshow> LightShowRegistry::get_lightshow(Song *song) {
  Logger::debug("trying to get lightshow for song \"{}\"", song->get_file_path());
  //Logger::debug("Content: ");
  //log_content();

  auto search = lightshow_files.find(song->get_file_path());
  if(search != lightshow_files.end() && ghc::filesystem::exists(search->second)){
    Logger::info("found LightShow for song {} at path {}", song->get_song_name(), search->second);
    return this->read_lightshow(search->second);
  }
  else{
    Logger::info("found no LightShow for song \"{}\"", song->get_song_name());
    return nullptr;
  }

}

void LightShowRegistry::log_content() {
  for(auto &entry : lightshows){
    Logger::debug("Entry: \"{}\"", entry.first);
  }
}

void LightShowRegistry::write_lightshow(const std::string &lightshow_filename, std::shared_ptr<Lightshow> lightshow) {
  Logger::debug("Writing lightshow to {}", lightshow_filename);
  tinyxml2::XMLDocument lightshow_xml;
  //tinyxml2::XMLDeclaration * decl = new tinyxml2::XMLDeclaration( "1.0", "", "" );
  //tinyxml2::XMLNode *lightshow_element = lightshow_xml.NewElement("lightshow");
  tinyxml2::XMLElement *lightshow_element = lightshow_xml.NewElement("lightshow");
  lightshow_element->SetAttribute("src", lightshow->get_sound_src().c_str());
  lightshow_element->SetAttribute("length", lightshow->get_length());
  lightshow_element->SetAttribute("res", lightshow->get_resolution());
  lightshow_element->SetAttribute("bpm", lightshow->get_bpm());
  lightshow_xml.InsertFirstChild(lightshow_element);

  for(auto fixture : lightshow->get_fixtures()){
    tinyxml2::XMLElement *fixture_element = lightshow_xml.NewElement("fixture");
    fixture_element->SetAttribute("name", fixture.get_name().c_str());
    fixture_element->SetAttribute("start_channel", fixture.get_start_channel());
    fixture_element->SetAttribute("number_of_channels", fixture.get_number_of_channels());
    fixture_element->SetAttribute("type", fixture.get_type().c_str());
    fixture_element->SetAttribute("position_inside_group", fixture.get_position_in_group());
    fixture_element->SetAttribute("moving_head_type", fixture.get_moving_head_type().c_str());
    fixture_element->SetAttribute("position_on_stage", fixture.get_position_on_stage().c_str());
    fixture_element->SetAttribute("modifier_pan", fixture.get_modifier_pan());
    fixture_element->SetAttribute("modifier_tilt", fixture.get_modifier_tilt());
    fixture_element->SetAttribute("timestamps_type", fixture.get_timestamps_type().c_str());
    std::string colors;
    for(std::string c: fixture.get_colors()) {
      colors.append(c);
      colors.append("/");
    }
    if(colors.length() > 1)
      colors.pop_back();
    fixture_element->SetAttribute("colors", colors.c_str());

    for(auto channel : fixture.get_channels()) {
      if(channel.get_value_changes().size() > 0) {

        tinyxml2::XMLElement *channel_element = lightshow_xml.NewElement("channel");
        channel_element->SetAttribute("channel", channel.get_channel());

        for(auto vc : channel.get_value_changes()) {
          tinyxml2::XMLElement *vc_element = lightshow_xml.NewElement("vc");
          vc_element->SetAttribute("t", vc.get_timestamp());
          vc_element->SetText(vc.get_value());
          channel_element->InsertEndChild(vc_element);
        }

        fixture_element->InsertEndChild(channel_element);

      }
    }

    lightshow_element->InsertEndChild(fixture_element);
  }

  lightshow_xml.SaveFile(lightshow_filename.c_str());
}

void LightShowRegistry::register_lightshow_file(Song *song, std::shared_ptr<Lightshow> lightshow, const std::string &path_to_lightshow_directory) {
  Logger::debug("register lightshow and lightshow file for song \"{}\"", song->get_song_name());
  //const std::string xml_file_name = ghc::filesystem::path("xml-lightshows").append(song->get_song_name()).replace_extension(".xml");
  const std::string xml_file_name = ghc::filesystem::path(path_to_lightshow_directory).append(song->get_song_name()).replace_extension(".xml");
  lightshow_files.insert(std::pair<std::string, std::string>(song->get_file_path(), xml_file_name));

  this->write_lightshow(xml_file_name, lightshow);

  Logger::debug("After: ");
  log_content();
}

void LightShowRegistry::register_song(Song *song, const std::string &path_to_lightshow_directory) {
  Logger::debug("register lightshow and lightshow file for song \"{}\"", song->get_song_name());
  //const std::string xml_file_name = ghc::filesystem::path("xml-lightshows").append(song->get_song_name()).replace_extension(".xml");
  const std::string xml_file_name = ghc::filesystem::path(path_to_lightshow_directory).append(song->get_song_name()).replace_extension(".xml");
  lightshow_files.insert(std::pair<std::string, std::string>(song->get_file_path(), xml_file_name));

  Logger::debug("After: ");
  log_content();
}

void LightShowRegistry::add_existing_song_and_lightshow(Song *song, const std::string &path_to_lightshow_directory) {
  //const std::string xml_file_name = ghc::filesystem::path("xml-lightshows").append(song->get_song_name()).replace_extension(".xml");
  const std::string xml_file_name = ghc::filesystem::path(path_to_lightshow_directory).append(song->get_song_name()).replace_extension(".xml");
  lightshow_files.insert(std::pair<std::string, std::string>(song->get_file_path(), xml_file_name));
  Logger::debug("After: ");
  log_content();
}

std::shared_ptr<Lightshow> LightShowRegistry::read_lightshow(const std::string file_path) {
  tinyxml2::XMLDocument lightshow_xml;
  lightshow_xml.LoadFile(file_path.c_str());
  std::shared_ptr<Lightshow> lightshow = std::make_shared<Lightshow>();

  tinyxml2::XMLElement *lightshow_element = lightshow_xml.FirstChildElement("lightshow");

  lightshow->set_sound_src(lightshow_element->Attribute("src"));
  lightshow->set_length(std::stoi(lightshow_element->Attribute("length")));
  lightshow->set_resolution(std::stoi(lightshow_element->Attribute("res")));
  lightshow->set_bpm(std::stoi(lightshow_element->Attribute("bpm")));


  if (lightshow_element == nullptr){ Logger::error("Fehler beim Laden einer Lightshow XML"); }
  else {
    tinyxml2::XMLElement *fixture_element = lightshow_element->FirstChildElement("fixture");
    while (fixture_element != nullptr) {
      std::string colors = fixture_element->Attribute("colors");
      LightshowFixture fixture(fixture_element->Attribute("name"), std::stoi(fixture_element->Attribute("start_channel")), std::stoi(fixture_element->Attribute("number_of_channels")), fixture_element->Attribute("type"), colors, std::stoi(fixture_element->Attribute("position_inside_group")), fixture_element->Attribute("position_on_stage"), fixture_element->Attribute("moving_head_type"), std::stoi(fixture_element->Attribute("modifier_pan")), std::stoi(fixture_element->Attribute("modifier_tilt")), fixture_element->Attribute("timestamps_type"));

      tinyxml2::XMLElement *channel_element = fixture_element->FirstChildElement("channel");
      while(channel_element != nullptr) {
        Channel channel(std::stoi(channel_element->Attribute("channel")));

        tinyxml2::XMLElement *value_change_element = channel_element->FirstChildElement("vc");
        while(value_change_element != nullptr) {
          float time = std::stof(value_change_element->Attribute("t"));
          int vc_value = std::stoi(value_change_element->GetText());
          ValueChange value_change(time, vc_value);
          channel.add_value_change(value_change);
          value_change_element = value_change_element->NextSiblingElement("vc");
          std::string fixture_type(fixture_element->Attribute("type"));
          if(fixture_type == "bass")
            lightshow->add_value_change_bass({time, vc_value});
          else if(fixture_type == "mid")
            lightshow->add_value_change_mid({time, vc_value});
          else if(fixture_type == "high")
            lightshow->add_value_change_high({time, vc_value});
        }

        fixture.add_channel(channel);
        channel_element = channel_element->NextSiblingElement("channel");
      }

      if(fixture.get_type() == "bass") {
        lightshow->add_fixture_bass(fixture);
      } else if(fixture.get_type() == "mid") {
        lightshow->add_fixture_middle(fixture);
      } else if(fixture.get_type() == "high") {
        lightshow->add_fixture_high(fixture);
      } else if(fixture.get_type() == "ambient") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "color_change") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "flash") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "flash_reverse") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "blink") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_one_after_another") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_one_after_another_back_and_forth") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_one_after_another_blink") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_one_after_another_back_and_forth_blink") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_two_after_another") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_alternate_odd_even") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "group_random_flashes") {
        lightshow->add_fixture_ambient(fixture);
      } else if(fixture.get_type() == "strobe_if_many_onsets") {
        lightshow->add_fixture_ambient(fixture);
      } else {
        lightshow->add_fixture(fixture);
      }
      fixture_element = fixture_element->NextSiblingElement("fixture");
    }
  }

  Logger::debug("successfully read lightshow");

  return lightshow;
}

void LightShowRegistry::renew_lightshow_for_song(Song *song, std::shared_ptr<Lightshow> lightshow, const std::string &path_to_lightshow_directory) {

  //std::string xml_file_name = ghc::filesystem::path("xml-lightshows").append(song->get_song_name()).replace_extension(".xml");
  std::string xml_file_name = ghc::filesystem::path(path_to_lightshow_directory).append(song->get_song_name()).replace_extension(".xml");
  auto search = lightshow_files.find(song->get_file_path());
  if(search != lightshow_files.end()) {
    search->second = xml_file_name;
  }
  this->write_lightshow(xml_file_name, lightshow);
  Logger::info("renewed lightshow for Song {}", song->get_song_name());
}
