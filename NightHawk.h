#ifndef NIGHTHAWK_H
#define NIGHTHAWK_H

#include <algorithm>
#include <array>
#include <cmath>
#include <fmt/ranges.h>

#include "FPlayer.h"

struct Move {
  std::size_t index;
  std::size_t value;
};

class NightHawk : public FPlayer {
  static constexpr char const *PLAYER_NAME = "NightHawk";
  static constexpr char const *VERSION = "1.0";
  static constexpr char const *FIRSTNAME = "Christos";
  static constexpr char const *LASTNAME = "Gkantidis";

private:
  std::size_t m_num_nodes{};
  std::vector<std::size_t> m_nodes;
  std::vector<std::size_t> m_my_moves;
  std::vector<std::size_t> m_vs_moves;
  std::vector<Move> m_followup_moves;

public:
  explicit NightHawk(Player player)
      : FPlayer(player, PLAYER_NAME, VERSION, FIRSTNAME, LASTNAME) {}

  void initialize(FacilityGame const &game) override {
    m_num_nodes = game.get_num_nodes();
    m_nodes = game.get_nodes();
  }

private:
  Move start_best_possible_triplet(FacilityGame const &game) {
    std::size_t max_sum{};
    std::size_t a{};
    std::size_t b{};
    std::size_t c{};

    for (std::size_t i = 0; i < m_nodes.size() - 4; i++) {
      if (game.get_status(i) == FacilityStatus::FREE
          && game.get_status(i + 2) == FacilityStatus::FREE
          && game.get_status(i + 4) == FacilityStatus::FREE) {
        std::size_t sum = m_nodes[i] + m_nodes[i + 2] + m_nodes[i + 4];
        if (sum > max_sum) {
          max_sum = sum;
          a = i;
          b = i + 2;
          c = i + 4;
        }
      }
    }

    for (std::size_t i = 0; i < m_nodes.size() - 5; i++) {
      if (game.get_status(i) == FacilityStatus::FREE
          && game.get_status(i + 2) == FacilityStatus::FREE
          && game.get_status(i + 5) == FacilityStatus::FREE) {
        std::size_t sum = m_nodes[i] + m_nodes[i + 2] + m_nodes[i + 5];
        if (sum > max_sum) {
          max_sum = sum;
          a = i;
          b = i + 2;
          c = i + 5;
        }
      }
    }

    for (std::size_t i = 0; i < m_nodes.size() - 5; i++) {
      if (game.get_status(i) == FacilityStatus::FREE
          && game.get_status(i + 3) == FacilityStatus::FREE
          && game.get_status(i + 5) == FacilityStatus::FREE) {
        std::size_t sum = m_nodes[i] + m_nodes[i + 3] + m_nodes[i + 5];
        if (sum > max_sum) {
          max_sum = sum;
          a = i;
          b = i + 3;
          c = i + 5;
        }
      }
    }

    for (std::size_t i = 0; i < m_nodes.size() - 6; i++) {
      if (game.get_status(i) == FacilityStatus::FREE
          && game.get_status(i + 3) == FacilityStatus::FREE
          && game.get_status(i + 6) == FacilityStatus::FREE) {
        std::size_t sum = m_nodes[i] + m_nodes[i + 3] + m_nodes[i + 6];
        if (sum > max_sum) {
          max_sum = sum;
          a = i;
          b = i + 3;
          c = i + 6;
        }
      }
    }

    if (max_sum > 0) {
      std::array<Move, 3> abc{};
      abc[0] = {a, m_nodes[a]};
      abc[1] = {b, m_nodes[b]};
      abc[2] = {c, m_nodes[c]};
      std::sort(abc.begin(), abc.end(), [](Move const &lhs, Move const &rhs) {
        return lhs.value > rhs.value;
      });

      auto expected_value =
          static_cast<std::size_t>(std::round(2.5 * (double)max_sum));
      if (abc[0].index == a) {
        m_followup_moves.emplace_back(a, expected_value);
        m_followup_moves.emplace_back(b, expected_value);
        m_followup_moves.emplace_back(c, expected_value);
      } else if (abc[0].index == c) {
        m_followup_moves.emplace_back(c, expected_value);
        m_followup_moves.emplace_back(b, expected_value);
        m_followup_moves.emplace_back(a, expected_value);
      } else {
        m_followup_moves.emplace_back(abc[0].index, expected_value);
        m_followup_moves.emplace_back(abc[1].index, expected_value);
        m_followup_moves.emplace_back(abc[2].index, expected_value);
      }
      return abc[0];
    }
    return {0, 0};
  }

  Move compute_points_for_edges(
      FacilityGame const &game,
      std::size_t first,
      std::size_t last,
      std::size_t continuous) {
    using FacilityStatus::FREE;
    Move to_rtn{0, 0};
    auto const &nodes = game.get_nodes();
    std::size_t const n = nodes.size();

    // checks if it can make or increment a triplet by adding to the left
    // of the left-most node
    if (first >= 2 && game.get_status(first - 2) == FREE) {
      std::size_t points{};
      if (continuous == 2) {
        points = 3 * (nodes[first - 2] + nodes[first] + nodes[last]);
      } else {
        points = 3 * nodes[first - 2];
      }
      if (points > to_rtn.value) {
        to_rtn = {first - 2, points};
      }
    }
    if (first >= 3 && game.get_status(first - 3) == FREE) {
      std::size_t points{};
      if (continuous == 2) {
        points = 3 * (nodes[first - 3] + nodes[first] + nodes[last]);
      } else {
        points = 3 * nodes[first - 3];
      }
      if (points > to_rtn.value) {
        to_rtn = {first - 3, points};
      }
    }
    // checks if it can make or increment a triplet by adding to the right
    // of the right-most node
    if (last <= n - 3 && game.get_status(last + 2) == FREE) {
      std::size_t points{};
      if (continuous == 2) {
        points = 3 * (nodes[first] + nodes[last] + nodes[last + 2]);
      } else {
        points = 3 * nodes[last + 2];
      }
      if (points > to_rtn.value) {
        to_rtn = {last + 2, points};
      }
    }
    if (last <= n - 4 && game.get_status(last + 3) == FREE) {
      std::size_t points{};
      if (continuous == 2) {
        points = 3 * (nodes[first] + nodes[last] + nodes[last + 3]);
      } else {
        points = 3 * nodes[last + 3];
      }
      if (points > to_rtn.value) {
        to_rtn = {last + 3, points};
      }
    }
    return to_rtn;
  }

  Move inc_best_triplet_by_edges(
      FacilityGame const &game,
      std::vector<std::size_t> const &moves) {
    std::size_t i = 1;
    std::size_t first = moves[0];
    std::size_t continuous = 1;
    Move to_rtn{0, 0};

    while (i < moves.size()) {
      if (moves[i] - moves[i - 1] <= 3) {
        continuous++;
      } else {
        if (continuous >= 2) {
          Move tmp_move =
              compute_points_for_edges(game, first, moves[i - 1], continuous);
          if (tmp_move.value > to_rtn.value) {
            to_rtn = tmp_move;
          }
        }
        first = moves[i];
        continuous = 1;
      }
      i++;
    }
    if (continuous >= 2) {
      Move tmp_move =
          compute_points_for_edges(game, first, moves[i - 1], continuous);
      if (tmp_move.value > to_rtn.value) {
        to_rtn = tmp_move;
      }
    }

    return to_rtn;
  }

  static Move compute_points_for_middle(
      FacilityGame const &game,
      std::size_t first,
      std::size_t last) {
    using FacilityStatus::FREE;
    Move to_rtn{0, 0};
    auto const &nodes = game.get_nodes();

    // if there is one free node between the two teams I can select only the
    // middle one
    if (last - first == 4 && game.get_status(first + 2) == FREE) {
      auto move = first + 2;
      to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
    }
    // if there are two FREE nodes between the two teams I can create a new
    // bigger team by selecting anyone of them, so I select the one with the
    // highest value
    else if (last - first == 5) {
      // if both FREE, select the one with the highest value
      if (game.get_status(first + 2) == FREE
          && game.get_status(first + 3) == FREE) {
        if (nodes[first + 2] > nodes[first + 3]) {
          auto move = first + 2;
          to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
        } else {
          auto move = first + 3;
          to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
        }
      }
      // else select the one who is FREE (if any)
      else if (game.get_status(first + 2) == FREE) {
        auto move = first + 2;
        to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
      } else if (game.get_status(first + 3) == FREE) {
        auto move = first + 3;
        to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
      }
    }
    // if there are three FREE nodes between the two teams, I can either
    // select the middle one and combine the two teams, or select the left
    // or right and combine the two teams on the next round
    // the left/right nodes are computed with a weight in their points
    // because of the probability that one of them might be taken by the
    // opponent
    else if (last - first == 6) {
      if (game.get_status(first + 2) == FREE
          && game.get_status(first + 4) == FREE) {
        std::size_t max_points{};
        if (nodes[first + 2] > nodes[first + 4]) {
          auto move = first + 2;
          to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
        } else {
          auto move = first + 4;
          to_rtn = {move, 3 * (nodes[first] + nodes[move] + nodes[last])};
        }

        to_rtn.value = static_cast<std::size_t>(std::floor(
            0.8
            * (2 * nodes[first] + 3 * nodes[first + 2] + 3 * nodes[first + 4]
               + 2 * nodes[last])));
      }
      if (game.get_status(first + 3) == FREE) {
        std::size_t tmp_points =
            3 * (nodes[first] + nodes[first + 3] + nodes[last]);
        if (tmp_points > to_rtn.value) {
          to_rtn = {first + 3, tmp_points};
        }
      }
    }
    return to_rtn;
  }

  static Move inc_best_triplet_by_middle(
      FacilityGame const &game,
      std::vector<std::size_t> const &moves) {
    std::size_t i = 1;
    std::size_t first{};
    std::size_t last{};
    Move move{0, 0};

    while (i < moves.size()) {
      std::size_t space = moves[i] - moves[i - 1];
      if (space >= 4 && space <= 6) {
        first = moves[i - 1];
        last = moves[i];
        auto tmp_move = compute_points_for_middle(game, first, last);
        if (tmp_move.value > move.value) {
          move = tmp_move;
        }
      }
      i++;
    }

    return move;
  }

  static Move find_best_node(FacilityGame const &game) {
    std::size_t best_idx{};
    std::size_t best_value{};

    static constexpr auto free = FacilityStatus::FREE;
    auto const &nodes = game.get_nodes();
    auto const &statuses = game.get_statuses();
    for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
      if (statuses[idx] == free && nodes[idx] > best_value) {
        best_idx = idx;
        best_value = nodes[idx];
      }
    }

    return {best_idx, best_value};
  }

  static void add_move(std::size_t move, std::vector<std::size_t> &moves) {
    auto it = std::lower_bound(moves.begin(), moves.end(), move);
    moves.insert(it, move);
  }

public:
  std::size_t next_move(FacilityGame const &game) override {
    if (std::vector<std::size_t> const &moves = game.get_moves();
        !moves.empty()) {
      std::size_t vs_move = moves.back();
      add_move(vs_move, m_vs_moves);
    }

    Move my_move{0, 0};

    // if there is a move in memory to be made, put it in my move, and
    // delete it from memory if it's played
    while (!m_followup_moves.empty()) {
      // get the first node
      Move move = m_followup_moves.front();
      // if the first node is free, make it my move
      if (game.get_status(move.index) == FacilityStatus::FREE) {
        if (move.value > my_move.value) {
          my_move = move;
          break;
        }
      } else {
        // else the first move in memory couldn't be played and if the
        // memory is full, empty the memory as the triplet is now
        // screwed
        if (m_followup_moves.size() == 3) {
          m_followup_moves.clear();
        } else {
          m_followup_moves.erase(m_followup_moves.begin());
        }
      }
    }

    // if the memory was empty, or the next move didn't get a value from
    // memory, find the next best triplet and make it my move
    if (my_move.value == 0) {
      Move move = start_best_possible_triplet(game);
      if (move.value > my_move.value) {
        my_move = move;
      }
    }

    // if I have made more than two moves, that means that I can now make
    // triplets, so search if there is a triplet to be made, and if the
    // points it gives are higher that those of the current move, change my
    // move
    if (m_my_moves.size() >= 2) {
      auto tmp_move = inc_best_triplet_by_edges(game, m_my_moves);
      if (tmp_move.value > my_move.value) {
        my_move = tmp_move;
      }

      tmp_move = inc_best_triplet_by_middle(game, m_my_moves);
      if (tmp_move.value > my_move.value) {
        my_move = tmp_move;
      }
    }

    // if no triplet was found in memory, or stored in my current move, and
    // no triplets could be made or incremented, choose the free node with
    // the highest value
    if (my_move.value == 0) {
      auto tmp_move = find_best_node(game);
      if (tmp_move.value > my_move.value) {
        my_move = tmp_move;
      }
    }

    // if the opponent has made more than two moves, that means that he can
    // now make triplets, so search if there is a triplet to be made, and if
    // the points it gives are higher that those of the current move, change
    // my move to block his move
    if (m_vs_moves.size() >= 2) {
      auto tmp_move = inc_best_triplet_by_edges(game, m_vs_moves);
      if (tmp_move.value > 0) {
        if (2.5 * tmp_move.value / 3 > my_move.value) {
          my_move = {
              tmp_move.index,
              tmp_move.value + game.get_node(tmp_move.index)};
        }
      }

      tmp_move = inc_best_triplet_by_middle(game, m_vs_moves);
      if (tmp_move.value != 0) {
        if (2.5 * tmp_move.value / 3 > my_move.value) {
          my_move = {
              tmp_move.index,
              tmp_move.value + game.get_node(tmp_move.index)};
        }
      }
    }

    // add my last move to the Vector with my moves so far
    add_move(my_move.index, m_my_moves);

    // if the current move is the one that I got from memory, remove it from
    // memory
    if (!m_followup_moves.empty()
        && my_move.index == m_followup_moves[0].index) {
      m_followup_moves.erase(m_followup_moves.begin());
    }

    // return my next move
    return my_move.index;
  }
};

#endif // NIGHTHAWK_H
