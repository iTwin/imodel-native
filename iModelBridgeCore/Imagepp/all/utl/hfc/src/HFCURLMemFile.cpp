//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLMemFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLMemFile.h>

//:Ignore
// This is the creator that registers itself in the scheme list.
struct URLMemoryCreator : public HFCURL::Creator
    {
    URLMemoryCreator()
        {
        HFCURL::RegisterCreator(HFCURLMemFile::s_SchemeName(), this);
        }
    virtual HFCURL* Create(const Utf8String& pi_URL) const
        {
        return new HFCURLMemFile(pi_URL);
        }
    } g_URLMemoryCreator;

//:End Ignore

/**----------------------------------------------------------------------------
 This constructor configures the object from the complete URL specification.

 Syntax :  @c{memory:{Filename[.extension]}

 @param pi_URL Constant reference to a string that contains a complete URL
               specification or a pathname specification.

 @inheritance This class is an instanciable one that correspond to one precise
              kind of URL and no child of it are expected to be defined.
-----------------------------------------------------------------------------*/
HFCURLMemFile::HFCURLMemFile(const Utf8String& pi_URL,
                             const HFCPtr<HFCBuffer>& pi_pBuffer)
    : HFCURL(pi_URL),
      m_pBuffer(pi_pBuffer)
    {
    HPRECONDITION(GetSchemeType() == HFCURLMemFile::s_SchemeName());

    //Init default values
    m_creationTime=BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000;    // time_t is in second.
    m_modificationTime=m_creationTime;

    // Filename part is not interpreted we only skip the leading "//" 
    if (GetSchemeSpecificPart().substr(0,2).compare("//") == 0)
        m_Filename = GetSchemeSpecificPart().substr(2);
    else
        m_Filename = GetSchemeSpecificPart();
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURLMemFile::~HFCURLMemFile()
    {
    // Nothing to do here.
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns the standardized and complete URL string.

 @see HFCURL
-----------------------------------------------------------------------------*/
Utf8String HFCURLMemFile::GetURL() const
    {
    return s_SchemeName() + "://" + m_Filename;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns true only if a relative path can be calculated from this URL and the
 specified one.

 @see HFCURL
-----------------------------------------------------------------------------*/
bool HFCURLMemFile::HasPathTo(HFCURL* pi_pURL)
    {
    if (HFCURL::HasPathTo(pi_pURL))
        return (pi_pURL->GetSchemeSpecificPart() == GetSchemeSpecificPart());
    return false;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Calculates a relative path that can be added to this URL in order to get
 the specified one.

 @see HFCURL
-----------------------------------------------------------------------------*/
Utf8String HFCURLMemFile::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(HasPathTo(pi_pDest));
    return FindPath(GetSchemeSpecificPart(), pi_pDest->GetSchemeSpecificPart());
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Creates a brand new URL descriptor that corresponds to this URL added with
 the specified relative path.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLMemFile::MakeURLTo(const Utf8String& pi_Path)
    {
    return new HFCURLMemFile(s_SchemeName() + "://" + AddPath(GetSchemeSpecificPart(), pi_Path));  //!!!! //HChk BB
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HFCURLMemFile::GetFilename() const
    {
    return m_Filename;
    }

/**----------------------------------------------------------------------------
 Returns a string containing the extension part of the name of the resource,
 which is the text after the last dot in its name.
-----------------------------------------------------------------------------*/
Utf8String HFCURLMemFile::GetExtension() const
    {
    Utf8String::size_type LastDotPos = m_Filename.find_last_of(".");
    if ((LastDotPos != Utf8String::npos) && (LastDotPos < (m_Filename.length() - 1)))
        return m_Filename.substr(LastDotPos + 1, m_Filename.length() - LastDotPos - 1);
   
    return Utf8String();
    }

/**----------------------------------------------------------------------------
Returns a smart pointer to a HFCBuffer.

@return A smart pointer to a HFCBuffer

@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
HFCPtr<HFCBuffer>& HFCURLMemFile::GetBuffer()
    {
    return m_pBuffer;
    }

/**----------------------------------------------------------------------------
Set a internal smart pointer to an HFCBuffer.

@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
void HFCURLMemFile::SetBuffer(HFCPtr<HFCBuffer>& pi_rpBuffer)
    {
    m_pBuffer = pi_rpBuffer;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
void HFCURLMemFile::SetCreationTime(time_t   pi_NewTime)
    {
    m_creationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
time_t HFCURLMemFile::GetCreationTime() const
    {
    return m_creationTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
void HFCURLMemFile::SetModificationTime(time_t   pi_NewTime)
    {
    m_modificationTime=pi_NewTime;
    }

/**----------------------------------------------------------------------------
@inheritance This method cannot be overriden.
-----------------------------------------------------------------------------*/
time_t HFCURLMemFile::GetModificationTime() const
    {
    return m_modificationTime;
    }

#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLMemFile::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLMemFile" << endl;
    out << "Host = " << GetHost() << endl;
    out << "Path = " << GetPath() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif

