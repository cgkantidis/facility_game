#ifndef ENUM_PLAYER_TYPE_H
#define ENUM_PLAYER_TYPE_H

#include <fmt/core.h>

enum class EnumPlayerType {
  FPLAYER_SIMPLE_1,
  FPLAYER_SIMPLE_2,
  FPLAYER_SLOW,
  NIGHTHAWK_COMPLEMENT,
  NIGHTHAWK
};

EnumPlayerType str_to_player_type(std::string const &player_type);
char const *player_type_to_str(EnumPlayerType player_type);

#endif // ENUM_PLAYER_TYPE_H
