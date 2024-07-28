#ifndef FPLAYER_SIMPLE2_H
#define FPLAYER_SIMPLE2_H

#include <random>

#include "FPlayer.h"
#include "FacilityGameException.h"

class FPlayerSimple2 : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "SimpleFPlayer2";
  static constexpr char const *VERSION = "1.4";
  static constexpr int AFM = 11111;
  static constexpr char const *FIRSTNAME = "Data";
  static constexpr char const *LASTNAME = "Structures";

  std::size_t m_num_nodes{};
  std::size_t m_start_node{};
  bool m_left_to_right{};

public:
  explicit FPlayerSimple2(EnumPlayer player)
      : FPlayer(player, PLAYER_NAME, VERSION, AFM, FIRSTNAME, LASTNAME) {}

  void initialize([[maybe_unused]] FacilityGameAPI const *game) override {
    m_num_nodes = game->get_num_nodes();
    // seed the generator witht the value of the first node
    std::mt19937 gen(game->get_value(0));
    m_start_node =
        std::uniform_int_distribution<std::size_t>(0, m_num_nodes - 1)(gen);
    m_left_to_right = std::uniform_int_distribution<int>(0, 1)(gen) == 0;
  }

  std::size_t next_move(FacilityGameAPI const *game) override {
    // TODO: consider using std::ranges
    for (std::size_t idx = 0; idx < m_num_nodes; idx++) {
      std::size_t wrap_idx =
          m_left_to_right ? (m_start_node + idx) % m_num_nodes
                          : (m_num_nodes + m_start_node - idx) % m_num_nodes;
      if (game->get_status(wrap_idx) == EnumFacilityStatus::FREE) {
        return wrap_idx;
      }
    }
    throw FacilityGameException("No available move");
  }
};

#endif // FPLAYER_SIMPLE2_H
