//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLOracleFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLOracleFile
//-----------------------------------------------------------------------------
// This class defines a specialized version of URL interface for the "file:"
// scheme type.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCURL.h"

BEGIN_IMAGEPP_NAMESPACE
// URL specification at this level is:
// horacle:{//layername:imageid@host}


class HFCURLOracleFile : public HFCURL
    {
public:

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"horacle");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    // Primary methods

    HFCURLOracleFile(const WString& pi_rURL);

    HFCURLOracleFile(const WString& pi_rSchema,
                     const WString& pi_rHost,
                     const WString& pi_rDirTableName,
                     const WString& pi_rPath);

    HFCURLOracleFile(const WString& pi_rSchema,
                     const WString& pi_rHost,
                     const WString& pi_rPath);

    HFCURLOracleFile() { } // required for persistence
    virtual                 ~HFCURLOracleFile();

    // Content access methods

    virtual WString          GetURL() const;
    const WString&           GetHost() const;
    const WString&           GetSchema() const;
    const WString&           GetDirTableName() const;
    const WString&           GetPath() const;

    // Overriden methods, used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual WString          FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const WString& pi_Path);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLOracleFileCreator;

    // Components of the scheme-specific part of the URL string.

    WString      m_Host;
    WString      m_Schema;
    WString      m_DirTableName;
    WString      m_Path;

    // Disabled methods

    HFCURLOracleFile(const HFCURLOracleFile&);
    HFCURLOracleFile& operator=(const HFCURLOracleFile&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLOracleFile.hpp"

