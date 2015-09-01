//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLSql.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLSql
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HFCURL.h"

BEGIN_IMAGEPP_NAMESPACE
// URL specification at this level is:
// hsql:{//?sql}

class HFCURLSql : public HFCURL
    {
public:

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"hsql");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    // Primary methods

    HFCURLSql(const WString& pi_rURL);


    HFCURLSql() { } // required for persistence
    virtual                 ~HFCURLSql();

    // Content access methods

    virtual WString          GetURL() const;
    const WString&           GetQuery() const;

    // Overriden methods, used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual WString         FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const WString& pi_Path);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLSqlCreator;

    // Components of the scheme-specific part of the URL string.

    WString      m_Query;

    // Disabled methods

    HFCURLSql(const HFCURLSql&);
    HFCURLSql& operator=(const HFCURLSql&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLSql.hpp"

