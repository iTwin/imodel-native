//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCallbacks.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCCallback.h"
#include <ImagePP/h/HUncopyable.h>

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// class HFCAuthenticationError
//
// Base class for an authentication error type. An authentication error is
// always associated with an exception id. Classes of the authentication
// error hierarchy should provide a way to generate a textual error message.
//-----------------------------------------------------------------------------
class HFCAuthenticationError : public HFCShareableObject<HFCAuthenticationError>
    {
public:
    IMAGEPP_EXPORT virtual              ~HFCAuthenticationError        ();

    IMAGEPP_EXPORT WString              ToString                       () const;
    IMAGEPP_EXPORT void                 Throw                          () const;

protected:
    IMAGEPP_EXPORT explicit             HFCAuthenticationError         ();

private:
    virtual WString             _ToString                      () const = 0;
    virtual void                _Throw                         () const = 0;

    };


struct HFCAuthentication
    {
    HDECLARE_BASECLASS_ID(HFCAuthenticationId_Base)

public:
    virtual                                 ~HFCAuthentication     ();

    IMAGEPP_EXPORT virtual void                     SetByString            (const WString&              pi_rAuthenticationString) = 0;

    IMAGEPP_EXPORT bool                            Failed                 () const;
    IMAGEPP_EXPORT size_t                           GetErrorsQty           () const;
    IMAGEPP_EXPORT const HFCAuthenticationError&    GetLastError           () const;
    IMAGEPP_EXPORT void                             PopLastError           ();
    IMAGEPP_EXPORT void                             ResetAllErrors         ();

    IMAGEPP_EXPORT void                             PushLastError          (const HFCPtr<HFCAuthenticationError>&
                                                                    pi_pLastError);

    IMAGEPP_EXPORT unsigned short                  IncrementRetryCount    ();
    IMAGEPP_EXPORT unsigned short                  GetRetryCount          () const;

protected:
    typedef stack<HFCPtr<HFCAuthenticationError> > ErrorStack;

    explicit                                HFCAuthentication      (const unsigned short       pi_RetryCount = 1);


private:
    virtual ErrorStack&                     _GetErrorStack         ()       {return m_LastErrors;};
    virtual const ErrorStack&               _GetErrorStack         () const {return m_LastErrors;};

    unsigned short                         m_RetryCount;
    ErrorStack                             m_LastErrors;

    };


class HFCAuthenticationCallback : public HFCCallback
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Callback, HFCCallback)

public:
    HFCAuthenticationCallback() {};
    virtual             ~HFCAuthenticationCallback() {};

    virtual bool       GetAuthentication (HFCAuthentication* pio_Authentication) const = 0;
    virtual unsigned short RetryCount(HCLASS_ID pi_AuthenticationType) const = 0;
    virtual bool       IsCancelled() const = 0;
    virtual bool       CanAuthenticate(HCLASS_ID pi_AuthenticationType) const = 0;

    //Static method used to get the authentication callback that can handle a particular
    //type of authentication.
    IMAGEPP_EXPORT static const HFCAuthenticationCallback* GetCallbackFromRegistry(
        HCLASS_ID pi_AuthenticationType);
    };

struct HFCInternetAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Internet, HFCAuthentication)

public:
    IMAGEPP_EXPORT explicit         HFCInternetAuthentication  (const WString&          pi_Server,
                                                        const WString&          pi_User = L"",
                                                        const WString&          pi_Password = L"",
                                                        const unsigned short   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCInternetAuthentication ();

    IMAGEPP_EXPORT virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    IMAGEPP_EXPORT void             SetUser                    (const WString&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const WString&          pi_Password);

    IMAGEPP_EXPORT const WString&   GetUser                    () const;
    IMAGEPP_EXPORT const WString&   GetPassword                () const;
    IMAGEPP_EXPORT const WString&   GetServer                  () const;

private:
    // authentication's members
    WString         m_User;
    WString         m_Password;
    WString         m_Server;
    };

struct HFCOracleAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Oracle, HFCAuthentication)

public:

    IMAGEPP_EXPORT explicit         HFCOracleAuthentication    (const unsigned short   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCOracleAuthentication   ();

    IMAGEPP_EXPORT virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    IMAGEPP_EXPORT const WString&   GetUser                    () const;
    IMAGEPP_EXPORT const WString&   GetPassword                () const;
    IMAGEPP_EXPORT const WString&   GetDatabaseName               () const;

    IMAGEPP_EXPORT const WString&   GetConnectionString        () const;

    IMAGEPP_EXPORT void             SetUser                    (const WString&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const WString&          pi_Password);
    IMAGEPP_EXPORT void             SetDatabaseName            (const WString&          pi_DbName);


private: 
    WString         m_ConnectionString;
    WString         m_User;
    WString         m_Password;
    WString         m_DbName;
    };

struct HFCProxyAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Proxy, HFCAuthentication)

public:
    IMAGEPP_EXPORT                  HFCProxyAuthentication     (const WString&          pi_User = L"",
                                                        const WString&          pi_Password = L"",
                                                        const unsigned short   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCProxyAuthentication    ();

    IMAGEPP_EXPORT virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    IMAGEPP_EXPORT void             SetUser                    (const WString&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const WString&          pi_Password);

    IMAGEPP_EXPORT const WString&   GetUser                    () const;
    IMAGEPP_EXPORT const WString&   GetPassword                () const;

private:
    // authentication's members
    WString         m_User;
    WString         m_Password;
    };


struct HFCPDFAuthentication : public HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_PDF, HFCAuthentication)

public:
    enum PasswordType
        {
        OPEN,
        PERMISSION
        };

    typedef uint32_t PDFError;

    IMAGEPP_EXPORT                  HFCPDFAuthentication       (const WString&          pi_FileName,
                                                        const PasswordType      pi_PasswordType,
                                                        const unsigned short   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCPDFAuthentication      ();

    IMAGEPP_EXPORT const WString&   GetFileName                () const;
    IMAGEPP_EXPORT PasswordType     GetPasswordType            () const;
    IMAGEPP_EXPORT const string&    GetPassword                () const;

    IMAGEPP_EXPORT void             SetPassword                (const string&           pi_rPassword);
    IMAGEPP_EXPORT virtual void     SetByString                (const WString&          pi_rAuthenticationString);


private:

    // Prevent copies of any kind
    HFCPDFAuthentication       (const HFCPDFAuthentication&);
    HFCPDFAuthentication&   operator=                  (const HFCPDFAuthentication&);


    WString         m_FileName;
    PasswordType    m_PasswordType;
    string          m_Password;
    };

END_IMAGEPP_NAMESPACE