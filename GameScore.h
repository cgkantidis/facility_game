#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include <fmt/core.h>

class GameScore {
public:
  std::size_t m_playerA{};
  std::size_t m_playerB{};

  std::string toString() {
    return fmt::format(
        "Game score -- Player A: {}, Player B: {}",
        m_playerA,
        m_playerB);
  }
};

#endif // GAME_SCORE_H
