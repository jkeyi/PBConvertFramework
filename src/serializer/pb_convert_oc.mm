#include "serializer/pb_convert_oc.h"

#include <google/protobuf/descriptor.pb.h>

#include <fstream>

namespace magic {
PBConvert::PBConvert(const std::string& pb_desc_path) {
  std::ifstream file(pb_desc_path, std::ios::binary | std::ios::ate);
  if (file) {
    using google::protobuf::FileDescriptorSet;
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    FileDescriptorSet descriptors;
    if (!descriptors.ParseFromArray(buffer.data(),
                                    static_cast<int>(buffer.size())))
      return;
    pb_pool_ = std::make_unique<DescriptorPool>();
    for (int i = 0; i < descriptors.file_size(); i++) {
      assert(pb_pool_->BuildFile(descriptors.file(i)));
    }
  }
}

PBConvert::~PBConvert() = default;

std::unique_ptr<PBConvert> PBConvert::New(const std::string& pb_desc_path) {
  auto convert = std::unique_ptr<PBConvert>(new PBConvert(pb_desc_path));
  return convert->pb_pool_ ? std::move(convert) : std::unique_ptr<PBConvert>{};
}

NSData* PBConvert::Encode(PlatformObject object, std::string_view pb_type) {
  auto res = to_pb(object, pb_pool_.get(), pb_type);
  return !res.first ? res.second : nil;
}

PlatformObject PBConvert::Decode(const PBInfo& pb_info,
                                 const PBOptions& options) {
  auto res = from_pb(pb_pool_.get(), pb_info, options);
  return !res.first ? res.second : nil;
}

PlatformObject PBConvert::Create(std::string_view pb_type,
                                 const PBOptions& options) {
  auto res = from_default_pb(pb_pool_.get(), pb_type, options);
  return !res.first ? res.second : nil;
}
}  // namespace magic
