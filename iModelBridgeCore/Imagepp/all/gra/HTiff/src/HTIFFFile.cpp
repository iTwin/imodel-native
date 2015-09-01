//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HTIFFFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HTIFFDirectory.h>
#include <Imagepp/all/h/HTIFFTagDefinition.h>

#include <Imagepp/all/h/HPMAttributeSet.h>

#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecFlashpixOLDForMSI10.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecJBIG.h>
#include <Imagepp/all/h/HCDCodecIJG.h>

#include <Imagepp/all/h/HFCMemcpy.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCBinStreamLockManager.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodec.h>
#include <Imagepp/all/h/HTIFFTag.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HTIFFGeoKey.h>



// Private Directory
//const Int32  HTIFFFile::HMR_DIRECTORY = -1;


// Simulate strip if the image has only one, and the strip is >
#define DEF_MAX_STRIPSIZE           (256*1024)
#define DEF_MAX_SIMULATESTRIPSIZE   (64*1024)

// default value
static unsigned short s_a1BitBySample[4]   = {1,1,1,1};
static unsigned short s_a16BitBySample[4]  = {16,16,16,16};

#define IDENTIFY_UNDOREDO_FILE      "Imagepp Undo-Redo file"

#define MAX_SUM_MOVE_DATA           134217728


static HTIFFTagInfo HTIFF_TAG_INFO;

//-----------------------------------------------------------------------------
// public
// Constructor,
//-----------------------------------------------------------------------------
HTIFFFile::HTIFFFile(const WString& pi_rFilename,
                     HFCAccessMode  pi_Mode,
                     uint64_t      pi_OriginOffset,
                     bool          pi_CreateBigTifFormat,
                     bool          pi_ValidateDir,
                     bool*         po_pNewFile)
    :   HTagFile(pi_rFilename, HTIFF_TAG_INFO, pi_Mode, pi_OriginOffset, pi_CreateBigTifFormat, pi_ValidateDir, po_pNewFile)
    {
    Initialize();

    // See HTagFile class documentation
    Construct(0, &pi_rFilename, pi_Mode, pi_OriginOffset, pi_CreateBigTifFormat, pi_ValidateDir, po_pNewFile);
    }

//-----------------------------------------------------------------------------
// public
// Constructor,
//-----------------------------------------------------------------------------
HTIFFFile::HTIFFFile(const HFCPtr<HFCURL>&  pi_rpURL,
                     uint64_t               pi_offset,
                     HFCAccessMode          pi_AccessMode,
                     bool                  pi_CreateBigTifFormat,
                     bool*                 po_pNewFile)
    :   HTagFile(pi_rpURL, HTIFF_TAG_INFO, pi_AccessMode, pi_CreateBigTifFormat, po_pNewFile)
    {
    Initialize();

    // See HTagFile class documentation
    Construct(pi_rpURL, 0, pi_AccessMode, pi_offset, pi_CreateBigTifFormat, true, po_pNewFile);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HTIFFFile::~HTIFFFile()
    {
    // See HTagFile class documentation
    SaveTagFile();
    }


void HTIFFFile::Initialize ()
    {
    m_pPacket = new HCDPacket();
    m_pPacket->SetBufferOwnership(true);

    // Init Members
    m_IsCompress            = false;
    m_FillOrder             = FILLORDER_MSB2LSB; // Default value, return Bit in

    m_pCompressFunc         = 0;
    m_pUncompressFunc       = 0;
    m_pSetHeightFunc        = 0;
    m_CompressionQuality    = -1;       // not used
    // Disable    m_SynchroOffset         = 0;
    m_NbBitUsed             = 16;
    m_InterpretMaxSampleValue  = true;


    m_HasBlockFlags   = false;
    m_HasHMRFormat    = false;
    m_HasiTiffFormat  = false;
    m_UndoRedoFileMode  = false;

    m_StripAreSimulated = false;
    m_SimulateLine_OneStripPackBitRGBA = false;
    }


bool HTIFFFile::SetPage(uint32_t pi_Page)
    {
    // search page
    uint32_t DirIndex = 0;
    if (pi_Page > 0)
        {
        uint32_t CurrentPage = -1;        // We start at -1, we will increment when we encounter a page.
        uint32_t ImageType;

        for (DirIndex = 0; DirIndex < m_ListDirCount; DirIndex++)
            {
            // if the tag is not present, it is FULLIMAGE
            if (m_ppListDir[DirIndex]->GetValues(SUBFILETYPE, &ImageType))
                {
                if (ImageType == FILETYPE_FULLIMAGE || (ImageType & FILETYPE_PAGE) != 0 || ImageType == FILETYPE_EMPTYPAGE)
                    CurrentPage++;
                }
            else
                CurrentPage++;         // if the tag is not present, it is FULLIMAGE

            if (CurrentPage == pi_Page)
                break;
            }
        }

    if (DirIndex < m_ListDirCount)
        return SetDirectoryImpl(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, DirIndex), true);
    else
        return false;
    }

bool HTIFFFile::GetEXIFDefinedGPSTags(uint32_t pi_PageDirInd, HPMAttributeSet& po_rExifGpsTags)
    {
    HPRECONDITION(pi_PageDirInd < m_ListDirCount);
    HPRECONDITION(IsTiff64() == false);

    uint32_t        GPSIFDOffset;
    HTIFFError*     pTIFFError = 0;
    double         ConvRationalToDblVals[3];
    Byte*          pByteVal;
    unsigned short*        pShortVal;
    double*        pDblVal;
    char*          pCharVal;
    uint32_t        NbVals;
    vector<Byte>   ByteVec;
    vector<double> DblVec;

    if (m_ppListDir[pi_PageDirInd]->GetValues(GPSDIRECTORY, &GPSIFDOffset))
        {
        HAutoPtr<HTIFFDirectory> pGPSDir(new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64()));

        pGPSDir->ReadDirectory(m_pFile, GPSIFDOffset);

        if ((pGPSDir->IsValid(&pTIFFError) == true) ||
            (pTIFFError->IsFatal() == false))
            {
            if (pGPSDir->GetValues(GPS_VERSIONID, &NbVals, &pByteVal))
                {
                HASSERT(NbVals == 4);

                FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSVersionID(ByteVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_LATITUDEREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSLatitudeRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_LATITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);
                pDblVal = ConvRationalToDblVals;

                FILL_VECTOR_WITH_PTR_VALS(DblVec, NbVals, pDblVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSLatitude(DblVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_LONGITUDEREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSLongitudeRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_LONGITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);
                pDblVal = ConvRationalToDblVals;

                FILL_VECTOR_WITH_PTR_VALS(DblVec, NbVals, pDblVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSLongitude(DblVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_ALTITUDEREF, &NbVals, &pByteVal))
                {
                HASSERT(NbVals == 1);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSAltitudeRef(*pByteVal);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_ALTITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSAltitude(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_TIMESTAMP, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);
                pDblVal = ConvRationalToDblVals;

                FILL_VECTOR_WITH_PTR_VALS(DblVec, NbVals, pDblVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSTimeStamp(DblVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_SATELLITES, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSSatellites(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_STATUS, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSStatus(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_MEASUREMODE, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSMeasureMode(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DOP, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDOP(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_SPEEDREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSSpeedRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_SPEED, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSSpeed(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_TRACKREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSTrackRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_TRACK, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSTrack(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_IMGDIRECTIONREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSImgDirectionRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_IMGDIRECTION, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSImgDirection(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_MAPDATUM, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSMapDatum(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTLATITUDEREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestLatitudeRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTLATITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);
                pDblVal = ConvRationalToDblVals;

                FILL_VECTOR_WITH_PTR_VALS(DblVec, NbVals, pDblVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestLatitude(DblVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTLONGITUDEREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestLongitudeRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTLONGITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);
                pDblVal = ConvRationalToDblVals;

                FILL_VECTOR_WITH_PTR_VALS(DblVec, NbVals, pDblVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestLongitude(DblVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTBEARINGREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestBearingRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTBEARING, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestBearing(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTDISTANCEREF, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestDistanceRef(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DESTDISTANCE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDestDistance(ConvRationalToDblVals[0]);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_PROCESSINGMETHOD, &NbVals, &pByteVal))
                {
                FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSProcessingMethod(ByteVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_AREAINFORMATION, &NbVals, &pByteVal))
                {
                FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSAreaInformation(ByteVec);
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DATESTAMP, &pCharVal))
                {
                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDateStamp(WString(pCharVal,false));
                po_rExifGpsTags.Set(pTag);
                }

            if (pGPSDir->GetValues(GPS_DIFFERENTIAL, &NbVals, &pShortVal))
                {
                HASSERT(NbVals == 1);

                HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGPSDifferential(*pShortVal);
                po_rExifGpsTags.Set(pTag);
                }
            }
        }

    return true;
    }


bool HTIFFFile::GetEXIFTags(uint32_t pi_PageDirInd, HPMAttributeSet& po_rExifGpsTags)
    {
    HPRECONDITION(pi_PageDirInd < m_ListDirCount);
    HPRECONDITION(IsTiff64() == false);

    uint32_t                 GPSIFDOffset;
    HTIFFError*              pTIFFError = 0;
    double                  ConvRationalToDblVals[3];

    Byte                   UByteVal;
    unsigned short          UShortVal;
    uint32_t                 ULongVal;

    Byte*                   pByteVal;
    unsigned short*                 pUShortVal;
    double*                 pDblVal;
    char*                   pCharVal;
    uint32_t                 NbVals;
    vector<char>            CharVec;
    vector<Byte>            ByteVec;
    vector<double>          DblVec;
    vector<unsigned short>          UShortVec;

    //By default, search the EXIF tags in the base directory
    HTIFFDirectory*          pDirToSearch(m_ppListDir[pi_PageDirInd]);
    HAutoPtr<HTIFFDirectory> pExifDir;

    //Check for a private IFD
    if (m_ppListDir[pi_PageDirInd]->GetValues(EXIFDIRECTORY, &GPSIFDOffset))
        {
        pExifDir = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
        pExifDir->ReadDirectory(m_pFile, GPSIFDOffset);

        if ((pExifDir->IsValid(&pTIFFError) == true) ||
            (pTIFFError->IsFatal() == false))
            {
            //If a private IFD is found, search the EXIF tags from that directory instead.
            pDirToSearch = pExifDir;
            }
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSURETIME, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExposureTime(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FNUMBER, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFNumber(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREPROGRAM, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExposureProgram(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SPECTRALSENSITIVITY, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSpectralSensitivity(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_ISOSPEEDRATINGS, &NbVals, &pUShortVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(UShortVec, NbVals, pUShortVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeISOSpeedRatings(UShortVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_OECF, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeOptoElectricConversionFactor(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_EXIFVERSION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        FILL_VECTOR_WITH_PTR_VALS(CharVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExifVersion(CharVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_DATETIMEORIGINAL, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeDateTimeOriginal(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_DATETIMEDIGITIZED, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeDateTimeDigitized(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_COMPONENTSCONFIGURATION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeComponentsConfiguration(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_COMPRESSEDBITSPERPIXEL, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeCompressedBitsPerPixel(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SHUTTERSPEEDVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals,
                                                 true);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeShutterSpeedValue(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }


    if (pDirToSearch->GetValues(EXIF_APERTUREVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeShutterSpeedValue(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_BRIGHTNESSVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals,
                                                 true);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeBrightnessValue(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREBIASVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals,
                                                 true);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExposureBiasValue(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_MAXAPERTUREVALUE, &NbVals, &pDblVal))
        {
        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeMaxApertureValue(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTDISTANCE, &NbVals, &pDblVal))
        {
        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubjectDistance(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_METERINGMODE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeMeteringMode(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_LIGHTSOURCE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeLightSource(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FLASH, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFlash(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALLENGTH, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFocalLength(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTAREA, &NbVals, &pUShortVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(UShortVec, NbVals, pUShortVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubjectArea(UShortVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_MAKERNOTE, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeMakerNote(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_USERCOMMENT, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeUserComment(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubSecTime(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME_ORIGINAL, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubSecTimeOriginal(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME_DIGITIZED, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubSecTimeDigitized(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FLASHPIXVERSION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFlashpixVersion(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_COLORSPACE, &NbVals, &pUShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeColorSpace(*pUShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_PIXELXDIMENSION, &ULongVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributePixelXDimension(ULongVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_PIXELYDIMENSION, &ULongVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributePixelYDimension(ULongVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_RELATEDSOUNDFILE, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeRelatedSoundFile(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FLASHENERGY, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFlashEnergy(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SPATIALFREQUENCYRESPONSE, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSpatialFrequencyResponse(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANEXRESOLUTION, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFocalPlaneXResolution(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANEYRESOLUTION, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFocalPlaneYResolution(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANERESOLUTIONUNIT, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFocalPlaneResolutionUnit(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTLOCATION, &NbVals, &pUShortVal))
        {
        HASSERT(NbVals == 2);

        FILL_VECTOR_WITH_PTR_VALS(UShortVec, NbVals, pUShortVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubjectLocation(UShortVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREINDEX, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExposureIndex(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SENSINGMETHOD, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSensingMethod(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FILESOURCE, &UByteVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFileSource(UByteVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SCENETYPE, &UByteVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSceneType(UByteVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_CFAPATTERN, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeCFAPattern(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_CUSTOMRENDERED, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeCustomRendered(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREMODE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeExposureMode(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_WHITEBALANCE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeWhiteBalance(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_DIGITALZOOMRATIO, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeDigitalZoomRatio(ConvRationalToDblVals[0]);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALLENGTHIN35MMFILM, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeFocalLengthIn35mmFilm(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SCENECAPTURETYPE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSceneCaptureType(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_GAINCONTROL, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeGainControl(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_CONTRAST, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeContrast(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SATURATION, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSaturation(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SHARPNESS, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSharpness(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_DEVICESETTINGDESCRIPTION, &NbVals, &pByteVal))
        {
        FILL_VECTOR_WITH_PTR_VALS(ByteVec, NbVals, pByteVal)
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeDeviceSettingDescription(ByteVec);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTDISTANCERANGE, &UShortVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeSubjectDistanceRange(UShortVal);
        po_rExifGpsTags.Set(pTag);
        }

    if (pDirToSearch->GetValues(EXIF_IMAGEUNIQUEID, &pCharVal))
        {
        HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeImageUniqueID(WString(pCharVal,false));
        po_rExifGpsTags.Set(pTag);
        }

    return true;
    }


uint32_t HTIFFFile::StripSize() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return m_StripTileSize;
    }

uint32_t HTIFFFile::TileSize() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return m_StripTileSize;
    }


// Get and Set methods
bool HTIFFFile::GetField (HTagID pi_Tag, unsigned short** po_ppVal1, unsigned short** po_ppVal2, unsigned short** po_ppVal3) const
    {
    HPRECONDITION(po_ppVal1 != 0);
    HPRECONDITION(po_ppVal2 != 0);
    HPRECONDITION(po_ppVal3 != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    bool Ret = false;

    switch (pi_Tag)
        {
        case TCOLORMAP:
            {
            unsigned short*     pBuf;
            uint32_t    Count;
            uint32_t    Len = (1L<<m_pBitsBySample[0]);

            if (m_pCurDir->GetValues(pi_Tag, &Count, &pBuf))
                {
                Ret = true;

                *po_ppVal1 = (unsigned short*)pBuf;
                *po_ppVal2 = (unsigned short*)(pBuf + Len);
                *po_ppVal3 = (unsigned short*)(pBuf + Len + Len);
                }
            }
        break;

        case TRANSFERFUNCTION:
            HASSERT(false);
            break;

        default:
            break;
        }

    return (Ret);
    }

bool HTIFFFile::SetField (HTagID pi_Tag, const unsigned short* pi_pVal1, const unsigned short* pi_pVal2, const unsigned short* pi_pVal3)
    {
    HPRECONDITION(pi_pVal1 != 0);
    HPRECONDITION(pi_pVal2 != 0);
    HPRECONDITION(pi_pVal3 != 0);
    HFCMonitor Monitor(m_Key);

    bool Ret = false;

    switch (pi_Tag)
        {
        case TCOLORMAP:
            {
            uint32_t    Len     = (1L<<m_pBitsBySample[0]);
            uint32_t    Count   = Len * 3; // RGB
            unsigned short*     pBuf   = new unsigned short[Count];
            HASSERT(pBuf != 0);

            memcpy(pBuf,            pi_pVal1, Len*sizeof(unsigned short));
            memcpy((pBuf+ Len),     pi_pVal2, Len*sizeof(unsigned short));
            memcpy((pBuf+ Len+ Len),pi_pVal3, Len*sizeof(unsigned short));

            Ret = m_pCurDir->SetValues(pi_Tag, Count, pBuf);

            delete pBuf;
            }
        break;

        case TRANSFERFUNCTION:
            HASSERT(false);
            break;

        default:
            break;
        }

    return (Ret);
    }


bool HTIFFFile::GetField (HTagID pi_Tag, RATIONAL* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    bool  Ret = false;
    uint32_t Val1;
    uint32_t Val2;

    //                              Numerator
    //                                         Denominator
    if (m_pCurDir->GetValues(pi_Tag, &Val1, &Val2))
        {
        if (Val2 != 0)
            {
            po_pVal->Value = ((double)Val1 / (double)Val2);
            Ret = true;
            }
        }

    return Ret;
    }

// Special case, po_pVal must be allocated before the call
// po_pCount represent the number of Rational read, basically the original count / 2.
//
bool HTIFFFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, RATIONAL* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    bool  Ret = false;
    uint32_t* pVal;
    uint32_t Count;

    if (m_pCurDir->GetValues(pi_Tag, &Count, &pVal))
        {
        *po_pCount = 0;
        for (uint32_t i=0; i<Count; i+=2)
            {
            if (pVal[i+1] != 0)
                (po_pVal[*po_pCount]).Value = ((double)pVal[i] / (double)pVal[i+1]);
            ++(*po_pCount);
            }

        Ret = true;
        }

    return Ret;
    }


bool HTIFFFile::SetField (HTagID pi_Tag, RATIONAL pi_Val)
    {
    double Val = pi_Val.Value;
    uint32_t Denominator = 1L;
    uint32_t Numerator;
    HFCMonitor Monitor(m_Key);

    if (Val < 0)
        {
        HTIFFError::NegativeValueForRationalErInfo ErInfo;
        string TagName = string(GetTagNameString(pi_Tag));
        ErInfo.m_TagName = WString(TagName.c_str(),false);
        ErInfo.m_RationalValue = Val;

        ErrorMsg(&m_pError,
                 HTIFFError::NEGATIVE_VALUE_FOR_RATIONAL,
                 &ErInfo,
                 false);
        goto WRAPUP;
        }

    if (Val > 0)
        {
        while (Val < 1L<<(31-3) && Denominator < 1L<<(31-3))
            {
            Val *= 1<<3;
            Denominator *= 1L<<3;
            }
        }

    Numerator = (uint32_t)(Val + 0.5);

    return m_pCurDir->SetValues(pi_Tag, Numerator, Denominator);
WRAPUP:
    return false;
    }

bool HTIFFFile::SetField (HTagID pi_Tag, unsigned short pi_Val)
    {
    bool Ret;
    HFCMonitor Monitor(m_Key);

    // Support only Depth == 1 for the moment.
    if (((pi_Tag == IMAGEDEPTH) || (pi_Tag == TILEDEPTH)) && (pi_Val != 1))
        Ret = false;

    else if ((Ret=m_pCurDir->SetValues(pi_Tag, pi_Val)))
        {
        switch (pi_Tag)
            {
            case BITSPERSAMPLE:
                m_pCurDir->GetValues (BITSPERSAMPLE, &m_NbSampleFromFile, &m_pBitsBySample);
                m_pBitsBySample[0] = pi_Val;
                break;

            case PHOTOMETRIC:
                m_Photometric = pi_Val;
                break;

            case SAMPLESPERPIXEL:
                m_SamplesByPixel = pi_Val;
                break;

            case PLANARCONFIG:
                m_PlanarConfig  = pi_Val;
                break;

            default:
                break;
            }
        }
    return Ret;
    }

bool HTIFFFile::SetField (HTagID pi_Tag, uint32_t pi_Val)
    {
    bool Ret(true);
    HFCMonitor Monitor(m_Key);

    // Support only Depth == 1 for the moment.
    if (((pi_Tag == IMAGEDEPTH) || (pi_Tag == TILEDEPTH)) && (pi_Val != 1))
        Ret = false;

    else if ((Ret=m_pCurDir->SetValues(pi_Tag, pi_Val)))
        {
        switch (pi_Tag)
            {
            case IMAGEWIDTH:
                m_ImageWidth = pi_Val;
                break;
            case IMAGELENGTH:
                m_ImageLength= pi_Val;
                break;
            case ROWSPERSTRIP:
                m_RowsByStrip= pi_Val;
                break;

            case COMPRESSION_QUALITY:   // Not saved in file
                m_CompressionQuality = pi_Val;
                SetQualityToCodec();
                break;

            default:
                break;
            }
        }

    return Ret;
    }




HSTATUS HTIFFFile::TileRead (Byte* po_pData, uint32_t pi_PosX, uint32_t pi_PosY)
    {
    HFCMonitor Monitor(m_Key);

    uint32_t TilePos = ComputeTile(pi_PosX, pi_PosY);
    return ReadData(po_pData, TilePos);
    }

HSTATUS HTIFFFile::TileRead (HFCPtr<HCDPacket>& po_prPacket, uint32_t pi_PosX, uint32_t pi_PosY)
    {
    // If the data is read compressed we can't swap the bytes.
    HPRECONDITION(m_IsCompress && 16 == m_pBitsBySample[0]? !m_ByteOrder.NeedSwapByte() : true);

    HFCMonitor Monitor(m_Key);

    uint32_t TilePos = ComputeTile(pi_PosX, pi_PosY);
    return ReadSeparateOrContigData(po_prPacket, TilePos);
    }

HSTATUS HTIFFFile::TileWrite (const Byte* pi_pData, uint32_t pi_PosX, uint32_t pi_PosY)
    {
    HFCMonitor Monitor(m_Key);

    if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
        {
        if (m_IsCompress)
            {
            HCLASS_ID CurrentCodec = GetCurrentCodec()->GetClassID();

            if (CurrentCodec == HCDCodecIJG::CLASS_ID ||
                CurrentCodec == HCDCodecFlashpix::CLASS_ID ||
                CurrentCodec == HCDCodecFlashpixOLDForMSI10::CLASS_ID)
                {
                // Get the tile size
                uint32_t TileWidth    = 0;
                uint32_t TileLength   = 0;

                m_pCurDir->GetValues (TILEWIDTH, &TileWidth);
                m_pCurDir->GetValues (TILELENGTH, &TileLength);

                // Compute the size of the relevant data
                uint32_t RelevantWidth  = MIN(TileWidth, m_ImageWidth - pi_PosX);
                uint32_t RelevantHeight = MIN(TileLength, m_ImageLength - pi_PosY);

                // Pad the data
                // DG note: We don't want to make a copy of the data ... so we
                //          const_cast the ptr. We can do this asuming the the modified
                //          data is out of the image bondary.
                PrepareForJPEG(const_cast <Byte*> (pi_pData),
                               TileWidth,
                               TileLength,
                               RelevantWidth,
                               RelevantHeight,
                               m_BitsByPixel);
                }
            }

        uint32_t TilePos = ComputeTile(pi_PosX, pi_PosY);
        return WriteSeparateOrContigData(pi_pData, TilePos);
        }
    else
        return H_READ_ONLY_ACCESS;
    }

HSTATUS HTIFFFile::StripRead (Byte* po_pData, uint32_t pi_Strip)
    {
    HFCMonitor Monitor(m_Key);

    if (m_SimulateLine_OneStripPackBitRGBA)
        {
        // If the data is read compressed we can't swap the bytes.
        HASSERT(m_IsCompress && 16 == m_pBitsBySample[0] ? !m_ByteOrder.NeedSwapByte() : true);

        // First strip, read the data completely
        if (m_pCompressedPacketSimulateLine == 0)
            {
            m_pCompressedPacketSimulateLine = new HCDPacket();
            m_pCompressedPacketSimulateLine->SetBufferOwnership(true);
            ReadSeparateOrContigData(m_pCompressedPacketSimulateLine, 0);
            ((HFCPtr<HCDCodecImage>&)m_pCompressedPacketSimulateLine->GetCodec())->SetDimensions(m_ImageWidth, m_ImageLength);
            ((HFCPtr<HCDCodecImage>&)m_pCompressedPacketSimulateLine->GetCodec())->SetSubset(m_ImageWidth, 1, 0, 0);

            m_CurrentLineSimulateLine = 0;
            }

        HASSERT(pi_Strip >= m_CurrentLineSimulateLine);

        // Skip line if necessary
        for (; m_CurrentLineSimulateLine <= pi_Strip; ++m_CurrentLineSimulateLine)
            {
            m_pCompressedPacketSimulateLine->GetCodec()->DecompressSubset(
                m_pCompressedPacketSimulateLine->GetBufferAddress() +
                ((HFCPtr<HCDCodecImage>&)m_pCompressedPacketSimulateLine->GetCodec())->GetCompressedImageIndex(),
                m_pCompressedPacketSimulateLine->GetDataSize(),
                po_pData,
                m_ImageWidth);
            }

        // last strip, free the memory
        if (pi_Strip >= m_ImageLength-1)
            m_pCompressedPacketSimulateLine = 0;

        return H_SUCCESS;
        }
    else
        {
        // Ajust dimension in the Codec, if it is the last Strip.
        //
        if (m_pSetHeightFunc != 0)
            {
            // Last Strip
            if (pi_Strip == (m_NbData32-1))
                (this->*m_pSetHeightFunc)(m_ImageLength - (pi_Strip*m_RowsByStrip));
            else
                (this->*m_pSetHeightFunc)(m_RowsByStrip);
            }

        return ReadData(po_pData, pi_Strip);
        }
    }

HSTATUS HTIFFFile::StripRead (HFCPtr<HCDPacket>& po_prPacket, uint32_t pi_Strip)
    {
    HFCMonitor Monitor(m_Key);
    HPRECONDITION(m_SimulateLine_OneStripPackBitRGBA == false);

    // If the data is read compressed we can't swap the bytes.
    HPRECONDITION(m_IsCompress && m_pBitsBySample[0] == 16 ? !m_ByteOrder.NeedSwapByte() : true);

    // Ajust dimension in the Codec, if it is the last Strip.
    //
    if (m_pSetHeightFunc != 0)
        {
        // Last Strip
        if (pi_Strip == (m_NbData32-1))
            (this->*m_pSetHeightFunc)(m_ImageLength - (pi_Strip*m_RowsByStrip));
        else
            (this->*m_pSetHeightFunc)(m_RowsByStrip);
        }

    return ReadSeparateOrContigData(po_prPacket, pi_Strip);
    }

HSTATUS HTIFFFile::StripWrite (const Byte* pi_pData, uint32_t pi_Strip)
    {
    HFCMonitor Monitor(m_Key);

    if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
        {
        // Ajust dimension in the Codec, if it is the last Strip.
        if (m_pSetHeightFunc != 0)
            {
            // Last Strip
            if (pi_Strip == (m_NbData32-1))
                (this->*m_pSetHeightFunc)(m_ImageLength - (pi_Strip*m_RowsByStrip));
            else
                (this->*m_pSetHeightFunc)(m_RowsByStrip);
            }
        return WriteSeparateOrContigData(pi_pData, pi_Strip);
        }
    else
        return H_READ_ONLY_ACCESS;
    }

HSTATUS HTIFFFile::TileWriteCompress   (const Byte* pi_pData, uint32_t pi_DataLen, uint32_t pi_PosX, uint32_t pi_PosY)
    {
    HFCMonitor Monitor(m_Key);

    if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
        {
        uint32_t TilePos = ComputeTile(pi_PosX, pi_PosY);
        return WriteSeparateOrContigData(pi_pData, TilePos, pi_DataLen);
        }
    else
        return H_READ_ONLY_ACCESS;
    }

HSTATUS HTIFFFile::StripWriteCompress  (const Byte* pi_pData, uint32_t pi_DataLen, uint32_t pi_Strip)
    {
    HFCMonitor Monitor(m_Key);

    if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
        return WriteSeparateOrContigData(pi_pData, pi_Strip, pi_DataLen);
    else
        return H_READ_ONLY_ACCESS;
    }


//-----------------------------------------------------------------------------
// ProjectWise project
//

// ReadProjectWiseBlob : Read Data and/or Get info
//      The po_pData must be big enough to read the blob completly.
//
//      Each parameter can be set to 0.
//      ex: To get the size only, ReadProjectWiseBlob(0, &BlobSize)
//
//  Return true if tag present and no read error.
bool HTIFFFile::ReadProjectWiseBlob(uint32_t pi_Page, Byte* po_pData, uint32_t* po_pSize) const
    {
    bool Ret = false;
    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    HMRDirectory64* pHMRDir = m_ppListHMRDir64[pi_Page];
    if ((pHMRDir != 0) && pHMRDir->m_pDirectory->TagIsPresent(HMR_PROJECTWISE_BLOB))
        {
        uint32_t* pOffset_Size;
        uint32_t NbEntry;
        pHMRDir->m_pDirectory->GetValues (HMR_PROJECTWISE_BLOB,  &NbEntry, &pOffset_Size);

        // Check Gets options
        if ((po_pData == 0) && (po_pSize == 0))
            Ret = true;
        else
            {
            // Presently, support only 1 block of data...--> 2 entry [Offset][Size]
            HASSERT(NbEntry == 2);

            if (po_pSize != 0)
                *po_pSize = pOffset_Size[1];

            if (po_pData != 0)
                {
                if ((m_pFile->Seek (pOffset_Size[0], SEEK_SET) &&
                     (m_pFile->Read (po_pData, 1, pOffset_Size[1]) == pOffset_Size[1])))
                    Ret = true;
                }
            }
        }

    CacheFileLock.ReleaseKey();

    return Ret;
    }

// WriteProjectWiseBlob : Write Blob.
//
//  The HMR directory, must be created before call this method.
//
//  Return true if no problem to write.
bool HTIFFFile::WriteProjectWiseBlob(uint32_t pi_Page, const Byte* pi_pData, uint32_t pi_pSize)
    {
    HPRECONDITION(pi_Page < m_ListHMRDirCount);
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_pSize > 0);

    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    bool Ret = false;
    HMRDirectory64* pHMRDir = m_ppListHMRDir64[pi_Page];
    if (pHMRDir != 0)
        {
        uint64_t Offset_Size[2];
        uint32_t NbEntry = 2;

        if (!pHMRDir->m_pDirectory->TagIsPresent(HMR_PROJECTWISE_BLOB))
            {
            if (IsTiff64())
                {
                uint64_t aDefaultOffsetSize[2];
                aDefaultOffsetSize[0] = 0;
                aDefaultOffsetSize[1] = 0;

                // Create the entry
                pHMRDir->m_pDirectory->SetValues (HMR_PROJECTWISE_BLOB,  NbEntry, aDefaultOffsetSize);
                }
            else
                {
                uint32_t aDefaultOffsetSize[2];
                aDefaultOffsetSize[0] = 0;
                aDefaultOffsetSize[1] = 0;

                // Create the entry
                pHMRDir->m_pDirectory->SetValues (HMR_PROJECTWISE_BLOB,  NbEntry, aDefaultOffsetSize);
                }
            }

        void* pOffset_Size = 0;
        if (IsTiff64())
            {
            uint64_t* pVal;
            pHMRDir->m_pDirectory->GetValues (HMR_PROJECTWISE_BLOB,  &NbEntry, &pVal);
            Offset_Size[0] = ((uint64_t*)pVal)[0];
            Offset_Size[1] = ((uint64_t*)pVal)[1];
            pOffset_Size = pVal;
            }
        else
            {
            uint32_t* pVal;
            pHMRDir->m_pDirectory->GetValues (HMR_PROJECTWISE_BLOB,  &NbEntry, &pVal);
            Offset_Size[0] = ((uint32_t*)pVal)[0];
            Offset_Size[1] = ((uint32_t*)pVal)[0];
            pOffset_Size = pVal;
            }

        // Find a space in the file, to write the Data.
        m_pFile->CheckAlloc (&(Offset_Size[0]), (uint32_t)Offset_Size[1], pi_pSize);
        Offset_Size[1] = pi_pSize;

        if (!m_pFile->Seek (Offset_Size[0], SEEK_SET) ||
            (m_pFile->Write (pi_pData, 1, (uint32_t)Offset_Size[1]) != (size_t)Offset_Size[1]))
            {
            // Do as if the tag was never written
            Offset_Size[0] = 0;
            Offset_Size[1] = 0;

            HTIFFError::CannotWritePWblobErInfo ErInfo;
            ErInfo.m_Offset = Offset_Size[0];
            ErInfo.m_Length = Offset_Size[1];

            ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_PW_BLOB, &ErInfo, true);
            }
        else
            Ret = true;

        // set the tag values
        if (IsTiff64())
            {
            ((uint64_t*)pOffset_Size)[0] = Offset_Size[0];
            ((uint64_t*)pOffset_Size)[1] = Offset_Size[1];
            }
        else
            {
            ((uint32_t*)pOffset_Size)[0] = (uint32_t)Offset_Size[0];
            ((uint32_t*)pOffset_Size)[1] = (uint32_t)Offset_Size[1];
            }

        // Flag the tag as dirty
        pHMRDir->m_pDirectory->Touched(HMR_PROJECTWISE_BLOB);
        }

    CacheFileLock.ReleaseKey();

    return Ret;
    }

// ProjectWise project End
//-----------------------------------------------------------------------------


//-------------------------------------------------------------- Privates



//-------------------------------------------------------------------------------------
// GetInfoFileData
// Method special to implement support Codec Wavelet.
// This method return the pointer on Stream directly and the Offset of data.
//-------------------------------------------------------------------------------------
HTIFFStream* HTIFFFile::GetInfoFileData(uint64_t* po_Offset)
    {
    SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, 0));

    // Check if a new file ?
    if (GetOffset(0) == 0)
        {
        m_pFile->Seek(0, SEEK_END);
        SetOffset(0, m_pFile->Tell());
        SetCount(0, 1);

        // Touched the list of Offset\Count
        if (IsTiled())
            {
            m_pCurDir->Touched(TILEOFFSETS);
            m_pCurDir->Touched(TILEBYTECOUNTS);
            }
        else
            {
            m_pCurDir->Touched(STRIPOFFSETS);
            m_pCurDir->Touched(STRIPBYTECOUNTS);
            }
        }

    // Offset in the first strip is the offset of the data.
    *po_Offset = GetOffset(0);

    return m_pFile;
    }


//-----------------------------------------------------------------------------
// Returns the magic number for a little endian file
//-----------------------------------------------------------------------------
HTagFile::MagicNumber HTIFFFile::GetLittleEndianMagicNumber () const
    {
    return 0x4949; // ASCII -> "II" (for Intel)
    }

//-----------------------------------------------------------------------------
// Returns the magic number for a big endian file
//-----------------------------------------------------------------------------
HTagFile::MagicNumber HTIFFFile::GetBigEndianMagicNumber () const
    {
    return 0x4d4d; // ASCII -> "MM" (for Motorola)
    }


bool HTIFFFile::IsValidTopDirectory (HTIFFError** po_ppError)
    {
    // Check if the file is valid
    if (!m_pCurDir->TagIsPresent (IMAGEWIDTH) || !m_pCurDir->TagIsPresent (IMAGELENGTH))
        {
        if (0 != po_ppError)
            {
            ErrorMsg(po_ppError,
                     HTIFFError::IMAGE_WIDTH_AND_LENGTH_ARE_REQUIRED,
                     0,
                     true);
            }

        return false;
        }

    return true;
    }

void HTIFFFile::OnTopDirectoryFirstInitialized ()
    {
    m_HasBlockFlags   = m_pCurDir->TagIsPresent(HMR2_TILEFLAG);
    m_HasHMRFormat    = m_pCurDir->TagIsPresent(HMR_IMAGEINFORMATION);
    m_HasiTiffFormat  = m_pCurDir->TagIsPresent(HMR2_IMAGEINFORMATION);
    }

//-------------------------------------------------------------------------------------
// IsValidReducedImage
//-------------------------------------------------------------------------------------
bool HTIFFFile::IsValidReducedImage(HTIFFDirectory* pi_ReducedImageDir, HTIFFDirectory* pi_pCurPageDir)
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(pi_ReducedImageDir != 0 && pi_pCurPageDir != 0);
    HPRECONDITION(pi_ReducedImageDir->TagIsPresent(SUBFILETYPE));

    // PHOTOMETRIC
    unsigned short Photometric_Main        = 0;
    pi_pCurPageDir->GetValues(PHOTOMETRIC, &Photometric_Main);
    unsigned short Photometric_SubRes      = Photometric_Main;             // Init from main res in case tag is not defined in sub-res.
    pi_ReducedImageDir->GetValues(PHOTOMETRIC, &Photometric_SubRes);
    if(Photometric_Main != Photometric_SubRes)
        return false;

    // SAMPLESPERPIXEL
    unsigned short SamplePerPixel_Main     = 0;
    pi_pCurPageDir->GetValues(SAMPLESPERPIXEL, &SamplePerPixel_Main);
    unsigned short SamplePerPixel_SubRes   = SamplePerPixel_Main;          // Init from main res in case tag is not defined in sub-res.
    pi_ReducedImageDir->GetValues(SAMPLESPERPIXEL, &SamplePerPixel_SubRes);
    if(SamplePerPixel_Main != SamplePerPixel_SubRes)
        return false;

    // BITSPERSAMPLE
    uint32_t    NbSample                = SamplePerPixel_Main;
    unsigned short*    pBitPerSample_Main      = 0;
    pi_pCurPageDir->GetValues(BITSPERSAMPLE, &NbSample, &pBitPerSample_Main);
    unsigned short*    pBitPerSample_SubRes    = pBitPerSample_Main;           // Init from main res in case tag is not defined in sub-res.
    pi_ReducedImageDir->GetValues(BITSPERSAMPLE, &NbSample, &pBitPerSample_SubRes);
    if(pBitPerSample_Main != 0)
        {
        for(unsigned short i=0; i < NbSample; ++i)
            {
            if(pBitPerSample_Main[i] != pBitPerSample_SubRes[i])
                return false;
            }
        }

    // COMPRESSION
    uint32_t   compression_Main        = COMPRESSION_NONE;
    pi_pCurPageDir->GetValues(COMPRESSION, &compression_Main);
    uint32_t   compression_SubRes      = compression_Main;             // Init from main res in case tag is not defined in sub-res.
    pi_ReducedImageDir->GetValues(COMPRESSION, &compression_SubRes);
    if(compression_Main != compression_SubRes)
        return false;

    // TILE or STRIP
    if(IsTiled(pi_pCurPageDir) != IsTiled(pi_ReducedImageDir))
        return false;

    return true;
    }

bool HTIFFFile::DirectoryIsValid (HTIFFDirectory* pi_Dir, HTIFFDirectory* pi_pCurPageDir)
    {
    HFCMonitor Monitor(m_Key);

    // Validate needed Tag
    uint32_t LVal;
    uint32_t CurWidth  = 0L;
    bool  EmptyPage = false;

    if (pi_Dir->TagIsPresent(SUBFILETYPE))
        {
        // Be sure to really have a SubImage.
        pi_Dir->GetValues (SUBFILETYPE, &LVal);
        EmptyPage = (LVal == FILETYPE_EMPTYPAGE);
        }

    if (!EmptyPage)
        {
        if (pi_Dir->TagIsPresent(IMAGEDEPTH) || pi_Dir->TagIsPresent(TILEDEPTH))
            {
            if (!pi_Dir->GetValues(IMAGEDEPTH, &LVal))
                pi_Dir->GetValues(TILEDEPTH, &LVal);
            if (LVal != 1)
                {
                ErrorMsg(&m_pError,
                         HTIFFError::IMAGE_DEPTH_DIFFERENT_THAN_1_NOT_SUPPORTED,
                         0,
                         true);
                goto WRAPUP;
                }
            }

        if (!pi_Dir->TagIsPresent(IMAGEWIDTH) || !pi_Dir->TagIsPresent(IMAGELENGTH))
            {
            ErrorMsg(&m_pError,
                     HTIFFError::IMAGE_WIDTH_AND_LENGTH_ARE_REQUIRED,
                     0,
                     true);
            goto WRAPUP;
            }
        else
            {
            if (!pi_Dir->GetValues(IMAGEWIDTH, &CurWidth))
                pi_Dir->GetValues(IMAGELENGTH, &CurWidth);
            }

        if ( !(pi_Dir->TagIsPresent(STRIPOFFSETS) ||  pi_Dir->TagIsPresent(TILEOFFSETS)) )
            {
            ErrorMsg(&m_pError,
                     HTIFFError::MISSING_STRIP_OR_TILE_OFFSET,
                     0,
                     true);
            goto WRAPUP;
            }

        if (!pi_Dir->TagIsPresent(PLANARCONFIG))
            {
            ErrorMsg(&m_pError,
                     HTIFFError::MISSING_IMAGE_PLANAR_CONFIGURATION,
                     0,
                     false);
            }

        if (pi_Dir->GetValues(PHOTOMETRIC, &LVal))
            {
            if ((LVal == PHOTOMETRIC_PALETTE) && (!pi_Dir->TagIsPresent(TCOLORMAP)))
                {
                ErrorMsg(&m_pError,
                         HTIFFError::MISSING_COLOR_MAP,
                         0,
                         true);
                goto WRAPUP;
                }
            }

        if (pi_Dir->TagIsPresent(SUBFILETYPE))
            {
            // Be sure to really have a SubImage.
            pi_Dir->GetValues (SUBFILETYPE, &LVal);
            if (LVal == FILETYPE_REDUCEDIMAGE && m_ListDirCount > 0)
                {
                HASSERT(CurWidth);
                uint32_t MainWidth  = 0L;

                // Retreive the main width
                if (!pi_pCurPageDir->GetValues(IMAGEWIDTH, &MainWidth))
                    pi_pCurPageDir->GetValues(IMAGELENGTH, &MainWidth);

                if (CurWidth >= MainWidth)
                    {
                    // A valid FILETYPE_REDUCEDIMAGE must be smaller than the main res.
                    goto WRAPUP;
                    }
                }
            }
        }
    return true;
WRAPUP:
    return false;
    }

//
// NumberOfTiles
//
// In separate planar config it returns the number of tiles like
// it was a contig planar because the separate planes are treated
// by the read/write operation.  In this case NumberOfTiles()
// is not equal to m_NbData.
uint32_t HTIFFFile::NumberOfTiles() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    uint32_t TileWidth    = 0;
    uint32_t TileLength   = 0;
    uint32_t NBTiles      = 0;

    if (m_pCurDir->GetValues (TILEWIDTH, &TileWidth) &&
        m_pCurDir->GetValues (TILELENGTH, &TileLength))
        NBTiles = HowMany(m_ImageWidth, TileWidth) * HowMany(m_ImageLength, TileLength);

    return (NBTiles);
    }

uint32_t HTIFFFile::ComputeTile (uint32_t pi_PosX, uint32_t pi_PosY) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    uint32_t TileWidth    = 0;
    uint32_t TileLength   = 0;
    uint32_t TileID       = (uint32_t)-1;

    if (m_pCurDir->GetValues (TILEWIDTH, &TileWidth) &&
        m_pCurDir->GetValues (TILELENGTH, &TileLength))
        {
        uint32_t NbX = HowMany(m_ImageWidth, TileWidth);

        TileID = NbX*(pi_PosY/TileLength) + pi_PosX/TileWidth;
        }

    return TileID;
    }


uint32_t HTIFFFile::TileSizePrivate() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);
    uint32_t TileSize = 0;
    uint32_t TileLength = 0;
    uint32_t TileWidth  = 0;
    uint32_t Scanline;

    m_pCurDir->GetValues (TILELENGTH, &TileLength);
    m_pCurDir->GetValues (TILEWIDTH,  &TileWidth);

    // PLANARCONFIG_SEPARATE implementation.
    // In separate mode we return the tile size like a non-planar because
    // seperate data will be treated by ReadData(...)
    Scanline = m_BitsByPixel * TileWidth;

    TileSize = HowMany(Scanline, 8) * TileLength;

    return TileSize;
    }

uint32_t HTIFFFile::StripSizePrivate() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);
    uint32_t StripSize = 0;

    StripSize = m_RowsByStrip * ScanlineSize();

    return StripSize;
    }


void HTIFFFile::InitStripList()
    {
    HFCMonitor Monitor(m_Key);

    // Normally it is a new file
    //
    if (IsTiled())
        {
        m_NbData32 = NumberOfTiles();
        m_StripTileSize = TileSizePrivate();
        }
    else
        {
        m_NbData32 = NumberOfStrips();
        m_StripTileSize = StripSizePrivate();
        }

    // In separate planar we have an entry for each sample. ex RGB:  NbData = NumberOfStrips()*3.
    if(m_PlanarConfig == PLANARCONFIG_SEPARATE)
        {
        m_NbData32 = m_NbData32 * m_SamplesByPixel;
        }

    if (m_NbData32 > 0)
        {
        // Create the Tags
        //
        if (IsTiff64())
            {
            uint64_t* pData = new uint64_t[m_NbData32];
            HASSERT(pData != 0);
            memset (pData, 0, sizeof(uint64_t)*m_NbData32);

            SetField ((IsTiled() ? TILEOFFSETS : STRIPOFFSETS),   m_NbData32, pData);
            SetField ((IsTiled() ? TILEBYTECOUNTS : STRIPBYTECOUNTS),m_NbData32, pData);
            delete[] pData;

            ReadOffsetCountTags();
            }
        else
            {
            uint32_t* pData = new uint32_t[m_NbData32];
            HASSERT(pData != 0);
            memset (pData, 0, sizeof(uint32_t)*m_NbData32);

            SetField ((IsTiled() ? TILEOFFSETS : STRIPOFFSETS),   m_NbData32, pData);
            SetField ((IsTiled() ? TILEBYTECOUNTS : STRIPBYTECOUNTS),m_NbData32, pData);
            delete[] pData;

            ReadOffsetCountTags();
            }
        }
    }

bool HTIFFFile::IsAllSamplesWithSameBitsCount() const
    {
    for(uint32_t i(1); i < m_NbSampleFromFile; ++i)
        {
        if(m_pBitsBySample[0] != m_pBitsBySample[i])
            {
            return false;
            }
        }

    return true;
    }

void HTIFFFile::SimulateStripList(uint32_t pi_CompressMode)
    {
    HFCMonitor Monitor(m_Key);

    // Try to simulate List of with if possible, for
    // the big image without strip.
    // We Can not simulate strip when the file is in creation
    // or the file has block flags

    if ((!m_pFile->GetMode().m_HasCreateAccess) &&
        (!m_HasBlockFlags) &&
        (!m_HasHMRFormat) &&
        (!m_HasiTiffFormat) &&
        (pi_CompressMode == COMPRESSION_NONE) &&
        (m_NbData32 == 1) &&
        (m_StripTileSize > DEF_MAX_STRIPSIZE) &&
        (m_CurDir == 0))
        {
        uint32_t RowBytes      = ScanlineSize();
        uint64_t StartOffset  = GetOffset(0);      // Keep previous offset


        HDEBUGTEXT(L"SimulateStripList: Image with one strip, simulate Offset/Count fields\n");

        // Set members
        // Set row by strip to respect the strip capabilities, MIN 32 with increment of 16
        m_RowsByStrip   = (MAX(DEF_MAX_SIMULATESTRIPSIZE / RowBytes, 32) / 16) * 16;  // Assert we a mutiple of 16
        HASSERT(m_RowsByStrip % 16 == 0);

        m_NbData32      = (m_ImageLength+m_RowsByStrip-1) / m_RowsByStrip;
        m_StripTileSize = RowBytes*m_RowsByStrip;

        // Create new list
        //
        uint64_t* pDataOffset64 = new uint64_t[m_NbData32];
        HASSERT(pDataOffset64 != 0);
        uint32_t* pDataCount32 = new uint32_t[m_NbData32];
        HASSERT(pDataCount32 != 0);

        pDataOffset64[0] = StartOffset;
        pDataCount32[0]  = m_StripTileSize;
        for(uint32_t i=1; i<m_NbData32; i++)
            {
            pDataOffset64[i] = pDataOffset64[i-1]+m_StripTileSize;
            pDataCount32[i]  = m_StripTileSize;
            }

        // Ajust the last strip size
        uint32_t OverLines = (m_NbData32*m_RowsByStrip) - m_ImageLength;
        if (OverLines > 0)
            pDataCount32[m_NbData32-1] = (m_RowsByStrip-OverLines)*RowBytes;

        // The pDataOffset64 and pDataCount32 will be delete by this method when needed
        SetOffsetCountData(m_NbData32, pDataOffset64, pDataCount32);

        m_StripAreSimulated = true;
        }
    // Image not too big with a missing Strip-Count or invalid count
    //
    else if ((!m_pFile->GetMode().m_HasCreateAccess) &&
             (pi_CompressMode == COMPRESSION_NONE) &&
             (m_NbData32 == 1) && (!m_pCurDir->TagIsPresent(STRIPBYTECOUNTS) ||
                                   (GetCount(0) != m_StripTileSize)))
        {
        HDEBUGTEXT(L"SimulateStripList: Image not too big with a missing Strip-Count\n");
        uint32_t* pDataCount32 = new uint32_t[1];
        HASSERT(pDataCount32 != 0);

        pDataCount32[0] = m_StripTileSize;

        // The pDataCount32 will be delete by this method when needed
        SetOffsetCountData(1, 0, pDataCount32);
        }

    // Line image, with bad strip count.
    else if ((!m_pFile->GetMode().m_HasCreateAccess) &&
             (!m_HasBlockFlags) &&
             (!m_HasHMRFormat) &&
             (!m_HasiTiffFormat) &&
             (pi_CompressMode == COMPRESSION_NONE) &&
             (m_pCurDir->TagIsPresent(STRIPBYTECOUNTS)) &&
             (m_NbData32 == m_ImageLength) &&
             (GetCount(0) != m_StripTileSize))
        {
        HDEBUGTEXT(L"SimulateStripList: Line image, with bad strip count\n");
        for(uint32_t i=0; i<m_NbData32; i++)
            SetCount(i, m_StripTileSize);
        }

    // Image with a missing Strip-Count and Compressed or with an invalid count.
    //
    //  Estimate the data size, Offset - FileSize
    //
    else if ((!m_pFile->GetMode().m_HasCreateAccess) &&
             (pi_CompressMode != COMPRESSION_NONE) &&
             (m_NbData32 == 1) && (!m_pCurDir->TagIsPresent(STRIPBYTECOUNTS) ||
                                   (GetCount(0) >= m_pFile->GetSize())) )
        {
        HDEBUGTEXT(L"SimulateStripList: Image with a missing Strip-Count and Compressed or Invalid Count\n");
        uint32_t* pDataCount32 = new uint32_t[1];
        HASSERT(pDataCount32 != 0);

        // The pDataCount32 will be delete by this method when needed
        SetOffsetCountData(1, 0, pDataCount32);

        SetCount(0, (m_pFile->GetSize() - GetOffset(0)));
        }

    // Image with a missing Strip-Count and UnCompressed
    //
    else if ((!m_pFile->GetMode().m_HasCreateAccess) &&
             (pi_CompressMode == COMPRESSION_NONE) &&
             (!m_pCurDir->TagIsPresent(STRIPBYTECOUNTS)) &&
             (m_NbData32 > 1) )
        {
        HDEBUGTEXT(L"SimulateStripList: Image with a missing Strip-Count and UnCompressed\n");
        uint32_t* pDataCount32 = new uint32_t[m_NbData32];
        HASSERT(pDataCount32 != 0);

        for(uint32_t i=0; i<m_NbData32; i++)
            pDataCount32[i]  = m_StripTileSize;

        // The pDataCount32 will be delete by this method when needed
        SetOffsetCountData(m_NbData32, 0, pDataCount32);
        }
    // Image compressed in one strip, Packbit compression, N>8
    //
    else if ((!m_pFile->GetMode().m_HasCreateAccess) &&
             (pi_CompressMode == COMPRESSION_PACKBITS) &&
             (m_BitsByPixel >= 8) &&
             (m_pCurDir->TagIsPresent(STRIPBYTECOUNTS)) &&
             (m_NbData32 == 1) &&
             (m_CurDir == 0) )
        {
        HDEBUGTEXT(L"SimulateLine-StripList: Image with one strip compress, simulate Offset/Count fields with empty value\n");

        // Set members
        m_RowsByStrip   = 1;
        m_NbData32      = m_ImageLength;
        m_StripTileSize = m_ImageWidth;

        // We don't modified the m_pDataOffset/m_pDataCount because the ReadData
        // will use it to read the data for the first time only and simulate line access after.
        m_SimulateLine_OneStripPackBitRGBA = true;
        }

    }

#if 0  // not use presently
void HTIFFFile::SimulateCountList(unsigned short pi_CompressMode)
    {
    HFCMonitor Monitor(m_Key);

    if (pi_CompressMode == COMPRESSION_NONE)
        {
        uint32_t RowBytes     = ScanlineSize();
        uint32_t RowsByStrip  = m_ImageLength / m_NbData;

        // Create new list
        //
        m_pDataCount = new uint32_t[m_NbData];
        HASSERT(m_pDataCount != 0);
        m_DataInfoAllocCount    = true;          // The list must be delete

        // Assign each entry in the Count list
        //
        m_pDataCount[0] = RowBytes*RowsByStrip;
        for(uint32_t i=1; i<m_NbData; i++)
            m_pDataCount[i]  = m_pDataCount[0];

        // Ajust the last strip size
        uint32_t OverLines = (m_NbData*m_RowsByStrip) - m_ImageLength;
        if (OverLines > 0)
            m_pDataCount[m_NbData-1] = (m_RowsByStrip-OverLines)*RowBytes;
        }
    }
#endif


//
// Normally use only with HMR file format implementation
// Needed to support old library HMR Dcartes...
// Because our library for optimization, don't write Data if the Datablock
// is empty.
void HTIFFFile::FillAllEmptyDataBlock()
    {
    // Alloc the Working buffer
    HAutoPtr<Byte> pBuffer(new Byte[m_StripTileSize]);
    memset (pBuffer, 0, m_StripTileSize);

    HTIFFFile::DirectoryID CurDir = CurrentDirectory();
    for (uint32_t Res=0; SetDirectoryImpl(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, Res), false); Res++)
        {
        for (uint32_t i=0; i<m_NbData32; i++)
            {
            if (GetOffset(i) == 0)
                WriteData (pBuffer, i, 0);
            }
        }

    SetDirectoryImpl(CurDir, false);
    }



//
// ReadData
// Read an uncompressed data block
HSTATUS HTIFFFile::ReadData (Byte* po_pData, uint32_t pi_StripTile)
    {
    HFCMonitor Monitor(m_Key);
    HSTATUS Ret = H_SUCCESS;

    if (pi_StripTile >= m_NbData32)
        {
        HTIFFError::BadBlockNbErInfo ErInfo;
        ErInfo.m_BlockNb = pi_StripTile;

        ErrorMsg(&m_pError, HTIFFError::BAD_BLOCK_NUMBER, &ErInfo, true);
        Ret = H_ERROR;
        goto WRAPUP;
        }

    // Data not allocated, should be a new file
    // Return a zero data
    if (GetOffset(pi_StripTile) == 0)
        {
        // Indicate the the tile is not available
        Ret = H_DATA_NOT_AVAILABLE;
        goto WRAPUP;
        }

    if (m_IsCompress)
        {
        // Image is Compressed
        // Allocate a packet to hold Compressed data
        HFCPtr <HCDPacket> pCompressedPacket = new HCDPacket();
        pCompressedPacket->SetBufferOwnership(true);


        // Read compressed Data
        Ret = ReadSeparateOrContigData (pCompressedPacket, pi_StripTile);

        if (Ret != H_SUCCESS)
            goto WRAPUP;

        // Allocate a Packet to hold uncompressed data
        HFCPtr <HCDPacket> pUnCompressedPacket = new HCDPacket(po_pData,
                                                               m_StripTileSize,
                                                               m_StripTileSize);

        // Uncompress Data
        pCompressedPacket->Decompress(pUnCompressedPacket);

        // Set an Identity codec
        pUnCompressedPacket->SetCodec(new HCDCodecIdentity());

        }
    else
        {
        // Image is uncompressed
        uint32_t PacketSize(GetCount(pi_StripTile));

        // Uncompressed tile/strip should always have the same size. So a packet size of
        // m_StripTileSize should be OK in all case but since we are close to a release we
        // will do specific case for the separate planar config to make sure we do not break
        // the contig tiff with the separate planar implementation.  I have also come across
        // a tiff file with different value(see tiif file in TR 111112)
        if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
            {
            PacketSize = m_StripTileSize;
            }

        // Fill a packet with uncompressed data
        HFCPtr <HCDPacket> pUnCompressedPacket = new HCDPacket(po_pData, PacketSize, PacketSize);

        // Read uncompressed Data
        Ret = ReadSeparateOrContigData (pUnCompressedPacket, pi_StripTile);
        }

    switch (m_pBitsBySample[0])
        {
        case 16:
            HASSERT(m_StripTileSize % 2 == 0);
            Treat16bitPerChannelForRead((unsigned short*)po_pData, m_StripTileSize/2);
            break;
        case 32:
            HASSERT(m_StripTileSize % 4 == 0);
            Treat32bitPerChannelForRead((uint32_t*)po_pData, m_StripTileSize/4);
            break;
        }

WRAPUP:
    return Ret;
    }


//
// WriteSeparateData
//
// If the parameter pi_pCompressSize != 0, then write the Data directly, it is compressed if
// necessary, the pi_pCompressSize contain the number of byte to write.
//
HSTATUS HTIFFFile::WriteSeparateData (const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize)
    {
    HPRECONDITION(IsAllSamplesWithSameBitsCount());
    HPRECONDITION(m_pBitsBySample[0] == 8 || m_pBitsBySample[0] == 16);
    HPRECONDITION(IsTiled() ? pi_StripTile <= NumberOfTiles() :pi_StripTile <= NumberOfStrips());
    HPRECONDITION(m_BitsByPixel % 8 == 0);
    HPRECONDITION(m_pBitsBySample[0] % 8 == 0);
    HPRECONDITION((pi_StripTile*m_SamplesByPixel) + (m_SamplesByPixel-1) < m_NbData32);
    HPRECONDITION(m_StripTileSize % m_NbSampleFromFile == 0);
    HPRECONDITION(!IsTiled() ? m_StripTileSize / m_NbSampleFromFile == m_RowsByStrip * m_ImageWidth * (m_pBitsBySample[0] / 8) : true);
    HPRECONDITION(!IsTiled() ? m_StripTileSize == m_RowsByStrip*m_ImageWidth*m_BitsByPixel / 8 : true); // x sample
    HPRECONDITION(!IsTiled() ? m_StripTileSize/m_SamplesByPixel == m_RowsByStrip * m_ImageWidth * m_pBitsBySample[0] / 8 : true);  // plane (one sample)
    HPRECONDITION(m_StripTileSize%m_SamplesByPixel == 0);
    HPRECONDITION(m_NbSampleFromFile == m_SamplesByPixel);
    HPRECONDITION(pi_pCompressSize == 0);

    if(m_PlanarConfig != PLANARCONFIG_SEPARATE || (m_IsCompress && pi_pCompressSize != 0))
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_SEPARATE_PLANE_ON_SEPARATE_CFG, 0, true);
        return H_ERROR;
        }

    HSTATUS Ret = H_SUCCESS;
    uint32_t StripsTilesCount(IsTiled() ? NumberOfTiles() : NumberOfStrips());
    uint32_t BytesPerPixel(m_BitsByPixel / 8);
    uint32_t BytesPerSample(m_pBitsBySample[0] / 8);
    uint32_t PlanesSize(m_StripTileSize/m_SamplesByPixel);       // Uncompressed plane(one sample)

    // Allocate a packet for the uncompressed planes
    HFCPtr<HCDPacket> pPlanePacket (new HCDPacket(new HCDCodecIdentity(), new Byte[PlanesSize], PlanesSize));
    pPlanePacket->SetBufferOwnership(true);
    pPlanePacket->SetDataSize(PlanesSize);

    // For each sample in the raster. In separate planar the stripTile in the file is equal to : NumberOfStrips()*SamplePerPixel.
    for(uint32_t SampleItr(0), CurrentStripTile(pi_StripTile); SampleItr < m_SamplesByPixel; ++SampleItr, CurrentStripTile+=StripsTilesCount)
        {
        if(1 == BytesPerSample)
            {
            // Position input buffer itr
            const Byte* pInputItr = pi_pData + (BytesPerSample*SampleItr);
            Byte* pPlaneBufferItr = pPlanePacket->GetBufferAddress();
            Byte* pPlaneBufferEnd = pPlanePacket->GetBufferAddress() + pPlanePacket->GetBufferSize();

            // Extract plane from input buffer
            while(pPlaneBufferItr < pPlaneBufferEnd)
                {
                // Copy one sample
                *pPlaneBufferItr = *pInputItr;

                ++pPlaneBufferItr;                  // Next sample
                pInputItr+=BytesPerPixel;           // Next Pixel
                }
            }
        else
            {
            HASSERT("Only 8 and 16 bits per sample is supported" && 2 == BytesPerSample);

            // Position input buffer itr
            const Byte* pInputItr = pi_pData + (BytesPerSample*SampleItr);
            unsigned short* pPlaneBufferItr = (unsigned short*)pPlanePacket->GetBufferAddress();
            unsigned short* pPlaneBufferEnd = (unsigned short*)(pPlanePacket->GetBufferAddress() + pPlanePacket->GetBufferSize());

            // Extract plane from input buffer
            while(pPlaneBufferItr < pPlaneBufferEnd)
                {
                // Copy one sample
                *pPlaneBufferItr = *(unsigned short*)pInputItr;

                ++pPlaneBufferItr;                  // Next sample
                pInputItr+=BytesPerPixel;           // Next Pixel
                }
            }

        // Write uncompress plane, will get compressed if required.
        if(H_SUCCESS != (Ret = WriteData(pPlanePacket->GetBufferAddress(), CurrentStripTile)))
            goto WRAPUP;
        }

WRAPUP:
    return Ret;
    }

//
// ReadSeparateOrContigData
//
HSTATUS HTIFFFile::ReadSeparateOrContigData (HFCPtr<HCDPacket>& po_pPacket, uint32_t pi_StripTile)
    {
    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
        {
        return ReadSeparateData(po_pPacket, pi_StripTile);
        }

    return ReadDataWithPacket(po_pPacket, pi_StripTile);
    }

//
// WriteSeparateOrContigData
//
HSTATUS HTIFFFile::WriteSeparateOrContigData(const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize)
    {
    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
        {
        return WriteSeparateData(pi_pData, pi_StripTile, pi_pCompressSize);
        }

    return WriteData(pi_pData, pi_StripTile, pi_pCompressSize);
    }

//
// WriteData
//
// If the parameter pi_pCompressSize != 0, then write the Data directly, it is compressed if
// necessary, the pi_pCompressSize contain the number of byte to write.
//
HSTATUS HTIFFFile::WriteData (const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize)
    {
    HFCMonitor Monitor(m_Key);
    HSTATUS Ret = H_SUCCESS;

    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    // If list of Strip\Tile not already allocated
    if (m_NbData32 == 0)
        InitStripList();

    uint32_t CompressSize = m_StripTileSize; // Set default Size
    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
        {
        HPRECONDITION(m_StripTileSize % m_NbSampleFromFile == 0);
        HPRECONDITION(!IsTiled() ? m_StripTileSize / m_NbSampleFromFile == m_RowsByStrip * m_ImageWidth * (m_pBitsBySample[0] / 8) : true);
        CompressSize = m_StripTileSize / m_NbSampleFromFile;
        }

    // TileId valid ?
    if (pi_StripTile >= m_NbData32)
        {
        if (m_UndoRedoFileMode)
            {
            // Undo/Redo file support size increase.
            HPRECONDITION(IsTiff64() == true);
            uint32_t NbData = m_NbData32 + 10000;

            SetField (IMAGELENGTH, NbData);         // ImageHeight == Nb strip in the file.

            HArrayAutoPtr<uint64_t> pData64(new uint64_t[NbData]);
            HASSERT(pData64 != 0);
            memset(pData64, 0, NbData*sizeof(uint64_t));
            BeStringUtilities::Memcpy(pData64, NbData*sizeof(uint64_t), m_pOffset64, m_NbData32*sizeof(uint64_t));
            SetField(STRIPOFFSETS, NbData, pData64);

            memset(pData64, 0, NbData*sizeof(uint64_t));
            BeStringUtilities::Memcpy(pData64, NbData*sizeof(uint64_t), m_pCount64, m_NbData32*sizeof(uint64_t));
            SetField(STRIPBYTECOUNTS, NbData, pData64);

            ReadOffsetCountTags();
            }
        else
            {
            HTIFFError::BadBlockNbErInfo ErInfo;
            ErInfo.m_BlockNb = pi_StripTile;

            ErrorMsg(&m_pError, HTIFFError::BAD_BLOCK_NUMBER, &ErInfo,true);
            Ret = H_ERROR;
            goto WRAPUP;
            }
        }

    // Data is compressed
    if (m_IsCompress && (pi_pCompressSize == 0))
        {
        // Swap 16 bit per canal before the compression starts
        HASSERT(16 == m_pBitsBySample[0] ? m_pPacket->GetCodec()->GetDataSize() % 2 == 0 : true);
        Treat16bitPerChannelForWrite((unsigned short*)pi_pData, m_pPacket->GetCodec()->GetDataSize()/2);

        // Alloc the Working buffer
        m_pPacket->SetDataSize(0);

        // Compress
        HASSERT_X64(m_pPacket->GetCodec()->GetDataSize() < ULONG_MAX);
        if((Ret = (this->*m_pCompressFunc)(pi_pData, (uint32_t)m_pPacket->GetCodec()->GetDataSize(), m_pPacket)) != H_SUCCESS)
            {
            HASSERT(0);
            }

        HASSERT_X64(m_pPacket->GetDataSize() < ULONG_MAX);
        CompressSize = (uint32_t)m_pPacket->GetDataSize();

        // Need to Bit Rev.
        if (m_ByteOrder.NeedBitRev())
            m_ByteOrder.ReverseBits(m_pPacket->GetBufferAddress(), m_pPacket->GetDataSize());

        // Change the buffer pointer to write
        pi_pData = m_pPacket->GetBufferAddress();
        }
    else
        {
        // Write a CompressData directly
        if (pi_pCompressSize != 0)
            {
            CompressSize = pi_pCompressSize;
            }
        else
            {
            // If the strips are simulated, we initialize the size with current value of the
            // strip size, because the last strip can be smaller than the others.
            if (m_StripAreSimulated)
                CompressSize = GetCount(pi_StripTile);
            }

        // Need to Bit Rev.
        if (m_ByteOrder.NeedBitRev())
            {
            // Alloc the Working buffer
            if(m_pPacket->GetBufferSize() < CompressSize)
                m_pPacket->SetBuffer(new Byte[CompressSize], CompressSize);

            memcpy (m_pPacket->GetBufferAddress(), pi_pData, CompressSize);

            m_ByteOrder.ReverseBits(m_pPacket->GetBufferAddress(), CompressSize);
            pi_pData = m_pPacket->GetBufferAddress();

            m_pPacket->SetDataSize(CompressSize);
            }

        // Swap 16 bit per canal here.  If the data is already compressed we can't swap the bytes == trouble.
        HASSERT((m_ByteOrder.NeedSwapByte() && 16 == m_pBitsBySample[0]) ? !m_IsCompress : true);
        HASSERT(16 == m_pBitsBySample[0] ? CompressSize % 2 == 0 : true);
        if(!m_IsCompress)
            {
            Treat16bitPerChannelForWrite((unsigned short*)pi_pData, CompressSize/2);
            }
        }


    // New Tile ? or Tile data changed.
    if ((GetOffset(pi_StripTile) == 0) || (GetCount(pi_StripTile) != CompressSize))
        {
        uint64_t Offset = GetOffset(pi_StripTile);
        m_pFile->CheckAlloc (&Offset, GetCount(pi_StripTile), CompressSize);
        SetOffset(pi_StripTile, Offset);

        // Touched the list of Offset\Count
        if (IsTiled())
            {
            m_pCurDir->Touched(TILEOFFSETS);
            m_pCurDir->Touched(TILEBYTECOUNTS);
            }
        else
            {
            m_pCurDir->Touched(STRIPOFFSETS);
            m_pCurDir->Touched(STRIPBYTECOUNTS);
            }
        }

    // Set Tile Size and Write it
    SetCount(pi_StripTile, CompressSize);
    if (!m_pFile->Seek (GetOffset(pi_StripTile), SEEK_SET) ||
        (m_pFile->Write (pi_pData, 1, GetCount(pi_StripTile)) != GetCount(pi_StripTile)))
        {
        HTIFFError::BlockIOErInfo ErInfo;
        ErInfo.m_BlockNb = pi_StripTile;
        ErInfo.m_Offset = GetOffset(pi_StripTile);
        ErInfo.m_Length = GetCount(pi_StripTile);

        if (dynamic_cast<HFCFileOutOfRangeException const*>(m_pFile->GetFilePtr()->GetLastException()) != 0)
            {
            ErrorMsg(&m_pError, HTIFFError::SIZE_OUT_OF_RANGE, 0, true);
            Ret = H_OUT_OF_RANGE;
            }
        else
            {
            ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_BLOCK, &ErInfo, true);
            Ret = H_WRITE_ERROR;
            }

        goto WRAPUP;
        }

    m_DataWasModified = true;

WRAPUP:
    CacheFileLock.ReleaseKey();

    return Ret;
    }

HSTATUS HTIFFFile::ReadSeparateData (HFCPtr<HCDPacket>& po_rpPacket, uint32_t pi_StripTile)
    {
    HPRECONDITION(IsAllSamplesWithSameBitsCount());
    HPRECONDITION(m_pBitsBySample[0] == 8 || m_pBitsBySample[0] == 16);
    HPRECONDITION(IsTiled() ? pi_StripTile <= NumberOfTiles() :pi_StripTile <= NumberOfStrips());
    HPRECONDITION(m_BitsByPixel % 8 == 0);
    HPRECONDITION(m_pBitsBySample[0] % 8 == 0);
    HPRECONDITION((pi_StripTile*m_SamplesByPixel) + (m_SamplesByPixel-1) < m_NbData32);
    HPRECONDITION(m_StripTileSize % m_NbSampleFromFile == 0);
    HPRECONDITION(!IsTiled() ? m_StripTileSize / m_NbSampleFromFile == m_RowsByStrip * m_ImageWidth * (m_pBitsBySample[0] / 8) : true);
    HPRECONDITION(!IsTiled() ? m_StripTileSize == m_RowsByStrip*m_ImageWidth*m_BitsByPixel / 8 : true); // x sample
    HPRECONDITION(!IsTiled() ? m_StripTileSize/m_SamplesByPixel == m_RowsByStrip * m_ImageWidth * m_pBitsBySample[0] / 8 : true); // plane (one sample)
    HPRECONDITION(m_StripTileSize%m_SamplesByPixel == 0);
    HPRECONDITION(m_NbSampleFromFile == m_SamplesByPixel);

    if(m_PlanarConfig != PLANARCONFIG_SEPARATE)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_SEPARATE_PLANE_ON_SEPARATE_CFG, 0, true);
        return H_ERROR;
        }

    HSTATUS Ret = H_SUCCESS;
    uint32_t StripsTilesCount(IsTiled() ? NumberOfTiles() : NumberOfStrips());
    uint32_t BytesPerPixel(m_BitsByPixel / 8);
    uint32_t BytesPerSample(m_pBitsBySample[0] / 8);
    uint32_t PlanesSize(m_StripTileSize/m_SamplesByPixel);   // Uncompressed plane(one sample)
    uint32_t DataSize(m_StripTileSize);                      // Uncompressed (x sample)
    uint32_t SampleItr(0);
    uint32_t CurrentStripTile(pi_StripTile);

    // Create a empty packet for the compressed planes
    HFCPtr<HCDPacket> pCompressPacket (new HCDPacket());
    pCompressPacket->SetBufferOwnership(true);

    // Allocate a packet for the uncompressed planes
    HFCPtr<HCDPacket> pPlanePacket (new HCDPacket(new HCDCodecIdentity(), new Byte[PlanesSize], PlanesSize));
    pPlanePacket->SetBufferOwnership(true);

    // Make sure output packet can hold the data
    if (po_rpPacket->GetBufferSize() < DataSize)
        {
        if (po_rpPacket->HasBufferOwnership())
            po_rpPacket->SetBuffer(new Byte[DataSize], DataSize);
        else
            {
            HASSERT(false);
            Ret = H_READ_ERROR;
            goto WRAPUP;
            }
        }

    // For each sample in the raster. In separate planar the stripTile in the file is equal to : NumberOfStrips()*SamplePerPixel.
    for(SampleItr = 0, CurrentStripTile = pi_StripTile; SampleItr < m_SamplesByPixel; ++SampleItr, CurrentStripTile+=StripsTilesCount)
        {
        if(m_IsCompress)
            {
            // Get plane. Will reuse pCompressPacket buffer if possible or allocate a new one.
            if(H_SUCCESS != (Ret = ReadDataWithPacket(pCompressPacket, CurrentStripTile)))
                goto WRAPUP;

            // Uncompress Data
            pCompressPacket->Decompress(pPlanePacket);
            pPlanePacket->SetCodec(new HCDCodecIdentity());
            }
        else
            {
            // Get plane
            if(H_SUCCESS != (Ret = ReadDataWithPacket(pPlanePacket, CurrentStripTile)))
                goto WRAPUP;
            }

        // uncompress here.
        HASSERT(pPlanePacket->GetCodec()->GetClassID() == HCDCodecIdentity::CLASS_ID);
        HASSERT(pPlanePacket->GetDataSize() <= PlanesSize);
        HASSERT(pPlanePacket->GetDataSize() % BytesPerSample == 0);

        if(1 == BytesPerSample)
            {
            // Position output buffer itr
            Byte* pOutputItr = po_rpPacket->GetBufferAddress() + (BytesPerSample*SampleItr);
            Byte* pPlaneBufferItr = pPlanePacket->GetBufferAddress();
            Byte* pPlaneBufferEnd = pPlanePacket->GetBufferAddress() + pPlanePacket->GetDataSize();

            // Copy current plane into the output buffer
            while(pPlaneBufferItr < pPlaneBufferEnd)
                {
                HASSERT(pOutputItr < po_rpPacket->GetBufferAddress() + po_rpPacket->GetBufferSize());

                *pOutputItr = *pPlaneBufferItr;

                ++pPlaneBufferItr;
                pOutputItr+=BytesPerPixel;
                }
            }
        else
            {
            HASSERT("Only 8 and 16 bits per sample is supported" && 2 == BytesPerSample);

            // Position output buffer itr
            Byte* pOutputItr = po_rpPacket->GetBufferAddress() + (BytesPerSample*SampleItr);
            unsigned short* pPlaneBufferItr = (unsigned short*)pPlanePacket->GetBufferAddress();
            unsigned short* pPlaneBufferEnd = (unsigned short*)(pPlanePacket->GetBufferAddress() + pPlanePacket->GetDataSize());

            // Copy current plane into the output buffer
            while(pPlaneBufferItr < pPlaneBufferEnd)
                {
                HASSERT(pOutputItr < po_rpPacket->GetBufferAddress() + po_rpPacket->GetBufferSize());

                *(unsigned short*)pOutputItr = *pPlaneBufferItr;

                ++pPlaneBufferItr;
                pOutputItr+=BytesPerPixel;
                }
            }
        }

    // Set output data size
    po_rpPacket->SetDataSize(DataSize);
    po_rpPacket->SetCodec(new HCDCodecIdentity(DataSize));

WRAPUP:
    return Ret;
    }


//
// ReadData with Packet
// Read compressed or uncompressed data
//
HSTATUS HTIFFFile::ReadDataWithPacket(HFCPtr<HCDPacket>& po_rpPacket, uint32_t pi_StripTile)
    {
    HPRECONDITION(pi_StripTile <= m_NbData32);

    HFCMonitor Monitor(m_Key);
    HSTATUS Ret = H_SUCCESS;

    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    uint32_t DataSize           = GetCount(pi_StripTile);
    uint32_t LineIndexTableSize = 0;

    if (pi_StripTile >= m_NbData32)
        {
        HTIFFError::BadBlockNbErInfo ErInfo;
        ErInfo.m_BlockNb = pi_StripTile;
        ErrorMsg(&m_pError, HTIFFError::BAD_BLOCK_NUMBER, &ErInfo, true);
        Ret = H_ERROR;
        goto WRAPUP;
        }

    if (GetOffset(pi_StripTile) == 0)
        {
        // Data not allocated, return an error
        Ret = H_DATA_NOT_AVAILABLE;
        goto WRAPUP;
        }


    if (m_IsCompress)
        {
        // Source data is compressed

        // Clone the current codec
        HFCPtr<HCDCodec> pCodec;
        pCodec = (HCDCodec*) m_pPacket->GetCodec()->Clone();

        if (m_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID)
            {
            // The codec is RLE1
            HFCPtr<HCDCodecHMRRLE1> pCodecRLE1 = (HFCPtr<HCDCodecHMRRLE1>&) pCodec;

            size_t CurrentHeight = pCodecRLE1->GetHeight();

            // Compute the line index table size and subtract it from the total data size
            if (!IsTiled() && pi_StripTile == NumberOfStrips()-1)
                {
                CurrentHeight = m_ImageLength - (NumberOfStrips()-1) * m_RowsByStrip;
                pCodecRLE1->SetDimensions(pCodecRLE1->GetWidth(), CurrentHeight);
                }

            LineIndexTableSize = static_cast<uint32_t>(pCodecRLE1->GetHeight() * sizeof(uint32_t));
            DataSize -= LineIndexTableSize;

            // Initialize codec
            pCodecRLE1->EnableLineIndexesTable(true);

            }

        if (m_pPacket->GetCodec()->GetClassID() == HCDCodecHMRPackBits::CLASS_ID)
            {
            // The codec is RLE1
            HFCPtr<HCDCodecHMRPackBits> pCodecPackbits = (HFCPtr<HCDCodecHMRPackBits>&) pCodec;

            if (!IsTiled() && pi_StripTile == NumberOfStrips()-1)
                {
                uint32_t CurrentHeight = m_ImageLength - (NumberOfStrips()-1) * m_RowsByStrip;
                pCodecPackbits->SetDimensions(pCodecPackbits->GetWidth(), CurrentHeight);
                }
            else if (IsTiled())
                {
                uint32_t TileWidth;
                uint32_t TileLength;
                if (m_pCurDir->GetValues (TILEWIDTH, &TileWidth) &&
                    m_pCurDir->GetValues (TILELENGTH, &TileLength))
                    {
                    uint32_t NbX = HowMany(m_ImageWidth, TileWidth);
                    uint32_t NbY = HowMany(m_ImageLength, TileLength);

                    if (pi_StripTile >= NbX*(NbY-1))
                        {
                        uint32_t CurrentHeight = m_ImageLength - (NbY-1) * TileLength;
                        pCodecPackbits->SetDimensions(pCodecPackbits->GetWidth(), CurrentHeight);
                        }
                    }
                }
            }

        // Initialize Packet
        po_rpPacket->SetCodec((HFCPtr<HCDCodec>) pCodec);

        // Verify if the packet is large enough to hold the compressed data
        HPRECONDITION(po_rpPacket->HasBufferOwnership() || (po_rpPacket->GetBufferSize() >= DataSize));
        if (po_rpPacket->GetBufferSize() < DataSize)
            {
            po_rpPacket->SetBuffer(new Byte[DataSize], DataSize);
            po_rpPacket->SetBufferOwnership(true);
            }

        // Fill the packet
        if (m_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID)
            {
            HFCPtr<HCDCodecHMRRLE1> pCodecRLE1 = (HFCPtr<HCDCodecHMRRLE1>&) po_rpPacket->GetCodec();

            // Read RLE1 Compressed data
            if ((!m_pFile->Seek (GetOffset(pi_StripTile), SEEK_SET))                                                  ||
                (m_pFile->Read ((Byte*)pCodecRLE1->GetLineIndexesTable(), 1, LineIndexTableSize) != LineIndexTableSize)||
                (m_pFile->Read ((Byte*)po_rpPacket->GetBufferAddress(),  1, DataSize)            != DataSize)            )
                {
                HTIFFError::BlockIOErInfo ErInfo;
                ErInfo.m_BlockNb = pi_StripTile;
                ErInfo.m_Offset = GetOffset(pi_StripTile);
                ErInfo.m_Length = DataSize;

                ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_BLOCK, &ErInfo, true);
                Ret = H_READ_ERROR;
                goto WRAPUP;
                }
            }
        else
            {
            // Read compressed data
            if ((!m_pFile->Seek (GetOffset(pi_StripTile), SEEK_SET) ||
                 (m_pFile->Read (po_rpPacket->GetBufferAddress(), 1, DataSize) != DataSize)))
                {
                HTIFFError::BlockIOErInfo ErInfo;
                ErInfo.m_BlockNb = pi_StripTile;
                ErInfo.m_Offset = GetOffset(pi_StripTile);
                ErInfo.m_Length = DataSize;

                ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_BLOCK, &ErInfo, true);
                Ret = H_READ_ERROR;
                goto WRAPUP;
                }

            }
        }
    else
        {
        // Source data is uncompressed

        // Verify if the packet is large enough to hold the uncompressed data
        if (po_rpPacket->GetBufferSize() < DataSize)
            {
            if (po_rpPacket->HasBufferOwnership())
                po_rpPacket->SetBuffer(new Byte[DataSize], DataSize);
            else
                {
                Ret = H_READ_ERROR;
                goto WRAPUP;
                }
            }

        // Set an Identity codec
        po_rpPacket->SetCodec(new HCDCodecIdentity());

        if ((!m_pFile->Seek (GetOffset(pi_StripTile), SEEK_SET) ||
             (m_pFile->Read (po_rpPacket->GetBufferAddress(), 1, DataSize) != DataSize)))
            {
            HTIFFError::BlockIOErInfo ErInfo;
            ErInfo.m_BlockNb = pi_StripTile;
            ErInfo.m_Offset = GetOffset(pi_StripTile);
            ErInfo.m_Length = DataSize;

            ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_BLOCK, &ErInfo, true);
            Ret = H_READ_ERROR;
            goto WRAPUP;
            }

        // Set packet data size
        po_rpPacket->SetDataSize(DataSize);
        }

    // Need to Bit Rev.
    if (m_ByteOrder.NeedBitRev())
        {
        if (m_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID)
            {
            m_ByteOrder.ReverseBits((Byte*)((HFCPtr<HCDCodecHMRRLE1>&) (po_rpPacket->GetCodec()))->GetLineIndexesTable(), LineIndexTableSize);
            m_ByteOrder.ReverseBits(po_rpPacket->GetBufferAddress(), DataSize);
            }
        else
            {
            m_ByteOrder.ReverseBits(po_rpPacket->GetBufferAddress(), DataSize);
            }
        }

    po_rpPacket->SetDataSize(DataSize);

WRAPUP:
    CacheFileLock.ReleaseKey();
    return Ret;
    }


void HTIFFFile::_PostReallocDirectories (uint32_t pi_NewDirCountCapacity)
    {
    // Do nothing
    }

void HTIFFFile::_PreWriteCurrentDir (HTagFile::DirectoryID pi_DirID)
    {
    // Do nothing
    }

bool HTIFFFile::PreCurrentDirectoryChanged (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    // Do nothing
    return true;
    }


bool HTIFFFile::OnCurrentDirectoryChanged (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    bool Ret = ValidateAndCorrectBlocInfo();

    //
    // Don't get standard TAG if HMR Directory
    //
    uint32_t DirNum = GetDirectoryNum(pi_DirID);
    HTIFFFile::DirectoryType DirType = GetDirectoryType(pi_DirID);
    if (Ret && DirType == HTIFFFile::STANDARD)
        {
        // Init many use Tag values.
        m_ImageWidth    = 0;
        m_ImageLength   = 0;
        m_pCurDir->GetValues (IMAGEWIDTH,       &m_ImageWidth);
        m_pCurDir->GetValues (IMAGELENGTH,      &m_ImageLength);

        m_SamplesByPixel   = 1;                //Default Value
        m_pCurDir->GetValues (SAMPLESPERPIXEL,  &m_SamplesByPixel);
        m_pBitsBySample    = 0;                //Default Value
        m_NbSampleFromFile = 1;
        m_BitsByPixel      = 0;
        if (!m_pCurDir->GetValues (BITSPERSAMPLE, &m_NbSampleFromFile, &m_pBitsBySample))
            {
            // Suppose 1bits by sample
            m_pBitsBySample = s_a1BitBySample;
            }

        // Tif file support 8 or 12 bits by sample
        // We consider that if the sample is >8 and < 16, the data is stored in a 16bits
        //
        if (m_pBitsBySample[0] > 8 && m_pBitsBySample[0] < 16)
            m_pBitsBySample = s_a16BitBySample;

        // Compute Bits by pixel
        if (m_NbSampleFromFile != m_SamplesByPixel)
            {
            if (m_NbSampleFromFile == 1)
                {
                // Same value by channel...
                m_BitsByPixel = (unsigned short)(m_pBitsBySample[0] * m_SamplesByPixel);
                }
            else
                HASSERT(false);
            }
        else
            {
            for (size_t i=0; i<m_SamplesByPixel; i++)    // Compute Bits by pixel
                m_BitsByPixel += m_pBitsBySample[i];
            }

        m_RowsByStrip      = 0;
        m_pCurDir->GetValues (ROWSPERSTRIP,     &m_RowsByStrip);
        if (m_RowsByStrip == (uint32_t)-1 || ((DirNum == 0) && (DirType == HTIFFFile::STANDARD) &&    // if Infinite or invalid value and directory==0(1:1)
                                            (m_RowsByStrip > m_ImageLength)) )                      // (TR 186499,189228), set to ImageHeight
            m_RowsByStrip = m_ImageLength;
        m_PlanarConfig     = PLANARCONFIG_CONTIG; //default
        m_pCurDir->GetValues (PLANARCONFIG,     &m_PlanarConfig);
        m_Photometric      = PHOTOMETRIC_RGB;
        m_pCurDir->GetValues (PHOTOMETRIC,      &m_Photometric);


        // Bit ordering
        //
        unsigned short FillOrder = FILLORDER_MSB2LSB;
        m_pCurDir->GetValues (FILLORDER,        &FillOrder);
        m_ByteOrder.SetBitRev(m_FillOrder != FillOrder);

        // Compression...
        unsigned short Compress = COMPRESSION_NONE;
        m_pCurDir->GetValues (COMPRESSION, &Compress);

        // Free the list allocated internally, else do nothing.
        FreeOffsetCount();

        // Load or Create Offset\Size for Data
        if (m_pCurDir->TagIsPresent(TILEOFFSETS))
            {
            ReadOffsetCountTags();

            m_StripTileSize = TileSizePrivate();
            }
        else if (m_pCurDir->TagIsPresent(STRIPOFFSETS))
            {
            ReadOffsetCountTags();

            // If RowByStrip absent, use the imageLenght by default
            if ((m_RowsByStrip == 0) && (m_NbData32 == 1))
                m_RowsByStrip = m_ImageLength;

            m_StripTileSize = StripSizePrivate();

            // used for images without compression
            SimulateStripList(Compress);
            }
        else
            {
            // Try to init strip list if not already done.
            m_NbData32 = 0;
            InitStripList();
            }

        // Compute the number of bit used for 16 bit per channel
        ComputeNbBitUsed();

        // Skip this setting if possible, take many times
        if (pi_ReloadCodec)
            {
            // Check compression
            //
            // These codes must be below the setting of m_StripTileSize, because it is
            // used in Set...Algo
            //
            // Set CallBack and Compress parameter.
            switch (Compress)
                {
                case COMPRESSION_CCITTRLE:
                case COMPRESSION_CCITTFAX3:
                case COMPRESSION_CCITTFAX4:
                case COMPRESSION_CCITTRLEW:
                    // Init CCITT compression, don't reverse the Bit
                    //
                    m_ByteOrder.SetBitRev(false);

                    SetCCITTAlgo (Compress, (m_FillOrder != FillOrder));
                    m_IsCompress = true;
                    break;

                case COMPRESSION_OJPEG:
                case COMPRESSION_JPEG:
                    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
                        {
                        // In separate we have only one sample by block/packet.
                        HASSERT(IsAllSamplesWithSameBitsCount());   // Will return SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED below
                        SetJPEGAlgo (m_pBitsBySample[0]);
                        }
                    else
                        SetJPEGAlgo (m_BitsByPixel);

                    m_IsCompress = true;
                    break;

#ifdef JBIG_SUPPORT
                case COMPRESSION_JBIG:
                    //TODO : SetJBIGAlgo ()
                    {
                    HFCMonitor Monitor(m_Key);

                    m_IsCompress = true;

                    // Get the block size
                    uint32_t BlockWidth    = 0;
                    uint32_t BlockHeight   = 0;

                    if (m_pCurDir->TagIsPresent(TILEOFFSETS))
                        {
                        m_pCurDir->GetValues (TILEWIDTH, &BlockWidth);
                        m_pCurDir->GetValues (TILELENGTH, &BlockHeight);
                        }
                    else
                        {
                        BlockWidth = m_ImageWidth;
                        m_pCurDir->GetValues (ROWSPERSTRIP, &BlockHeight);
                        }

                    m_pPacket->SetCodec(new HCDCodecJBIG(BlockWidth, BlockHeight));

                    m_pCompressFunc     = &HTIFFFile::CompressBlock;
                    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
                    m_pSetHeightFunc    = 0;
                    }
                break;
#endif

                case COMPRESSION_PACKBITS:
                    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
                        {
                        // In separate we have only one sample by block/packet.
                        HASSERT(IsAllSamplesWithSameBitsCount());   // Will return SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED below
                        SetPackBitsAlgo (m_pBitsBySample[0]);
                        }
                    else
                        SetPackBitsAlgo (m_BitsByPixel);
                    m_IsCompress = true;
                    break;

                case COMPRESSION_DEFLATE:
                case COMPRESSION_ADOBE_DEFLATE:
                    m_IsCompress = true;
                    SetDeflateAlgo();
                    break;

                case COMPRESSION_LZW:
                    m_IsCompress = true;

                    unsigned short Predictor;

                    if( !GetField(PREDICTOR, &Predictor) )
                        Predictor = 1;

                    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
                        {
                        // In separate we have only one sample by block/packet.
                        HASSERT(IsAllSamplesWithSameBitsCount());   // Will return SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED below
                        SetLZWAlgo(m_pBitsBySample[0], Predictor, m_SamplesByPixel);
                        }
                    else
                        SetLZWAlgo(m_BitsByPixel, Predictor, m_SamplesByPixel);

                    break;

                case COMPRESSION_NONE:
                    m_IsCompress = false;
                    SetNoneAlgo();
                    break;

                case COMPRESSION_HMR_FLASHPIX_OLD:
                    // Remove the old value
                    m_pCurDir->SetValues (COMPRESSION, (unsigned short)COMPRESSION_HMR_FLASHPIX);

                case COMPRESSION_HMR_FLASHPIX:
                    if(m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
                        {
                        // In separate we have only one sample by block/packet.
                        HASSERT(IsAllSamplesWithSameBitsCount());   // Will return SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED below
                        SetFlashpixAlgo(m_pBitsBySample[0]);
                        }
                    else
                        SetFlashpixAlgo(m_BitsByPixel);

                    m_IsCompress = true;
                    break;

                case COMPRESSION_HMR_RLE1_OLD:
                    // Remove the old value
                    m_pCurDir->SetValues (COMPRESSION, (unsigned short)COMPRESSION_HMR_RLE1);

                case COMPRESSION_HMR_RLE1:
                    SetRLE1Algo();
                    m_IsCompress = true;
                    break;
                default:
                    Ret             = false;
                    m_IsCompress    = false;
                    SetNoneAlgo();
                        {
                        HTIFFError::UnknownCompressionErInfo ErInfo;
                        ErInfo.m_CompressionType = Compress;
                        ErrorMsg(&m_pError, HTIFFError::UNKNOWN_COMPRESSION_TYPE, &ErInfo, true);
                        }
                    break;
                }
            }
        }

    return Ret;
    }

bool HTIFFFile::PostCurrentDirectorySet (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    bool Ret = true;

    // Validate supported separate planar raster
    if (m_PlanarConfig == PLANARCONFIG_SEPARATE && m_SamplesByPixel > 1)
        {
        // We support a maximum of 4 samples with 8 or 16 bits per sample
        if(m_SamplesByPixel > 4 || (m_pBitsBySample[0] != 8 && m_pBitsBySample[0] != 16) ||
           !IsAllSamplesWithSameBitsCount())
            {
            ErrorMsg(&m_pError,
                     HTIFFError::SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED,
                     0,
                     true);
            Ret = false;
            }
        }

    // We are using the Tiff file to implement our Undo/Redo file.
    // We identify the file with :
    if (m_ImageWidth == 0)
        {
        char* pStr;
        if (m_pCurDir->GetValues (SOFTWARE, &pStr) &&
            strcmp(pStr, IDENTIFY_UNDOREDO_FILE) == 0)
            m_UndoRedoFileMode = true;
        }

    return Ret;
    }



//-----------------------------------------------------------------------------
// public
// This methods prepares an image to be compressed with less artifact
// if the image is not a dimension of a multiple of 8 and the
//-----------------------------------------------------------------------------
void HTIFFFile::PrepareForJPEG(Byte* pio_pData,
                               uint32_t pi_Width,
                               uint32_t pi_Height,
                               uint32_t pi_RelevantWidth,
                               uint32_t pi_RelevantHeight,
                               uint32_t pi_BitsPerPixel)
    {
    HPRECONDITION(pio_pData != 0);
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);
    HPRECONDITION(pi_RelevantWidth > 0);
    HPRECONDITION(pi_RelevantHeight > 0);
    HPRECONDITION(pi_BitsPerPixel > 0);
    HPRECONDITION((pi_BitsPerPixel % 8) == 0);
    uint32_t BytesPerPixel = pi_BitsPerPixel / 8;
    uint32_t LineWidth = pi_Width * BytesPerPixel;

    // if the relevant width is smaller that the whole width, then
    // pad each line
    if (pi_RelevantWidth < pi_Width)
        {
        // compute the size of the padded width
        uint32_t PaddingWidth = MIN((8 - pi_RelevantWidth % 8), (pi_Width - pi_RelevantWidth));

        Byte* pSrc  = pio_pData + (pi_RelevantWidth - 1) * BytesPerPixel;
        Byte* pDest = pio_pData + pi_RelevantWidth * BytesPerPixel;
        uint32_t Pixel;
        for (uint32_t Y = 0; Y < pi_RelevantHeight; ++Y, pDest += LineWidth, pSrc += LineWidth)
            {
            for (Pixel = 0; Pixel < PaddingWidth; ++Pixel)
                HFCMemcpy(pDest + Pixel * BytesPerPixel, pSrc, BytesPerPixel);
            }
        }

    // if the relevant height is smaller that the whole height, then
    // pad the data so that the relevant height is a multiple of 8 in
    // order not to affect the last relevant information with the garbage
    // in the buffer.
    if (pi_RelevantHeight < pi_Height)
        {
        // compute the size of the padded height,
        uint32_t PaddingHeight = MIN((8 - pi_RelevantHeight % 8), (pi_Height - pi_RelevantHeight));

        Byte* pSrc  = pio_pData + (pi_RelevantHeight - 1) * LineWidth;
        Byte* pDest = pio_pData + pi_RelevantHeight * LineWidth;
        for (uint32_t Y = 0; Y < PaddingHeight; ++Y, pDest += LineWidth)
            HFCMemcpy(pDest, pSrc, LineWidth);
        }
    }



//-----------------------------------------------------------------------------
// private
// This methods handle the big endian/little endian problem and the nb bit used for
// 16 bit per channel.
//-----------------------------------------------------------------------------
void HTIFFFile::Treat16bitPerChannelForRead(unsigned short* pio_pData, size_t pi_DataCount) const
    {
    // Special treatment for 16 bit per canal
    // We don't want to modify 16 bit mono channel because it is normally used by DEM module.
    if (16 == m_pBitsBySample[0] && m_SamplesByPixel != 1)
        {
        HASSERT(IsAllSamplesWithSameBitsCount());

        // Code is duplicated for performance reasons
        if(m_NbBitUsed != 16 && m_ByteOrder.NeedSwapByte())
            {
            // FIRST swap and THEN scale
            for(uint32_t i(0); i < pi_DataCount; ++i)
                {
                pio_pData[i] = SWAPBYTE_SHORT(pio_pData[i]) << (16 - m_NbBitUsed);
                }
            }
        else if(m_NbBitUsed != 16)
            {
            // Scale only
            for(uint32_t i(0); i < pi_DataCount; ++i)
                {
                pio_pData[i] = pio_pData[i] << (16 - m_NbBitUsed);
                }
            }
        else if(m_ByteOrder.NeedSwapByte())
            {
            // Swap only
            SwabArrayOfShort(pio_pData, pi_DataCount);
            }
        }
    else if(16 == m_pBitsBySample[0] && m_ByteOrder.NeedSwapByte())
        {
        HASSERT(IsAllSamplesWithSameBitsCount());
        SwabArrayOfShort(pio_pData, pi_DataCount);
        }
    }

//-----------------------------------------------------------------------------
// private
// This methods handle the big endian/little endian problem and the nb bit used for
// 32 bit per channel.
//-----------------------------------------------------------------------------
void HTIFFFile::Treat32bitPerChannelForRead(uint32_t* pio_pData, size_t pi_DataCount) const
    {
    if (32 == m_pBitsBySample[0] && m_ByteOrder.NeedSwapByte())
        {
        HASSERT(IsAllSamplesWithSameBitsCount());
        SwabArrayOfLong(pio_pData, pi_DataCount);
        }
    }

//-----------------------------------------------------------------------------
// private
// This methods handle the big endian/little endian problem and the nb bit used for
// 16 bit per channel.
//-----------------------------------------------------------------------------
void HTIFFFile::Treat16bitPerChannelForWrite(unsigned short* pio_pData, size_t pi_DataCount) const
    {
    // Special treatment for 16 bit per canal
    // We don't want to modify 16 bit mono channel because it is normally used by DEM module.
    if (16 == m_pBitsBySample[0] && m_SamplesByPixel != 1)
        {
        HASSERT(IsAllSamplesWithSameBitsCount());

        // Code is duplicated for performance reasons
        if(m_NbBitUsed != 16  && m_ByteOrder.NeedSwapByte())
            {
            // FIRST scale and THEN swap
            for(uint32_t i(0); i < pi_DataCount; ++i)
                {
                pio_pData[i] = SWAPBYTE_SHORT(pio_pData[i] >> (16 - m_NbBitUsed));
                }
            }
        else if(m_NbBitUsed != 16)
            {
            // Scale only
            for(uint32_t i(0); i < pi_DataCount; ++i)
                {
                pio_pData[i] = pio_pData[i] >> (16 - m_NbBitUsed);
                }
            }
        else if(m_ByteOrder.NeedSwapByte())
            {
            // Swap only
            SwabArrayOfShort(pio_pData, pi_DataCount);
            }
        }
    else if(16 == m_pBitsBySample[0] && m_ByteOrder.NeedSwapByte())
        {
        HASSERT(IsAllSamplesWithSameBitsCount());
        SwabArrayOfShort(pio_pData, pi_DataCount);
        }
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
void HTIFFFile::SetInterpretMaxSampleValue(bool interpret)
    {
    m_InterpretMaxSampleValue = interpret;

    ComputeNbBitUsed();
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
void HTIFFFile::ComputeNbBitUsed()
    {
    unsigned short SampleFormat;

    //If the data are signed (possibly representing elevation measurements),
    //don't modify the data (see the functions Treat16bitPerChannelForRead,
    //Treat16bitPerChannelForWrite).
    if((16 == m_pBitsBySample[0]) &&
       ((GetField(SAMPLEFORMAT, &SampleFormat) == false) ||
        (SampleFormat != SAMPLEFORMAT_INT)))
        {
        // Use all bits by default
        m_NbBitUsed = 16;

        unsigned short MaxSampleValue;
        if(m_InterpretMaxSampleValue && GetField(MAXSAMPLEVALUE, &MaxSampleValue))
            {
            int RequiredBitPerPixel = (int)floor(log10( (double)(MaxSampleValue + 1)) / log10(2.0) + 0.9999);
            HASSERT((RequiredBitPerPixel <= 16) && (RequiredBitPerPixel > 0));
            m_NbBitUsed = (unsigned short)MAX (MIN( RequiredBitPerPixel, 16), 8);
            }
        }
    }

#if 0 //Disable
bool HTIFFFile::SetSynchroField()
    {
    bool Ret = true;
    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    // Check if we need to write in the file the offset used to implement the synchro
    // need by the Concurrence support.
    if ( ( m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess ) &&
         (m_pHMRDir != 0) &&
         !m_pHMRDir->TagIsPresent(HMR_SYNCHRONIZE_FIELD) )
        {
        // If not allready reserved, take a place at the end of the file.
        if (m_SynchroOffset == 0)
            {
            m_pFile->Seek(0, SEEK_END);
            m_SynchroOffset = m_pFile->Tell();

            // Synchro field(the data value is not important)
            if (m_pFile->Write(&m_SynchroOffset, sizeof(uint32_t), 1) != 1)
                Ret = false;
            }

        // Add the Tag
        m_pHMRDir->SetValues (HMR_SYNCHRONIZE_FIELD, m_SynchroOffset);
        WriteDirectories();
        }

    // Load the offset frime the file.
    if ((m_pHMRDir != 0) &&
        m_pHMRDir->TagIsPresent(HMR_SYNCHRONIZE_FIELD) )
        {
        m_pHMRDir->GetValues (HMR_SYNCHRONIZE_FIELD, &m_SynchroOffset);
        }

    CacheFileLock.ReleaseKey();

    return Ret;
    }
#endif


bool HTIFFFile::ValidateAndCorrectBlocInfo()
    {
    bool IsValid = true;

    if ((m_pCurDir->TagIsPresent(TILEWIDTH) || m_pCurDir->TagIsPresent(TILELENGTH)) &&
        (m_pCurDir->TagIsPresent(STRIPOFFSETS) && m_pCurDir->TagIsPresent(ROWSPERSTRIP)))
        {
        uint32_t NbBlocks;
        uint32_t TileWidth;
        uint32_t TileHeight;
        uint32_t RowsPerStrip;
        uint32_t ImageWidth;
        uint32_t ImageHeight;

        m_pCurDir->GetValues(IMAGEWIDTH, &ImageWidth);
        m_pCurDir->GetValues(IMAGELENGTH, &ImageHeight);
        m_pCurDir->GetValues(TILEWIDTH, &TileWidth);
        m_pCurDir->GetValues(TILELENGTH, &TileHeight);
        m_pCurDir->GetValues(ROWSPERSTRIP, &RowsPerStrip);

        if (IsTiff64())
            {
            uint64_t* pDummy = 0;
            m_pCurDir->GetValues(STRIPOFFSETS, &NbBlocks, &pDummy);
            }
        else
            {
            uint32_t* pDummy = 0;
            m_pCurDir->GetValues(STRIPOFFSETS, &NbBlocks, &pDummy);
            }

        uint32_t NbStrips = (uint32_t)ceil((double)ImageHeight / RowsPerStrip);

        if (NbBlocks != NbStrips)
            {
            uint32_t NbTiles = (uint32_t)(ceil((double)ImageWidth / TileWidth) *  ceil((double)ImageHeight / TileHeight));

            if (NbTiles == NbBlocks)
                {
                //Repair the directory
                uint32_t  NbValues;

                if (IsTiff64())
                    {
                    uint64_t* pData;

                    m_pCurDir->GetValues(STRIPBYTECOUNTS, &NbValues, &pData);
                    m_pCurDir->SetValues(TILEBYTECOUNTS, NbValues, pData);
                    m_pCurDir->GetValues(STRIPOFFSETS, &NbValues, &pData);
                    m_pCurDir->SetValues(TILEOFFSETS, NbValues, pData);
                    }
                else
                    {
                    uint32_t* pData;

                    m_pCurDir->GetValues (STRIPBYTECOUNTS, &NbValues, &pData);
                    m_pCurDir->SetValues (TILEBYTECOUNTS, NbValues, pData);
                    m_pCurDir->GetValues (STRIPOFFSETS, &NbValues, &pData);
                    m_pCurDir->SetValues (TILEOFFSETS, NbValues, pData);
                    }

                m_pCurDir->Remove(STRIPBYTECOUNTS);
                m_pCurDir->Remove(STRIPOFFSETS);
                }
            else
                {   //Maybe another imaginative misuse of TIFF tags.
                IsValid = false;
                }
            }
        }

    return IsValid;
    }


//-----------------------------------------------------------------------------
// public
// Utility function to create an Undo-Redo file
//   - Strip, LZW compression, increase dynamically
//-----------------------------------------------------------------------------
/*static*/ HTIFFFile* HTIFFFile::UndoRedoFile(const WString& pi_rFilename, HFCAccessMode pi_Mode)
    {
    HAutoPtr<HTIFFFile> pFile;
    try
        {
        bool NewFile=false;
        pFile = new HTIFFFile(pi_rFilename, pi_Mode, 0L, true, false, &NewFile);
        if (NewFile)
            {
            uint32_t ResolutionType = FILETYPE_EMPTYPAGE;
            if (!pFile->AppendDirectory())
                {
                HASSERT(false);
                }
            pFile->SetField (SUBFILETYPE, (uint32_t)ResolutionType);
            pFile->SetField (PLANARCONFIG, (unsigned short)PLANARCONFIG_CONTIG);
            pFile->SetFieldA (SOFTWARE, IDENTIFY_UNDOREDO_FILE);
                {
                char aDateTime[20];
                time_t timer;
                struct tm gm;

                time (&timer);
                gm = *localtime (&timer);

                sprintf ((char*)aDateTime, "%4d:%02d:%02d %02d:%02d:%02d",1900+gm.tm_year,
                         1+gm.tm_mon,
                         gm.tm_mday,
                         gm.tm_hour,
                         gm.tm_min,
                         gm.tm_sec);
                pFile->SetFieldA (DATETIME, aDateTime);
                }

            unsigned short BitPerSample = 8;
            pFile->SetField (BITSPERSAMPLE, (uint32_t) 1, &BitPerSample);
            pFile->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            pFile->SetField (SAMPLESPERPIXEL, (unsigned short)1);

            pFile->SetField (IMAGEWIDTH, (uint32_t)0);
            pFile->SetField (IMAGELENGTH, (uint32_t)3);
            pFile->SetField (ROWSPERSTRIP, (uint32_t)1);
            pFile->SetField (COMPRESSION, (unsigned short)COMPRESSION_DEFLATE);
            uint64_t DirOffset = 0;
            pFile->SetField (HMR2_IMAGEINFORMATION, 1, &DirOffset);
            pFile->SetDirectory(0);
            }
        else if(!pFile->IsValid())
            {
            pFile = 0;      // Error
            }
        }
    catch(...)
        {
        pFile = 0;
        }

    return pFile.release();
    }

//-----------------------------------------------------------------------------
// protected
// CompactITIFF()
//-----------------------------------------------------------------------------
bool HTIFFFile::CompactITIFF()
    {
    uint64_t BlockFreeRemoved        = 0;
    uint32_t PositionSortedData      = 0;
    uint32_t StartPositionSortedData = 0;

    //Build the freeblock list. This list will include the directories as a freeblock.
    SetFreeBlockList();

    if (m_FreeBlockDirectory.size() != 0)
        {
        //Build the list, once it's built, it's sorted
        SetTileData();

        size_t NumberDataInVector = m_MergedDirectories.size() - 1;

        //Do the same pattern for each block free, except the first one
        for (uint32_t Index = 0; Index < m_FreeBlockDirectory.size(); Index++)
            {
            //If there's two freeblock in a row we merge them.
            if (DataFreeBlockCanBeMerged(Index))
                {

                CompareDataWithBlockFree(m_FreeBlockDirectory[Index].Offset,
                                         PositionSortedData,
                                         StartPositionSortedData);
                if (Index != 0)
                    {
                    ReadWriteNewPosition(BlockFreeRemoved,
                                         PositionSortedData,
                                         StartPositionSortedData,
                                         m_FreeBlockDirectory[Index-1].Size);

                    //if there's only freeblocks after the last strip/tile
                    //so we can step to the next step.
                    if (PositionSortedData == (NumberDataInVector))
                        break;
                    }
                }
            }

        //If there's data after the last freeblock.
        if (PositionSortedData != NumberDataInVector)
            {
            ReadWriteNewPosition(BlockFreeRemoved,
                                 PositionSortedData,
                                 (uint32_t)NumberDataInVector,
                                 0);
            }

        SetDirectoryTouched();

        //Set the pointer in the file after the last strip
        //the directories will be written right after
        GetFilePtr()->SeekToPos(m_MergedDirectories[NumberDataInVector].Offset + m_MergedDirectories[NumberDataInVector].Size);
        m_pFile->GetFilePtr()->SetEOF();

        WriteDirectories();
        }

    return true;
    }

//-----------------------------------------------------------------------------
// private
// ReadWriteNewPosition()
//-----------------------------------------------------------------------------
void HTIFFFile::ReadWriteNewPosition(uint64_t& p_CountFreeBlockTotal,
                                     uint32_t p_PositionSortedData,
                                     uint32_t p_IteratorPositionSortedData,
                                     uint64_t p_CountFreeBlock)
    {
    uint64_t*   DataToMove;
    unsigned short IteratorLoopBigStrip  = 0;

    p_CountFreeBlockTotal          += p_CountFreeBlock;

    //Make sure that the last strip/tile will be checked
    if (p_PositionSortedData == (m_MergedDirectories.size() - 1))
        p_PositionSortedData++;

    //Move the Tile/Strip data in the file.
    for (p_IteratorPositionSortedData; p_IteratorPositionSortedData < p_PositionSortedData; p_IteratorPositionSortedData++)
        {
        uint64_t SumByteToMoved    = m_MergedDirectories[p_IteratorPositionSortedData].Size;
        uint64_t InitialPosition   = m_MergedDirectories[p_IteratorPositionSortedData].Offset;
        uint64_t NewPosition       = m_MergedDirectories[p_IteratorPositionSortedData].Offset - (uint64_t)p_CountFreeBlockTotal;

        while (SumByteToMoved != 0)
            {
            //Check if the data to be moved isn't > 128 mo
            if (SumByteToMoved > MAX_SUM_MOVE_DATA)
                {

                DataToMove = new uint64_t[MAX_SUM_MOVE_DATA];

                GetFilePtr()->SeekToPos(m_MergedDirectories[p_IteratorPositionSortedData].Offset +
                                        (IteratorLoopBigStrip * MAX_SUM_MOVE_DATA));
                GetFilePtr()->Read     (         DataToMove,
                                                 (size_t) MAX_SUM_MOVE_DATA);
                GetFilePtr()->SeekToPos(NewPosition + (IteratorLoopBigStrip * MAX_SUM_MOVE_DATA));
                GetFilePtr()->Write    (         DataToMove,
                                                 (size_t) MAX_SUM_MOVE_DATA);

                delete[] DataToMove;

                SumByteToMoved -= MAX_SUM_MOVE_DATA;
                IteratorLoopBigStrip++;
                }
            else
                {
                DataToMove  = new uint64_t[(size_t) SumByteToMoved];

                GetFilePtr()->SeekToPos(InitialPosition + (IteratorLoopBigStrip * MAX_SUM_MOVE_DATA));

                GetFilePtr()->Read     (         DataToMove,
                                                 (size_t) SumByteToMoved);

                GetFilePtr()->SeekToPos(NewPosition + (IteratorLoopBigStrip * MAX_SUM_MOVE_DATA));

                GetFilePtr()->Write    (         DataToMove,
                                                 (size_t) SumByteToMoved);

                this->SetDirectory     (m_MergedDirectories[p_IteratorPositionSortedData].Dir);

                //If the data is a tag we won't set the new offset the same way that if it's not a tag.
                if (!m_MergedDirectories[p_IteratorPositionSortedData].isATag)
                    {
                    //Set the new offset
                    SetOffset(m_MergedDirectories[p_IteratorPositionSortedData].Position, NewPosition);
                    }
                else
                    {
                    if (!m_MergedDirectories[p_IteratorPositionSortedData].isHMRDir)
                        m_ppListDir[m_MergedDirectories[p_IteratorPositionSortedData].Dir]->SetEntryOffset(m_MergedDirectories[p_IteratorPositionSortedData].Position, NewPosition);
                    else
                        m_ppListHMRDir64[0]->m_pDirectory->SetEntryOffset(m_MergedDirectories[p_IteratorPositionSortedData].Position, NewPosition);
                    }

                m_MergedDirectories[p_IteratorPositionSortedData].Offset = NewPosition;

                //Make sure we will break out the loop
                SumByteToMoved        = 0;

                IteratorLoopBigStrip  = 0;

                delete[] DataToMove;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// private
// SetTileData()
//-----------------------------------------------------------------------------
void HTIFFFile::SetTileData()
    {
    OffsetAndSize temp;

    //Get important infos to compress the file
    if (IsTiff64())
        {
        //Fill a vector with the data from all directories
        for (uint32_t Index = 0; Index < m_ListDirCount; Index++)
            {
            GetDataForEachTag(Index, false);

            this->SetDirectory(Index);

            for (uint32_t j=0; j < m_NbData32; j++)
                {
                temp.Offset   = m_pOffset64[j];
                temp.Size     = m_pCount64[j];
                temp.Dir      = Index;
                temp.Position = j;
                temp.isATag   = false;
                temp.isHMRDir = false;

                m_MergedDirectories.push_back(temp);
                }
            }
        }
    else
        {
        //Fill a vector with the data from all directories
        for (uint32_t Index = 0; Index < m_ListDirCount; Index++)
            {
            GetDataForEachTag(Index, false);

            this->SetDirectory(Index);

            for (uint32_t j=0; j < m_NbData32; j++)
                {
                temp.Offset   = (uint64_t)m_pOffset32[j];
                temp.Size     = (uint64_t)m_pCount32[j];
                temp.Dir      = Index;
                temp.Position = j;
                temp.isATag   = false;
                temp.isHMRDir = false;

                m_MergedDirectories.push_back(temp);
                }
            }
        }

    //Get the information for the HMR directory
    GetDataForEachTag(0, true);

    //sort the vector
    InsertSortMergedDirectories();
    }


//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
bool HTIFFFile::IsTiled(HTIFFDirectory* pi_Dir) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return ((pi_Dir->TagIsPresent(TILEWIDTH) || pi_Dir->TagIsPresent(TILELENGTH)) && !pi_Dir->TagIsPresent(STRIPOFFSETS));
    }


//-----------------------------------------------------------------------------
// private
// SetDirectoryTouched()
//-----------------------------------------------------------------------------
void HTIFFFile::SetDirectoryTouched()
    {
    m_ppListHMRDir64[0]->m_pDirectory->Touched(HMR_VERSION);
    m_ppListHMRDir64[0]->m_DirOffset64 = 0;

    if (IsTiled())
        {
        for (uint32_t Index = 0; Index < m_ListDirCount; Index++)
            {
            m_ppListDir[Index]->Touched(TILEBYTECOUNTS);
            m_ppListDir[Index]->Touched(TILEOFFSETS);

            m_pListDirOffset64[Index] = 0;
            }
        }
    else
        {
        for (uint32_t Index = 0; Index < m_ListDirCount; Index++)
            {
            m_ppListDir[Index]->Touched(STRIPBYTECOUNTS);
            m_ppListDir[Index]->Touched(STRIPOFFSETS);

            m_pListDirOffset64[Index] = 0;
            }
        }

    //Directory with the freeblocks, remove the freeblocks tags
    m_ppListDir[0]->Remove(FREEOFFSETS);
    m_ppListDir[0]->Remove(FREEBYTECOUNTS);
    }


// GetValues Methods
//
bool HTIFFFile::GetField (HTagID pi_Tag, unsigned short* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    switch (pi_Tag)
        {
    case BITSPERSAMPLE:
        *po_pVal = (unsigned short)m_pBitsBySample[0];
        break;
    case PHOTOMETRIC:
        *po_pVal = (unsigned short)m_Photometric;
        break;
    case SAMPLESPERPIXEL:
        *po_pVal = (unsigned short)m_SamplesByPixel;
        break;
    case PLANARCONFIG:
        *po_pVal = (unsigned short)m_PlanarConfig;
        break;

    default:
        return m_pCurDir->GetValues(pi_Tag, po_pVal);
        break;
        }

    return true;
    }


bool HTIFFFile::GetField (HTagID pi_Tag, uint32_t* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    switch (pi_Tag)
        {
    case IMAGEWIDTH:
        *po_pVal = m_ImageWidth;
        break;
    case IMAGELENGTH:
        *po_pVal = m_ImageLength;
        break;
    case ROWSPERSTRIP:
        *po_pVal = m_RowsByStrip;
        break;

    case COMPRESSION_QUALITY:   // Not saved in file
        *po_pVal = m_CompressionQuality;
        break;

    default:
        return m_pCurDir->GetValues(pi_Tag, po_pVal);
        break;
        }
    return true;
    }

bool HTIFFFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    switch (pi_Tag)
        {
    case BITSPERSAMPLE:
        if (m_pBitsBySample != 0)
            {
            *po_pCount  = m_NbSampleFromFile;
            *po_ppVal   = m_pBitsBySample;
            return true;
            }
        else
            return false;
        break;

    default:
        return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
        break;
        }
    }


bool HTIFFFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    bool Ret(true);

    if ((Ret = m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal)))
        {
        if (pi_Tag == BITSPERSAMPLE)
            m_pCurDir->GetValues (BITSPERSAMPLE, &pi_Count, &m_pBitsBySample);
        }

    return Ret;
    }

HTagID HTIFFFile::GetPacketOffsetsTagID      () const
    {
    return (IsTiled() ? TILEOFFSETS : STRIPOFFSETS);
    }

HTagID HTIFFFile::GetPacketByteCountsTagID   () const
    {
    return (IsTiled() ? TILEBYTECOUNTS : STRIPBYTECOUNTS);
    }


uint32_t HTIFFFile::_NumberOfDirectory(DirectoryType pi_DirType) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    if (IsSimulateLine_OneStripPackBitRGBAFile())
        return 1;   // chck GT
    else if (pi_DirType == STANDARD)
        return m_ListDirCount;
    else if (pi_DirType == HMR)
        return m_ListHMRDirCount;
    else
        {
        HASSERT(0);
        return 0;
        }
    }

uint32_t HTIFFFile::NumberOfPages() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    uint32_t PageCount = 0;
    uint32_t ImageType;
    for (uint32_t i = 0; i < m_ListDirCount; i++)
        {
        m_ppListDir[i]->GetValues (SUBFILETYPE, &ImageType);
        if ((ImageType & FILETYPE_PAGE) != 0)
            PageCount++;
        }

    return PageCount;
    }



uint32_t HTIFFFile::ComputeStrip (uint32_t pi_Line) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return (m_RowsByStrip == 0 ? 0 : (pi_Line / m_RowsByStrip));
    }

//
// NumberOfStrips
//
// In separate planar config it returns the number of strips like
// it was a contig planar because the separate planes are treated
// by the read/write operation.  In this case NumberOfStrips()
// is not equal to m_NbData.
uint32_t HTIFFFile::NumberOfStrips() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return (m_RowsByStrip == 0 ? (m_ImageLength != 0 ? 1 : 0) :
        HowMany(m_ImageLength, m_RowsByStrip));
    }


uint32_t HTIFFFile::ScanlineSize() const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);
    uint32_t Scanline;

    // PLANARCONFIG_SEPARATE implementation.
    // In separate mode we return the scanline size like a non-planar because
    // seperate data will be treated by ReadData(...)
    Scanline = m_BitsByPixel * m_ImageWidth;

    return (HowMany(Scanline, 8));
    }


bool HTIFFFile::IsTiled() const
    {
    return IsTiled(m_pCurDir);
    }


HFCPtr<HCDCodec> HTIFFFile::GetCurrentCodec () const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return m_pPacket->GetCodec();
    }

size_t HTIFFFile::StripBlockSize (uint32_t pi_Strip) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    if (pi_Strip < m_NbData32)
        return GetCount(pi_Strip);
    else
        return 0;
    }


size_t HTIFFFile::TileBlockSize (uint32_t pi_PosX, uint32_t pi_PosY) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);
    uint32_t TilePos = ComputeTile(pi_PosX, pi_PosY);

    return StripBlockSize(TilePos);
    }


HTIFFGeoKey& HTIFFFile::GetGeoKeyInterpretation()
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);

    return m_pCurDir->GetGeoKeyInterpretation();
    }


#if 0 // Disable
uint32_t HTIFFFile::GetSynchroOffsetInFile() const
    {
    return m_SynchroOffset;
    }
#endif


bool HTIFFFile::GetConvertedField (HTagID pi_Tag, vector<double>& po_rValues) const
    {
    HFCMonitor Monitor(const_cast<HTIFFFile*>(this)->m_Key);
    return m_pCurDir->GetConvertedValues(pi_Tag, po_rValues);
    }

bool HTIFFFile::IsSimulateLine_OneStripPackBitRGBAFile() const
    {
    return m_SimulateLine_OneStripPackBitRGBA;
    }