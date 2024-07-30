#ifndef ENUM_PLAYER_STATE_H
#define ENUM_PLAYER_STATE_H

enum class EnumPlayerState {
  UNINIT,
  STARTING,
  WAITING_FOR_ME,
  WAITING_FOR_OPPONENT,
  TERMINATING
};

char const *player_state_to_str(EnumPlayerState player_state);

#endif // ENUM_PLAYER_STATE_H
