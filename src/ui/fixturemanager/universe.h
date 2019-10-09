#ifndef UNIVERS_H
#define UNIVERS_H

#include "fixture.h"
#include <list>
#include <string>

using namespace std;

class Universe {
public:
  /**
   * @brief Universe Default constructor.
   * The default constructor of Universe.
   */
  Universe();

  /**
   * @brief Universe Constructor of Universe with parameters.
   * @param name Name of the Universe.
   * @param description Description of the Universe.
   * Constructor of the Universe if the name and description are known.
   */
  Universe(string name, string description);

  /**
   * Default destructor of Universe.
   */
  ~Universe() = default;

  /**
   * @brief get_name Gives the name of the Universe.
   * @return The name of the Universe.
   */
  string get_name();

  /**
   * @brief get_description
   * @return Description of the Universe.
   */
  string get_description();

  /**
   * @brief add_fixture Adds an fixture to the Universe.
   * @param fixture Fixture that will be added.
   * @return Returns the fixture that was added.
   *
   * The Fixture will be added to the Universe fixturelist.
   * Start channel and end channel will be set according to the last channel
   * of the Fixture before and the channel count of the added fixture.
   *
   * If the Fixturelist was empty the channels will start at 1.
   */
  Fixture add_fixture(Fixture fixture);

  /**
   * @brief get_fixture Get the fixture at fixture_id.
   * @param fixture_id Place of the fixture in the list.
   * @return Returns the Fixture.
   *
   * Will give you the Fixture from the list. If it doesnt exist will give a empty Fixture.
   */
  Fixture get_fixture(int fixture_id);

  /**
   * @brief get_fixtures Gives you alle the Fixtures.
   * @return Fixtures contained in this Universe.
   */
  list<Fixture> get_fixtures();

  /**
   * @brief get_fixtures_by_type Gives you Fixtureadesses from a type or empty list if the type does not exist.
   * @param type Type of the fixtures to be returned.
   * @return Fixtures from a type.
   */
  std::list<Fixture *> get_fixtures_by_type(std::string type);

  /**
   * @brief remove_fixture Removes the Fixture at given index..
   * @param fixture_id Fixture number to be removed.
   */
  void remove_fixture(int fixture_id);

  /**
   * @brief get_fixture_count Gives you the number of fixtures in this universe.
   * @return Returns how many fixtures are in this universe.
   */
  int get_fixture_count();

  /**
   * @brief get_blocked_adress_range gets already used Adress ranges.
   * @return Adresses that are used. Empty list when there are fixtures.
   * First entry is start_adress, second is end_adress.
   *
   */
  std::list<int> get_blocked_adress_range();

  /**
   * @brief set_fixture Sets a new Fixture to given Position.
   * @param new_fixture The new Fixture.
   * @param id Postion of the Fixture, that will be replaced.
   */
  void set_fixture(Fixture new_fixture, int id);

  /**
   * @brief empty_fixtures emptys the Fixtures.
   */
  void empty_fixtures();

  /**
   * @brief get_fixtures_by_type Gives you Fixtures from a type or empty list if the type does not exist.
   * @param type Type of the fixtures to be returned.
   * @return Fixtures from a type.
   */
  std::list<Fixture> get_fixture_by_type(std::string type);

  /**
   * @brief get_fixtureid_by_startchannel Looks for the Fixture with given Startchannel.
   * @param start_channel The startchannel to be searched.
   * @return Place of the Fixture in the List. 0 if not found.
   */
  int get_fixtureid_by_startchannel(int start_channel);

private:
  /**
   * @brief data_fixturelist Contains the fixtures that belong to this Universe.
   */
  list<Fixture> data_fixturelist;

  /**
   * @brief name Name of the Universe.
   */
  string name;

  /**
   * @brief description Description of the Universe.
   * This could be an indicator where the Universe is connected.
   * Via USB Interface or GPIO.
   */
  string description;
};

#endif // UNIVERS_H
