#pragma once

#include <algorithm>
#include <deque>
#include <fstream>
#include <sstream>

#include "Generic_mapper.hpp"

namespace travel {
class Mapper : public Generic_mapper {

  struct travel_data {
    uint64_t from = 0; // départ d'une ligne de métro
    uint64_t to = 0; // arrrivée d'une ligne de métro
    uint64_t time = 0; // temps total pour une ligne de métro
    uint64_t stations = 0;
    int change = -1;
    bool location = false;
  };

 public:
  void read_stations(const std::string &) override;
  void read_connections(const std::string &) override;
  std::vector<std::pair<uint64_t, uint64_t> > compute_travel(uint64_t, uint64_t) override;
  std::vector<std::pair<uint64_t, uint64_t> > compute_and_display_travel(uint64_t, uint64_t) override;
  void display_travel(std::string &, std::string &);
  uint64_t get_time() const noexcept(false);
  int get_change() const noexcept(false);

 private:
  std::vector<std::pair<uint64_t, uint64_t> > path;
  int change;

  std::deque<std::string> hash_csv(const std::string &);
  std::string time_converter(uint64_t) const;
  bool are_same_station(uint64_t, uint64_t);
  travel_data &travel_change_state(travel_data &t, const std::vector<std::pair<uint64_t, uint64_t> >::iterator &);
  void display_travel_message(travel_data &, const uint64_t &) noexcept(false);
  uint64_t get_id(std::string) noexcept(false);
  std::string &str_simplify(std::string &);

};

}

