#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include "enums.h"
#include <fmt/core.h>

class GameScore {
private:
  std::size_t m_playerA{};
  std::size_t m_playerB{};

public:
  void set_score(Player player, std::size_t score) {
    if (player == Player::PLAYER_A) {
      m_playerA = score;
    } else {
      m_playerB = score;
    }
  }

  [[nodiscard]] std::size_t get_score(Player player) const {
    if (player == Player::PLAYER_A) {
      return m_playerA;
    }
    return m_playerB;
  }

  [[nodiscard]] std::string to_string() const {
    return fmt::format(
        "Game score -- Player A: {}, Player B: {}",
        m_playerA,
        m_playerB);
  }
};

#endif // GAME_SCORE_H
