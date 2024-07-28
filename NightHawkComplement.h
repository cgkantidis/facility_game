#ifndef NIGHTHAWK_COMPLETEMENT_H
#define NIGHTHAWK_COMPLETEMENT_H

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "EnumPlayer.h"
#include "FPlayer.h"
#include "FacilityGameException.h"

class NightHawkComplement : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "NightHawk_Complement";
  static constexpr char const *VERSION = "1.0";
  static constexpr int AFM = 56483;
  static constexpr char const *FIRSTNAME = "Christos";
  static constexpr char const *LASTNAME = "Gkantidis";

  static constexpr std::size_t COMPLEMENT = 9999;
  // a map from node value to the indices where this value appears
  std::unordered_map<std::size_t, std::vector<std::size_t>> indices;

public:
  explicit NightHawkComplement(EnumPlayer player)
      : FPlayer(player, PLAYER_NAME, VERSION, AFM, FIRSTNAME, LASTNAME) {}

  void initialize(FacilityGameAPI const *game) override {
    auto const &value_vec = game->get_value_vec();
    std::size_t idx = 0;
    for (auto const &value : value_vec) {
      indices[value].emplace_back(idx++);
    }
  }

  std::size_t next_move(FacilityGameAPI const *game) override {
    std::size_t const num_moves = game->get_num_moves();
    std::vector<std::size_t> const &move_locations = game->get_move_locations();
    std::size_t const vs_value = game->get_value(move_locations[num_moves]);
    std::size_t const my_value = COMPLEMENT - vs_value;

    auto it1 = indices.find(my_value);
    if (it1 == indices.end()) {
      throw FacilityGameException(
          "No available move. No node was found with the needed value.");
    }

    auto it2 = std::ranges::find_if(it1->second, [game](std::size_t idx) {
      return game->get_status(idx) == EnumFacilityStatus::FREE;
    });
    if (it2 == it1->second.end()) {
      throw FacilityGameException(
          "No available move. No FREE node was found with the needed value.");
    }

    return *it2;
  }
};

#endif // NIGHTHAWK_COMPLETEMENT_H
