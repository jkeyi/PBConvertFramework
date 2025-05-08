#ifndef CONVERT_SRC_MAGIC_SERIALIZER_H_
#define CONVERT_SRC_MAGIC_SERIALIZER_H_

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
}

#endif  // CONVERT_SRC_MAGIC_SERIALIZER_H_
