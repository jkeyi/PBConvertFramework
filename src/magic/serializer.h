#ifndef CONVERT_SRC_MAGIC_SERIALIZER_H_
#define CONVERT_SRC_MAGIC_SERIALIZER_H_

#include <charconv>
#include <sstream>

namespace magic::detail {
auto from_fn(auto&&... args) {
  return from(std::forward<decltype(args)>(args)...);
}

auto to_fn(auto&&... args) {
  return to(std::forward<decltype(args)>(args)...);
}

template <typename Stream, typename T>
struct Serializer {
  static auto from(auto&&... args) {
    return from_fn(std::forward<decltype(args)>(args)...);
  }

  static auto to(auto&&... args) {
    return to_fn(std::forward<decltype(args)>(args)...);
  }
};

template <typename T>
  requires(std::is_arithmetic_v<T>)
std::optional<T> string_to_number(std::string_view str) {
  T v{};
  std::stringstream ss;
  ss << str;
  ss >> v;
  return ss.good() ? std::optional<T>(v) : std::optional<T>();
}
}

#endif  // CONVERT_SRC_MAGIC_SERIALIZER_H_
