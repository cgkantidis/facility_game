#ifndef FPLAYER_HIGHEST_H
#define FPLAYER_HIGHEST_H

#include "FPlayer.h"

class FPlayerHighest : public FPlayer {
private:
  static constexpr char const *PLAYER_NAME = "Highest";
  static constexpr char const *VERSION = "1.0";
  static constexpr char const *FIRSTNAME = "";
  static constexpr char const *LASTNAME = "";

  std::size_t m_last_idx{};
  std::vector<std::size_t> m_indices_sorted_by_node_value;

public:
  explicit FPlayerHighest(Player player)
      : FPlayer(player, PLAYER_NAME, VERSION, FIRSTNAME, LASTNAME) {}

  void initialize([[maybe_unused]] FacilityGame const &game) override {
    auto const &nodes = game.get_nodes(); // copy

    std::vector<std::pair<std::size_t, std::size_t>> idx_node(nodes.size());
    for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
      idx_node[idx] = {idx, nodes[idx]};
    }

    std::stable_sort(
        idx_node.begin(),
        idx_node.end(),
        [](auto const &left, auto const &right) {
          return left.second > right.second;
        });

    m_indices_sorted_by_node_value.resize(nodes.size());
    for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
      m_indices_sorted_by_node_value[idx] = idx_node[idx].first;
    }
  }

  // return the next largest node available
  std::size_t next_move(FacilityGame const &game) override {
    auto const &statuses = game.get_statuses();
    for (; m_last_idx < m_indices_sorted_by_node_value.size(); ++m_last_idx) {
      if (statuses[m_indices_sorted_by_node_value[m_last_idx]]
          == FacilityStatus::FREE) {
        return m_indices_sorted_by_node_value[m_last_idx];
      }
    }
    std::unreachable();
  }
};

#endif // FPLAYER_HIGHEST_H
