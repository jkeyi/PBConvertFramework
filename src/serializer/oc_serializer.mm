#include "serializer/oc_serializer.h"

namespace magic::detail {
#define MACRO_FROM_OC_IMPL(NativeType, NSMethod)          \
  ErrorCode from_oc(NSObject* value, NativeType& cpp_v) { \
    if (auto* number = to_nstype<NSNumber>(value)) {      \
      cpp_v = [number NSMethod];                          \
      return magic::CommonError::SUCCESS;                 \
    } else {                                              \
      return magic::CommonError::ARG_TYPE_ERROR;          \
    }                                                     \
  }
#define MACRO_TO_OC_IMPL(NativeType, NSType)    \
  NSObject* to_oc(NativeType cpp_v) {           \
    return [NSNumber numberWith##NSType:cpp_v]; \
  }

template<typename T>
T* to_nstype(NSObject* value) {
  return [value isKindOfClass:[T class]] ? (T*)value : nil;
}

MACRO_FROM_OC_IMPL(int8_t, charValue)
MACRO_FROM_OC_IMPL(uint8_t, unsignedCharValue)
MACRO_FROM_OC_IMPL(int16_t, shortValue)
MACRO_FROM_OC_IMPL(uint16_t, unsignedShortValue)
MACRO_FROM_OC_IMPL(int32_t, intValue)
MACRO_FROM_OC_IMPL(uint32_t, unsignedIntValue)
MACRO_FROM_OC_IMPL(int64_t, longLongValue)
MACRO_FROM_OC_IMPL(uint64_t, unsignedLongLongValue)
MACRO_FROM_OC_IMPL(float, floatValue)
MACRO_FROM_OC_IMPL(double, doubleValue)
MACRO_FROM_OC_IMPL(bool, boolValue)

ErrorCode from_oc(NSObject* value, std::string& cpp_v) {
  std::string_view view;
  if(!from_oc(value, view)) {
    cpp_v = view;
    return magic::CommonError::SUCCESS;
  } else {
    return magic::CommonError::ARG_TYPE_ERROR;
  }
}

ErrorCode from_oc(NSObject* value, std::string_view& cpp_v) {
  if(auto* ns_v = to_nstype<NSString>(value)) {
    cpp_v = [ns_v UTF8String];
    return magic::CommonError::SUCCESS;
  } else {
    return magic::CommonError::ARG_TYPE_ERROR;
  }
}

ErrorCode from_oc(NSObject* value, std::span<const uint8_t>& cpp_v) {
  if(auto* ns_v = to_nstype<NSData>(value)) {
    cpp_v = std::span<const uint8_t>(static_cast<const uint8_t*>(ns_v.bytes), std::size_t(ns_v.length));
    return magic::CommonError::SUCCESS;
  } else {
    return magic::CommonError::ARG_TYPE_ERROR;
  }
}

ErrorCode from_oc(NSObject* value, std::vector<uint8_t>& cpp_v) {
  std::span<const uint8_t> data;
  if(!from_oc(value, data)) {
    cpp_v.assign(data.begin(), data.end());
    return magic::CommonError::SUCCESS;
  } else {
    return magic::CommonError::ARG_TYPE_ERROR;
  }
}

MACRO_TO_OC_IMPL(int8_t, Char)
MACRO_TO_OC_IMPL(uint8_t, UnsignedChar)
MACRO_TO_OC_IMPL(int16_t, Short)
MACRO_TO_OC_IMPL(uint16_t, UnsignedShort)
MACRO_TO_OC_IMPL(int32_t, Int)
MACRO_TO_OC_IMPL(uint32_t, UnsignedInt)
MACRO_TO_OC_IMPL(int64_t, LongLong)
MACRO_TO_OC_IMPL(uint64_t, UnsignedLongLong)
MACRO_TO_OC_IMPL(float, Float)
MACRO_TO_OC_IMPL(double, Double)
MACRO_TO_OC_IMPL(bool, Bool)

NSObject* to_oc(const char* cpp_v) {
  return [NSString stringWithUTF8String:cpp_v];
}

NSObject* to_oc(const std::string& cpp_v) {
  return to_oc(cpp_v.data());
}

NSObject* to_oc(std::span<const uint8_t> cpp_v) {
  return [NSData dataWithBytes:cpp_v.data() length:cpp_v.size()];
}

NSObject* to_oc(const std::vector<uint8_t>& cpp_v) {
  return [NSData dataWithBytes:cpp_v.data() length:cpp_v.size()];
}
}

