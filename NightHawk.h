#ifndef NIGHTHAWK_H
#define NIGHTHAWK_H

#include <algorithm>
#include <array>
#include <fmt/ranges.h>
#include <map>

#include "FPlayer.h"
#include "FacilityGameException.h"

#define ORIGIN_TRACKING

struct Move {
  std::size_t index;
  std::size_t value;
};

struct MoveCmp {
  bool operator()(Move const &m1, Move const &m2) {
    return m1.value > m2.value || m1.index < m2.index;
  }
};

struct Node {
  std::size_t index;
  std::size_t value;

  std::string toString() {
    return fmt::format("node[{}] == {}", index, value);
  }
};

struct NodeCmp {
  bool operator()(Node const &n1, Node const &n2) {
    return n1.value > n2.value || n1.index < n2.index;
  }
};

struct Group {
  std::size_t left_idx;
  std::size_t right_idx;
  std::size_t num_nodes;
  std::size_t value;
  bool blocked_left;
  bool blocked_right;
};

void append_move_to_groups(
    std::vector<Group> &groups,
    std::size_t idx,
    std::size_t node) {

  // create a new group, just for this move
  auto it = std::upper_bound(
      groups.begin(),
      groups.end(),
      idx,
      [](std::size_t value, Group const &group) {
        return value < group.left_idx;
      });
  it = groups.insert(it, {idx, idx, 1, node, false, false});

  // try to merge with next
  auto next_it = std::next(it);
  if (next_it != groups.end()) {
    if (next_it->left_idx - it->right_idx <= 3) {
      it->right_idx = next_it->right_idx;
      it->num_nodes += next_it->num_nodes;
      it->value += next_it->value;
      groups.erase(next_it);
    }
  }

  // try to merge with prev
  if (it != groups.begin()) {
    auto prev_it = std::prev(it);
    if (it->left_idx - prev_it->right_idx <= 3) {
      it->left_idx = prev_it->left_idx;
      it->num_nodes += prev_it->num_nodes;
      it->value += prev_it->value;
      groups.erase(prev_it);
    }
  }
}

class NightHawk : public FPlayer {
  static constexpr char const *PLAYER_NAME = "NightHawk";
  static constexpr char const *VERSION = "1.0";
  static constexpr char const *FIRSTNAME = "Christos";
  static constexpr char const *LASTNAME = "Gkantidis";

private:
  std::size_t m_num_nodes{};
  std::vector<std::size_t> m_nodes;
  std::vector<Group> m_my_groups;
  std::vector<Group> m_vs_groups;

  std::vector<Node> m_sorted_nodes;
  std::vector<Node>::iterator m_sorted_nodes_it{};
  std::size_t m_old_score{};
  std::size_t m_expected{};
  std::string m_origin;
  std::size_t m_num_my_moves{};
  std::size_t m_num_vs_moves{};

#ifdef ORIGIN_TRACKING
  std::map<std::string, std::size_t> m_origins;
#endif

public:
  explicit NightHawk(Player player)
      : FPlayer(player, PLAYER_NAME, VERSION, FIRSTNAME, LASTNAME) {}

#ifdef ORIGIN_TRACKING
  ~NightHawk() override {
    std::vector<std::pair<std::string, std::size_t>> origins(
        m_origins.begin(),
        m_origins.end());
    std::sort(
        origins.begin(),
        origins.end(),
        [](auto const &p1, auto const &p2) { return p1.second > p2.second; });
    for (auto const &p : origins) {
      fmt::println(
          "{}: {} ({:.2f}%)",
          p.first,
          p.second,
          100.0 * static_cast<double>(p.second)
              / static_cast<double>(m_num_my_moves));
    }
  }
#endif

  void initialize(FacilityGame const &game) override {
    m_num_nodes = game.get_num_nodes();
    m_nodes = game.get_nodes();
    m_sorted_nodes.reserve(m_num_nodes);

    for (std::size_t idx = 0; idx < m_num_nodes; ++idx) {
      m_sorted_nodes.emplace_back(idx, m_nodes[idx]);
    }
    std::ranges::stable_sort(
        m_sorted_nodes,
        [](Node const &n1, Node const &n2) -> bool {
          return n1.value > n2.value || n1.index < n2.index;
        });
    m_sorted_nodes_it = m_sorted_nodes.begin();
  }

  std::size_t next_move(FacilityGame const &game) override {
#ifdef ORIGIN_TRACKING
    std::size_t new_score = game.get_score(m_player);
    if (m_old_score != 0 && new_score - m_old_score != m_expected) {
      fmt::println(
          "On move {} expected {} but gained {} ({})",
          m_num_my_moves,
          m_expected,
          new_score - m_old_score,
          m_origin);
    }
    m_old_score = new_score;
#endif

    // if the opponent has made at least one move, add his last move to the
    // Vector with his moves so far
    if (!game.get_moves().empty()) {
      ++m_num_vs_moves;
      std::size_t vs_move = game.get_moves().back();
      append_move_to_groups(m_vs_groups, vs_move, m_nodes[vs_move]);
    }

    // initialize my move
    bool is_valid = false;
    Move my_move{0, 0};

    // if I have made more than two moves, that means that I can now make
    // triplets, so search if there is a triplet to be made, and if the
    // points it gives are higher that those of the current move, change my
    // move
    if (m_num_my_moves >= 2) {
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_edges(game, m_my_groups);
        if (tmp_valid && tmp_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
          m_origin = "triplet_edges";
#endif
          is_valid = true;
          my_move = tmp_move;
        }
      }
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_middle(game, m_my_groups);
        if (tmp_valid && tmp_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
          m_origin = "triplet_middle";
#endif
          is_valid = true;
          my_move = tmp_move;
        }
      }
    }

    // choose the free node with the highest value
    auto highest_node_move = find_best_node(game);
    if (highest_node_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
      m_origin = "highest_node";
#endif
      is_valid = true;
      my_move = highest_node_move;
    }

    auto is_worth = [](std::size_t new_move, std::size_t old_move) {
      return 2.5 * static_cast<double>(new_move) / 3.0
             > static_cast<double>(old_move);
    };

    // if the opponent has made more than two moves, that means that he can
    // now make triplets, so search if there is a triplet to be made, and if
    // the points it gives are higher that those of the current move, change
    // my move to block his move
    if (m_num_vs_moves >= 2) {
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_edges(game, m_vs_groups);
        if (tmp_valid) {
          if (is_worth(
                  tmp_move.value + m_nodes[tmp_move.index],
                  my_move.value)) {
#ifdef ORIGIN_TRACKING
            m_origin = "block_edges";
#endif
            is_valid = true;
            my_move = {
                tmp_move.index,
                tmp_move.value + m_nodes[tmp_move.index]};
          }
        }
      }
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_middle(game, m_vs_groups);
        if (tmp_valid) {
          if (is_worth(
                  tmp_move.value + m_nodes[tmp_move.index],
                  my_move.value)) {
#ifdef ORIGIN_TRACKING
            m_origin = "block_middle";
#endif
            is_valid = true;
            my_move = {
                tmp_move.index,
                tmp_move.value + m_nodes[tmp_move.index]};
          }
        }
      }
    }

    if (!is_valid) {
      throw FacilityGameException("No valid move was found");
    }

    m_expected = my_move.value;

    // add my last move to the Vector with my moves so far
    ++m_num_my_moves;
    append_move_to_groups(
        m_my_groups,
        my_move.index,
        game.get_node(my_move.index));

#ifdef ORIGIN_TRACKING
    auto it = m_origins.find(m_origin);
    if (it == m_origins.end()) {
      m_origins[m_origin] = 1;
    } else {
      ++(it->second);
    }
#endif

    // return my next move
    return my_move.index;
  }

private:
  Move find_best_node(FacilityGame const &game) {
    while (m_sorted_nodes_it != m_sorted_nodes.end()) {
      Node node = *m_sorted_nodes_it;
      ++m_sorted_nodes_it;
      if (game.get_status(node.index) == FacilityStatus::FREE) {
        return {node.index, node.value};
      }
    }
    std::unreachable();
  }

  static std::size_t
  compute_points_edges(Group const &group, std::size_t node) {
    if (group.num_nodes == 1) {
      return node;
    }
    if (group.num_nodes == 2) {
      return 3 * node + 2 * group.value;
    }
    return 3 * node;
  }

  std::pair<bool, Move> inc_best_triplet_by_edges(
      FacilityGame const &game,
      std::vector<Group> &groups) {
    auto const &statuses = game.get_statuses();
    auto const &nodes = game.get_nodes();

    bool is_valid = false;
    Move rtn_move{0, 0};

    for (auto &group : groups) {
      if (!group.blocked_left) {
        bool found = false;
        if (group.left_idx >= 2) {
          std::size_t idx = group.left_idx - 2;
          if (statuses[idx] == FacilityStatus::FREE) {
            found = true;
            std::size_t points = compute_points_edges(group, nodes[idx]);
            if (!is_valid || points > rtn_move.value) {
              is_valid = true;
              rtn_move = {idx, points};
            }
          }
        }
        if (group.left_idx >= 3) {
          std::size_t idx = group.left_idx - 3;
          if (statuses[idx] == FacilityStatus::FREE) {
            found = true;
            std::size_t points = compute_points_edges(group, nodes[idx]);
            if (!is_valid || points > rtn_move.value) {
              is_valid = true;
              rtn_move = {idx, points};
            }
          }
        }
        if (!found) {
          group.blocked_left = true;
        }
      }

      if (!group.blocked_right) {
        bool found = false;
        if (nodes.size() - group.right_idx > 2) {
          std::size_t idx = group.right_idx + 2;
          if (statuses[idx] == FacilityStatus::FREE) {
            found = true;
            std::size_t points = compute_points_edges(group, nodes[idx]);
            if (!is_valid || points > rtn_move.value) {
              is_valid = true;
              rtn_move = {idx, points};
            }
          }
        }
        if (nodes.size() - group.left_idx > 3) {
          std::size_t idx = group.left_idx + 3;
          if (statuses[idx] == FacilityStatus::FREE) {
            found = true;
            std::size_t points = compute_points_edges(group, nodes[idx]);
            if (!is_valid || points > rtn_move.value) {
              is_valid = true;
              rtn_move = {idx, points};
            }
          }
        }
        if (!found) {
          group.blocked_right = true;
        }
      }
    }

    return {is_valid, rtn_move};
  }

  static std::size_t compute_points_middle(
      Group const &left,
      Group const &right,
      std::size_t node) {
    if (left.num_nodes > 2 && right.num_nodes > 2) {
      return 3 * node;
    }
    if (left.num_nodes > 2) {
      return 3 * node + 2 * right.value;
    }
    if (right.num_nodes > 2) {
      return 2 * left.value + 3 * node;
    }
    return 2 * (left.value + right.value) + 3 * node;
  }

  std::pair<bool, Move> inc_best_triplet_by_middle(
      FacilityGame const &game,
      std::vector<Group> const &groups) {
    auto const &nodes = game.get_nodes();

    bool is_valid = false;
    Move rtn_move{0, 0};

    for (std::size_t group_idx = 1; group_idx < groups.size(); ++group_idx) {
      Group const &left = groups[group_idx - 1];
      Group const &right = groups[group_idx];

      if (left.blocked_right || right.blocked_left) {
        continue;
      }

      std::size_t diff = right.left_idx - left.right_idx;
      if (diff > 6) {
        continue;
      }

      if (diff == 6 || diff == 4) {
        std::size_t idx = left.right_idx + diff / 2;
        std::size_t points = compute_points_middle(left, right, nodes[idx]);
        if (!is_valid || points > rtn_move.value) {
          is_valid = true;
          rtn_move = {idx, points};
        }
      } else if (diff == 5) {
        for (std::size_t idx : {left.right_idx + 2, left.right_idx + 3}) {
          std::size_t points = compute_points_middle(left, right, nodes[idx]);
          if (!is_valid || points > rtn_move.value) {
            is_valid = true;
            rtn_move = {idx, points};
          }
        }
      }
    }

    return {is_valid, rtn_move};
  }
};

#endif // NIGHTHAWK_H
