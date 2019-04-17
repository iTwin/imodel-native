//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCCallbackRegistry.h>
#include <ImagePP/all/h/HFCCallbacks.h>



//-----------------------------------------------------------------------------
// class HFCAuthenticationCallback
//-----------------------------------------------------------------------------
const HFCAuthenticationCallback* HFCAuthenticationCallback::GetCallbackFromRegistry(
    HCLASS_ID pi_AuthenticationType)
    {
    HFCAuthenticationCallback* pRetCallback = 0;

    uint16_t NbCallbacks = HFCCallbackRegistry::GetInstance()->
                          GetNbCallbacks(HFCAuthenticationCallback::CLASS_ID);

    for (uint16_t CallbackInd = 0; CallbackInd < NbCallbacks; CallbackInd++)
        {
        pRetCallback = (HFCAuthenticationCallback*)HFCCallbackRegistry::GetInstance()->
                       GetCallback(HFCAuthenticationCallback::CLASS_ID,
                                   CallbackInd);

        HASSERT(pRetCallback != 0);

        if (pRetCallback->CanAuthenticate(pi_AuthenticationType) == true)
            {
            break;
            }
        else
            {
            pRetCallback = 0;
            }
        }

    return pRetCallback;
    }



//-----------------------------------------------------------------------------
// class HFCAuthenticationError
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Protected
// constructor
//-----------------------------------------------------------------------------
HFCAuthenticationError::HFCAuthenticationError ()
    {

    };

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCAuthenticationError::~HFCAuthenticationError ()
    {

    };

//-----------------------------------------------------------------------------
// Public
// Returns the authentication message
//-----------------------------------------------------------------------------
Utf8String HFCAuthenticationError::ToString () const
    {
    return _ToString();
    }

//-----------------------------------------------------------------------------
// Public
// Rethrow underlying exception
//-----------------------------------------------------------------------------
void HFCAuthenticationError::Throw () const
    {
    _Throw();
    }




//-----------------------------------------------------------------------------
// class HFCAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected
// constructor
//-----------------------------------------------------------------------------
HFCAuthentication::HFCAuthentication(const uint16_t pi_RetryCount)
    : m_RetryCount(pi_RetryCount)
    {

    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCAuthentication::~HFCAuthentication()
    {

    }

//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
uint16_t HFCAuthentication::IncrementRetryCount()
    {
    return ++m_RetryCount;
    };

uint16_t HFCAuthentication::GetRetryCount() const
    {
    return m_RetryCount;
    }

//-----------------------------------------------------------------------------
// Public
// Returns if there was an error in the authentication process
//-----------------------------------------------------------------------------
bool HFCAuthentication::Failed () const
    {
    return !_GetErrorStack().empty();
    }

size_t HFCAuthentication::GetErrorsQty () const
    {
    return _GetErrorStack().size();
    }

//-----------------------------------------------------------------------------
// Public
// Returns the last error encountered.
//-----------------------------------------------------------------------------
const HFCAuthenticationError& HFCAuthentication::GetLastError () const
    {
    HPRECONDITION(!_GetErrorStack().empty());
    return *_GetErrorStack().top();
    }

//-----------------------------------------------------------------------------
// Public
// Remove the last error from the stack.
//-----------------------------------------------------------------------------
void HFCAuthentication::PopLastError ()
    {
    HPRECONDITION(!_GetErrorStack().empty());

    if (_GetErrorStack().empty())
        return;

    _GetErrorStack().pop();
    }

//-----------------------------------------------------------------------------
// Public
// Add an error to the authentication error stack.
//-----------------------------------------------------------------------------
void HFCAuthentication::PushLastError(const HFCPtr<HFCAuthenticationError>& pi_pLastError)
    {
    if (0 == pi_pLastError)
        {
        HPRECONDITION(!"Input should never be null ptr!");
        return;
        }

    _GetErrorStack().push(pi_pLastError);
    }

void HFCAuthentication::ResetAllErrors ()
    {
    while (!_GetErrorStack().empty())
        _GetErrorStack().pop();
    }


//-----------------------------------------------------------------------------
// class HFCInternetAuthentication
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCInternetAuthentication::HFCInternetAuthentication(const Utf8String&          pi_Server,
                                                     const Utf8String&          pi_User,
                                                     const Utf8String&          pi_Password,
                                                     const uint16_t   pi_RetryCount)
    :   HFCAuthentication(pi_RetryCount),
        m_User(pi_User),
        m_Password(pi_Password),
        m_Server(pi_Server)
    {
    }

//-----------------------------------------------------------------------------
// Public
// destructor
//-----------------------------------------------------------------------------
HFCInternetAuthentication::~HFCInternetAuthentication()
    {
    }

//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
void HFCInternetAuthentication::SetUser (const Utf8String& pi_User)
    {
    m_User = pi_User;
    }

void HFCInternetAuthentication::SetPassword (const Utf8String& pi_Password)
    {
    m_Password = pi_Password;
    }

const Utf8String& HFCInternetAuthentication::GetUser () const
    {
    return m_User;
    }

const Utf8String& HFCInternetAuthentication::GetPassword () const
    {
    return m_Password;
    }

const Utf8String& HFCInternetAuthentication::GetServer () const
    {
    return m_Server;
    }



//-----------------------------------------------------------------------------
// public
// SetByString
//
// A authentication string must be like that
// user[:password[@server]]
//-----------------------------------------------------------------------------
void HFCInternetAuthentication::SetByString(const Utf8String& pi_rAuthenticationString)
    {
    // parse the authentication string
    Utf8String::size_type Pos = pi_rAuthenticationString.find(':');
    Utf8String::size_type Pos2;
    if (Pos != Utf8String::npos)
        {
        m_User = pi_rAuthenticationString.substr(0, Pos);
        ++Pos;

        if ((Pos2 = pi_rAuthenticationString.find('@', Pos)) != Utf8String::npos)
            {
            m_Password = pi_rAuthenticationString.substr(Pos, Pos2 - Pos);
            m_Server = pi_rAuthenticationString.substr(Pos2 + 1);
            }
        else
            m_Password = pi_rAuthenticationString.substr(Pos);
        }
    else if ((Pos = pi_rAuthenticationString.find('@')) != Utf8String::npos)
        {
        m_User = pi_rAuthenticationString.substr(0, Pos);
        m_Server = pi_rAuthenticationString.substr(Pos + 1);
        }
    else
        {
        m_User = pi_rAuthenticationString;
        }
    }


//-----------------------------------------------------------------------------
// class HFCOracleAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCOracleAuthentication::HFCOracleAuthentication(const uint16_t pi_RetryCount)
    :   HFCAuthentication(pi_RetryCount)
    {

    }

//-----------------------------------------------------------------------------
// Public
// destructor
//-----------------------------------------------------------------------------
HFCOracleAuthentication::~HFCOracleAuthentication()
    {

    }

//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
const Utf8String& HFCOracleAuthentication::GetConnectionString () const
    {
    return m_ConnectionString;
    }

const Utf8String& HFCOracleAuthentication::GetUser () const
    {
    return m_User;
    }

const Utf8String& HFCOracleAuthentication::GetPassword () const
    {
    return m_Password;
    }

const Utf8String& HFCOracleAuthentication::GetDatabaseName () const
    {
    return m_DbName;
    }
void HFCOracleAuthentication::SetUser (const Utf8String& pi_User)
    {
    m_User = pi_User;
    }

void HFCOracleAuthentication::SetPassword (const Utf8String& pi_Password)
    {
    m_Password = pi_Password;
    }
void HFCOracleAuthentication::SetDatabaseName (const Utf8String& pi_DbName)
    {
    m_DbName = pi_DbName;
    }


//-----------------------------------------------------------------------------
// public
// SetByString
//
// A authentication string must use the following format:
// user/password@geosvs
//-----------------------------------------------------------------------------
void HFCOracleAuthentication::SetByString(const Utf8String& pi_rAuthenticationString)
    {
    // OCI
        {
        const Utf8String::size_type SlashPos = pi_rAuthenticationString.find('/');
        const Utf8String::size_type AtPos    = pi_rAuthenticationString.find('@');

        if (Utf8String::npos != SlashPos &&  Utf8String::npos != AtPos)
            {
            m_User      = pi_rAuthenticationString.substr(0, SlashPos);
            m_Password  = pi_rAuthenticationString.substr(SlashPos + 1, AtPos - SlashPos - 1);
            m_DbName   = pi_rAuthenticationString.substr(AtPos + 1, Utf8String::npos);
            }
        else
            {
            HASSERT(!"Invalid/unsupported OCCI authentication string.");
            }
        }

    m_ConnectionString = pi_rAuthenticationString;
    }


//-----------------------------------------------------------------------------
// class HFCProxyAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCProxyAuthentication::HFCProxyAuthentication  (const Utf8String&          pi_User,
                                                 const Utf8String&          pi_Password,
                                                 const uint16_t   pi_RetryCount)
    :   HFCAuthentication(pi_RetryCount),
        m_User(pi_User),
        m_Password(pi_Password)
    {
    }

//-----------------------------------------------------------------------------
// Public
// destructor
//-----------------------------------------------------------------------------
HFCProxyAuthentication::~HFCProxyAuthentication()
    {
    }


//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
void HFCProxyAuthentication::SetUser (const Utf8String& pi_User)
    {
    m_User = pi_User;
    }

void HFCProxyAuthentication::SetPassword (const Utf8String& pi_Password)
    {
    m_Password = pi_Password;
    }

void HFCProxyAuthentication::SetServer(const Utf8String& pi_Server)
    {
    m_Server = pi_Server;
    }

const Utf8String& HFCProxyAuthentication::GetUser () const
    {
    return m_User;
    }

const Utf8String& HFCProxyAuthentication::GetPassword () const
    {
    return m_Password;
    }

const Utf8String& HFCProxyAuthentication::GetServer() const
    {
    return m_Server;
    }

//-----------------------------------------------------------------------------
// public
// SetByString
//
// A authentication string must be like that
// user[:password]
//-----------------------------------------------------------------------------
void HFCProxyAuthentication::SetByString(const Utf8String& pi_rAuthenticationString)
    {
    // parse the authentication string
    Utf8String::size_type Pos = pi_rAuthenticationString.find(':');
    if (Pos != Utf8String::npos)
        {
        m_User = pi_rAuthenticationString.substr(0, Pos);
        ++Pos;

        m_Password = pi_rAuthenticationString.substr(Pos);
        }
    else
        {
        m_User = pi_rAuthenticationString;
        }
    }

//-----------------------------------------------------------------------------
// class HFCCertificateAutoritiesAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCCertificateAutoritiesAuthentication::HFCCertificateAutoritiesAuthentication()
: HFCAuthentication(0)
    {
    }

//-----------------------------------------------------------------------------
// Public
// destructor
//-----------------------------------------------------------------------------
HFCCertificateAutoritiesAuthentication::~HFCCertificateAutoritiesAuthentication()
    {
    }

//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
void HFCCertificateAutoritiesAuthentication::SetCertificateAuthFileUrl(const Utf8String& pi_CertificateAuthFileUrl)
    {
    m_CertificateAuthFileUrl = pi_CertificateAuthFileUrl;
    }

const Utf8String& HFCCertificateAutoritiesAuthentication::GetCertificateAuthFileUrl() const
    {
    return m_CertificateAuthFileUrl;
    }

//-----------------------------------------------------------------------------
// class HRFPDFAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCPDFAuthentication::HFCPDFAuthentication (const Utf8String&          pi_FileName,
                                            const PasswordType      pi_PasswordType,
                                            const uint16_t   pi_RetryCount)
    :   HFCAuthentication(pi_RetryCount),
        m_FileName(pi_FileName),
        m_PasswordType(pi_PasswordType),
        m_Password("")
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCPDFAuthentication::~HFCPDFAuthentication()
    {

    }


//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
void HFCPDFAuthentication::SetPassword (const string&   pi_rPassword)
    {
    m_Password = pi_rPassword;
    }

const string& HFCPDFAuthentication::GetPassword () const
    {
    return m_Password;
    }

const Utf8String& HFCPDFAuthentication::GetFileName () const
    {
    return m_FileName;
    }

HFCPDFAuthentication::PasswordType HFCPDFAuthentication::GetPasswordType () const
    {
    return m_PasswordType;
    }

//-----------------------------------------------------------------------------
// public
// SetByString
//-----------------------------------------------------------------------------
void HFCPDFAuthentication::SetByString(const Utf8String& pi_rAuthenticationString)
    {
    WString passwordW(pi_rAuthenticationString.c_str(), BentleyCharEncoding::Utf8);
    AString passwordA;
    BeStringUtilities::WCharToCurrentLocaleChar(passwordA, passwordW.c_str());
    m_Password = passwordA.c_str();
    }
