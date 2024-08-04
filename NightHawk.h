#ifndef NIGHTHAWK_H
#define NIGHTHAWK_H

#include <algorithm>
#include <array>
#include <cmath>
#include <fmt/ranges.h>
#include <map>

#include "FPlayer.h"
#include "FacilityGameException.h"

//#define ORIGIN_TRACKING

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

class NightHawk : public FPlayer {
  static constexpr char const *PLAYER_NAME = "NightHawk";
  static constexpr char const *VERSION = "1.0";
  static constexpr char const *FIRSTNAME = "Christos";
  static constexpr char const *LASTNAME = "Gkantidis";

private:
  std::size_t m_num_nodes{};
  std::vector<std::size_t> m_nodes;
  std::vector<std::size_t> m_my_moves_so_far;
  std::vector<std::size_t> m_vs_moves_so_far;
  std::vector<Node> m_sorted_nodes;
  std::vector<Node>::iterator m_sorted_nodes_it{};
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
              / static_cast<double>(m_my_moves_so_far.size()));
    }
  }
#endif

  void initialize(FacilityGame const &game) override {
    m_num_nodes = game.get_num_nodes();
    m_nodes = game.get_nodes();
    m_my_moves_so_far.reserve(m_num_nodes);
    m_vs_moves_so_far.reserve(m_num_nodes);
    m_sorted_nodes.reserve(m_num_nodes);
    m_sorted_nodes_it = m_sorted_nodes.begin();

    for (std::size_t idx = 0; idx < m_num_nodes; ++idx) {
      m_sorted_nodes.emplace_back(idx, m_nodes[idx]);
    }
    std::ranges::stable_sort(
        m_sorted_nodes,
        [](Node const &n1, Node const &n2) -> bool {
          return n1.value > n2.value || n1.index < n2.index;
        });
  }

  static void append_move(std::vector<std::size_t> &moves, std::size_t idx) {
    auto it = std::lower_bound(moves.begin(), moves.end(), idx);
    moves.insert(it, idx);
  }

  std::size_t next_move(FacilityGame const &game) override {
    // if the opponent has made at least one move, add his last move to the
    // Vector with his moves so far
    if (!game.get_moves().empty()) {
      std::size_t vs_move = game.get_moves().back();
      append_move(m_vs_moves_so_far, vs_move);
    }

    // initialize my move
    bool is_valid = false;
    Move my_move{0, 0};
#ifdef ORIGIN_TRACKING
    std::string origin;
#endif

    // if I have made more than two moves, that means that I can now make
    // triplets, so search if there is a triplet to be made, and if the
    // points it gives are higher that those of the current move, change my
    // move
    if (m_my_moves_so_far.size() >= 2) {
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_edges(game, m_my_moves_so_far);
        if (tmp_valid && tmp_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
          origin = "triplet_edges";
#endif
          is_valid = true;
          my_move = tmp_move;
        }
      }
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_middle(game, m_my_moves_so_far);
        if (tmp_valid && tmp_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
          origin = "triplet_middle";
#endif
          is_valid = true;
          my_move = tmp_move;
        }
      }
    }

    // if no triplet was found in memory, or stored in my current move, and
    // no triplets could be made or incremented, choose the free node with
    // the highest value
    if (!is_valid) {
      auto [tmp_valid, tmp_move] = find_best_node(game);
      if (tmp_valid && tmp_move.value > my_move.value) {
#ifdef ORIGIN_TRACKING
        origin = "highest_node";
#endif
        is_valid = true;
        my_move = tmp_move;
      }
    }

    auto is_worth = [](std::size_t new_move, std::size_t old_move) {
      return 2.5 * static_cast<double>(new_move) / 3.0
             > static_cast<double>(old_move);
    };

    // if the opponent has made more than two moves, that means that he can
    // now make triplets, so search if there is a triplet to be made, and if
    // the points it gives are higher that those of the current move, change
    // my move to block his move
    if (m_my_moves_so_far.size() >= 2) {
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_edges(game, m_vs_moves_so_far);
        if (tmp_valid) {
          if (!is_valid
              || is_worth(
                  tmp_move.value + m_nodes[tmp_move.index],
                  my_move.value)) {
#ifdef ORIGIN_TRACKING
            origin = "block_edges";
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
            inc_best_triplet_by_middle(game, m_vs_moves_so_far);
        if (tmp_valid) {
          if (!is_valid
              || is_worth(
                  tmp_move.value + m_nodes[tmp_move.index],
                  my_move.value)) {
#ifdef ORIGIN_TRACKING
            origin = "block_middle";
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

    // add my last move to the Vector with my moves so far
    append_move(m_my_moves_so_far, my_move.index);
#ifdef ORIGIN_TRACKING
    auto it = m_origins.find(origin);
    if (it == m_origins.end()) {
      m_origins[origin] = 1;
    } else {
      ++(it->second);
    }
#endif

    // return my next move
    return my_move.index;
  }

  std::pair<bool, Move> find_best_node(FacilityGame const &game) {
    while (m_sorted_nodes_it != m_sorted_nodes.end()) {
      Node node = *m_sorted_nodes_it;
      ++m_sorted_nodes_it;
      if (game.get_status(node.index) == FacilityStatus::FREE) {
        return {true, {node.index, node.value}};
      }
    }
    return {false, {0, 0}};
  }

  std::pair<bool, Move> start_best_tripplet(FacilityGame const &game) {
    std::size_t max_sum{0};
    std::size_t sum{0};

    bool is_valid{false};
    std::size_t a{0};
    std::size_t b{0};
    std::size_t c{0};
    static constexpr auto FREE = FacilityStatus::FREE;

    for (std::size_t i = 0; i < m_num_nodes - 4; i++) {
      if (game.get_status(i) == FREE && game.get_status(i + 2) == FREE
          && game.get_status(i + 4) == FREE) {
        sum = m_nodes[i] + m_nodes[i + 2] + m_nodes[i + 4];
        if (sum > max_sum) {
          is_valid = true;
          max_sum = sum;
          a = i;
          b = i + 2;
          c = i + 4;
        }
      }
    }

    for (std::size_t i = 0; i < m_num_nodes - 5; i++) {
      if (game.get_status(i) == FREE && game.get_status(i + 2) == FREE
          && game.get_status(i + 5) == FREE) {
        sum = m_nodes[i] + m_nodes[i + 2] + m_nodes[i + 5];
        if (sum > max_sum) {
          is_valid = true;
          max_sum = sum;
          a = i;
          b = i + 2;
          c = i + 5;
        }
      }
    }

    for (std::size_t i = 0; i < m_num_nodes - 5; i++) {
      if (game.get_status(i) == FREE && game.get_status(i + 3) == FREE
          && game.get_status(i + 5) == FREE) {
        sum = m_nodes[i] + m_nodes[i + 3] + m_nodes[i + 5];
        if (sum > max_sum) {
          is_valid = true;
          max_sum = sum;
          a = i;
          b = i + 3;
          c = i + 5;
        }
      }
    }

    for (std::size_t i = 0; i < m_num_nodes - 6; i++) {
      if (game.get_status(i) == FREE && game.get_status(i + 3) == FREE
          && game.get_status(i + 6) == FREE) {
        sum = m_nodes[i] + m_nodes[i + 3] + m_nodes[i + 6];
        if (sum > max_sum) {
          max_sum = sum;
          a = i;
          b = i + 3;
          c = i + 6;
        }
      }
    }

    if (!is_valid) {
      return {false, {0, 0}};
    }

    std::array<Node, 3> abc{
        {{a, m_nodes[a]}, {b, m_nodes[b]}, {c, m_nodes[c]}}};
    std::ranges::sort(abc, NodeCmp());

    std::size_t value = static_cast<std::size_t>(
        std::round(2.5 * static_cast<double>(max_sum)));

    return {true, {abc[0].index, value}};
  }

  std::pair<bool, Move> inc_best_triplet_by_edges(
      FacilityGame const &game,
      std::vector<std::size_t> const &moves_so_far) {
    std::size_t move_idx = 1;
    std::size_t first = moves_so_far[0];
    std::size_t continuous = 1;

    bool is_valid = false;
    Move rtn_move{0, 0};

    while (move_idx < moves_so_far.size()) {
      if (moves_so_far[move_idx] - moves_so_far[move_idx - 1] <= 3) {
        continuous++;
      } else {
        if (continuous >= 2) {
          std::size_t last = moves_so_far[move_idx - 1];
          auto [tmp_valid, tmp_move] =
              computePointsForEdges(game, first, last, continuous);
          if (tmp_valid && tmp_move.value > rtn_move.value) {
            is_valid = true;
            rtn_move = tmp_move;
          }
        }
        first = moves_so_far[move_idx];
        continuous = 1;
      }
      move_idx++;
    }
    if (continuous >= 2) {
      std::size_t last = moves_so_far[move_idx - 1];
      auto [tmp_valid, tmp_move] =
          computePointsForEdges(game, first, last, continuous);
      if (tmp_valid && tmp_move.value > rtn_move.value) {
        is_valid = true;
        rtn_move = tmp_move;
      }
    }

    return {is_valid, rtn_move};
  }

  std::pair<bool, Move> computePointsForEdges(
      FacilityGame const &game,
      std::size_t first,
      std::size_t last,
      std::size_t continuous) {
    std::size_t points;
    static constexpr auto FREE = FacilityStatus::FREE;

    bool is_valid = false;
    Move rtn_move{0, 0};

    // checks if it can make or increment a triplet by adding to the left
    // of the left-most node
    if (first >= 2 && game.get_status(first - 2) == FREE) {
      if (continuous == 2) {
        points = 3 * m_nodes[first - 2] + 2 * (m_nodes[first] + m_nodes[last]);
      } else {
        points = 3 * m_nodes[first - 2];
      }
      if (points > rtn_move.value) {
        is_valid = true;
        rtn_move = {first - 2, points};
      }
    }
    if (first >= 3 && game.get_status(first - 3) == FREE) {
      if (continuous == 2) {
        points = 3 * m_nodes[first - 3] + 2 * (m_nodes[first] + m_nodes[last]);
      } else {
        points = 3 * m_nodes[first - 3];
      }
      if (points > rtn_move.value) {
        is_valid = true;
        rtn_move = {first - 3, points};
      }
    }
    // checks if it can make or increment a triplet by adding to the right
    // of the right-most node
    if (last <= m_num_nodes - 3 && game.get_status(last + 2) == FREE) {
      if (continuous == 2) {
        points = 2 * (m_nodes[first] + m_nodes[last]) + 3 * m_nodes[last + 2];
      } else {
        points = 3 * m_nodes[last + 2];
      }
      if (points > rtn_move.value) {
        is_valid = true;
        rtn_move = {last + 2, points};
      }
    }
    if (last <= m_num_nodes - 4 && game.get_status(last + 3) == FREE) {
      if (continuous == 2) {
        points = 2 * (m_nodes[first] + m_nodes[last]) + 3 * m_nodes[last + 3];
      } else {
        points = 3 * m_nodes[last + 3];
      }
      if (points > rtn_move.value) {
        is_valid = true;
        rtn_move = {last + 3, points};
      }
    }
    return {is_valid, rtn_move};
  }

  /**
   * Try to increment two nodes, separated by 1 free, 2 free or 3 free nodes.
   * In case of 1 free node, this node is selected. In case of 2 nodes, the
   * node with the highest value is selected. In case of 3 nodes, the middle
   * (2nd) node is selected (resulting in a triplet) if the points to be given
   * are more that 0.8 of the points to be given by selecting the 1st and and
   * 3rd nodes. That weigh in the values is because by selecting the 1st and
   * 2nd nodes, the triplet might be stopped by the opponent in the next
   * round, so this represents the risk to be taken.
   *
   * @param game
   *            the facility game instance
   */
  std::pair<bool, Move> inc_best_triplet_by_middle(
      FacilityGame const &game,
      std::vector<std::size_t> const &moves_so_far) {
    std::size_t i = 1;
    bool is_valid = false;
    Move rtn_move{0, 0};

    while (i < moves_so_far.size()) {
      std::size_t space = moves_so_far[i] - moves_so_far[i - 1];
      if (space >= 4 && space <= 6) {
        std::size_t first = moves_so_far[i - 1];
        std::size_t last = moves_so_far[i];
        auto [tmp_valid, tmp_move] = computePointsForMiddle(game, first, last);
        if (tmp_valid && tmp_move.value > rtn_move.value) {
          is_valid = true;
          rtn_move = tmp_move;
        }
      }
      i++;
    }

    return {is_valid, rtn_move};
  }

  /**
   * Calculates the point that will be gained if I connect two teams of nodes
   * by choosing one of the nodes between the teams
   *
   * @param game
   *            the facility game instance
   * @param first
   *            the right-most node of the left team of nodes
   * @param last
   *            the left-most node of the right team of nodes
   */
  std::pair<bool, Move> computePointsForMiddle(
      FacilityGame const &game,
      std::size_t first,
      std::size_t last) {
    static constexpr auto FREE = FacilityStatus::FREE;

    bool is_valid = false;
    Move rtn_move{0, 0};
    // if there is one free node between the two teams I can select only the
    // middle one
    if (last - first == 4 && game.get_status(first + 2) == FREE) {
      is_valid = true;
      rtn_move = {
          first + 2,
          2 * m_nodes[first] + 3 * m_nodes[first + 2] + 2 * m_nodes[last]};
    }
    // if there are two FREE m_nodes between the two teams I can create a new
    // bigger team by selecting anyone of them, so I select the one with the
    // highest value
    else if (last - first == 5) {
      // if both FREE, select the one with the highest value
      if (game.get_status(first + 2) == FREE
          && game.get_status(first + 3) == FREE) {
        is_valid = true;
        if (m_nodes[first + 2] > m_nodes[first + 3]) {
          rtn_move.index = first + 2;
        } else {
          rtn_move.index = first + 3;
        }
      }
      // else select the one who is FREE (if any)
      else if (game.get_status(first + 2) == FREE) {
        is_valid = true;
        rtn_move.index = first + 2;
      } else if (game.get_status(first + 3) == FREE) {
        is_valid = true;
        rtn_move.index = first + 3;
      }

      if (is_valid) {
        rtn_move.value = 2 * m_nodes[first] + 3 * m_nodes[rtn_move.index]
                         + 2 * m_nodes[last];
      }
    }
    // if there are three FREE m_nodes between the two teams, I can either
    // select the middle one and combine the two teams, or select the left
    // or right and combine the two teams on the next round
    // the left/right m_nodes are computed with a weight in their points
    // because of the probability that one of them might be taken by the
    // opponent
    else if (last - first == 6) {
      if (game.get_status(first + 3) == FREE) {
        std::size_t tmp_points =
            2 * m_nodes[first] + 3 * m_nodes[first + 3] + 2 * m_nodes[last];
        if (!is_valid || tmp_points >= rtn_move.value) {
          is_valid = true;
          rtn_move = {first + 3, tmp_points};
        }
      }
    }
    return {is_valid, rtn_move};
  }
};

#endif // NIGHTHAWK_H
