#include "serializer/pb_serializer_oc.h"

#include <fstream>

#include "magic/serializer.h"

namespace magic::pb {
// BEGIN FROM_PB IMPL
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
    return {obj ? CommonError::SUCCESS : CommonError::FAILED, obj};       \
  }

#define MACRO_FROM_PB_FUNCTION_MAP_ITEM(type)                    \
  {                                                              \
    FieldDescriptor::CPPTYPE_##type,                             \
        from_pb<FieldDescriptor::CPPTYPE_##type, PlatformObject> \
  }

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
  return {obj ? CommonError::SUCCESS : CommonError::FAILED, obj};
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
// END FROM_PB IMPL

// BEGIN TO_PB IMPL
#define MACRO_TO_PB_IMPL(type, type_name)                                  \
  template <>                                                              \
  ErrorCode to_pb<FieldDescriptor::CPPTYPE_##type, PlatformObject>(        \
      PlatformObject object, Context & pb_context) {                       \
    std::pair<const FieldDescriptor*,                                      \
              decltype(std::declval<Reflection>().Get##type_name(          \
                  std::declval<Message>(), nullptr))>                      \
        value{};                                                           \
    value.first = pb_context.field;                                        \
    if (auto error_code = from_oc(object, value); !error_code) {           \
      auto func = pb_context.field->is_repeated()                          \
                      ? &Reflection::Add##type_name                        \
                      : &Reflection::Set##type_name;                       \
      (pb_context.reflection->*func)(pb_context.message, pb_context.field, \
                                     std::move(value.second));             \
      return CommonError::SUCCESS;                                         \
    } else {                                                               \
      PB_LOG(ERROR) << "binary_to_pb_impl error, name: "                   \
                    << pb_context.field->name() << ", " << error_code;     \
      return error_code;                                                   \
    }                                                                      \
  }

#define MACRO_TO_PB_FUNCTION_MAP_ITEM(type)                    \
  {                                                            \
    FieldDescriptor::CPPTYPE_##type,                           \
        to_pb<FieldDescriptor::CPPTYPE_##type, PlatformObject> \
  }

template <typename T>
ErrorCode from_oc(PlatformObject object,
                  std::pair<const FieldDescriptor*, T>& value) {
  using Type = std::decay_t<T>;
  if constexpr (std::is_same_v<T, const EnumValueDescriptor*>) {
    const EnumDescriptor* enum_desc = value.first->enum_type();
    if (!enum_desc) {
      return CommonError::CORRUPTED_DATA;
    }
    if ([object isKindOfClass:[NSString class]]) {
      std::string str;
      detail::from_oc(object, str);
      value.second = enum_desc->FindValueByName(str);
    } else if (int enum_value = 1; !detail::from_oc(object, enum_value)) {
      value.second = enum_desc->FindValueByNumber(enum_value);
    }
    return value.second ? CommonError::SUCCESS : CommonError::ARG_TYPE_ERROR;
  } else if constexpr (std::is_arithmetic_v<Type>) {
    if ([object isKindOfClass:[NSString class]]) {
      std::string str;
      detail::from_oc(object, str);
      if (auto v = detail::string_to_number<Type>(str)) {
        value.second = *v;
        return CommonError::SUCCESS;
      } else {
        return CommonError::ARG_TYPE_ERROR;
      }
    } else {
      return detail::from_oc(object, value.second);
    }
  } else {
    return detail::from_oc(object, value.second);
  }
}

MACRO_TO_PB_IMPL(INT32, Int32)
MACRO_TO_PB_IMPL(UINT32, UInt32)
MACRO_TO_PB_IMPL(INT64, Int64)
MACRO_TO_PB_IMPL(UINT64, UInt64)
MACRO_TO_PB_IMPL(FLOAT, Float)
MACRO_TO_PB_IMPL(DOUBLE, Double)
MACRO_TO_PB_IMPL(BOOL, Bool)
MACRO_TO_PB_IMPL(ENUM, Enum)

template <>
ErrorCode to_pb<FieldDescriptor::CPPTYPE_STRING, PlatformObject>(
    PlatformObject object,
    Context& pb_context) {
  auto func = pb_context.field->is_repeated() ? &Reflection::AddString
                                              : &Reflection::SetString;
  std::string str;
  if ([object isKindOfClass:[NSString class]]) {
    detail::from_oc(object, str);
    (pb_context.reflection->*func)(pb_context.message, pb_context.field,
                                   std::move(str));
    return CommonError::SUCCESS;
  } else if ([object isKindOfClass:[NSNumber class]]) {
    detail::from_oc([(NSNumber*)(object) stringValue], str);
    (pb_context.reflection->*func)(pb_context.message, pb_context.field,
                                   std::move(str));
    return CommonError::SUCCESS;
  } else {
    std::span<const uint8_t> data;
    if (auto error_code = detail::from_oc(object, data); !error_code) {
      (pb_context.reflection->*func)(pb_context.message, pb_context.field,
                                     std::string(data.begin(), data.end()));
      return CommonError::SUCCESS;
    } else {
      PB_LOG(ERROR) << "v8_to_pb_impl string error"
                    << ", name: " << pb_context.field->name();
      return CommonError::ARG_TYPE_ERROR;
    }
  }
}

template <>
ErrorCode to_pb<FieldDescriptor::CPPTYPE_MESSAGE, PlatformObject>(
    PlatformObject object,
    Context& pb_context) {
  Message* message = pb_context.message;
  assert(pb_context.field);
  if (pb_context.field) {
    message = pb_context.field->is_repeated()
                  ? pb_context.reflection->AddMessage(pb_context.message,
                                                      pb_context.field)
                  : pb_context.reflection->MutableMessage(pb_context.message,
                                                          pb_context.field);
  }
  return to_pb<PlatformObject>(object, message, pb_context.warnning_fields);
}

template <>
const ToPbFunctionMap<PlatformObject>& GetToPbFunctionMap<PlatformObject>() {
  static const auto* map = new ToPbFunctionMap<PlatformObject>{
      MACRO_TO_PB_FUNCTION_MAP_ITEM(INT32),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(UINT32),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(INT64),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(UINT64),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(FLOAT),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(DOUBLE),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(BOOL),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(STRING),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(MESSAGE),
      MACRO_TO_PB_FUNCTION_MAP_ITEM(ENUM)};
  return *map;
}
// END TO_PB IMPL
}  // namespace magic::pb

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

struct NSDataWrapper {
  using value_type = void;

  NSDataWrapper() = default;
  ~NSDataWrapper() { data_ = nil; }
  void resize(std::size_t size) { data_ = [NSMutableData dataWithLength:size]; }

  const void* data() const noexcept { return [data_ bytes]; }

  NSData* data_ = nil;
};

std::pair<ErrorCode, NSData*> to_pb(PlatformObject object,
                                    DescriptorPool* descriptor_pool,
                                    std::string_view pb_type,
                                    WarnningFields* warnning_fields) {
  auto res = pb::to_pb<PlatformObject, NSDataWrapper>(object, descriptor_pool,
                                                      pb_type, warnning_fields);
  return {res.first, res.second.data_};
}
}  // namespace magic
