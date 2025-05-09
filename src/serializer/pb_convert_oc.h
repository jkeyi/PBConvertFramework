#ifndef CONVERT_SRC_SERIALIZER_PB_CONVERT_OC_H_
#define CONVERT_SRC_SERIALIZER_PB_CONVERT_OC_H_

#include "serializer/pb_serializer_oc.h"

namespace magic {
class PBConvert {
 public:
  ~PBConvert();

  static std::unique_ptr<PBConvert> New(const std::string& pb_desc_path);

  NSData* Encode(PlatformObject object, std::string_view pb_type);

  PlatformObject Decode(const PBInfo& pb_info, const PBOptions& options = {});

  PlatformObject Create(std::string_view pb_type,
                        const PBOptions& options = {});

 private:
  PBConvert(const std::string& pb_desc_path);

 private:
  std::unique_ptr<DescriptorPool> pb_pool_;
};
}  // namespace magic

#endif  // CONVERT_SRC_SERIALIZER_PB_CONVERT_OC_H_
