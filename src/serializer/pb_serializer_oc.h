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
  ArrayWrapper() { array_ = [[Array alloc] init]; }

  ArrayWrapper(PlatformObject array) {
    assert([array isKindOfClass:[Array class]]);
    array_ = (Array*)(array);
  }

  ~ArrayWrapper() { array_ = nil; }

  operator PlatformObject() const noexcept { return array_; }

  void Add(PlatformObject value) { [array_ addObject:value]; }

  std::vector<PlatformObject> Values() const {
    std::vector<PlatformObject> res;
    for (NSObject* v in array_) {
      res.emplace_back(v);
    }
    return res;
  }

 private:
  Array* array_;
};

template <bool reader>
struct DictWrapper<PlatformObject, reader> {
  using Dict = std::conditional_t<reader, NSDictionary, NSMutableDictionary>;
  DictWrapper() { dict_ = [[Dict alloc] init]; }

  DictWrapper(PlatformObject dict) {
    assert([dict isKindOfClass:[Dict class]]);
    dict_ = (Dict*)(dict);
  }

  ~DictWrapper() { dict_ = nil; }

  operator PlatformObject() const noexcept { return dict_; }

  void Add(PlatformObject key, PlatformObject value) {
    assert([key isKindOfClass:[NSString class]]);
    [dict_ setValue:value forKey:(NSString*)(key)];
  }

  void Add(const char* key, PlatformObject value) {
    [dict_ setValue:value forKey:detail::to_oc(key)];
  }

  std::vector<std::pair<PlatformObject, PlatformObject>> KeyAndValues() const {
    std::vector<std::pair<PlatformObject, PlatformObject>> res;
    for (NSString* key in dict_) {
      res.emplace_back(key, dict_[key]);
    }
    return res;
  }

 private:
  Dict* dict_;
};

template <>
struct TypeCheck<PlatformObject> {
  TypeCheck(PlatformObject obj) : obj_(obj) {}
  bool IsNullOrUndefined() const {
    return obj_ != nil && [obj_ isKindOfClass:[NSNull class]];
  }

  bool IsArray() const {
    return obj_ != nil && [obj_ isKindOfClass:[NSArray class]];
  }

  bool IsDict() const {
    return obj_ != nil && [obj_ isKindOfClass:[NSDictionary class]];
  }

  PlatformObject obj_;
};

template <typename T>
struct Serializer<PlatformObject, T> {
  T from_platform(PlatformObject object) {
    T v{};
    detail::from_oc(object, v);
    return v;
  }

  PlatformObject to_platform(const T& v) { return detail::to_oc(v); }
};

}  // namespace magic::pb

namespace magic {
using pb::DescriptorPool;
using pb::PBInfo;
using pb::PBOptions;
using pb::PlatformObject;
using pb::WarnningFields;

std::pair<ErrorCode, PlatformObject> from_pb(DescriptorPool* descriptor_pool,
                                             const PBInfo& pb_info,
                                             const PBOptions& options = {});

std::pair<ErrorCode, PlatformObject> from_default_pb(
    DescriptorPool* descriptor_pool,
    std::string_view pb_type,
    const PBOptions& options = {});

std::pair<ErrorCode, NSData*> to_pb(PlatformObject object,
                                    DescriptorPool* descriptor_pool,
                                    std::string_view pb_type,
                                    WarnningFields* warnning_fields = nullptr);
}  // namespace magic

#endif  // CONVERT_SRC_SERIALIZER_PB_SERIALIZER_OC_H_
