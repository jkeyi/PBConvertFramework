#import "pb_convert.h"
#include "serializer/pb_serializer_oc.h"

@implementation PBConvert

-(void)Test:(id) anyData {
  if([anyData isKindOfClass:[NSString class]]) {
    NSString* str = anyData;
    NSLog(@"%@", str);
    NSFileHandle* file = [NSFileHandle fileHandleForReadingAtPath:str];
    NSData* data = [file readDataToEndOfFileAndReturnError:nil];
    NSLog(@"%@", data);
    NSValue* v = nil;

    //
    magic::from_pb(nullptr, {}, {});
    magic::to_pb(nil, nullptr, {}, {});
  }
}

@end
