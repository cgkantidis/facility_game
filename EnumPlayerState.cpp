#include "EnumPlayerState.h"
#include "FacilityGameException.h"

char const *player_state_to_str(EnumPlayerState player_state) {
  switch (player_state) {
  case EnumPlayerState::UNINIT: {
    return "UNINIT";
  }
  case EnumPlayerState::STARTING: {
    return "STARTING";
  }
  case EnumPlayerState::WAITING_FOR_ME: {
    return "WAITING_FOR_ME";
  }
  case EnumPlayerState::WAITING_FOR_OPPONENT: {
    return "WAITING_FOR_OPPONENT";
  }
  case EnumPlayerState::TERMINATING: {
    return "TERMINATING";
  }
  }
  throw FacilityGameException("Unknown player state");
}
