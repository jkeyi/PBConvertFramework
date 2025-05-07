#include "pb_serializer.h"
#include <vector>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.pb.h>

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptorSet;

bool Test() {
  std::vector<uint8_t> pb_bytes;
  FileDescriptorSet descriptors;
  if (!descriptors.ParseFromArray(pb_bytes.data(), pb_bytes.size()) ||
      !pb_bytes.size())
    return false;

//  pb_pool_ = std::make_unique<DescriptorPool>();
//  for (int i = 0; i < descriptors.file_size(); i++) {
//    pb_pool_->BuildFile(descriptors.file(i));
//  }
  return true;
}
