//
//  ViewController.m
//  iModelJsApp
//
//  Created by Satyakam Khadilkar on 4/5/18.
//  Copyright © 2018 Bentley Systems. All rights reserved.
//

#import "ViewController.h"
#import  <IModelJsHost/IModelJsHost.h>
#import "AssetHandler.h"
#import "AppDelegate.h"

/*! @brief The OIDC issuer from which the configuration will be discovered.
 */
static NSString *const kIssuer = @"https://qa-imsoidc.bentley.com";
static NSString *const kClientID = @"native-iWIy7d3SeTruxXF4zyYpS1hSl";
static NSString *const kRedirectURI = @"imodeljs://app/signin-redirect";
static NSString *const kAppAuthStateKey = @"authState";
static NSString *const kResponseType = @"code id_token";
static NSString *const kLocalStorageKey = @"ios:oidc_info";
static NSString *const kAuthStateKey = @"oidc_info";
static NSArray  *const kScopes = @[OIDScopeOpenID,
                                   OIDScopeProfile,
                                   OIDScopeEmail,
                                   @"organization",
                                   @"imodelhub",
                                   @"reality-data:read",
                                   @"context-registry-service",
                                   @"imodeljs-router",
                                   @"visible-api-scope",
                                   @"offline_access"];

@interface ViewController () <OIDAuthStateChangeDelegate, OIDAuthStateErrorDelegate, WKScriptMessageHandler>
- (void)performTokenExchange: (OIDAuthState*) authState :(NSError *) error;
@end

@implementation ViewController
WKWebView* _appWebView;
/*! @brief setup view
 @remarks Setup view and configure it
 */
- (void) loadView {
    WKWebViewConfiguration* webConfiguration = [[WKWebViewConfiguration alloc] init];
    
    // Setup message handler for javascript side OIDC client
    WKUserContentController* contentController = [WKUserContentController new];
    [contentController addScriptMessageHandler:self name:@"signIn"];
    [contentController addScriptMessageHandler:self name:@"signOut"];
    [contentController addScriptMessageHandler:self name:@"refreshToken"];
    [webConfiguration setUserContentController:contentController];

    // Allow local file access
    @try {[webConfiguration.preferences setValue:@YES forKey:@"allowFileAccessFromFileURLs"];} @catch (NSException *exception) {}
    @try {[webConfiguration setValue:@YES forKey:@"allowUniversalAccessFromFileURLs"];} @catch (NSException *exception) {}
    
    // Initialize assest handler and view
    AssetHandler* handler = [[AssetHandler alloc] initWithAssetPathAndDefaultPage:@"Assets/webresources" :@"index.html"];
    [webConfiguration setURLSchemeHandler:handler forURLScheme:@"imodeljs"];
    _appWebView  = [[WKWebView alloc] initWithFrame:CGRectZero configuration:webConfiguration];
    [_appWebView  setUIDelegate:self];
    [_appWebView  setNavigationDelegate:self];
    [_appWebView .scrollView.pinchGestureRecognizer setEnabled:NO];
    [_appWebView .scrollView setScrollEnabled:NO];
    [self setView:_appWebView ];
}

/*! @brief OIDC auth state change event
 @remarks Send the token to webview
 */
- (void)didChangeState:(OIDAuthState *)state {
      [self logMessage:@"State changed"];
}

/*! @brief Handle OIDC error
 @remarks OIDC error may be show error reload webview
 */
- (void)authState:(OIDAuthState *)state didEncounterAuthorizationError:(nonnull NSError *)error {
    [self logMessage:@"Received authorization error: %@", error];
}

/*! @brief Load app in webview
 @remarks Frontend of the app is loaded into webview with the port
 */
- (void) clearLoginAndLoadApp {
    NSString* customURL = [NSString stringWithFormat:@"imodeljs://app#%u", [[IModelJsHost sharedInstance] getPort]];
    NSURL* indexURL = [[NSURL alloc] initWithString:customURL];
    NSURLRequest* request = [NSURLRequest requestWithURL:indexURL];
    [_appWebView loadRequest:request];
}
/*! @brief OidcIosClient handle this event
 @remarks This will propagate the login event back to javascript base frontend in webview
 */
- (void) notifyApp {
    [_appWebView evaluateJavaScript:@"window.notifyOidcClient();"
                   completionHandler:^(id _Nullable, NSError * _Nullable error) {
        NSLog(@"Notified webview");
     }];
}
/*! @brief Saves the @c OIDAuthState to @c NSUSerDefaults.
 */
- (void)saveState {
    // for production usage consider using the OS Keychain instead
    NSError* err;
    NSData* archivedAuthState = [NSKeyedArchiver archivedDataWithRootObject:_authState requiringSecureCoding:YES error:&err];
    [[NSUserDefaults standardUserDefaults] setObject:archivedAuthState forKey:kAuthStateKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

/*! @brief Loads the @c OIDAuthState from @c NSUSerDefaults.
 */
- (void)loadState {
    NSData *archivedAuthState = [[NSUserDefaults standardUserDefaults] objectForKey:kAuthStateKey];
    OIDAuthState *authState  = [NSKeyedUnarchiver unarchiveObjectWithData:archivedAuthState];
        [self setAuthState:authState];
}

- (void)setAuthState:(nullable OIDAuthState *)authState {
    if (_authState == authState) {
        return;
    }
    _authState = authState;
    _authState.stateChangeDelegate = self;
    [self stateChanged];
}

- (void)stateChanged {
    [self saveState];
}
/*! @brief update token information into webview main window localstorage
 @remarks This should be done before notifyApp
 */
- (void) updateWebViewLocalStorage :(OIDAuthState*)authState :(NSString*)userInfo {
    NSMutableString* str = [NSMutableString new];
    [str appendFormat:@"window.localStorage.setItem(\"%@\", JSON.stringify({", kLocalStorageKey];
    if (authState.lastTokenResponse.scope)
        [str appendFormat:@"\"scope\":\"%@\",", authState.scope];
    if (authState.lastTokenResponse.idToken)
        [str appendFormat:@"\"id_token\":\"%@\",", authState.lastTokenResponse.idToken];
    if (authState.lastTokenResponse.accessToken)
        [str appendFormat:@"\"access_token\":\"%@\",", authState.lastTokenResponse.accessToken];
    if (authState.lastTokenResponse.refreshToken)
        [str appendFormat:@"\"refresh_token\":\"%@\",", authState.lastTokenResponse.refreshToken];
    if (authState.lastTokenResponse.tokenType)
        [str appendFormat:@"\"token_type\":\"%@\",", authState.lastTokenResponse.tokenType];
    if (authState.lastTokenResponse.accessTokenExpirationDate) {
        NSDate* now = [NSDate new];
        NSTimeInterval expiresIn = [authState.lastTokenResponse.accessTokenExpirationDate timeIntervalSinceDate:now] * 1000;
        NSTimeInterval expiresAt = [now timeIntervalSince1970]* 1000;
        [str appendFormat:@"\"expires_in\":%f,", floor(expiresIn)];
        [str appendFormat:@"\"expires_at\":%f,", floor(expiresAt)];
    }
    if (userInfo)
        [str appendFormat:@"\"user_info\":%@", userInfo];

    [str appendString:@"}));"];
    [_appWebView evaluateJavaScript:str completionHandler:^(id _Nullable, NSError * _Nullable error) {
        NSLog(@"Reloading page after setting OIDC info");
        [self notifyApp];
    }];
}

/*! @brief perform token exchange
 @remarks At this point we do need to verify token_id
 */
- (void)performTokenExchange: (OIDAuthState*) authState :(NSError *) error {
    if (authState) {
        // [authState.lastTokenResponse.idToken]
        OIDAuthorizationResponse* authorizationResponse = authState.lastAuthorizationResponse;
        OIDTokenRequest *tokenExchangeRequest =
        [authorizationResponse tokenExchangeRequest];
        [OIDAuthorizationService performTokenRequest:tokenExchangeRequest
                       originalAuthorizationResponse:authorizationResponse
                                            callback:^(OIDTokenResponse *_Nullable tokenResponse,
                                                       NSError *_Nullable tokenError) {
                                                OIDAuthState *authState;
                                                if (tokenResponse) {
                                                    authState = [[OIDAuthState alloc]
                                                                 initWithAuthorizationResponse:
                                                                 authorizationResponse
                                                                 tokenResponse:tokenResponse];
                                                }
                                                [self setAuthState:authState];
                                                [self getUserInfo];
                                            }];
    } else {
        [self logMessage:@"Authorization error: %@", [error localizedDescription]];
        [self setAuthState:nil];
    }
}

/*! @brief Perform code exchange.
 @remarks This is the first step to get initial code auth
 */
- (void)doAuthWithAutoCodeExchange:(OIDServiceConfiguration *)configuration
                          clientID:(NSString *)clientID
                      clientSecret:(NSString *)clientSecret {
    NSURL *redirectURI = [NSURL URLWithString:kRedirectURI];
    // builds authentication request
    OIDAuthorizationRequest *request =
    [[OIDAuthorizationRequest alloc] initWithConfiguration:configuration
                                                  clientId:clientID
                                              clientSecret:clientSecret
                                                    scopes:kScopes
                                               redirectURL:redirectURI
                                              responseType:kResponseType
                                      additionalParameters:nil];
    // performs authentication request
    AppDelegate *appDelegate = (AppDelegate *) [UIApplication sharedApplication].delegate;
    [self logMessage:@"Initiating authorization request with scope: %@", request.scope];
    appDelegate.currentAuthorizationFlow =
    [OIDAuthState authStateByPresentingAuthorizationRequest:request
                                   presentingViewController:self
                                                   callback:^(OIDAuthState *_Nullable tokenResponse,
                                                              NSError *_Nullable tokenError) {
                                                       [self performTokenExchange:tokenResponse :tokenError];
                                                   }];
}

/*! @brief Handle OIDC error
 @remarks OIDC error may be show error reload webview
 */
- (void)discoverConfigAndInitiateSigin {
    NSURL *issuer = [NSURL URLWithString:kIssuer];
    
    [self logMessage:@"Fetching configuration for issuer: %@", issuer];
    
    // discovers endpoints
    [OIDAuthorizationService
     discoverServiceConfigurationForIssuer:issuer
                                completion:^(OIDServiceConfiguration *_Nullable configuration, NSError *_Nullable error) {
                                    if (!configuration) {
                                        [self logMessage:@"Error retrieving discovery document: %@", [error localizedDescription]];
                                        [self setAuthState:nil];
                                        return;
                                    }
                                    
                                    [self logMessage:@"Got configuration: %@", configuration];
                                    [self doAuthWithAutoCodeExchange:configuration clientID:kClientID clientSecret:nil];
                                }];
}

/*! @brief Load app after view is loaded
 */
- (void)viewDidLoad {
    [super viewDidLoad];
    [self loadState];
    [self clearLoginAndLoadApp];
 }

/*! @brief handle memory warning
 */
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)webView:(WKWebView *)webView
didReceiveServerRedirectForProvisionalNavigation:(WKNavigation *)navigation {
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
    decisionHandler(WKNavigationActionPolicyAllow);
}

- (void)logMessage:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2) {
    // gets message as string
    va_list argp;
    va_start(argp, format);
    NSString *log = [[NSString alloc] initWithFormat:format arguments:argp];
    va_end(argp);
    
    // outputs to stdout
    NSLog(@"%@", log);
    /*
    // appends to output log
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    dateFormatter.dateFormat = @"hh:mm:ss";
    NSString *dateString = [dateFormatter stringFromDate:[NSDate date]];
    _logTextView.text = [NSString stringWithFormat:@"%@%@%@: %@",
                         _logTextView.text,
                         ([_logTextView.text length] > 0) ? @"\n" : @"",
                         dateString,
                         log];
     */
}

- (void)userContentController:(nonnull WKUserContentController *)userContentController didReceiveScriptMessage:(nonnull WKScriptMessage *)message {
    if ([message.name compare:@"signIn" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
        if ([_authState isAuthorized]) {
            [self getUserInfo];
        } else {
            [self discoverConfigAndInitiateSigin];
        }
    } else if ([message.name compare:@"signOut" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
            
    }
}

/*! @brief get userinfo from user endpoint
 @remarks update this everytime. This also refresh token
 */
- (void) getUserInfo {
    NSURL *userinfoEndpoint =
    _authState.lastAuthorizationResponse.request.configuration.discoveryDocument.userinfoEndpoint;
    if (!userinfoEndpoint) {
        [self logMessage:@"Userinfo endpoint not declared in discovery document"];
        return;
    }
    NSString *currentAccessToken = _authState.lastTokenResponse.accessToken;
    
    [self logMessage:@"Performing userinfo request"];
    __block typeof(_authState) authState = _authState;
    [_authState performActionWithFreshTokens:^(NSString *_Nullable accessToken,
                                               NSString *_Nullable idToken,
                                               NSError *_Nullable error) {
        if (error) {
            [self logMessage:@"Error fetching fresh tokens: %@", [error localizedDescription]];
            return;
        }
        
        // log whether a token refresh occurred
        if (![currentAccessToken isEqual:accessToken]) {
            [self logMessage:@"Access token was refreshed automatically (%@ to %@)",
             currentAccessToken,
             accessToken];
        } else {
            [self logMessage:@"Access token was fresh and not updated [%@]", accessToken];
        }
        [self stateChanged];
        // creates request to the userinfo endpoint, with access token in the Authorization header
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:userinfoEndpoint];
        NSString *authorizationHeaderValue = [NSString stringWithFormat:@"Bearer %@", accessToken];
        [request addValue:authorizationHeaderValue forHTTPHeaderField:@"Authorization"];
        
        NSURLSessionConfiguration *configuration =
        [NSURLSessionConfiguration defaultSessionConfiguration];
        NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration
                                                              delegate:nil
                                                         delegateQueue:nil];
        
        // performs HTTP request
        NSURLSessionDataTask *postDataTask =
        [session dataTaskWithRequest:request
                   completionHandler:^(NSData *_Nullable data,
                                       NSURLResponse *_Nullable response,
                                       NSError *_Nullable error) {
                       dispatch_async(dispatch_get_main_queue(), ^() {
                           if (error) {
                               [self logMessage:@"HTTP request failed %@", error];
                               return;
                           }
                           if (![response isKindOfClass:[NSHTTPURLResponse class]]) {
                               [self logMessage:@"Non-HTTP response"];
                               return;
                           }
                           
                           NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                           id jsonDictionaryOrArray =
                           [NSJSONSerialization JSONObjectWithData:data options:0 error:NULL];
                           
                           if (httpResponse.statusCode != 200) {
                               // server replied with an error
                               NSString *responseText = [[NSString alloc] initWithData:data
                                                                              encoding:NSUTF8StringEncoding];
                               if (httpResponse.statusCode == 401) {
                                   // "401 Unauthorized" generally indicates there is an issue with the authorization
                                   // grant. Puts OIDAuthState into an error state.
                                   NSError *oauthError =
                                   [OIDErrorUtilities resourceServerAuthorizationErrorWithCode:0
                                                                                 errorResponse:jsonDictionaryOrArray
                                                                               underlyingError:error];
                                   [authState updateWithAuthorizationError:oauthError];
                                   // log error
                                   [self logMessage:@"Authorization Error (%@). Response: %@", oauthError, responseText];
                               } else {
                                   [self logMessage:@"HTTP: %d. Response: %@",
                                    (int)httpResponse.statusCode,
                                    responseText];
                               }
                               return;
                           }
                           [self updateWebViewLocalStorage:authState :[[NSString alloc] initWithData:data
                                                                          encoding:NSUTF8StringEncoding]];
                           // success response
                           [self logMessage:@"Success: %@", jsonDictionaryOrArray];
                       });
                   }];
        
        [postDataTask resume];
    }];
}

@end
