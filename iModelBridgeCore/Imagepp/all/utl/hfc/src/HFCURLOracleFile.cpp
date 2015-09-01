//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLOracleFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLOracleFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLOracleFile.h>


// This is the creator that registers itself in the scheme list.
struct URLOracleFileCreator : public HFCURL::Creator
    {
    URLOracleFileCreator()
        {
        HFCURLOracleFile::GetSchemeList().insert(HFCURLOracleFile::SchemeList::value_type(HFCURLOracleFile::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLOracleFile(pi_URL);
        }
    } g_URLOracleFileCreator;


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
HFCURLOracleFile::HFCURLOracleFile(const WString& pi_rSchema,
                                   const WString& pi_rHost,
                                   const WString& pi_rDirTableName,
                                   const WString& pi_rPath)
    : HFCURL(s_SchemeName(), WString(L"//") + BuildHostString(pi_rSchema,
                                                                    pi_rHost,
                                                                    pi_rDirTableName,
                                                                    pi_rPath)),
    m_Host(pi_rHost),
    m_Schema(pi_rSchema),
    m_DirTableName(pi_rDirTableName),
    m_Path(pi_rPath)
    {
    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Schema);
    FREEZE_STL_STRING(m_DirTableName);
    FREEZE_STL_STRING(m_Path);
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLOracleFile::HFCURLOracleFile(const WString& pi_rSchema,
                                   const WString& pi_rHost,
                                   const WString& pi_rPath)
    : HFCURL(s_SchemeName(), WString(L"//") + BuildHostString(pi_rSchema,
                                                                    pi_rHost,
                                                                    WString(),
                                                                    pi_rPath)),
    m_Host(pi_rHost),
    m_Schema(pi_rSchema),
    m_Path(pi_rPath)
    {
    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Schema);
    FREEZE_STL_STRING(m_DirTableName);
    FREEZE_STL_STRING(m_Path);
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the complete URL specification.
// Syntax :  horacle://<Schema>@<Host>?<DirTableName:>/Path
//
//-----------------------------------------------------------------------------
HFCURLOracleFile::HFCURLOracleFile(const WString& pi_rURL)
    : HFCURL(pi_rURL)
    {

    bool Error = false;

    if (BeStringUtilities::Wcsicmp(GetSchemeType().c_str(), s_SchemeName().c_str()) == 0)
        {
        WString SchemeSpecificPart = GetSchemeSpecificPart();

        // must be start with "//"
        if ( (SchemeSpecificPart.length() > 2) && (SchemeSpecificPart.substr(0,2) == L"//") )
            {
            WString::size_type QuestionMarkPos;
            if ((QuestionMarkPos = SchemeSpecificPart.find_first_of(L"?", 2)) != WString::npos)
                {
                WString Schema = SchemeSpecificPart.substr(2, QuestionMarkPos - 2);
                WString Image  = SchemeSpecificPart.substr(QuestionMarkPos + 1);

                if (!Schema.empty())
                    {
                    WString::size_type AtPos;

                    if ((AtPos = Schema.find_first_of(L"@")) != WString::npos)
                        {
                        if (AtPos != 0)
                            {
                            m_Schema = Schema.substr(0, AtPos);
                            m_Host = Schema.substr(AtPos + 1);
                            }
                        else
                            {
                            Error = true;
                            }
                        }
                    else
                        {
                        m_Schema = Schema;
                        }
                    }

                if (!Error && !Image.empty())
                    {
                    WString::size_type CommonPos;

                    if ((CommonPos = Image.find_first_of(L":")) != WString::npos)
                        {
                        m_DirTableName = Image.substr(0, CommonPos);
                        m_Path = Image.substr(CommonPos + 1);
                        }
                    else
                        {
                        m_Path = Image;
                        }
                    }
                }
            }
        }

    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Schema);
    FREEZE_STL_STRING(m_DirTableName);
    FREEZE_STL_STRING(m_Path);
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HFCURLOracleFile::~HFCURLOracleFile()
    {
    // Nothing to do here.
    }

//-----------------------------------------------------------------------------
// public
// GetURL
// Returns the standardized and complete URL string.
//-----------------------------------------------------------------------------
WString HFCURLOracleFile::GetURL() const
    {
    WString URL = (s_SchemeName() + L"://");

    if (!m_Schema.empty())
        URL += m_Schema;

    if (!m_Host.empty())
        URL += L"@" + m_Host;

    URL += L"?";

    if (!m_DirTableName.empty())
        URL += m_DirTableName + L":";

    URL += m_Path;

    return URL;
    }


//-----------------------------------------------------------------------------
// public
// FindPathTo
//
// Not implemented.
//-----------------------------------------------------------------------------
WString HFCURLOracleFile::FindPathTo(HFCURL* pi_pDest)
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
HFCURL* HFCURLOracleFile::MakeURLTo(const WString& pi_Path)
    {
    HASSERT(0);
    return 0;
    }


#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLOracleFile::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLOracleFile" << endl;
    out << "Host = " << GetHost() << endl;
    out << "Schema = " << GetSchema() << endl;
    out << "DirTableName = " << GetDirTableName() << endl;
    out << "Path = " << GetPath() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
