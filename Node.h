#ifndef NODE_H
#define NODE_H

#include <fmt/core.h>

struct Node {
  std::size_t index;
  std::size_t value;

  std::string toString() {
    return fmt::format("node[{}] == {}", index, value);
  }
};

#endif // NODE_H
