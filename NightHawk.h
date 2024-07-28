#ifndef NIGHTHAWK_H
#define NIGHTHAWK_H

#include <algorithm>
#include <array>
#include <cmath>

#include "EnumFacilityStatus.h"
#include "FPlayer.h"
#include "Node.h"

struct Move {
  std::size_t index;
  std::size_t value;
};

struct MoveCmp {
  bool operator()(Move const &m1, Move const &m2) {
    return m1.value > m2.value || m1.index < m2.index;
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
  static constexpr int AFM = 56483;
  static constexpr char const *FIRSTNAME = "Christos";
  static constexpr char const *LASTNAME = "Gkantidis";

private:
  std::size_t m_num_nodes{};
  std::vector<std::size_t> m_nodes;
  std::vector<std::size_t> m_my_moves_so_far;
  std::vector<std::size_t> m_vs_moves_so_far;
  std::vector<Move> m_mem_for_next_move;
  std::vector<Node> m_sorted_nodes;
  std::vector<Node>::iterator m_sorted_nodes_it{};

public:
  explicit NightHawk(EnumPlayer player)
      : FPlayer(player, PLAYER_NAME, VERSION, AFM, FIRSTNAME, LASTNAME) {}

  void initialize(FacilityGameAPI const *game) override {
    m_num_nodes = game->get_num_nodes();
    m_nodes = game->get_value_vec();
    m_my_moves_so_far.reserve(m_num_nodes / 3);
    m_vs_moves_so_far.reserve(m_num_nodes / 3);
    m_mem_for_next_move.reserve(3);
    m_sorted_nodes.reserve(m_num_nodes);
    m_sorted_nodes_it = m_sorted_nodes.begin();

    for (std::size_t idx = 0; idx < m_num_nodes; ++idx) {
      m_sorted_nodes[idx] = {idx, m_nodes[idx]};
    }
    std::ranges::stable_sort(
        m_sorted_nodes,
        [](Node const &n1, Node const &n2) -> bool {
          return n1.value > n2.value || n1.index < n2.index;
        });
  }

  std::size_t next_move(FacilityGameAPI const *game) override {
    // if the opponent has made at least one move, add his last move to the
    // Vector with his moves so far
    if (game->get_num_moves() > 0) {
      std::size_t vs_move = game->get_move_locations().back();
      m_vs_moves_so_far.emplace_back(vs_move);
    }

    // initialize my move
    bool is_valid = false;
    Move my_move{0, 0};

    // if there is a move in memory to be made, put it in my move, and
    // delete it from memory if it's played
    while (!m_mem_for_next_move.empty()) {
      // get the first node
      Move tmp_move = m_mem_for_next_move.front();
      // if the first node is free, make it my move
      if (game->get_status(tmp_move.index) == EnumFacilityStatus::FREE) {
        if (tmp_move.value > my_move.value) {
          is_valid = true;
          my_move = tmp_move;
          break;
        }
      } else {
        // else the first move in memory couldn't be played and if the
        // memory is full, empty the memory as the triplet is now
        // screwed
        if (m_mem_for_next_move.size() == 3) {
          m_mem_for_next_move.clear();
        } else {
          m_mem_for_next_move.erase(m_mem_for_next_move.begin());
        }
      }
    }

    // if the memory was empty, or the next move didn't get a value from
    // memory, find the next best triplet and make it my move
    if (!is_valid) {
      auto [tmp_valid, tmp_move] = start_best_tripplet(game);
      if (tmp_valid && tmp_move.value > my_move.value) {
        is_valid = true;
        my_move = tmp_move;
      }
    }

    // if I have made more than two moves, that means that I can now make
    // triplets, so search if there is a triplet to be made, and if the
    // points it gives are higher that those of the current move, change my
    // move
    if (m_my_moves_so_far.size() >= 2) {
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_edges(game, m_my_moves_so_far);
        if (tmp_valid && tmp_move.value > my_move.value) {
          is_valid = true;
          my_move = tmp_move;
        }
      }
      {
        auto [tmp_valid, tmp_move] =
            inc_best_triplet_by_middle(game, m_my_moves_so_far);
        if (tmp_valid && tmp_move.value > my_move.value) {
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
          if (is_worth(tmp_move.value, my_move.value)) {
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
          if (is_worth(tmp_move.value, my_move.value)) {
            my_move = {
                tmp_move.index,
                tmp_move.value + m_nodes[tmp_move.index]};
          }
        }
      }
    }

    // add my last move to the Vector with my moves so far
    append_my_move(my_move);

    // if the current move is the one that I got from memory, remove it from
    // memory
    if (!m_mem_for_next_move.empty()) {
      Move tmp_move = m_mem_for_next_move.front();
      if (my_move.index == tmp_move.index) {
        m_mem_for_next_move.erase(m_mem_for_next_move.begin());
      }
    }

    // return my next move
    return my_move.index;
  }

  std::pair<bool, Move> find_best_node(FacilityGameAPI const *game) {
    while (m_sorted_nodes_it != m_sorted_nodes.end()) {
      Node node = *m_sorted_nodes_it;
      ++m_sorted_nodes_it;
      if (game->get_status(node.index) == EnumFacilityStatus::FREE) {
        return {true, {node.index, node.value}};
      }
    }
    return {false, {0, 0}};
  }

  std::pair<bool, Move> start_best_tripplet(FacilityGameAPI const *game) {
    std::size_t max_sum{0};
    std::size_t sum{0};

    bool is_valid{false};
    std::size_t a{0};
    std::size_t b{0};
    std::size_t c{0};
    static constexpr auto free = EnumFacilityStatus::FREE;

    for (std::size_t i = 0; i < m_num_nodes - 4; i++) {
      if (game->get_status(i) == free && game->get_status(i + 2) == free
          && game->get_status(i + 4) == free) {
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
      if (game->get_status(i) == free && game->get_status(i + 2) == free
          && game->get_status(i + 5) == free) {
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
      if (game->get_status(i) == free && game->get_status(i + 3) == free
          && game->get_status(i + 5) == free) {
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
      if (game->get_status(i) == free && game->get_status(i + 3) == free
          && game->get_status(i + 6) == free) {
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

    if (abc[0].index == a) {
      m_mem_for_next_move.emplace_back(a, value);
      m_mem_for_next_move.emplace_back(b, value);
      m_mem_for_next_move.emplace_back(c, value);
    } else if (abc[0].index == c) {
      m_mem_for_next_move.emplace_back(c, value);
      m_mem_for_next_move.emplace_back(b, value);
      m_mem_for_next_move.emplace_back(a, value);
    } else {
      m_mem_for_next_move.emplace_back(abc[0].index, value);
      m_mem_for_next_move.emplace_back(abc[1].index, value);
      m_mem_for_next_move.emplace_back(abc[2].index, value);
    }

    return {true, {abc[0].index, value}};
  }

  std::pair<bool, Move> inc_best_triplet_by_edges(
      FacilityGameAPI const *game,
      std::vector<std::size_t> &movesSoFar) {
    int i = 1;
    int first = movesSoFar.get(0);
    int last = 0;
    int continuous = 1;
    Pair tmpPair;
    Pair toReturn = new Pair(-1, 0);

    while (i < movesSoFar.size()) {
      if (movesSoFar.get(i) - movesSoFar.get(i - 1) <= 3) {
        continuous++;
      } else {
        last = movesSoFar.get(i - 1);
        if (continuous >= 2) {
          tmpPair = computePointsForEdges(game, first, last, continuous);
          if (tmpPair.value > toReturn.value) {
            toReturn = new Pair(tmpPair.index, tmpPair.value);
          }
        }
        first = movesSoFar.get(i);
        continuous = 1;
      }
      i++;
    }
    if (continuous >= 2) {
      last = movesSoFar.get(i - 1);
      tmpPair = computePointsForEdges(game, first, last, continuous);
      if (tmpPair.value > toReturn.value) {
        toReturn = new Pair(tmpPair.index, tmpPair.value);
      }
    }

    return toReturn;
  }

  /**
   * Calculates the points that will be gained by incrementing a team of nodes
   * by choosing one of the nodes at the edges of the team
   *
   * @param game
   *            the facility game instance
   * @param first
   *            the left-most node of the team
   * @param last
   *            the right-most node of the team
   * @param continuous
   *            the number of nodes in the team
   */
  std::pair<bool, Move> computePointsForEdges(
      FacilityGameAPI const *game,
      std::size_t first,
      std::size_t last,
      std::size_t continuous) {
    std::size_t points;
    // checks if it can make or increment a triplet by adding to the left
    // of the left-most node
    if (first >= 2 && game->get_status(first - 2) == free) {
      if (continuous == 2) {
        points = 3 * (nodes[first - 2] + nodes[first] + nodes[last]);
      } else {
        points = 3 * nodes[first - 2];
      }
      if (points > toReturn.value) {
        toReturn = new Pair(first - 2, points);
      }
    }
    if (first >= 3 && game->get_status(first - 3) == free) {
      if (continuous == 2) {
        points = 3 * (nodes[first - 3] + nodes[first] + nodes[last]);
      } else {
        points = 3 * nodes[first - 3];
      }
      if (points > toReturn.value) {
        toReturn = new Pair(first - 3, points);
      }
    }
    // checks if it can make or increment a triplet by adding to the right
    // of the right-most node
    if (last <= n - 3 && game->get_status(last + 2) == free) {
      if (continuous == 2) {
        points = 3 * (nodes[first] + nodes[last] + nodes[last + 2]);
      } else {
        points = 3 * nodes[last + 2];
      }
      if (points > toReturn.value) {
        toReturn = new Pair(last + 2, points);
      }
    }
    if (last <= n - 4 && game->get_status(last + 3) == free) {
      if (continuous == 2) {
        points = 3 * (nodes[first] + nodes[last] + nodes[last + 3]);
      } else {
        points = 3 * nodes[last + 3];
      }
      if (points > toReturn.value) {
        toReturn = new Pair(last + 3, points);
      }
    }
    return {false, {0, 0}};
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
      FacilityGameAPI const *game,
      std::vector<std::size_t> &movesSoFar) {
    int i = 1;
    int first;
    int last;
    Pair tmpPair;
    Pair toReturn = new Pair(-1, 0);

    while (i < movesSoFar.size()) {
      int space = movesSoFar.get(i) - movesSoFar.get(i - 1);
      if (space >= 4 && space <= 6) {
        first = movesSoFar.get(i - 1);
        last = movesSoFar.get(i);
        tmpPair = computePointsForMiddle(game, first, last);
        if (tmpPair.value > toReturn.value) {
          toReturn = new Pair(tmpPair.index, tmpPair.value);
        }
      }
      i++;
    }

    return toReturn;
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
  std::pair<bool, Move>
  computePointsForMiddle(FacilityGameAPI game, int first, int last) {
    int points = 0;
    int move = -1;
    // if there is one free node between the two teams I can select only the
    // middle one
    if (last - first == 4 && game->get_status(first + 2) == free) {
      move = first + 2;
      points = 3 * (nodes[first] + nodes[move] + nodes[last]);
    }
    // if there are two free nodes between the two teams I can create a new
    // bigger team by selecting anyone of them, so I select the one with the
    // highest value
    else if (last - first == 5) {
      // if both free, select the one with the highest value
      if (game->get_status(first + 2) == free
          && game->get_status(first + 3) == free) {
        if (nodes[first + 2] > nodes[first + 3]) {
          move = first + 2;
        } else {
          move = first + 3;
        }
      }
      // else select the one who is free (if any)
      else if (game->get_status(first + 2) == free) {
        move = first + 2;
      } else if (game->get_status(first + 3) == free) {
        move = first + 3;
      }

      if (move != -1) {
        points = 3 * (nodes[first] + nodes[move] + nodes[last]);
      }
    }
    // if there are three free nodes between the two teams, I can either
    // select the middle one and combine the two teams, or select the left
    // or right and combine the two teams on the next round
    // the left/right nodes are computed with a weight in their points
    // because of the probability that one of them might be taken by the
    // opponent
    else if (last - first == 6) {
      int maxPoints = 0;
      if (game->get_status(first + 2) == free
          && game->get_status(first + 4) == free) {
        if (nodes[first + 2] > nodes[first + 4]) {
          move = first + 2;
        } else {
          move = first + 4;
        }

        maxPoints = (int)Math.floor(
            0.8
            * (2 * nodes[first] + 3 * nodes[first + 2] + 3 * nodes[first + 4]
               + 2 * nodes[last]));
      }
      if (game->get_status(first + 3) == free) {
        int tempPoints = 3 * (nodes[first] + nodes[first + 3] + nodes[last]);
        if (tempPoints >= maxPoints) {
          move = first + 3;
          maxPoints = tempPoints;
        }
      }
      points = maxPoints;
    }
    return new Pair(move, points);
  }

  void append_my_move(Move my_move) {
    m_my_moves_so_far.push_back(my_move);
    std::ranges::push_heap(m_my_moves_so_far);
  }

  void append_vs_move(Move vs_move) {
    m_my_moves_so_far.push_back(vs_move);
    std::ranges::push_heap(m_vs_moves_so_far);
  }
};

#endif // NIGHTHAWK_H
