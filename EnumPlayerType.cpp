#include "EnumPlayerType.h"
#include "FacilityGameException.h"

EnumPlayerType str_to_player_type(std::string const &player_type) {
  if (player_type == "FPLAYER_SIMPLE_1") {
    return EnumPlayerType::FPLAYER_SIMPLE_1;
  }
  if (player_type == "FPLAYER_SIMPLE_2") {
    return EnumPlayerType::FPLAYER_SIMPLE_2;
  }
  if (player_type == "FPLAYER_SLOW;") {
    return EnumPlayerType::FPLAYER_SLOW;
  }
  if (player_type == "NIGHTHAWK_COMPLEMENT") {
    return EnumPlayerType::NIGHTHAWK_COMPLEMENT;
  }
  if (player_type == "NIGHTHAWK") {
    return EnumPlayerType::NIGHTHAWK;
  }
  throw FacilityGameException(
      fmt::format("Unknown player type {}", player_type).c_str());
}

char const *player_type_to_str(EnumPlayerType player_type) {
  switch (player_type) {
  case EnumPlayerType::FPLAYER_SIMPLE_1: {
    return "FPLAYER_SIMPLE_1";
  }
  case EnumPlayerType::FPLAYER_SIMPLE_2: {
    return "FPLAYER_SIMPLE_2";
  }
  case EnumPlayerType::FPLAYER_SLOW: {
    return "FPLAYER_SLOW";
  }
  case EnumPlayerType::NIGHTHAWK_COMPLEMENT: {
    return "NIGHTHAWK_COMPLEMENT";
  }
  case EnumPlayerType::NIGHTHAWK: {
    return "NIGHTHAWK";
  }
  }
  throw FacilityGameException("Unknown player type");
}
