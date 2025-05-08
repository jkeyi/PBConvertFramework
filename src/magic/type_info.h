#ifndef CONVERT_SRC_MAGIC_TYPE_INFO_H_
#define CONVERT_SRC_MAGIC_TYPE_INFO_H_

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// -----------------------------------------------------------------------------
// Usage documentation
// -----------------------------------------------------------------------------
//
// Overview:
// This file implements the shellapi v2 TypeInfo and TypeCast;

// Because aha disable RTTI, it is difficult to identify the same type of
// typeinfo in two dynamic libraries, this will cause data stored in one dynamic
// library to fail when converted to the same type of data in another dynamic
// library(std::any has the same problem, if store std::string in one dynamic
// library, convert it to std::string in another dynamic library will fail);

// This file implements a method that is compatible with obtaining type
// information across dynamic libraries when RTTI is disabled, this way will be
// enabled by default under aha debug; Under aha release, it is compiled into
// the only one dynamic library, so there is no such problem; It implements a
// method(get typeinfo) similar to std::any under release

// NOTE: If RTTI enable, use typeid to get TypeInfo by default

#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define SHELL_API_RTTI_ENABLED
#endif
#elif defined(__GNUC__)
#if defined(__GXX_RTTI)
#define SHELL_API_RTTI_ENABLED
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define SHELL_API_RTTI_ENABLED
#endif
#endif

#ifndef SHELL_API_TYPE_INFO_IMPL_TYPE
#ifdef SHELL_API_RTTI_ENABLED
#define SHELL_API_TYPE_INFO_IMPL_TYPE 0
#else
#ifndef NDEBUG
#define SHELL_API_TYPE_INFO_IMPL_TYPE 1
#else
#define SHELL_API_TYPE_INFO_IMPL_TYPE 2
#endif
#endif
#endif

#define IMPL_TYPE_NAME(Type)                         \
  template <>                                        \
  struct TypeName<Type> {                            \
    const char* operator()() const { return #Type; } \
  };

namespace magic {
class Null {};
class Undefined {};

inline constexpr Undefined _undefined{};
inline constexpr Null _null{};
}  // namespace magic

namespace magic::detail {
template <typename T>
struct UniqueTypeId {
  static constexpr int id = 0;
};
template <typename T>
constexpr int UniqueTypeId<T>::id;

template <typename T>
struct TypeId {
  constexpr const void* operator()() const { return &UniqueTypeId<T>::id; }
};

template <typename T>
struct TypeName {
  const char* operator()() const;
};

template <typename T>
const char* MakeTypeName() {
  return TypeName<std::remove_cv_t<std::remove_reference_t<T>>>()();
}

template <typename T>
const void* MakeTypeId() {
  return TypeId<std::remove_cv_t<std::remove_reference_t<T>>>()();
}

inline bool CompareTypeInfo(const void* src_type_info,
                            const void* dst_type_info) {
  if (!src_type_info || !dst_type_info) {
    return false;
  }
#if SHELL_API_TYPE_INFO_IMPL_TYPE == 0
  return *reinterpret_cast<const std::type_info*>(src_type_info) ==
         *reinterpret_cast<const std::type_info*>(dst_type_info);
#elif SHELL_API_TYPE_INFO_IMPL_TYPE == 1
  return strcmp(reinterpret_cast<const char*>(src_type_info),
                reinterpret_cast<const char*>(dst_type_info)) == 0;
#elif SHELL_API_TYPE_INFO_IMPL_TYPE == 2
  return src_type_info == dst_type_info;
#else
  static_assert(false, "not support SHELL_API_TYPE_INFO_IMPL_TYPE");
  return false;
#endif
}

template <typename T>
struct TypeName<T*> {
  const char* operator()() const {
    const char* prefix = "pointer_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class T>
struct TypeName<std::vector<T>> {
  const char* operator()() const {
    const char* prefix = "vector_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class T>
struct TypeName<std::list<T>> {
  const char* operator()() const {
    const char* prefix = "list_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class Key, class T>
struct TypeName<std::map<Key, T>> {
  const char* operator()() const {
    const char* prefix = "map_";
    static const auto* name = new std::string(
        std::string(prefix) + MakeTypeName<Key>() + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class Key, class T>
struct TypeName<std::unordered_map<Key, T>> {
  const char* operator()() const {
    const char* prefix = "unordered_map_";
    static const auto* name = new std::string(
        std::string(prefix) + MakeTypeName<Key>() + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class Key>
struct TypeName<std::set<Key>> {
  const char* operator()() const {
    const char* prefix = "set_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<Key>());
    return name->c_str();
  }
};

template <class Key>
struct TypeName<std::unordered_set<Key>> {
  const char* operator()() const {
    const char* prefix = "unordered_set_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<Key>());
    return name->c_str();
  }
};

template <class T>
struct TypeName<std::shared_ptr<T>> {
  const char* operator()() const {
    const char* prefix = "shared_ptr_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class T>
struct TypeName<std::unique_ptr<T>> {
  const char* operator()() const {
    const char* prefix = "unique_ptr_";
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <class T>
struct TypeName<std::weak_ptr<T>> {
  const char* prefix = "weak_ptr_";
  const char* operator()() const {
    static const auto* name =
        new std::string(std::string(prefix) + MakeTypeName<T>());
    return name->c_str();
  }
};

template <>
struct TypeName<std::variant<>> {
  const char* operator()() const { return "std::variant"; }
};

template <class T, class... Types>
struct TypeName<std::variant<T, Types...>> {
  const char* operator()() const {
    static const auto* name =
        new std::string(std::string(MakeTypeName<std::variant<Types...>>()) +
                        "_" + MakeTypeName<T>());
    return name->c_str();
  }
};

IMPL_TYPE_NAME(void)
IMPL_TYPE_NAME(bool)
IMPL_TYPE_NAME(char)
IMPL_TYPE_NAME(uint8_t)
IMPL_TYPE_NAME(int8_t)
IMPL_TYPE_NAME(uint16_t)
IMPL_TYPE_NAME(int16_t)
IMPL_TYPE_NAME(uint32_t)
IMPL_TYPE_NAME(int32_t)
IMPL_TYPE_NAME(uint64_t)
IMPL_TYPE_NAME(int64_t)
IMPL_TYPE_NAME(float)
IMPL_TYPE_NAME(double)
IMPL_TYPE_NAME(std::string)
IMPL_TYPE_NAME(magic::Null)
IMPL_TYPE_NAME(magic::Undefined)
}  // namespace magic::detail

namespace magic {
class TypeInfo {
 public:
  TypeInfo() = default;
  TypeInfo(TypeInfo&&) = default;
  TypeInfo(const TypeInfo&) = default;
  TypeInfo& operator=(const TypeInfo&) = default;
  TypeInfo& operator=(TypeInfo&&) = default;

  bool operator==(const TypeInfo& other) const noexcept {
    return detail::CompareTypeInfo(type_info_, other.type_info_);
  }

  bool operator!=(const TypeInfo& other) const noexcept {
    return !TypeInfo::operator==(other);
  }

  explicit operator bool() const noexcept { return !type_info_; }

  inline const void* type_info() const noexcept { return type_info_; }

 private:
  template <typename>
  friend TypeInfo MakeTypeInfo();

  explicit TypeInfo(const void* type_info) : type_info_(type_info) {}

 private:
  const void* type_info_ = nullptr;
};

template <typename T>
TypeInfo MakeTypeInfo() {
#if SHELL_API_TYPE_INFO_IMPL_TYPE == 0
  return TypeInfo(&typeid(T));
#elif SHELL_API_TYPE_INFO_IMPL_TYPE == 1
  return TypeInfo(detial::MakeTypeName<T>());
#elif SHELL_API_TYPE_INFO_IMPL_TYPE == 2
  return TypeInfo(detial::MakeTypeId<T>());
#else
  static_assert(false, "not support SHELL_API_TYPE_INFO_IMPL_TYPE");
  return TypeInfo(nullptr);
#endif
}

template <typename T>
T* TypeCast(void* src_ptr, const TypeInfo& type_info) {
  return MakeTypeInfo<T>() == type_info ? reinterpret_cast<T*>(src_ptr)
                                        : nullptr;
}
}
#endif  // CONVERT_SRC_MAGIC_TYPE_INFO_H_
