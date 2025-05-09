#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface PBConvert : NSObject
- (instancetype)initWithPBDescPath:(NSString*)pbDescPath;

- (NSData*)encode:(id)object messageType:(NSString*)messageType;

- (id)decode:(NSData*)data
     messageType:(NSString*)messageType
    useCamelcase:(BOOL)useCamelcase;

- (id)create:(NSString*)messageType useCamelcase:(BOOL)useCamelcase;
@end

NS_ASSUME_NONNULL_END
