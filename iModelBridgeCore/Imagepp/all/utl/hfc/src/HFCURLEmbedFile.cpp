//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLEmbedFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLOracleFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLEmbedFile.h>

// This is the creator that registers itself in the scheme list.
struct URLEmbedFileCreator : public HFCURL::Creator
    {
    URLEmbedFileCreator()
        {
        HFCURLEmbedFile::GetSchemeList().insert(HFCURLEmbedFile::SchemeList::value_type(HFCURLEmbedFile::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLEmbedFile(pi_URL, NULL);
        }
    } g_URLEmbedFileCreator;

//-----------------------------------------------------------------------------
// This constructor configures the object from the complete URL specification.
// Syntax :  embed://<fileName>
//
//-----------------------------------------------------------------------------
HFCURLEmbedFile::HFCURLEmbedFile(const WString& pi_rURL,
                                 IFileReaderHandler* pi_pHandler)
    : HFCURL(pi_rURL),
      m_pHandler(pi_pHandler)
    {
    //Init default values
    m_creationTime=BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000;    // time_t is in second.
    m_modificationTime=m_creationTime;

    if (BeStringUtilities::Wcsicmp(GetSchemeType().c_str(), s_SchemeName().c_str()) == 0)
        {
        WString SchemeSpecificPart = GetSchemeSpecificPart();

        m_Path = SchemeSpecificPart.substr(2, SchemeSpecificPart.length());
        }

    FREEZE_STL_STRING(m_Path);
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HFCURLEmbedFile::~HFCURLEmbedFile()
    {
    // Nothing to do here.
    }

//-----------------------------------------------------------------------------
// public
// GetURL
// Returns the standardized and complete URL string.
//-----------------------------------------------------------------------------
WString HFCURLEmbedFile::GetURL() const
    {
    return WString(s_SchemeName() + L"://") + m_Path;
    }

//-----------------------------------------------------------------------------
// public
// FindPathTo
//
// Not implemented.
//-----------------------------------------------------------------------------
WString HFCURLEmbedFile::FindPathTo(HFCURL* pi_pDest)
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
HFCURL* HFCURLEmbedFile::MakeURLTo(const WString& pi_Path)
    {
    HASSERT(0);
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
// RegisterFileReaderHandler
//-----------------------------------------------------------------------------
void HFCURLEmbedFile::RegisterFileReaderHandler(HFCPtr<IFileReaderHandler> pi_pHandler)
    {
    m_pHandler = pi_pHandler;
    }

//-----------------------------------------------------------------------------
// Public
// GetPWHandler
//-----------------------------------------------------------------------------
HFCPtr<IFileReaderHandler> HFCURLEmbedFile::GetFileReaderHandler() const
    {
    return m_pHandler;
    }

#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLEmbedFile::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLEmbedFile" << endl;
    out << "Schema = " << GetSchema() << endl;
    out << "Path = " << GetPath() << endl;

    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
