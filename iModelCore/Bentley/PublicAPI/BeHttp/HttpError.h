/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpError.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/HttpResponse.h>
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncError.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE HttpError : public Tasks::AsyncError
    {
private:
    ConnectionStatus m_connectionStatus;
    HttpStatus m_httpStatus;

    static Utf8String GetDescription(ConnectionStatus connectionStatus, HttpStatus httpStatus);
    static Utf8String GetMessage(ConnectionStatus connectionStatus, HttpStatus httpStatus);

public:
    BEHTTP_EXPORT HttpError();
    BEHTTP_EXPORT HttpError(HttpResponse httpResponse);
    BEHTTP_EXPORT HttpError(ConnectionStatus connectionStatus, HttpStatus httpStatus);

    BEHTTP_EXPORT ConnectionStatus GetConnectionStatus() const;
    BEHTTP_EXPORT HttpStatus GetHttpStatus() const;

    //! DEPRECATED - use GetMessage()
    BEHTTP_EXPORT virtual Utf8String GetDisplayMessage() const;
    //! DEPRECATED - use GetDescription()
    BEHTTP_EXPORT virtual Utf8String GetDisplayDescription () const;

    BEHTTP_EXPORT static Utf8String GetConnectionErrorDisplayMessage (ConnectionStatus connectionStatus);
    BEHTTP_EXPORT static Utf8String GetHttpDisplayMessage (HttpStatus httpStatus);
    };

typedef HttpError& HttpErrorR;
typedef const HttpError& HttpErrorCR;

END_BENTLEY_HTTP_NAMESPACE
