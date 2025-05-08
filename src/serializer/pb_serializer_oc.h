#ifndef CONVERT_SRC_SERIALIZER_PB_SERIALIZER_OC_H_
#define CONVERT_SRC_SERIALIZER_PB_SERIALIZER_OC_H_

#include "serializer/oc_serializer.h"
#include "serializer/pb_serializer.h"

namespace magic::pb {
using PlatformObject = NSObject*;

template <>
const FromPbFunctionMap<PlatformObject>& GetFromPbFunctionMap();

template <bool reader>
struct ArrayWrapper<PlatformObject, reader> {
  using Array = std::conditional_t<reader, NSArray, NSMutableArray>;
  ArrayWrapper() {
    array_ = [[Array alloc] init];
  }

  ArrayWrapper(PlatformObject array) {
    assert([array isKindOfClass:[Array class]]);
    array_ = (Array*)(array);
  }

  ~ArrayWrapper() {
    array_ = nil;
  }

  operator PlatformObject() const noexcept {
    return array_;
  }

  void Add(PlatformObject value) {
    [array_ addObject:value];
  }

private:
  Array* array_;
};

template <bool reader>
struct DictWrapper<PlatformObject, reader> {
  using Dict = std::conditional_t<reader, NSDictionary, NSMutableDictionary>;
  DictWrapper() {
    dict_ = [[Dict alloc] init];
  }

  DictWrapper(PlatformObject dict) {
    assert([dict isKindOfClass:[Dict class]]);
    dict_ = (Dict*)(dict);
  }

  ~DictWrapper() {
    dict_ = nil;
  }

  operator PlatformObject() const noexcept {
    return  dict_;
  }

  void Add(PlatformObject key, PlatformObject value) {
    assert([key isKindOfClass:[NSString class]]);
    [dict_ setValue:value forKey:(NSString*)(key)];
  }

  void Add(const char* key, PlatformObject value) {
    [dict_ setValue:value forKey: detail::to_oc(key)];
  }

private:
  Dict* dict_;
};

}

namespace magic {
using pb::DescriptorPool;
using pb::PBInfo;
using pb::PBOptions;
using pb::PlatformObject;

std::pair<ErrorCode, PlatformObject> from_pb(DescriptorPool* descriptor_pool,
                                             const PBInfo& pb_info,
                                             const PBOptions& options = {});

std::pair<ErrorCode, PlatformObject> from_default_pb(
    DescriptorPool* descriptor_pool,
    std::string_view pb_type,
    const PBOptions& options = {});
}


#endif // CONVERT_SRC_SERIALIZER_PB_SERIALIZER_OC_H_
