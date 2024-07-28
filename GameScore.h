#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include <fmt/core.h>

class GameScore {
public:
  int m_playerA{};
  int m_playerB{};

  std::string toString() {
    return fmt::format(
        "Game score -- Player A: {}, Player B: {}",
        m_playerA,
        m_playerB);
  }
};

#endif // GAME_SCORE_H
