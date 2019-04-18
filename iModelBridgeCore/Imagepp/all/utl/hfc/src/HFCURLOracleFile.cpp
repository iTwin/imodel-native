//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLOracleFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLOracleFile.h>


// This is the creator that registers itself in the scheme list.
struct URLOracleFileCreator : public HFCURL::Creator
    {
    URLOracleFileCreator()
        {
        HFCURL::RegisterCreator(HFCURLOracleFile::s_SchemeName(), this);
        }
    virtual HFCURL* Create(const Utf8String& pi_URL) const
        {
        return new HFCURLOracleFile(pi_URL);
        }
    } g_URLOracleFileCreator;


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
HFCURLOracleFile::HFCURLOracleFile(const Utf8String& pi_rSchema,
                                   const Utf8String& pi_rHost,
                                   const Utf8String& pi_rDirTableName,
                                   const Utf8String& pi_rPath)
    : HFCURL(s_SchemeName(), Utf8String("//") + BuildHostString(pi_rSchema,
                                                                    pi_rHost,
                                                                    pi_rDirTableName,
                                                                    pi_rPath)),
    m_Host(pi_rHost),
    m_Schema(pi_rSchema),
    m_DirTableName(pi_rDirTableName),
    m_Path(pi_rPath)
    {
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the
// scheme-specific part of the URL string.
//-----------------------------------------------------------------------------
HFCURLOracleFile::HFCURLOracleFile(const Utf8String& pi_rSchema,
                                   const Utf8String& pi_rHost,
                                   const Utf8String& pi_rPath)
    : HFCURL(s_SchemeName(), Utf8String("//") + BuildHostString(pi_rSchema,
                                                                    pi_rHost,
                                                                    Utf8String(),
                                                                    pi_rPath)),
    m_Host(pi_rHost),
    m_Schema(pi_rSchema),
    m_Path(pi_rPath)
    {
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the complete URL specification.
// Syntax :  horacle://<Schema>@<Host>?<DirTableName:>/Path
//
//-----------------------------------------------------------------------------
HFCURLOracleFile::HFCURLOracleFile(const Utf8String& pi_rURL)
    : HFCURL(pi_rURL)
    {

    bool Error = false;

    if (GetSchemeType().EqualsI(s_SchemeName()))
        {
        Utf8String SchemeSpecificPart = GetSchemeSpecificPart();

        // must be start with "//"
        if ( (SchemeSpecificPart.length() > 2) && (SchemeSpecificPart.substr(0,2) == "//") )
            {
            Utf8String::size_type QuestionMarkPos;
            if ((QuestionMarkPos = SchemeSpecificPart.find_first_of("?", 2)) != Utf8String::npos)
                {
                Utf8String Schema = SchemeSpecificPart.substr(2, QuestionMarkPos - 2);
                Utf8String Image  = SchemeSpecificPart.substr(QuestionMarkPos + 1);

                if (!Schema.empty())
                    {
                    Utf8String::size_type AtPos;

                    if ((AtPos = Schema.find_first_of("@")) != Utf8String::npos)
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
                    Utf8String::size_type CommonPos;

                    if ((CommonPos = Image.find_first_of(":")) != Utf8String::npos)
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
Utf8String HFCURLOracleFile::GetURL() const
    {
    Utf8String URL = (s_SchemeName() + "://");

    if (!m_Schema.empty())
        URL += m_Schema;

    if (!m_Host.empty())
        URL += "@" + m_Host;

    URL += "?";

    if (!m_DirTableName.empty())
        URL += m_DirTableName + ":";

    URL += m_Path;

    return URL;
    }


//-----------------------------------------------------------------------------
// public
// FindPathTo
//
// Not implemented.
//-----------------------------------------------------------------------------
Utf8String HFCURLOracleFile::FindPathTo(HFCURL* pi_pDest)
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
HFCURL* HFCURLOracleFile::MakeURLTo(const Utf8String& pi_Path)
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
