//
// Created by Jan on 06.06.2019.
//

#include "lightshow_generator.h"

LightshowGenerator::LightshowGenerator() {

}

std::shared_ptr<Lightshow> LightshowGenerator::generate(int resolution, Song *song, std::list<Fixture> fixtures) {
  auto start = chrono::steady_clock::now();
  std::shared_ptr<Lightshow> lightshow_from_analysis = std::make_shared<Lightshow>();
  lightshow_from_analysis->set_resolution(resolution);

  lightshow_from_analysis->set_sound_src(song->get_file_path());
  lightshow_from_analysis->prepare_analysis_for_song((char*)song->get_file_path().c_str());

  std::vector<LightshowFixture> my_fixtures;
  for (Fixture fix: fixtures) {
    //std::cout << "new fix. name: " << fix.get_name() << ". start address: " << fix.get_start_channel() << ", number of addresses: " << fix.get_channel_count() << std::endl;
    if (fix.get_name().compare("Cameo Flat RGB 10") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else if(fix.get_name().compare("Helios 7") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else if(fix.get_name().compare("Cobalt Plus Spot 5R") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else if(fix.get_name().compare("Varytec PAD7 seventy") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else if (fix.get_name().compare("TOURSPOT PRO") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else if (fix.get_name().compare("BAR TRI-LED") == 0) {
      my_fixtures.push_back(LightshowFixture(fix.get_name(), fix.get_start_channel(), fix.get_channel_count(), fix.get_type()));
    } else std::cout << "Fixture type unknown." << std::endl;
  }


  for (LightshowFixture fix: my_fixtures) {
    std::string fix_type = fix.get_type();
    std::transform(fix_type.begin(), fix_type.end(), fix_type.begin(), ::tolower);

    if (fix_type.compare("bass") == 0) {

      if(fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(),
                                         fix.get_channel_dimmer());

        std::vector<std::string> colors;
        colors.push_back("red");
        colors.push_back("cyan");
        colors.push_back("light-green");
        colors.push_back("blue");
        colors.push_back("pink");
        lightshow_from_analysis->generate_color_fades(fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_bass(),
                                         fix.get_channel_red());
      }
      lightshow_from_analysis->add_fixture_bass(fix);
    } else if (fix_type.compare("mid") == 0) {

      if(fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(),
                                         fix.get_channel_dimmer());

        std::vector<std::string> colors;
        colors.push_back("blue");
        colors.push_back("light-green");
        colors.push_back("cyan");
        colors.push_back("red");
        colors.push_back("green");
        lightshow_from_analysis->generate_color_fades(fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_middle(),
                                         fix.get_channel_blue());
      }
      lightshow_from_analysis->add_fixture_middle(fix);
    } else if (fix_type.compare("high") == 0) {

      if(fix.has_global_dimmer) {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(),
                                         fix.get_channel_dimmer());

        std::vector<std::string> colors;
        colors.push_back("green");
        colors.push_back("yellow");
        colors.push_back("red");
        colors.push_back("white");
        colors.push_back("blue");
        lightshow_from_analysis->generate_color_fades(fix, colors);

      } else {
        fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_high(),
                                         fix.get_channel_green());
      }
      lightshow_from_analysis->add_fixture_high(fix);
    } else if (fix_type.compare("action") == 0) {
      lightshow_from_analysis->set_color_of_fixture_during_song(fix, {"white"});
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_action(),
                                       fix.get_channel_dimmer());
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_action(),
                                       fix.get_channel_strobo());
      lightshow_from_analysis->add_fixture_action(fix);
    } else if (fix_type.compare("everything") == 0) {
      lightshow_from_analysis->set_color_of_fixture_during_song(fix, {"white"});
      fix.add_value_changes_to_channel(lightshow_from_analysis->get_value_changes_everything(),
                                       fix.get_channel_dimmer());
      lightshow_from_analysis->add_fixture(fix);
    } else if (fix_type.compare("ambient") == 0) {
      if(fix.has_global_dimmer) {
        std::vector<time_value_int> v;
        v.push_back({0.0, 200});
        v.push_back({((float)lightshow_from_analysis->get_length() - 3) / lightshow_from_analysis->get_resolution() , 0});
        fix.add_value_changes_to_channel(v, fix.get_channel_dimmer());

        std::vector<std::string> colors;
        colors.push_back("blue");
        colors.push_back("light-green");
        colors.push_back("cyan");
        colors.push_back("red");
        colors.push_back("green");
        lightshow_from_analysis->generate_color_fades(fix, colors);
        //lightshow_from_analysis->generate_ambient_color_fades(fix, colors);  // funktioniert noch gar nicht gut

      } else {

      }
      lightshow_from_analysis->add_fixture(fix);
    }
  }
  auto end = chrono::steady_clock::now();

  Logger::info("Lightshow creation took {}s", chrono::duration_cast<chrono::seconds>(end - start).count());


  return lightshow_from_analysis;
}
