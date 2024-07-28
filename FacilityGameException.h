#ifndef ENUM_FACILITY_GAME_EXCEPTION_H
#define ENUM_FACILITY_GAME_EXCEPTION_H

#include <exception>
#include <string>

class FacilityGameException : public std::exception {
public:
  explicit FacilityGameException(char const *message)
      : m_message("FacilityGameException: ") {
    m_message += message;
  }

  [[nodiscard]] char const *what() const noexcept override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

#endif // ENUM_FACILITY_GAME_EXCEPTION_H
