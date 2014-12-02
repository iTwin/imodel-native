/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/DgnDocumentManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "../DgnCore/DgnCoreLog.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* ** util_findFile
* short *handle <= IGNORED
* char *outName, <= complete file spec. drive+path+file+extension. must be MAXFILELENGTH chars long or NULL
* char *inName, => Any portion of full file spec. Always used first. Should be root file name (or NULL)
* char *envvar, => Any portion of full file spec. Always used second. May contain multiple parts of file specs
* separated by sepchar. (or NULL). Cfg vars are IGNORED!
* char *defaultFile, => IGNORED
* int option => Option flags. IGNORED
* @bsimethod                                                    JVB             02/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      util_findFile
(
BeFile          *unusedFile,           /* <= File handle                          */
wchar_t         *outName,           /* <= Output file name                     */
const wchar_t   *inName,            /* => Input file name                      */
const wchar_t   *envvar,            /* => List of related names/logicals       */
const wchar_t   *defaultFileName,   /* => Default name and/or extension        */
int             option              /* => Open mode / search options           */
)
    {
    BeAssert (unusedFile == NULL);
    BeAssert (defaultFileName == NULL);
    BeFileName basename (BeFileName::NameAndExt, inName);

    if (BeFileName(inName).IsAbsolutePath() && BeFileName::DoesPathExist (inName))
        {
        wcscpy (outName, inName);
        return SUCCESS;
        }
        
    if (NULL != envvar)
        {
        WString paths (envvar);
        size_t curr = 0;
        do
            {
            size_t start = paths.find_first_not_of (PATH_SEPARATOR_CHAR, curr);
                   curr  = paths.find (PATH_SEPARATOR_CHAR, start);

            if (curr == start)
                break;

            BeFileName dir (BeFileName::DevAndDir, paths.substr (start, curr-start).c_str());   // envvar is usually the filename of the parent file. Use the directory portion only.

            if (0 == BeStringUtilities::Wcsicmp (dir, L"MS_RFDIR") || !BeFileName::DoesPathExist (dir))  // We don't support cfgvars!
                {
                LOG.infov(L"Cfgvar '%ls' not supported when resolving monikers (e.g., reference attachments)", dir.GetName());
                continue;
                }

            BeFileName path (NULL, dir, basename, NULL);
            if (BeFileName::DoesPathExist (path))
                {
                wcscpy (outName, path);
                return  SUCCESS;
                }

            } while (curr<WString::npos);
        }

    *outName = 0;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
// *** WIP_IMPORTER -- Do we need this logic for DgnDb??
static StatusInt parsePackagedName (WStringP packageName, Int32* embedId, WStringP embedName, WCharCP inputName)
    {
    WCharCP embedStart = ::wcschr (inputName, '<');
    if (NULL == embedStart)
        return  ERROR;

    WCharCP embedEnd  = ::wcschr (embedStart, '>');
    if (NULL == embedEnd)
        return ERROR;

    if (packageName)
        *packageName = WString (inputName, embedStart);

    if (embedName)
        embedName->assign (embedEnd+1);

    if (embedId)
        {
        if (1 == embedEnd - embedStart)
            {
            *embedId = -1;
            }
        else
            {
            WString embedStr (embedStart+1, embedEnd);
            if (1 != BE_STRING_UTILITIES_SWSCANF (embedStr.c_str(), L"%d", embedId))
                return ERROR;
            }
        }

    return SUCCESS;
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//// Storing DgnDocumentMoniker as XML  //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
static const Utf8CP XMLTAG_DocumentMonikerRoot     = "MSDocMoniker";
static const Utf8CP XMLTAG_FileName                = "FileName";
static const Utf8CP XMLTAG_FullPath                = "FullPath";
static const Utf8CP XMLTAG_ProviderId              = "ProviderId";
static const Utf8CP XMLTAG_Relative                = "isRelative";
static const Utf8CP XMLTAG_FolderMonikerRoot       = "MSFolderMoniker";
static const Utf8CP XMLTAG_FolderName              = "FolderName";
// static const Utf8CP XMLTAG_EmbeddedFile            = "EmbeddedFile";
// static const Utf8CP XMLTAG_PackageFileName         = "PackageFileName";
// static const Utf8CP XMLTAG_PackageFullPath         = "PackageFullPath";
// static const Utf8CP XMLTAG_EmbedName               = "EmbedName";

/*=================================================================================**//**
* @bsiclass                                                     YogeshSajanikar 10/10
+===============+===============+===============+===============+===============+======*/
struct PathUtil
    {
    static bool IsSeparator (WChar inChar);
    static void EnsureSeparatorAtEnd (WStringR wFolderName);
    static void StripSeparatorAtEnd (WStringR wFolderName);
    };


/*=================================================================================**//**
* @bsiclass                                                     YogeshSajanikar 10/10
+===============+===============+===============+===============+===============+======*/
struct DgnMonikerXMLSupport
    {
protected:
    Utf8CP XMLTag_Root;
    Utf8CP XMLTag_FileNode;
    Utf8CP XMLTag_RelativeAttr;
    Utf8CP XMLTag_FullPathNode;
    Utf8CP XMLTag_ProviderId;

    DgnMonikerXMLSupport (Utf8CP rootNode, Utf8CP fileNode, Utf8CP relativeAttr = XMLTAG_Relative, Utf8CP fullPathNode = XMLTAG_FullPath, Utf8CP providerIdNode = XMLTAG_ProviderId)
        : XMLTag_Root (rootNode), XMLTag_FileNode (fileNode), XMLTag_RelativeAttr (relativeAttr), XMLTag_FullPathNode (XMLTAG_FullPath), XMLTag_ProviderId (XMLTAG_ProviderId)
    {}

public:
    bool ToXML (WString &xmlString, WCharCP fileName, WCharCP providerId, WCharCP fullPath, bool relative, WCharCP customString);

    void FromXML (WString &fileName, WString &providerId, WString &fullPath, bool &relativeFlag, WString &customString, WCharCP xmlString);
    };

/*=================================================================================**//**
* @bsiclass                                                     YogeshSajanikar 10/10
+===============+===============+===============+===============+===============+======*/
struct DgnDocumentMonikerXMLSupport : public DgnMonikerXMLSupport
    {
    DgnDocumentMonikerXMLSupport ()
        : DgnMonikerXMLSupport (XMLTAG_DocumentMonikerRoot, XMLTAG_FileName)
    {}

    };

/*=================================================================================**//**
* @bsiclass                                                     YogeshSajanikar 10/10
+===============+===============+===============+===============+===============+======*/
struct DgnFolderMonikerXMLSupport : public DgnMonikerXMLSupport
    {
    DgnFolderMonikerXMLSupport ()
        : DgnMonikerXMLSupport (XMLTAG_FolderMonikerRoot, XMLTAG_FolderName)
    {}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMonikerXMLSupport::FromXML (WStringR fileName, WStringR providerId, WStringR fullPath, bool& relativeFlag, WStringR customString, WCharCP xmlString)
    {
    if (WString::IsNullOrEmpty (xmlString))
        return;

    BeXmlStatus              xmlStatus;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (xmlStatus, xmlString, wcslen (xmlString));
    if (BEXML_Success != xmlStatus)
        return;
    
    BeXmlWriterPtr customStringWriter = BeXmlWriter::Create ();
    if (!customStringWriter.IsValid ())
        return;
    
    WString     relativeStr         = L"false";
    WStringP    currentNodeString   = NULL;

    while (BeXmlReader::READ_RESULT_Success == reader->Read ())
        {
        switch (reader->GetCurrentNodeType ())
            {
            case BeXmlReader::NODE_TYPE_Element:
                {
                currentNodeString = NULL;

                Utf8String currentNodeName;
                if (BEXML_Success != reader->GetCurrentNodeName (currentNodeName))
                    return;
                
                if (0 == strcmp (XMLTag_FileNode, currentNodeName.c_str ()))
                    currentNodeString = &fileName;
                else if (0 == strcmp (XMLTag_FullPathNode, currentNodeName.c_str ()))
                    currentNodeString = &fullPath;
                else if (0 == strcmp (XMLTag_ProviderId, currentNodeName.c_str ()))
                    currentNodeString = &providerId;
                else
                    customStringWriter->AddFromReader (reader->GetReader ());
                }
                break;

            case BeXmlReader::NODE_TYPE_Attribute:
                // I don't understand this code, I think it was wrong, so I set up an BeAssert so it cao be analyzed in the future.
                // here is what it was, but it's comparing whatever happens to be in currentNodeString with an XML tag. That can't be right.
                // if ((NULL != currentNodeString) && (0 == wcscmp (XMLTag_FileNode, currentNodeString->c_str ())))
                BeAssert (false);
                if ( (NULL != currentNodeString) && (currentNodeString == &fileName) )
                    currentNodeString = &relativeStr;
                else 
                    customStringWriter->AddFromReader (reader->GetReader ());
                break;

            case BeXmlReader::NODE_TYPE_Text:
                if (NULL != currentNodeString)
                    {
                    WString readerValue;
                    reader->GetCurrentNodeValue (readerValue);
                    currentNodeString->append (readerValue);
                    }
                else
                    {
                    customStringWriter->AddFromReader (reader->GetReader ());
                    }
                
                break;

            case BeXmlReader::NODE_TYPE_EndElement:
            default:
                if (NULL == currentNodeString)
                    customStringWriter->AddFromReader (reader->GetReader ());
                
                break;
            }
        }
    
    relativeFlag = (0 == BeStringUtilities::Wcsicmp (L"true", relativeStr.c_str ()));

    customStringWriter->ToString (customString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
static bool monikerDataToXml
(
WStringR    xmlString,
Utf8CP      documentTag,
WCharCP     fileName,       Utf8CP fileNameTag,
bool        isRelative,     Utf8CP isRelativeTag,
WCharCP     fullPath,       Utf8CP fullPathTag,
WCharCP     providerId,     Utf8CP providerTag,
WCharCP     customString
)
    {
    BeXmlWriterPtr writer = BeXmlWriter::Create ();
    if (!writer.IsValid ())
        return false;

    writer->WriteElementStart (documentTag);

    // Add the data nodes to the root
    if (NULL != fileName)
        {
        writer->WriteElementStart (fileNameTag);
        if (isRelative)
            writer->WriteAttribute (isRelativeTag, L"true");

        writer->WriteText (fileName);
        writer->WriteElementEnd ();
        }

    if (NULL != fullPath)
        {
        writer->WriteElementStart (fullPathTag);
        writer->WriteText (fullPath);
        writer->WriteElementEnd ();
        }

    if (NULL != providerId)
        {
        writer->WriteElementStart (providerTag);
        writer->WriteText (providerId);
        writer->WriteElementEnd ();
        }

    if (NULL != customString)
        {
        // Custom string is an XML that should be appended as it is. 
        // **** Note: No XML validation is done.
        writer->WriteRaw (customString);
        }

    writer->WriteElementEnd ();
    
    writer->ToString (xmlString);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 10/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnMonikerXMLSupport::ToXML 
(
WStringR    xmlString, 
WCharCP     fileName, 
WCharCP     providerId, 
WCharCP     fullPath, 
bool        relative, 
WCharCP     customString
)
    {
    return monikerDataToXml (
            xmlString,
            XMLTag_Root,
            fileName,       XMLTag_FileNode,
            relative,       XMLTag_RelativeAttr,
            fullPath,       XMLTag_FullPathNode,
            providerId,     XMLTag_ProviderId,
            customString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool buildMonikerXMLDom (WString &xmlString, WCharCP fileName, WCharCP providerId, WCharCP fullPath, bool relative, WCharCP customString)
    {
    return monikerDataToXml (
            xmlString,
            XMLTAG_DocumentMonikerRoot,
            fileName,                   XMLTAG_FileName,
            relative,                   XMLTAG_Relative,
            fullPath,                   XMLTAG_FullPath,
            providerId,                 XMLTAG_ProviderId,
            customString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_Externalize () const
    {
    WString externalizedString;
    buildMonikerXMLDom (externalizedString, GetString(STRINGID_PortableName), GetString(STRINGID_ProviderId), GetString(STRINGID_FileName), GetIsRelative(), GetString (STRINGID_Custom));
    return externalizedString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr DgnDocumentMoniker::Create (WCharCP externalizedState, WCharCP basePath, bool fullPathFirst)
    {
    BeXmlStatus             xmlStatus;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (xmlStatus, externalizedState, wcslen (externalizedState));
    if (BEXML_Success != xmlStatus)
        return NULL;
    
    // Find the root.
    if (BeXmlReader::READ_RESULT_Success != reader->ReadTo (BeXmlReader::NODE_TYPE_Element, XMLTAG_DocumentMonikerRoot, false, NULL))
        return NULL;

    bool    isRelative      = false;
    WString portableName;
    WString savedFullPath;
    WString providerId;
    WString customStrings;
    
    // Process all first-level children.
    while (BeXmlReader::READ_RESULT_Success == reader->ReadTo (BeXmlReader::NODE_TYPE_Element))
        {
        Utf8String currNodeName;
        if ((BEXML_Success == reader->GetCurrentNodeName (currNodeName)) && (!currNodeName.empty ()))
            {
            if (0 == strcmp (XMLTAG_FileName, currNodeName.c_str ()))
                {
                Utf8String attributeName;
                while (BEXML_Success == reader->ReadToNextAttribute (&attributeName, (Utf8StringP)NULL))
                    {
                    if (0 != strcmp (XMLTAG_Relative, attributeName.c_str ()))
                        continue;

                    Utf8String attributeValue;
                    if ((BEXML_Success != reader->GetCurrentNodeName (attributeName)) || attributeName.empty ())
                        return NULL;
                    
                    isRelative = (0 == BeStringUtilities::Stricmp ("true", attributeValue.c_str ()));
                    }
                
                if (BeXmlReader::NODE_TYPE_Text != reader->GetCurrentNodeType ())
                    reader->ReadTo (BeXmlReader::NODE_TYPE_Text, NULL, true, NULL);
                
                if ((BEXML_Success != reader->GetCurrentNodeValue (portableName)) || portableName.empty ())
                    return NULL;
                }
            else if (0 == strcmp (XMLTAG_FullPath, currNodeName.c_str ()))
                {
                // Don't read past the current element looking for NODE_TYPE_Text (e.g. element with no content). If not SUCCESS, this leaves the reader on the NODE_TYPE_EndElement, so bypass ReadToEndOfCurrentElement below.
                if (BeXmlReader::READ_RESULT_Success != reader->ReadTo (BeXmlReader::NODE_TYPE_Text, NULL, true, &savedFullPath))
                    continue;
                }
            else if (0 == strcmp (XMLTAG_ProviderId, currNodeName.c_str ()))
                {
                // Don't read past the current element looking for NODE_TYPE_Text (e.g. element with no content). If not SUCCESS, this leaves the reader on the NODE_TYPE_EndElement, so bypass ReadToEndOfCurrentElement below.
                if (BeXmlReader::READ_RESULT_Success != reader->ReadTo (BeXmlReader::NODE_TYPE_Text, NULL, true, &providerId))
                    continue;
                }
            else
                {
                WString outerXml;
                if (BEXML_Success == reader->GetCurrentNodeOuterXml (outerXml))
                    customStrings.append (outerXml);
                }
            }

        reader->ReadToEndOfCurrentElement ();
        }

    return DgnDocumentManager::GetManager ()._CreateMoniker (portableName.c_str (), savedFullPath.c_str (), providerId.c_str (), isRelative, basePath, fullPathFirst, customStrings.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/10
//---------------------------------------------------------------------------------------
DgnDocumentMonikerPtr DgnDocumentMoniker::Clone (DgnDocumentMonikerCR source)
    {
    return new DgnDocumentMoniker (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    09/07
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnDocumentManager::_FolderMonikerToNewBasePath (WCharCP sourceMoniker, WCharCP sourceBasePath, WCharCP destBasePath)
    {
    // Assuming that this is rare enough to use a DOM.
    BeXmlStatus             xmlStatus;
    BeXmlDomPtr   dom             = BeXmlDom::CreateAndReadFromString (xmlStatus, sourceMoniker, wcslen (sourceMoniker));
    if (BEXML_Success != xmlStatus)
        {
        BeAssert (false);
        return sourceMoniker;
        }
    
    BeXmlNodeP              folderNameNode  = NULL;
    if (BEXML_Success != dom->SelectChildNodeByName (folderNameNode, *dom->GetRootElement(), XMLTAG_FolderName, BeXmlDom::NODE_BIAS_FailIfMultiple))
        {
        BeAssert (false);
        return sourceMoniker;
        }
    
    WString folderName;
    folderNameNode->GetContent (folderName);
    if (folderName.empty ())
        {
        BeAssert (false);
        return sourceMoniker;
        }
    
    bool isSourceRelative = false;

    WString isSourceRelativeString;
    if ((BEXML_Success == folderNameNode->GetAttributeStringValue (isSourceRelativeString, XMLTAG_Relative)) && !isSourceRelativeString.empty ())
        isSourceRelative = (0 == BeStringUtilities::Wcsicmp (L"true", isSourceRelativeString.c_str ()));

    if (!isSourceRelative)
        return sourceMoniker;

    WString resolvedPath;

    if (SUCCESS == BeFileName::ResolveRelativePath (resolvedPath, folderName.c_str (), sourceBasePath))
        {
        WString destRelative;
        BeFileName::FindRelativePath (destRelative, resolvedPath.c_str(), destBasePath);
        
        folderNameNode->SetContent (destRelative.c_str());
        
        WString domText;
        dom->ToString (domText, (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_OmitXmlDeclaration));
        
        return domText;
        }
    
    return sourceMoniker;
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//// FindFile helper functions ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Yogesh.Sajanikar                08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMoniker::SearchStatus DgnBaseMoniker::SearchRelativeForFile
(
WCharP        outFoundFileName,
WCharCP       inFileName,
WCharCP       basePath,
WCharCP       environmentVariable,
int             searchOption
)
    {
    /*--------------------------------------------------------------------------
       Search relative to the base path.
    --------------------------------------------------------------------------*/
    if (NULL != basePath && '\0' != basePath[0])
        {
        if (SUCCESS == util_findFile (NULL, outFoundFileName, inFileName, basePath, NULL, UF_NO_CUR_DIR|searchOption))
            return SEARCH_FoundRelative;
        }

    /*--------------------------------------------------------------------------
       Search relative to the environment variable.
    --------------------------------------------------------------------------*/
    if (NULL != environmentVariable && '\0' != environmentVariable[0])
        {
        if (SUCCESS == util_findFile (NULL, outFoundFileName, inFileName, environmentVariable, NULL, UF_NO_CUR_DIR|searchOption))
            return SEARCH_FoundEnvironment;
        }

    return SEARCH_Failed;
    }

struct FileNameBuf
    {
    WChar m_name[MAX_PATH];
    WCharP GetNamePtr(){return m_name;}
    WCharCP GetName(){return m_name;}
    void Clear() {m_name[0] = 0;}
    operator WCharCP() const {return m_name;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnBaseMoniker::SearchForFile (SearchStatus& searchStatus, WCharCP inFileName, WCharCP fullPath, WCharCP basePath, bool fullPathFirst, bool searchAsFolder)
    {
    FileNameBuf      foundFile;
    BeFileName      fileName;
    bool            skipRelativeSearches = false;
    bool            fullPathSupplied     = (NULL != fullPath) && ('\0' != fullPath[0]);
    int             folderOption         = searchAsFolder ? UF_FIND_FOLDER : 0;

    searchStatus = SEARCH_Failed;

    // *** basePath can embed many environment variables. If you really want to treat the last one as a special case, you can easily
    //        parse and remove it from the end of basePath.
    WCharCP environmentVariable = NULL;

    /*--------------------------------------------------------------------------
       Search using full path first if requested.
    --------------------------------------------------------------------------*/
    if (fullPathFirst && fullPathSupplied)
        {
        if (SUCCESS == util_findFile (NULL, foundFile.GetNamePtr(), fullPath, NULL, NULL, UF_NO_CUR_DIR|folderOption))
            searchStatus = SEARCH_FoundFull;
        }

    /*--------------------------------------------------------------------------
       Treat the inFileName as fully qualified.
    --------------------------------------------------------------------------*/
    if (SEARCH_Failed == searchStatus)
        {
        WString device;
        WString fileNameWOExtension;
        WString fileExtension;

        BeFileName::ParseName (&device, NULL, &fileNameWOExtension, &fileExtension, inFileName);
        fileName.BuildName (NULL, NULL, fileNameWOExtension.c_str(), fileExtension.c_str());

        /*--------------------------------------------------------------------------
           If the provided name has a logical device skip all relative searches.
        --------------------------------------------------------------------------*/
        if (device.size() > 1)
            {
            skipRelativeSearches = true;

            if (SUCCESS == util_findFile (NULL, foundFile.GetNamePtr(), inFileName, NULL, NULL, UF_NO_CUR_DIR|folderOption))
                searchStatus = SEARCH_FoundLogical;
            }
        else
            {
            if (SUCCESS == util_findFile (NULL, foundFile.GetNamePtr(), inFileName, NULL, NULL, UF_NO_CUR_DIR|folderOption))
                searchStatus = SEARCH_FoundComplete;
            }
        }

    /*--------------------------------------------------------------------------
       Search for relative path using portable name.
    --------------------------------------------------------------------------*/
    if (SEARCH_Failed == searchStatus && !skipRelativeSearches)
        searchStatus = SearchRelativeForFile (foundFile.GetNamePtr (), inFileName, basePath, environmentVariable, folderOption);

    /*--------------------------------------------------------------------------
       Search for relative path using file name. Skipped if portable name and
       file name are same.
    --------------------------------------------------------------------------*/
    if ( (SEARCH_Failed == searchStatus) &&
         (L'\0'         != *(fileName.GetName ())) &&
         (0 != BeStringUtilities::Wcsicmp (fileName.GetName(), inFileName)))
        searchStatus = SearchRelativeForFile (foundFile.GetNamePtr (), fileName.GetName(), basePath, environmentVariable, folderOption);

    /*--------------------------------------------------------------------------
       Search using full path as a last resort.
    --------------------------------------------------------------------------*/
    if (SEARCH_Failed == searchStatus && fullPathSupplied && ( ! fullPathFirst) )
        {
        if (SUCCESS == util_findFile (NULL, foundFile.GetNamePtr(), fullPath, NULL, NULL, UF_NO_CUR_DIR|folderOption))
            searchStatus = SEARCH_FoundFull;
        }

    if (SEARCH_Failed == searchStatus)
        foundFile.Clear();

    return foundFile.GetNamePtr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static WString findFile
(
DgnDocumentMoniker::SearchStatus&   searchStatus,
WCharCP       inFileName,
WCharCP       fullPath,
WCharCP       basePath,
bool            fullPathFirst
)
    {
    WString foundFile = DgnBaseMoniker::SearchForFile (searchStatus, inFileName, fullPath, basePath, fullPathFirst, false);

    if (searchStatus > DgnDocumentMoniker::SEARCH_Failed)
        {
        // Find file did find the file, we need to ascertain that it is indeed a file and not a directory.
        if (BeFileName::IsDirectory (foundFile.c_str()))
            {
            foundFile.clear ();
            searchStatus = DgnDocumentMoniker::SEARCH_Failed;
            }
        }
    return foundFile;
    }
/*----------------------------------------------------------------------------------
  Essentially moniker represents a path to either a file or folder on
  a given system. A system may be a file system or it may be a virtual
  file system as it exists on ProjectWise. The DgnBaseMoniker represents
  the path to the resource on either systems. The classes
  DgnDocumentMoniker and DgnFolderMoniker exist because we may want to
  differentiate between a document and a folder and also acknowledge
  that the folder represents merely a hierarchical model to store documents.
  ----------------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////////////////////
//// FindFolder helper functions //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool PathUtil::IsSeparator (WChar inChar)
    {
    return (WCSDIR_SEPARATOR_CHAR     == inChar) || 
           (WCSALT_DIR_SEPARATOR_CHAR == inChar);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    09/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PathUtil::EnsureSeparatorAtEnd (WStringR wFolderName)
    {
    WString::reverse_iterator rit = wFolderName.rbegin ();
    if (wFolderName.empty () || !IsSeparator (*rit))
        wFolderName.push_back (WCSDIR_SEPARATOR_CHAR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    09/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PathUtil::StripSeparatorAtEnd (WStringR wFolderName)
    {
    if (wFolderName.empty())
        return;

    WString::iterator it = wFolderName.end ();
    WString::reverse_iterator rit = wFolderName.rbegin ();

    for (; rit != wFolderName.rend (); ++rit)
        {
        if (IsSeparator (*rit))
            it--;
        }

    wFolderName.erase (it, wFolderName.end ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString findFolder
(
DgnBaseMoniker::SearchStatus& searchStatus,
WCharCP       inFolderName,
WCharCP       fullPath,
WCharCP       searchPath,
bool            fullPathFirst
)
    {
    WString     defaultFileName (inFolderName);
    WString     foundDir;
    PathUtil::StripSeparatorAtEnd (defaultFileName);

    // The folder name may be a drive. Search for that possibility.
    WString     folderName;
    WString     folderNameExt;
    WString     folderDevice;

    BeFileName::ParseName (&folderDevice, NULL, &folderName, &folderNameExt, defaultFileName.c_str());
    if (!folderDevice.empty() && folderName.empty() && folderNameExt.empty())
        {
        if (BeFileName::IsDirectory (folderDevice.c_str ()))
            {
            searchStatus = DgnBaseMoniker::SEARCH_FoundComplete;

            BeFileName driveName (folderDevice.c_str(), NULL, NULL, NULL);

            foundDir     = WString (driveName.GetName());

            PathUtil::EnsureSeparatorAtEnd (foundDir);
            return foundDir;
            }
        }

    foundDir = DgnBaseMoniker::SearchForFile (searchStatus, defaultFileName.c_str (), fullPath, searchPath, fullPathFirst, true);

    // Strip off the dot from the found file character.
    if (searchStatus > DgnBaseMoniker::SEARCH_Failed)
        {
        // Find file did find the file, we need to ascertain that it is indeed a folder.
        if (!BeFileName::IsDirectory (foundDir.c_str()))
            {
            foundDir.clear ();
            searchStatus = DgnBaseMoniker::SEARCH_Failed;
            return foundDir;
            }

        PathUtil::EnsureSeparatorAtEnd (foundDir);
        return foundDir;
        }

    return foundDir;
    }

//////

///////////////////////////////////////////////////////////////////////////////////////////////
//// DgnDocumentMoniker ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMoniker::DgnDocumentMoniker (WCharCP portableName, WCharCP fullPath, WCharCP providerId, bool isRelative, WCharCP basePath, bool fullPathFirst, WCharCP customStrings)
    :
    m_flags (0)
    //m_strings ()        // initialize to 0's
    {
    memset (m_strings, 0, sizeof(m_strings));
    if (NULL == fullPath)   // See Note "Cache filename"
        fullPath = portableName;
    SetString (STRINGID_FileName, fullPath);
    SetString (STRINGID_PortableName, portableName, true);
    SetString (STRINGID_ProviderId, providerId);
    SetString (STRINGID_SearchPath, basePath);
    SetIsRelative (isRelative);
    SetFullPathFirst (fullPathFirst);
    SetSearchStatus (SEARCH_NotAttempted);
    SetString (STRINGID_Custom, customStrings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMoniker& DgnDocumentMoniker::operator=(DgnDocumentMoniker const& rhs)
    {
    m_flags = rhs.m_flags;
    for (int i=0; i<STRINGID_Count__; ++i)
        SetString ((StringId)i, rhs.m_strings[i]);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMoniker::DgnDocumentMoniker (DgnDocumentMoniker const& rhs)
    {
    m_flags = 0;
    memset (m_strings, 0, sizeof(m_strings));
    *this = rhs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMoniker::~DgnDocumentMoniker ()
    {
    for (int i=0; i<STRINGID_Count__; ++i)
        SetString ((StringId)i, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnDocumentMoniker::SetString (StringId id, WCharCP newStr, bool storeEmptyString) const
    {
    WCharP& str = m_strings[id];

    if (str == newStr)
        return;

    if (str != NULL)
        {
        free (str);
        str = NULL;
        }

    if (newStr != NULL && (storeEmptyString || *newStr != '\0'))
        str = BeStringUtilities::Wcsdup (newStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP       DgnDocumentMoniker::GetString (StringId id) const
    {
    wchar_t const* p = m_strings[id];
    return p? p: L"";
    }

void    DgnDocumentMoniker::_UpdateSavedFileName (WCharCP fullPath)       {SetString (STRINGID_FileName, fullPath);}
WString DgnDocumentMoniker::_GetSavedFilePath () const                      {return GetString (STRINGID_FileName);}
WString DgnDocumentMoniker::_GetProviderId () const                         {return GetString (STRINGID_ProviderId);}
void    DgnDocumentMoniker::SetSearchPath (WCharCP searchPath)            {SetString (STRINGID_SearchPath, searchPath);}
void    DgnDocumentMoniker::_SetParentSearchPath (WCharCP newPath)        {SetSearchPath (newPath);}
WString DgnDocumentMoniker::_GetPortableName () const                       {return GetString (STRINGID_PortableName);}
WString DgnDocumentMoniker::_GetParentSearchPath () const                   {return GetString (STRINGID_SearchPath);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_GetShortDisplayName () const
    {
    WString ext, name;
    BeFileName::ParseName (NULL, NULL, &name, &ext, _GetPortableName().c_str());

    BeFileName fileName (NULL, NULL, name.c_str(), ext.c_str());

    return fileName.GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnDocumentMoniker::IsPackagedFileDocMoniker () const
    {
    WString packageFileName = GetString (STRINGID_FileName);
    
    int embeddedID = 0;
    return SUCCESS == parsePackagedName (NULL, &embeddedID, NULL, packageFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_ResolveFileName (StatusInt* statusOut, bool dontRetryIfFailed) const
    {
    if (GetSearchStatus() > SEARCH_Failed)
        {
        if (NULL != statusOut)
            *statusOut = SUCCESS;
        return GetString (STRINGID_FileName);
        }

    StatusInt _status;
    StatusInt& status = (NULL == statusOut)? _status: *statusOut;

    status = DGNOPEN_STATUS_FileNotFound;

    if (SEARCH_Failed == GetSearchStatus() && !dontRetryIfFailed)
        SetSearchStatus (SEARCH_NotAttempted);
    
    if (IsPackagedFileDocMoniker())
        {
        status = SUCCESS;
        SetSearchStatus (SEARCH_FoundRelative);//Todo get the right status;
        return GetString (STRINGID_FileName);
        }

    // If we've never searched before, do the search now and cache the result.
    if (SEARCH_NotAttempted == GetSearchStatus())
        {
        SearchStatus searchStatus;

        WString foundFullPath = findFile (searchStatus, GetString (STRINGID_PortableName), GetString (STRINGID_FileName), GetString(STRINGID_SearchPath), GetFullPathFirst());

        SetSearchStatus (searchStatus);

        if (SEARCH_Failed < searchStatus)
            SetString (STRINGID_FileName, foundFullPath.c_str());
        }

    // If this moniker has ever been searched successfully, the result is cached in the moniker string.
    if (SEARCH_Failed >= GetSearchStatus())
        return L"";

    status = SUCCESS;
    return GetString (STRINGID_FileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            getCompareFilename (WStringR nm, DgnDocumentMoniker const& rhs)
    {
    nm = rhs.GetPortableName();
    if (nm.empty())
        {
        nm = rhs.GetSavedFileName();
        if (nm.empty())
            nm = rhs.Externalize();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int             DgnDocumentMoniker::Compare (DgnDocumentMoniker const& rhs) const
    {
    StatusInt   status1, status2;
    WString string1 = ResolveFileName (&status1);
    WString string2 = rhs.ResolveFileName (&status2);

    if (SUCCESS == status1 && SUCCESS == status2) // common case: both are resolved
        return BeStringUtilities::Wcsicmp (string1.GetWCharCP (), string2.GetWCharCP ());

    //  One or both is unresolved.
    //  NB: If a moniker is resolved, then we must use its resolved filename for all comparisons. We cannot
    //      sometimes use its resolved filename and sometimes use its saved or portable name or its externalized state.
    //      Otherwise, there's no unique ordering among monikers, and the tree will be incorrect.
    if (SUCCESS != status1)
        getCompareFilename (string1, *this);
    if (SUCCESS != status2)
        getCompareFilename (string2, rhs);

    return BeStringUtilities::Wcsicmp (string1.GetWCharCP (), string2.GetWCharCP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static int safe_wcscmp(WCharCP first, WCharCP second, bool ignore_case = true)
    {
    if (NULL == first && NULL == second)
        return 0;
  
    else if (NULL == first && NULL != second)
        return -1;

    else if (NULL != first && NULL == second)
        return 1;

    if (ignore_case)
        return BeStringUtilities::Wcsicmp (first, second);

    return wcscmp(first, second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// Compare the moniker memberwise. NOTE: The search status is not compared, as a
// 'resolved' moniker with fields identical to a 'unresolved' moniker should be
// essentially same. Custom XML Strings are 
int DgnDocumentMoniker::CompareBinary (DgnDocumentMoniker const& rhs) const
    {
    if (GetIsRelative () != rhs.GetIsRelative ())
        return int(GetIsRelative ()) - int(rhs.GetIsRelative ());

    for (StringId id = STRINGID_PortableName; id < STRINGID_Count__; id = static_cast <StringId>(id + 1))
        {
        int cmpResult    = 0;
        cmpResult = safe_wcscmp (GetString (id), rhs.GetString (id), false);

        if (0 != cmpResult)
            return cmpResult;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_ResolveLocationDisplayName (StatusInt& status) const
    {
    WString fileName = ResolveFileName (&status);

    if (SUCCESS != status)
        return L"";

    WString dir;
    BeFileName::ParseName (NULL, &dir, NULL, NULL, fileName.c_str());

    return dir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_ResolveLocation (StatusInt& status) const
    {
    return ResolveLocationDisplayName (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentMoniker::_ResolveDisplayName () const
    {
    StatusInt   status;
    WString     fileName = ResolveFileName (&status);

    return (SUCCESS == status) ? fileName : _GetPortableName ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
int     DgnDocumentMoniker::_Compare (DgnBaseMoniker const& anotherMoniker) const
    {
    DgnDocumentMonikerCP documentMoniker = dynamic_cast<DgnDocumentMonikerCP> (&anotherMoniker);
    if (NULL == documentMoniker)
        return -1; // Return result just to denote result of comparison;

    return Compare (*documentMoniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnDocumentMoniker::_CompareBinary (DgnBaseMoniker const &anotherMoniker) const
    {
    DgnDocumentMonikerCP documentMoniker = dynamic_cast<DgnDocumentMonikerCP> (&anotherMoniker);
    if (NULL == documentMoniker)
        return -1; // Return result just to denote result of comparison;

    return CompareBinary (*documentMoniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr DgnDocumentMoniker::_TransformTo (DgnDocumentMonikerCR base)
    {
    if (!GetIsRelative ())
        return this;

    WString sourceRelative = GetSavedFileName ();
    WString resolvedPath;

    if (SUCCESS == BeFileName::ResolveRelativePath (resolvedPath, sourceRelative.c_str (), GetString (STRINGID_SearchPath)))
        {
        WString destRelative;
        BeFileName::FindRelativePath (destRelative, resolvedPath.c_str(), base.GetString (STRINGID_SearchPath));
        return DgnDocumentMoniker::CreateFromFileName (destRelative.c_str(), base.GetString (STRINGID_SearchPath));
        }

    return this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnDocumentMoniker::GetDmsMonikerString(WStringR value) const
    {
    value.clear();
    
    if (WString::IsNullOrEmpty(m_strings[STRINGID_Custom]))
        return ERROR;
    
    static WCharCP XML_TAG_DmsMonikerOpen = L"<DmsMoniker>";
    static WCharCP XML_TAG_DmsMonikerClose = L"</DmsMoniker>";

    WCharCP openPos = wcsstr(m_strings[STRINGID_Custom], XML_TAG_DmsMonikerOpen);
    if (NULL == openPos)
        return ERROR;

    WCharCP closePos = wcsstr(m_strings[STRINGID_Custom], XML_TAG_DmsMonikerClose);
    if (NULL == closePos)
        return ERROR;

    WCharCP valueStart = openPos + wcslen(XML_TAG_DmsMonikerOpen);
    if (valueStart >= closePos)
        return SUCCESS;
    
    value.assign(valueStart, (size_t)(closePos - valueStart));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocument::DgnDocument (DgnDocumentMonikerR moniker) : m_moniker (&moniker) 
    {
    m_readOnly = false;
    m_scratchFile = false;
    m_nameForFileList = NULL;
    m_dgnProject = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocument::~DgnDocument ()
    {
    ClearNameForRecentFileList ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nancy.McCall    07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnDocument::_IsInSameRepository (DgnDocumentCP compareDocument) const
    {
    if ((compareDocument->GetDocState() & DgnDocument::STATE_InDMS) != (GetDocState() & DgnDocument::STATE_InDMS))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocument::State   DgnDocument::_GetDocState () const
    {
    StatusInt   status = util_findFile (NULL, NULL, _GetFileName().c_str(), NULL, NULL, UF_CUR_DIR_SWAP);

    return (SUCCESS == status) ? STATE_InFileSystem : STATE_DoesNotExist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsifunc                                                      JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool        DgnDocument::_CanGetFileHandle (bool readOnly) const
    {
    BeFileAccess    accessMode = readOnly?  BeFileAccess::Read: BeFileAccess::ReadWrite;
//WIP_BeFile    BeFileSharing   shareMode  = readOnly?  BeFileSharing::Read : BeFileSharing::None;

    BeFile handle;
    return BeFileStatus::Success == handle.Open (_GetFileName().c_str(), accessMode/*WIP_BeFile, shareMode*/);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocument::Access  DgnDocument::_GetAvailableAccess() const
    {
    if (_CanGetFileHandle (false))
        return ACCESS_ExclusiveWrite;

    if (_CanGetFileHandle (true))
        return ACCESS_ReadOnly;

    return ACCESS_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocument::Permissions  DgnDocument::_GetPermissions() const
    {
    Permissions permissions = PERMISSIONS_None;
    WString             fileName = _GetFileName();

    if (BeFileNameStatus::Success == BeFileName::CheckAccess (fileName.c_str(), BeFileNameAccess::Read))
        permissions = (Permissions) (permissions | PERMISSIONS_Write);

    if (BeFileNameStatus::Success == BeFileName::CheckAccess (fileName.c_str(), BeFileNameAccess::Write))
        permissions = (Permissions) (permissions | PERMISSIONS_Read);

    return permissions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnDocument::_GetCommitTime (time_t& commitTime) const
    {
    return (StatusInt) BeFileName::GetFileTime (NULL, NULL, &commitTime, _GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnDocument::_SetLocalFileModifyTime (time_t newTime) const
    {
    return (StatusInt) BeFileName::SetFileTime (_GetFileName().c_str(), NULL, &newTime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDocumentManager::_DeleteDocument (DgnDocumentR doc, DeleteOptions opts)
    {
    return (StatusInt) BeFileName::BeDeleteFile (doc._GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnDocument::ClearNameForRecentFileList ()
    {
    if (m_nameForFileList != NULL)
        {
        free (m_nameForFileList);
        m_nameForFileList = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnDocument::SetNameForRecentFileList (WCharCP name)
    {
    ClearNameForRecentFileList ();
    if (name != NULL)
        m_nameForFileList = BeStringUtilities::Wcsdup (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnDocument::GetNameForRecentFileList () const {return m_nameForFileList? m_nameForFileList: L"";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocument::_GetFileName () const
    {
    if (m_moniker == NULL)
        return L"";
    WString sn = GetMoniker().GetSavedFileName();
    if (!sn.empty())
        return sn;
    return GetMoniker().GetPortableName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDocument::_IsSameFile (DgnDocument const& rhs) const
    {
    return GetFileName().EqualsI (rhs.GetFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectP DgnDocument::GetDgnProject() const {return m_dgnProject;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr DgnDocumentManager::_CreateMonikerFromRawData (WCharCP portableName, WCharCP fullPath, WCharCP providerId, WCharCP basePath, bool fullPathFirst, WCharCP customString)
    {
    bool        haveBasePath        = (NULL != basePath && '\0' != basePath[0]);
    bool        havelogicalDevice   = false;
    WString     portableNameComputed;

    if (NULL != portableName && '\0' != portableName[0])
        {
        WString device;

        BeFileName::ParseName (&device, NULL, NULL, NULL, portableName);

        if (device.size() > 1)
            havelogicalDevice = true;
        }
    else if (NULL != fullPath && '\0' != fullPath[0])
        {
        portableNameComputed = BuildPortableFileName (fullPath, fullPath);
        portableName = portableNameComputed.c_str();
        }

    bool        isRelative    = haveBasePath && ! havelogicalDevice;

    return _CreateMoniker (portableName, fullPath, providerId, isRelative, basePath, fullPathFirst, customString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr DgnDocumentManager::_CreateMoniker (WCharCP portableName, WCharCP fullPath, WCharCP providerID, bool isRelative, WCharCP basePath, bool fullPathFirst, WCharCP customString)
    {
    return new DgnDocumentMoniker (portableName, fullPath, providerID, isRelative, basePath, fullPathFirst, customString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr DgnDocumentManager::_CreateMonikerFromFileName (WCharCP fileName, WCharCP basePath)
    {
    DgnBaseMoniker::SearchStatus searchStatus = DgnBaseMoniker::SEARCH_NotAttempted;
    WString         foundFile = DgnBaseMoniker::SearchForFile (searchStatus, fileName, NULL, basePath, false, false);

    if (DgnBaseMoniker::SEARCH_FoundRelative != searchStatus
     && DgnBaseMoniker::SEARCH_FoundEnvironment != searchStatus)
        basePath = NULL;

    return _CreateMonikerFromRawData (fileName, NULL, NULL, basePath, false, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/05
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentManager::BuildPortableFolderName (WCharCP inputName, WCharCP foundName)
    {
    WString inputFolderName (inputName);
    WString foundFolderName (foundName);

    PathUtil::StripSeparatorAtEnd (inputFolderName);
    PathUtil::StripSeparatorAtEnd (foundFolderName);

    return BuildPortableFileName (inputFolderName.c_str (), foundFolderName.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerPtr DgnDocumentManager::_CreateFolderMonikerFromRawData
(
WCharCP   portableName,
WCharCP   fullPath,
WCharCP   providerId,
WCharCP   basePath,
bool        fullPathFirst,
WCharCP   customXMLStrings
)
    {
    bool        haveBasePath        = (NULL != basePath && '\0' != basePath[0]);
    bool        havelogicalDevice   = false;
    WString     portableNameComputed;

    if (NULL != portableName && '\0' != portableName[0])
        {
        WString device;

        BeFileName::ParseName (&device, NULL, NULL, NULL, portableName);

        if (device.size() > 1)
            havelogicalDevice = true;
        }
    else
        {
        portableNameComputed = BuildPortableFolderName (fullPath, fullPath);
        portableName = portableNameComputed.c_str();
        }

    bool        isRelative    = haveBasePath && ! havelogicalDevice;

    return _CreateFolderMoniker (portableName, fullPath, providerId, isRelative, basePath, fullPathFirst, customXMLStrings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerPtr DgnDocumentManager::_CreateFolderMoniker 
(
WCharCP portableName, 
WCharCP fullPath, 
WCharCP providerID, 
bool      isRelative,
WCharCP searchPath, 
bool      fullPathFirst, 
WCharCP customXMLString
)
    {
    return new DgnFolderMoniker (portableName, fullPath, providerID, isRelative, searchPath, fullPathFirst, customXMLString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerPtr DgnDocumentManager::_CreateMonikerFromFolderName (WCharCP folderName, WCharCP basePath)
    {
    DgnBaseMoniker::SearchStatus searchStatus = DgnBaseMoniker::SEARCH_NotAttempted;
    WString foundFile = DgnBaseMoniker::SearchForFile (searchStatus, folderName, NULL, basePath, true);

    if (DgnBaseMoniker::SEARCH_FoundRelative    != searchStatus && 
        DgnBaseMoniker::SEARCH_FoundEnvironment != searchStatus)
        basePath = NULL;

    return _CreateFolderMonikerFromRawData (folderName, NULL, NULL, basePath, false, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr  DgnDocumentManager::_CreateDocument0 (DgnDocumentMonikerR moniker)
    {
    return new DgnDocument (moniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr  DgnDocumentManager::_CreateDocumentFromMoniker (StatusInt& status, DgnDocumentMonikerR moniker, int, DgnDocument::FetchMode openMode, DgnDocument::FetchOptions)
    {
    WString     fileName = moniker.ResolveFileName (&status);

    if (SUCCESS != status)
        return NULL;

    DgnDocumentPtr doc = _CreateDocument0 (moniker);
    if (doc == NULL)
        return NULL;

    doc->SetReadOnly (DgnDocument::FETCH_Read == openMode);
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentManager&  DgnDocumentManager::GetManager ()
    {
#if defined (WIP_V10_DOCUMENT_MANAGER) // *** Re-think document manager in V10
    return T_HOST.GetDocumentManager();
#else
    static DgnDocumentManager* s_wip_wip_wip_wip_singleton;
    if (NULL == s_wip_wip_wip_wip_singleton)
        s_wip_wip_wip_wip_singleton = new DgnDocumentManager();
    return *s_wip_wip_wip_wip_singleton;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB                             9/90
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnDocumentManager::BuildPortableFileName (WCharCP inputNameP, WCharCP completeNameP)
    {
    WCharCP pKeepName = inputNameP;
    WString     filename;
    WString     fileExtension;
    WString     openExtension;

    if (false /*checkDisallowRelativeRefPath()*/)
        {
        /* strip any DOS or UNIX stuff off front */
        if (NULL != (pKeepName=::wcsrchr(inputNameP, DIR_SEPARATOR_CHAR)))
            {
            WString device;

            /* changed 5/99 for ProjectBank....dont strip if preceded by logical device */
            BeFileName::ParseName (&device, NULL, NULL, NULL, inputNameP);
            if (device.empty() 
#if defined (WIP_CFGVAR) // Support for cfgvar as part of portable filename (e.g., RFDIR)
            || !ConfigurationManager::IsVariableDefined (device)
#endif
                )
                pKeepName = pKeepName+1;
            else
                pKeepName = inputNameP;
            }
        else
            {
            pKeepName = inputNameP;
            }
        }
    else
        {
        // if relative paths allowed, strip out the directory info only if it matches the complete name.
        if ( (0 == BeStringUtilities::Wcsicmp (inputNameP, completeNameP)) && (NULL != (pKeepName=::wcsrchr(inputNameP, DIR_SEPARATOR_CHAR))) )
            {
            pKeepName = pKeepName + 1;
            }
        else
            {
            pKeepName = inputNameP;
            }
        }

    /* see if there is an extension in what the user typed in */
    BeFileName::ParseName (NULL, NULL, &filename, &fileExtension, pKeepName);

    /* find the name it was opened with */
    BeFileName::ParseName (NULL, NULL, NULL, &openExtension, completeNameP);

    WString attachName (pKeepName);

    if ( !attachName.empty() && fileExtension.empty() && !openExtension.empty() )
        {
        if (attachName[attachName.size()-1] != '.')
            attachName.append (L".");

        attachName.append (openExtension);
        }

    return attachName;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr  DgnDocumentManager::_CreateDocumentForNewFile (DgnFileStatus& status, WCharCP fileName, WCharCP searchPath, int,
                                                     WCharCP defaultName, DgnDocument::CreateOptions options)
    {
    status = DGNFILE_STATUS_Success;
    FileNameBuf newFile;

    StatusInt   found = util_findFile (NULL, newFile.GetNamePtr(), fileName, searchPath, defaultName, UF_CUR_DIR_SWAP);
    if (SUCCESS == found)
        {
        switch (options)
            {
            default:
                BeAssert (false);
                return NULL;

            case DgnDocument::CREATE_OPTION_ApplicationReserved:
            case DgnDocument::CREATE_OPTION_FailIfExists:
                status = DGNOPEN_STATUS_FileAlreadyExists;
                return  NULL;

            case DgnDocument::CREATE_OPTION_PromptOverwrite:
            case DgnDocument::CREATE_OPTION_Overwrite:
                if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (newFile.GetNamePtr()))
                    {
                    status = DGNOPEN_STATUS_AccessViolation;
                    return  NULL;
                    }
                break;
            }
        }

    WString portableName = BuildPortableFileName (fileName, newFile);

    DgnDocumentMonikerPtr moniker = _CreateMonikerFromRawData (portableName.c_str(), newFile, NULL, searchPath, false, NULL);

    return _CreateDocument0 (*moniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr DgnDocumentManager::_CreateDocumentForEmbeddedFile (WCharCP pseudoFilePath)
    {
    DgnDocumentMonikerPtr moniker = _CreateMonikerFromRawData (pseudoFilePath, NULL, NULL, NULL, false, NULL);
    return _CreateDocument0 (*moniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr DgnDocumentManager::_CreateDocumentForLocalFile (WCharCP fileName)
    {
    DgnDocumentMonikerPtr moniker = _CreateMonikerFromRawData (NULL, fileName, NULL, NULL, false, NULL);
    return _CreateDocument0 (*moniker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr DgnDocument::CreateFromFileName (DgnFileStatus& openStatus, WCharCP fileName, WCharCP searchPath, int defFileId, DgnDocument::FetchMode mode, DgnDocument::FetchOptions options)
    {
    DgnDocumentMonikerPtr moniker = DgnDocumentMoniker::CreateFromFileName (fileName, searchPath);
    if (moniker == NULL)
        {
        openStatus = DGNFILE_STATUS_UnknownError;
        return NULL;
        }

    StatusInt status;
    DgnDocumentPtr openedDoc = CreateFromMoniker (status, *moniker, defFileId, mode, options);
    if (openedDoc == NULL)
        {
        openStatus = (DgnFileStatus) status;
        return NULL;
        }

    return  openedDoc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentPtr DgnDocumentManager::_OpenDocumentDialog (DgnBrowserStatus& status, DgnDocumentBrowserDefaults const & defaults)
    {
    status = DOC_BROWSER_NoIntegrationLoaded;
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerPtr DgnDocumentManager::_OpenFolderBrowser (DgnBrowserStatus& status, DgnFolderBrowserDefaults const & defaults)
    {
    status = DOC_BROWSER_NoIntegrationLoaded;
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMonikerListPtr     DgnDocumentManager::_CreateBaseMonikerList ()     { return new DgnBaseMonikerList (); } 
DgnDocumentMonikerListPtr DgnDocumentManager::_CreateDocumentMonikerList () { return new DgnDocumentMonikerList (); }
DgnFolderMonikerListPtr   DgnDocumentManager::_CreateFolderMonikerList ()   { return new DgnFolderMonikerList (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMonikerListPtr DgnDocumentManager::_GetFolderContents (DgnFolderMonikerPtr folder, WCharCP pattern)
    {
    DgnBaseMonikerListPtr monikerList = DgnBaseMonikerList::Create ();
    if (!folder.IsValid ())
        return monikerList;

    StatusInt status = SUCCESS;
    WString folderPath = folder->ResolveFolderName (&status);

    if (SUCCESS != status)
        return monikerList;

    if (NULL == pattern)
        folderPath.append (L"*"); // We need all files
    else
        folderPath.append (pattern);

    BeFileListIterator fsIterator (folderPath.c_str (), false);

    BeFileName fileName;
    while (SUCCESS == fsIterator.GetNextFileName (fileName))
        {
        // Not sure if this moniker should inherit the search path.
        DgnBaseMonikerPtr moniker;
        if (BeFileName::IsDirectory (fileName.GetName ()))
            {
            DgnFolderMonikerPtr folderMoniker = DgnFolderMoniker::CreateFromFolderName (fileName.GetName (), NULL);
            moniker = folderMoniker.get ();
            }
        else
            {
            DgnDocumentMonikerPtr documentMoniker = DgnDocumentMoniker::CreateFromFileName (fileName.GetName (), NULL);
            moniker = documentMoniker.get ();
            }

        monikerList->Add (moniker);
        }

    return monikerList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerListPtr    DgnDocumentManager::_GetSubFolders (DgnFolderMonikerPtr folder, WCharCP pattern)
    {
    DgnFolderMonikerListPtr monikerList = DgnFolderMonikerList::Create ();
    if (!folder.IsValid ())
        return monikerList;

    StatusInt status = SUCCESS;
    WString folderPath = folder->ResolveFolderName (&status);

    if (SUCCESS != status)
        return monikerList;

    if (NULL == pattern)
        folderPath.append (L"*"); // We need all files
    else
        folderPath.append (pattern);

    BeFileListIterator fsIterator (folderPath.c_str (), false);

    BeFileName fileName;
    while (SUCCESS == fsIterator.GetNextFileName (fileName))
        {
        // Not sure if this moniker should inherit the search path.
        DgnBaseMonikerPtr moniker;
        if (BeFileName::IsDirectory (fileName.GetName ()))
            {
            DgnFolderMonikerPtr folderMoniker = DgnFolderMoniker::CreateFromFolderName (fileName.GetName (), NULL);
            monikerList->Add (folderMoniker);
            }
        }

    return monikerList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerListPtr  DgnDocumentManager::_GetDocuments (DgnFolderMonikerPtr folder, WCharCP pattern)
    {
    DgnDocumentMonikerListPtr monikerList = DgnDocumentMonikerList::Create ();
    if (!folder.IsValid ())
        return monikerList;

    StatusInt status = SUCCESS;
    WString folderPath = folder->ResolveFolderName (&status);

    if (SUCCESS != status)
        return monikerList;

    if (NULL == pattern)
        folderPath.append (L"*"); // We need all files
    else
        folderPath.append (pattern);

    BeFileListIterator fsIterator (folderPath.c_str (), false);

    BeFileName fileName;
    while (SUCCESS == fsIterator.GetNextFileName (fileName))
        {
        // Not sure if this moniker should inherit the search path.
        DgnBaseMonikerPtr moniker;
        if (!BeFileName::IsDirectory (fileName.GetName ()))
            {
            DgnDocumentMonikerPtr documentMoniker = DgnDocumentMoniker::CreateFromFileName (fileName.GetName (), NULL);
            monikerList->Add (documentMoniker);
            }
        }

    return monikerList;
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// DgnFolderMonikers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 10/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP       DgnFolderMoniker::GetString (StringId id) const
    {
    wchar_t const* p = m_strings[id];
    return p? p: L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnFolderMoniker::SetString (StringId id, WCharCP newStr, bool storeEmptyString) const
    {
    WCharP& str = m_strings[id];

    if (str == newStr)
        return;

    if (str != NULL)
        {
        free (str);
        str = NULL;
        }

    if (newStr != NULL && (storeEmptyString || *newStr != '\0'))
        str = BeStringUtilities::Wcsdup (newStr);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnFolderMoniker::_Externalize () const 
    {
    WString externalizedString;

    DgnFolderMonikerXMLSupport folderXml;
    folderXml.ToXML (externalizedString, GetString(STRINGID_PortableName), GetString(STRINGID_ProviderId), GetString(STRINGID_FileName), GetIsRelative(), GetString (STRINGID_Custom));
    return externalizedString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 10/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMonikerPtr DgnFolderMoniker::Create (WCharCP externalizedState, WCharCP basePath, bool fullPathFirst)
    {
    WString fileName, fullPath, providerId, customXML;
    bool     isRelative;

    DgnFolderMonikerXMLSupport folderXml;
    folderXml.FromXML (fileName, providerId, fullPath, isRelative, customXML, externalizedState);

    return DgnDocumentManager::GetManager()._CreateFolderMoniker (fileName.c_str(), fullPath.c_str(), providerId.c_str(), isRelative, basePath, fullPathFirst, customXML.c_str ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    03/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMoniker::DgnFolderMoniker (WCharCP portableName, WCharCP fullPath, WCharCP providerId, bool isRelative, WCharCP searchPath, bool fullPathFirst, WCharCP customXMLStrings)
    :
    m_flags (0)
    {
    memset (m_strings, 0, sizeof(m_strings));
    if (NULL == fullPath)   // See Note "Cache filename"
        fullPath = portableName;
    SetString (STRINGID_FileName, fullPath);
    SetString (STRINGID_PortableName, portableName, true);
    SetString (STRINGID_ProviderId, providerId);
    SetString (STRINGID_SearchPath, searchPath);
    SetIsRelative (isRelative);
    SetFullPathFirst (fullPathFirst);
    SetSearchStatus (SEARCH_NotAttempted);
    SetString (STRINGID_Custom, customXMLStrings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMoniker& DgnFolderMoniker::operator=(DgnFolderMoniker const& rhs)
    {
    m_flags = rhs.m_flags;
    for (int i=0; i<STRINGID_Count__; ++i)
        SetString ((StringId)i, rhs.m_strings[i]);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMoniker::DgnFolderMoniker (DgnFolderMoniker const& rhs)
    {
    m_flags = 0;
    memset (m_strings, 0, sizeof(m_strings));
    *this = rhs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFolderMoniker::~DgnFolderMoniker ()
    {
    for (int i=0; i<STRINGID_Count__; ++i)
        SetString ((StringId)i, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    DgnFolderMoniker::_UpdateSavedFolderName (WCharCP fullPath)     
    {
    WString fullPathW (fullPath);
    PathUtil::EnsureSeparatorAtEnd (fullPathW);
    SetString (STRINGID_FileName, fullPathW.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_GetSavedFolderName () const                    {return GetString (STRINGID_FileName);}
WString DgnFolderMoniker::_GetProviderId () const                         {return GetString (STRINGID_ProviderId);}
void    DgnFolderMoniker::SetSearchPath (WCharCP searchPath)            {SetString (STRINGID_SearchPath, searchPath);}
void    DgnFolderMoniker::_SetParentSearchPath (WCharCP newPath)        {SetSearchPath (newPath);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_GetPortableName () const                       
    {
    WString portableName = GetString (STRINGID_PortableName);
    PathUtil::EnsureSeparatorAtEnd (portableName);
    return portableName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_GetShortDisplayName () const
    {
    WString portableName = _GetPortableName ();
    if (portableName.empty ())
        return WString ();

    // Ensure that we remove the separation character at the end of
    // the folder. This way, the mdlFile_* functions will treat the
    // folder name as the file name. May be we can wrap directory
    // functions. TODO:
    PathUtil::StripSeparatorAtEnd (portableName);

    WString         folderName;
    WString         folderNameExt;
    WString         folderDevice;

    BeFileName::ParseName (&folderDevice, NULL, &folderName, &folderNameExt, portableName.c_str ());

    BeFileName         folderDisplayName;
    if (folderName.empty() && folderNameExt.empty())
        {
        // Build the folder name from its logical device
        if (!folderDevice.empty())
            folderDisplayName.BuildName (folderDevice.c_str(), NULL, NULL, NULL);
        else
            return WString ();
        }
    else
        {
        folderDisplayName.BuildName (NULL, NULL, folderName.c_str(), folderNameExt.c_str());
        }

    WString displayName = folderDisplayName.GetName ();
    PathUtil::EnsureSeparatorAtEnd (displayName);

    return displayName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_ResolveFolderName (StatusInt* statusOut, bool dontRetryIfFailed) const
    {
    if (SEARCH_Failed < GetSearchStatus ())
        {
        if (NULL != statusOut)
            *statusOut = SUCCESS;
        return GetString (STRINGID_FileName);
        }

    StatusInt _status;
    StatusInt& status = (NULL == statusOut)? _status: *statusOut;

    status = DGNOPEN_STATUS_FileNotFound;     

    if (SEARCH_Failed == GetSearchStatus() && !dontRetryIfFailed)
        SetSearchStatus (SEARCH_NotAttempted);

    // If we've never searched before, do the search now and cache the result.
    if (SEARCH_NotAttempted == GetSearchStatus())
        {
        SearchStatus searchStatus;

        WString foundFolderPath = findFolder (searchStatus, 
                                              GetString (STRINGID_PortableName), 
                                              GetString (STRINGID_FileName), 
                                              GetString (STRINGID_SearchPath), 
                                              GetFullPathFirst());

        SetSearchStatus (searchStatus);

        if (SEARCH_Failed < searchStatus)
            {
            PathUtil::EnsureSeparatorAtEnd (foundFolderPath);
            SetString (STRINGID_FileName, foundFolderPath.c_str());
            }

        }

    // If this moniker has ever been searched successfully, the result is cached in the moniker string.
    if (SEARCH_Failed >= GetSearchStatus())
        return L"";

    status = SUCCESS;
    return GetString (STRINGID_FileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_ResolveDisplayName () const
    {
    StatusInt   status;
    WString     folderName = ResolveFolderName (&status);

    return (SUCCESS == status) ? folderName : _GetPortableName ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnFolderMoniker::_ResolveLocationDisplayName (StatusInt& status) const
    {
    WString folderName = ResolveFolderName(&status);

    if (SUCCESS != status)
        return L"";

    PathUtil::StripSeparatorAtEnd (folderName);

    WString dir;
    BeFileName::ParseName (NULL, &dir, NULL, NULL, folderName.c_str());

    return dir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nancy.McCall                    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DgnFolderMoniker::_ResolveLocation (StatusInt& status) const
    {
    return ResolveLocationDisplayName (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnFolderMoniker::_Compare (DgnBaseMoniker const& anotherMoniker) const
    {
    DgnFolderMonikerCP folderMoniker = dynamic_cast<DgnFolderMonikerCP> (&anotherMoniker);

    if (NULL == folderMoniker)
        return -1; // Return result just to denote result of comparison;

    StatusInt   status1, status2;

    WString string1 =   ResolveFolderName (&status1);
    WString string2 =   folderMoniker->ResolveFolderName (&status2);

    if (SUCCESS != status1 || SUCCESS != status2)
        {
        // Current implementation insists that a failed moniker cannot be equal to
        // another failed moniker.  Perhaps there are cases where this is incorrect.
        // For now, I don't care about those cases, so I'm taking the easy way out.

        return (SUCCESS == status1) ? -1 : 1;
        }

    return BeStringUtilities::LexicographicCompare (string1.c_str (), string2.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnFolderMoniker::_CompareBinary (DgnBaseMoniker const &rhs) const
    {
    return -1; // TODO:
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString DgnFolderMoniker::_GetParentSearchPath () const {return GetString (STRINGID_SearchPath);}

/////// End DgnFolderMonikers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMonikerListPtr  DgnDocumentMonikerList::ToDgnBaseMonikerList () const
    {
    DgnBaseMonikerListPtr  baseMonikerList = DgnBaseMonikerList::Create ();

    for (DgnDocumentMonikerList::const_iterator curr=begin(); curr<end(); curr++)
        baseMonikerList->Add (curr->get());

    return baseMonikerList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMonikerListPtr  DgnFolderMonikerList::ToDgnBaseMonikerList () const
    {
    DgnBaseMonikerListPtr  baseMonikerList = DgnBaseMonikerList::Create ();

    for (DgnFolderMonikerList::const_iterator curr=begin(); curr<end(); curr++)
        baseMonikerList->Add (curr->get());

    return baseMonikerList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnBaseMonikerList::Remove (DgnBaseMonikerPtr moniker) 
    { 
    if (!moniker.IsValid ())
        return false;
    
    DgnBaseMonikerList::iterator found = std::find (m_baseMonikers.begin (), m_baseMonikers.end (), moniker);
    if (found == m_baseMonikers.end ())
        return false;
        
    m_baseMonikers.erase (found);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDocumentMonikerList::Remove (DgnDocumentMonikerPtr moniker) 
    { 
    if (!moniker.IsValid ())
        return false;
    
    DgnDocumentMonikerList::iterator found = std::find (m_documentMonikers.begin (), m_documentMonikers.end (), moniker);
    if (found == m_documentMonikers.end ())
        return false;
        
    m_documentMonikers.erase (found);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnFolderMonikerList::Remove (DgnFolderMonikerPtr moniker) 
    { 
    if (!moniker.IsValid ())
        return false;
    
    DgnFolderMonikerList::iterator found = std::find (m_folderMonikers.begin (), m_folderMonikers.end (), moniker);
    if (found == m_folderMonikers.end ())
        return false;
        
    m_folderMonikers.erase (found);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseMonikerListPtr DgnBaseMonikerList::Create () { return DgnDocumentManager::GetManager()._CreateBaseMonikerList ();}
DgnDocumentMonikerListPtr DgnDocumentMonikerList::Create () { return DgnDocumentManager::GetManager()._CreateDocumentMonikerList ();}
DgnFolderMonikerListPtr DgnFolderMonikerList::Create () { return DgnDocumentManager::GetManager()._CreateFolderMonikerList ();}

// These method will be removed, we should return the ptr and not the pointer directly.
DgnBaseMonikerListP DgnBaseMonikerList::CreateList () { return new DgnBaseMonikerList ();}
DgnDocumentMonikerListP DgnDocumentMonikerList::CreateList () { return new DgnDocumentMonikerList ();}
DgnFolderMonikerListP DgnFolderMonikerList::CreateList () { return new DgnFolderMonikerList ();}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Move these to a ...dllinlines.h file?!
DgnDocumentMonikerCR        DgnDocument::GetMoniker() const                                 {return *m_moniker;}
DgnDocumentMonikerPtr       DgnDocument::GetMonikerPtr() const                              {return m_moniker;}
StatusInt                   DgnDocument::_Fetch (FetchMode a, FetchOptions b)               {return SUCCESS;}
StatusInt                   DgnDocument::Fetch (FetchMode a, FetchOptions b)                {return _Fetch(a,b);}
StatusInt                   DgnDocument::_Put (PutAction a, PutOptions b, WCharCP)        {return SUCCESS;}
StatusInt                   DgnDocument::Put (PutAction a, PutOptions b, WCharCP c)       {return _Put(a,b,c);}
bool                        DgnDocument::_HasLocalChanges () const                          {return false;}
bool                        DgnDocument::HasLocalChanges () const                           {return _HasLocalChanges();}
bool                        DgnDocument::_HasServerChanges () const                         {return false;}
bool                        DgnDocument::HasServerChanges () const                          {return _HasServerChanges();}
bool                        DgnDocument::IsSameFile (DgnDocumentCR a) const                 {return _IsSameFile(a);}
WString                     DgnDocument::GetFileName () const                               {return _GetFileName(); }
StatusInt                   DgnDocument::GetCommitTime (time_t& commitTime) const           {return _GetCommitTime(commitTime);}
StatusInt                   DgnDocument::SetLocalFileModifyTime (time_t newTime) const      {return _SetLocalFileModifyTime(newTime);}
bool                        DgnDocument::IsInSameRepository (DgnDocumentCP a) const         { return _IsInSameRepository (a); }
bool                        DgnDocument::_IsReadOnly () const                               {return m_readOnly;}
bool                        DgnDocument::IsReadOnly () const                                {return _IsReadOnly();}
void                        DgnDocument::_SetReadOnly (bool b)                              {m_readOnly=b;}
void                        DgnDocument::SetReadOnly (bool b)                               {_SetReadOnly(b);}
bool                        DgnDocument::_IsScratchFile () const                            {return m_scratchFile;}
bool                        DgnDocument::IsScratchFile () const                             {return _IsScratchFile();}
void                        DgnDocument::_SetScratchFile (bool b)                           {m_scratchFile=b;}
void                        DgnDocument::SetScratchFile (bool b)                            {_SetScratchFile(b);}
StatusInt                   DgnDocument::_OnNewFileCreated ()                               {return SUCCESS;}
StatusInt                   DgnDocument::OnNewFileCreated ()                                {return _OnNewFileCreated();}
DgnDocument::State          DgnDocument::GetDocState () const                               {return _GetDocState();}
DgnDocument::Access         DgnDocument::_GetGlobalAccess()    const                        {return _GetAvailableAccess();}
DgnDocument::Access         DgnDocument::_GetMyAccess()        const                        {return _GetAvailableAccess();}
DgnDocument::Access         DgnDocument::_GetPreferredAccess() const                        {return ACCESS_ExclusiveWrite;}
DgnDocument::Access         DgnDocument::GetAvailableAccess()  const                        {return _GetAvailableAccess();}
DgnDocument::Permissions    DgnDocument::GetPermissions()      const                        {return _GetPermissions();}
DgnDocument::Access         DgnDocument::GetGlobalAccess()     const                        {return _GetGlobalAccess();}
DgnDocument::Access         DgnDocument::GetMyAccess()         const                        {return _GetMyAccess();}
DgnDocument::Access         DgnDocument::GetPreferredAccess () const                        {return _GetPreferredAccess();}
int                         DgnBaseMoniker::Compare (DgnBaseMoniker const& a) const         {return _Compare (a); }
int                         DgnBaseMoniker::CompareBinary (DgnBaseMoniker const& a) const         {return _CompareBinary (a); }
WString                     DgnBaseMoniker::GetPortableName ()    const                     {return _GetPortableName(); }
WString                     DgnBaseMoniker::ResolveDisplayName () const                     {return _ResolveDisplayName(); }
WString                     DgnBaseMoniker::GetShortDisplayName ()const                     {return _GetShortDisplayName(); }
WString                     DgnBaseMoniker::ResolveLocationDisplayName (StatusInt& a) const {return _ResolveLocationDisplayName(a); }
WString                     DgnBaseMoniker::ResolveLocation (StatusInt& a) const            {return _ResolveLocation(a); }
WString                     DgnBaseMoniker::Externalize () const                            {return _Externalize(); }
WString                     DgnDocumentMoniker::GetSavedFileName () const                   {return _GetSavedFilePath();}
WString                     DgnDocumentMoniker::GetProviderId () const                      {return _GetProviderId();}
WString                     DgnDocumentMoniker::ResolveFileName (StatusInt* a,bool b) const {return _ResolveFileName(a,b); }
void                        DgnDocumentMoniker::SetParentSearchPath (WCharCP a)           {_SetParentSearchPath(a);}
WString                     DgnDocumentMoniker::GetParentSearchPath () const                {return _GetParentSearchPath();}
void                        DgnDocumentMoniker::UpdateSavedFileName (WCharCP a)           {_UpdateSavedFileName(a);}
DgnDocumentMonikerPtr       DgnDocumentMoniker::TransformTo (DgnDocumentMonikerCR a)          {return _TransformTo (a);}
StatusInt                   DgnDocumentManager::DeleteDocument (DgnDocumentR d, DeleteOptions o)
                                                                                            {return _DeleteDocument(d,o);}
DgnDocumentPtr              DgnDocument::CreateFromMoniker (StatusInt& a, DgnDocumentMonikerR b, int c, DgnDocument::FetchMode d, DgnDocument::FetchOptions e)
                                                                                            {return DgnDocumentManager::GetManager()._CreateDocumentFromMoniker(a,b,c,d,e);}
DgnDocumentPtr              DgnDocument::CreateForEmbeddedFile (WCharCP a)                {return DgnDocumentManager::GetManager()._CreateDocumentForEmbeddedFile(a);}
DgnDocumentPtr              DgnDocument::CreateForLocalFile (WCharCP a)                   {return DgnDocumentManager::GetManager()._CreateDocumentForLocalFile(a);}
DgnDocumentPtr              DgnDocument::CreateForNewFile (DgnFileStatus& a, WCharCP b, WCharCP c, int d, WCharCP e, CreateOptions f)
                                                                                            {return DgnDocumentManager::GetManager()._CreateDocumentForNewFile(a,b,c,d,e,f);}
DgnDocumentMonikerPtr       DgnDocumentMoniker::CreateFromRawData (WCharCP a, WCharCP b, WCharCP c, WCharCP d, bool e, WCharCP f)
                                                                                            {return DgnDocumentManager::GetManager()._CreateMonikerFromRawData(a,b,c,d,e,f);}
DgnDocumentMonikerPtr       DgnDocumentMoniker::CreateFromFileName (WCharCP a, WCharCP b)
                                                                                            {return DgnDocumentManager::GetManager()._CreateMonikerFromFileName (a, b);}

DgnFolderMonikerPtr         DgnDocumentManager::CreateMonikerFromFolderName (WCharCP a, WCharCP b)
                                                                                            {return DgnDocumentManager::GetManager()._CreateMonikerFromFolderName (a, b);}

DgnFolderMonikerPtr         DgnFolderMoniker::CreateFolderMonikerFromRawData (WCharCP a, WCharCP b, WCharCP c, WCharCP d, bool e, WCharCP f)
                                                                                            {return DgnDocumentManager::GetManager ()._CreateFolderMonikerFromRawData(a,b,c,d,e,f);}
DgnFolderMonikerPtr         DgnFolderMoniker::CreateFromFolderName (WCharCP a, WCharCP b)
                                                                                            { return DgnDocumentManager::GetManager ()._CreateMonikerFromFolderName (a,b);}
WString                     DgnFolderMoniker::ResolveFolderName (StatusInt* a, bool b) const {return _ResolveFolderName (a, b);}
WString                     DgnFolderMoniker::GetSavedFolderName () const                     {return _GetSavedFolderName();}
void                        DgnFolderMoniker::UpdateSavedFolderName (WCharCP a)           {_UpdateSavedFolderName(a);}
void                        DgnFolderMoniker::SetParentSearchPath (WCharCP a)             {_SetParentSearchPath(a);}
DgnDocumentPtr              DgnDocumentManager::OpenDocumentDialog (DgnBrowserStatus& a, DgnDocumentBrowserDefaults const & b) {return _OpenDocumentDialog(a,b);}
DgnFolderMonikerPtr         DgnDocumentManager::OpenFolderBrowser (DgnBrowserStatus& a, DgnFolderBrowserDefaults const & b) {return _OpenFolderBrowser(a,b);}
//// MonikerList classes
DgnBaseMonikerList::DgnBaseMonikerList () {}
DgnBaseMonikerList::const_iterator DgnBaseMonikerList::begin () const { return m_baseMonikers.begin (); }
DgnBaseMonikerList::iterator DgnBaseMonikerList::begin () { return m_baseMonikers.begin (); }
DgnBaseMonikerList::const_iterator DgnBaseMonikerList::end () const { return m_baseMonikers.end (); }
DgnBaseMonikerList::iterator DgnBaseMonikerList::end () { return m_baseMonikers.end (); }
DgnBaseMonikerList::size_type DgnBaseMonikerList::size () const { return m_baseMonikers.size (); }
void DgnBaseMonikerList::Add (DgnBaseMonikerPtr moniker) { if (moniker.IsValid ()) m_baseMonikers.push_back (moniker); }
void DgnBaseMonikerList::Clear () { m_baseMonikers.clear (); }

DgnDocumentMonikerList::DgnDocumentMonikerList () {}
DgnDocumentMonikerList::const_iterator DgnDocumentMonikerList::begin () const { return m_documentMonikers.begin (); }
DgnDocumentMonikerList::iterator DgnDocumentMonikerList::begin () { return m_documentMonikers.begin (); }
DgnDocumentMonikerList::const_iterator DgnDocumentMonikerList::end () const { return m_documentMonikers.end (); }
DgnDocumentMonikerList::iterator DgnDocumentMonikerList::end () { return m_documentMonikers.end (); }
DgnDocumentMonikerList::size_type DgnDocumentMonikerList::size () const { return m_documentMonikers.size (); }
void DgnDocumentMonikerList::Add (DgnDocumentMonikerPtr moniker) { if (moniker.IsValid ()) m_documentMonikers.push_back (moniker); }
void DgnDocumentMonikerList::Clear () { m_documentMonikers.clear (); }


DgnFolderMonikerList::DgnFolderMonikerList () {}
DgnFolderMonikerList::const_iterator DgnFolderMonikerList::begin () const { return m_folderMonikers.begin (); }
DgnFolderMonikerList::iterator DgnFolderMonikerList::begin () { return m_folderMonikers.begin (); }
DgnFolderMonikerList::const_iterator DgnFolderMonikerList::end () const { return m_folderMonikers.end (); }
DgnFolderMonikerList::iterator DgnFolderMonikerList::end () { return m_folderMonikers.end (); }
DgnFolderMonikerList::size_type DgnFolderMonikerList::size () const { return m_folderMonikers.size (); }
void DgnFolderMonikerList::Add (DgnFolderMonikerPtr moniker) { if (moniker.IsValid ()) m_folderMonikers.push_back (moniker); }
void DgnFolderMonikerList::Clear () { m_folderMonikers.clear (); }

DgnBaseMonikerListPtr     DgnDocumentManager::GetFolderContents (DgnFolderMonikerPtr folder, WCharCP pattern) { return DgnDocumentManager::GetManager ()._GetFolderContents (folder, pattern); }
DgnFolderMonikerListPtr   DgnDocumentManager::GetSubFolders (DgnFolderMonikerPtr folder, WCharCP pattern) { return DgnDocumentManager::GetManager ()._GetSubFolders (folder, pattern); }
DgnDocumentMonikerListPtr DgnDocumentManager::GetDocuments (DgnFolderMonikerPtr folder, WCharCP pattern) { return DgnDocumentManager::GetManager ()._GetDocuments (folder, pattern); }

/*
Notes:

Cache filename
--------------

The STRINGID_FileName string is first set up to cache the stored filename (or, if none, the portable) name. It is then updated by _ResolveFileName to cache the
actual filename. When opening a normal design file, dgnfileio will always call _ResolveFileName, so STRINGID_FileName will always be updated. When opening an
embedded file, dgnfileio will not call _ResolveFileName, as the name of an embedded file is an identifier, not a real filename. For a new design file, of source,
the specified filename refers to a file that has not yet been created.

DgnFile::GetFileName calls GetSavedFilePath. Therefore, it will always return the value cached in STRINGID_FileName. Thus, it will return the saved or resolved filename,
depending on whether _ResolveFileName was called. This is the correct behavior for existing files, new files, and embedded files. Moreoever, this avoids the overhead
of querying the XML data.

*/
