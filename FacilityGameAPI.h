#ifndef FACILITY_GAME_API_H
#define FACILITY_GAME_API_H

#include <vector>

#include "EnumFacilityStatus.h"
#include "EnumPlayer.h"
#include "GameScore.h"

class FacilityGameAPI {
public:
  FacilityGameAPI() = default;
  FacilityGameAPI(FacilityGameAPI const &) = delete;
  FacilityGameAPI(FacilityGameAPI &&) = delete;
  FacilityGameAPI &operator=(FacilityGameAPI const &) = delete;
  FacilityGameAPI &operator=(FacilityGameAPI &&) = delete;
  virtual ~FacilityGameAPI() = default;

  // Return the number of nodes in the game
  [[nodiscard]] virtual std::size_t get_num_nodes() const = 0;

  // Return the seed for the pseudorandom number generator
  [[nodiscard]] virtual std::size_t get_seed() const = 0;

  // Get the value of a specific node
  [[nodiscard]] virtual std::size_t get_value(std::size_t idx) const = 0;

  // Get an array with the values of all nodes
  [[nodiscard]] virtual std::vector<std::size_t> const &get_values() const = 0;

  // Get the status of a specific node
  [[nodiscard]] virtual EnumFacilityStatus
  get_status(std::size_t node) const = 0;

  // Get an array with the status of all nodes
  [[nodiscard]] virtual std::vector<EnumFacilityStatus> const &
  get_statuses() const = 0;

  // The number of moves so far
  [[nodiscard]] virtual std::size_t get_num_moves() const = 0;

  // Which player made each move?
  [[nodiscard]] virtual std::vector<EnumPlayer> const &
  get_move_by_player() const = 0;

  // Which location/node was chosen at each move?
  [[nodiscard]] virtual std::vector<std::size_t> const &
  get_move_locations() const = 0;

  // Get current score
  [[nodiscard]] virtual GameScore get_score() const = 0;

  // Are there any moves left in the game?
  [[nodiscard]] virtual bool is_finished() const = 0;
};

#endif // FACILITY_GAME_API_H
