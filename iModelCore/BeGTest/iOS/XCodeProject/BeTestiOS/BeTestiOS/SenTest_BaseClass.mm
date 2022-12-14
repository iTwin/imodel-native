#import "SenTest_BaseClass.h"
@implementation SenTest_BaseClass

+ (void) begtest_initialize {
    NSArray*  documentPaths  = NSSearchPathForDirectoriesInDomains (NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentDir    = [[documentPaths objectAtIndex:0] stringByAppendingPathComponent:@"Documents"];
    
    #ifdef COMMENT_OUT // *** No. We have some tests that have no documents
    // When running the tests on a simulator for the first time, XCode is not able to deploy the application data package
    // (you get a warning about that). Application data package contains test documents so the tests that depend on them fail.
    // If the app data package is deployed successfully, it always has the documents.manifest file. Here we check if it exists
    // and fail immediately if it doesn't. So if the tests fail on this assert (and you're running them for the first time
    // on a simulator), simply try to rerun them. 
    BeFileName documentsManifest (WString ([[documentPaths objectAtIndex:0] cStringUsingEncoding:NSUTF8StringEncoding], BentleyCharEncoding::Utf8).c_str ());
    documentsManifest.AppendToPath (L"documents.manifest");
    BeAssert (documentsManifest.DoesPathExist ());
    #endif
        
    NSString* bundleDir      = [[NSBundle mainBundle] resourcePath];
    NSString* tempDir        = NSTemporaryDirectory ();
    NSArray*  appSupportUrls = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    NSString* appBundleId    = [[NSBundle mainBundle] bundleIdentifier];
    NSURL*    appSupportUrl  = [[appSupportUrls objectAtIndex:0] URLByAppendingPathComponent:appBundleId];
    NSString* appSupportDir  = [appSupportUrl path];
    [[NSFileManager defaultManager] createDirectoryAtURL:appSupportUrl withIntermediateDirectories:YES attributes:nil error:nil];

    BeTest::Initialize (*BeTestHost::Create([documentDir    cStringUsingEncoding:NSUTF8StringEncoding],
                                            [bundleDir      cStringUsingEncoding:NSUTF8StringEncoding],
                                            [tempDir        cStringUsingEncoding:NSUTF8StringEncoding],
                                            [appSupportDir  cStringUsingEncoding:NSUTF8StringEncoding]));

    static bool s_cleaned;
    if (!s_cleaned)
        {
        // Delete any files that may be hanging around since the last run
        s_cleaned = true;
        BeFileName dirName;
        BeTest::GetHost().GetOutputRoot(dirName);
        BeFileName::EmptyDirectory(dirName);
        BeTest::GetHost().GetTempDir(dirName);
        BeFileName::EmptyDirectory(dirName);
        }

    BentleyApi::NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider (BentleyApi::NativeLogging::CONSOLE_LOGGING_PROVIDER); // send messages to syslog
    BentleyApi::NativeLogging::LoggingConfig::SetSeverity(L"ECDb", NativeLogging::LOG_FATAL); // ECDb error messages are so verbose as to significantly slow down the test run
    BentleyApi::NativeLogging::LoggingConfig::SetSeverity (L"TestRunner", NativeLogging::LOG_INFO);
    BentleyApi::NativeLogging::LoggingConfig::SetSeverity (L"Performance", NativeLogging::LOG_TRACE);
    
}

+ (void) begtest_uninitialize {
    BentleyApi::NativeLogging::LoggingConfig::DeactivateProvider ();
    BeTest::Uninitialize ();
}

@end
