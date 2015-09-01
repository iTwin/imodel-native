//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLSql.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLSql
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLSql.h>


// This is the creator that registers itself in the scheme list.
struct URLSqlCreator : public HFCURL::Creator
    {
    URLSqlCreator()
        {
        HFCURLSql::GetSchemeList().insert(HFCURLSql::SchemeList::value_type(HFCURLSql::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLSql(pi_URL);
        }
    } g_URLSqlCreator;

#if (0)
//-----------------------------------------------------------------------------
// Little static function that is used by constructor

static WString BuildHostString(const WString& pi_rSchema,
                               const WString& pi_rHost,
                               const WString& pi_rDirTableName,
                               const WString& pi_rPath)
    {
    WString  Result;
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
            Result += L"@" + pi_rHost;

        Result += L"?";

        if (!pi_rDirTableName.empty())
            Result += pi_rDirTableName + L":";

        Result += pi_rPath;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLSql::HFCURLSql(const WString& pi_rQuery)
    : HFCURL(s_SchemeName(), WString(L"//?") + pi_rQuery),
      m_Query(pi_rQuery)
    {
    FREEZE_STL_STRING(m_Query);
    }
#endif
//-----------------------------------------------------------------------------
// This constructor configures the object from the complete URL specification.
// Syntax :  hsql://?<Query>
//
//-----------------------------------------------------------------------------
HFCURLSql::HFCURLSql(const WString& pi_rURL)
    : HFCURL(pi_rURL)
    {

    if (BeStringUtilities::Wcsicmp(GetSchemeType().c_str(), s_SchemeName().c_str()) == 0)
        {
        WString SchemeSpecificPart = GetSchemeSpecificPart();

        // must be start with "//"
        if ( (SchemeSpecificPart.length() > 3) && (SchemeSpecificPart.substr(0,3) == L"//?") )
            {
            m_Query = SchemeSpecificPart.substr(3);
            }
        }

    FREEZE_STL_STRING(m_Query);
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
WString HFCURLSql::GetURL() const
    {
    WString URL(WString(s_SchemeName()+ L"://?") + m_Query);

    return URL;
    }


//-----------------------------------------------------------------------------
// public
// FindPathTo
//
// Not implemented.
//-----------------------------------------------------------------------------
WString HFCURLSql::FindPathTo(HFCURL* pi_pDest)
    {
    HASSERT(0);
    return WString();
    }


//-----------------------------------------------------------------------------
// public
// MakeURLTo
//
// Not implemented.
//-----------------------------------------------------------------------------
HFCURL* HFCURLSql::MakeURLTo(const WString& pi_Path)
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
