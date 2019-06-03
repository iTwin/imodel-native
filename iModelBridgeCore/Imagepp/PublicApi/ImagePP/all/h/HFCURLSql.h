//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("hsql");
        return Val;
        }

    // Primary methods

    HFCURLSql(const Utf8String& pi_rURL);

    virtual                 ~HFCURLSql();

    // Content access methods

    virtual Utf8String          GetURL() const;
    const Utf8String&           GetQuery() const;

    // Overriden methods, used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual Utf8String         FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const Utf8String& pi_Path);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLSqlCreator;

    // Components of the scheme-specific part of the URL string.

    Utf8String      m_Query;

    // Disabled methods

    HFCURLSql(const HFCURLSql&);
    HFCURLSql& operator=(const HFCURLSql&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLSql.hpp"

