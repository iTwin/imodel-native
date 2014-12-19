//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLMemFile.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLMemFile
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCURLMemFile.h>

//:Ignore
// This is the creator that registers itself in the scheme list.

struct URLMemoryCreator : public HFCURL::Creator
    {
    URLMemoryCreator()
        {
        HFCURLMemFile::GetSchemeList().insert(HFCURLMemFile::SchemeList::value_type(HFCURLMemFile::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLMemFile(pi_URL);
        }
    } g_URLMemoryCreator;

//:End Ignore

/**----------------------------------------------------------------------------
 This constructor configures the object from the detached parts of the
 scheme-specific part of the URL string.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_Host Constant reference to a string that contains the specification
                of the host (its network name) or the specification of
                the drive (which includes the trailing colon).

 @param pi_Path Constant reference to a string that contains the path to the
                file, relative to the root of the host or of the drive.
                May be empty if the URL is used to locate the host or
                the drive itself.  Must not begin by a slash or
                backslash.

 @inheritance This class is an instanciable one that correspond to one precise
              kind of URL and no child of it are expected to be defined.
-----------------------------------------------------------------------------*/
HFCURLMemFile::HFCURLMemFile(const WString&     pi_Host,
                             const WString&     pi_Path,
                             const HFCPtr<HFCBuffer>& pi_rpBuffer)
    : HFCURL(s_SchemeName(), WString(L"//") + pi_Host + pi_Path),
      m_Host(pi_Host),
      m_Path(pi_Path),
      m_pBuffer(pi_rpBuffer)
    {
    //Init default values
    m_creationTime=BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000;    // time_t is in second.
    m_modificationTime=m_creationTime;

    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Path);
    }

/**----------------------------------------------------------------------------
 This constructor configures the object from the complete URL specification.

 Syntax :  @c{memory:{//host/[path] | [//]drive/[path]}}

 where @r{host} can be a drive spec or a machine name; drive will be stored
 as host; slashes are replaceable by backslaches; the slash before the path
 is included in the path obtained by @k{GetPath}.

 This constructor can also receive a filename specification (as used on
 DOS and Windows) instead of an URL.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_URL Constant reference to a string that contains a complete URL
               specification or a pathname specification.

 @inheritance This class is an instanciable one that correspond to one precise
              kind of URL and no child of it are expected to be defined.
-----------------------------------------------------------------------------*/
HFCURLMemFile::HFCURLMemFile(const WString&     pi_URL,
                             const HFCPtr<HFCBuffer>& pi_pBuffer)
    : HFCURL(pi_URL),
      m_pBuffer(pi_pBuffer)
    {
    //Init default values
    m_creationTime=BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000;    // time_t is in second.
    m_modificationTime=m_creationTime;

    // Is there a double-slash at beginning?

    if (GetSchemeSpecificPart().length() >= 2)
        {
        WString::size_type Pos = 0;
        if ((GetSchemeSpecificPart().substr(Pos,2) == L"//") ||
            (GetSchemeSpecificPart().substr(Pos,2) == L"\\\\"))
            {
            Pos += 2;

            // Yes : find where the path begins, and extract both parts
            if (GetSchemeSpecificPart().length() > 2)
                {
                // if the scheme starts with "\\", it is a network drive (UNC)
                //
                // if the begining of the URL starts with a machine name, the host
                // contains only the machine name
                //
                // e.g.: file://\\alpha\cert\dataset\image++\
                //
                //       machine name = alpha
                //       share point name = cert
                //       host = \\alpha
                //       path = cert\dataset\image++
                //
                if (GetSchemeSpecificPart().substr(Pos, 2) == L"\\\\")
                    Pos += 2;
                else if (GetSchemeSpecificPart().substr(Pos, 2) == L"//")
                    Pos += 2;

                WString::size_type SingleSlashPos =
                    GetSchemeSpecificPart().find_first_of(L"\\/", Pos);
                if (SingleSlashPos != WString::npos)
                    {
                    m_Host = GetSchemeSpecificPart().substr(2, SingleSlashPos - 2);
                    m_Path = GetSchemeSpecificPart().substr(SingleSlashPos + 1,
                                                            GetSchemeSpecificPart().length()-SingleSlashPos - 1);
                    }
                else
                    {
                    m_Host = GetSchemeSpecificPart().substr(2,
                                                            GetSchemeSpecificPart().length()-2);
                    }

                // if the host contains no slash nor colon, it is not a drive spec but a
                // UNC host without the additional double-slash (ie: file://host/path)
                // Then we add a double-slash...

                WString::size_type SlashOrColonPos = m_Host.find_first_of(L"\\/:", 0);
                if (SlashOrColonPos == WString::npos)
                    m_Host = L"\\\\" + m_Host;
                }
            }
        else
            {
            // No : is there a drive spec at beginning?
            if (GetSchemeSpecificPart()[1] == L':')
                {
                // yes : extract it as the host, then extract the path
                m_Host = GetSchemeSpecificPart().substr(0,2);
                if (GetSchemeSpecificPart().length() > 3)  // has a path?
                    m_Path = GetSchemeSpecificPart().substr(3, GetSchemeSpecificPart().length()-3);
                }
            }
        }

    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Path);
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
WString HFCURLMemFile::GetURL() const
    {
    return WString(s_SchemeName() + L"://") + m_Host + L"\\" + m_Path;
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
        return (((HFCURLMemFile*)pi_pURL)->GetHost() == GetHost());
    return false;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Calculates a relative path that can be added to this URL in order to get
 the specified one.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLMemFile::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(HasPathTo(pi_pDest));
    return FindPath(GetPath(), ((HFCURLMemFile*)pi_pDest)->GetPath());
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Creates a brand new URL descriptor that corresponds to this URL added with
 the specified relative path.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLMemFile::MakeURLTo(const WString& pi_Path)
    {
    return new HFCURLMemFile(GetHost(), /* string("\\") + */ AddPath(GetPath(), pi_Path));  //!!!! //HChk BB
    }

/**----------------------------------------------------------------------------
 Returns a string containing the name of the resource without any path.
-----------------------------------------------------------------------------*/
WString HFCURLMemFile::GetFilename() const
    {
    WString::size_type LastSlashPos = m_Path.find_last_of(L"\\/");
    if (LastSlashPos != WString::npos)
        {
        if (LastSlashPos < (m_Path.length() - 1))
            return m_Path.substr(LastSlashPos + 1, m_Path.length() - LastSlashPos - 1);
        else
            return WString();
        }
    else
        return m_Path;
    }

/**----------------------------------------------------------------------------
 Returns a string containing the extension part of the name of the resource,
 which is the text after the last dot in its name.
-----------------------------------------------------------------------------*/
WString HFCURLMemFile::GetExtension() const
    {
    WString::size_type LastDotPos = m_Path.find_last_of(L".");
    if ((LastDotPos != WString::npos) && (LastDotPos < (m_Path.length() - 1)))
        return m_Path.substr(LastDotPos + 1, m_Path.length() - LastDotPos - 1);
    else
        return WString();
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
