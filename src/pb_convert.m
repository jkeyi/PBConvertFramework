#import "pb_convert.h"
@implementation PBConvert

-(void)Test:(id) anyData {
  if([anyData isKindOfClass:[NSString class]]) {
    NSString* str = anyData;
    NSLog(@"%@", str);
    NSFileHandle* file = [NSFileHandle fileHandleForReadingAtPath:str];
    NSData* data = [file readDataToEndOfFileAndReturnError:nil];
    NSLog(@"%@", data);
  }
}

@end
