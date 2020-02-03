//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCCallback.h"

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

    IMAGEPP_EXPORT Utf8String              ToString                       () const;
    IMAGEPP_EXPORT void                 Throw                          () const;

protected:
    IMAGEPP_EXPORT explicit             HFCAuthenticationError         ();

private:
    virtual Utf8String             _ToString                      () const = 0;
    virtual void                _Throw                         () const = 0;

    };


struct HFCAuthentication
    {
    HDECLARE_BASECLASS_ID(HFCAuthenticationId_Base)

public:
    virtual                                 ~HFCAuthentication     ();

    virtual void                            SetByString            (const Utf8String&              pi_rAuthenticationString) = 0;

    IMAGEPP_EXPORT bool                            Failed                 () const;
    IMAGEPP_EXPORT size_t                           GetErrorsQty           () const;
    IMAGEPP_EXPORT const HFCAuthenticationError&    GetLastError           () const;
    IMAGEPP_EXPORT void                             PopLastError           ();
    IMAGEPP_EXPORT void                             ResetAllErrors         ();

    IMAGEPP_EXPORT void                             PushLastError          (const HFCPtr<HFCAuthenticationError>&
                                                                    pi_pLastError);

    IMAGEPP_EXPORT uint16_t                  IncrementRetryCount    ();
    IMAGEPP_EXPORT uint16_t                  GetRetryCount          () const;

protected:
    typedef stack<HFCPtr<HFCAuthenticationError> > ErrorStack;

    explicit                                HFCAuthentication      (const uint16_t       pi_RetryCount = 1);


private:
    virtual ErrorStack&                     _GetErrorStack         ()       {return m_LastErrors;};
    virtual const ErrorStack&               _GetErrorStack         () const {return m_LastErrors;};

    uint16_t                         m_RetryCount;
    ErrorStack                             m_LastErrors;

    };


class HFCAuthenticationCallback : public HFCCallback
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Callback, HFCCallback)

public:
    HFCAuthenticationCallback() {};
    virtual             ~HFCAuthenticationCallback() {};

    virtual bool       GetAuthentication (HFCAuthentication* pio_Authentication) const = 0;
    virtual uint16_t RetryCount(HCLASS_ID pi_AuthenticationType) const = 0;
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
    IMAGEPP_EXPORT explicit         HFCInternetAuthentication  (const Utf8String&          pi_Server,
                                                        const Utf8String&          pi_User = "",
                                                        const Utf8String&          pi_Password = "",
                                                        const uint16_t   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCInternetAuthentication ();

    IMAGEPP_EXPORT void     SetByString                (const Utf8String&          pi_rAuthenticationString) override;

    IMAGEPP_EXPORT void             SetUser                    (const Utf8String&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const Utf8String&          pi_Password);

    IMAGEPP_EXPORT const Utf8String&   GetUser                    () const;
    IMAGEPP_EXPORT const Utf8String&   GetPassword                () const;
    IMAGEPP_EXPORT const Utf8String&   GetServer                  () const;
    
private:
    // authentication's members
    Utf8String         m_User;
    Utf8String         m_Password;
    Utf8String         m_Server;
    };

struct HFCOracleAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Oracle, HFCAuthentication)

public:

    IMAGEPP_EXPORT explicit         HFCOracleAuthentication    (const uint16_t   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCOracleAuthentication   ();

    IMAGEPP_EXPORT void     SetByString                (const Utf8String&          pi_rAuthenticationString) override;

    IMAGEPP_EXPORT const Utf8String&   GetUser                    () const;
    IMAGEPP_EXPORT const Utf8String&   GetPassword                () const;
    IMAGEPP_EXPORT const Utf8String&   GetDatabaseName               () const;

    IMAGEPP_EXPORT const Utf8String&   GetConnectionString        () const;

    IMAGEPP_EXPORT void             SetUser                    (const Utf8String&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const Utf8String&          pi_Password);
    IMAGEPP_EXPORT void             SetDatabaseName            (const Utf8String&          pi_DbName);


private: 
    Utf8String         m_ConnectionString;
    Utf8String         m_User;
    Utf8String         m_Password;
    Utf8String         m_DbName;
    };

struct HFCProxyAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(HFCAuthenticationId_Proxy, HFCAuthentication)

public:
    IMAGEPP_EXPORT                  HFCProxyAuthentication     (const Utf8String&          pi_User = "",
                                                        const Utf8String&          pi_Password = "",
                                                        const uint16_t   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCProxyAuthentication    ();

    IMAGEPP_EXPORT void     SetByString                (const Utf8String&          pi_rAuthenticationString) override;

    IMAGEPP_EXPORT void             SetUser                    (const Utf8String&          pi_User);
    IMAGEPP_EXPORT void             SetPassword                (const Utf8String&          pi_Password);
    IMAGEPP_EXPORT void             SetServer                  (const Utf8String&          pi_Server);

    IMAGEPP_EXPORT const Utf8String&   GetUser                    () const;
    IMAGEPP_EXPORT const Utf8String&   GetPassword                () const;
    IMAGEPP_EXPORT const Utf8String&   GetServer() const;
    
private:
    // authentication's members
    Utf8String         m_User;
    Utf8String         m_Password;
    Utf8String         m_Server;
    };

struct HFCCertificateAutoritiesAuthentication : HFCAuthentication
{
    HDECLARE_CLASS_ID(HFCAuthenticationId_CertificateAutorities, HFCAuthentication)

public:
    IMAGEPP_EXPORT                  HFCCertificateAutoritiesAuthentication();
    IMAGEPP_EXPORT virtual          ~HFCCertificateAutoritiesAuthentication();    
    
    IMAGEPP_EXPORT void              SetCertificateAuthFileUrl(const Utf8String& pi_CertificateAuthFileUrl);
    IMAGEPP_EXPORT const Utf8String& GetCertificateAuthFileUrl() const;

    IMAGEPP_EXPORT virtual void      SetByString(const Utf8String& pi_rAuthenticationString) override { SetCertificateAuthFileUrl(pi_rAuthenticationString); }
    
private:
    // authentication's members
    Utf8String         m_CertificateAuthFileUrl;
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

    IMAGEPP_EXPORT                  HFCPDFAuthentication       (const Utf8String&          pi_FileName,
                                                        const PasswordType      pi_PasswordType,
                                                        const uint16_t   pi_RetryCount = 0);
    IMAGEPP_EXPORT virtual          ~HFCPDFAuthentication      ();

    IMAGEPP_EXPORT const Utf8String&   GetFileName                () const;
    IMAGEPP_EXPORT PasswordType     GetPasswordType            () const;
    IMAGEPP_EXPORT const string&    GetPassword                () const;

    IMAGEPP_EXPORT void             SetPassword                (const string&           pi_rPassword);
    IMAGEPP_EXPORT void     SetByString                (const Utf8String&          pi_rAuthenticationString) override;


private:

    // Prevent copies of any kind
    HFCPDFAuthentication       (const HFCPDFAuthentication&);
    HFCPDFAuthentication&   operator=                  (const HFCPDFAuthentication&);


    Utf8String         m_FileName;
    PasswordType    m_PasswordType;
    string          m_Password;
    };

END_IMAGEPP_NAMESPACE
