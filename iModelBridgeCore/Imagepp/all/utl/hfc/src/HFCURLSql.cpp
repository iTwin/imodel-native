//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLSql
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLSql.h>


// This is the creator that registers itself in the scheme list.
struct URLSqlCreator : public HFCURL::Creator
    {
    URLSqlCreator()
        {
        HFCURL::RegisterCreator(HFCURLSql::s_SchemeName(), this);
        }
    virtual HFCURL* Create(const Utf8String& pi_URL) const
        {
        return new HFCURLSql(pi_URL);
        }
    } g_URLSqlCreator;

#if (0)
//-----------------------------------------------------------------------------
// Little static function that is used by constructor

static Utf8String BuildHostString(const Utf8String& pi_rSchema,
                               const Utf8String& pi_rHost,
                               const Utf8String& pi_rDirTableName,
                               const Utf8String& pi_rPath)
    {
    Utf8String  Result;
    bool   Error = false;

    // validation
    if (!pi_rHost.empty() && pi_rSchema.empty())
        Error = true;

    if (!pi_rPath.empty())
        Error = true;

    if (!Error)
        {
        if (!pi_rSchema.empty())
            Result += pi_rSchema;

        if (!pi_rHost.empty())
            Result += "@" + pi_rHost;

        Result += "?";

        if (!pi_rDirTableName.empty())
            Result += pi_rDirTableName + ":";

        Result += pi_rPath;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLSql::HFCURLSql(const Utf8String& pi_rQuery)
    : HFCURL(s_SchemeName(), Utf8String("//?") + pi_rQuery),
      m_Query(pi_rQuery)
    {
    }
#endif
//-----------------------------------------------------------------------------
// This constructor configures the object from the complete URL specification.
// Syntax :  hsql://?<Query>
//
//-----------------------------------------------------------------------------
HFCURLSql::HFCURLSql(const Utf8String& pi_rURL)
    : HFCURL(pi_rURL)
    {

    if (GetSchemeType().EqualsI(s_SchemeName()))
        {
        Utf8String SchemeSpecificPart = GetSchemeSpecificPart();

        // must be start with "//"
        if ( (SchemeSpecificPart.length() > 3) && (SchemeSpecificPart.substr(0,3) == "//?") )
            {
            m_Query = SchemeSpecificPart.substr(3);
            }
        }
    }
//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HFCURLSql::~HFCURLSql()
    {
    // Nothing to do here.
    }

//-----------------------------------------------------------------------------
// public
// GetURL
// Returns the standardized and complete URL string.
//-----------------------------------------------------------------------------
Utf8String HFCURLSql::GetURL() const
    {
    Utf8String URL(Utf8String(s_SchemeName()+ "://?") + m_Query);

    return URL;
    }


//-----------------------------------------------------------------------------
// public
// FindPathTo
//
// Not implemented.
//-----------------------------------------------------------------------------
Utf8String HFCURLSql::FindPathTo(HFCURL* pi_pDest)
    {
    HASSERT(0);
    return Utf8String();
    }


//-----------------------------------------------------------------------------
// public
// MakeURLTo
//
// Not implemented.
//-----------------------------------------------------------------------------
HFCURL* HFCURLSql::MakeURLTo(const Utf8String& pi_Path)
    {
    HASSERT(0);
    return 0;
    }


#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLSql::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLSql" << endl;
    out << "Query = " << GetQuery() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
