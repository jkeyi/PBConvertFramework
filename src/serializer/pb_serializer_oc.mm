#include "serializer/pb_serializer_oc.h"

namespace magic::pb {
#define MACRO_FROM_PB_IMPL(type, name, dname)                             \
  template <>                                                             \
  inline std::pair<ErrorCode, PlatformObject>                             \
  from_pb<FieldDescriptor::CPPTYPE_##type, PlatformObject>(               \
      const Context& pb_context) {                                        \
    std::pair<const FieldDescriptor*,                                     \
              decltype(std::declval<Reflection>().Get##name(              \
                  std::declval<Message>(), nullptr))>                     \
        value{};                                                          \
    value.first = pb_context.field;                                       \
    if (pb_context.index) {                                               \
      value.second = pb_context.reflection->GetRepeated##name(            \
          *pb_context.message, pb_context.field, *pb_context.index);      \
    } else {                                                              \
      value.second = pb_context.reflection->HasField(*pb_context.message, \
                                                     pb_context.field)    \
                         ? pb_context.reflection->Get##name(              \
                               *pb_context.message, pb_context.field)     \
                         : pb_context.field->default_value_##dname();     \
    }                                                                     \
    PlatformObject obj = to_oc(value);                                    \
    return {obj ? CommonError::FAILED : CommonError::SUCCESS, obj};       \
  }

#define MACRO_FROM_PB_FUNCTION_MAP_ITEM(type) \
  {FieldDescriptor::CPPTYPE_##type,           \
   from_pb<FieldDescriptor::CPPTYPE_##type, PlatformObject>}

template <typename T>
PlatformObject to_oc(const std::pair<const FieldDescriptor*, T>& value) {
  if constexpr (std::is_same_v<T, const EnumValueDescriptor*>) {
    return detail::to_oc(value.second->number());
  } else if constexpr (std::is_same_v<T, int64_t> ||
                       std::is_same_v<T, uint64_t>) {
    return detail::to_oc(std::to_string(value.second));
  } else {
    return detail::to_oc(value.second);
  }
}

MACRO_FROM_PB_IMPL(INT32, Int32, int32)
MACRO_FROM_PB_IMPL(UINT32, UInt32, uint32)
MACRO_FROM_PB_IMPL(INT64, Int64, int64)
MACRO_FROM_PB_IMPL(UINT64, UInt64, uint64)
MACRO_FROM_PB_IMPL(FLOAT, Float, float)
MACRO_FROM_PB_IMPL(DOUBLE, Double, double)
MACRO_FROM_PB_IMPL(BOOL, Bool, bool)
MACRO_FROM_PB_IMPL(ENUM, Enum, enum)

template <>
std::pair<ErrorCode, PlatformObject>
from_pb<FieldDescriptor::CPPTYPE_STRING, PlatformObject>(
    const Context& pb_context) {
  std::string_view view;
  std::string scratch;
  if (pb_context.index) {
    view = pb_context.reflection->GetRepeatedStringReference(
        *pb_context.message, pb_context.field, *pb_context.index, &scratch);
  } else {
    view =
        pb_context.reflection->HasField(*pb_context.message, pb_context.field)
            ? pb_context.reflection->GetStringReference(
                  *pb_context.message, pb_context.field, &scratch)
            : pb_context.field->default_value_string();
  }

  PlatformObject obj = nil;
  if (pb_context.field->type() == FieldDescriptor::TYPE_BYTES) {
    obj = detail::to_oc(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(view.data()), view.size()));
  } else {
    obj = detail::to_oc(view.data());
  }
  return {obj ? CommonError::FAILED : CommonError::SUCCESS, obj};
}

template <>
std::pair<ErrorCode, PlatformObject>
from_pb<FieldDescriptor::CPPTYPE_MESSAGE, PlatformObject>(
    const Context& pb_context) {
  const Message* message =
      pb_context.reflection->HasField(*pb_context.message, pb_context.field)
          ? &(pb_context.reflection->GetMessage(*pb_context.message,
                                                pb_context.field))
          : nullptr;

  if (message) {
    return from_pb<PlatformObject>(const_cast<Message*>(message),
                    pb_context.options);
  } else {
    DynamicMessageFactory factory;
    std::unique_ptr<Message> msg_ptr(
        factory.GetPrototype(pb_context.field->message_type())
            ->New());  // new message
    if (msg_ptr) {
      return from_pb<PlatformObject>(msg_ptr.get(), pb_context.options);
    } else {
      PB_LOG(ERROR) << "pb_to_v8_impl<FieldDescriptor::CPPTYPE_MESSAGE> "
                       "message not exist and create Message failed: "
                    << pb_context.field->message_type();
      return {PBError::kPBNoExistAndNoDefaultValue, {}};
    }
  }
}

template <>
const FromPbFunctionMap<PlatformObject>& GetFromPbFunctionMap() {
  static const auto* map = new FromPbFunctionMap<PlatformObject>{
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(INT32),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(UINT32),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(INT64),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(UINT64),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(FLOAT),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(DOUBLE),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(BOOL),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(STRING),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(MESSAGE),
      MACRO_FROM_PB_FUNCTION_MAP_ITEM(ENUM)};
  return *map;
}
}


namespace magic {
std::pair<ErrorCode, PlatformObject> from_pb(DescriptorPool* descriptor_pool,
                                           const PBInfo& pb_info,
                                           const PBOptions& options) {
  return pb::from_pb<PlatformObject>(descriptor_pool, pb_info, options);
}

std::pair<ErrorCode, PlatformObject> from_default_pb(
  DescriptorPool* descriptor_pool,
  std::string_view pb_type,
  const PBOptions& options) {
  return pb::from_default_pb<PlatformObject>(descriptor_pool, pb_type, options);
}
}
