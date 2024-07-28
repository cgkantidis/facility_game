#ifndef FPLAYER_SIMPLE1_H
#define FPLAYER_SIMPLE1_H

#include <algorithm>

#include "FPlayer.h"
#include "FacilityGameException.h"

class FPlayerSimple1 : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "SimpleFPlayer1";
  static constexpr char const *VERSION = "1.2";
  static constexpr int AFM = 11111;
  static constexpr char const *FIRSTNAME = "Data";
  static constexpr char const *LASTNAME = "Structures";

public:
  explicit FPlayerSimple1(EnumPlayer player)
      : FPlayer(player, PLAYER_NAME, VERSION, AFM, FIRSTNAME, LASTNAME) {}

  // return the first free node
  std::size_t next_move(FacilityGameAPI const *game) override {
    auto const &status_vec = game->get_status_vec();
    auto it = std::ranges::find(status_vec, EnumFacilityStatus::FREE);

    if (it == status_vec.end()) {
      throw FacilityGameException("No available move");
    }

    return static_cast<std::size_t>(std::distance(it, status_vec.begin()));
  }
};

#endif // FPLAYER_SIMPLE1_H
