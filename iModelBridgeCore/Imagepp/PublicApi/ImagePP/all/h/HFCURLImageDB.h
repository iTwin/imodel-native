//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLImageDB.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLImageDB
//-----------------------------------------------------------------------------
// This class defines a specialized version of URL interface for the "imagedb:"
// scheme type.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCURL.h"
BEGIN_IMAGEPP_NAMESPACE
// URL specification at this level is:
// imagedb:{//internalname/driveName[/dir.../dir/imagename]}
// imagedb:{//dbtype:ConnectString[?prompt string]/tablename[/dir.../dir/imagename]}
//     ConnectString as the format Usr:Pwd@Server


class HFCURLImageDB : public HFCURL
    {
public:

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"imagedb");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    // Primary methods

    HFCURLImageDB(const WString& pi_rURL);

    HFCURLImageDB(const WString& pi_rInternalName,
                  const WString& pi_rDriveName,
                  const WString& pi_rPath);

    HFCURLImageDB(const WString& pi_rDatabaseType,
                  const WString& pi_rUsrName,
                  const WString& pi_rPwd,
                  const WString& pi_rServerName,
                  const WString& pi_rPromptString,
                  const WString& pi_rDriveName,
                  const WString& pi_rPath);

    virtual                 ~HFCURLImageDB();

    // Content access methods

    virtual WString         GetURL() const;

    bool                   HasInternalName() const;
    bool                   HasConnectString() const;

    const WString&          GetInternalName() const;

    const WString&          GetUsr() const;
    const WString&          GetPwd() const;
    const WString&          GetServer() const;
    const WString&          GetPromptString() const;

    const WString&          GetDriveName() const;
    const WString&          GetPath() const;

    const WString           GetDatabaseType() const;

    // Overriden methods, used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual WString         FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const WString& pi_Path);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLImageDBCreator;

    // Components of the scheme-specific part of the URL string.

    WString      m_InternalName;
    WString      m_DatabaseType;
    WString      m_Usr;
    WString      m_Pwd;
    WString      m_Server;

    WString      m_DriveName;
    WString      m_Path;

    WString      m_PromptString;

    // Disabled methods
    HFCURLImageDB& operator=(const HFCURLImageDB&);
    HFCURLImageDB(const HFCURLImageDB&);
    };

END_IMAGEPP_NAMESPACE