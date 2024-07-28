#ifndef FPLAYER_H
#define FPLAYER_H

#include <fmt/core.h>
#include <string>

#include "EnumFacilityStatus.h"
#include "EnumPlayer.h"
#include "FacilityGameAPI.h"

class FPlayer {
  const EnumPlayer m_player;
  const std::string m_player_name;
  const std::string m_version;
  const int m_AFM;
  const std::string m_firstname;
  const std::string m_lastname;
  const std::string m_about;
  const EnumFacilityStatus m_me;
  const EnumFacilityStatus m_opponent;

public:
  FPlayer(
      EnumPlayer player,
      char const *player_name,
      char const *version,
      int AFM,
      char const *firstname,
      char const *lastname)
      : m_player(player),
        m_player_name(player_name),
        m_version(version),
        m_AFM(AFM),
        m_firstname(firstname),
        m_lastname(lastname),
        m_about(fmt::format(
            "{} v{}, by {} {} (AM: {})",
            m_player_name,
            m_version,
            m_firstname,
            m_lastname,
            m_AFM)),
        m_me(
            m_player == EnumPlayer::PLAYER_A ? EnumFacilityStatus::PLAYER_A
                                             : EnumFacilityStatus::PLAYER_B),
        m_opponent(
            m_player == EnumPlayer::PLAYER_A ? EnumFacilityStatus::PLAYER_B
                                             : EnumFacilityStatus::PLAYER_A) {}
  FPlayer(FPlayer const &) = delete;
  FPlayer(FPlayer &&) = delete;
  FPlayer &operator=(FPlayer const &) = delete;
  FPlayer &operator=(FPlayer &&) = delete;
  virtual ~FPlayer() = default;

  [[nodiscard]] EnumPlayer whoAmI() const {
    return m_player;
  }

  [[nodiscard]] EnumPlayer whoIsMyOpponent() const {
    return m_player == EnumPlayer::PLAYER_A ? EnumPlayer::PLAYER_B
                                            : EnumPlayer::PLAYER_A;
  }

  std::string const &about() {
    return m_about;
  }

  virtual void initialize([[maybe_unused]] FacilityGameAPI const *game) {}
  virtual std::size_t next_move(FacilityGameAPI const *game) = 0;
};

#endif // FPLAYER_H
