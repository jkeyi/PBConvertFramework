#ifndef CONVERT_SRC_MAGIC_ERROR_CODE_H_
#define CONVERT_SRC_MAGIC_ERROR_CODE_H_
#include <ostream>
#include <string>
#include <system_error>
#include <unordered_map>

#include "magic/type_info.h"

// -----------------------------------------------------------------------------
// Usage documentation
// -----------------------------------------------------------------------------
//
// Overview:
// This file implements the error handling mechanism of shellapi v2, the input
// parameters will be uniformly checked at the framework layer, and the api will
// be called only after the parameter validation is correct.

// Of course, you can also customize the error code for you api, for next step:
// 1. define custom error type, for example:
// enum class CustomError : int {
//   MISSING_ARG,
//   ARG_TYPE_ERROR,
//   NO_SUCH_METHOD,
//   PIPELINE_POLICY_FORBIDDEN,
//   FORBIDDEN_CALL_FROM_CHILD_FRAME,
//   CORRUPTED_DATA,
//   UNSUPPORTED_OS,
//   END
// };
// NOTE: The error code cannot be 0, because 0 means success
// 2. define you custom error category, for example:
// class CustomCategory : public std::error_category {
//  public:
//   static const CustomCategory& instance() {
//     static auto* instance = new CustomCategory();
//     return *instance;
//   }

//   const char* name() const override{return "CustomCategory"}

//   std::string message(int code) const override {
//     if (code == MISSING_ARG)
//       return "MISSING_ARG";
//     else if (code = 'XXX')
//       return "xxxx";
//     else
//       return "";
//   }
// };
// 3. template specialization std::is_error_code_enum and impl make_error_code
// namespace std {
// template <>
// struct std::is_error_code_enum<CustomError> : true_type {};
// }  // namespace std

// inline std::error_code make_error_code(CustomError code) {
//   return {static_cast<int>(code), CustomCategory::instance()};
// }

namespace magic {
enum class CommonError;
}

namespace std {
template <>
struct std::is_error_code_enum<magic::CommonError> : true_type {};
}  // namespace std

namespace magic {
enum class CommonError : int {
  UNKNOWN = -1,
  SUCCESS = 0,
  FAILED,
  MISSING_ARG,
  ARG_TYPE_ERROR,
  NO_SUCH_METHOD,
  PIPELINE_POLICY_FORBIDDEN,
  FORBIDDEN_CALL_FROM_CHILD_FRAME,
  CORRUPTED_DATA,
  UNSUPPORTED_OS,
  TOO_MANY_ARGS,
  INVALID_ARG,
  RSP_TYPE_ERROR,
  END
};

class ErrorCategory : public std::error_category {
 public:
  static const ErrorCategory& instance();

  const char* name() const noexcept override;

  std::string message(int code) const override;
};

std::error_code make_error_code(magic::CommonError code);

class ErrorCode : public std::error_code {
 public:
  using std::error_code::error_code;

  ErrorCode(const std::string& additional, const ErrorCode& other);

  // NOLINTNEXTLINE(google-explicit-constructor)
  ErrorCode(bool val);

  ErrorCode(int val, const std::string& message, const std::string& category);

  ErrorCode(const ErrorCode&) = default;
  ErrorCode(ErrorCode&&) = default;
  ErrorCode& operator=(const ErrorCode&) = default;
  ErrorCode& operator=(ErrorCode&&) = default;

  const std::string& message() const;

  const std::string& category() const;

  void set_message(const std::string& message);

  bool operator==(const ErrorCode& other) const noexcept;

  bool operator!=(const ErrorCode& other) const noexcept;

 private:
  mutable std::string custom_message_;
  mutable std::string category_name_;
};

namespace detail {
IMPL_TYPE_NAME(magic::ErrorCode)
}
}  // namespace magic

std::ostream& operator<<(std::ostream& os,
                         const magic::ErrorCode& error_code);

#endif  // CONVERT_SRC_MAGIC_ERROR_CODE_H_
