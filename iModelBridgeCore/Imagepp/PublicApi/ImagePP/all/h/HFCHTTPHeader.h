//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHTTPHeader.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFCHTTPHeader
//-----------------------------------------------------------------------------

#pragma once

class HFCHTTPHeader
    {
public:

    // Enums
    enum HHTTPStatus
        {
        _100_CONTINUE,
        _101_SWITCHING_PROTOCOLS,
        _200_OK,
        _201_CREATED,
        _202_ACCEPTED,
        _203_NONAUTH_INFORMATION,
        _204_NO_CONTENT,
        _205_RESET_CONTENT,
        _206_PARTIAL_CONTENT,
        _300_MULTIPLE_CHOICES,
        _301_MOVED_PERMANENTLY,
        _302_FOUND,
        _303_SEE_OTHER,
        _304_NOT_MODIFIED,
        _305_USE_PROXY,
        _307_TEMP_REDIRECT,
        _400_BAD_REQUEST,
        _401_UNAUTHORIZED,
        _402_PAYMENT_REQUIRED,
        _403_FORBIDDEN,
        _404_NOT_FOUND,
        _405_METHOD_NOTALLOWED,
        _406_NOT_ACCEPTABLE,
        _407_PROXY_AUTH_REQUIRED,
        _408_REQUEST_TIMEOUT,
        _409_CONFLICT,
        _410_GONE,
        _411_LENGTH_REQUIRED,
        _412_PRECONDITION_FAILED,
        _413_REQ_ENTITY_TOO_LARGE,
        _414_REQ_URI_TOO_LONG,
        _415_UNSUPPORTED_MEDIA_TYPE,
        _416_REQ_RANGE_NOT_SATISFIABLE,
        _417_EXPECTATION_FAILED,
        _426_UPGRADE_REQUIRED,
        _500_INTERNAL_SERVER_ERROR,
        _501_NOT_IMPLEMENTED,
        _502_BAD_GATEWAY,
        _503_SERVICE_UNAVAILABLE,
        _504_GATEWAY_TIMEOUT,
        _505_VERSION_NOT_SUPPORTED,
        _MAX_STATUS_CODE
        };

    // Constructor - Destructor
    HFCHTTPHeader(double pi_Version = 1.0);
    HFCHTTPHeader(const HFCHTTPHeader& pi_rHeader);
    virtual     ~HFCHTTPHeader();

    // Operations
    bool       SetStatusCode(HHTTPStatus pi_StatusCode);
    HHTTPStatus GetStatusCode() const;
    void        AddToHeader(const WString& pi_rHeader);
    void        ClearHeader();
    WString     GetHeader() const;
    string      GetUTF8Header() const;
    void        HeaderIsComplete(bool pi_Complete);

    WString     GetStatusCodeString() const;

protected:

private:

    // Not implemented
    HFCHTTPHeader& operator=(const HFCHTTPHeader&);

    // Attributes
    WString     m_Header;
    HHTTPStatus m_Status;
    double     m_Version;
    bool       m_HeaderComplete;
    };

