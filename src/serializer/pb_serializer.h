#ifndef CONVERT_SRC_SERIALIZER_PB_SERIALIZER_H_
#define CONVERT_SRC_SERIALIZER_PB_SERIALIZER_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include <functional>
#include <ostream>
#include <tuple>

#include "magic/error_code.h"

namespace magic::pb {
enum class PBError;
}

namespace std {
template <>
struct std::is_error_code_enum<magic::pb::PBError> : true_type {};
}  // namespace std

namespace magic::pb {
using namespace google::protobuf::util;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Map;
using google::protobuf::Message;
using google::protobuf::Reflection;
using google::protobuf::util::JsonParseOptions;

enum class PBError : int {
  kNoConvertFunction = 100,
  kPBMessageInfoError,
  kPBGetRepeatItemError,
  kPBNoExistAndNoDefaultValue,
  KPBMessageNotFound,
  kPBParseError,
  kPBPoolIsNull,
};

class PBErrorCategory : public std::error_category {
 public:
  static const PBErrorCategory& instance();

  const char* name() const noexcept override;

  std::string message(int code) const override;
};

std::error_code make_error_code(PBError code);

#define PB_LOG(LEVEL) std::cout

using WarnningFields = std::set<std::string>;

struct PBInfo {
  std::string_view type;
  std::string_view data;
};

struct PBOptions {
  bool use_camelcase = false;
};

struct Context {
  Message* message = nullptr;
  const Reflection* reflection = nullptr;
  const FieldDescriptor* field = nullptr;
  PBOptions options;
  std::optional<int> index;
  WarnningFields* warnning_fields = nullptr;
};

const FieldDescriptor* find_field(const Descriptor* descriptor,
                                  const Reflection* ref,
                                  const std::string& name);

bool IsMessageInitialized(Message* message);

ErrorCode MakeErrorCode(ErrorCode error_code,
                        const std::string& field_name,
                        const std::string& message_type);

void AddWarnningField(WarnningFields* warnning_fields,
                      const Descriptor* descriptor,
                      const FieldDescriptor* field);

// BEGIN FROM_PB FORWARD DEFINE
template <FieldDescriptor::CppType T, typename Object>
std::pair<ErrorCode, Object> from_pb(const Context& pb_context);

template <typename Object>
using FromPbFunction =
    std::function<std::pair<ErrorCode, Object>(const Context&)>;

template <typename Object>
using FromPbFunctionMap =
    std::unordered_map<FieldDescriptor::CppType, FromPbFunction<Object>>;

template <typename Object>
const FromPbFunctionMap<Object>& GetFromPbFunctionMap();
// END FROM_PB FORWARD DEFINE

// BEGIN TO_PB FORWARD DEFINE
template <FieldDescriptor::CppType T, typename Object>
ErrorCode to_pb(Object obj, Context& pb_context);

template <typename Object>
using ToPbFunction = std::function<ErrorCode(Object, Context&)>;

template <typename Object>
using ToPbFunctionMap =
    std::unordered_map<FieldDescriptor::CppType, ToPbFunction<Object>>;

template <typename Object>
const ToPbFunctionMap<Object>& GetToPbFunctionMap();
// END TO_PB FORWARD DEFINE

template <typename Object, bool reader>
struct DictWrapper {
  operator Object() const noexcept;
  void Add(Object key, Object value);
};

template <typename Object, bool reader>
struct ArrayWrapper {
  operator Object() const noexcept;
  void Add(Object value);
};

template <typename Object>
struct TypeCheck {
  bool IsNullOrUndefined() const;
  bool IsArray() const;
  bool IsDict() const;
};

template <typename Object, typename T>
struct Serializer {
  T from_platform(Object);
  Object to_platform(const T&);
};

template <typename Object>
struct IngoreErrorWhenConvertToPbOptionalField : public std::false_type {};

static const std::string& kKeyFieldName = *(new std::string("key"));
static const std::string& kValueFieldName = *(new std::string("value"));

// BEGIN FROM_PB IMPL
template <typename Key, typename Value>
std::tuple<ErrorCode, Key, Value> from_pb(Message* message,
                                          const PBOptions& options) {
  const auto* descriptor = message->GetDescriptor();
  const auto* ref = message->GetReflection();
  const auto* key = descriptor->FindFieldByName(kKeyFieldName);
  const auto* value = descriptor->FindFieldByName(kValueFieldName);

  static const auto& function_map = GetFromPbFunctionMap<Key>();
  auto it = function_map.find(key->cpp_type());
  if (it == function_map.end()) {
    assert(false);
    PB_LOG(ERROR) << "pb_to_v8<true> key ConvertFunction not found: "
                  << key->cpp_type();
    return {PBError::kNoConvertFunction, {}, {}};
  }
  Context context{
      .message = message, .reflection = ref, .field = key, .options = options};
  if (auto key_result = it->second(context); key_result.first) {
    return {std::move(key_result.first), std::move(key_result.second), {}};
  } else {
    static const auto& function_map = GetFromPbFunctionMap<Value>();
    it = function_map.find(value->cpp_type());
    if (it == function_map.end()) {
      assert(false);
      PB_LOG(ERROR) << "pb_to_v8<true> value ConvertFunction not found: "
                    << value->cpp_type();
      return {PBError::kNoConvertFunction, {}, {}};
    }
    context.field = value;
    auto value_result = it->second(context);
    return {std::move(value_result.first), std::move(key_result.second),
            std::move(value_result.second)};
  }
}

template <typename Object>
std::pair<ErrorCode, Object> from_pb(Message* message,
                                     const PBOptions& options) {
  const auto* descriptor = message->GetDescriptor();
  const auto* ref = message->GetReflection();
  auto field_count = descriptor->field_count();
  DictWrapper<Object, false> object_wrapper;
  std::pair<ErrorCode, Object> result;

  for (decltype(field_count) i = 0; i < field_count; ++i) {
    const auto* field = descriptor->field(i);
    if (field->is_optional() && !ref->HasField(*message, field) &&
        !field->has_default_value()) {
      continue;
    }
    static const auto& func_map = GetFromPbFunctionMap<Object>();
    auto it = func_map.find(field->cpp_type());
    if (it == func_map.end()) {
      assert(false);
      PB_LOG(ERROR) << "pb_to_v8<false> field ConvertFunction not found: "
                    << field->cpp_type() << ", " << field->name() << ", "
                    << descriptor->full_name();
      return {PBError::kNoConvertFunction, {}};
    }

    const auto& field_name =
        options.use_camelcase ? field->json_name() : field->name();

    Context context{.message = message,
                    .reflection = ref,
                    .field = field,
                    .options = options};
    if (field->is_repeated()) {
      auto size = ref->FieldSize(*message, field);
      Object object = field->is_map()
                          ? static_cast<Object>(DictWrapper<Object, false>())
                          : static_cast<Object>(ArrayWrapper<Object, false>());
      const bool is_map = field->is_map();
      for (decltype(size) index = 0; index < size; ++index) {
        Object key{}, value{};
        if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
          auto* item = const_cast<Message*>(
              &(ref->GetRepeatedMessage(*message, field, index)));
          if (field->is_map()) {
            auto sub_result = from_pb<Object, Object>(item, options);
            result.first = std::move(std::get<0>(sub_result));
            key = std::move(std::get<1>(sub_result));
            value = std::move(std::get<2>(sub_result));
          } else {
            auto sub_result = from_pb<Object>(item, options);
            result.first = std::move(sub_result.first);
            value = std::move(sub_result.second);
          }
        } else {
          context.index = index;
          result = it->second(context);
          value = std::move(result.second);
        }
        if (result.first) {
          break;
        } else if (is_map) {
          DictWrapper<Object, false>(object).Add(key, value);
        } else {
          ArrayWrapper<Object, false>(object).Add(value);
        }
      }
      result.first = CommonError::SUCCESS;
      result.second = object;
    } else {
      result = it->second(context);
    }
    if (result.first) {
      break;
    } else {
      object_wrapper.Add(field_name.c_str(), result.second);
    }
  }
  return {std::move(result.first), object_wrapper};
}

template <typename Object>
std::pair<ErrorCode, Object> from_pb(DescriptorPool* descriptor_pool,
                                     const PBInfo& pb_info,
                                     const PBOptions& options = {}) {
  const Descriptor* descriptor =
      descriptor_pool->FindMessageTypeByName(std::string(pb_info.type));
  if (!descriptor) {
    PB_LOG(ERROR) << "FindMessageTypeByName error, type: " << pb_info.type;
    return {PBError::KPBMessageNotFound, {}};
  }

  DynamicMessageFactory factory;
  std::unique_ptr<Message> message(
      factory.GetPrototype(descriptor)->New());  // new message
  if (!message->ParseFromArray(pb_info.data.data(), pb_info.data.size())) {
    PB_LOG(ERROR) << "ParseFromArray error, pb.size(): " << pb_info.data.size();
    return {PBError::kPBParseError, {}};
  }

  return from_pb<Object>(message.get(), options);
}

template <typename Object>
std::pair<ErrorCode, Object> from_default_pb(DescriptorPool* descriptor_pool,
                                             std::string_view pb_type,
                                             const PBOptions& options = {}) {
  const Descriptor* descriptor =
      descriptor_pool->FindMessageTypeByName(std::string(pb_type));
  if (!descriptor) {
    PB_LOG(ERROR) << "FindMessageTypeByName error, type: " << pb_type;
    return {PBError::KPBMessageNotFound, {}};
  }

  DynamicMessageFactory factory;
  std::unique_ptr<Message> message(
      factory.GetPrototype(descriptor)->New());  // new message

  return from_pb<Object>(message.get(), options);
}
// END FROM_PB IMPL

// BEGIN TO_PB IMPL
template <typename Key, typename Value>
ErrorCode to_pb(Key k,
                Value v,
                Message* message,
                WarnningFields* warnning_fields) {
  const auto* descriptor = message->GetDescriptor();
  const auto* ref = message->GetReflection();
  const auto* key = descriptor->FindFieldByName(kKeyFieldName);
  const auto* value = descriptor->FindFieldByName(kValueFieldName);

  const auto& func_map = GetToPbFunctionMap<Key>();
  auto it = func_map.find(key->cpp_type());
  if (it == func_map.end()) {
    assert(false);
    PB_LOG(ERROR) << "v8_to_pb_map_item key ConvertFunction not found: "
                  << key->cpp_type();
    return PBError::kNoConvertFunction;
  }

  Context context{.message = message,
                  .reflection = ref,
                  .field = key,
                  .warnning_fields = warnning_fields};
  if (auto error_code = it->second(k, context)) {
    return error_code;
  }

  it = func_map.find(value->cpp_type());
  if (it == func_map.end()) {
    assert(false);
    PB_LOG(ERROR) << "v8_to_pb_map_item value ConvertFunction not found: "
                  << value->cpp_type();
    return PBError::kNoConvertFunction;
  }

  context.field = value;
  return it->second(v, context);
}

template <typename Object>
ErrorCode to_pb(Object object,
                Message* message,
                WarnningFields* warnning_fields) {
  const auto* descriptor = message->GetDescriptor();
  const auto* ref = message->GetReflection();
  auto values = DictWrapper<Object, true>(object).KeyAndValues();
  if (values.empty()) {
    return message->IsInitialized() ? CommonError::SUCCESS
                                    : CommonError::MISSING_ARG;
  }

  const FieldDescriptor* field = nullptr;
  for (auto& [k, v] : values) {
    if (TypeCheck<Object>(k).IsNullOrUndefined() ||
        TypeCheck<Object>(v).IsNullOrUndefined()) {
      continue;
    }
    field = find_field(descriptor, ref,
                       Serializer<Object, std::string>().from_platform(k));
    if (!field) {
      continue;
    }
    const auto& func_map = GetToPbFunctionMap<Object>();
    auto it = func_map.find(field->cpp_type());
    if (it == func_map.end()) {
      assert(false);
      PB_LOG(ERROR) << "v8_to_pb field ConvertFunction not found: "
                    << field->cpp_type() << ", " << field->name() << ", "
                    << descriptor->full_name();
      return MakeErrorCode(PBError::kNoConvertFunction, field->name(),
                           descriptor->full_name());
    }

    Context context{.message = message,
                    .reflection = ref,
                    .field = field,
                    .warnning_fields = warnning_fields};

    if (field->is_repeated()) {
      TypeCheck<Object> type_check(v);
      if ((field->is_map() && (type_check.IsArray() || !type_check.IsDict())) ||
          (!field->is_map() && !type_check.IsArray())) {
        PB_LOG(ERROR) << "v8_to_pb MESSAGE error: "
                      << "name: " << field->name();
        return MakeErrorCode(CommonError::ARG_TYPE_ERROR, field->name(),
                             descriptor->full_name());
      }
      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        if (field->is_map()) {
          auto property_list = DictWrapper<Object, true>(v).KeyAndValues();
          for (const auto& [property_key, property_value] : property_list) {
            Message* item = ref->AddMessage(message, field);
            if (auto error_code = to_pb<Object, Object>(
                    property_key, property_value, item, warnning_fields)) {
              return MakeErrorCode(error_code, field->name(),
                                   descriptor->full_name());
            }
          }
        } else {
          auto item_list = ArrayWrapper<Object, true>(v).Values();
          for (const auto& array_tiem : item_list) {
            Message* item = ref->AddMessage(message, field);
            if (auto error_code =
                    to_pb<Object>(array_tiem, item, warnning_fields)) {
              return MakeErrorCode(error_code, field->name(),
                                   descriptor->full_name());
            }
          }
        }
      } else {
        auto item_list = ArrayWrapper<Object, true>(v).Values();
        for (size_t i = 0; i < item_list.size(); ++i) {
          context.index = static_cast<int>(i);
          if (auto error_code = it->second(item_list[i], context)) {
            return MakeErrorCode(error_code, field->name(),
                                 descriptor->full_name());
          }
        }
      }
    } else {
      auto error_code = it->second(v, context);
      if (error_code &&
          (!IngoreErrorWhenConvertToPbOptionalField<Object>::value ||
           !field->is_optional())) {
        return MakeErrorCode(error_code, field->name(),
                             descriptor->full_name());
      } else if (error_code) {
        AddWarnningField(warnning_fields, descriptor, field);
      }
    }
  }
  return message->IsInitialized()
             ? CommonError::SUCCESS
             : MakeErrorCode(CommonError::MISSING_ARG,
                             message->InitializationErrorString(),
                             descriptor->full_name());
}

template <typename Object, typename Buffer = std::vector<uint8_t>>
std::pair<ErrorCode, Buffer> to_pb(Object object,
                                   DescriptorPool* descriptor_pool,
                                   std::string_view pb_type,
                                   WarnningFields* warnning_fields = nullptr) {
  const Descriptor* descriptor =
      descriptor_pool->FindMessageTypeByName(std::string(pb_type));
  if (!descriptor) {
    PB_LOG(ERROR) << "FindMessageTypeByName error, type: " << pb_type;
    return {PBError::KPBMessageNotFound, Buffer{}};
  }

  DynamicMessageFactory factory;
  std::unique_ptr<Message> message(
      factory.GetPrototype(descriptor)->New());  // new message

  auto error_code = to_pb<Object>(object, message.get(), warnning_fields);
  Buffer pb_buffer;
  if (!error_code) {
    pb_buffer.resize(message->ByteSizeLong());
    auto* memory = reinterpret_cast<uint8_t*>(
        const_cast<typename Buffer::value_type*>(pb_buffer.data()));
    message->SerializeWithCachedSizesToArray(memory);
  }
  return {std::move(error_code), std::move(pb_buffer)};
}
// END TO_PB IMPL
}  // namespace magic::pb

#endif  // CONVERT_SRC_SERIALIZER_PB_SERIALIZER_H_
