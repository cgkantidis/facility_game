#ifndef FPLAYER_SIMPLE1_H
#define FPLAYER_SIMPLE1_H

#include <algorithm>

#include "FPlayer.h"
#include "FacilityGameException.h"

class FPlayerLinear : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "SimpleFPlayer1";
  static constexpr char const *VERSION = "1.2";
  static constexpr char const *FIRSTNAME = "Data";
  static constexpr char const *LASTNAME = "Structures";

public:
  explicit FPlayerLinear(Player player)
      : FPlayer(player, PLAYER_NAME, VERSION, FIRSTNAME, LASTNAME) {}

  void initialize([[maybe_unused]] FacilityGame const &game) override {}

  // return the first free node
  std::size_t next_move(FacilityGame const &game) override {
    auto const &statuses = game.get_statuses();
    auto it = std::ranges::find(statuses, FacilityStatus::FREE);

    if (it == statuses.end()) {
      throw FacilityGameException("No available move");
    }

    return static_cast<std::size_t>(std::distance(statuses.begin(), it));
  }
};

#endif // FPLAYER_SIMPLE1_H
