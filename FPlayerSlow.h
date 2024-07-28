#ifndef FPLAYER_SLOW_H
#define FPLAYER_SLOW_H

#include <random>
#include <ranges>
#include <thread>

#include "FPlayer.h"
#include "FacilityGameException.h"

class FPlayerSlow : public FPlayer {
  static constexpr char const *playerName = "SlowPlayer";
  static constexpr char const *version = "1.0";
  static constexpr int afm = 11111;
  static constexpr char const *firstname = "Data";
  static constexpr char const *lastname = "Structures";
  static constexpr unsigned SEED = 1234;
  static constexpr unsigned MIN_SLEEP = 21;
  static constexpr unsigned MAX_SLEEP = 23;

  std::mt19937_64 m_gen;
  std::uniform_int_distribution<uint64_t> m_dist;

public:
  explicit FPlayerSlow(EnumPlayer player)
      : FPlayer(player, playerName, version, afm, firstname, lastname),
        m_gen(SEED),
        m_dist(MIN_SLEEP, MAX_SLEEP) {}

  std::size_t next_move(FacilityGameAPI const *game) override {
    // make the player slow and check what happens
    std::this_thread::sleep_for(std::chrono::seconds(m_dist(m_gen)));

    auto it = std::ranges::find(
        game->get_status_vec() | std::views::reverse,
        EnumFacilityStatus::FREE);
    if (it == game->get_status_vec().rend()) {
      throw FacilityGameException("No available move");
    }

    auto const distance = static_cast<std::size_t>(
        std::distance(game->get_status_vec().rbegin(), it));
    return game->get_num_nodes() - distance - 1;
  }
};

#endif // FPLAYER_SLOW_H
