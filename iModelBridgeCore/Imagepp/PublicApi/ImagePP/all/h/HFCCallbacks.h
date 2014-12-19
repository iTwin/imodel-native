//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCallbacks.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCCallback.h"

#include <ImagePP/h/HUncopyable.h>

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
    _HDLLu virtual              ~HFCAuthenticationError        ();

    _HDLLu WString              ToString                       () const;
    _HDLLu void                 Throw                          () const;
    _HDLLu ExceptionID          GetExceptionID                 () const;

protected:
    _HDLLu explicit             HFCAuthenticationError         (const ExceptionID   pi_ExceptionID);

private:
    virtual WString             _ToString                      () const = 0;
    virtual void                _Throw                         () const = 0;

    const ExceptionID           m_ExceptionID;
    };


struct HFCAuthentication
    {
    HDECLARE_BASECLASS_ID(1548)

public:
    virtual                                 ~HFCAuthentication     ();

    _HDLLu virtual void                     SetByString            (const WString&              pi_rAuthenticationString) = 0;

    _HDLLu bool                            Failed                 () const;
    _HDLLu size_t                           GetErrorsQty           () const;
    _HDLLu const HFCAuthenticationError&    GetLastError           () const;
    _HDLLu void                             PopLastError           ();
    _HDLLu void                             ResetAllErrors         ();

    _HDLLu void                             PushLastError          (const HFCPtr<HFCAuthenticationError>&
                                                                    pi_pLastError);

    _HDLLu unsigned short                  IncrementRetryCount    ();
    _HDLLu unsigned short                  GetRetryCount          () const;

protected:
    typedef stack<HFCPtr<HFCAuthenticationError> >
                                               ErrorStack;

    explicit                                HFCAuthentication      (const unsigned short       pi_RetryCount = 1);

    // TDORAY: put back in private section when possible -> Backward compatibility with dcartes 8.11.7...
public:
    unsigned short                         m_RetryCount;

    // TDORAY: this is a hack to enable having a supplementary member without breaking backward
    // compatibility with dcartes 8.11.7. Remove as it becomes possible and don't propagate to
    // beijing.
private:
    virtual ErrorStack&                     _GetErrorStack         () = 0;
    virtual const ErrorStack&               _GetErrorStack         () const = 0;
    };


class HFCAuthenticationCallback : public HFCCallback
    {
    HDECLARE_CLASS_ID(1547, HFCCallback)

public:
    HFCAuthenticationCallback() {};
    virtual             ~HFCAuthenticationCallback() {};

    virtual bool       GetAuthentication (HFCAuthentication* pio_Authentication) const = 0;
    virtual unsigned short RetryCount(HCLASS_ID pi_AuthenticationType) const = 0;
    virtual bool       IsCancelled() const = 0;
    virtual bool       CanAuthenticate(HCLASS_ID pi_AuthenticationType) const = 0;

    //Static method used to get the authentication callback that can handle a particular
    //type of authentication.
    _HDLLu static const HFCAuthenticationCallback* GetCallbackFromRegistry(
        HCLASS_ID pi_AuthenticationType);
    };

struct HFCInternetAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(1549, HFCAuthentication)

public:
    _HDLLu explicit         HFCInternetAuthentication  (const WString&          pi_Server,
                                                        const WString&          pi_User = L"",
                                                        const WString&          pi_Password = L"",
                                                        const unsigned short   pi_RetryCount = 0);
    _HDLLu virtual          ~HFCInternetAuthentication ();

    _HDLLu virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    _HDLLu void             SetUser                    (const WString&          pi_User);
    _HDLLu void             SetPassword                (const WString&          pi_Password);

    _HDLLu const WString&   GetUser                    () const;
    _HDLLu const WString&   GetPassword                () const;
    _HDLLu const WString&   GetServer                  () const;

private:
    // authentication's members
    WString         m_User;
    WString         m_Password;
    WString         m_Server;

    // TDORAY: this is a hack to enable having a supplementary member without breaking backward
    // compatibility with dcartes 8.11.7. Remove as it becomes possible and don't propagate to
    // beijing.
    ErrorStack                              m_LastErrors;
    virtual ErrorStack&                     _GetErrorStack         () override {return m_LastErrors;}
    virtual const ErrorStack&               _GetErrorStack         () const override {
        return m_LastErrors;
    }
    };

struct HFCOracleAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(1550, HFCAuthentication)

public:

    _HDLLu explicit         HFCOracleAuthentication    (const unsigned short   pi_RetryCount = 0);
    _HDLLu virtual          ~HFCOracleAuthentication   ();

    _HDLLu virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    _HDLLu const WString&   GetUser                    () const;
    _HDLLu const WString&   GetPassword                () const;
    _HDLLu const WString&   GetDatabaseName               () const;

    _HDLLu const WString&   GetConnectionString        () const;

    _HDLLu void             SetUser                    (const WString&          pi_User);
    _HDLLu void             SetPassword                (const WString&          pi_Password);
    _HDLLu void             SetDatabaseName            (const WString&          pi_DbName);


private: 
    WString         m_ConnectionString;
    WString         m_User;
    WString         m_Password;
    WString         m_DbName;

    // TDORAY: this is a hack to enable having a supplementary member without breaking backward
    // compatibility with dcartes 8.11.7. Remove as it becomes possible and don't propagate to
    // beijing.
    ErrorStack                              m_LastErrors;
private:
    virtual ErrorStack&                     _GetErrorStack         () override {return m_LastErrors;}
    virtual const ErrorStack&               _GetErrorStack         () const override {
        return m_LastErrors;
    }
    };

struct HFCProxyAuthentication : HFCAuthentication
    {
    HDECLARE_CLASS_ID(1553, HFCAuthentication)

public:
    _HDLLu                  HFCProxyAuthentication     (const WString&          pi_User = L"",
                                                        const WString&          pi_Password = L"",
                                                        const unsigned short   pi_RetryCount = 0);
    _HDLLu virtual          ~HFCProxyAuthentication    ();

    _HDLLu virtual void     SetByString                (const WString&          pi_rAuthenticationString);

    _HDLLu void             SetUser                    (const WString&          pi_User);
    _HDLLu void             SetPassword                (const WString&          pi_Password);

    _HDLLu const WString&   GetUser                    () const;
    _HDLLu const WString&   GetPassword                () const;

private:
    // authentication's members
    WString         m_User;
    WString         m_Password;

    // TDORAY: this is a hack to enable having a supplementary member without breaking backward
    // compatibility with dcartes 8.11.7. Remove as it becomes possible and don't propagate to
    // beijing.
    ErrorStack                              m_LastErrors;
    virtual ErrorStack&                     _GetErrorStack         () override {return m_LastErrors;}
    virtual const ErrorStack&               _GetErrorStack         () const override {
        return m_LastErrors;
    }
    };


struct HFCPDFAuthentication : public HFCAuthentication
    {
    HDECLARE_CLASS_ID(1551, HFCAuthentication)

public:
    enum PasswordType
        {
        OPEN,
        PERMISSION
        };

    typedef uint32_t PDFError;

    _HDLLu                  HFCPDFAuthentication       (const WString&          pi_FileName,
                                                        const PasswordType      pi_PasswordType,
                                                        const unsigned short   pi_RetryCount = 0);
    _HDLLu virtual          ~HFCPDFAuthentication      ();

    _HDLLu const WString&   GetFileName                () const;
    _HDLLu PasswordType     GetPasswordType            () const;
    _HDLLu const string&    GetPassword                () const;

    _HDLLu void             SetPassword                (const string&           pi_rPassword);
    _HDLLu virtual void     SetByString                (const WString&          pi_rAuthenticationString);


private:

    // Prevent copies of any kind
    HFCPDFAuthentication       (const HFCPDFAuthentication&);
    HFCPDFAuthentication&   operator=                  (const HFCPDFAuthentication&);


    WString         m_FileName;
    PasswordType    m_PasswordType;
    string          m_Password;

    // TDORAY: this is a hack to enable having a supplementary member without breaking backward
    // compatibility with dcartes 8.11.7. Remove as it becomes possible and don't propagate to
    // beijing.
    ErrorStack                              m_LastErrors;
    virtual ErrorStack&                     _GetErrorStack         () override {return m_LastErrors;}
    virtual const ErrorStack&               _GetErrorStack         () const override {
        return m_LastErrors;
    }
    };
