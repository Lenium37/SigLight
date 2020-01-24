#include "universe.h"

Universe::Universe() {
  Universe("Universe");
}

Universe::Universe(string _name) {
    name = _name;
}

string Universe::get_name() {
    return name;
}

Fixture Universe::add_fixture(Fixture new_fixture) {
    data_fixturelist.push_back(new_fixture);
    return data_fixturelist.back();
}

Fixture Universe::get_fixture(int fixture_id) {
    // starts at 0.
    return *(std::next(data_fixturelist.begin(), fixture_id));
}

list<Fixture> Universe::get_fixtures() {
    return data_fixturelist;
}

std::list<Fixture *> Universe::get_fixtures_by_type(string type)
{
    std::list<Fixture *> type_list;
    for (auto fixture : data_fixturelist) {
        if(fixture.get_type() == type) {
            type_list.push_back(&fixture);
        }
    }

    return type_list;
}

void Universe::remove_fixture(int fixture_id) {
    list<Fixture>::iterator it = data_fixturelist.begin();
    advance(it, fixture_id);
    data_fixturelist.erase(it);
}

int Universe::get_fixture_count()
{
    return data_fixturelist.size();
}

std::list<int> Universe::get_blocked_adress_range()
{
    std::list<int> blocked_range;

    for(auto fixture : data_fixturelist) {
        blocked_range.push_back(fixture.get_start_channel());
        blocked_range.push_back(fixture.get_last_channel());
    }
    return blocked_range;
}

void Universe::set_fixture(Fixture new_fixture, int id)
{
    auto tmp_fixture = data_fixturelist.erase(std::next(data_fixturelist.begin(), id));
    data_fixturelist.insert(tmp_fixture, new_fixture);
}

void Universe::empty_fixtures()
{
    Universe::data_fixturelist.clear();
}

std::list<Fixture> Universe::get_fixture_by_type(std::string type) {
    std::list<Fixture> fixtures;
    for (auto fixture : Universe::data_fixturelist) {
        if(fixture.get_type() == type) fixtures.push_back(fixture);
    }

    return fixtures;
}

int Universe::get_fixtureid_by_startchannel(int start_channel)
{
    int fixture_place = 0;
    for(int i = 0; i < data_fixturelist.size(); i++) {
        if((*std::next(data_fixturelist.begin(), i)).get_start_channel() == start_channel) {
            fixture_place = i;
            break;
        }
    }
    return fixture_place;
}
