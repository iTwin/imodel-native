//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBilFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFBilFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFBilFile.h>
#include <Imagepp/all/h/HRFBilLineEditor.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFTWFPageFile.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

bool ConvertStringToDouble(const string& pi_rString, double* po_pDouble) ;
bool ConvertStringToLong(const string& pi_rString, int32_t* po_pLong) ;
bool ConvertStringToULong(const string& pi_rString, uint32_t* po_pLong) ;


//-----------------------------------------------------------------------------
// HRFBMPBlockCapabilities
//-----------------------------------------------------------------------------
class HRFBILBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBILBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,
                                  LONG_MAX,
                                  HRFBlockAccess::RANDOM));
        }
    };

//-----------------------------------------------------------------------------
// HRFBMPCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFBILCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBILCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFBILBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFBilCapabilities
//-----------------------------------------------------------------------------
HRFBilCapabilities::HRFBilCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFBILCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFBILCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFBILCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFBILCodecIdentityCapabilities()));

    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));

    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID     ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID    ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID   ));

    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxSampleValue));
    }

HFC_IMPLEMENT_SINGLETON(HRFBilCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate BIL format
//-----------------------------------------------------------------------------
HRFBilCreator::HRFBilCreator()
    : HRFRasterFileCreator(HRFBilFile::CLASS_ID)
    {
    // BIL capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFBilCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_Bil()); // BIL File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFBilCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFBilCreator::GetExtensions() const
    {
    return WString(L"*.bil");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFBilCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFBilFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);
    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFBilCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset)const
    {
    HPRECONDITION(pi_rpURL != 0);

    return IsKindOfFileWithExternalHeader(pi_rpURL, pi_Offset) ||
           IsKindOfFileWithInternalHeader(pi_rpURL, pi_Offset) ||
           IsKindOfFileOpenFromExternalHeader(pi_rpURL, pi_Offset);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is a BIL type with an ".hdr" file for the
// header.
//-----------------------------------------------------------------------------
bool HRFBilCreator::IsKindOfFileWithExternalHeader(const HFCPtr<HFCURL>& pi_rpURL,
                                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream>  pHdrFile;
    HFCPtr<HFCURL>          HdrUrl;
    WString                 UrlString(pi_rpURL->GetURL());
    HArrayAutoPtr<char>    Header;
    uint32_t                HeaderLength;
    char*                   ValueStartPos;
    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;

    size_t extPos = UrlString.rfind(L'.');

    if(extPos > 0 && extPos != WString::npos)
        {
        // Get the Url for the ".hdr" file.
        UrlString.replace(extPos + 1, wcslen(L"hdr"), L"hdr");
        HdrUrl = HFCURL::Instanciate(UrlString);

        HFCStat HDRFileFileStat(HdrUrl);
        if (HDRFileFileStat.IsExistent())
            {
            // This creates the sister file for file sharing control if necessary.
            (const_cast<HRFBilCreator*>(this))->HdrSharingControlCreate(HdrUrl);
            HFCLockMonitor SisterFileLock(GetHdrLockManager());

            // Open the hdr file.
            pHdrFile = HFCBinStream::Instanciate(HdrUrl, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

            if ((pHdrFile == 0) || (pHdrFile->GetLastException() != 0))
                {
                SisterFileLock.ReleaseKey();
                HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl->IsLocked());
                (const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl = 0;

                return false;
                }

            pHdrFile->SeekToEnd();
            HeaderLength = (uint32_t)pHdrFile->GetCurrentPos();

            // Performance problem, we read the file entire...not a good idea
            // TR 179480
            if (HeaderLength > 2048)
                return false;

            Header = new char[HeaderLength + 1];
            Header[HeaderLength] = '\0';
            pHdrFile->SeekToBegin();
            pHdrFile->Read(Header, HeaderLength);

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl->IsLocked());
            (const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl = 0;

            HdrUrl = 0;
            pHdrFile = 0;

            BeStringUtilities::Strupr(Header);
            ValueStartPos = strstr(Header, "LAYOUT");
            if (NULL != ValueStartPos)
                {
                ValueStartPos += 7;
                int i = 0;
                while (((' ' == ValueStartPos[i]) || ('\t' == ValueStartPos[i])) && ('\0' != ValueStartPos[i]))
                    i ++;

                if (strncmp(ValueStartPos + i, "BIL", 3) == 0)
                    {
                    i += 3;
                    if ((ValueStartPos[i] == ' ')   ||
                        (ValueStartPos[i] == '\0')  ||
                        (ValueStartPos[i] == '\t')  ||
                        (ValueStartPos[i] == '\r')  ||
                        (ValueStartPos[i] == '\n'))
                        {
                        //minimal size in bytes for the BIL file
                        //this is how we know we're opening a valid BIL file
                        uint32_t MinSizeBILFile = GetULongValueFromHeader(Header,"NROWS", pi_rpURL) *
                                                GetULongValueFromHeader(Header, "NCOLS", pi_rpURL);

                        (const_cast<HRFBilCreator*>(this))->SharingControlCreate(pi_rpURL);
                        HFCLockMonitor SisterFileLock(GetLockManager());

                        // Open the Bil File & place file pointer at the start of the file
                        pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

                        if (pFile != 0 && pFile->GetLastException() == 0)
                            {
                            pFile->SeekToEnd();
                            uint64_t SizeOfFile = pFile->GetCurrentPos();
                            if(SizeOfFile >= MinSizeBILFile )
                                bResult = true;
                            }

                        SisterFileLock.ReleaseKey();
                        HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pSharingControl->IsLocked());
                        (const_cast<HRFBilCreator*>(this))->m_pSharingControl = 0;

                        pFile = 0;
                        }
                    }
                }
            else
                {
                int NbBands = 0;
                ValueStartPos = strstr(Header, "NBANDS");
                if (NULL != ValueStartPos)
                    {
                    ValueStartPos += 7;
                    if (NULL != ValueStartPos)
                        {
                        NbBands = atoi((const char*)ValueStartPos);
                        }
                    }
                if(((NULL != ValueStartPos) && (NbBands == 1 || NbBands == 3)) ? true : false)
                    {
                    //minimal size in bytes for the BIL file
                    //this is how we know we're opening a valid BIL file
                    uint32_t MinSizeBILFile = GetULongValueFromHeader(Header, "NROWS", pi_rpURL) *
                                            GetULongValueFromHeader(Header, "NCOLS", pi_rpURL);

                    (const_cast<HRFBilCreator*>(this))->SharingControlCreate(pi_rpURL);
                    HFCLockMonitor SisterFileLock(GetLockManager());

                    // Open the Bil File & place file pointer at the start of the file
                    pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

                    if (pFile != 0 && pFile->GetLastException() == 0)
                        {
                        pFile->SeekToEnd();
                        uint64_t SizeOfFile = pFile->GetCurrentPos();
                        if(SizeOfFile >= MinSizeBILFile )
                            bResult = true;
                        }

                    SisterFileLock.ReleaseKey();
                    HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pSharingControl->IsLocked());
                    (const_cast<HRFBilCreator*>(this))->m_pSharingControl = 0;

                    pFile = 0;
                    }
                }
            }
        }

    return bResult;
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is BIL type with both data and header
// in the same file.
//-----------------------------------------------------------------------------
bool HRFBilCreator::IsKindOfFileWithInternalHeader(const HFCPtr<HFCURL>& pi_rpURL,
                                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;
    RawPixelFileHeader      FileHeader;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFBilCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the BMP File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if ((pFile != 0) && (pFile->GetLastException() == 0))
        {
        pFile->Read(&FileHeader.identifier, 4);

        // Verify that the type is BMP image
        if (strncmp((char*)FileHeader.identifier, RPIX_IDENTIFIER, 4) == 0)
            {
            pFile->Read(&FileHeader.hdrlength,      sizeof FileHeader.hdrlength);
            pFile->Read(&FileHeader.majorversion,   sizeof FileHeader.majorversion);
            pFile->Read(&FileHeader.minorversion,   sizeof FileHeader.minorversion);
            pFile->Read(&FileHeader.width,          sizeof FileHeader.width);
            pFile->Read(&FileHeader.height,         sizeof FileHeader.height);
            pFile->Read(&FileHeader.comptype,       sizeof FileHeader.comptype);
            pFile->Read(&FileHeader.interleave,     sizeof FileHeader.interleave);
            pFile->Read(&FileHeader.numbands,       sizeof FileHeader.numbands);
            pFile->Read(&FileHeader.rchannel,       sizeof FileHeader.rchannel);
            pFile->Read(&FileHeader.gchannel,       sizeof FileHeader.gchannel);
            pFile->Read(&FileHeader.bchannel,       sizeof FileHeader.bchannel);
            pFile->Read(&FileHeader.reserved,       sizeof FileHeader.reserved);

            if((RPIX_MAJOR_VERSION == FileHeader.majorversion)  &&
               (RPIX_MINOR_VERSION == FileHeader.minorversion)  &&
               (RPIX_COMPRESSION_NONE == FileHeader.comptype)   &&
               (RPIX_INTERLEAVING_BIL == FileHeader.interleave))
                {
                bResult = true;
                }
            }

        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFBilCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is BIL type with both data and header
// in the same file.
//-----------------------------------------------------------------------------
bool HRFBilCreator::IsKindOfFileOpenFromExternalHeader(const HFCPtr<HFCURL>& pi_rpURL,
                                                        uint64_t                 pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream>  pHdrFile;
    HFCPtr<HFCURL>          BILUrl;
    WString                 UrlString(pi_rpURL->GetURL());
    HArrayAutoPtr<char>    Header;
    uint32_t                HeaderLength;
    char*                   ValueStartPos;
    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;

    // This creates the sister file for file sharing control if necessary.
    (const_cast<HRFBilCreator*>(this))->HdrSharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetHdrLockManager());

    // Open the hdr file.
    pHdrFile = HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if ((pHdrFile == 0) || (pHdrFile->GetLastException() != 0))
        {
        SisterFileLock.ReleaseKey();
        HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl->IsLocked());
        (const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl = 0;

        return false;
        }

    pHdrFile->SeekToEnd();
    HeaderLength = (uint32_t)pHdrFile->GetCurrentPos();

    // Performance problem, we read the file entire...not a good idea
    // TR 179480
    if (HeaderLength > 2048)
        {
        SisterFileLock.ReleaseKey();
        HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl->IsLocked());
        (const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl = 0;

        return false;
        }

    Header = new char[HeaderLength + 1];
    Header[HeaderLength] = '\0';
    pHdrFile->SeekToBegin();
    pHdrFile->Read(Header, HeaderLength);

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl->IsLocked());
    (const_cast<HRFBilCreator*>(this))->m_pHdrSharingControl = 0;

    pHdrFile = 0;

    BeStringUtilities::Strupr(Header);
    ValueStartPos = strstr(Header, "LAYOUT");
    if (NULL != ValueStartPos)
        {
        ValueStartPos += 7;
        int i = 0;
        while (((' ' == ValueStartPos[i]) || ('\t' == ValueStartPos[i])) && ('\0' != ValueStartPos[i]))
            i ++;

        if (strncmp(ValueStartPos + i, "BIL", 3) == 0)
            {
            i += 3;
            if ((ValueStartPos[i] == ' ')   ||
                (ValueStartPos[i] == '\0')  ||
                (ValueStartPos[i] == '\t')  ||
                (ValueStartPos[i] == '\r')  ||
                (ValueStartPos[i] == '\n'))
                {
                // Get the Url for the ".bil" file, because we might be opening from a header file
                UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"bil"), L"bil");
                BILUrl = HFCURL::Instanciate(UrlString);

                (const_cast<HRFBilCreator*>(this))->SharingControlCreate(BILUrl);
                HFCLockMonitor SisterFileLock(GetLockManager());

                // Open the BMP File & place file pointer at the start of the file
                pFile = HFCBinStream::Instanciate(BILUrl, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

                if ((pFile != 0) && (pFile->GetLastException() == 0))
                    bResult = true;

                SisterFileLock.ReleaseKey();
                HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pSharingControl->IsLocked());
                (const_cast<HRFBilCreator*>(this))->m_pSharingControl = 0;

                pFile = 0;
                }
            }
        }
    else
        {
        int NbBands = 0;
        ValueStartPos = strstr(Header, "NBANDS");
        if (NULL != ValueStartPos)
            {
            ValueStartPos += 7;
            if (NULL != ValueStartPos)
                {
                NbBands = atoi((const char*)ValueStartPos);
                }
            }
        if(((NULL != ValueStartPos) && (NbBands == 1 || NbBands == 3)) ? true : false)
            {
            // Get the Url for the ".bil" file, because we might be opening from a header file
            UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"bil"), L"bil");
            BILUrl = HFCURL::Instanciate(UrlString);

            (const_cast<HRFBilCreator*>(this))->SharingControlCreate(BILUrl);
            HFCLockMonitor SisterFileLock(GetLockManager());

            // Open the BMP File & place file pointer at the start of the file
            pFile = HFCBinStream::Instanciate(BILUrl, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

            if ((pFile != 0) && (pFile->GetLastException() == 0))
                bResult = true;

            SisterFileLock.ReleaseKey();
            HASSERT(!(const_cast<HRFBilCreator*>(this))->m_pSharingControl->IsLocked());
            (const_cast<HRFBilCreator*>(this))->m_pSharingControl = 0;

            pFile = 0;
            }
        }

    return bResult;
    }

//-----------------------------------------------------------------------------
// GetValueFromHeader
// Protected
// Gets the value associated with a keyword in the header.
//-----------------------------------------------------------------------------
uint32_t HRFBilCreator::GetULongValueFromHeader(const char*           pi_Header,
                                              const char*           pi_Keyword,
                                              const HFCPtr<HFCURL>& pi_rpURL) const
    {
    HPRECONDITION(NULL != pi_Header);
    HPRECONDITION(NULL != pi_Keyword);

    const char*   ValueStartPos;
    uint32_t  Result = 0;

    ValueStartPos = strstr(pi_Header, pi_Keyword);
    if (NULL != ValueStartPos)
        {
        ValueStartPos += strlen(pi_Keyword);
        if (!HRFBilFile::IsValidChar(*ValueStartPos))
            {
            ValueStartPos = strpbrk(ValueStartPos, "-0123456789");
            if (NULL != ValueStartPos)
                {
                if (!ConvertStringToULong(ValueStartPos, &Result))
                    throw HFCCorruptedFileException(pi_rpURL->GetURL());
                }
            }
        }
    return Result;
    }




//-----------------------------------------------------------------------------
// Get list of related files from a given URL.
//-----------------------------------------------------------------------------
bool HRFBilCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                    ListOfRelatedURLs&    pio_rRelatedURLs) const
    {
    HASSERT (pio_rRelatedURLs.size() == 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;
    
    // Find the file extension
    WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    WString::size_type DotPos = Path.rfind(L'.');

    if (DotPos != WString::npos)
        Path = Path.substr(0, DotPos);

    // Compose url for .hdr, and .bnd files
    WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                       + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() + WString(L"\\")
                       + Path;

    // Create related files
    HFCPtr<HFCURL> pHDRFileURL = new HFCURLFile(FileName + WString(L".hdr"));
    HFCPtr<HFCURL> pBNDFileURL = new HFCURLFile(FileName + WString(L".bnd"));
    HFCPtr<HFCURL> pSTXFileURL = new HFCURLFile(FileName + WString(L".stx"));

    pio_rRelatedURLs.push_back(pHDRFileURL);
    pio_rRelatedURLs.push_back(pBNDFileURL);
    pio_rRelatedURLs.push_back(pSTXFileURL);

    return true;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of BIL file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFBilCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFBilCapabilities();
    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFBilFile::HRFBilFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    m_IsOpen = false;

    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;
    HFCPtr<HFCURL>                      Url;
    WString                             UrlString(GetURL()->GetURL());

    m_geoRefInfo.m_A00 = 0.0;
    m_geoRefInfo.m_A01 = 0.0;
    m_geoRefInfo.m_A10 = 0.0;
    m_geoRefInfo.m_A11 = 0.0;
    m_geoRefInfo.m_Tx  = 0.0;
    m_geoRefInfo.m_Ty  = 0.0;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    else
        {
        Open();

        pResolution =  new HRFResolutionDescriptor(
            GetAccessMode(),
            GetCapabilities(),                              // Capabilities,
            1.0,                                            // XResolutionRatio,
            1.0,                                            // YResolutionRatio,
            CreatePixelTypeFromFile(),                      // PixelType,
            new HCDCodecIdentity(),                         // Codec,
            HRFBlockAccess::RANDOM,                         // RBlockAccess,
            HRFBlockAccess::RANDOM,                         // WBlockAccess,
            GetScanLineOrientationFromFile(),               // ScanLineOrientation,
            HRFInterleaveType::PIXEL,                       // InterleaveType
            false,                                          // IsInterlace,
            m_bilFileHeader.width,                          // Width,
            m_bilFileHeader.height,                         // Height,
            m_bilFileHeader.width,                          // BlockWidth,
            1,                                              // BlockHeight,
            0,                                              // BlocksDataFlag
            HRFBlockType::LINE);

        HPMAttributeSet TagList;

        vector<double> MinSampleValues;
        vector<double> MaxSampleValues;
        if(m_bilFileHeader.numbands == 1 && m_bilFileInfo.HaveRedStats)
            {
            MinSampleValues.push_back(m_bilFileInfo.RedMinValue);
            MaxSampleValues.push_back(m_bilFileInfo.RedMaxValue);
            }
        else if(m_bilFileHeader.numbands == 3 &&
                m_bilFileInfo.HaveRedStats && m_bilFileInfo.HaveGreenStats && m_bilFileInfo.HaveBlueStats)
            {
            MinSampleValues.push_back(m_bilFileInfo.RedMinValue);
            MaxSampleValues.push_back(m_bilFileInfo.RedMaxValue);
            MinSampleValues.push_back(m_bilFileInfo.GreenMinValue);
            MaxSampleValues.push_back(m_bilFileInfo.GreenMaxValue);
            MinSampleValues.push_back(m_bilFileInfo.BlueMinValue);
            MaxSampleValues.push_back(m_bilFileInfo.BlueMaxValue);
            }

        if(!MinSampleValues.empty())
            TagList.Set(new HRFAttributeMinSampleValue(MinSampleValues));

        if(!MaxSampleValues.empty())
            TagList.Set(new HRFAttributeMaxSampleValue(MaxSampleValues));

        if  (IsValidGeoRefInfo())
            {
            pPage = new HRFPageDescriptor (GetAccessMode(),         // AccessMode
                                           GetCapabilities(),       // Capabilities,
                                           pResolution,             // ResolutionDescriptor,
                                           0,                       // RepresentativePalette,
                                           0,                       // Histogram,
                                           0,                       // Thumbnail,
                                           0,                       // ClipShape,
                                           BuildTransfoModel(),     // TransfoModel,
                                           0,                       // Filters
                                           &TagList);               // Taglist
            }
        else
            {
            pPage = new HRFPageDescriptor (GetAccessMode(),         // AccessMode
                                           GetCapabilities(),       // Capabilities,
                                           pResolution,             // ResolutionDescriptor,
                                           0,                       // RepresentativePalette,
                                           0,                       // Histogram,
                                           0,                       // Thumbnail,
                                           0,                       // ClipShape,
                                           0,                       // TransfoModel,
                                           0,                       // Filters
                                           &TagList);               // Taglist
            }

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFBilFile::HRFBilFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFBilFile::~HRFBilFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        try
            {
            if ( ( GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess ) && IsValidGeoRefInfo() && GetTransfoModel() )
                {
                // Only ".hdr" files of already existing BIL images can contain
                // georeference information and need to be modified.
                if((m_pHdrFile != 0) && (GetAccessMode().m_HasReadAccess))
                    ModifyHdrHeader();
                }

            m_IsOpen = false;
            m_pBilFile = 0;
            m_pHdrFile = 0;
            m_pBndFile = 0;
            m_pStxFile = 0;
            }
        catch(...)
            {
            // Simply stop exceptions in the destructor
            // We want to known if a exception is throw.
            HASSERT(0);
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFBilFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFBilFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFBilLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFBilFile::Save()
    {
    HASSERT(!"HRFBilFile::Save():Bil format is read only");

    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFBilFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pBilFile);
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFBilFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    char Header[256];
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    // Set the basic information for the hdr header and the file info.
    m_bilFileHeader.hdrlength = 0;
    m_bilFileHeader.width = (uint32_t)pResolutionDescriptor->GetWidth();
    m_bilFileHeader.height = (uint32_t)pResolutionDescriptor->GetHeight();
    m_bilFileHeader.comptype = RPIX_COMPRESSION_NONE;
    m_bilFileHeader.pixelorder = RPIX_PIXEL_ORDER_NORMAL;
    m_bilFileHeader.scnlorder = RPIX_SCANLINE_ORDER_NORMAL;
    m_bilFileHeader.interleave = RPIX_INTERLEAVING_BIL;
    m_bilFileInfo.NbBandGapBytes = 0;
    m_bilFileInfo.HaveRedStats = m_bilFileInfo.HaveGreenStats = m_bilFileInfo.HaveBlueStats = false;

    // Set the information concerning the pixel type: the number of bands,
    // the default band for each channel, the number of bits per pixel per channel
    // and the number of bytes per row.
    if (pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() == 24)
        {
        m_bilFileHeader.numbands = 3;
        m_bilFileHeader.rchannel = 1;
        m_bilFileHeader.gchannel = 2;
        m_bilFileHeader.bchannel = 3;

        m_bilFileInfo.NbBitsPerBandPerPixel = 8;
        m_bilFileInfo.NbBytesPerBand = m_bilFileHeader.width;
        m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;
        }
    else if (pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() == 48)
        {
        m_bilFileHeader.numbands = 3;
        m_bilFileHeader.rchannel = 1;
        m_bilFileHeader.gchannel = 2;
        m_bilFileHeader.bchannel = 3;

        m_bilFileInfo.NbBitsPerBandPerPixel = 16;
        m_bilFileInfo.NbBytesPerBand = m_bilFileHeader.width * 2;
        m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;
        }
    else if (pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() == 8)
        {
        m_bilFileHeader.numbands = 1;
        m_bilFileHeader.rchannel = 1;
        m_bilFileHeader.gchannel = RPIX_CHANNEL_UNDEFINED;
        m_bilFileHeader.bchannel = RPIX_CHANNEL_UNDEFINED;

        m_bilFileInfo.NbBitsPerBandPerPixel = 8;
        m_bilFileInfo.NbBytesPerBand = m_bilFileHeader.width;
        m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;
        }
    else
        {
        m_bilFileHeader.numbands = 1;
        m_bilFileHeader.rchannel = 1;
        m_bilFileHeader.gchannel = RPIX_CHANNEL_UNDEFINED;
        m_bilFileHeader.bchannel = RPIX_CHANNEL_UNDEFINED;

        m_bilFileInfo.NbBitsPerBandPerPixel = 16;
        m_bilFileInfo.NbBytesPerBand = m_bilFileHeader.width * 2;
        m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;
        }

    // Compute the standard deviation to bring back the color adjustment factor to 1.
    m_bilFileInfo.RedStdDeviation   =
        m_bilFileInfo.GreenStdDeviation =
            m_bilFileInfo.BlueStdDeviation  = (pow(2.0, (int)m_bilFileInfo.NbBitsPerBandPerPixel) - 1) / 3;

    // Create the ".hdr" file.
    sprintf(Header, "BYTEORDER     M\r\nLAYOUT        BIL\r\nNROWS         %i\r\nNCOLS         %i\r\nNBANDS        %i\r\nNBITS         %i\r\nBANDROWBYTES  %i\r\nTOTALROWBYTES %i\r\nBANDGAPBYTES  0",
            m_bilFileHeader.height,
            m_bilFileHeader.width,
            m_bilFileHeader.numbands,
            m_bilFileInfo.NbBitsPerBandPerPixel,
            m_bilFileInfo.NbBytesPerBand,
            m_bilFileInfo.NbBytesPerRow);

    // Lock the header file.
    HFCLockMonitor HdrFileLock(GetHdrLockManager());

    m_pHdrFile->Write(Header, strlen((char*)Header));

    HdrSharingControlIncrementCount();

    // Unlock the header file.
    HdrFileLock.ReleaseKey();

    // Create the ".bnd" file.
    sprintf(Header, "RED          %i\r\nGREEN        %i\r\nBLUE         %i",
            m_bilFileHeader.rchannel,
            m_bilFileHeader.gchannel,
            m_bilFileHeader.bchannel);

    // Lock the band file.
    HFCLockMonitor BndFileLock(GetBndLockManager());

    m_pBndFile->Write(Header, strlen((char*)Header));
    BndSharingControlIncrementCount();

    // Unlock the band file.
    BndFileLock.ReleaseKey();

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFBilFile::GetCapabilities () const
    {
    return (HRFBilCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFBilFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;

    if (RPIX_CHANNEL_UNDEFINED != m_bilFileHeader.rchannel)
        {
        if ((1 <= m_bilFileHeader.numbands) &&
            (RPIX_CHANNEL_UNDEFINED == m_bilFileHeader.gchannel) &&
            (RPIX_CHANNEL_UNDEFINED == m_bilFileHeader.bchannel))
            {
            if (8 == m_bilFileInfo.NbBitsPerBandPerPixel)
                pPixelType = new HRPPixelTypeV8Gray8();
            else if (16 == m_bilFileInfo.NbBitsPerBandPerPixel)
                {
                pPixelType = new HRPPixelTypeV16Gray16();
                }
            }
        else
            {
            if ((3 <= m_bilFileHeader.numbands) &&
                (RPIX_CHANNEL_UNDEFINED != m_bilFileHeader.gchannel) &&
                (RPIX_CHANNEL_UNDEFINED != m_bilFileHeader.bchannel) &&
                (m_bilFileHeader.rchannel != m_bilFileHeader.gchannel) &&
                (m_bilFileHeader.rchannel != m_bilFileHeader.bchannel) &&
                (m_bilFileHeader.bchannel != m_bilFileHeader.gchannel))
                {
                if (8 == m_bilFileInfo.NbBitsPerBandPerPixel)
                    pPixelType = new HRPPixelTypeV24R8G8B8();
                else if (16 == m_bilFileInfo.NbBitsPerBandPerPixel)
                    {
                    //not supported
                    //pPixelType = new HRPPixelTypeV48R16G16B16();
                    }
                }
            }
        }

    if (pPixelType == 0)
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// GetScanLineOrientationFromFile
// Private
// Find the scanline orientation from file
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFBilFile::GetScanLineOrientationFromFile() const
    {
    HRFScanlineOrientation orientation;

    if(RPIX_SCANLINE_ORDER_NORMAL == m_bilFileHeader.scnlorder)
        {
        if(RPIX_PIXEL_ORDER_NORMAL == m_bilFileHeader.pixelorder)
            {
            orientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            }
        else if (RPIX_PIXEL_ORDER_REVERSE == m_bilFileHeader.pixelorder)
            {
            orientation = HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL;
            }
        }
    else if(RPIX_SCANLINE_ORDER_INVERSE == m_bilFileHeader.scnlorder)
        {
        if(RPIX_PIXEL_ORDER_NORMAL == m_bilFileHeader.pixelorder)
            {
            orientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
            }
        else if (RPIX_PIXEL_ORDER_REVERSE == m_bilFileHeader.pixelorder)
            {
            orientation = HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL;
            }
        }
    return orientation;
    }


//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFBilFile::Open()
    {
    HFCPtr<HFCURL>              BILUrl;
    WString                     UrlString(GetURL()->GetURL());

    // Open the file
    if (!m_IsOpen)
        {
        // Get the Url for the ".bil" file, because we might be opening from a header file
        UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"bil"), L"bil");
        BILUrl = HFCURL::Instanciate(UrlString);

        // Open the actual bil file.
        m_pBilFile = HFCBinStream::Instanciate(BILUrl, m_Offset, GetPhysicalAccessMode() , 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Initialisation of file struct.
        HFCLockMonitor SisterFileLock(GetLockManager());

        if(!ReadHdrHeader())
            {
            m_pBilFile->Read(&m_bilFileHeader, sizeof m_bilFileHeader);
            m_bilFileInfo.NbBitsPerBandPerPixel = 8;
            m_bilFileInfo.NbBytesPerBand = m_bilFileHeader.width;
            m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;
            m_bilFileInfo.NbBandGapBytes = 0;
            m_bilFileHeader.hdrlength += 4;
            }

        GetBandsStatsFromFile();

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return true;
    }



//-----------------------------------------------------------------------------
// ReadHdrHeader
// Protected
// Gets the header information from the ".hdr" file. Returns false if there
// is no such file.
//-----------------------------------------------------------------------------
bool HRFBilFile::ReadHdrHeader()
    {
    HFCPtr<HFCURL>              HdrUrl;
    WString                     UrlString(GetURL()->GetURL());
    string                      Header;
    string                      HeaderLine;

    // Get the Url for the ".hdr" file.
    UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"hdr"), L"hdr");
    HdrUrl = HFCURL::Instanciate(UrlString);

    // Open the actual hdr file.
    m_pHdrFile = HFCBinStream::Instanciate(HdrUrl, m_Offset, GetPhysicalAccessMode());

    if ((m_pHdrFile == 0) || (m_pHdrFile->GetLastException() != 0))
        return false;

    // This creates the sister file for file sharing control if necessary.
    HdrSharingControlCreate(HdrUrl);

    // Initialisation of file struct.
    HFCLockMonitor SisterFileLock(GetHdrLockManager());

    m_pHdrFile->SeekToBegin();
    while(!m_pHdrFile->EndOfFile())
        {
        ReadLine(&HeaderLine, m_pHdrFile, false, true);
        Header += HeaderLine;
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    // Get the information
    m_bilFileHeader.comptype = RPIX_COMPRESSION_NONE;
    m_bilFileHeader.majorversion = RPIX_MAJOR_VERSION;
    m_bilFileHeader.minorversion = RPIX_MINOR_VERSION;
    m_bilFileHeader.interleave = RPIX_INTERLEAVING_BIL;
    m_bilFileHeader.pixelorder = RPIX_PIXEL_ORDER_NORMAL;
    m_bilFileHeader.scnlorder = RPIX_SCANLINE_ORDER_NORMAL;
    m_bilFileHeader.hdrlength = GetLongValueFromHeader(Header.c_str(), "SKIPBYTES");
    m_bilFileHeader.width = GetLongValueFromHeader(Header.c_str(), "NCOLS");
    m_bilFileHeader.height = GetLongValueFromHeader(Header.c_str(), "NROWS");
    m_bilFileHeader.numbands = (Byte)GetLongValueFromHeader(Header.c_str(), "NBANDS");
    m_bilFileInfo.NbBitsPerBandPerPixel = GetLongValueFromHeader(Header.c_str(), "NBITS");
    m_bilFileInfo.NbBytesPerBand = GetLongValueFromHeader(Header.c_str(), "BANDROWBYTES");
    m_bilFileInfo.NbBytesPerRow = GetLongValueFromHeader(Header.c_str(), "TOTALROWBYTES");
    m_bilFileInfo.NbBandGapBytes = GetLongValueFromHeader(Header.c_str(), "BANDGAPBYTES");
    m_bilFileInfo.IsMsByteFirst = FileByteOrderMostSignificant(Header.c_str());

    if (0 == m_bilFileHeader.width)
        throw HRFChildFileParameterException(GetURL()->GetURL(),
                                             m_pHdrFile->GetURL()->GetURL(),
                                             L"NCOLS");

    if (0 == m_bilFileHeader.height)
        throw HRFChildFileParameterException(GetURL()->GetURL(),
                                             m_pHdrFile->GetURL()->GetURL(),
                                             L"NROWS");

    if (0 == m_bilFileHeader.numbands)
        m_bilFileHeader.numbands = 1;

    if (0 == m_bilFileInfo.NbBitsPerBandPerPixel)
        m_bilFileInfo.NbBitsPerBandPerPixel = 8;

    if (0 == m_bilFileInfo.NbBytesPerBand)
        m_bilFileInfo.NbBytesPerBand = m_bilFileInfo.NbBitsPerBandPerPixel * m_bilFileHeader.width / 8;

    if (0 == m_bilFileInfo.NbBytesPerRow)
        m_bilFileInfo.NbBytesPerRow = m_bilFileInfo.NbBytesPerBand * m_bilFileHeader.numbands;

    GetBandsFromFile();
    GetGeoRefInfo(Header.c_str());

    return true;
    }

//-----------------------------------------------------------------------------
// ReadHdrHeader
// Protected
// Gets the header information from the ".hdr" file. Returns false if there
// is no such file.
//-----------------------------------------------------------------------------
void HRFBilFile::ModifyHdrHeader()
    {
    string                      Header;
    string                      HeaderLine;

    // Initialisation of file struct.
    HFCLockMonitor SisterFileLock(GetHdrLockManager());

    if (HdrSharingControlNeedSynchronization())
        HdrSharingControlSynchronize();

    m_pHdrFile->SeekToBegin();

    // For each line of the header, check for keywords related the georeference
    // information. If you find one, replace its value with the new one.
    while(!m_pHdrFile->EndOfFile())
        {
        ReadLine(&HeaderLine, m_pHdrFile, false, false);
        SetHeaderDoubleValue(&HeaderLine, "XULCORNER", m_geoRefInfo.m_Tx);
        SetHeaderDoubleValue(&HeaderLine, "XLLCORNER", m_geoRefInfo.m_Tx);
        SetHeaderDoubleValue(&HeaderLine, "ULXMAP", m_geoRefInfo.m_Tx);
        SetHeaderDoubleValue(&HeaderLine, "YULCORNER", m_geoRefInfo.m_Ty);
        SetHeaderDoubleValue(&HeaderLine, "YLLCORNER", m_geoRefInfo.m_Ty + (m_geoRefInfo.m_A11 * (m_bilFileHeader.height - 1)));
        SetHeaderDoubleValue(&HeaderLine, "ULYMAP", m_geoRefInfo.m_Ty);
        SetHeaderDoubleValue(&HeaderLine, "XDIM", m_geoRefInfo.m_A00);
        SetHeaderDoubleValue(&HeaderLine, "YDIM", -m_geoRefInfo.m_A11);
        SetHeaderCellsizeValue(&HeaderLine);
        Header += HeaderLine;
        }

    m_pHdrFile->Write(Header.c_str(), Header.length());

    // If the old header file was longer then the new one, erase the end of the file.
    if (m_pHdrFile->GetSize() > Header.length())
        {
        uint32_t NewSize = (uint32_t)(m_pHdrFile->GetSize() - Header.length());
        char    aBuffer[256];
        memset(aBuffer, ' ', 256);

        // clear the end of the file
        while (NewSize > 256)
            {
            m_pHdrFile->Write(aBuffer, 256);
            NewSize -= 256;
            }

        if (NewSize > 0)
            m_pHdrFile->Write(aBuffer, NewSize);
        }

    HdrSharingControlIncrementCount();

    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// Protected
// GetGeoRefInfo
//-----------------------------------------------------------------------------
void HRFBilFile::GetGeoRefInfo
(
    const char* pi_pHdrHeader
)
    {
    HPRECONDITION(NULL != pi_pHdrHeader);

    const char*   YllCorner;
    bool   IsYllCorner = false;

    // Try reading the georeference information from the hdr header file.
    m_geoRefInfo.m_A00 = GetDoubleValueFromHeader(pi_pHdrHeader, "CELLSIZE");
    if (0 == m_geoRefInfo.m_A00)
        {
        m_geoRefInfo.m_A00 = GetDoubleValueFromHeader(pi_pHdrHeader, "XDIM");
        m_geoRefInfo.m_A11 = GetDoubleValueFromHeader(pi_pHdrHeader, "YDIM");
        }
    else
        m_geoRefInfo.m_A11 = -m_geoRefInfo.m_A00;

    m_geoRefInfo.m_A10 = m_geoRefInfo.m_A01 = 0.0;

    m_geoRefInfo.m_Tx = GetDoubleValueFromHeader(pi_pHdrHeader, "XULCORNER");
    if (0 == m_geoRefInfo.m_Tx)
        {
        m_geoRefInfo.m_Tx = GetDoubleValueFromHeader(pi_pHdrHeader, "ULXMAP");
        if (0 == m_geoRefInfo.m_Tx)
            m_geoRefInfo.m_Tx = GetDoubleValueFromHeader(pi_pHdrHeader, "XLLCORNER");
        }

    m_geoRefInfo.m_Ty = GetDoubleValueFromHeader(pi_pHdrHeader, "YULCORNER");
    if (0 == m_geoRefInfo.m_Ty)
        {
        m_geoRefInfo.m_Ty = GetDoubleValueFromHeader(pi_pHdrHeader, "ULYMAP");
        if (0 == m_geoRefInfo.m_Ty)
            {
            m_geoRefInfo.m_Ty = GetDoubleValueFromHeader(pi_pHdrHeader, "YLLCORNER");
            YllCorner = strstr(pi_pHdrHeader, "YLLCORNER");
            IsYllCorner = (NULL != YllCorner) && !IsValidChar(*(YllCorner + 9));
            }
        }
    if (IsYllCorner)
        m_geoRefInfo.m_Ty -= m_geoRefInfo.m_A11 * (m_bilFileHeader.height - 1);
    else
        {
        m_geoRefInfo.m_Ty *= -1;
        }
    }

//-----------------------------------------------------------------------------
// GetBandsFromFile
// Protected
// Gets the band number to use for the R, G and B channels from a ".bnd" file.
//-----------------------------------------------------------------------------
void HRFBilFile::GetBandsFromFile()
    {
    HFCPtr<HFCURL>              BndUrl;
    WString                     UrlString(GetURL()->GetURL());
    char*                       BandFileContent;
    uint32_t                    BandFileLength;

    // Get the Url for the ".bnd" file.
    UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"bnd"), L"bnd");
    BndUrl = HFCURL::Instanciate(UrlString);

    bool FileNotFound = false;
    HFCStat BNDFileFileStat(BndUrl);
    if (BNDFileFileStat.IsExistent())
        {
        // Open the actual bnd file.
        try
            {
            m_pBndFile = HFCBinStream::Instanciate(BndUrl, GetPhysicalAccessMode(), 0, true);


            // This creates the sister file for file sharing control if necessary.
            BndSharingControlCreate(BndUrl);

            // Initialisation of file struct.
            HFCLockMonitor SisterFileLock(GetBndLockManager());

            m_pBndFile->SeekToEnd();
            BandFileLength = (uint32_t)m_pBndFile->GetCurrentPos();
            BandFileContent = new char[BandFileLength + 1];
            BandFileContent[BandFileLength] = '\0';
            m_pBndFile->SeekToBegin();
            m_pBndFile->Read(BandFileContent, BandFileLength);

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            BndUrl = 0;

            m_bilFileHeader.rchannel = (Byte)GetLongValueFromHeader(BandFileContent, "RED");
            m_bilFileHeader.gchannel = (Byte)GetLongValueFromHeader(BandFileContent, "GREEN");
            m_bilFileHeader.bchannel = (Byte)GetLongValueFromHeader(BandFileContent, "BLUE");
            delete [] BandFileContent;
            }
        catch(HFCFileNotFoundException&)
            {
            FileNotFound = true;
            }
        }
    else
        FileNotFound = true;

    if (FileNotFound)
        {
        if (3 <= m_bilFileHeader.numbands)
            {
            m_bilFileHeader.rchannel = 1;
            m_bilFileHeader.gchannel = 2;
            m_bilFileHeader.bchannel = 3;
            }
        else
            {
            m_bilFileHeader.rchannel = 1;
            m_bilFileHeader.gchannel = 0;
            m_bilFileHeader.bchannel = 0;
            }
        }
    }

//-----------------------------------------------------------------------------
// GetBandsStatsFromFile
// Private
// Gets the statistics for a specified band from the ".stx" sister file.
//-----------------------------------------------------------------------------
void HRFBilFile::GetBandsStatsFromFile()
    {
    HFCPtr<HFCURL>              StxUrl;
    WString                     UrlString(GetURL()->GetURL());

    m_bilFileInfo.HaveRedStats = m_bilFileInfo.HaveGreenStats = m_bilFileInfo.HaveBlueStats = false;

    // Get the Url for the ".stx" file.
    UrlString.replace(UrlString.rfind(L'.') + 1, wcslen(L"stx"), L"stx");
    StxUrl = HFCURL::Instanciate(UrlString);

    HFCStat STXFileFileStat(StxUrl);
    if (STXFileFileStat.IsExistent())
        {
        // Open the actual bil file.
        try
            {
            HArrayAutoPtr<char>         Stats;
            uint32_t                    StatsLength;

            m_pStxFile = HFCBinStream::Instanciate(StxUrl, GetPhysicalAccessMode(), 0, true);

            // This creates the sister file for file sharing control if necessary.
            StxSharingControlCreate(StxUrl);

            // Initialisation of file struct.
            HFCLockMonitor SisterFileLock(GetStxLockManager());

            m_pStxFile->SeekToEnd();
            StatsLength = (uint32_t)m_pStxFile->GetCurrentPos();
            Stats = new char[StatsLength + 1];
            Stats[StatsLength] = '\0';
            m_pStxFile->SeekToBegin();
            m_pStxFile->Read(Stats, StatsLength);

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            m_bilFileInfo.HaveRedStats = GetBandStdDeviation(Stats,
                                                             m_bilFileHeader.rchannel,
                                                             &m_bilFileInfo.RedMinValue,
                                                             &m_bilFileInfo.RedMaxValue,
                                                             &m_bilFileInfo.RedMeanValue,
                                                             &m_bilFileInfo.RedStdDeviation);

            m_bilFileInfo.HaveGreenStats = GetBandStdDeviation((char*)Stats,
                                                               m_bilFileHeader.gchannel,
                                                               &m_bilFileInfo.GreenMinValue,
                                                               &m_bilFileInfo.GreenMaxValue,
                                                               &m_bilFileInfo.GreenMeanValue,
                                                               &m_bilFileInfo.GreenStdDeviation);

            m_bilFileInfo.HaveBlueStats = GetBandStdDeviation((char*)Stats,
                                                              m_bilFileHeader.bchannel,
                                                              &m_bilFileInfo.BlueMinValue,
                                                              &m_bilFileInfo.BlueMaxValue,
                                                              &m_bilFileInfo.BlueMeanValue,
                                                              &m_bilFileInfo.BlueStdDeviation);
            }
        catch(HFCFileNotFoundException&) 
        { 
        }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// IsValidGeoRefInfo
// Multiple returns for simplicity
//-----------------------------------------------------------------------------
bool HRFBilFile::IsValidGeoRefInfo() const
    {
    if((HDOUBLE_EQUAL(m_geoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_geoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_geoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_geoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_geoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_geoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_geoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_geoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR)))
        return false;

    // Origin must be greater that -2e294
    if (m_geoRefInfo.m_Tx < -HMAX_EPSILON || m_geoRefInfo.m_Ty < -HMAX_EPSILON)
        return false;

    // Origin must be smaller than 2e294
    if (m_geoRefInfo.m_Tx >= HMAX_EPSILON || m_geoRefInfo.m_Ty >= HMAX_EPSILON)
        return false;

    // Limit the pixelsize (?) inside [-2e294, 2e294]
    if (m_geoRefInfo.m_A00 > HMAX_EPSILON || m_geoRefInfo.m_A00 < -HMAX_EPSILON ||
        m_geoRefInfo.m_A11 > HMAX_EPSILON || m_geoRefInfo.m_A11 < -HMAX_EPSILON)
        return false;

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// Get the current transformation model's parameters.
//-----------------------------------------------------------------------------
bool HRFBilFile::GetTransfoModel()
    {
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HGF2DTransfoModel> pModel = pPageDescriptor->GetTransfoModel();
    bool HasChanged = false;

    // Check if the transformation can be represented by a matrix.
    if (pModel->CanBeRepresentedByAMatrix())
        {
        double FactorModelToMeter = GetDefaultRatioToMeter();

        // Apply inverse factor to Matrix
        if (FactorModelToMeter != 1.0)
            {
            HASSERT(FactorModelToMeter != 0);

            HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
            pScaleModel->SetXScaling(1/FactorModelToMeter);
            pScaleModel->SetYScaling(1/FactorModelToMeter);

            pModel = pModel->ComposeInverseWithDirectOf(*pScaleModel);
            }

        // Create a translation to remove the 0.5 x 0.5 pixel centering.

        // Extract the matrix parameters from the transformation.
        HFCMatrix<3, 3> Matrix = pModel->GetMatrix();
        Matrix[1][2] += 0.5*(Matrix[0][1] + Matrix[1][1]); // remove the 0.5 x 0.5 pixel centering.
        Matrix[0][2] += 0.5*(Matrix[0][0] + Matrix[1][0]); // remove the 0.5 x 0.5 pixel centering.

        HasChanged =    (m_geoRefInfo.m_A00 == Matrix[0][0]) ||
                        (m_geoRefInfo.m_A01 == Matrix[0][1]) ||
                        (m_geoRefInfo.m_A10 == Matrix[1][0]) ||
                        (m_geoRefInfo.m_A11 == Matrix[1][1]) ||
                        (m_geoRefInfo.m_Tx  == Matrix[0][2]) ||
                        (m_geoRefInfo.m_Ty  == Matrix[1][2]);

        m_geoRefInfo.m_A00 = Matrix[0][0];
        m_geoRefInfo.m_A01 = Matrix[0][1];
        m_geoRefInfo.m_A10 = Matrix[1][0];
        m_geoRefInfo.m_A11 = Matrix[1][1];
        m_geoRefInfo.m_Tx  = Matrix[0][2];
        m_geoRefInfo.m_Ty  = Matrix[1][2];
        }
    else
        {
        throw HRFTransfoCannotBeAMatrixException(GetURL()->GetURL());
        }

    return HasChanged;
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
// The translation for BIL files are defined from the center of the first pixel
// in the upper left corner.
//
//  x' = Ax + By + C
//  y' = Dx + Ey + F
//
//  where x'  = calculated x-coordinate of the pixel on the map
//        y'  = calculated y-coordinate of the pixel on the map
//        x   = column number of the pixel in the image
//        y   = row number of the pixel in the image
//        A   = x-scale
//        B,D = rotation term
//        C   = translation term; x-origin (x-coordinate of the center of the
//              upper left corner)
//        E   = y-scale
//        F   = translation term; y-origin (y-coordinate of the center of the
//              upper left corner)
//
// Change axes x,y by i,j
// where i = x - 0.5
//       j = y - 0.5
//
// Now, use the i and j axes instead of x and y
// x' = Ai + Bj + C  => x' = A(x - 0.5) + B(y - 0.5) + C  => x' = Ax + By + C - 0.5(A + B)
// y' = Di + Ej + F  => y' = D(x - 0.5) + E(y - 0.5) + F  => y' = Dx + Ey + F - 0.5(D + E)
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFBilFile::BuildTransfoModel() const
    {
    // Build the transformation model.
    HFCPtr<HGF2DAffine> pModel;
    HFCPtr<HGF2DTransfoModel> pFinalTransfo;

    double FactorModelToMeter = GetDefaultRatioToMeter();

    pModel = new HGF2DAffine();
    // TWF are centered at the center of first pixel.
    ((HFCPtr<HGF2DAffine>&)pModel)->SetByMatrixParameters(m_geoRefInfo.m_Tx - 0.5*(m_geoRefInfo.m_A00 + m_geoRefInfo.m_A10),
                                                          m_geoRefInfo.m_A00,
                                                          m_geoRefInfo.m_A01,
                                                          m_geoRefInfo.m_Ty - 0.5*(m_geoRefInfo.m_A01 + m_geoRefInfo.m_A11),
                                                          m_geoRefInfo.m_A10,
                                                          m_geoRefInfo.m_A11);

    // If we are in creationMode, create a new file, we don't apply the Factor, it will be
    // apply only at save time. Because if we apply the Factor here, and remove at the save time
    // the factor will be 1.0
    if ((FactorModelToMeter != 1.0) && !GetAccessMode().m_HasCreateAccess)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(FactorModelToMeter);
        pScaleModel->SetYScaling(FactorModelToMeter);

        pFinalTransfo = pModel->ComposeInverseWithDirectOf(*pScaleModel);
        }
    else
        pFinalTransfo = pModel;

    HRFScanlineOrientation SLO = GetScanLineOrientationFromFile();
    if (SLO == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL ||
        SLO == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
        {
        HFCPtr<HGF2DStretch> pSLOConverterModel = new HGF2DStretch();
        pSLOConverterModel->SetYScaling(-1.0);

        pSLOConverterModel->SetTranslation(HGF2DDisplacement(0.0, m_bilFileHeader.height));
        pFinalTransfo = ((HFCPtr<HGF2DTransfoModel>&)pSLOConverterModel)->ComposeInverseWithDirectOf(*pFinalTransfo);
        }

    return pFinalTransfo;
    }

//-----------------------------------------------------------------------------
// Protected
// ReadLine
//-----------------------------------------------------------------------------
void HRFBilFile::ReadLine
(
    string*         po_pString,
    HFCBinStream*   pi_pFile,
    bool           pi_CleanUpLine,
    bool           pi_RemoveHdrComments
)
    {
    HPRECONDITION(pi_pFile != 0);
    HPRECONDITION(po_pString != 0);

    const int BufferSize = 64;
    char      Buffer[BufferSize];
    char*     Comment;
    string    CurrentLine;

    bool EndOfLine = false;
    po_pString->erase();
    while (!EndOfLine)
        {
        memset(Buffer, 0, BufferSize);
        for (unsigned short i = 0; i < BufferSize && !EndOfLine; i++)
            {
            pi_pFile->Read(&Buffer[i], 1);
            EndOfLine = Buffer[i] == '\n' || pi_pFile->EndOfFile();
            }

        Comment = strstr(Buffer, "/*");
        if (NULL != Comment)
            *Comment = '\0';

        *po_pString += BeStringUtilities::Strupr(Buffer);

        if (!pi_RemoveHdrComments && (NULL != Comment))
            {
            *Comment = '/';
            *po_pString += Comment;
            }
        }
    if (pi_CleanUpLine)
        CleanUpString(po_pString);
    }

//-----------------------------------------------------------------------------
// FileByteOrderMostSignificant
// Private
// Determines if the byte order is most or least significant first.
//-----------------------------------------------------------------------------
bool HRFBilFile::FileByteOrderMostSignificant
(
    const char* pi_Header
)
    {
    HPRECONDITION(NULL != pi_Header);

    bool IsMostFirst = false;
    const char* ValueStartPos;

    ValueStartPos = strstr(pi_Header, "BYTEORDER");
    if (NULL != ValueStartPos)
        {
        ValueStartPos += 10;

        if (!IsValidChar(*ValueStartPos))
            {
            while (!IsValidChar(*ValueStartPos) && ('\0' != *ValueStartPos))
                ValueStartPos ++;

            switch(*ValueStartPos)
                {
                case 'i':
                case 'I':
                case 'l':
                case 'L':
                    IsMostFirst = false;
                    ValueStartPos ++;
                    break;
                case 'm':
                case 'M':
                    IsMostFirst = true;
                    ValueStartPos ++;
                    break;
                default:
                    throw HFCCorruptedFileException(GetURL()->GetURL());
                }

            if (strncmp(ValueStartPos, "sbfirst", 7) == 0)
                ValueStartPos += 7;

            if ((*ValueStartPos != ' ')   &&
                (*ValueStartPos != '\0')  &&
                (*ValueStartPos != 10)    &&
                (*ValueStartPos != 13))
                {
                throw HFCCorruptedFileException(GetURL()->GetURL());
                }
            }
        }
    return IsMostFirst;
    }

//-----------------------------------------------------------------------------
// GetValueFromHeader
// Protected
// Gets the value associated with a keyword in the header.
//-----------------------------------------------------------------------------
int32_t HRFBilFile::GetLongValueFromHeader
(
    const char* pi_Header,
    const char* pi_Keyword
)
    {
    HPRECONDITION(NULL != pi_Header);
    HPRECONDITION(NULL != pi_Keyword);

    const char*   ValueStartPos;
    int32_t  Result = 0;

    ValueStartPos = strstr(pi_Header, pi_Keyword);
    if (0 != ValueStartPos)
        {
        ValueStartPos += strlen(pi_Keyword);
        if (!IsValidChar(*ValueStartPos))
            {
            ValueStartPos = strpbrk(ValueStartPos, "-0123456789");
            if (0 != ValueStartPos)
                {
                if (!ConvertStringToLong(ValueStartPos, &Result))
                    throw HFCCorruptedFileException(GetURL()->GetURL());
                }
            }
        }
    return Result;
    }

//-----------------------------------------------------------------------------
// GetValueFromHeader
// Protected
// Gets the value associated with a keyword in the header.
//-----------------------------------------------------------------------------
double HRFBilFile::GetDoubleValueFromHeader
(
    const char* pi_Header,
    const char* pi_Keyword
)
    {
    HPRECONDITION(NULL != pi_Header);
    HPRECONDITION(NULL != pi_Keyword);

    const char*   ValueStartPos;
    double Result = 0.0;

    ValueStartPos = strstr(pi_Header, pi_Keyword);
    if (NULL != ValueStartPos)
        {
        ValueStartPos += strlen(pi_Keyword);
        if (!IsValidChar(*ValueStartPos))
            {
            ValueStartPos = strpbrk(ValueStartPos, "-0123456789.");
            if (NULL != ValueStartPos)
                {
                if (!ConvertStringToDouble(ValueStartPos, &Result))
                    throw HFCCorruptedFileException(GetURL()->GetURL());
                }
            }
        }
    return Result;
    }

//-----------------------------------------------------------------------------
// SetHeaderDoubleValue
// Protected
// Sets the value associated with a keyword in the header.
//-----------------------------------------------------------------------------
void HRFBilFile::SetHeaderDoubleValue
(
    string*     pi_HeaderLine,
    const char* pi_Keyword,
    double     pi_Value
)
    {
    HPRECONDITION(NULL != pi_HeaderLine);
    HPRECONDITION(NULL != pi_Keyword);

    char*   ValueEndPos;
    size_t  ValueStartPos;
    char    Value[64];

    sprintf(Value, "%lf", pi_Value);

    ValueStartPos = pi_HeaderLine->find(pi_Keyword);
    if (pi_HeaderLine->npos != ValueStartPos)
        {
        ValueStartPos += strlen(pi_Keyword);
        if (!IsValidChar(pi_HeaderLine->at(ValueStartPos)))
            {
            ValueStartPos = pi_HeaderLine->find_first_of("-0123456789.", ValueStartPos);
            if (pi_HeaderLine->npos != ValueStartPos)
                {
                strtod(&pi_HeaderLine->c_str()[ValueStartPos], &ValueEndPos);
                pi_HeaderLine->replace(ValueStartPos, ValueEndPos - &(pi_HeaderLine->c_str()[ValueStartPos]), Value);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// SetHeaderCellsizeValue
// Protected
// Sets the value associated with a keyword in the header.
//-----------------------------------------------------------------------------
void HRFBilFile::SetHeaderCellsizeValue
(
    string*     pi_HeaderLine
)
    {
    HPRECONDITION(NULL != pi_HeaderLine);

    char*   ValueEndPos;
    size_t  ValueStartPos;
    char    Value[64];

    ValueStartPos = pi_HeaderLine->find("CELLSIZE");
    if (pi_HeaderLine->npos != ValueStartPos)
        {
        ValueStartPos += 8;
        if (!IsValidChar(pi_HeaderLine->at(ValueStartPos)))
            {
            // To have a unique cellsize the scaling in X and Y must be the same.
            if (m_geoRefInfo.m_A00 == -m_geoRefInfo.m_A11)
                {
                ValueStartPos = pi_HeaderLine->find_first_of("-0123456789.", ValueStartPos);
                if (pi_HeaderLine->npos != ValueStartPos)
                    {
                    sprintf(Value, "%lf", m_geoRefInfo.m_A00);
                    strtod(&pi_HeaderLine->c_str()[ValueStartPos], &ValueEndPos);
                    pi_HeaderLine->replace(ValueStartPos, ValueEndPos - &(pi_HeaderLine->c_str()[ValueStartPos]), Value);
                    }
                }
            // If not, replace the cellsize with xdim and ydim.
            else
                {
                pi_HeaderLine->erase();
                sprintf(Value, "XDIM %lf\r\nYDIM %lf\r\n", m_geoRefInfo.m_A00, -m_geoRefInfo.m_A11);
                *pi_HeaderLine += Value;
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// GetBandStdDeviation
// Private
// Extracts the standard deviation for a specified band.
// Here's the format of the stx file:
//
// bandNumber1 minValue1 maxValue1 meanValue1 stdDeviation1
// bandNumber2 minValue2 maxValue2 meanValue2 stdDeviation2
// ...
//
//-----------------------------------------------------------------------------
bool HRFBilFile::GetBandStdDeviation (const char* pi_pStats,
                                       int32_t    pi_BandNumber,
                                       int32_t*     po_pMinValue,
                                       int32_t*     po_pMaxValue,
                                       double*    po_pMeanValue,
                                       double*    po_pStdDeviation)
    {
    HPRECONDITION(pi_pStats        != 0);
    HPRECONDITION(po_pStdDeviation != 0);
    HPRECONDITION(po_pMinValue     != 0);
    HPRECONDITION(po_pMaxValue     != 0);
    HPRECONDITION(po_pMeanValue    != 0);

    char* subString;
    bool bFoundBandStats = false;

    // Set default values.
    *po_pMinValue     = 0;
    *po_pMaxValue     = (1 << m_bilFileInfo.NbBitsPerBandPerPixel) - 1;
    *po_pMeanValue    = ((double)(*po_pMaxValue >> 1));
    *po_pStdDeviation = ((double)(1 << m_bilFileInfo.NbBitsPerBandPerPixel)) / 3.0;

    // Try to get to the standard deviation and then extract it.
    subString = (char*)strpbrk(pi_pStats, "0123456789");
    while ((NULL != subString) && (!bFoundBandStats))
        {
        if(strtol(subString, &subString, 10) == pi_BandNumber)
            {
            bFoundBandStats = true;
            if (IsValidChar(*subString))
                break;

            subString = strpbrk(subString, "-0123456789");
            if (NULL == subString)
                break;

            if (!ConvertStringToLong(subString, po_pMinValue))
                throw HFCCorruptedFileException(GetURL()->GetURL());

            strtol(subString, &subString, 10);
            if (IsValidChar(*subString))
                break;

            subString = strpbrk(subString, "-0123456789");
            if (NULL == subString)
                break;

            if (!ConvertStringToLong(subString, po_pMaxValue))
                throw HFCCorruptedFileException(GetURL()->GetURL());


            strtod(subString, &subString);
            if (IsValidChar(*subString))
                break;

            subString = strpbrk(subString, "-0123456789.");
            if (NULL == subString)
                break;

            if(!ConvertStringToDouble(subString, po_pMeanValue))
                throw HFCCorruptedFileException(GetURL()->GetURL());

            strtod(subString, &subString);
            if (IsValidChar(*subString))
                break;

            subString = strpbrk(subString, "-0123456789.");
            if (NULL == subString)
                break;

            if(!ConvertStringToDouble(subString, po_pStdDeviation))
                throw HFCCorruptedFileException(GetURL()->GetURL());
            }
        else
            {
            subString = strchr(subString, '\n');
            if (NULL != subString)
                subString = strpbrk(subString, "0123456789");
            }
        }

    return bFoundBandStats;
    }


//-----------------------------------------------------------------------------
// Private
// CleanUpString
//
// Remove SPACE/TAB/ENTER from the begin and end of the file.
//-----------------------------------------------------------------------------
void HRFBilFile::CleanUpString(string* pio_pString)
    {
    HPRECONDITION(pio_pString != 0);

    size_t Pos = 0;

    // Remove the SPACE/TAB at the begin of the string.
    while (Pos < pio_pString->size() && !IsValidChar((*pio_pString)[Pos]))
        Pos++;

    *pio_pString = pio_pString->substr(Pos);

    // Remove the SPACE/TAB/ENTER at the end of the string.
    Pos = pio_pString->size() - 1;
    while(Pos >= 0 && !IsValidChar((*pio_pString)[Pos]))
        Pos--;

    *pio_pString = pio_pString->substr(0, Pos+1);
    }

//-----------------------------------------------------------------------------
// Private
// IsValidChar
//
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER.
//-----------------------------------------------------------------------------
bool HRFBilFile::IsValidChar(const char pi_Char)
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
//
// ConvertStringToDouble
//-----------------------------------------------------------------------------
bool ConvertStringToDouble(const string& pi_rString, double* po_pDouble)
    {
    char* pStopPtr;
    *po_pDouble = strtod(pi_rString.c_str(), &pStopPtr);

    return ((' ' == *pStopPtr) || ('\n' == *pStopPtr) || ('\r' == *pStopPtr) ||
            ('\t' == *pStopPtr)|| ('\0' == *pStopPtr));
    }

//-----------------------------------------------------------------------------
//
// ConvertStringToLong
//-----------------------------------------------------------------------------
bool ConvertStringToLong(const string& pi_rString, int32_t* po_pLong)
    {
    char* pStopPtr;
    *po_pLong = strtol(pi_rString.c_str(), &pStopPtr, 10);

    return ((' ' == *pStopPtr) || ('\n' == *pStopPtr) || ('\r' == *pStopPtr) ||
            ('\t' == *pStopPtr)|| ('\0' == *pStopPtr));
    }

//-----------------------------------------------------------------------------
//
// ConvertStringToLong
//-----------------------------------------------------------------------------
bool ConvertStringToULong(const string& pi_rString, uint32_t* po_pLong)
    {
    char* pStopPtr;
    *po_pLong = strtoul(pi_rString.c_str(), &pStopPtr, 10);

    return ((' ' == *pStopPtr) || ('\n' == *pStopPtr) || ('\r' == *pStopPtr) ||
            ('\t' == *pStopPtr)|| ('\0' == *pStopPtr));
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFBilFile::GetHdrLockManager()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);
    return m_pHdrSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFBilFile::HdrSharingControlCreate(HFCURL* pi_pHdrUrl)
    {
    HPRECONDITION (pi_pHdrUrl != 0);

    if (m_pHdrSharingControl == 0)
        m_pHdrSharingControl = new HRFSisterFileSharing(pi_pHdrUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFBilFile::HdrSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    return m_pHdrSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::HdrSharingControlSynchronize()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    m_pHdrSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::HdrSharingControlIncrementCount()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    m_pHdrSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFBilFile::GetHdrSharingControl()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    return (m_pHdrSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFBilFile::HdrSharingControlIsLocked()
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    return m_pHdrSharingControl->IsLocked();
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFBilFile::GetBndLockManager()
    {
    HPRECONDITION (m_pBndSharingControl != 0);
    return m_pBndSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFBilFile::BndSharingControlCreate(HFCURL* pi_pBndUrl)
    {
    HPRECONDITION (pi_pBndUrl != 0);

    if (m_pBndSharingControl == 0)
        m_pBndSharingControl = new HRFSisterFileSharing(pi_pBndUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFBilFile::BndSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pBndSharingControl != 0);

    return m_pBndSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::BndSharingControlSynchronize()
    {
    HPRECONDITION (m_pBndSharingControl != 0);

    m_pBndSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::BndSharingControlIncrementCount()
    {
    HPRECONDITION (m_pBndSharingControl != 0);

    m_pBndSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFBilFile::GetBndSharingControl()
    {
    HPRECONDITION (m_pBndSharingControl != 0);

    return (m_pBndSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFBilFile::BndSharingControlIsLocked()
    {
    HPRECONDITION (m_pBndSharingControl != 0);

    return m_pBndSharingControl->IsLocked();
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFBilFile::GetStxLockManager()
    {
    HPRECONDITION (m_pStxSharingControl != 0);
    return m_pStxSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFBilFile::StxSharingControlCreate(HFCURL* pi_pStxUrl)
    {
    HPRECONDITION (pi_pStxUrl != 0);

    if (m_pStxSharingControl == 0)
        m_pStxSharingControl = new HRFSisterFileSharing(pi_pStxUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFBilFile::StxSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pStxSharingControl != 0);

    return m_pStxSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::StxSharingControlSynchronize()
    {
    HPRECONDITION (m_pStxSharingControl != 0);

    m_pStxSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFBilFile::StxSharingControlIncrementCount()
    {
    HPRECONDITION (m_pStxSharingControl != 0);

    m_pStxSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFBilFile::GetStxSharingControl()
    {
    HPRECONDITION (m_pStxSharingControl != 0);

    return (m_pStxSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFBilFile::StxSharingControlIsLocked()
    {
    HPRECONDITION (m_pStxSharingControl != 0);

    return m_pStxSharingControl->IsLocked();
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// The rest of this file is the implentation of the sharing control for the
// Creator struct.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFBilCreator::HdrSharingControlCreate(const HFCPtr<HFCURL>& pi_pURL)
    {
    HPRECONDITION (pi_pURL != 0);

    if (m_pHdrSharingControl == 0)
        m_pHdrSharingControl = new HRFSisterFileSharing(pi_pURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFBilCreator::GetHdrSharingControl() const
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    return (m_pHdrSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HFCBinStreamLockManager instance.
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFBilCreator::GetHdrLockManager() const
    {
    HPRECONDITION (m_pHdrSharingControl != 0);

    return m_pHdrSharingControl->GetLockManager();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetRedChannel() const
    {
    return m_bilFileHeader.rchannel;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetGreenChannel() const
    {
    return m_bilFileHeader.gchannel;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetBlueChannel() const
    {
    return m_bilFileHeader.bchannel;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetBandNumber() const
    {
    return m_bilFileHeader.numbands;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetHeaderLength() const
    {
    return m_bilFileHeader.hdrlength;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetTotalRowBytes() const
    {
    return m_bilFileInfo.NbBytesPerRow;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetBandRowBytes() const
    {
    return m_bilFileInfo.NbBytesPerBand;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFBilFile::GetBandGapBytes() const
    {
    return m_bilFileInfo.NbBandGapBytes;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFBilFile::IsMsByteFirst() const
    {
    return m_bilFileInfo.IsMsByteFirst;
    }
