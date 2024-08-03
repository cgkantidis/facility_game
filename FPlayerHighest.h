#ifndef FPLAYER_HIGHEST_H
#define FPLAYER_HIGHEST_H

#include "FPlayer.h"

class FPlayerHighest : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "Highest";
  static constexpr char const *VERSION = "1.0";
  static constexpr char const *FIRSTNAME = "";
  static constexpr char const *LASTNAME = "";

public:
  explicit FPlayerHighest(Player player)
      : FPlayer(player, PLAYER_NAME, VERSION, FIRSTNAME, LASTNAME) {}

  void initialize([[maybe_unused]] FacilityGame const &game) override {}

  // return the next largest node available
  std::size_t next_move(FacilityGame const &game) override {
    std::size_t max_node{};
    std::size_t max_node_idx{};
    auto const &nodes = game.get_nodes();
    auto const &statuses = game.get_statuses();
    for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
      auto const &status = statuses[idx];
      auto const &node = nodes[idx];
      if (status == FacilityStatus::FREE && node > max_node) {
        max_node_idx = idx;
        max_node = node;
      }
    }
    return max_node_idx;
  }
};

#endif // FPLAYER_HIGHEST_H
