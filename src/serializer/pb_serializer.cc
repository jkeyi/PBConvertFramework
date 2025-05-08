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

const FieldDescriptor* find_field(const Descriptor* descriptor,
                                  const Reflection* ref,
                                  const std::string& name) {
  const auto* field = descriptor->FindFieldByName(name);
  if (!field) {
    field = ref->FindKnownExtensionByName(name);
  }
  if (!field) {
    for (auto i = 0; i < descriptor->field_count(); ++i) {
      if (descriptor->field(i)->json_name() == name) {
        field = descriptor->field(i);
        break;
      }
    }
  }
  return field;
}

ErrorCode MakeErrorCode(ErrorCode error_code,
                        const std::string& field_name,
                        const std::string& message_type) {
  static const char* word = "Field: ";
  if (error_code && error_code.message().find(word) == std::string::npos) {
    error_code.set_message(error_code.message() + " Message Type: " +
                           message_type + ", " + word + field_name);
  }
  return error_code;
}

bool IsMessageInitialized(Message* message) {
  return message->IsInitialized();
}

const Message& GetPBMessage(const Reflection* reflection,
                            const Message& message,
                            const FieldDescriptor* field) {
  return reflection->GetMessage(message, field);
}

void AddWarnningField(WarnningFields* warnning_fields,
                      const Descriptor* descriptor,
                      const FieldDescriptor* field) {
  if (warnning_fields && field && descriptor) {
    warnning_fields->emplace(descriptor->full_name() + ":" + field->name());
  }
}
}  // namespace magic::pb
