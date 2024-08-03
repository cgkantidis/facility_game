#ifndef FACILITY_GAME_H
#define FACILITY_GAME_H

#include <algorithm>
#include <fmt/base.h>
#include <random>
#include <ranges>
#include <utility>

#include "FacilityGameException.h"
#include "GameScore.h"
#include "enums.h"

static constexpr std::size_t MIN_VALUE = 10;
static constexpr std::size_t MAX_VALUE = 50;
static constexpr std::size_t BONUS_MIN_GROUP_SIZE = 3;
static constexpr std::size_t BONUS_FACTOR = 3;

class FacilityGame {
private:
  std::size_t m_seed;
  std::vector<std::size_t> m_nodes;
  std::vector<FacilityStatus> m_statuses;
  GameScore m_score;
  std::vector<std::size_t> m_moves;
  // player_A plays first, player_B plays second

public:
  FacilityGame(std::size_t size, std::size_t seed)
      : m_seed(seed),
        m_nodes(size),
        m_statuses(size) {
    std::mt19937 gen(m_seed);
    std::uniform_int_distribution<std::size_t> dist(1, MAX_VALUE);
    std::generate(m_nodes.begin(), m_nodes.end(), [&gen, &dist]() {
      return dist(gen);
    });
  }

  [[nodiscard]] std::size_t get_num_nodes() const {
    return m_nodes.size();
  }

  [[nodiscard]] std::size_t get_seed() const {
    return m_seed;
  }

  [[nodiscard]] std::size_t get_node(std::size_t node_idx) const {
    return m_nodes[node_idx];
  }

  [[nodiscard]] std::vector<std::size_t> const &get_nodes() const {
    return m_nodes;
  }

  [[nodiscard]] FacilityStatus get_status(std::size_t node_idx) const {
    return m_statuses[node_idx];
  }

  [[nodiscard]] std::vector<FacilityStatus> const &get_statuses() const {
    return m_statuses;
  }

  [[nodiscard]] GameScore get_score() const {
    return m_score;
  }

  [[nodiscard]] bool is_finished() const {
    return !std::ranges::any_of(m_statuses, [](FacilityStatus status) {
      return status == FacilityStatus::FREE;
    });
  }

  [[nodiscard]] std::vector<std::size_t> const &get_moves() const {
    return m_moves;
  }

  bool append_move(Player player, std::size_t idx) {
    if (player == Player::PLAYER_A) {
      if (m_moves.size() % 2 == 1) {
        throw FacilityGameException(
            "PLAYER_B played when it was PLAYER_A's turn");
      }
    } else {
      if (m_moves.size() % 2 == 0) {
        throw FacilityGameException(
            "PLAYER_A played when it was PLAYER_B's turn");
      }
    }

    if (idx >= m_nodes.size()) {
      fmt::println(
          "{} tried to select location {} which is outside the range [0, {}]",
          player_to_str(player),
          idx,
          m_nodes.size() - 1);
      return false;
    }

    if (m_statuses[idx] != FacilityStatus::FREE) {
      fmt::println(
          "{} tried to select location {} which is not free",
          player_to_str(player),
          idx);
      return false;
    }

    m_moves.emplace_back(idx);

    // occupy the location
    if (player == Player::PLAYER_A) {
      m_statuses[idx] = FacilityStatus::PLAYER_A;
    } else {
      m_statuses[idx] = FacilityStatus::PLAYER_B;
    }

    // block neighbors
    if (m_nodes.size() > 2) {
      if (idx > 0 && m_statuses[idx - 1] == FacilityStatus::FREE) {
        m_statuses[idx - 1] = FacilityStatus::BLOCKED;
      }
      if (idx < m_nodes.size() - 1
          && m_statuses[idx + 1] == FacilityStatus::FREE) {
        m_statuses[idx + 1] = FacilityStatus::BLOCKED;
      }
    }

    update_score(player);
    return true;
  }

private:
  void update_score(Player player) {
    FacilityStatus const search_status = player == Player::PLAYER_A
                                             ? FacilityStatus::PLAYER_A
                                             : FacilityStatus::PLAYER_B;
    std::size_t tmp_score{0};
    std::size_t num_consecutive{0};
    std::size_t score{0};

    for (std::size_t idx = 0; idx < m_nodes.size(); ++idx) {
      auto const &status = m_statuses[idx];
      auto const &node = m_nodes[idx];

      if (status == search_status) {
        ++num_consecutive;
        tmp_score += node;
      } else if (status == FacilityStatus::BLOCKED) {
        continue;
      } else {
        if (num_consecutive >= BONUS_MIN_GROUP_SIZE) {
          tmp_score *= BONUS_FACTOR;
        }
        score += tmp_score;
        tmp_score = 0;
        num_consecutive = 0;
      }
    }
    if (num_consecutive >= BONUS_MIN_GROUP_SIZE) {
      tmp_score *= BONUS_FACTOR;
    }
    score += tmp_score;
    m_score.set_score(player, score);
  }

public:
  void print_score() const {
    fmt::println(
        "SCORE: PLAYER_A:{} PLAYER_B:{}",
        m_score.get_score(Player::PLAYER_A),
        m_score.get_score(Player::PLAYER_B));
  }

  void print_score_calculation() const {
    for (auto player : {Player::PLAYER_A, Player::PLAYER_B}) {
      std::string detailed;
      FacilityStatus const search_status = player == Player::PLAYER_A
                                               ? FacilityStatus::PLAYER_A
                                               : FacilityStatus::PLAYER_B;
      std::size_t tmp_score{0};
      std::size_t num_consecutive{0};
      std::size_t score{0};

      for (std::size_t idx = 0; idx < m_nodes.size(); ++idx) {
        auto const status = m_statuses[idx];
        auto const node = m_nodes[idx];

        if (status == search_status) {
          ++num_consecutive;
          tmp_score += node;

          if (num_consecutive == 1) {
            if (score > 0) {
              detailed += ' ';
            }
            detailed += '(';
          } else {
            detailed += '+';
          }
          detailed += std::to_string(node);
        } else if (status == FacilityStatus::BLOCKED) {
          continue;
        } else if (num_consecutive > 0) {
          detailed += ')';
          if (num_consecutive >= BONUS_MIN_GROUP_SIZE) {
            tmp_score *= BONUS_FACTOR;
            detailed += '*' + std::to_string(BONUS_FACTOR);
          }
          score += tmp_score;
          detailed += '=' + std::to_string(tmp_score);

          tmp_score = 0;
          num_consecutive = 0;
        }
      }
      if (num_consecutive > 0) {
        detailed += ')';
        if (num_consecutive >= BONUS_MIN_GROUP_SIZE) {
          tmp_score *= BONUS_FACTOR;
          detailed += '*' + std::to_string(BONUS_FACTOR);
        }
        score += tmp_score;
        detailed += '=' + std::to_string(tmp_score);
      }

      detailed += " === " + std::to_string(score);
      fmt::println("{}: {}", player_to_str(player), detailed);
    }
  }

  void print_num_moves() const {
    std::size_t num_moves_A{};
    std::size_t num_moves_B{};
    for (auto const &status : m_statuses) {
      if (status == FacilityStatus::PLAYER_A) {
        ++num_moves_A;
      } else if (status == FacilityStatus::PLAYER_B) {
        ++num_moves_B;
      }
    }
    fmt::println("MOVES: PLAYER_A:{} PLAYER_B:{}", num_moves_A, num_moves_B);
  }

  void print(bool verbose = false) const {
    if (verbose) {
      for (auto [idx, node, status] :
           std::views::zip(std::views::iota(0), m_nodes, m_statuses)) {
        fmt::println("{}: {} {}", idx, node, status_to_str(status));
      }
    }
    print_score();
    print_num_moves();
  }
};
#endif // FACILITY_GAME_H
