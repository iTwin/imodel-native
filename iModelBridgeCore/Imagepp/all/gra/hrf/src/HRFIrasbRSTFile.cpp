//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIrasbRSTFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    // must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HRFIrasbRSTFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFIntergraphMPFFile.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

HFC_IMPLEMENT_SINGLETON(HRFIrasbRSTCreator)

//-----------------------------------------------------------------------------
// Utility
// IsValidChar
//
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER
//-----------------------------------------------------------------------------
bool IsValidChar(const char pi_Char)
    {
    bool IsValid = true;

    switch (pi_Char)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            IsValid = false;
            break;
        }

    return IsValid;
    }

//-----------------------------------------------------------------------------
// Utility
// CleanUpString
//
// Remove SPACE/TAB/ENTER from the begin and end of the string
//-----------------------------------------------------------------------------
void CleanUpString(string* pio_pString)
    {
    HPRECONDITION(pio_pString != 0);

    if (pio_pString->size() > 0)
        {
        size_t Pos = 0;

        // Remove the SPACE/TAB at the begin of the string
        while (Pos < pio_pString->size() && !IsValidChar((*pio_pString)[Pos]))
            Pos++;

        *pio_pString = pio_pString->substr(Pos);

        // Remove the SPACE/TAB/ENTER at the end of the string
        if (pio_pString->size() > 0)
            {
            Pos = pio_pString->size() - 1;
            while(Pos >= 0 && !IsValidChar((*pio_pString)[Pos]))
                Pos--;

            *pio_pString = pio_pString->substr(0, Pos+1);
            }
        }
    }

//-----------------------------------------------------------------------------
// Utility
// ReadLine
//-----------------------------------------------------------------------------
void ReadLine(const HAutoPtr<HFCBinStream>& pi_pFile,
              string*                       po_pString)
    {
    HPRECONDITION(pi_pFile != 0);
    HPRECONDITION(po_pString != 0);

    const int BufferSize = MAX_PATH*3;
    char      Buffer[BufferSize + 1];
    // WString    CurrentLine;

    bool EndOfLine = false;
    po_pString->erase();
    memset(Buffer, 0, BufferSize + 1);
    for (unsigned short i = 0; i < BufferSize && !EndOfLine; i++)
        {
        if (pi_pFile->Read(&Buffer[i], 1) != 1)
            EndOfLine = true;
        else
            EndOfLine = Buffer[i] == '\n' || pi_pFile->EndOfFile();
        }

    *po_pString += Buffer;
    CleanUpString(po_pString);
    }

/** ---------------------------------------------------------------------------
    Constructor
    Creator
    ---------------------------------------------------------------------------
 */
HRFIrasbRSTCreator::HRFIrasbRSTCreator()
    : HRFRasterFileCreator(HRFIrasbRSTFile::CLASS_ID)
    {
    // Capabilities instance member initialization
    // m_pCapabilities = 0;
    m_pCapabilities = HRFIntergraphMPFCreator::GetInstance()->GetCapabilities();
    }

/** ---------------------------------------------------------------------------
    Return file format label
    ---------------------------------------------------------------------------
 */
WString HRFIrasbRSTCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_IrasbRST());  //Raster Save Set
    }

/** ---------------------------------------------------------------------------
    Return file format scheme
    ---------------------------------------------------------------------------
 */
WString HRFIrasbRSTCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

/** ---------------------------------------------------------------------------
    Return file format extension
    ---------------------------------------------------------------------------
 */
WString HRFIrasbRSTCreator::GetExtensions() const
    {
    return WString(L"*.rst");
    }

/** ---------------------------------------------------------------------------
    Open/Create RST raster file
    ---------------------------------------------------------------------------
 */
HFCPtr<HRFRasterFile> HRFIrasbRSTCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                 HFCAccessMode   pi_AccessMode,
                                                 uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFIrasbRSTFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

/** ---------------------------------------------------------------------------
    Open file and verify its validity
    ---------------------------------------------------------------------------
 */
bool HRFIrasbRSTCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                       uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    HAutoPtr<HFCBinStream> pFile;

    bool                  Result  = false;
    int32_t               layerNumber;
    int32_t               color;
    int32_t               visibility;
    int32_t               views;
    int32_t               lock;
    string currLine;

    (const_cast<HRFIrasbRSTCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // Read "rst"
    ReadLine(pFile, &currLine);

    if (strncmp(currLine.c_str(), "rst", 3) == 0)
        {
        bool bHasLayer = false;

        // Read all layers
        while (1)
            {
            // Read "layer ..."
            ReadLine(pFile, &currLine);

            if (strncmp(currLine.c_str(), "layer", 5) == 0)
                {
                bHasLayer = true;
                const char* currStr = &(currLine.c_str()[5]);

                // Try to read all "layer" fields
                // Sample: layer   0    0  1  11111111  0  d:\test\sample.cit
                char pathName[MAX_PATH];
                if (sscanf(currStr, "%ld %ld %d %ld %d %s", &layerNumber, &color, &visibility, &views, &lock, pathName) != 6)
                    goto WRAPUP;
                }
            else if (bHasLayer)
                break;
            else
                goto WRAPUP;
            }

        // Read "end"
        if (strncmp(currLine.c_str(), "end", 3) != 0)
            goto WRAPUP;
        }
    else
        goto WRAPUP;

    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

/** ---------------------------------------------------------------------------
    Open rst file and extract information
    ---------------------------------------------------------------------------
 */
void HRFIrasbRSTCreator::OpenFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  list<RSTSubFileInfo>& po_rListOfRSTSubFileInfo,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream> pFile;

    (const_cast<HRFIrasbRSTCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    try
        {
            pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true);
        }
    catch (...)
        {
        SisterFileLock.ReleaseKey();
        HASSERT(!(const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl->IsLocked());
        (const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl = 0;
        throw;  // propagate to caller.
        }

    // Read "rst"
    string currLine;
    ReadLine(pFile, &currLine);

    if (strncmp(currLine.c_str(), "rst", 3) == 0)
        {
        // Read all layers
        while (1)
            {
            // Read "layer ..."
            ReadLine(pFile, &currLine);

            if (strncmp(currLine.c_str(), "layer", 5) == 0)
                {
                const char* currStr = &(currLine.c_str()[5]);

                // Read all layers
                RSTSubFileInfo info;
                if (sscanf(currStr, "%ld %ld %ld %ld %ld", &info.layerNumber, &info.color, &info.visibility, &info.views, &info.lock) == 5)
                    {
                    // TR 176108 Unable to open rasters in .rst files when raster paths have spaces.
                    // Using the read settings, find the complete filename which may contain spaces.
                    HAutoPtr<int32_t> pFoundSettings;
                    pFoundSettings = new int32_t[5];

                    pFoundSettings[0] = info.layerNumber;
                    pFoundSettings[1] = info.color;
                    pFoundSettings[2] = info.visibility;
                    pFoundSettings[3] = info.views;
                    pFoundSettings[4] = info.lock;

                    // Move pointer position until the beginning of the filename is reached
                    for (int i = 0; i < 5; i++)
                        {
                        char settings[MAX_PATH];
                        sprintf (settings, "%ld", pFoundSettings[i]);

                        currStr = strstr (currStr, settings);
                        HASSERT (currStr);
                        currStr += strlen (settings);
                        }

                    // Clean up to remove any possible spaces before the filename
                    while (currStr[0] == ' ')
                        {
                        currStr++;
                        }

                    BeStringUtilities::Utf8ToWChar(info.fileName,currStr);
                    po_rListOfRSTSubFileInfo.push_back(info);
                    }
                }
            else
                break;
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIrasbRSTCreator*>(this))->m_pSharingControl = 0;
    }

/** ---------------------------------------------------------------------------
    Get list of related files from a given URL
    ---------------------------------------------------------------------------
 */
bool HRFIrasbRSTCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                         ListOfRelatedURLs&    pio_rRelatedURLs) const
    {
    HASSERT (pio_rRelatedURLs.size() == 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    list<RSTSubFileInfo> listOfRSTSubFileInfo;

    // Open RST file and get info
    OpenFile(pi_rpURL, listOfRSTSubFileInfo);

    // Scan all files
    for (list<RSTSubFileInfo>::const_iterator Itr = listOfRSTSubFileInfo.begin(); Itr != listOfRSTSubFileInfo.end(); ++Itr)
        {
        WString FileName;

        if ((Itr->fileName).find(L"\\") == WString::npos)
            {
            // If file has no path, build complete path from RSP file
            WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            WString::size_type SepPos = Path.rfind(L"\\");

            if (SepPos != WString::npos)
                Path = Path.substr(0, SepPos);

            // Compose url
            FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                       + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() + WString(L"\\")
                       + Path + Itr->fileName;
            }
        else
            FileName = WString(HFCURLFile::s_SchemeName() + L"://" + Itr->fileName);

        // Remove (x:n:format) from mpf files since we want to refer the file, not the page

        // Find the file extension
        WString::size_type DotPos = FileName.rfind(L'.');
        WString::size_type SepPos = FileName.rfind(L"\\");

        if ((DotPos != WString::npos)
            && (BeStringUtilities::Wcsnicmp(FileName.c_str() + DotPos + 1, L"mpf", 3) == 0)
            && (SepPos < DotPos)
            && (FileName.c_str()[DotPos+4] == '(')
            && (FileName.c_str()[FileName.length()-1] == ')'))
            {
            // Remove page related stuff
            FileName = FileName.substr(0, DotPos+4);
            }

        // Create related files
        HFCPtr<HFCURL> pSubFileURL = new HFCURLFile(FileName);

        pio_rRelatedURLs.push_back(pSubFileURL);
        }

    return true;
    }

/** ---------------------------------------------------------------------------
    Get capabilities of RST file format

    @return RST format capabilities
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFIrasbRSTCreator::GetCapabilities()
    {
    return m_pCapabilities;
    }


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
 */
HRFIrasbRSTFile::HRFIrasbRSTFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 HFCAccessMode         pi_AccessMode,
                                 uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    HRFIrasbRSTCreator::GetInstance()->OpenFile(pi_rpURL, m_listOfFileInfo, pi_Offset);
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
 */
HRFIrasbRSTFile::~HRFIrasbRSTFile()
    {
    }

/** ---------------------------------------------------------------------------
    Get capabilities of RST file format
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFIrasbRSTFile::GetCapabilities () const
    {
    return (HRFIrasbRSTCreator::GetInstance()->GetCapabilities());
    }

/** ---------------------------------------------------------------------------
    Create RST line editor for data manipulation
    ---------------------------------------------------------------------------
 */
HRFResolutionEditor* HRFIrasbRSTFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                             unsigned short pi_Resolution,
                                                             HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return 0;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIrasbRSTFile::GetFileCurrentSize() const
    {
    HASSERT(0);

    return 0;
    /*
    HFCLockMonitor SisterFileLock (GetLockManager());

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);



    HAutoPtr<HFCBinStream> pFile;
    bool                   Result  = false;
    HFCLockMonitor           SisterFileLock (GetLockManager());

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    */
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFIrasbRSTFile::Save()
    {

    //Nothing to do here
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------

void HRFIrasbRSTFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                             uint32_t pi_Page,
                                             bool   pi_CheckSpecificUnitSpec,
                                             bool   pi_InterpretUnitINTGR)
    {
    //The format is currently not supported by RM, so disable this function
    //for now.
    }

/** ---------------------------------------------------------------------------
    Get the world identificator of the RST file format.
    ---------------------------------------------------------------------------
 */
const HGF2DWorldIdentificator HRFIrasbRSTFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

/** ---------------------------------------------------------------------------
    Get the list of RST file info
    ---------------------------------------------------------------------------
 */
const list<RSTSubFileInfo>& HRFIrasbRSTFile::GetFileInfoList () const
    {
    return m_listOfFileInfo;
    }
