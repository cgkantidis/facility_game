#include <algorithm>
#include <random>
#include <ranges>
#include <utility>

#include "EnumClientServer.h"
#include "EnumFacilityStatus.h"
#include "EnumGameType.h"
#include "FacilityGameAPI.h"
#include "FacilityGameException.h"

static constexpr std::size_t MIN_VALUE = 10;
static constexpr std::size_t MAX_VALUE = 50;
static constexpr std::size_t COMPLEMENT = 9999;

class FacilityGame : FacilityGameAPI {
private:
  std::size_t m_seed;
  EnumGameType m_game_type;
  std::vector<std::size_t> m_nodes;
  std::vector<EnumFacilityStatus> m_statuses;
  GameScore m_score;
  std::size_t m_cur_move_idx;
  std::vector<EnumPlayer> m_move_by_player;
  std::vector<std::size_t> m_move_locations;
  std::string m_about_player_A;
  std::string m_about_player_B;

  EnumClientServer m_what_am_i;

public:
  FacilityGame(
      EnumClientServer what_am_i,
      std::size_t size,
      std::size_t seed,
      EnumGameType game_type)
      : m_seed(seed),
        m_game_type(game_type),
        m_nodes(size),
        m_statuses(size),
        m_score{0, 0},
        m_cur_move_idx{0},
        m_what_am_i(what_am_i) {

    std::mt19937 gen(m_seed);
    switch (m_game_type) {
    case EnumGameType::COPY: {
      if (m_nodes.size() < 8) {
        static constexpr char const *message =
            "FacilityGame COPY mode requires at least n=8 nodes (currently: "
            "n={})";
        fmt::println(message, m_nodes.size());
        throw FacilityGameException(
            fmt::format(message, m_nodes.size()).c_str());
      }

      // generate the node values
      auto begin_it = m_nodes.begin();
      auto end_it = std::next(m_nodes.begin(), m_nodes.size() / 2);
      std::iota(begin_it, end_it, MIN_VALUE);

      // shuffle the node values
      std::shuffle(begin_it, end_it, gen);

      // copy the node values to the other half
      if (m_nodes.size() % 2 != 0) {
        *end_it = MIN_VALUE;
        std::copy(m_nodes.begin(), end_it, std::next(end_it));
      } else {
        std::copy(m_nodes.begin(), end_it, end_it);
      }

      // block the middle node
      if (m_nodes.size() % 2 != 0) {
        m_statuses[m_nodes.size() / 2 + 1] = EnumFacilityStatus::BLOCKED;
      }
      return;
    }
    case EnumGameType::COMPLEMENT: {
      if (m_nodes.size() < 8) {
        static constexpr char const *message =
            "FacilityGame COMPLEMENT mode requires at least n=8 nodes "
            "(currently: n={})";
        fmt::println(message, m_nodes.size());
        throw FacilityGameException(
            fmt::format(message, m_nodes.size()).c_str());
      }

      // generate odd values for the first half (why?)
      std::ranges::copy(
          std::views::iota(0) | std::views::transform([](std::size_t i) {
            return MIN_VALUE + 1 + 2 * i;
          }) | std::views::take(m_nodes.size() / 2),
          m_nodes.begin());

      auto begin_it = m_nodes.begin();
      auto end_it = std::next(m_nodes.begin(), m_nodes.size() / 2);
      // shuffle the node values
      std::shuffle(begin_it, end_it, gen);

      // copy the node values to the other half
      if (m_nodes.size() % 2 != 0) {
        *end_it = MIN_VALUE;
        std::transform(
            m_nodes.begin(),
            end_it,
            std::next(end_it),
            [](std::size_t val) { return COMPLEMENT - val; });
      } else {
        std::transform(m_nodes.begin(), end_it, end_it, [](std::size_t val) {
          return COMPLEMENT - val;
        });
      }

      // block the middle node
      if (m_nodes.size() % 2 != 0) {
        m_statuses[m_nodes.size() / 2 + 1] = EnumFacilityStatus::BLOCKED;
      }
      return;
    }
    case EnumGameType::NORMAL: {
      std::uniform_int_distribution<std::size_t> dist(1, MAX_VALUE);
      std::generate(m_nodes.begin(), m_nodes.end(), [&gen, &dist]() {
        return dist(gen);
      });
      return;
    }
    }
    std::unreachable();
  }

  [[nodiscard]] std::size_t get_num_nodes() const override {
    return m_nodes.size();
  }

  [[nodiscard]] std::string const &get_about_player_A() const {
    return m_about_player_A;
  }

  void set_about_player_A(std::string about_player_A) {
    m_about_player_A = std::move(about_player_A);
  }

  [[nodiscard]] std::string const &get_about_player_B() const {
    return m_about_player_B;
  }

  void set_about_player_B(std::string about_player_B) {
    m_about_player_B = std::move(about_player_B);
  }

  [[nodiscard]] std::size_t get_seed() const override {
    return m_seed;
  }

  [[nodiscard]] std::size_t get_value(std::size_t node_idx) const override {
    return m_nodes[node_idx];
  }

  [[nodiscard]] std::vector<std::size_t> const &get_values() const override {
    return m_nodes;
  }

  [[nodiscard]] EnumFacilityStatus
  get_status(std::size_t node_idx) const override {
    return m_statuses[node_idx];
  }

  [[nodiscard]] std::vector<EnumFacilityStatus> const &
  get_statuses() const override {
    return m_statuses;
  }

  [[nodiscard]] GameScore get_score() const override {
    return m_score;
  }

  [[nodiscard]] bool is_finished() const override {
    return !std::ranges::any_of(m_statuses, [](EnumFacilityStatus status) {
      return status == EnumFacilityStatus::FREE;
    });
  }

  [[nodiscard]] std::size_t get_num_moves() const override {
    return m_cur_move_idx;
  }

  [[nodiscard]] std::vector<EnumPlayer> const &
  get_move_by_player() const override {
    return m_move_by_player;
  }

  [[nodiscard]] std::vector<std::size_t> const &
  get_move_locations() const override {
    return m_move_locations;
  }

  void processMove(int location, EnumPlayer player) {

    // int size = moveByPlayer.size();
    // if (curMoveIndex > size) {
    // moveByPlayer.setSize(size + n);
    // moveLocation.setSize(size + n);
    // }
    // moveByPlayer.set(curMoveIndex, player);
    // moveLocation.set(curMoveIndex, location);
    m_move_by_player.add(player);
    m_move_locations.add(location);
    m_cur_move_idx++;

    if (location >= 0) {
      occupyLocation(location, player);
    }

    if (gameType == EnumGameType.COPY) {
      checkMostRecentPairOfMoves();
    }

    if (gameType == EnumGameType.COMPLEMENT) {
      checkMostRecentPairOfMovesComplement();
    }
  }

  void occupyLocation(int location, EnumPlayer player) {
    if (status[location] != EnumFacilityStatus.FREE) {
      System.err.println(
          "Location: " + location + " is not free (status = " + status[location]
          + ")");
      // ignore the request
    } else {
      // occupy the location
      switch (player) {
      case PLAYER_A:
        status[location] = EnumFacilityStatus.PLAYER_A;
        break;
      case PLAYER_B:
        status[location] = EnumFacilityStatus.PLAYER_B;
        break;
      }

      // block the neighboring locations
      if (location > 0) {
        status[location - 1] = EnumFacilityStatus.BLOCKED;
      }
      if (location < n - 1) {
        status[location + 1] = EnumFacilityStatus.BLOCKED;
      }

      // update score
      switch (player) {
      case PLAYER_A:
        // score.playerA += value[location];
        m_score.playerA = calculateScore(player);
        break;
      case PLAYER_B:
        // score.playerB += value[location];
        m_score.playerB = calculateScore(player);
        break;
      default:
        // This case should never be reached
        System.err.println("Invalid player: " + player);
      }
    }
  }

private
  int calculateScore(EnumPlayer player) {
    int score = 0;

    EnumFacilityStatus nodeStatus = null;
    if (player == EnumPlayer.PLAYER_A) {
      nodeStatus = EnumFacilityStatus.PLAYER_A;
    } else if (player == EnumPlayer.PLAYER_B) {
      nodeStatus = EnumFacilityStatus.PLAYER_B;
    }

    int tempBonus = 0;
    int numOfConsecutiveFacilities = 0;

    int node = 0;
    while (node < n) {
      if (m_statuses[node] == nodeStatus) {
        int nodeValue = m_nodes[node];
        score += nodeValue;

        // bonus
        tempBonus += (Finals.BONUS_FACTOR - 1) * nodeValue;
        numOfConsecutiveFacilities++;
        node += 1;
      } else if (status[node] == EnumFacilityStatus.BLOCKED) {
        node += 1;
      } else {
        if (numOfConsecutiveFacilities >= Finals.BONUS_MIN_GROUP_SIZE) {
          score += tempBonus;
        }
        tempBonus = 0;
        numOfConsecutiveFacilities = 0;
        node += 1;
      }
    }
    if (numOfConsecutiveFacilities >= Finals.BONUS_MIN_GROUP_SIZE) {
      score += tempBonus;
    }

    return score;
  }

  void printGameInfo() {
    System.out.println("Facility Game");
    System.out.println("n: " + n);
    System.out.println("seed: " + seed);
    System.out.println("GameType: " + gameType);
    System.out.println("PlayerA: " + aboutPlayerA);
    System.out.println("PlayerB: = " + aboutPlayerB);
  }

  void printGameGraph() {
    for (int i = 0; i < n; i++) {
      System.out.println("Location " + i + ", value:" + value[i]);
    }
  }

  void printStatus() {
    System.out.println("FacilityGame status");
    System.out.println("Finished: " + isFinished());
    System.out.println("Score: " + getScore());
  }

  void printStatusLong() {
    System.out.println("FacilityGame status");
    System.out.println("Finished: " + isFinished());
    System.out.println("Score: " + getScore());
    for (int i = 0; i < n; i++) {
      System.out.println(
          "Location " + i + ", value:" + value[i] + ", status:" + status[i]);
    }
  }

  void printAllMoves() {
    for (int i = 0; i < m_cur_move_idx; i++) {
      int location = m_move_locations.get(i);
      System.out.println(
          "move : " + i + ", player: " + moveByPlayer.get(i)
          + ", location: " + location + ", value: " + value[location]);
    }
  }

  static void checkParameters(
      boolean isServer,
      EnumPlayerType playerType,
      int n,
      EnumGameType gameType,
      EnumPlayer clientRole) throws FacilityGameException {
    if (isServer && gameType == EnumGameType.COPY
        && playerType != EnumPlayerType.FPLAYER_COPY) {
      // System.err
      // .println("ERROR: FacilityGame Server is not using FPLAYER_COPY
      // and therefore not expecting game type COPY");
      throw new FacilityGameException(
          "ERROR: FacilityGame Server is not using FPLAYER_COPY and therefore "
          "not expecting game type COPY");
    }
    if (isServer && gameType != EnumGameType.COPY
        && playerType == EnumPlayerType.FPLAYER_COPY) {
      // System.err
      // .println("ERROR: FacilityGame Server is using FPLAYER_COPY and
      // therefore expecting game type COPY");
      throw new FacilityGameException(
          "ERROR: FacilityGame Server is using FPLAYER_COPY and therefore "
          "expecting game type COPY");
    }
    if (gameType == EnumGameType.COPY) {
      if (n < 8) {
        // System.err
        // .println("ERROR: FacilityGame COPY mode requires at least n=8
        // nodes (currently: n="
        // + n + ")");
        throw new FacilityGameException(
            "ERROR: FacilityGame COPY mode requires at least n=8 nodes "
            "(currently: n="
            + n + ")");
      }
      if (clientRole != EnumPlayer.PLAYER_B) {
        // System.err
        // .println("ERROR: FacilityGame COPY mode requires the client
        // to be player B (currently: client is "
        // + clientRole);
        throw new FacilityGameException(
            "ERROR: FacilityGame COPY mode requires the client to be player B "
            "(currently: client is "
            + clientRole);
      }
    }

    // Checks for COMPLEMENT mode
    if (isServer && gameType == EnumGameType.COMPLEMENT
        && playerType != EnumPlayerType.FPLAYER_COMPLEMENT) {
      // System.err
      // .println("ERROR: FacilityGame Server is not using FPLAYER_COPY
      // and therefore not expecting game type COPY");
      throw new FacilityGameException(
          "ERROR: FacilityGame Server is not using FPLAYER_COMPLEMENT and "
          "therefore not expecting game type COMPLEMENT");
    }
    if (isServer && gameType != EnumGameType.COMPLEMENT
        && playerType == EnumPlayerType.FPLAYER_COMPLEMENT) {
      // System.err
      // .println("ERROR: FacilityGame Server is using FPLAYER_COPY and
      // therefore expecting game type COPY");
      throw new FacilityGameException(
          "ERROR: FacilityGame Server is using FPLAYER_COMPLEMENT and "
          "therefore expecting game type COMPLEMENT");
    }
    if (gameType == EnumGameType.COMPLEMENT) {
      if (n < 8) {
        // System.err
        // .println("ERROR: FacilityGame COPY mode requires at least n=8
        // nodes (currently: n="
        // + n + ")");
        throw new FacilityGameException(
            "ERROR: FacilityGame COMPLEMENT mode requires at least n=8 nodes "
            "(currently: n="
            + n + ")");
      }
      if (clientRole != EnumPlayer.PLAYER_B) {
        // System.err
        // .println("ERROR: FacilityGame COPY mode requires the client
        // to be player B (currently: client is "
        // + clientRole);
        throw new FacilityGameException(
            "ERROR: FacilityGame COMPLEMENT mode requires the client to be "
            "player B (currently: client is "
            + clientRole);
      }
    }
  }

  boolean checkPairOfMoves(int move1, int move2) {
    int locationA = moveLocation.get(move1);
    int locationB = moveLocation.get(move2);
    boolean notTheSame = false;
    if ((locationA < 0 && locationB > 0) || (locationA > 0 && locationB < 0)) {
      notTheSame = true;
    } else if (locationA < 0 && locationB < 0) {
      // notTheSame = false;
    } else {
      int valueA = getValue(locationA);
      int valueB = getValue(locationB);

      if (valueA != valueB) {
        notTheSame = true;
      }
    }
    // if (notTheSame) {
    // // print error message only at the client side
    // if (whatAmI == EnumClientServer.CLIENT) {
    // System.err.println("WARNING: In COPY moves " + move1 + " and "
    // + move2 + " have not the same value");
    // }
    // }

    return (!notTheSame);
  }

  boolean checkMostRecentPairOfMoves() {
    boolean checkMostRecentPairOfMoves = true;
    int curIndex = getCurMoveIndex();
    int lastMove = curIndex - 1;
    boolean first = ((curIndex % 2) == 1); // True for moves 0, 2, 4, 6,
    if (first) {
      // there is no pair of moves yet
      checkMostRecentPairOfMoves = true;
    } else {
      int indexToCheck = lastMove - (lastMove % 2);
      int move1 = indexToCheck;
      int move2 = indexToCheck + 1;
      if (move2 > lastMove) {
        // the last pair is not complete yet, take the previous pair
        move1 -= 2;
        move2 -= 2;
      }
      if (move1 < 0) {
        checkMostRecentPairOfMoves = true;
      } else {
        checkMostRecentPairOfMoves = checkPairOfMoves(move1, move2);
      }

      if (!checkMostRecentPairOfMoves) {
        // print error message only at the client side
        if (whatAmI == EnumClientServer.CLIENT) {
          System.err.println(
              "WARNING: In COPY moves " + move1 + " and " + move2
              + " have not the same value");
        }
      }
    }

    return checkMostRecentPairOfMoves;
  }

  boolean checkPairOfMovesComplement(int move1, int move2) {
    int locationA = moveLocation.get(move1);
    int locationB = moveLocation.get(move2);
    boolean notcomplementary = false;
    if ((locationA < 0 && locationB > 0) || (locationA > 0 && locationB < 0)) {
      notcomplementary = true;
    } else if (locationA < 0 && locationB < 0) {
      // notTheSame = false;
    } else {
      int valueA = getValue(locationA);
      int valueB = getValue(locationB);

      if ((valueA + valueB) != 9999) {
        notcomplementary = true;
      }
    }
    // if (notTheSame) {
    // // print error message only at the client side
    // if (whatAmI == EnumClientServer.CLIENT) {
    // System.err.println("WARNING: In COPY moves " + move1 + " and "
    // + move2 + " have not the same value");
    // }
    // }

    return (!notcomplementary);
  }

  boolean checkMostRecentPairOfMovesComplement() {
    boolean checkMostRecentPairOfMoves = true;
    int curIndex = getCurMoveIndex();
    int lastMove = curIndex - 1;
    boolean first = ((curIndex % 2) == 1); // True for moves 0, 2, 4, 6,
    if (first) {
      // there is no pair of moves yet
      checkMostRecentPairOfMoves = true;
    } else {
      int indexToCheck = lastMove - (lastMove % 2);
      int move1 = indexToCheck;
      int move2 = indexToCheck + 1;
      if (move2 > lastMove) {
        // the last pair is not complete yet, take the previous pair
        move1 -= 2;
        move2 -= 2;
      }
      if (move1 < 0) {
        checkMostRecentPairOfMoves = true;
      } else {
        checkMostRecentPairOfMoves = checkPairOfMovesComplement(move1, move2);
      }

      if (!checkMostRecentPairOfMoves) {
        // print error message only at the client side
        if (whatAmI == EnumClientServer.CLIENT) {
          System.err.println(
              "WARNING: In COMPLEMENT mode: moves " + move1 + " and " + move2
              + " are not complementary to each other");
        }
      }
    }

    return checkMostRecentPairOfMoves;
  }

  boolean checkMoves(int fromMove) {
    boolean check = true;

    if (gameType == EnumGameType.COPY)
    // Check that player B is following player A
    {
      int moveToCheck = fromMove;
      Vector<Integer> moveLocation = getMoveLocation();
      int size = moveLocation.size();
      while (moveToCheck + 2 <= size) {
        int locationA = moveLocation.get(moveToCheck);
        int locationB = moveLocation.get(moveToCheck + 1);
        boolean notTheSame = false;
        if ((locationA < 0 && locationB > 0)
            || (locationA > 0 && locationB < 0)) {
          notTheSame = true;
        } else if (locationA < 0 && locationB < 0) {
          // notTheSame = false;
        } else {
          int valueA = getValue(locationA);
          int valueB = getValue(locationB);

          if (valueA != valueB) {
            notTheSame = true;
          }
        }
        if (notTheSame) {
          check = false;
          // print error message only at the client side
          if (whatAmI == EnumClientServer.CLIENT) {
            System.err.println(
                "WARNING: In COPY moves " + moveToCheck + " and "
                + (moveToCheck + 1) + " have not the same value");
          }
        }
        moveToCheck += 2;
      }
    }

    return check;
  }

  boolean checkMovesComplement(int fromMove) {
    boolean check = true;

    if (gameType == EnumGameType.COMPLEMENT)
    // Check that player B is following player A
    {
      int moveToCheck = fromMove;
      Vector<Integer> moveLocation = getMoveLocation();
      int size = moveLocation.size();
      while (moveToCheck + 2 <= size) {
        int locationA = moveLocation.get(moveToCheck);
        int locationB = moveLocation.get(moveToCheck + 1);
        boolean notComplementary = false;
        if ((locationA < 0 && locationB > 0)
            || (locationA > 0 && locationB < 0)) {
          notComplementary = true;
        } else if (locationA < 0 && locationB < 0) {
          // notTheSame = false;
        } else {
          int valueA = getValue(locationA);
          int valueB = getValue(locationB);

          if ((valueA + valueB) != 9999) {
            notComplementary = true;
          }
        }
        if (notComplementary) {
          check = false;
          // print error message only at the client side
          if (whatAmI == EnumClientServer.CLIENT) {
            System.err.println(
                "WARNING: In COMPLEMENT mode: moves " + moveToCheck + " and "
                + (moveToCheck + 1) + " are not complementary to each other");
          }
        }
        moveToCheck += 2;
      }
    }

    return check;
  }

  // check on the server if the client player succeeded
  // 0: faileure
  // 1: OK
  int checkSuccess(boolean serverIsPlayerA) {
    boolean checkSuccess = false;

    if (gameType == EnumGameType.NORMAL) {
      int serverScore;
      int clientScore;
      if (serverIsPlayerA) {
        serverScore = getScore().playerA;
        clientScore = getScore().playerB;
      } else {
        serverScore = getScore().playerB;
        clientScore = getScore().playerA;
      }
      checkSuccess = (clientScore >= serverScore);
    } else if (gameType == EnumGameType.COPY) {
      if (serverIsPlayerA) {
        checkSuccess = checkMoves(0);
      } else {
        // In COPY games, server must be player A
        checkSuccess = false;
      }
    } else if (gameType == EnumGameType.COMPLEMENT) {
      if (serverIsPlayerA) {
        checkSuccess = checkMovesComplement(0);
      } else {
        // In COMPLEMENT games, server must be player A
        checkSuccess = false;
      }
    }

    return (checkSuccess ? 1 : 0);
  }
};
