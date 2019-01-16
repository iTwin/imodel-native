//
//  XMLHttpRequest.h
//  TestApp
//
//  Created by Affan Khan on 11/6/18.
//  Copyright © 2018 Affan Khan. All rights reserved.
//

#ifndef XMLHttpRequest_h
#define XMLHttpRequest_h

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>

typedef NS_ENUM(NSUInteger , ReadyState) {
    UNSENT =0,    // open()has not been called yet.
    OPENED,        // send()has not been called yet.
    HEADERS_RECEIVED,      // RECEIVED    send() has been called, and headers and status are available.
    LOADING,      // Downloading; responseText holds partial data.
    DONE          // The operation is complete.
};
@protocol XMLHttpRequest<JSExport>
// An EventHandler that is called whenever the readyState attribute changes.
@property (nonatomic) JSValue* onload;
// An EventHandler that is called whenever the readyState attribute changes.
@property (nonatomic) JSValue* onerror;
// An EventHandler that is called when error is encounter
@property (nonatomic) JSValue* onreadystatechange;
// Returns an unsigned short, the state of the request.
@property (nonatomic) NSNumber* readyState;
// Returns an ArrayBuffer, Blob, Document, JavaScript object, or a DOMString, depending on the value of XMLHttpRequest.responseType. that contains the response entity body.
@property (nonatomic) JSValue* response;
// Returns a DOMString that contains the response to the request as text, or null if the request was unsuccessful or has not yet been sent.
@property (nonatomic) NSString* responseText;
// Is an enumerated value that defines the response type.
@property (nonatomic) NSString* responseType;
// Returns the serialized URL of the response or the empty string if the URL is null.
@property (nonatomic) NSString* responseURL;
// Returns an unsigned short with the status of the response of the request.
@property (nonatomic) NSNumber* status;
// Returns a DOMString containing the response string returned by the HTTP server. Unlike XMLHTTPRequest.status, this includes the entire text of the response message ("200 OK", for example).
@property (nonatomic) NSString* statusText;
// Is an unsigned long representing the number of milliseconds a request can take before automatically being terminated.
@property (nonatomic) NSNumber*  timeout;
// Is an EventHandler that is called whenever the request times out.
@property (nonatomic) JSValue* ontimeout;
// Aborts the request if it has already been sent.
- (void) abort;
// Returns all the response headers, separated by CRLF, as a string, or null if no response has been received.
- (NSString*) getResponseHeader:(NSString*)header;
// Initializes a request. This method is to be used from JavaScript code; to initialize a request from native code, use openRequest() instead.
- (void) open:(JSValue*)method
             :(JSValue*)url
             :(JSValue*)async
             :(JSValue*)user
             :(JSValue*)password;

// Overrides the MIME type returned by the server.
- (void) overrideMimeType:(NSString*)mimeType;
// Sends the request. If the request is asynchronous (which is the default), this method returns as soon as the request is sent.
- (void) send:(JSValue*)body;
// Sets the value of an HTTP request header. You must call setRequestHeader()after open(), but before send().
- (void) setRequestHeader :(NSString*)header
                          :(NSString*)value;
//Url file
- (void)downloadFile :(NSString*)fileName;
- (NSString *)getAllResponseHeaders;
@end


@interface XMLHttpRequest : NSObject <XMLHttpRequest>

- (instancetype)initWithURLSession: (NSURLSession *)urlSession;
- (void)extend:(id)jsContext;

@end

#endif /* XMLHttpRequest_h */
