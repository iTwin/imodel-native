//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFHGRPageFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFHGRPageFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HRFHGRPageFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCIniFileBrowser.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFUtility.h>

// HGR SPECIFIC DEFINE
#define ID                  "ID"
#define FILE                "File"
#define VERSION             "Version"
#define GEOREFSETTING       "GeoRefSetting"
#define ORIGIN_LOWER_LEFT_X "Origin_Lower_Left_X"
#define ORIGIN_LOWER_LEFT_Y "Origin_Lower_Left_Y"
#define PIXEL_SIZE_X        "Pixel_Size_X"
#define PIXEL_SIZE_Y        "Pixel_Size_Y"
#define IMAGE_WIDTH         "Image_Width"
#define IMAGE_HEIGHT        "Image_Height"
#define ROTATION            "Rotation"
#define AFFINITY            "Affinity"
#define IMAGEINFO           "ImageInfo"
#define IMAGE_OWNER         "Image_Owner"
#define IMAGE_DESCRIPTION   "Image_Description"
#define SCANNING_RES_X      "Scanning_Res_X"
#define SCANNING_RES_Y      "Scanning_Res_Y"

#define FILE_VALUE              "HMRGeoReferenceFile"
#define FILE_VERSION_CREATION   "2.2"


#define AFFINITY_TOLERANCE  6e-12


//-----------------------------------------------------------------------------
// Class HRFHGRCapabilities
//-----------------------------------------------------------------------------
HRFHGRCapabilities::HRFHGRCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));


    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Tags
    Add(new HRFTagCapability(HFC_READ_ONLY | HFC_CREATE_ONLY, new HRFAttributeHGRFile("")));
    Add(new HRFTagCapability(HFC_READ_ONLY | HFC_CREATE_ONLY, new HRFAttributeHGRVersion("")));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHGROwner("")));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHGRDescription("")));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0)));

    }


// Singleton
HFC_IMPLEMENT_SINGLETON(HRFHGRPageFileCreator)

//-----------------------------------------------------------------------------
// Class HRFHGRPageFileCreator
//
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 @return true if the file sister file HGR associate with the Raster file
         is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true : Try to find an associate the sister file HGR with
                     all types of files(ITiff, GeoTiff, etc)
                     false(default) : Try to find an associate sister file HGR
                     only with the files don't support Georeference.
-----------------------------------------------------------------------------*/
bool HRFHGRPageFileCreator::HasFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                    bool pi_ApplyonAllFiles) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    bool HasPageFile = false;

    // Don't forget to synchronize HRFTFWPageFile.cpp too.

    // TR 108925 HFC_READ_ONLY must be used because we don't want sister of georeferenced file like sid and ecw if pi_ApplyonAllFiles is OFF.
    //(pi_ApplyonAllFiles || !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_CREATE_ONLY))) )

    // We added pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) to fix a problem caused by the TR 162805
    //  This TR adds the capability of TransfoModel on CAL file only to support SLO.

    // Check only the first page in the File.
    if ((pi_rpForRasterFile->GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID)) &&
        (pi_ApplyonAllFiles || pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) ||
         !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_READ_ONLY))) )
        {
        HFCPtr<HFCURL>  URLForPageFile = ComposeURLFor(pi_rpForRasterFile->GetURL());
        if (URLForPageFile != 0)
            {
            HFCStat PageFileStat(URLForPageFile);

            // Check if the decoration file exist and the time stamp.
            if (PageFileStat.IsExistent())
                HasPageFile = true;
            }
        }

    return HasPageFile;
    }

//-----------------------------------------------------------------------------
// public
// CreateFor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageFile> HRFHGRPageFileCreator::CreateFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);

    HFCPtr<HRFPageFile> pPageFile;

    if (pi_rpForRasterFile->GetAccessMode().m_HasCreateAccess)
        {
        HFCPtr<HRFResolutionDescriptor> pResDesc;

        pResDesc = pi_rpForRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0);

        HASSERT(pResDesc->GetWidth() <= ULONG_MAX);
        HASSERT(pResDesc->GetHeight() <= ULONG_MAX);

        pPageFile = new HRFHGRPageFile(ComposeURLFor(pi_rpForRasterFile->GetURL()),
                                       (uint32_t)pResDesc->GetWidth(),
                                       (uint32_t)pResDesc->GetHeight(),
                                       pi_rpForRasterFile->GetAccessMode());
        }
    else
        {
        pPageFile = new HRFHGRPageFile(ComposeURLFor(pi_rpForRasterFile->GetURL()),
                                       pi_rpForRasterFile->GetAccessMode());
        }

    return pPageFile;
    }

//-----------------------------------------------------------------------------
// public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFHGRPageFileCreator::ComposeURLFor(const HFCPtr<HFCURL>&   pi_rpURLFileName) const
    {
    HPRECONDITION(pi_rpURLFileName != 0);

    HFCPtr<HFCURL>  URLForPageFile;

    if (!pi_rpURLFileName->IsCompatibleWith(HFCURLFile::CLASS_ID))
        throw HFCInvalidUrlForSisterFileException(pi_rpURLFileName->GetURL());

    // Decompose the file name
    WString DriveDirName;

    // Extract the Path
    WString Path(((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetHost()+WString(L"\\")+((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetPath());

    // Find the file extension
    WString::size_type DotPos = Path.rfind(L'.');

    // Extract the extension and the drive dir name
    if (DotPos != WString::npos)
        {
        // Compose the decoration file name
        DriveDirName = Path.substr(0, DotPos);
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + DriveDirName + WString(L".hgr"));
        }
    else
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + Path + WString(L".hgr"));

    return URLForPageFile;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFHGRPageFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFHGRCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Class HRFHGRPageFile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFHGRPageFile::HRFHGRPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                               HFCAccessMode           pi_AccessMode)
    : HRFPageFile(pi_rpURL, pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);

    m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode, 0, true);

    ReadFile();
    if (!IsValidHGRFile())
        throw HRFInvalidSisterFileException(pi_rpURL->GetURL());

    CreateDescriptor();
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFHGRPageFile::HRFHGRPageFile(const HFCPtr<HFCURL>&    pi_rpURL,
                               uint32_t                 pi_Width,
                               uint32_t                 pi_Height,
                               HFCAccessMode            pi_AccessMode,
                               FileVersion              pi_Version)
    : HRFPageFile(pi_rpURL, pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);
    HPRECONDITION(pi_Version < HRFHGRPageFile::UNSUPPORTED_VERSION);

    if (pi_AccessMode == HFC_CREATE_ONLY)
        m_pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_CREATE_ONLY | HFC_WRITE_ONLY , 0, true);
    else
        m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode, 0, true);

    // initialize members
    m_File = FILE_VALUE;
    m_Version = pi_Version;

    m_ImageWidth = pi_Width;
    m_ImageHeight = pi_Height;

    if (m_Version == VERSION_2_2)
        {
        m_ScanningResX = 300;
        m_ScanningResY = 300;
        }
    else
        {
        m_ScanningResX = 0;
        m_ScanningResY = 0;
        }

    m_OriginX = 0.0;
    m_OriginY = 0.0;
    m_PixelSizeX = 1.0;
    m_PixelSizeY = 1.0;
    m_Rotation = 0.0;
    m_Affinity = 0.0;

    CreateDescriptor();
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// This function do nothing, since the transformation described by an HGR file
// is always in meters.
//-----------------------------------------------------------------------------
void HRFHGRPageFile::SetDefaultRatioToMeter(double pi_RatioToMeter)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFHGRPageFile::~HRFHGRPageFile()
    {
    // Obtain the Page descriptor
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

    bool UpdateFile = false;
    if (GetAccessMode().m_HasCreateAccess)
        UpdateFile = true;
    else if (pPageDescriptor->HasTransfoModel() && pPageDescriptor->TransfoModelHasChanged())
        UpdateFile = true;

    if (UpdateFile)
        {
        WriteToDisk();
        }
    }

//-----------------------------------------------------------------------------
// Public
// WriteToDisk
// Write current transformation to disk
//----------------------------------------------------------------------------
void HRFHGRPageFile::WriteToDisk()
    {
    try
        {
        WriteFile();
        m_pFile->Flush();

        //// Reopen R/W with share access for Projectwise
        //if (m_pFile->GetAccessMode().m_HasWriteAccess && !(m_pFile->GetAccessMode().m_HasReadShare))
        //    {
        //    m_pFile = 0;    // Close the file
        //    m_pFile = HFCBinStream::Instanciate(GetURL(), HFC_READ_WRITE_OPEN | HFC_SHARE_READ_WRITE, 0, true);
        //    }
        }
    catch(...)
        {
        // Errors can happen, but they surely can't propagate in a destructor!
        // Break anyway so that we can make sure it's not a fatal error.
#if defined (ANDROID) || defined (__APPLE__)
        HASSERT(!L"Write error HGR file");
#elif defined (_WIN32)
        HDEBUGCODE(DebugBreak();); 
#endif
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFHGRPageFile::GetCapabilities () const
    {
    return HRFHGRPageFileCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFHGRPageFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }


//-----------------------------------------------------------------------------
// Private section
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Private
// GetFileVersion
//-----------------------------------------------------------------------------
HRFHGRPageFile::FileVersion HRFHGRPageFile::GetFileVersion(const string& pi_rVersion) const
    {
    HPRECONDITION(!pi_rVersion.empty());

    FileVersion Version = UNSUPPORTED_VERSION;
    if (strcmp(pi_rVersion.c_str(), "2.0") == 0)
        Version = VERSION_2_0;
    else if (strcmp(pi_rVersion.c_str(), "2.1") == 0)
        Version = VERSION_2_1;
    else if (strcmp(pi_rVersion.c_str(), "2.2") == 0)
        Version = VERSION_2_2;
    else
        throw HRFUnsupportedHGRVersionException(GetURL()->GetURL());


    return Version;
    }

//-----------------------------------------------------------------------------
// Private
// GetFileVersion
//-----------------------------------------------------------------------------
string HRFHGRPageFile::GetFileVersion(FileVersion pi_Version) const
    {
    HPRECONDITION(pi_Version < UNSUPPORTED_VERSION);
    string Version;
    switch(pi_Version)
        {
        case VERSION_2_0:
            Version = "2.0";
            break;

        case VERSION_2_1:
            Version = "2.1";
            break;

        case VERSION_2_2:
            Version = "2.2";
            break;
        default:
            HASSERT(!L"Unknown HGR version");
        }
    return Version;
    }

//-----------------------------------------------------------------------------
// Private
// ReadFile
//-----------------------------------------------------------------------------
void HRFHGRPageFile::ReadFile()
    {
    HPRECONDITION(m_pFile != 0);

    HFCIniFileBrowser InitFile(m_pFile);
    string            Value;

    // Read ID section
    if (!InitFile.FindTopic(ID))
        throw HRFSisterFileMissingParamGroupException(GetURL()->GetURL(),
                                        WString(ID,false));

    // File
    if (!InitFile.GetVariableValue(FILE, m_File))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(FILE,false));

    // Version
    if (!InitFile.GetVariableValue(VERSION, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(VERSION,false));

    m_Version = GetFileVersion(Value);

    // Read the GoeRefSetting section
    if (!InitFile.FindTopic(GEOREFSETTING))
        throw HRFSisterFileMissingParamGroupException(GetURL()->GetURL(),
                                        WString(GEOREFSETTING,false));

    // OriginX
    if (!InitFile.GetVariableValue(ORIGIN_LOWER_LEFT_X, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(ORIGIN_LOWER_LEFT_X,false));

    if (!ConvertStringToDouble(Value, &m_OriginX))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(ORIGIN_LOWER_LEFT_X,false));

    // OriginY
    if (!InitFile.GetVariableValue(ORIGIN_LOWER_LEFT_Y, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(ORIGIN_LOWER_LEFT_Y,false));

    if (!ConvertStringToDouble(Value, &m_OriginY))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(ORIGIN_LOWER_LEFT_Y,false));

    // PixelSizeX
    if (!InitFile.GetVariableValue(PIXEL_SIZE_X, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(PIXEL_SIZE_X,false));

    if (!ConvertStringToDouble(Value, &m_PixelSizeX))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(PIXEL_SIZE_X,false));

    // PixelSizeY
    if (!InitFile.GetVariableValue(PIXEL_SIZE_Y, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(PIXEL_SIZE_Y,false));

    if (!ConvertStringToDouble(Value, &m_PixelSizeY))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(PIXEL_SIZE_Y,false));

    // ImageWidth
    if (!InitFile.GetVariableValue(IMAGE_WIDTH, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(IMAGE_WIDTH,false));

    if (!ConvertStringToUnsignedLong(Value, &m_ImageWidth))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(IMAGE_WIDTH,false));

    // ImageHeight
    if (!InitFile.GetVariableValue(IMAGE_HEIGHT, Value))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(IMAGE_HEIGHT,false));

    if (!ConvertStringToUnsignedLong(Value, &m_ImageHeight))
        throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                        WString(IMAGE_HEIGHT,false));

    m_Rotation = 0.0;
    m_Affinity = 0.0;
    if (m_Version >= VERSION_2_1)
        {
        // Rotation
        if (!InitFile.GetVariableValue(ROTATION, Value))
            throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                            WString(ROTATION,false));

        if (!ConvertStringToDouble(Value, &m_Rotation))
            throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                            WString(ROTATION,false));

        // Affinity
        if (!InitFile.GetVariableValue(AFFINITY, Value))
            throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                            WString(AFFINITY,false));

        if (!ConvertStringToDouble(Value, &m_Affinity))
            throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                            WString(AFFINITY,false));
        }

    // Read ImageInfo section
    if (!InitFile.FindTopic(IMAGEINFO))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(IMAGEINFO,false));

    if (!InitFile.GetVariableValue(IMAGE_OWNER, m_Owner))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(IMAGE_OWNER,false));

    if (!InitFile.GetVariableValue(IMAGE_DESCRIPTION, m_Description))
        throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                        WString(IMAGE_DESCRIPTION,false));

    if (m_Version>= VERSION_2_2)
        {
        // ScanningResX
        if (!InitFile.GetVariableValue(SCANNING_RES_X, Value))
            throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                            WString(SCANNING_RES_X,false));

        double ScanningRes;
        if (!ConvertStringToDouble(Value, &ScanningRes))
            throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                            WString(SCANNING_RES_X,false));

        m_ScanningResX = (uint32_t)ScanningRes;

        // ScanningResY
        if (!InitFile.GetVariableValue(SCANNING_RES_Y, Value))
            throw HRFSisterFileMissingParamException(GetURL()->GetURL(),
                                            WString(SCANNING_RES_Y,false));

        if (!ConvertStringToDouble(Value, &ScanningRes))
            throw HRFSisterFileInvalidParamValueException(GetURL()->GetURL(),
                                            WString(SCANNING_RES_Y,false));

        m_ScanningResY = (uint32_t)ScanningRes;
        }
    }

//-----------------------------------------------------------------------------
// Private
// IsValidHGRFile
// Multiple returns for simplification.
//-----------------------------------------------------------------------------
bool HRFHGRPageFile::IsValidHGRFile() const
    {
    bool Result = true;

    // check if we have a right file
    if (BeStringUtilities::Stricmp(m_File.c_str(), FILE_VALUE) != 0)
        return false;
    else if (m_Version == UNSUPPORTED_VERSION)
        return false;

    // Validate affinity. Invalid values are 90, -90,
    // 270, -270 with a precision of 6e-12.
    double Affinity = m_Affinity;
    while (Affinity > 360)
        Affinity -= 360;
    while (Affinity < -360)
        Affinity += 360;
    if (HDOUBLE_EQUAL(Affinity, 90.0, AFFINITY_TOLERANCE) ||
        HDOUBLE_EQUAL(Affinity, -90.0, AFFINITY_TOLERANCE) ||
        HDOUBLE_EQUAL(Affinity, 270.0, AFFINITY_TOLERANCE) ||
        HDOUBLE_EQUAL(Affinity, -270.0, AFFINITY_TOLERANCE))
        return false;

    // Pixelsize cannot be 0 +- 1e-14
    if (HDOUBLE_EQUAL(m_PixelSizeX, 0.0, HEPSILON_MULTIPLICATOR) ||
        HDOUBLE_EQUAL(m_PixelSizeY, 0.0, HEPSILON_MULTIPLICATOR))
        return false;

    // Limit the pixelsize inside [-2e294, 2e294]
    if (m_PixelSizeX > HMAX_EPSILON || m_PixelSizeX < -HMAX_EPSILON ||
        m_PixelSizeY > HMAX_EPSILON || m_PixelSizeY < -HMAX_EPSILON)
        return false;

    // dimensions must be positive, not null
    if (m_ImageWidth <= 0 || m_ImageHeight <= 0)
        return false;

    // Origin must be greater that -2e294
    if (m_OriginX < -HMAX_EPSILON || m_OriginY < -HMAX_EPSILON)
        return false;

    // Verify width overflow (max must be smaller
    // than 2e294)
    if (m_OriginX + m_PixelSizeX* m_ImageWidth >= HMAX_EPSILON)
        return false;

    // Verify height overflow (max must be smaller
    // than 2e294)
    if (m_OriginY + m_PixelSizeY* m_ImageHeight >= HMAX_EPSILON)
        return false;

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
// CreateDescriptor
//-----------------------------------------------------------------------------
void HRFHGRPageFile::CreateDescriptor()
    {
    HPRECONDITION(IsValidHGRFile());

    HPMAttributeSet     TagList;
    HFCPtr<HPMGenericAttribute> pTag;
    // ID section
    pTag = new HRFAttributeHGRFile(m_File);
    TagList.Set(pTag);
    pTag = new HRFAttributeHGRVersion(GetFileVersion(m_Version));
    TagList.Set(pTag);

    // ImageInfo section
    pTag = new HRFAttributeHGROwner(m_Owner);
    TagList.Set(pTag);
    pTag = new HRFAttributeHGRDescription(m_Description);
    TagList.Set(pTag);

    if (m_Version >= VERSION_2_2)
        {
        pTag = new HRFAttributeXResolution(m_ScanningResX);
        TagList.Set(pTag);

        pTag = new HRFAttributeYResolution(m_ScanningResY);
        TagList.Set(pTag);
        }

    HFCPtr<HGF2DTransfoModel> pTransfoModel = BuildTransfoModel(m_OriginX,
                                                                m_OriginY,
                                                                m_PixelSizeX,
                                                                m_PixelSizeY,
                                                                m_Affinity,
                                                                m_Rotation);
    HRFScanlineOrientation TransfoModelSLO = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;

    // Create the Page information and add it to the list of page descriptor
    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor(GetAccessMode(),
                                  GetCapabilities(),    // Capabilities,
                                  0,                    // RepresentativePalette,
                                  0,                    // Histogram,
                                  0,                    // Thumbnail,
                                  0,                    // ClipShape,
                                  pTransfoModel,        // TransfoModel,
                                  &TransfoModelSLO,     // TransfoModelOrientation
                                  0,                    // Filters
                                  &TagList);            // Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Private
// ConvertStringToDouble
//-----------------------------------------------------------------------------
bool HRFHGRPageFile::ConvertStringToDouble(const string& pi_rString, double* po_pDouble) const
    {
    HPRECONDITION(po_pDouble != 0);

    char* pStopPtr;
    *po_pDouble = strtod(pi_rString.c_str(), &pStopPtr);

    return (pStopPtr - pi_rString.c_str() == pi_rString.length());
    }

//-----------------------------------------------------------------------------
// Private
// ConvertStringToUndignedLong
//-----------------------------------------------------------------------------
bool HRFHGRPageFile::ConvertStringToUnsignedLong(const string& pi_rString, uint32_t* po_pLong) const
    {
    HPRECONDITION(po_pLong != 0);

    char* pStopPtr;
    *po_pLong= strtol(pi_rString.c_str(), &pStopPtr, 10);

    return (pStopPtr - pi_rString.c_str() == pi_rString.length());
    }

//-----------------------------------------------------------------------------
// Private
// BuildTransfoModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFHGRPageFile::BuildTransfoModel(double pi_OriginX,
                                                            double pi_OriginY,
                                                            double pi_PixelSizeX,
                                                            double pi_PixelSizeY,
                                                            double pi_Affinity,
                                                            double pi_Rotation) const
    {

    // Valid that the Affinity value is in the range of [-360,360]
    while (pi_Affinity > 360)
        pi_Affinity -= 360;
    while (pi_Affinity < -360)
        pi_Affinity += 360;

    // The affinity cannot be 90 degrees, or -90 degrees
    // Note that exact compare operations are intentional.
    HASSERT(pi_Affinity != 90.0 && pi_Affinity != -90.0);

    // Check that the Affinity is in the range of ]-90,90[
    // IMPORTANT:
    //    Do not change the order of testing.
    if (pi_Affinity > 270)
        {
        pi_Affinity -= 360;
        }
    else if (pi_Affinity > 90)
        {
        pi_Affinity -= 180;
#if (0)
        pi_Rotation += 180;
#else
        pi_PixelSizeY = -pi_PixelSizeY;
#endif
        }
    else if (pi_Affinity < -270)
        {
        pi_Affinity += 360;
        }
    else if (pi_Affinity < -90)
        {
        pi_Affinity += 180;
#if (0)
        pi_Rotation += 180;
#else
        pi_PixelSizeY = -pi_PixelSizeY;
#endif
        }

    // Valid that the rotation value is in the range of [-360,360]
    while (pi_Rotation > 360)
        pi_Rotation -= 360;
    while (pi_Rotation < -360)
        pi_Rotation += 360;

    // Build the transformation model.
    HFCPtr<HGF2DTransfoModel> pModel;
    double            Degree(PI/180);
    pModel = new HGF2DAffine(HGF2DDisplacement(pi_OriginX,
                                               pi_OriginY),
                             pi_Rotation * Degree,
                             pi_PixelSizeX,
                             pi_PixelSizeY,
                             pi_Affinity * Degree);


    return pModel;
    }

//-----------------------------------------------------------------------------
// Private
// Write the data into the HGR file.  To do this we need to rewrite the entire
// file.
//-----------------------------------------------------------------------------
void HRFHGRPageFile::WriteFile()
    {
    HPRECONDITION(m_pFile != 0);
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);

    double Affinity   = 0.0;
    double OriginX    = 0.0;
    double OriginY    = 0.0;
    double PixelSizeX = 0.0;
    double PixelSizeY = 0.0;
    double Rotation   = 0.0;

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

    // Check if the transformation can be represented by a matrix.
    HFCPtr<HGF2DTransfoModel> pModel = pPageDescriptor->GetTransfoModel();
    if (pModel->CanBeRepresentedByAMatrix())
        {
        // Create a projective from the current transformation matrix.
        HGF2DProjective Projective(pModel->GetMatrix());

        // Extract information from the projective.
//        double       Degree = PI/180;
        Affinity   = Projective.GetAnorthogonality() * 180.0 / PI;
        Rotation   = Projective.GetRotation() * 180.0 / PI;
        PixelSizeX = Projective.GetXScaling();
        PixelSizeY = Projective.GetYScaling();
        OriginX    = Projective.GetTranslation().GetDeltaX();
        OriginY    = Projective.GetTranslation().GetDeltaY();
        }
    else
        {
        throw HRFInvalidTransfoForSisterFileException(m_pFile->GetURL()->GetURL());
        }

    double ScanningResX;
    double ScanningResY;

    HRFAttributeXResolution const* pTagXResolution = pPageDescriptor->FindTagCP<HRFAttributeXResolution>();
    ScanningResX = pTagXResolution->GetData();

    HRFAttributeYResolution const* pTagYResolution = pPageDescriptor->FindTagCP<HRFAttributeYResolution>();
    ScanningResY = pTagYResolution->GetData();

    // change the file version if we need it to save all informations
    if (!GetAccessMode().m_HasCreateAccess)
        {
        FileVersion WriteVersion = VERSION_2_0;
        if (ScanningResX > 0.0 && ScanningResY > 0.0)
            WriteVersion = VERSION_2_2;
        else if (ScanningResX == 0.0 && ScanningResY == 0.0)
            {
            if (!HDOUBLE_EQUAL_EPSILON(Rotation, 0.0) || !HDOUBLE_EQUAL_EPSILON(Affinity, 0.0))
                WriteVersion = VERSION_2_1;
            }
        HDEBUGCODE(else HASSERT(!L"Cannot find the HGR version with the given parameters");)

            if (WriteVersion > m_Version)
                m_Version = WriteVersion;
        }

    // reset the file
    m_pFile->SeekToBegin();

    string Data;
    size_t NewSize = 0;

    // ID section
    Data = "[" + string(ID) + "]" + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    // File
    Data = string(FILE) + string("=") + m_File + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    // Version
    Data = string(VERSION) + string("=") + GetFileVersion(m_Version) + string("\r\n\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    // GeoRefSetting section
    Data = string("[") + string(GEOREFSETTING) + string("]") + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    char aBuffer[256];

    Data = string(ORIGIN_LOWER_LEFT_X) + string("=");
    sprintf(aBuffer, "%.15g\r\n", OriginX);
    Data += string(aBuffer);
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    Data = string(ORIGIN_LOWER_LEFT_Y) + string("=");
    sprintf(aBuffer, "%.15g\r\n", OriginY);
    Data += string(aBuffer);
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    Data = string(PIXEL_SIZE_X) + string("=");
    sprintf(aBuffer, "%.15g\r\n", PixelSizeX);
    Data += string(aBuffer);
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    Data = string(PIXEL_SIZE_Y) + string("=");
    sprintf(aBuffer, "%.15g\r\n", PixelSizeY);
    Data += string(aBuffer);
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    sprintf(aBuffer, "%ld\r\n", m_ImageWidth);
    Data = string(IMAGE_WIDTH) + string("=") + aBuffer;
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    sprintf(aBuffer, "%ld\r\n", m_ImageHeight);
    Data = string(IMAGE_HEIGHT) + string("=") + aBuffer;
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    if (m_Version >= VERSION_2_1)
        {
        Data = string(ROTATION) + string("=");
        sprintf(aBuffer, "%.15g\r\n", Rotation);
        Data += string(aBuffer);
        NewSize += m_pFile->Write(Data.c_str(), Data.length());

        Data = string(AFFINITY) + string("=");
        sprintf(aBuffer, "%.15g\r\n", Affinity);
        Data += string(aBuffer);
        NewSize += m_pFile->Write(Data.c_str(), Data.length());
        }

    Data = "\r\n";
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    // ImageInfo section
    Data = string("[") + string(IMAGEINFO) + string("]") + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    HRFAttributeHGROwner const* pTagHGROwner = pPageDescriptor->FindTagCP<HRFAttributeHGROwner>();
    Data = string(IMAGE_OWNER) + string("=") + pTagHGROwner->GetData() + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    HRFAttributeHGRDescription const* pTagHGRDescription = pPageDescriptor->FindTagCP<HRFAttributeHGRDescription>();
    Data = string(IMAGE_DESCRIPTION) + string("=") + pTagHGRDescription->GetData() + string("\r\n");
    NewSize += m_pFile->Write(Data.c_str(), Data.length());

    if (m_Version >= VERSION_2_2)
        {
        // write
        sprintf(aBuffer, "%.15g\r\n", ScanningResX);
        Data = string(SCANNING_RES_X) + string("=") + aBuffer;
        NewSize += m_pFile->Write(Data.c_str(), Data.length());

        sprintf(aBuffer, "%.15g\r\n", ScanningResY);
        Data = string(SCANNING_RES_Y) + string("=") + aBuffer;
        NewSize += m_pFile->Write(Data.c_str(), Data.length());
        }

    if (m_pFile->GetSize() > NewSize)
        {
        NewSize = (size_t)(m_pFile->GetSize() - NewSize);
        memset(aBuffer, ' ', 256);

        // clear the end of the file
        while (NewSize > 256)
            {
            m_pFile->Write(aBuffer, 256);
            NewSize -= 256;
            }

        if (NewSize > 0)
            m_pFile->Write(aBuffer, NewSize);
        }

    m_pFile->Flush();
    }


