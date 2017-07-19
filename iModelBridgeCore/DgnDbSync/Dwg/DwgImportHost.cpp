/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgImportHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

// Microsoft's SDK needed to find the key path to the installed RealDWG component
#undef  MSI_INCLUDED
#if defined(BENTLEY_WIN32) && defined(DWGTOOLKIT_RealDwg)
    #define UNICODE
    #include    <Windows.h>
    #include    <Msi.h>         // MsiGetComponentPath
    #define MSI_INCLUDED
#endif

USING_NAMESPACE_DWGDB

BEGIN_DGNDBSYNC_DWG_NAMESPACE

// Autodesk provides below key path for finding the registry root key of the installed RealDWG component:
#if defined (_X64_)
static WChar                s_ODBXHOSTAPPREGROOT[] = L"{285CAB69-5CB7-240B-697E-996AA63B6415}";
#elif defined (_X86_)
static WChar                s_ODBXHOSTAPPREGROOT[] = L"{82C5BA96-C57B-42B0-96E7-996AA63B6415}";
#endif
static DwgImportHost*       s_dwgImportHostInstance = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::Initialize (DwgImporter& importer)
    {
    m_importer = &importer;
    m_registryRootKey.clear ();

    // we initialize the toolkitHost here because it depends on a valid m_importer:
    DwgImportHost::InitializeToolkit (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImportHost&  DwgImportHost::GetHost ()
    {
    if (nullptr == s_dwgImportHostInstance)
        {
        s_dwgImportHostInstance = new DwgImportHost ();
        }
    return  *s_dwgImportHostInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::_FatalError (WCharCP format, ...)
    {
    va_list     varArgs;
    va_start (varArgs, format);

    WString     errMessage = WPrintfString (format, varArgs);
    va_end (varArgs);

    m_importer->OnFatalError (DwgImporter::IssueCategory::ToolkitError(), DwgImporter::Issue::FatalError(), Utf8String(errMessage.c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::_Alert (WCharCP message) const
    {
    Utf8String  msg(message);
    // remove whilte spaces and linefeed etc from front & end of the string:
    msg.Trim ();
    m_importer->ReportError (DwgImporter::IssueCategory::ToolkitError(), DwgImporter::Issue::Error(), msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::_Message (WCharCP message, int numChars) const
    {
    // let MessageCenter handle the input string sent from the toolkit:
    if (nullptr != message && numChars > 0)
        m_importer->GetMessageCenter().ProcessInputMessage (message, numChars);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::_DebugPrintf (WCharCP format, ...) const
    {
    va_list     varArgs;
    va_start (varArgs, format);

    WString     message = WPrintfString (format, varArgs);
    va_end (varArgs);

    LOG.debug (message.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImportHost::FindXrefFile (WStringR outFile, WCharCP inFile, DwgDbDatabaseP dwg)
    {
    // if absolute path exists, return it!
    BeFileName  checkFile(inFile);
    if (checkFile.DoesPathExist())
        return  true;

    // get base path from the input or the root DWG file name
    WString     basePath;
    if (nullptr != dwg)
        basePath = BeFileName::GetDirectoryName (dwg->GetFileName().c_str());
    else if (nullptr != m_importer)
        basePath = BeFileName::GetDirectoryName (m_importer->GetRootDwgFileName().c_str());
    else
        return  false;
    
    // if looks like a relative path, try to resolve it
    if (checkFile.StartsWith(L"..") && BSISUCCESS == BeFileName::ResolveRelativePath(outFile, inFile, basePath.c_str()))
        return  true;

    // look for the file on the same folder of the base file
    WString     name, ext;
    checkFile.ParseName (nullptr, nullptr, &name, &ext);

    checkFile.SetName (basePath);
    checkFile.AppendToPath (name.c_str());
    checkFile.AppendExtension (ext.empty() ? L"dwg" : ext.c_str());

    if (checkFile.DoesPathExist())
        {
        outFile.assign (checkFile.c_str());
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImportHost::FindFontFile (WStringR outFile, WCharCP inFont, AcadFileType hint)
    {
    // if the full path exists, this is it!
    if (BeFileName::DoesPathExist(inFont))
        {
        outFile = inFont;
        return  true;
        }

    DgnFontType fontType = AcadFileType::CompiledShapeFile == hint ? DgnFontType::Shx : DgnFontType::TrueType;
    WString     fontName (inFont);
    fontName.Trim ();

    size_t      dot= fontName.find_last_of (L'.');
    if (WString::npos != dot)
        {
        if (AcadFileType::FontFile == hint)
            {
            // This font could be either an shx or a ttf - check for file extension.
            WString ext = fontName.substr (dot + 1, fontName.length() - dot - 1);
            fontType = 0 == ext.CompareToI(L"shx") ? DgnFontType::Shx : DgnFontType::TrueType;
            }
        fontName.erase (dot);
        }

    BeFileName  foundPath;
    if (m_importer->GetLoadedFonts().FindFontPath(foundPath, fontType, Utf8String(fontName.c_str())) && foundPath.DoesPathExist())
        {
        outFile.assign (foundPath.c_str());
        return  true;
        }

    // if input font name contains a path that does not exist, remove the path and try it again:
    size_t      backSlash = fontName.find_last_of (L'\\');
    if (WString::npos != backSlash)
        {
        fontName = fontName.substr (backSlash + 1, fontName.length() - backSlash - 1);
        if (m_importer->GetLoadedFonts().FindFontPath(foundPath, fontType, Utf8String(fontName.c_str())) && foundPath.DoesPathExist())
            {
            outFile.assign (foundPath.c_str());
            return  true;
            }
        }

    // if we get here, we cannot find the font - use an appropriate default font:
    if (DgnFontType::Shx == fontType && m_lastShxFontName.EqualsI(fontName.c_str()))
        {
        /*-------------------------------------------------------------------------------
        A RealDWG/AutoCAD problem:

        The input name fontIn can be an shx for text or an shx for shape.  There is no
        way we can tell which type of the shx it is looking for.  If we return a wrong
        type shx, the caller would enter into an infinite loop calling findFile until a
        right type of shx is returned.  ACAD does that by popping up a dialog box forcing
        user to manually pick a right file.  We are left with few choice.  AutoDesk offers
        no help on how to tell an shx for text vs one for shape, neither they have a way
        to stop the infinite loop!

        As a workaround, here we track and check for shx font name RealDWG has previously
        called for.  That shx file cannot be found, so we had returned our fallback shx
        file for text.  If now findFile is calling us for the same shx font again, we
        assume the shx font we had returned last time to be of a wrong type, so we now
        return an shx font for shape instead.  Not a reliable check but it seems to have
        stopped RealDWG's infinite loop of calling this method.  Ltypeshp.shx is the only
        shx file for shape delivered with ACAD and RealDWG.
        -------------------------------------------------------------------------------*/
        if (m_importer->GetFallbackFontPathForShape(foundPath))
            {
            outFile.assign (foundPath.c_str());
            m_lastShxFontName.clear ();
            return  true;
            }
        }
    else if (m_importer->GetFallbackFontPathForText(foundPath, fontType))
        {
        // use fallback font
        outFile.assign (foundPath.c_str());

        if (DgnFontType::Shx == fontType)
            m_lastShxFontName = fontName;

        return  true;
        }

    m_importer->ReportError (DwgImporter::IssueCategory::ToolkitError(), DwgImporter::Issue::FileNotFound(), inFont);

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgImportHost::_FindFile (WStringR fullpathOut, WCharCP filenameIn, DwgDbDatabaseP dwg, AcadFileType hint)
    {
    WCharP      pExtension = nullptr;
    WString     suggestedPath;

    switch (hint)
        {
        case AcadFileType::FontFile:
        case AcadFileType::CompiledShapeFile:
        case AcadFileType::TrueTypeFontFile:
            return  this->FindFontFile(fullpathOut, filenameIn, hint) ? DwgDbStatus::Success : DwgDbStatus::FileNotFound;

        case AcadFileType::FontMapFile:
            {
            pExtension = L".fmp";
            // NEEDSWORK - find font map file
            break;
            }

        case AcadFileType::PatternFile:
            {
            // NEEDSWORK - find .pat file
            return DwgDbStatus::FileNotFound;
            }

        case AcadFileType::ARXApplication:
            {
            pExtension = L".dbx";
#ifdef BENTLEY_WIN32
            // honor app's SetDllDirectory()
            WCHAR   dllDir[2000] = { 0 };
            DWORD   nChars = ::GetDllDirectoryW (2000, dllDir);
            if (nChars > 0 && nChars < 2000)
                suggestedPath.assign (dllDir, nChars);
#endif  // BENTLEY_WIN32
            break;
            }

        case AcadFileType::XRefDrawing:
            return  this->FindXrefFile(fullpathOut, filenameIn, dwg) ? DwgDbStatus::Success : DwgDbStatus::FileNotFound;

        case AcadFileType::EmbeddedImageFile:
        default:
            {
            pExtension = L"";
            break;
            }
        }

// NEEDSWORK - portable file search (Desktop::FileSystem::GetCwd not currently portable either).
#ifdef BENTLEY_WIN32
    LPCWSTR         fname = static_cast<LPCWSTR> (filenameIn);
    LPCWSTR         extname = static_cast<LPCWSTR> (pExtension);
    LPWSTR          filepart = nullptr;
    WCHAR           found[2000] = { 0 };
    static DWORD    s_maxChars = _countof (found);

    LPCWSTR         searchPath = nullptr;
    if (!suggestedPath.empty())
        searchPath = static_cast <LPCWSTR> (suggestedPath.c_str());

    DWORD   foundChars = ::SearchPathW (searchPath, fname, extname, s_maxChars, found, &filepart);
    if (foundChars > 0 && foundChars < s_maxChars)
        {
        fullpathOut.assign (reinterpret_cast<WCharCP>(found), foundChars);
        return  DwgDbStatus::Success;
        }
#endif

    return DwgDbStatus::FileNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImportHost::_GetAlternateFontName (WStringR altFont) const
    {
    altFont.assign (L"simplex.shx");
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         DwgImportHost::_GetRegistryProductRootKey (RootRegistry type)
    {
    if (type != RootRegistry::Machine)
        return  nullptr;

    if (m_registryRootKey.empty())
        {
        // try to get RealDWG's HKLM root key from config var REALDWG_REGISTRY_ROOTKEY set by a consumer:
        if (BSISUCCESS == ConfigurationManager::GetVariable(m_registryRootKey, REALDWG_REGISTRY_ROOTKEY))
            {
            if (LOG_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
                LOG.tracev ("Found HKLM root registry for RealDWG=%ls", m_registryRootKey.c_str());

            m_registryRootKey.ReplaceI(L"HKEY_LOCAL_MACHINE\\", L"");
            }
        else if (m_registryRootKey.empty())
            {
            // try to pick up a product GUID on which the RealDWG component is installed:
#ifdef MSI_INCLUDED
            WString productGuid;

            ConfigurationManager::GetVariable (productGuid, L"MS_PRODUCTCODEGUID");

            if (!productGuid.empty())
                {
                WChar   productPath[1024];
                DWORD   regLength = _countof (productPath);

                // find the full registry path of the installed RealDWG on the product:
                if (INSTALLSTATE_LOCAL == ::MsiGetComponentPath(productGuid.c_str(), s_ODBXHOSTAPPREGROOT, productPath, &regLength))
                    {
                    // make it a relative key path to be returned:
                    WCharCP   relativePath = wcschr (productPath, '\\');
                    if (NULL != relativePath && 0 != relativePath[0] && NULL != ++relativePath && 0 != relativePath[0])
                        m_registryRootKey = wcsdup (relativePath);
                    }
                }
#endif
            }
        }

    return m_registryRootKey.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
LCID            DwgImportHost::_GetRegistryProductLCID ()
    {
    // WIP - work with installer to set the registry key
    return 0x0409; // English
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         DwgImportHost::_Product () const
    {
    return  L"DwgImporter";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImportHost::_GetPassword (WCharCP dwgName, PasswordChoice choice, WCharP password, const size_t bufSize)
    {
    // WIP - get password
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportHost::NewProgressMeter ()
    {
    if (nullptr != m_progressMeter)
        delete m_progressMeter;

    DgnProgressMeterP   dgnMeter = &m_importer->GetProgressMeter ();

    m_progressMeter = new DwgProgressMeter (dgnMeter);

    DwgImportHost::SetWorkingProgressMeter (m_progressMeter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgProgressMeter::_Progress ()
    {
    if (nullptr != m_meter)
        m_meter->ShowProgress ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::MessageCenter::StartListMessageCollection (T_WStringVectorP out)
    {
    m_listMessageCollection = out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::MessageCenter::StopListMessageCollection ()
    {
    m_listMessageCollection = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::MessageCenter::DisplayAndFlush ()
    {
    // do not display a string with all white spaces
    m_displayMessage.Trim ();
    if (m_displayMessage.empty())
        return;

    if (nullptr != m_listMessageCollection)
        {
        // a LIST property has started message collection, add it to the collection:
        m_listMessageCollection->push_back (m_displayMessage);
        }
    else if (LOG_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        {
        LOG.infov ("From Toolkit: %ls", m_displayMessage.c_str());
        }

    // flush the string
    m_displayMessage.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::MessageCenter::ProcessInputMessage (WStringCR message)
    {
    WString     newString = message;
    size_t      newLineAt = WString::npos;
    size_t      newLineSize = newString.size ();

    /*-----------------------------------------------------------------------------------
    Find each and every LINEFEED in the input string, truncate and combine it with
    previously saved strings, then display them.  Leave the ending LINEFEED as the default
    process outside of the loop.
    -----------------------------------------------------------------------------------*/
    while (WString::npos != (newLineAt = newString.find(CHAR_LineFeed)) && newLineAt < (newLineSize-1))
        {
        WString endString;

        if (newLineAt > 0)
            {
            endString = newString.substr (0, newLineAt);
            m_displayMessage += endString.substr (0, newLineAt);
            }

        if (!m_displayMessage.empty())
            this->DisplayAndFlush ();

        newLineAt++;

        newLineSize -= newLineAt;

        newString = newString.substr (newLineAt, newLineSize);
        }

    m_displayMessage += newString.substr (0, newLineSize);

    // if the last line ends with a LINEFEED, display and flush
    if (CHAR_LineFeed == newString.at(newLineSize-1) || 0 == newString.at(newLineSize-1))
        this->DisplayAndFlush ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::MessageCenter::ProcessInputMessage (WCharCP message, int numChars)
    {
    if (numChars == 1)
        {
        // check the signal of an end of a full message collection
        bool    endStringFeed = *message == 0 || *message == CHAR_LineFeed;

        if (endStringFeed && !m_displayMessage.empty())
            this->DisplayAndFlush ();
        else
            m_displayMessage += *message;
        }
    else if (numChars < (int)wcslen(message) || !m_displayMessage.empty())
        {
        // start or continue collection of piecewise messages
        this->ProcessInputMessage (WString(message, numChars));
        }
    else
        {
        // the input message may be displayed by itself, if not all white spaces
        m_displayMessage.assign (message, numChars);
        this->DisplayAndFlush ();
        }
    }

END_DGNDBSYNC_DWG_NAMESPACE
