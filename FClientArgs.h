#ifndef FCLIENT_ARGS_H
#define FCLIENT_ARGS_H

#include <string>

#include "EnumGameType.h"
#include "Finals.h"

class FClientArgs {
public:
  EnumPlayerType playerType;
  bool verbose;
  // "mycomputer.ee.duth.gr" or "127.0.0.1" or "localhost"
  std::string serverHost;
  int serverPort;
  bool clientIsPlayerB;

  // Game parameters
  int n;
  long seed;
  EnumGameType gameType = DEFAULT_GAME_TYPE;
};

#endif // FCLIENT_ARGS_H
