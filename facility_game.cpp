#include "CreatePlayerObject.h"
#include "EnumClientServer.h"
#include "EnumFacilityStatus.h"
#include "EnumGameType.h"
#include "EnumPlayer.h"
#include "EnumPlayerState.h"
#include "EnumPlayerType.h"
#include "FClientArgs.h"
#include "FPlayer.h"
#include "FPlayerSimple1.h"
#include "FPlayerSimple2.h"
#include "FPlayerSlow.h"
#include "FacilityGameAPI.h"
#include "FacilityGameException.h"
#include "Finals.h"
#include "GameScore.h"
#include "MonitorThread.h"
#include "NightHawkComplement.h"
#include "Node.h"

constexpr EnumPlayerType DEFAULT_PLAYER_TYPE = EnumPlayerType::FPLAYER_SIMPLE_1;
constexpr bool DEFAULT_VERBOSE = false;

int main(int argc, char const **argv) {
  EnumPlayerType const player_type_A = (argc > 1) ? str_to_player_type(argv[1]) : DEFAULT_PLAYER_TYPE;
  EnumPlayerType const player_type_B = (argc > 2) ? str_to_player_type(argv[2]) : DEFAULT_PLAYER_TYPE;
  bool const verbose = (argc > 3) ? std::atoi(argv[3]) != 0 : DEFAULT_VERBOSE;

  FPlayer *player_A = create(EnumPlayer::PLAYER_A, player_type_A);
  FPlayer *player_B = create(EnumPlayer::PLAYER_B, player_type_B);

  fmt::println(
      "Starting game of {} VS {}",
      player_type_to_str(player_type_A),
      player_type_to_str(player_type_B));

  return 0;
}
//  // Play the game
//  server.play(serverPlayer);
//
//  // Print the game info
//  server.printGameInfo();
//
//  // Print the game status
//  server.printGameStatus();
//
//  // print roles
//  server.printRoles();
//} catch (FacilityGameException e) {
//System.err.println(e);
//
//}
//}
//
//public class FServer {
//  int port; // TCP port for accepting the socket connection
//  FPlayer serverPlayer; // The player object for FServer
//  EnumPlayerType playerType;
//
//  // Determine if server is player A or player B
//  boolean serverIsPlayerA;
//
//  // Player A or B
//  EnumPlayer serverRole;
//  EnumPlayer clientRole;
//
//  FacilityGame game;
//  ObjectInputStream in;
//  ObjectOutputStream out;
//
//  boolean verbose; // print information during the execution
//
//  // A thread to monitor the player state
//  MonitorThread monitorThread;
//
//  // Server Socket for listen for accepting client connection
//  ServerSocket ss;
//
//  // Socket for the connection with the client
//  Socket s;
//
//  public FServer(boolean parVerbose, int parPort, EnumPlayerType playerType) {
//    verbose = parVerbose;
//    port = parPort;
//    this.playerType = playerType;
//    monitorThread = new MonitorThread();
//  }
//
//  public void printGameStatus() {
//    if (verbose) {
//      game.printStatusLong();
//      game.printAllMoves();
//    } else {
//      game.printStatus();
//    }
//  }
//
//  public void printGameInfo() {
//    game.printGameInfo();
//  }
//
//
//  public void play(FPlayer parServerPlayer) {
//    try {
//      serverPlayer = parServerPlayer;
//
//      // Initialize the player
//      serverPlayer.initialize(game);
//
//      out.writeUTF(serverPlayer.about());
//      out.flush();
//
//      monitorThread.setState(game.getCurMoveIndex(),
//          EnumPlayerState.WAITING_FOR_OPPONENT);
//      monitorThread.start();
//
//      String aboutOpponent = in.readUTF();
//
//      if (serverIsPlayerA) {
//        game.setAboutPlayerA(serverPlayer.about());
//        game.setAboutPlayerB(aboutOpponent);
//      } else {
//        game.setAboutPlayerA(aboutOpponent);
//        game.setAboutPlayerB(serverPlayer.about());
//      }
//
//      int afm = in.readInt();
//      String firstname = in.readUTF();
//      String lastname = in.readUTF();
//
//      out.writeBoolean(true);
//      out.flush();
//
//      System.out.println("FServer (" + serverRole
//          + "): OK, the game is starting ...");
//
//      // Play the game
//      int countMoveServer = 0;
//      int countMoveClient = 0;
//
//      if (serverIsPlayerA) {
//        while (!game.isFinished()) {
//          // playerA
//          int moveOfServer = serverPlayer.nextMove(game);
//
//          monitorThread.setState(game.getCurMoveIndex(),
//              EnumPlayerState.WAITING_FOR_ME);
//          game.processMove(moveOfServer, serverRole);
//          if (verbose) {
//            System.out.println("FServer: move:" + countMoveServer
//                + ", location of Server:" + moveOfServer
//                + ", sending move");
//          }
//          countMoveServer++;
//          out.writeInt(moveOfServer);
//          out.flush();
//
//          if (game.isFinished()) {
//            continue;
//          }
//
//          if (verbose) {
//            System.out.println("FServer: Receiving move of Client");
//          }
//          monitorThread.setState(game.getCurMoveIndex(),
//              EnumPlayerState.WAITING_FOR_OPPONENT);
//          int moveOfClient = in.readInt();
//
//          game.processMove(moveOfClient, clientRole);
//          if (verbose) {
//            System.out.println("FServer: move of Client:"
//                + countMoveClient + ", location of Client:"
//                + moveOfClient);
//          }
//          countMoveClient++;
//        }
//      } else {
//        // Server is Player B
//        while (!game.isFinished()) {
//
//          if (verbose) {
//            System.out.println("FServer: Receiving move of Client");
//          }
//          monitorThread.setState(game.getCurMoveIndex(),
//              EnumPlayerState.WAITING_FOR_OPPONENT);
//          int moveOfClient = in.readInt();
//          game.processMove(moveOfClient, clientRole);
//          if (verbose) {
//            System.out.println("FServer: move of Client:"
//                + countMoveClient + ", location of Client:"
//                + moveOfClient);
//          }
//          countMoveClient++;
//
//          if (game.isFinished()) {
//            continue;
//          }
//
//          // playerA
//          int moveOfServer = serverPlayer.nextMove(game);
//
//          monitorThread.setState(game.getCurMoveIndex(),
//              EnumPlayerState.WAITING_FOR_ME);
//          game.processMove(moveOfServer, serverRole);
//          if (verbose) {
//            System.out.println("FServer: move:" + countMoveServer
//                + ", location of Server:" + moveOfServer
//                + ", sending move");
//          }
//          countMoveServer++;
//          out.writeInt(moveOfServer);
//          out.flush();
//
//        }
//      }
//
//      monitorThread.setState(game.getCurMoveIndex(),
//          EnumPlayerState.TERMINATING);
//
//      System.out.println("FServer (" + serverRole
//          + "): The game finished !!");
//      System.out.println("FServer (" + serverRole + "): "
//          + game.getScore());
//
//      String proof = UUID.randomUUID().toString();
//      out.writeUTF(proof);
//      out.flush();
//      // }
//
//      in.close();
//      out.close();
//
//    } catch (IOException e) {
//      System.err.println(e);
//      e.printStackTrace();
//    } catch (Exception e) {
//      e.printStackTrace();
//    }
//  }
//
//}


