#ifndef MONITOR_THREAD_H
#define MONITOR_THREAD_H

#include <chrono>
#include <fmt/core.h>
#include <mutex>
#include <thread>

#include "enums.h"

constexpr auto INFO_MESSAGE_DUR = std::chrono::milliseconds(8000);
constexpr auto WARN_MESSAGE_DUR = std::chrono::milliseconds(10000);
constexpr auto CHECK_DUR = std::chrono::milliseconds(500);
constexpr auto WAIT_DUR = std::chrono::milliseconds(10000);

class MonitorThread {
private:
  using clock_t = std::chrono::steady_clock;
  using time_point_t = std::chrono::time_point<clock_t>;

  PlayerState m_player_state{PlayerState::UNINIT};
  bool m_stop_received{};
  int m_game_round{};
  time_point_t m_last_change_time{};
  time_point_t m_start_time{};
  time_point_t m_last_info_time{};
  time_point_t m_last_warn_time{};
  mutable std::mutex m_mtx;

  [[nodiscard]] PlayerState get_state() const {
    std::scoped_lock sl(m_mtx);
    return m_player_state;
  }

  [[nodiscard]] bool get_stop() const {
    std::scoped_lock sl(m_mtx);
    return m_stop_received;
  }

  void check_progress() {
    std::scoped_lock sl(m_mtx);
    auto current_time = clock_t::now();
    auto dur_since_last_change = current_time - m_last_change_time;
    auto dur_since_last_warn = current_time - m_last_warn_time;

    if (dur_since_last_change > WAIT_DUR
        && dur_since_last_warn > WARN_MESSAGE_DUR) {
      fmt::println(
          "Monitor WARN: round:{}, player in state {} for {} sec",
          m_game_round,
          player_state_to_str(m_player_state),
          std::chrono::duration_cast<std::chrono::seconds>(
              dur_since_last_change)
              .count());
      m_last_warn_time = current_time;
    }
  }

  void check_info() {
    std::scoped_lock sl(m_mtx);
    auto current_time = clock_t::now();
    auto dur_since_last_info = current_time - m_last_info_time;
    if (dur_since_last_info > INFO_MESSAGE_DUR) {
      fmt::println("Monitor INFO: round:{}", m_game_round);
      m_last_info_time = current_time;
    }
  }

public:
  void set_state(int game_round, PlayerState player_state) {
    std::scoped_lock sl(m_mtx);
    m_game_round = game_round;
    if (m_player_state != player_state) {
      m_player_state = player_state;
      m_last_change_time = clock_t::now();
    }
  }

  void request_stop() {
    std::scoped_lock sl(m_mtx);
    m_stop_received = true;
  }

  void log(char const *const message) const {
    std::scoped_lock sl(m_mtx);
    auto current_time = clock_t::now();
    auto dur_since_start = current_time - m_start_time;
    fmt::println(
        "Monitor LOG: round:{}, gametime:{} sec, message:{}",
        m_game_round,
        std::chrono::duration_cast<std::chrono::seconds>(dur_since_start)
            .count(),
        message);
  }

  void run() {
    m_start_time = clock_t::now();
    m_last_info_time = m_start_time;
    m_last_warn_time = m_start_time;
    while (!get_stop() && get_state() != PlayerState::TERMINATING) {
      check_progress();
      check_info();
      std::this_thread::sleep_for(CHECK_DUR);
    }
  }
};

#endif // MONITOR_THREAD_H
