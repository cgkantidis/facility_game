#ifndef FPLAYER_H
#define FPLAYER_H

#include <fmt/core.h>
#include <string>

#include "enums.h"
#include "FacilityGame.h"

class FPlayer {
protected:
  const Player m_player;
  const std::string m_player_name;
  const std::string m_version;
  const std::string m_firstname;
  const std::string m_lastname;

public:
  FPlayer(
      Player player,
      char const *player_name,
      char const *version,
      char const *firstname,
      char const *lastname)
      : m_player(player),
        m_player_name(player_name),
        m_version(version),
        m_firstname(firstname),
        m_lastname(lastname) {}
  FPlayer(FPlayer const &) = delete;
  FPlayer(FPlayer &&) = delete;
  FPlayer &operator=(FPlayer const &) = delete;
  FPlayer &operator=(FPlayer &&) = delete;
  virtual ~FPlayer() = default;

  virtual void initialize([[maybe_unused]] FacilityGame const &game) = 0;
  virtual std::size_t next_move(FacilityGame const &game) = 0;
};

#endif // FPLAYER_H
