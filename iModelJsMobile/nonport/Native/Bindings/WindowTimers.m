#import "WindowTimers.h"
#import  <IModelJsHost/IModelJsHost.h>

@implementation WTWindowTimers {
    NSUInteger _timeoutCounter;
    dispatch_queue_t _queue;
    NSMapTable *_dispatchSourcesMapping;
}

- (instancetype)init {
    if (self = [super init]) {
        _timeoutCounter = 0;
        _queue = dispatch_get_main_queue();
        _dispatchSourcesMapping = [NSMapTable weakToWeakObjectsMapTable];
        self.tolerance = 10;
    }
    return self;
}

- (void)extend:(id)context {
    context[@"setTimeout"] = self.setTimeout;
    context[@"clearTimeout"] = self.clearTimeout;
    context[@"setInterval"] = self.setInterval;
    context[@"clearInterval"] = self.clearTimeout;
}

- (NSNumber *)intervalWithCallable:(JSValue *)function
                           timeout:(JSValue *)timeout
                         arguments:(NSArray *)arguments
                        isInterval:(bool)isInterval {
    __block NSNumber *timeoutID = @(_timeoutCounter += 1);
    __block dispatch_source_t dispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
            0,
            0,
            _queue);
    dispatch_source_set_event_handler(dispatchSource, ^{
        if (!isInterval) {
            dispatch_source_cancel(dispatchSource);
        }
        [[IModelJsHost sharedInstance] exec:function arguments:arguments];
    });
    dispatch_time_t dispatchInterval = (uint64_t)[timeout toUInt32] * NSEC_PER_MSEC;
    dispatch_time_t dispatchTime = dispatch_time(DISPATCH_TIME_NOW, dispatchInterval);
    dispatch_source_set_timer(dispatchSource,
            dispatchTime,
            isInterval ? dispatchInterval : DISPATCH_TIME_FOREVER,
            self.tolerance * NSEC_PER_MSEC);
    dispatch_resume(dispatchSource);
    [_dispatchSourcesMapping setObject:dispatchSource forKey:timeoutID];

    return timeoutID;
}

- (id)setTimeout {
    return ^(JSValue *function, JSValue *timeout) {
        NSArray *originalArguments = [JSContext currentArguments];
        NSArray *arguments = @[];
        if ([originalArguments count] > 2) {
            arguments = [originalArguments subarrayWithRange:NSMakeRange(2,
                    [originalArguments count] - 2)];
        }
        // JSManagedValue * func = [[JSManagedValue alloc] initWithValue:function];
        return [self intervalWithCallable:function
                                  timeout:timeout
                                arguments:arguments
                               isInterval:NO];
    };
}

- (id)clearTimeout {
    return ^(NSNumber *timeout) {
        dispatch_source_t source = [self->_dispatchSourcesMapping objectForKey:timeout];
        if (source != nil) {
            dispatch_source_cancel(source);
        }
    };
}

- (id)setInterval {
    return ^(JSValue *function, JSValue *timeout, ...) {
        NSArray *originalArguments = [JSContext currentArguments];
        NSArray *arguments = @[];
        if ([originalArguments count] > 2) {
            arguments = [originalArguments subarrayWithRange:NSMakeRange(2,
                    [originalArguments count] - 2)];
        }
        // JSManagedValue * func = [[JSManagedValue alloc] initWithValue:function];
        return [self intervalWithCallable:function
                                  timeout:timeout
                                arguments:arguments
                               isInterval:YES];
    };
}


@end
