//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLFile.h>

//:Ignore
// This is the creator that registers itself in the scheme list.
struct URLFileCreator : public HFCURL::Creator
    {
    URLFileCreator()
        {
        HFCURLFile::GetSchemeList().insert(HFCURLFile::SchemeList::value_type(HFCURLFile::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLFile(pi_URL);
        }
    } g_URLFileCreator;

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
HFCURLFile::HFCURLFile(const WString& pi_Host,
                       const WString& pi_Path)
    : HFCURL(s_SchemeName(), WString(L"//") + pi_Host + pi_Path),
      m_Host(pi_Host), m_Path(pi_Path)
    {
    FREEZE_STL_STRING(m_Host);
    FREEZE_STL_STRING(m_Path);
    }

/**----------------------------------------------------------------------------
 This constructor configures the object from the complete URL specification.

 Syntax :  @c{file:{//host/[path] | [//]drive/[path]}}

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
HFCURLFile::HFCURLFile(const WString& pi_URL)
    : HFCURL(pi_URL)
    {
    // Is there a double-slash at beginning?

    if (GetSchemeSpecificPart().length() >= 2)
        {
        WString::size_type Pos = 0;
        if ((GetSchemeSpecificPart().substr(Pos,2).compare(L"//") == 0) ||
            (GetSchemeSpecificPart().substr(Pos,2).compare(L"\\\\") == 0))
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
                bool HaveUNC = true;
                if (GetSchemeSpecificPart().substr(Pos, 2).compare(L"\\\\") == 0)
                    Pos += 2;
                else if (GetSchemeSpecificPart().substr(Pos, 2).compare(L"//") == 0)
                    Pos += 2;
                else
                    HaveUNC = false;

                WString::size_type SingleSlashPos =
                    GetSchemeSpecificPart().find_first_of(L"\\/", Pos);
                if (SingleSlashPos != WString::npos)
                    {
                    m_Host = GetSchemeSpecificPart().substr(2, SingleSlashPos - 2);
                    m_Path = GetSchemeSpecificPart().substr(SingleSlashPos + 1,
                                                            GetSchemeSpecificPart().length()-SingleSlashPos - 1);
                    }
                else
                    {   // we have only the host
                    m_Host = GetSchemeSpecificPart().substr(2,
                                                            GetSchemeSpecificPart().length()-2);
                    }

                // if the host contains no slash nor colon, it is not a drive spec but a
                // UNC host without the additional double-slash (ie: file://host/path)
                // Then we add a double-slash...

                WString::size_type SlashOrColonPos = m_Host.find_first_of(L"\\/:", 0);
                if (m_Host.length() != 0 && SlashOrColonPos == WString::npos)
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
HFCURLFile::~HFCURLFile()
    {
    // Nothing to do here.
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns the standardized and complete URL string.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLFile::GetURL() const
    {
    if (m_Host.length () != 0)
        return WString(s_SchemeName() + L"://") + m_Host + L"\\" + m_Path;
    else
        return WString(s_SchemeName() + L"://") + m_Path;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns the standardized and complete URL string.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLFile::GetAbsoluteFileName() const
    {
    if (m_Host.length () != 0)
        return m_Host + WCSDIR_SEPARATOR + m_Path;
    else
        return m_Path;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Returns true only if a relative path can be calculated from this URL and the
 specified one.

 @see HFCURL
-----------------------------------------------------------------------------*/
bool HFCURLFile::HasPathTo(HFCURL* pi_pURL)
    {
    if (HFCURL::HasPathTo(pi_pURL))
        return (((HFCURLFile*)pi_pURL)->GetHost() == GetHost());
    return false;
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Calculates a relative path that can be added to this URL in order to get
 the specified one.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLFile::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(HasPathTo(pi_pDest));
    return FindPath(GetPath(), ((HFCURLFile*)pi_pDest)->GetPath());
    }

/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 Creates a brand new URL descriptor that corresponds to this URL added with
 the specified relative path.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLFile::MakeURLTo(const WString& pi_Path)
    {
    return new HFCURLFile(GetHost(), /* string("\\") + */ AddPath(GetPath(), pi_Path));  //!!!! //HChk BB
    }

/**----------------------------------------------------------------------------
 Returns a string containing the name of the resource without any path.
-----------------------------------------------------------------------------*/
WString HFCURLFile::GetFilename() const
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
 Sets the file name of the URL.
-----------------------------------------------------------------------------*/
void HFCURLFile::SetFileName(const WString& pi_rFileName)
    {
    HPRECONDITION(pi_rFileName.find_last_of(L"\\/") == WString::npos);

    WString::size_type LastSlashPos = m_Path.find_last_of(L"\\/");
    if (LastSlashPos != WString::npos)
        {
        if (LastSlashPos < (m_Path.length() - 1))
            {
            m_Path = m_Path.substr(0, LastSlashPos + 1) + pi_rFileName;
            }
        else
            {
            m_Path = m_Path + pi_rFileName;
            }
        }
    else
        {
        m_Path = pi_rFileName;
        }
    }

/**----------------------------------------------------------------------------
 Returns a string containing the extension part of the name of the resource,
 which is the text after the last dot in its name.
-----------------------------------------------------------------------------*/
WString HFCURLFile::GetExtension() const
    {
    WString::size_type LastDotPos = m_Path.find_last_of(L".");
    if ((LastDotPos != WString::npos) && (LastDotPos < (m_Path.length() - 1)))
        return m_Path.substr(LastDotPos + 1, m_Path.length() - LastDotPos - 1);
    else
        return WString();
    }


/**----------------------------------------------------------------------------
 Creates the full path as contained in the URL. Non-existent directories will
 be created iteratively.
-----------------------------------------------------------------------------*/
bool HFCURLFile::CreatePath()
    {
    bool Result = true;

    // Extract the drive, Path and filename from URL
    WString Path(GetHost());
    Path += L"\\";
    Path += GetPath();

    // Replace all '\\' with '/'
    WString::size_type Pos;
    while ((Pos = Path.find(L'\\', 0)) != WString::npos)
        Path[Pos] = L'/';

    Pos = 0;
    // Parse each dir level to verify if it exists and if not, create it
    while (Result && (Pos < Path.size()) && (Pos != WString::npos))
        {
        // find the end of the current sub-dir level
        WString::size_type EndPos = Path.find(L'/', Pos);
        if (EndPos == WString::npos)
            EndPos = Path.size();

        // Create a sub-string of the current dir sub-level
        WString SubDir(Path, 0, EndPos);

        // If the sub-dir doesn't exist, create it
        if (!BeFileName::DoesPathExist(SubDir.c_str()))
            {
            // Any error means we're in trouble, since we already
            // know that the directory doesn't exist.
            if (BeFileName::CreateNewDirectory(SubDir.c_str()) != BeFileNameStatus::Success)
                Result = false;
            }

        // Proceed
        Pos = EndPos + 1;
        }

    return Result;
    }


#ifdef __HMR_DEBUG_MEMBER
//-----------------------------------------------------------------------------
// Test routine
//-----------------------------------------------------------------------------
void HFCURLFile::PrintState() const
    {
#   define out wcout

    out << "Object of type HFCURLFile" << endl;
    out << "Host = " << GetHost() << endl;
    out << "Path = " << GetPath() << endl;
    out << "Standardized URL = " << GetURL() << endl;
    }
#endif
