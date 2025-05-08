#ifndef CONVERT_SRC_SERIALIZER_OC_SERIALIZER_H_
#define CONVERT_SRC_SERIALIZER_OC_SERIALIZER_H_

#import <Foundation/Foundation.h>
#include "magic/error_code.h"
#include <span>

namespace magic::detail {

// ios platform data to cpp data
ErrorCode from_oc(NSObject* value, int8_t& cpp_v);
ErrorCode from_oc(NSObject* value, uint8_t& cpp_v);
ErrorCode from_oc(NSObject* value, int16_t& cpp_v);
ErrorCode from_oc(NSObject* value, uint16_t& cpp_v);
ErrorCode from_oc(NSObject* value, int32_t& cpp_v);
ErrorCode from_oc(NSObject* value, uint32_t& cpp_v);
ErrorCode from_oc(NSObject* value, int64_t& cpp_v);
ErrorCode from_oc(NSObject* value, uint64_t& cpp_v);
ErrorCode from_oc(NSObject* value, float& cpp_v);
ErrorCode from_oc(NSObject* value, double& cpp_v);
ErrorCode from_oc(NSObject* value, bool& cpp_v);
ErrorCode from_oc(NSObject* value, std::string& cpp_v);
ErrorCode from_oc(NSObject* value, std::string_view& cpp_v);
ErrorCode from_oc(NSObject* value, std::span<const uint8_t>& cpp_v);
ErrorCode from_oc(NSObject* value, std::vector<uint8_t>& cpp_v);


// cpp platform data to ios data
NSObject* to_oc(int8_t cpp_v);
NSObject* to_oc(uint8_t cpp_v);
NSObject* to_oc(int16_t cpp_v);
NSObject* to_oc(uint16_t cpp_v);
NSObject* to_oc(int32_t cpp_v);
NSObject* to_oc(uint32_t cpp_v);
NSObject* to_oc(int64_t cpp_v);
NSObject* to_oc(uint64_t cpp_v);
NSObject* to_oc(float cpp_v);
NSObject* to_oc(double cpp_v);
NSObject* to_oc(bool cpp_v);
NSObject* to_oc(const char* cpp_v);
NSObject* to_oc(const std::string& cpp_v);
NSObject* to_oc(std::span<const uint8_t> cpp_v);
NSObject* to_oc(const std::vector<uint8_t>& cpp_v);

}



#endif // CONVERT_SRC_SERIALIZER_OC_SERIALIZER_H_
