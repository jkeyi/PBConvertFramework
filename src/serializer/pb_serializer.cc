#include "serializer/pb_serializer.h"

namespace magic::pb {
// Begin: PBErrorCategory
const PBErrorCategory& PBErrorCategory::instance() {
  static auto* instance = new PBErrorCategory();
  return *instance;
}

const char* PBErrorCategory::name() const noexcept {
  return "ShellapiPBError";
}

std::string PBErrorCategory::message(int code) const {
  static auto* error_map = new std::unordered_map<int, std::string>{
      {static_cast<int>(PBError::kNoConvertFunction), "No convert function!"},
      {static_cast<int>(PBError::kPBMessageInfoError),
       "PB Message info error!"},
      {static_cast<int>(PBError::kPBGetRepeatItemError),
       "Get repeat item error!"},
      {static_cast<int>(PBError::kPBNoExistAndNoDefaultValue),
       "PB field no exist and no default value!"},
      {static_cast<int>(PBError::KPBMessageNotFound), "PB message not found!"},
      {static_cast<int>(PBError::kPBParseError), "PB parse error!"},
      {static_cast<int>(PBError::kPBPoolIsNull), "PB pool is null!"},
  };
  return (*error_map)[code];
}
// End: PBErrorCategory

std::error_code make_error_code(PBError code) {
  return {static_cast<int>(code), PBErrorCategory::instance()};
}
}
