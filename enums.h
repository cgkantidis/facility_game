#ifndef ENUMS_H
#define ENUMS_H

#include <utility>

enum class Player { PLAYER_A, PLAYER_B };
enum class FacilityStatus { FREE, BLOCKED, PLAYER_A, PLAYER_B };

enum class PlayerState {
  UNINIT,
  STARTING,
  WAITING_FOR_ME,
  WAITING_FOR_OPPONENT,
  TERMINATING
};

static constexpr char const *player_to_str(Player player) {
  switch (player) {
  case Player::PLAYER_A: {
    return "PLAYER_A";
  }
  case Player::PLAYER_B: {
    return "PLAYER_B";
  }
  }
  std::unreachable();
}

static constexpr char const *status_to_str_short(FacilityStatus status) {
  switch (status) {
  case FacilityStatus::FREE: {
    return " ";
  }
  case FacilityStatus::BLOCKED: {
    return " ";
  }
  case FacilityStatus::PLAYER_A: {
    return "A";
  }
  case FacilityStatus::PLAYER_B: {
    return "B";
  }
  }
  std::unreachable();
}

static constexpr char const *status_to_str(FacilityStatus status) {
  switch (status) {
  case FacilityStatus::FREE: {
    return "FREE";
  }
  case FacilityStatus::BLOCKED: {
    return "BLOCKED";
  }
  case FacilityStatus::PLAYER_A: {
    return "PLAYER_A";
  }
  case FacilityStatus::PLAYER_B: {
    return "PLAYER_B";
  }
  }
  std::unreachable();
}

static constexpr char const *player_state_to_str(PlayerState player_state) {
  switch (player_state) {
  case PlayerState::UNINIT: {
    return "UNINIT";
  }
  case PlayerState::STARTING: {
    return "STARTING";
  }
  case PlayerState::WAITING_FOR_ME: {
    return "WAITING_FOR_ME";
  }
  case PlayerState::WAITING_FOR_OPPONENT: {
    return "WAITING_FOR_OPPONENT";
  }
  case PlayerState::TERMINATING: {
    return "TERMINATING";
  }
  }
  std::unreachable();
}

#endif // ENUMS_H
