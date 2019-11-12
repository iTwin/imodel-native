/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/HttpResponse.h>
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncError.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE HttpError : Tasks::AsyncError
{
private:
    ConnectionStatus m_connectionStatus;
    HttpStatus m_httpStatus;

private:
    static Utf8String CreateMessage(ConnectionStatus connectionStatus, HttpStatus httpStatus, ResponseCP response);
    static Utf8String CreateDescription(ConnectionStatus connectionStatus, HttpStatus httpStatus);
    static Utf8String GetHttpDisplayMessage(HttpStatus httpStatus, const Response* response);
    static Utf8String GetConnectionErrorDisplayMessage(ConnectionStatus connectionStatus);
        
public:
    //! Create invalid error
    BEHTTP_EXPORT HttpError();
    //! Create error based on information in response
    BEHTTP_EXPORT HttpError(Response httpResponse);
    //! Create error based on status, may include less information than response
    BEHTTP_EXPORT HttpError(ConnectionStatus connectionStatus, HttpStatus httpStatus);

    //! Check if error is properly set
    BEHTTP_EXPORT bool IsValid() const;

    ConnectionStatus GetConnectionStatus() const {return m_connectionStatus;}
    HttpStatus GetHttpStatus() const {return m_httpStatus;}

    //! DEPRECATED - use GetMessage()
    virtual Utf8String GetDisplayMessage() const {return m_message;}
    //! DEPRECATED - use GetDescription()
    virtual Utf8String GetDisplayDescription() const {return m_description;}
    };

typedef HttpError& HttpErrorR;
typedef const HttpError& HttpErrorCR;

END_BENTLEY_HTTP_NAMESPACE 
