#import "pb_convert.h"

#import "serializer/pb_convert_oc.h"

@interface PBConvert ()
@property(readwrite, strong) NSString* pbDescPath;
@property(readwrite) std::shared_ptr<magic::PBConvert> impl;
@end

@implementation PBConvert
- (instancetype)initWithPBDescPath:(NSString*)pbDescPath {
  self = [super init];
  self.pbDescPath = pbDescPath;
  self.impl = magic::PBConvert::New([pbDescPath UTF8String]);
  return self.impl ? self : nil;
}

- (NSData*)encode:(id)object messageType:(NSString*)messageType {
  return self.impl->Encode(object, [messageType UTF8String]);
}

- (id)decode:(NSData*)data
     messageType:(NSString*)messageType
    useCamelcase:(BOOL)useCamelcase {
  return self.impl->Decode(
      magic::PBInfo{
          .type = [messageType UTF8String],
          .data = {reinterpret_cast<const char*>(data.bytes), data.length}},
      magic::PBOptions{.use_camelcase = useCamelcase});
}

- (id)create:(NSString*)messageType useCamelcase:(BOOL)useCamelcase {
  return self.impl->Create([messageType UTF8String],
                           magic::PBOptions { .use_camelcase = useCamelcase });
}
@end
