#ifndef ENUM_GAME_TYPE_H
#define ENUM_GAME_TYPE_H

enum class EnumGameType {
  NORMAL, // normal facility game
  COPY, // facility game where player B has to imitate player A
  COMPLEMENT // facility game where player B has to choose nodes of
             // complementary value to player's B moves
};

#endif // ENUM_GAME_TYPE_H
