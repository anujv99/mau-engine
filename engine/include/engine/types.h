#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mau {

  // TODO: create custom string
  // TODO: create custom vector
  // TODO: remove exceptions, use error codes
  using String = std::string;

  template <typename Key, typename Value>
  using UnorderedMap = std::unordered_map<Key, Value>;

  template <typename T>
  using Vector = std::vector<T>;

  typedef uint8_t  TUint8;
  typedef uint32_t TUint32;
  typedef uint64_t TUint64;

  typedef int32_t  TInt32;

}
