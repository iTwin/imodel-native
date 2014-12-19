//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCNetAPI.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Declaration for class HFCNetAPI
//
// NOTE:
// Application must link with: netapi32.lib
//-----------------------------------------------------------------------------
#pragma once

//######################
// INCLUDE FILES
//######################


class HFCNetAPI
    {
public:

    // stl typedef
    typedef list<WString, allocator<WString> > HFCNETAPI_USERNAME_LIST;
    typedef HFCNETAPI_USERNAME_LIST::iterator HFCNETAPI_USERNAME_LIST_ITR;

    typedef HFCNETAPI_USERNAME_LIST HFCNETAPI_USERACCOUNT_LIST;
    typedef HFCNETAPI_USERACCOUNT_LIST::iterator HFCNETAPI_USERACCOUNT_LIST_ITR;


    // Public enum
    enum HFC_WKSTA_USER_LEVEL
        {
        HFC_WKSTA_USER_LEVEL_0 = 0,
        HFC_WKSTA_USER_LEVEL_1 = 1,
        };

    // Construction - destruction
    HFCNetAPI(HFC_WKSTA_USER_LEVEL pi_Level = HFC_WKSTA_USER_LEVEL_0);
    virtual
    ~HFCNetAPI();

    // Initialization
    bool
    Initialize();

    // Operations
    const WString&
    GetLogonDomainName();

    const WString&
    GetLogonUserName();

    const WString&
    GetLogonServerName();

    const WString&
    GetOthersDomainsName();

    bool
    GetUserAccountList(const WString& pi_rServerName,
                       HFCNetAPI::HFCNETAPI_USERACCOUNT_LIST& po_rList);

    bool
    GetUserList(const WString& pi_rServerName,
                HFCNetAPI::HFCNETAPI_USERNAME_LIST& po_rList);

    bool
    GetDomainControllerName(const WString& pi_rDomainName,
                            WString& po_rDomainControllerName);

    bool
    GetLocalServerName(WString& po_rServerName);

    bool
    IsLogonDomainNameAvailable();

    bool
    IsLogonUserNameAvailable();

    bool
    IsLogonServerNameAvailable();

    bool
    IsOthersDomainsNameAvailable();

protected:


private:
    // Not implemented

    HFCNetAPI(const HFCNetAPI&);
    HFCNetAPI& operator=(const HFCNetAPI&);

    bool      GetCurrentWkstaUserInfo();

    // Attributes
    HFC_WKSTA_USER_LEVEL m_WkstaUserLevel;

    WString    m_LogonUserName;
    WString    m_LogonServerName;
    WString    m_LogonDomainName;
    WString    m_LogonOthersDomainsName;
    };

