//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCCallbacks.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCCallbackRegistry.h>
#include <Imagepp/all/h/HFCCallbacks.h>



//-----------------------------------------------------------------------------
// class HFCAuthenticationCallback
//-----------------------------------------------------------------------------
const HFCAuthenticationCallback* HFCAuthenticationCallback::GetCallbackFromRegistry(
    HCLASS_ID pi_AuthenticationType)
    {
    HFCAuthenticationCallback* pRetCallback = 0;

    unsigned short NbCallbacks = HFCCallbackRegistry::GetInstance()->
                          GetNbCallbacks(HFCAuthenticationCallback::CLASS_ID);

    for (unsigned short CallbackInd = 0; CallbackInd < NbCallbacks; CallbackInd++)
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
WString HFCAuthenticationError::ToString () const
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
HFCAuthentication::HFCAuthentication(const unsigned short pi_RetryCount)
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
unsigned short HFCAuthentication::IncrementRetryCount()
    {
    return ++m_RetryCount;
    };

unsigned short HFCAuthentication::GetRetryCount() const
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
HFCInternetAuthentication::HFCInternetAuthentication(const WString&          pi_Server,
                                                     const WString&          pi_User,
                                                     const WString&          pi_Password,
                                                     const unsigned short   pi_RetryCount)
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
void HFCInternetAuthentication::SetUser (const WString& pi_User)
    {
    m_User = pi_User;
    }

void HFCInternetAuthentication::SetPassword (const WString& pi_Password)
    {
    m_Password = pi_Password;
    }

const WString& HFCInternetAuthentication::GetUser () const
    {
    return m_User;
    }

const WString& HFCInternetAuthentication::GetPassword () const
    {
    return m_Password;
    }

const WString& HFCInternetAuthentication::GetServer () const
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
void HFCInternetAuthentication::SetByString(const WString& pi_rAuthenticationString)
    {
    // parse the authentication string
    WString::size_type Pos = pi_rAuthenticationString.find(L':');
    WString::size_type Pos2;
    if (Pos != WString::npos)
        {
        m_User = pi_rAuthenticationString.substr(0, Pos);
        ++Pos;

        if ((Pos2 = pi_rAuthenticationString.find(L'@', Pos)) != WString::npos)
            {
            m_Password = pi_rAuthenticationString.substr(Pos, Pos2 - Pos);
            m_Server = pi_rAuthenticationString.substr(Pos2 + 1);
            }
        else
            m_Password = pi_rAuthenticationString.substr(Pos);
        }
    else if ((Pos = pi_rAuthenticationString.find(L'@')) != WString::npos)
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
HFCOracleAuthentication::HFCOracleAuthentication(const unsigned short pi_RetryCount)
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
const WString& HFCOracleAuthentication::GetConnectionString () const
    {
    return m_ConnectionString;
    }

const WString& HFCOracleAuthentication::GetUser () const
    {
    return m_User;
    }

const WString& HFCOracleAuthentication::GetPassword () const
    {
    return m_Password;
    }

const WString& HFCOracleAuthentication::GetDatabaseName () const
    {
    return m_DbName;
    }
void HFCOracleAuthentication::SetUser (const WString& pi_User)
    {
    m_User = pi_User;
    }

void HFCOracleAuthentication::SetPassword (const WString& pi_Password)
    {
    m_Password = pi_Password;
    }
void HFCOracleAuthentication::SetDatabaseName (const WString& pi_DbName)
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
void HFCOracleAuthentication::SetByString(const WString& pi_rAuthenticationString)
    {
    // OCI
        {
        const WString::size_type SlashPos = pi_rAuthenticationString.find(L'/');
        const WString::size_type AtPos    = pi_rAuthenticationString.find(L'@');

        if (WString::npos != SlashPos &&  WString::npos != AtPos)
            {
            m_User      = pi_rAuthenticationString.substr(0, SlashPos);
            m_Password  = pi_rAuthenticationString.substr(SlashPos + 1, AtPos - SlashPos - 1);
            m_DbName   = pi_rAuthenticationString.substr(AtPos + 1, WString::npos);
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
HFCProxyAuthentication::HFCProxyAuthentication  (const WString&          pi_User,
                                                 const WString&          pi_Password,
                                                 const unsigned short   pi_RetryCount)
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
void HFCProxyAuthentication::SetUser (const WString& pi_User)
    {
    m_User = pi_User;
    }

void HFCProxyAuthentication::SetPassword (const WString& pi_Password)
    {
    m_Password = pi_Password;
    }

const WString& HFCProxyAuthentication::GetUser () const
    {
    return m_User;
    }

const WString& HFCProxyAuthentication::GetPassword () const
    {
    return m_Password;
    }

//-----------------------------------------------------------------------------
// public
// SetByString
//
// A authentication string must be like that
// user[:password]
//-----------------------------------------------------------------------------
void HFCProxyAuthentication::SetByString(const WString& pi_rAuthenticationString)
    {
    // parse the authentication string
    WString::size_type Pos = pi_rAuthenticationString.find(L':');
    if (Pos != WString::npos)
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
// class HRFPDFAuthentication
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// constructor
//-----------------------------------------------------------------------------
HFCPDFAuthentication::HFCPDFAuthentication (const WString&          pi_FileName,
                                            const PasswordType      pi_PasswordType,
                                            const unsigned short   pi_RetryCount)
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

const WString& HFCPDFAuthentication::GetFileName () const
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
void HFCPDFAuthentication::SetByString(const WString& pi_rAuthenticationString)
    {
    size_t  destinationBuffSize = pi_rAuthenticationString.GetMaxLocaleCharBytes();
    char*  pAuthenticationStringMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(pAuthenticationStringMBS, pi_rAuthenticationString.c_str(),destinationBuffSize);

    m_Password = string(pAuthenticationStringMBS);
    }
