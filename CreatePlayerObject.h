#ifndef CREATE_PLAYER_OBJECT_H
#define CREATE_PLAYER_OBJECT_H

#include "EnumPlayer.h"
#include "EnumPlayerType.h"
#include "FPlayer.h"
#include "FPlayerSimple1.h"
#include "FPlayerSimple2.h"
#include "FPlayerSlow.h"
#include "FacilityGameException.h"
#include "NightHawk.h"
#include "NightHawkComplement.h"

FPlayer *create(EnumPlayer player_AB, EnumPlayerType player_type) {
  switch (player_type) {
  case EnumPlayerType::FPLAYER_SIMPLE_1:
    return new FPlayerSimple1(player_AB);
  case EnumPlayerType::FPLAYER_SIMPLE_2:
    return new FPlayerSimple2(player_AB);
  case EnumPlayerType::FPLAYER_SLOW:
    return new FPlayerSlow(player_AB);
  case EnumPlayerType::NIGHTHAWK_COMPLEMENT:
    return new NightHawkComplement(player_AB);
  case EnumPlayerType::NIGHTHAWK:
    return new NightHawk(player_AB);
  default:
    throw FacilityGameException(fmt::format(
                                    "Unexpected player type: {}",
                                    player_type_to_str(player_type))
                                    .c_str());
  }
}

#endif // CREATE_PLAYER_OBJECT_H
