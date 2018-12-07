//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLOracleFile.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("horacle");
        return Val;
        }

    // Primary methods

    HFCURLOracleFile(const Utf8String& pi_rURL);

    HFCURLOracleFile(const Utf8String& pi_rSchema,
                     const Utf8String& pi_rHost,
                     const Utf8String& pi_rDirTableName,
                     const Utf8String& pi_rPath);

    HFCURLOracleFile(const Utf8String& pi_rSchema,
                     const Utf8String& pi_rHost,
                     const Utf8String& pi_rPath);

    virtual                 ~HFCURLOracleFile();

    // Content access methods

    virtual Utf8String          GetURL() const;
    const Utf8String&           GetHost() const;
    const Utf8String&           GetSchema() const;
    const Utf8String&           GetDirTableName() const;
    const Utf8String&           GetPath() const;

    // Overriden methods, used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual Utf8String          FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const Utf8String& pi_Path);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLOracleFileCreator;

    // Components of the scheme-specific part of the URL string.

    Utf8String      m_Host;
    Utf8String      m_Schema;
    Utf8String      m_DirTableName;
    Utf8String      m_Path;

    // Disabled methods

    HFCURLOracleFile(const HFCURLOracleFile&);
    HFCURLOracleFile& operator=(const HFCURLOracleFile&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLOracleFile.hpp"

