#include "magic/error_code.h"

namespace magic {

const ErrorCategory& ErrorCategory::instance() {
  static auto* instance = new ErrorCategory();
  return *instance;
}

const char* ErrorCategory::name() const noexcept {
  return "ShellapiCommonError";
}

std::string ErrorCategory::message(int code) const {
  static auto* error_map = new std::unordered_map<int, std::string>{
      {static_cast<int>(CommonError::UNKNOWN), "Unknown error!"},
      {static_cast<int>(CommonError::SUCCESS), "Success!"},
      {static_cast<int>(CommonError::FAILED), "Failed!"},
      {static_cast<int>(CommonError::MISSING_ARG), "Missing arg!"},
      {static_cast<int>(CommonError::ARG_TYPE_ERROR), "Arg type error!"},
      {static_cast<int>(CommonError::NO_SUCH_METHOD), "No such method!"},
      {static_cast<int>(CommonError::PIPELINE_POLICY_FORBIDDEN),
       "Pipeline policy fired, api call forbidden!"},
      {static_cast<int>(CommonError::FORBIDDEN_CALL_FROM_CHILD_FRAME),
       "Current not in main frame, forbidden!"},
      {static_cast<int>(CommonError::CORRUPTED_DATA), "Corrupted data!"},
      {static_cast<int>(CommonError::UNSUPPORTED_OS), "OS not supported!"},
      {static_cast<int>(CommonError::TOO_MANY_ARGS), "More args than need!"},
      {static_cast<int>(CommonError::INVALID_ARG), "Invalid arg!"},
      {static_cast<int>(CommonError::RSP_TYPE_ERROR), "Response type error!"},
  };
  return (*error_map)[code];
}

std::error_code make_error_code(magic::CommonError code) {
  return {static_cast<int>(code), magic::ErrorCategory::instance()};
}

ErrorCode::ErrorCode(const std::string& additional, const ErrorCode& other)
    : std::error_code(other.value(),
                      static_cast<const std::error_code&>(other).category()),
      custom_message_(std::string("\"") + additional + "\" " +
                      other.message()) {}

// NOLINTNEXTLINE(google-explicit-constructor)
ErrorCode::ErrorCode(bool val)
    : std::error_code(val ? CommonError::SUCCESS : CommonError::UNKNOWN) {}

ErrorCode::ErrorCode(int val,
                     const std::string& message,
                     const std::string& category)
    : std::error_code(static_cast<CommonError>(val)),
      custom_message_(message),
      category_name_(category) {}

const std::string& ErrorCode::message() const {
  if (custom_message_.empty()) {
    custom_message_ = std::error_code::message();
  }
  return custom_message_;
}

const std::string& ErrorCode::category() const {
  if (category_name_.empty()) {
    category_name_ = std::error_code::category().name();
  }
  return category_name_;
}

void ErrorCode::set_message(const std::string& message) {
  custom_message_ = message;
}

bool ErrorCode::operator==(const ErrorCode& other) const noexcept {
  return !value()
             ? value() == other.value()
             : (value() == other.value() && category() == other.category()) ||
                   static_cast<const std::error_code&>(*this) ==
                       static_cast<const std::error_code&>(other);
}

bool ErrorCode::operator!=(const ErrorCode& other) const noexcept {
  return !(*this == other);
}

}  // namespace magic

std::ostream& operator<<(std::ostream& os,
                         const magic::ErrorCode& error_code) {
  os << "category: " << error_code.category()
     << ", message: " << error_code.message()
     << ", value: " << error_code.value();
  return os;
}
