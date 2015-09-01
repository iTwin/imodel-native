//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGdalSupportedFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGdalSupportedFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/ImageppLib.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

#include <Imagepp/all/h/HRFGdalSupportedFile.h>
#include <Imagepp/all/h/HRFGdalSupportedFileEditor.h>
#include <Imagepp/all/h/HRFGdalUtilities.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFDoqEditor.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>


#include <Imagepp/all/h/ImagePPMessages.xliff.h>


#include <Imagepp/all/h/HCPGeoTiffKeys.h>

//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
//#include <ImagePP-GdalLib/cpl_string.h>
#include <ImagePP-GdalLib/cpl_error.h>



//-----------------------------------------------------------------------------
// ImagePP's own error handler for GDAL
//-----------------------------------------------------------------------------
void CPL_STDCALL GDALErrorHandler      (CPLErr          eErrClass,
                                        int32_t          nError,
                                        const char*    pszErrorMsg)
    {
    // Do nothing for the moment
    }


class HRFGDALLibInitializator
    {
private:
    void SetPathToGdalData()
        {
        CPLCleanFinderLocation();
        WString rPathToGdalData;
        if (BSISUCCESS == ImageppLib::GetHost().GetImageppLibAdmin()._GetGDalDataPath(rPathToGdalData))
            {
            // TFS#86887: GDAL_FILENAME_IS_UTF8
            Utf8String localeChars;
            BeStringUtilities::WCharToUtf8(localeChars, rPathToGdalData.c_str());

            CPLPushFinderLocation(localeChars.c_str());
            //The path should exists, otherwise, there is a problem with the
            //build/installation procedure.
            BeAssert(!rPathToGdalData.empty() && BeFileName::IsDirectory(rPathToGdalData.c_str()));
            }
        }

public :

    HRFGDALLibInitializator()
        {
        // Register our own error handler
        CPLSetErrorHandler(GDALErrorHandler);

        //Register only the driver we uses.
        GDALAllRegister();

        SetPathToGdalData();
        }


    ~HRFGDALLibInitializator()
        {
        GDALDestroyDriverManager();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRFGdalSupportedFile::Initialize()
    {
    static HRFGDALLibInitializator s_GdalLibInitializator;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFGdalSupportedFile::~HRFGdalSupportedFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.

    try
        {
        if ((m_IsOpen == true) &&
            (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess))
            {
            Save();
            }
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }

    //Close the file
    if (m_poDataset != 0)
        {
        int Ref = m_poDataset->Dereference();

        if (Ref <= 0)
            {
            GDALClose(m_poDataset);
            m_poDataset = 0;
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// Save
//
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::Save()
    {
    if (m_IsOpen &&
        (m_ListOfPageDescriptor.size() > 0) &&
        ((GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess)))
        {
        bool   SaveHeader = false;

        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

        // Lock the sister file for the SetPalette operation
        HFCLockMonitor SisterFileLock(GetLockManager());

        if (m_DisplayRep == PALETTE && pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
            {
            WriteColorTable();
            }

        if ((m_poDataset != 0) && pPageDescriptor->GeocodingHasChanged())
            {
            SetGeocodingInformation();
            SaveHeader = true;
            }

        // Update the TransfoModel
        if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
            {
            SetGeoRefMatrix();
            SaveHeader = true;
            }

        if (SaveHeader == true)
            {
            m_poDataset->FlushCache();
            }

        pPageDescriptor->Saved();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    }


//-----------------------------------------------------------------------------
// protected
// WriteColorTable Write the color table in the dataset
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::WriteColorTable(uint32_t              pi_Page,
                                           unsigned short       pi_Resolution)
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    Byte* pValue = 0;
    GDALColorEntry colorEntry;
    GDALColorTable colorTable;

    HRPPixelType* pPagePixelType = GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetPixelType();

    switch (pPagePixelType->GetPalette().GetPixelEntrySize())
        {
        case 1:
            for(size_t index = 0; index < pPagePixelType->GetPalette().GetMaxEntries(); index++)
                {
                Byte defaultValue[4];
                if (index < pPagePixelType->GetPalette().CountUsedEntries())
                    {
                    pValue = (Byte*)pPagePixelType->GetPalette().GetCompositeValue((uint32_t)index);
                    }
                else
                    {
                    defaultValue[0] = (Byte)index;
                    defaultValue[1] = (Byte)index;
                    defaultValue[2] = (Byte)index;
                    defaultValue[3] = 255;
                    pValue = (Byte*)&defaultValue;
                    }
                colorEntry.c1 = pValue[0];
                colorEntry.c2 = 0;
                colorEntry.c3 = 0;
                colorEntry.c4 = 255;
                colorTable.SetColorEntry((uint32_t)index, &colorEntry);
                }
            break;
        case 3:
            for(size_t index = 0; index < pPagePixelType->GetPalette().GetMaxEntries(); index++)
                {
                Byte defaultValue[4];
                if (index < pPagePixelType->GetPalette().CountUsedEntries())
                    {
                    pValue = (Byte*)pPagePixelType->GetPalette().GetCompositeValue((uint32_t)index);
                    }
                else
                    {
                    defaultValue[0] = (Byte)index;
                    defaultValue[1] = (Byte)index;
                    defaultValue[2] = (Byte)index;
                    defaultValue[3] = 255;
                    pValue = (Byte*)&defaultValue;
                    }
                colorEntry.c1 = pValue[0];
                colorEntry.c2 = pValue[1];
                colorEntry.c3 = pValue[2];
                colorEntry.c4 = 255;
                colorTable.SetColorEntry((uint32_t)index, &colorEntry);
                }
            break;
        case 4:
            for(size_t index = 0; index < pPagePixelType->GetPalette().GetMaxEntries(); index++)
                {
                Byte defaultValue[4];
                if (index < pPagePixelType->GetPalette().CountUsedEntries())
                    {
                    pValue = (Byte*)pPagePixelType->GetPalette().GetCompositeValue((uint32_t)index);
                    }
                else
                    {
                    defaultValue[0] = (Byte)index;
                    defaultValue[1] = (Byte)index;
                    defaultValue[2] = (Byte)index;
                    defaultValue[3] = 255;
                    pValue = (Byte*)&defaultValue;
                    }
                colorEntry.c1 = pValue[0];
                colorEntry.c2 = pValue[1];
                colorEntry.c3 = pValue[2];
                colorEntry.c4 = pValue[3];
                colorTable.SetColorEntry((uint32_t)index, &colorEntry);
                }
            break;
        default :
            HASSERT(0);
        }
    GetRasterBand(m_PaletteBandInd)->SetColorTable(&colorTable);
    }


//-----------------------------------------------------------------------------
// Protected
// OpenWithGDAL
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::Open()
    {
    HPRECONDITION(m_pGdalDriver != NULL);
    HPRECONDITION(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));
    
    if (m_IsOpen)
        return m_IsOpen;
      
    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());
    GDALAccess     GdalAccess;

    if ((GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess))
        {
        GdalAccess = GA_Update;
        }
    else
        {
        GdalAccess = GA_ReadOnly;
        }

    try
        {
        // TFS#86887: GDAL_FILENAME_IS_UTF8
        Utf8String filenameUtf8;
        BeStringUtilities::WCharToUtf8(filenameUtf8, static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName().c_str());

        // Only allowed the expected driver to avoid hostile file format that might be registered.
        const char * const allowedDrivers[] = {GDALGetDriverShortName(m_pGdalDriver), NULL};       
        CHECK_ERR(m_poDataset = (GDALDataset*) GDALOpenInternal(filenameUtf8.c_str(), GdalAccess, allowedDrivers);)

        if (m_poDataset == 0)
            {
            ThrowExBasedOnGDALErrorCode(CPLGetLastErrorNo());
            }

        DetectPixelType();

        if (m_pPixelType == 0)
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
            
        if (AreDataNeedToBeScaled() && (GdalAccess == GA_Update))
            {
            throw HRFDataHaveBeenScaledReadOnlyException(GetURL()->GetURL());
            }

        m_IsOpen = true;
        }
    catch (HFCException& rException)
        {
        // If the exception is not specific enough throw an exception that represents the current context
        if (dynamic_cast<HFCUndefinedException*>(&rException) != 0)
            throw HFCCannotOpenFileException(GetURL()->GetURL());
        else
            throw;
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// DetectOptimalBlockAccess
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectOptimalBlockAccess()
    {
    HPRECONDITION(m_NbBands > 0);
    HPRECONDITION(m_poDataset != 0);

    //It is assumed that each band has the same optimal access type
    CHECK_ERR(m_poDataset->GetRasterBand(1)->GetBlockSize(&m_BlockWidth, &m_BlockHeight);)
    }

//-----------------------------------------------------------------------------
// Private
// DetectPixelType
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectPixelType()
    {
    //Multiband - Use the most visible light bands
    bool        isDisplayableBandFound = false;

    GDALColorInterp bandColorType;

    m_NbBands = m_poDataset->GetRasterCount();

    for (int bandInd = 1; bandInd <= m_NbBands; bandInd++)
        {
        bandColorType = m_poDataset->GetRasterBand(bandInd)->GetColorInterpretation();

        switch (bandColorType)
            {
            case GCI_GrayIndex :
                if (m_GrayBandInd == -1)
                    {
                    m_GrayBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_PaletteIndex :
                if (m_PaletteBandInd == -1)
                    {
                    m_PaletteBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                else
                    {   //There should only be one palette band. If more, let specific handled file
                    //code work it out.
                    isDisplayableBandFound = false;
                    }
                break;
            case GCI_RedBand :
                if (m_RedBandInd == -1)
                    {
                    m_RedBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_GreenBand :
                if (m_GreenBandInd == -1)
                    {
                    m_GreenBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_BlueBand :
                if (m_BlueBandInd == -1)
                    {
                    m_BlueBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_AlphaBand :
                if (m_AlphaBandInd = -1)
                    {
                    m_AlphaBandInd = (signed char)bandInd;
                    }
                break;
            case GCI_YCbCr_YBand :
                if (m_YbandInd == -1)
                    {
                    m_YbandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_YCbCr_CbBand :
                if (m_CbBandInd == -1)
                    {
                    m_CbBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            case GCI_YCbCr_CrBand :
                if (m_CrBandInd == -1)
                    {
                    m_CrBandInd = (signed char)bandInd;
                    isDisplayableBandFound = true;
                    }
                break;
            }
        }

    if (!isDisplayableBandFound)
        HandleNoDisplayBands();

    //Default display preference. Should be superseded by any preference define by a particular file format.
    vector<DISPLAY_REP> DefaultDispPref;
    DefaultDispPref.push_back(RGB);
    DefaultDispPref.push_back(MONO);
    DefaultDispPref.push_back(PALETTE);
    DefaultDispPref.push_back(YCC);

    vector<DISPLAY_REP>::iterator DispPrefIter = DefaultDispPref.begin();
    vector<DISPLAY_REP>::iterator DispPrefIterEnd = DefaultDispPref.end();

    m_DisplayRep = UNDEFINED;

    while ((DispPrefIter != DispPrefIterEnd) && (m_DisplayRep == UNDEFINED))
        {
        switch (*DispPrefIter)
            {
            case MONO :
                if (m_GrayBandInd != -1)
                    {
                    DetectPixelTypeMono();
                    m_DisplayRep = MONO;
                    }
                break;

            case RGB :
                if ((m_RedBandInd != -1) && (m_GreenBandInd != -1) && (m_BlueBandInd != -1))
                    {
                    DetectPixelTypeRgb();
                    m_DisplayRep = RGB;
                    }
                break;

            case YCC :
                if ((m_YbandInd != -1) && (m_CbBandInd != -1) && (m_CrBandInd != -1))
                    {
                    DetectPixelTypeYCbCr();
                    m_DisplayRep = YCC;
                    }
                break;

            case PALETTE :
                if (m_PaletteBandInd != -1)
                    {
                    DetectPixelTypePalette();
                    m_DisplayRep = PALETTE;
                    }
                break;
            }
        DispPrefIter++;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// AreDataNeedToBeScaled
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::AreDataNeedToBeScaled()
    {
    return IsUnsignedPixelTypeForSignedData();
    }

//-----------------------------------------------------------------------------
// Protected
// DetectPixelTypeMono
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectPixelTypeMono()
    {
    HPRECONDITION(m_GrayBandInd != -1);
    m_NbBands = 1;

    switch (GetRasterBand(m_GrayBandInd)->GetRasterDataType())
        {
        case GDT_Byte :
            m_pPixelType = new HRPPixelTypeV8Gray8();
            m_BitsPerPixelPerBand = 8;
            m_Signed = false;
            m_IsGrayScale = true;
            break;
        case GDT_UInt16 :
            m_pPixelType = new HRPPixelTypeV16Gray16(); //in use
            m_BitsPerPixelPerBand = 16;
            m_Signed = false;
            m_IsGrayScale = true;
            break;
        case GDT_Int16 :
            {
            int Success;
            double NoDataValue = GetRasterBand(m_GrayBandInd)->GetNoDataValue(&Success);

            if (Success != false)
                {
                HASSERT((NoDataValue >= SHRT_MIN) && (NoDataValue <= SHRT_MAX));
                int16_t FittedNoDataValue = static_cast<int16_t>(NoDataValue);
                m_pPixelType = new HRPPixelTypeV16Int16(GetBandRole(m_GrayBandInd),
                                                        &FittedNoDataValue);
                }
            else
                {
                m_pPixelType = new HRPPixelTypeV16Int16(GetBandRole(m_GrayBandInd));
                }

            m_BitsPerPixelPerBand = 16;
            m_Signed = true;
            m_IsGrayScale = true;
            }
        break;
        case GDT_Float32 :
            {
            int Success;
            double NoDataValue = GetRasterBand(m_GrayBandInd)->GetNoDataValue(&Success);

            if (Success != false)
                {
                HASSERT((NoDataValue >= (-FLT_MAX)) && (NoDataValue <= FLT_MAX));
                float FittedNoDataValue = static_cast<float>(NoDataValue);
                m_pPixelType = new HRPPixelTypeV32Float32(GetBandRole(m_GrayBandInd),
                                                          &FittedNoDataValue);
                }
            else
                {
                m_pPixelType = new HRPPixelTypeV32Float32(GetBandRole(m_GrayBandInd));
                }
            }

        m_BitsPerPixelPerBand = 32;
        m_Signed = true;
        m_IsReadPixelReal = true;
        m_IsGrayScale = true;
        break;
        case(GDT_Float64):
            m_IsReadPixelReal = true;
            m_pPixelType = new HRPPixelTypeV16Gray16();
            m_BitsPerPixelPerBand = 16;
            m_Signed = true;
            m_IsGrayScale = true;
            m_IsUPixelTypeForSignedData = true;
            m_IsIntegerPixelTypeForRealData = true;
            break;
        default :
            //not implemented
            HASSERT(false);
            break;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// DetectPixelTypeRgb
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectPixelTypeRgb()
    {
    HPRECONDITION((m_RedBandInd != -1) && (m_GreenBandInd != -1) && (m_BlueBandInd != -1));

    GDALRasterBand* pRedBand = GetRasterBand(m_RedBandInd);
    GDALRasterBand* pGreenBand = GetRasterBand(m_GreenBandInd);
    GDALRasterBand* pBlueBand = GetRasterBand(m_BlueBandInd);

    HPRECONDITION((pGreenBand->GetRasterDataType() == pRedBand->GetRasterDataType()) &&
                  (pBlueBand->GetRasterDataType() == pRedBand->GetRasterDataType()));

    if ((m_AlphaBandInd != -1) || (m_ExtendedBandInd != -1))
        {
        m_NbBands = 4;
        }
    else
        {
        m_NbBands = 3;
        }

    GDALDataType DataType = pRedBand->GetRasterDataType();

    switch (DataType)
        {
        case GDT_Byte :
            if (m_AlphaBandInd != -1)
                {
                m_pPixelType = new HRPPixelTypeV32R8G8B8A8();
                }
            else
                {
                m_pPixelType = new HRPPixelTypeV24R8G8B8();
                }
            m_BitsPerPixelPerBand = 8;
            m_Signed = false;
            break;
        case GDT_UInt16 :
        case GDT_Int16 :
            if (m_AlphaBandInd != -1)
                {
                m_pPixelType = new HRPPixelTypeV64R16G16B16A16(); //in use
                }
            else if (m_ExtendedBandInd != -1)
                {
                m_pPixelType = new HRPPixelTypeV64R16G16B16X16(); //in use
                }
            else
                {
                m_pPixelType = new HRPPixelTypeV48R16G16B16(); //in use
                }

            m_BitsPerPixelPerBand = 16;

            if (DataType == GDT_Int16)
                {
                m_Signed = true;
                m_IsUPixelTypeForSignedData = true;
                }
            else
                {
                m_Signed = false;
                }
            break;
        case GDT_UInt32 :
        case GDT_Int32 :
            //Warning : Lost of alpha information
            HASSERT(m_AlphaBandInd == -1);

            m_pPixelType = new HRPPixelTypeV96R32G32B32(); //in use
            m_BitsPerPixelPerBand = 32;

            if (DataType == GDT_Int32)
                {
                m_Signed = true;
                m_IsUPixelTypeForSignedData = true;
                }
            else
                {
                m_Signed = false;
                }
            break;
        case GDT_Float32 :
            //Warning : Lost of alpha information
            HASSERT(m_AlphaBandInd == -1);

            m_pPixelType = new HRPPixelTypeV96R32G32B32(); //in use
            m_BitsPerPixelPerBand = 32;
            m_IsReadPixelReal = true;
            m_Signed = true;
            m_IsUPixelTypeForSignedData = true;
            m_IsIntegerPixelTypeForRealData = true;
            break;
        default :
            //not implemented
            HASSERT(false);
            break;
        }
    }


//-----------------------------------------------------------------------------
// Protected
// DetectPixelTypePalette
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectPixelTypePalette()
    {
    CHECK_ERR(GDALColorTable* pColorTable = m_poDataset->GetRasterBand(m_PaletteBandInd)->GetColorTable();)

    //IMPORTANT - There is a function called GetColorEntryAsRGB which was supposed to convert on-the-fly
    //any palette interpretation to RGB, but which is not already implemented.
    if (((pColorTable->GetPaletteInterpretation() == GPI_RGB) || (pColorTable->GetPaletteInterpretation() == GPI_Gray)) &&
        (m_poDataset->GetRasterBand(m_PaletteBandInd)->GetRasterDataType() == GDT_Byte) &&
        (pColorTable->GetColorEntryCount() <= 256))
        {
        bool IsRgbPaletteIN = pColorTable->GetPaletteInterpretation() == GPI_RGB;

        m_BitsPerPixelPerBand = 8;
        m_Signed = false;

        if (IsRgbPaletteIN == true)
            {
            m_pPixelType = new HRPPixelTypeI8R8G8B8();
            }
        else
            {
            m_pPixelType = new HRPPixelTypeI8Gray8();
            }

        if (m_pPixelType != 0)
            {
            HRPPixelPalette*    pPalette = 0;
            GDALColorTable*     pColorTable = 0;
            HAutoPtr<Byte>     pValue;

            pPalette = (HRPPixelPalette*)&(m_pPixelType->LockPalette());
            pValue = new Byte[4];
            CHECK_ERR(pColorTable = m_poDataset->GetRasterBand(m_PaletteBandInd)->GetColorTable();)
            HASSERT(pColorTable != 0);

            // Fill in the palette for the newly created pixel type
            if (IsRgbPaletteIN == true)
                {
                for (int32_t i = 0; i < pColorTable->GetColorEntryCount(); i++)
                    {
                    pValue[0] = (Byte)pColorTable->GetColorEntry(i)->c1;
                    pValue[1] = (Byte)pColorTable->GetColorEntry(i)->c2;
                    pValue[2] = (Byte)pColorTable->GetColorEntry(i)->c3;
                    //  pValue[3] = (Byte)pColorTable->GetColorEntry(i)->c4;

                    // Add the entry to the pixel palette
                    pPalette->SetCompositeValue(i, pValue);
                    }
                }
            else
                {
                //Gray scale palette is converted to RGB palette
                for (int32_t i = 0; i < pColorTable->GetColorEntryCount(); i++)
                    {
                    pValue[0] = (Byte)pColorTable->GetColorEntry(i)->c1;

                    // Add the entry to the pixel palette
                    pPalette->SetCompositeValue(i, pValue);
                    }
                }

            m_pPixelType->UnlockPalette();
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// DetectPixelTypeYCbCr
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::DetectPixelTypeYCbCr()
    {
    HPRECONDITION((m_YbandInd != -1) && (m_CbBandInd != -1) && (m_CrBandInd != -1));

    GDALRasterBand* pYBand = GetRasterBand(m_YbandInd);
    GDALRasterBand* pCbBand = GetRasterBand(m_CbBandInd);
    GDALRasterBand* pCrBand = GetRasterBand(m_CrBandInd);

    HPRECONDITION((pCbBand->GetRasterDataType() == pYBand->GetRasterDataType()) &&
                  (pCrBand->GetRasterDataType() == pYBand->GetRasterDataType()));

    m_NbBands = 3;

    //The only data type currently supported
    if (pYBand->GetRasterDataType() == GDT_Byte)
        {
        m_pPixelType = new HRPPixelTypeV24PhotoYCC();
        m_BitsPerPixelPerBand = 8;
        m_Signed = false;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// GetMinimumPossibleValue
//-----------------------------------------------------------------------------
double HRFGdalSupportedFile::GetMinimumPossibleValue(unsigned short pi_DataType)
    {
    double MinVal = (-LDBL_MAX);

    switch (pi_DataType)
        {
        case GDT_Byte:
            MinVal = 0;
            break;
        case GDT_UInt16:
            MinVal = 0;
            break;
        case GDT_Int16:
            MinVal = -32768;
            break;
        case GDT_Int32:
            MinVal = -2147483648.0;
            break;
        case GDT_UInt32:
            MinVal = 0;
            break;
            //The floating point types' minimum differs from those in GDAL,
            //which seems incorrect.
        case GDT_Float32:
            MinVal = (-FLT_MAX);
            break;
        case GDT_Float64:
            MinVal = (-DBL_MAX);
            break;
        default:
            HASSERT(0);
            break;
        }

    return MinVal;
    }

//-----------------------------------------------------------------------------
// Protected
// GetMaximumPossibleValue
//-----------------------------------------------------------------------------
double HRFGdalSupportedFile::GetMaximumPossibleValue(unsigned short pi_DataType)
    {
    double MaxVal = LDBL_MAX;

    switch (pi_DataType)
        {
        case GDT_Byte:
            MaxVal = 255;
            break;
        case GDT_UInt16:
            MaxVal = 65535;
            break;
        case GDT_Int16:
            MaxVal = 32767;
            break;
        case GDT_Int32:
            MaxVal = 2147483647.0;
            break;
        case GDT_UInt32:
            MaxVal = 4294967295.0;
            break;
            //The floating point types' maximum differs from those in GDAL,
            //which seems incorrect.
        case GDT_Float32:
            MaxVal = FLT_MAX;
            break;
        case GDT_Float64:
            MaxVal = DBL_MAX;
            break;
        default:
            HASSERT(0);
            break;
        }

    return MaxVal;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::CreateDescriptorsWith(const HFCPtr<HCDCodec>& pi_rpCodec,
                                                 HPMAttributeSet&       pi_rTagList,
                                                 HRPHistogram*          pi_pHistogram)
    {
    HPRECONDITION (m_IsOpen);
    HPRECONDITION (m_pPixelType != 0);
    HPRECONDITION (GetImageWidth() > 0);
    HPRECONDITION (GetImageHeight() > 0);

    // Create Page and resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor> pResolution;
    HFCPtr<HRFPageDescriptor>       pPage;
    bool                           IsTransfoModel = false;
    RasterFileGeocodingPtr         pGeocoding = RasterFileGeocoding::Create();//No geocoding by default, HRFFile MUST set it if supported;

    //warning : GetGeoRefMatrix needs to be called before IsValidGeoRefInfo
    //if theres no valid transfo model, we dont check for GeoInfo
    if(GetGeoRefMatrix())
        {
        pGeocoding = ExtractGeocodingInformation();
        IsTransfoModel = true;
        }

    DetectOptimalBlockAccess();

    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),                                    // AccessMode,
                                               GetCapabilities(),                                  // Capabilities,
                                               m_GeoRefInfo.m_A00 != 0.0f ? m_GeoRefInfo.m_A00 : 1.0,// XResolutionRatio,
                                               m_GeoRefInfo.m_A11 != 0.0f ? m_GeoRefInfo.m_A11 : 1.0,// YResolutionRatio,
                                               m_pPixelType,                                       // PixelType,
                                               pi_rpCodec,                                         // Codec,
                                               HRFBlockAccess::RANDOM,                             // RBlockAccess,
                                               HRFBlockAccess::RANDOM,                             // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,       // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,                           // InterleaveType
                                               false,                                              // IsInterlace,
                                               GetImageWidth(),                                    // Width image ,
                                               GetImageHeight(),                                   // Height image,
                                               m_BlockWidth,                                    // BlockWidth,
                                               m_BlockHeight,                                               // BlockHeight,
                                               0,                                                  // BlocksDataFlag
                                               HRFBlockType::AUTO_DETECT,
                                               1,
                                               (unsigned short)GetBitsPerPixelPerBand());

    HFCPtr<HGF2DTransfoModel> pBuildTransfoModel;

    //warning : GetGeoRefInfo needs to be called before IsValidGeoRefInfo
    if ((IsTransfoModel == true) && IsValidGeoRefInfo())
        {
        if (pGeocoding != 0)
            {
            bool DefaultUnitWasFound = false;

            pBuildTransfoModel = pGeocoding->TranslateToMeter(BuildTransfoModel(),
                                                                1.0,
                                                                false,
                                                                &DefaultUnitWasFound);

            SetUnitFoundInFile(DefaultUnitWasFound);
            }
        else
            {
            pBuildTransfoModel = BuildTransfoModel();
            }

        m_IsGeoReference = true;
        }

    if ((GetScanLineOrientation() != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL) &&
        (pBuildTransfoModel != 0))
        {
        // Extract Affine matrix
        HFCMatrix<3, 3> MyMatrix = pBuildTransfoModel->GetMatrix();

        // SLO 0
        if (GetScanLineOrientation() == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            // Instanciate required affine
            HFCPtr<HGF2DAffine> pAffineTransfo = new HGF2DAffine();

            pAffineTransfo->SetByMatrixParameters(MyMatrix[0][2],
                                                  MyMatrix[1][0],
                                                  -MyMatrix[1][1],
                                                  MyMatrix[1][2],
                                                  -MyMatrix[0][0],
                                                  MyMatrix[0][1]);

            pBuildTransfoModel = pAffineTransfo->CreateSimplifiedModel();

            if (pBuildTransfoModel == 0)
                {
                pBuildTransfoModel = pAffineTransfo;
                }
            }
        }

    // If one of the minimum or maximum sample value tags has been set
    // by the derived class don't try to obtain those values from the
    // more generic interface provided by GDAL.

    if ((!pi_rTagList.HasAttribute<HRFAttributeMinSampleValue>()) && (!pi_rTagList.HasAttribute<HRFAttributeMaxSampleValue>())) 
        {
        AddMinMaxSampleValueTags(pi_rTagList);
        }

    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),                       // Capabilities,
                                   pResolution,                             // ResolutionDescriptor,
                                   0,                                       // RepresentativePalette,
                                   pi_pHistogram,                           // Histogram,
                                   0,                                       // Thumbnail,
                                   0,                                       // ClipShape,
                                   pBuildTransfoModel,                      // TransfoModel,
                                   0,                                       // Filters
                                   &pi_rTagList);                           // Attribute set

    pPage->InitFromRasterFileGeocoding(*pGeocoding);

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Protected
// AddMinMaxSampleValueTags
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::AddMinMaxSampleValueTags(HPMAttributeSet& pi_rTagList)
    {
    BandIndList BandIndList;

    switch (GetDispRep())
        {
        case MONO :
            BandIndList.push_back(m_GrayBandInd);

            AddSampleValueLimitTag(pi_rTagList, BandIndList, false);
            AddSampleValueLimitTag(pi_rTagList, BandIndList, true);
            break;

        case RGB :
            BandIndList.push_back(m_RedBandInd);
            BandIndList.push_back(m_GreenBandInd);
            BandIndList.push_back(m_BlueBandInd);

            if (m_AlphaBandInd != -1)
                {
                BandIndList.push_back(m_AlphaBandInd);
                }
            else if (m_ExtendedBandInd != -1)
                {
                BandIndList.push_back(m_ExtendedBandInd);
                }

            AddSampleValueLimitTag(pi_rTagList, BandIndList, false);
            AddSampleValueLimitTag(pi_rTagList, BandIndList, true);
            break;

        case YCC :
            BandIndList.push_back(m_YbandInd);
            BandIndList.push_back(m_CbBandInd);
            BandIndList.push_back(m_CrBandInd);

            AddSampleValueLimitTag(pi_rTagList, BandIndList, false);
            AddSampleValueLimitTag(pi_rTagList, BandIndList, true);
            break;

        case PALETTE :
            BandIndList.push_back(m_PaletteBandInd);

            AddSampleValueLimitTag(pi_rTagList, BandIndList, false);
            AddSampleValueLimitTag(pi_rTagList, BandIndList, true);
            break;

        default :
            HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Protected
// AddSampleValueLimitTag
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::AddSampleValueLimitTag(HPMAttributeSet& pi_rTagList,
                                                  BandIndList&     pi_rBandIndList,
                                                  bool            pi_AddMinimumSampleValue)
    {
    //Get the minimum and maximum elevation values from the file's metadata.
    const char*                 pElevationLimit;
    HFCPtr<HPMGenericAttribute> pTag;
    vector<double>             ElevationLimit;
    double                     LimitValue;
    char*                      pStrNext;

    BandIndList::iterator IndListIter    = pi_rBandIndList.begin();
    BandIndList::iterator IndListIterEnd = pi_rBandIndList.end();

    while (IndListIter != IndListIterEnd)
        {
        if (pi_AddMinimumSampleValue == true)
            {
            pElevationLimit = GetRasterBand(*IndListIter)->GetMetadataItem("STATISTICS_MINIMUM");
            }
        else
            {
            pElevationLimit = GetRasterBand(*IndListIter)->GetMetadataItem("STATISTICS_MAXIMUM");
            }

        // Set minimum and maximum value tags.
        if (pElevationLimit != 0)
            {
            LimitValue = strtod(pElevationLimit, &pStrNext);

            //Conversion succeeded
            if (pStrNext > pElevationLimit)
                {
                ElevationLimit.push_back(LimitValue);
                }
            }

        IndListIter++;
        }

    if (ElevationLimit.size() == pi_rBandIndList.size())
        {
        if (pi_AddMinimumSampleValue == true)
            {
            pi_rTagList.Set(new HRFAttributeMinSampleValue(ElevationLimit));
            }
        else
            {
            pi_rTagList.Set(new HRFAttributeMaxSampleValue(ElevationLimit));
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// GetGeoRefMatrix
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::GetGeoRefMatrix()
    {
    HPRECONDITION(m_poDataset != 0);

    bool       ReturnBool = false;
    double      adfGeoTransform[6];
    CPLErr      Err = CE_None;

    //The upper left corner of the image is at coordinate (m_Tx, m_Ty).
    CHECK_ERR(Err = m_poDataset->GetGeoTransform(adfGeoTransform);)

    if(Err == CE_None)
        {
        m_GeoRefInfo.m_A00 = adfGeoTransform[1];
        m_GeoRefInfo.m_A01 = adfGeoTransform[2];
        m_GeoRefInfo.m_A10 = adfGeoTransform[4];
        m_GeoRefInfo.m_A11 = adfGeoTransform[5];
        m_GeoRefInfo.m_Tx  = adfGeoTransform[0];
        m_GeoRefInfo.m_Ty  = adfGeoTransform[3];

        ReturnBool = true;
        }

    return ReturnBool;
    }

//-----------------------------------------------------------------------------
// Protected
// SetGeoRefMatrixSetGeoRefMatrix
//-----------------------------------------------------------------------------

bool HRFGdalSupportedFile::SetGeoRefMatrix()
    {
    HPRECONDITION(m_poDataset != 0);
    HPRECONDITION(GetPageDescriptor(0) != 0);
    HPRECONDITION(GetPageDescriptor(0)->GetTransfoModel() != 0);

    bool       RetVal              = false;
    bool       DefaultUnitWasFound = false;
    double      adfGeoTransform[6];

    HFCPtr<HGF2DTransfoModel> pTransfoModel = GetPageDescriptor(0)->GetTransfoModel();

    // Translate the model units in geotiff units.
    IRasterBaseGcsCP pBaseGcs = GetPageDescriptor(0)->GetGeocodingCP();


    //If some geocoding information is available, get the transformation model in meters
    if (pBaseGcs != 0)
        {
        pTransfoModel = GetPageDescriptor(0)->GetRasterFileGeocoding().TranslateFromMeter(pTransfoModel, false, &DefaultUnitWasFound);
        }

    SetUnitFoundInFile(DefaultUnitWasFound);

    if (pTransfoModel->CanBeRepresentedByAMatrix())
        {
        HFCMatrix<3, 3> Matrix = pTransfoModel->GetMatrix();

        double OffsetX = Matrix[0][2];
        double OffsetY = Matrix[1][2];

        //Pixel is Area - The transformation's origin correspond to a center of a pixel.
        //Add a translation to position to origin to the upper left corner of a pixel.
        if(m_GTRasterType == TIFFGeo_RasterPixelIsArea)
            {
            OffsetX -= 0.5 * (Matrix[0][0]);
            OffsetY -= 0.5 * (Matrix[1][1]);
            }

        adfGeoTransform[1] = Matrix[0][0];
        adfGeoTransform[2] = Matrix[0][1];
        adfGeoTransform[4] = Matrix[1][0];
        adfGeoTransform[5] = Matrix[1][1];
        adfGeoTransform[0] = OffsetX;
        adfGeoTransform[3] = OffsetY;

        CPLErr ErrCode = CE_None;

        CHECK_ERR(ErrCode = m_poDataset->SetGeoTransform(adfGeoTransform);)

        if (ErrCode == CE_None)
            {
            RetVal = true;
            }
        }
    else
        {
        HASSERT(0);
        }

    return RetVal;
    }

//-----------------------------------------------------------------------------
// Protected
// IsValidGeoRefInfo
// Multiple returns for simplicity
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsValidGeoRefInfo() const
    {
    if((HDOUBLE_EQUAL(m_GeoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_GeoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_GeoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_GeoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_GeoRefInfo.m_A00, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_GeoRefInfo.m_A01, 0.0, HEPSILON_MULTIPLICATOR))     ||
       (HDOUBLE_EQUAL(m_GeoRefInfo.m_A10, 0.0, HEPSILON_MULTIPLICATOR) &&
        HDOUBLE_EQUAL(m_GeoRefInfo.m_A11, 0.0, HEPSILON_MULTIPLICATOR)))
        return false;

    // Origin must be greater that -2e294
    if (m_GeoRefInfo.m_Tx < -HMAX_EPSILON || m_GeoRefInfo.m_Ty < -HMAX_EPSILON)
        return false;

    // Origin must be smaller than 2e294
    if (m_GeoRefInfo.m_Tx >= HMAX_EPSILON || m_GeoRefInfo.m_Ty >= HMAX_EPSILON)
        return false;

    // Limit the pixelsize (?) inside [-2e294, 2e294]
    if (m_GeoRefInfo.m_A00 > HMAX_EPSILON || m_GeoRefInfo.m_A00 < -HMAX_EPSILON ||
        m_GeoRefInfo.m_A11 > HMAX_EPSILON || m_GeoRefInfo.m_A11 < -HMAX_EPSILON)
        return false;

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
// Inspired form HRFBilFile::BuildTransfoModel
// The translation for doq files are defined from the center of the first pixel
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
HFCPtr<HGF2DTransfoModel> HRFGdalSupportedFile::BuildTransfoModel()
    {
    HFCPtr<HGF2DTransfoModel> pFinalTransfo = 0;

    double OffsetX = m_GeoRefInfo.m_Tx;
    double OffsetY = m_GeoRefInfo.m_Ty;

    //Pixel is Area

    if(m_GTModelType == TIFFGeo_UserDefined)
        {
        OffsetX += 0.5 * abs(m_GeoRefInfo.m_A00);
        OffsetY += 0.5 * abs(m_GeoRefInfo.m_A11);
        }

    pFinalTransfo = new HGF2DAffine();
    // TWF are centered at the center of first pixel.
    ((HFCPtr<HGF2DAffine>&)pFinalTransfo)->SetByMatrixParameters(OffsetX,
                                                                 m_GeoRefInfo.m_A00,
                                                                 m_GeoRefInfo.m_A01,
                                                                 OffsetY ,
                                                                 m_GeoRefInfo.m_A10,
                                                                 m_GeoRefInfo.m_A11);

    /*
    static int z = 1;
    if(z && (m_GeoRefInfo.m_A11 > 0 && m_GTModelType == TIFFGeo_UserDefined))
    {
        // Flip the Y Axe because the origin of ModelSpace is lower-left
        HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

        pFlipModel->SetYScaling(-1.0);
        pFinalTransfo = pFlipModel->ComposeInverseWithDirectOf(*pFinalTransfo);

        HFCPtr<HGF2DStretch> pSLOConverterModel = new HGF2DStretch();
        pSLOConverterModel->SetTranslation(HGF2DDisplacement(0.0, -GetImageHeight()));

        pFinalTransfo = ((HFCPtr<HGF2DTransfoModel>&)pSLOConverterModel)->ComposeInverseWithDirectOf(*pFinalTransfo);
    }
    */
    HFCPtr<HGF2DTransfoModel> pSimplifiedTransfo = 0;

    pSimplifiedTransfo = pFinalTransfo->CreateSimplifiedModel();

    if (pSimplifiedTransfo != 0)
        {
        pFinalTransfo = pSimplifiedTransfo;
        }

    return pFinalTransfo;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFGdalSupportedFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                                  unsigned short pi_Resolution,
                                                                  HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFGdalSupportedFileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }


//-----------------------------------------------------------------------------
// Public
// SetColorAttributes
// Set the file's color attributes
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::SetColorAttributes()
    {
    HASSERT( (GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess) );

    //CPLErr error;
    //Note : There are some GDAL supported formats (e.g. : IMG) which haven't any support
    //       for setting the color interpretation. In those cases, the function
    //       HandleNoDisplayBands() should interpret the bands by taking into account
    //       the order they are written by default by ImagePP.

    m_NbBands = m_poDataset->GetRasterCount();

    HFCPtr<HRPPixelType> SourcePixelType = GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();

    if ((SourcePixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID) ||
        (SourcePixelType->GetClassID() == HRPPixelTypeI8Gray8::CLASS_ID))
        {
        m_DisplayRep = PALETTE;
        m_PaletteBandInd = 1;

        //band ID starts at 1 in GDAL
        GetRasterBand(m_PaletteBandInd)->SetColorInterpretation(GCI_PaletteIndex);
        }
    else if((SourcePixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID) ||
            (SourcePixelType->GetClassID() == HRPPixelTypeV16Gray16::CLASS_ID))
        {
        m_DisplayRep = MONO;
        m_GrayBandInd = 1;

        GetRasterBand(m_GrayBandInd)->SetColorInterpretation(GCI_GrayIndex);
        }
    else if((SourcePixelType->GetClassID() == HRPPixelTypeV16Int16::CLASS_ID) ||
            (SourcePixelType->GetClassID() == HRPPixelTypeV32Float32::CLASS_ID))
        {
        m_DisplayRep = MONO;
        m_GrayBandInd = 1;

        GetRasterBand(m_GrayBandInd)->SetColorInterpretation(GCI_Undefined);
        }
    else if ((SourcePixelType->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID) ||
             (SourcePixelType->GetClassID() == HRPPixelTypeV48R16G16B16::CLASS_ID) ||
             (SourcePixelType->GetClassID() == HRPPixelTypeV96R32G32B32::CLASS_ID))
        {
        m_DisplayRep = RGB;
        m_RedBandInd = 1;
        m_GreenBandInd = 2;
        m_BlueBandInd = 3;

        //band ID starts at 1 in GDAL
        GetRasterBand(m_RedBandInd)->SetColorInterpretation(GCI_RedBand);
        GetRasterBand(m_GreenBandInd)->SetColorInterpretation(GCI_GreenBand);
        GetRasterBand(m_BlueBandInd)->SetColorInterpretation(GCI_BlueBand);
        }
    else if ((SourcePixelType->GetClassID() == HRPPixelTypeV32R8G8B8A8::CLASS_ID) ||
             (SourcePixelType->GetClassID() == HRPPixelTypeV64R16G16B16A16::CLASS_ID))
        {
        m_DisplayRep = RGB;
        m_RedBandInd = 1;
        m_GreenBandInd = 2;
        m_BlueBandInd = 3;
        m_AlphaBandInd = 4;

        //band ID starts at 1 in GDAL
        GetRasterBand(m_RedBandInd)->SetColorInterpretation(GCI_RedBand);
        GetRasterBand(m_GreenBandInd)->SetColorInterpretation(GCI_GreenBand);
        GetRasterBand(m_BlueBandInd)->SetColorInterpretation(GCI_BlueBand);
        GetRasterBand(m_AlphaBandInd)->SetColorInterpretation(GCI_AlphaBand);
        }
    else if(SourcePixelType->GetClassID() == HRPPixelTypeV64R16G16B16X16::CLASS_ID)
        {
        m_DisplayRep = RGB;
        m_RedBandInd = 1;
        m_GreenBandInd = 2;
        m_BlueBandInd = 3;
        m_ExtendedBandInd = 4;

        //band ID starts at 1 in GDAL
        GetRasterBand(m_RedBandInd)->SetColorInterpretation(GCI_RedBand);
        GetRasterBand(m_GreenBandInd)->SetColorInterpretation(GCI_GreenBand);
        GetRasterBand(m_BlueBandInd)->SetColorInterpretation(GCI_BlueBand);
        GetRasterBand(m_ExtendedBandInd)->SetColorInterpretation(GCI_Undefined);
        }
    else
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbBands
// Get the number of bands
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetNbBands() const
    {
    HASSERT(m_poDataset != 0);

    return m_poDataset->GetRasterCount();
    }


//-----------------------------------------------------------------------------
// Protected
// GetImageWidth
// Get the number samples per line
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetImageWidth() const
    {
    if (m_poDataset != 0)
        {
        // SLO 0
        if (GetScanLineOrientation() == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            return m_poDataset->GetRasterYSize();
            }
        else
            {
            HASSERT((GetScanLineOrientation() == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL) ||
                    (GetScanLineOrientation() == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

            return m_poDataset->GetRasterXSize();
            }
        }
    else if ((GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess))
        {
        HASSERT(GetPageDescriptor(0) != 0);
        HASSERT(GetPageDescriptor(0)->GetResolutionDescriptor(0) != 0);

        return (uint32_t)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();
        }
    else
        {
        HASSERT(false);
        return 0;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageHeight
// Get the number of rows
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetImageHeight() const
    {
    if (m_poDataset != 0)
        {
        // SLO 0
        if (GetScanLineOrientation() == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            return m_poDataset->GetRasterXSize();
            }
        else
            {
            HASSERT((GetScanLineOrientation() == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL) ||
                    (GetScanLineOrientation() == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

            return m_poDataset->GetRasterYSize();
            }
        }
    else if ((GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess))
        {
        HASSERT(GetPageDescriptor(0) != 0);
        HASSERT(GetPageDescriptor(0)->GetResolutionDescriptor(0) != 0);

        return (int)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();
        }
    else
        {
        HASSERT(false);
        return 0;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// GetRasterBand
//
//-----------------------------------------------------------------------------
GDALRasterBand* HRFGdalSupportedFile::GetRasterBand(int i) const
    {
    HPRECONDITION(m_poDataset != 0);
    HPRECONDITION(i > 0 && i <= GetNbBands());

    return m_poDataset->GetRasterBand(i);
    }

//-----------------------------------------------------------------------------
// Protected
// GetBandRole
// Get the type of information (elevation, color, temperature, etc...) of
// the requested band.
//-----------------------------------------------------------------------------
HRPChannelType::ChannelRole HRFGdalSupportedFile::GetBandRole(int32_t pi_RasterBand) const
    {
    return HRPChannelType::USER;
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbImgBands
// Get the number of bands containing information about displayed image (multiband)
//-----------------------------------------------------------------------------

int HRFGdalSupportedFile::GetNbImgBands() const
    {
    return m_NbBands;
    }


//-----------------------------------------------------------------------------
// Protected
// GetBandInd
// Get the base 1 index of the band for the request color type (multiband)
//-----------------------------------------------------------------------------
signed char HRFGdalSupportedFile::GetBandInd(Byte pi_ColorType) const
    {
    signed char BandInd = -1;

    switch (pi_ColorType)
        {
        case GCI_GrayIndex :
            BandInd = m_GrayBandInd;
            break;
        case GCI_RedBand :
            BandInd = m_RedBandInd;
            break;
        case GCI_GreenBand :
            BandInd = m_GreenBandInd;
            break;
        case GCI_BlueBand :
            BandInd = m_BlueBandInd;
            break;
        case GCI_AlphaBand :
            BandInd = m_AlphaBandInd;
            break;
        case GCI_PaletteIndex :
            BandInd = m_PaletteBandInd;
            break;
        case GCI_Undefined :
            BandInd = m_ExtendedBandInd;
            break;
        case GCI_YCbCr_YBand :
            BandInd = m_YbandInd;
            break;
        case GCI_YCbCr_CbBand :
            BandInd = m_CbBandInd;
            break;
        case GCI_YCbCr_CrBand :
            BandInd = m_CrBandInd;
            break;
        }

    return BandInd;
    }


//-----------------------------------------------------------------------------
// Protected
// AddPage
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_pGdalDriver != 0);
    HPRECONDITION(CountPages() == 0); //Multipage is not implanted at this layer
    HPRECONDITION(pi_pPage->GetResolutionDescriptor(0) != 0);
    HPRECONDITION(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));

    bool   IsPageAdded = false;

    if (m_poDataset == 0)
        {
        HFCPtr<HRPPixelType> PixelType(pi_pPage->GetResolutionDescriptor(0)->GetPixelType());

        size_t NbChannels   = PixelType->GetChannelOrg().CountChannels();
        size_t BitsPerPixel = 0;

        if(PixelType->CountIndexBits() != 0)
            {
            BitsPerPixel = PixelType->CountIndexBits();
            SetHasPalette(true);
            //for gdal, a paletted raster has only 1 channel : the palette
            m_NbBands           = 1;
            }
        else
            {
            BitsPerPixel = PixelType->CountPixelRawDataBits();
            m_NbBands    = (uint32_t) NbChannels;
            }

        if(m_NbBands > 0)
            {
            bool IsSigned;
            bool IsFloat;

            if (m_NbBands == 1)
                {
                if (PixelType->GetChannelOrg().GetChannelPtr(0)->GetDataType() == HRPChannelType::FLOAT_CH)
                    {
                    IsSigned = true;
                    IsFloat  = true;
                    }
                else if (PixelType->GetChannelOrg().GetChannelPtr(0)->GetDataType() == HRPChannelType::SINT_CH)
                    {
                    IsSigned = true;
                    IsFloat  = false;
                    }
                else
                    {
                    IsSigned = false;
                    IsFloat  = false;
                    }
                }
            else
                {
                IsSigned = false;
                IsFloat  = false;
                }

            m_BitsPerPixelPerBand = (unsigned short)(BitsPerPixel/m_NbBands);

            char** ppCreationOptions = 0;

            SetCreationOptions(pi_pPage, ppCreationOptions );

            // TFS#86887: GDAL_FILENAME_IS_UTF8
            Utf8String filenameUtf8;
            BeStringUtilities::WCharToUtf8(filenameUtf8, static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName().c_str());

            CHECK_ERR(m_poDataset = m_pGdalDriver->Create(filenameUtf8.c_str(),
                                                         (uint32_t)pi_pPage->GetResolutionDescriptor(0)->GetWidth(),
                                                         (uint32_t)pi_pPage->GetResolutionDescriptor(0)->GetHeight(),
                                                         m_NbBands,
                                                         (GDALDataType)GetCorrespondingGDALDataType(m_BitsPerPixelPerBand,
                                                            NbChannels, HasPalette(), IsSigned, IsFloat),
                                                         ppCreationOptions);)

            CSLDestroy(ppCreationOptions);

            if (m_poDataset == 0)
                {
                ThrowExBasedOnGDALErrorCode(CPLGetLastErrorNo());
                }
            }
        }

    if (m_poDataset != 0)
        {
        m_BlockWidth = pi_pPage->GetResolutionDescriptor(0)->GetBlockWidth();
        m_BlockHeight = pi_pPage->GetResolutionDescriptor(0)->GetBlockHeight();

        // Add the page descriptor to the list
        IsPageAdded = HRFRasterFile::AddPage(pi_pPage);

        SetColorAttributes();

        HFCPtr<HRPPixelType> pixelType(pi_pPage->GetResolutionDescriptor(0)->GetPixelType());
        
        SetNoDataValue(pixelType);
               
        if (GetDispRep() == PALETTE)
            {
            HASSERT(m_PaletteBandInd != -1);
            WriteColorTable();
            }

        SetGeocodingInformation();

        // Update the TransfoModel
        if ((pi_pPage->HasTransfoModel()))
            {
            SetGeoRefMatrix();
            }
        }

    return IsPageAdded;
}

//-----------------------------------------------------------------------------
// Protected
// SetNoDataValue
// 
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::SetNoDataValue(HFCPtr<HRPPixelType>& pixelType)
    {
    if (m_NbBands == 1)
        {
        assert((pixelType->GetChannelOrg().GetChannelPtr(0) != 0) && (GetRasterBand(1) != 0));
              
        const double* pNoDataValue = pixelType->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue();

        if (pNoDataValue != 0)
            {                
            ((GDALRasterBand*)(GetRasterBand(1)))->SetNoDataValue(*pNoDataValue);                            
            }        
        }   
    }

//-----------------------------------------------------------------------------
// Protected
// GetGeoRefInformation
//
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr HRFGdalSupportedFile::ExtractGeocodingInformation()
    {
    RasterFileGeocodingPtr pGeocoding(RasterFileGeocoding::Create());

    //init
    m_GTModelType = TIFFGeo_Undefined;

    if (GCSServices->_IsAvailable())
        {
        CHECK_ERR(string WktGeocodeTemp = m_poDataset->GetProjectionRef();)

        WString WktGeocode;
        BeStringUtilities::CurrentLocaleCharToWChar(WktGeocode, WktGeocodeTemp.c_str());

        if (WktGeocode != L"")
            {

                //TR 241854 According to the GDAL documentation, the flavor is
                //WktFlavorOGC. But in some case, the WKT has an hybrid flavor,
                //so unknown is used instead.
                IRasterBaseGcsPtr  pBaseGCS = HRFGeoCoordinateProvider:: CreateRasterGcsFromWKT(NULL, NULL, IRasterGeoCoordinateServices::WktFlavorUnknown, WktGeocode.c_str());
                                
            }
        }

    //If GeoCoord could not convert the WKT, try with OGR
    if (pGeocoding->GetGeocodingCP()==NULL)  
        {
        //char* pLocalCS = "LOCAL_CS[\"Unknown\"";

        CHECK_ERR(string sProj = m_poDataset->GetProjectionRef();)

        if (sProj != "")
            {
            HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys;
            pGeoTiffKeys = HRFGdalUtilities::ConvertOGCWKTtoGeotiffKeys(sProj.c_str());

            if(pGeoTiffKeys != NULL)
                pGeocoding = RasterFileGeocoding::Create(pGeoTiffKeys);
            }
        }

    // Get common geotiff tags
    if (pGeocoding->GetGeocodingCP()!=NULL)
        {
        if ((pGeocoding->GetGeocodingCP()->IsValid() && pGeocoding->GetGeocodingCP()->GetBaseGCS() != NULL))
            {
            m_GTModelType = (pGeocoding->GetGeocodingCP()->IsProjected() ? 1 : 0);
            }
        }

    return pGeocoding;
    }


//-----------------------------------------------------------------------------
// Protected
// AddVerticalUnitToGeocoding
// Note that this function should currently be used only for obtaining the unit
// of elevation measurements present in a DEM raster.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::AddVerticalUnitToGeocoding(IRasterBaseGcsR pio_pBaseGCS) const
    {
    const char* pUnitType = GetRasterBand(m_GrayBandInd)->GetUnitType();
    uint32_t     VerticalUnitValue;
    double        VerticalUnitRatioToMeter = 1.0;

    //The unit type should be "m" for meter or "ft" for feet.
    if (strncmp(pUnitType, "m", 1) == 0)
        {
        HASSERT(strlen(pUnitType) == 1);
        VerticalUnitValue = TIFFGeo_Linear_Meter;
        VerticalUnitRatioToMeter = 1.0;
        }
    else
        {
        HASSERT((strncmp(pUnitType, "ft", 2) == 0) &&
                (strlen(pUnitType) == 2));
        VerticalUnitValue = TIFFGeo_Linear_Foot;
        VerticalUnitRatioToMeter = 0.3048;
        }

    pio_pBaseGCS.SetVerticalUnits(VerticalUnitRatioToMeter);
    }


//-----------------------------------------------------------------------------
// Private
// AddSupportedGeoCapability
// Add the capability obtained from GDAL only if the file is known to support it.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::AddSupportedGeoTag(HFCPtr<HRFRasterFileCapabilities>& pi_rpSupportedGeoTags,
                                              HFCPtr<HPMGenericAttribute>&       pi_rpGdalObtainedTag,
                                              HPMAttributeSet&                   pio_rTagList)
    {
    if ((pi_rpSupportedGeoTags != 0) && pi_rpSupportedGeoTags->Supports(new HRFTagCapability(HFC_READ_ONLY, pi_rpGdalObtainedTag)))
        {
        pio_rTagList.Set(pi_rpGdalObtainedTag);
        }
    }

//-----------------------------------------------------------------------------
// Protected
// SetGeocodingInformation
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::SetGeocodingInformation()
    {
    HPRECONDITION(GetPageDescriptor(0) != 0);

    HFCPtr<HRFPageDescriptor>    pPageDescriptor(GetPageDescriptor(0));
    bool                         IsGeocodingSet = false;
    WString                      OGCWellKnownText;
    HFCPtr<HCPGeoTiffKeys>       pGeoTiffKeys;

    IRasterBaseGcsCP pBaseGCS = pPageDescriptor->GetGeocodingCP();

    if (pBaseGCS != 0)
        {
        if (GCSServices->_IsAvailable())
            {
            pBaseGCS->GetWellKnownText(OGCWellKnownText, IRasterGeoCoordinateServices::WktFlavorOGC);
            if (OGCWellKnownText != L"")
                {
                pGeoTiffKeys = new HCPGeoTiffKeys();
                pBaseGCS->GetGeoTiffKeys(pGeoTiffKeys);

                pGeoTiffKeys->GetValue(GTModelType, &m_GTModelType);

                IsGeocodingSet = true;
                }
            }

        if ((IsGeocodingSet == false) && (pGeoTiffKeys != NULL))
            {
            IsGeocodingSet = HRFGdalUtilities::ConvertGeotiffKeysToOGCWKT(pGeoTiffKeys,
                                                                          OGCWellKnownText);
            }

        if (IsGeocodingSet == true)
            {
            size_t  destinationBuffSize = OGCWellKnownText.GetMaxLocaleCharBytes();
            char*   multiByteDestination= (char*)_alloca (destinationBuffSize);

            CHECK_ERR(m_poDataset->SetProjection(OGCWellKnownText.ConvertToLocaleChars(multiByteDestination, destinationBuffSize));)
            }
        else
            {   //The georeference in the newly created file or saved file will be lost.
            HASSERT_DATA(0);
            }
        }

    return IsGeocodingSet;
    }

//-----------------------------------------------------------------------------
// Protected
// SetCreationOptions
//
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::SetCreationOptions(HFCPtr<HRFPageDescriptor>& pi_rpPage,
                                              char** &                    pio_ppCreationOptions) const
    {
    //By default do nothing
    }

//-----------------------------------------------------------------------------
// Protected
// GetCorrespondingGDALDataType
//
//-----------------------------------------------------------------------------
uint32_t HRFGdalSupportedFile::GetCorrespondingGDALDataType(size_t pi_NbBitsPerBandPerPixel,
                                                         size_t pi_NbChannel,
                                                         bool  pi_HasPalette,
                                                         bool  pi_IsSigned,
                                                         bool  pi_IsFloat)
    {
    uint32_t result = 0;

    switch (pi_NbChannel)
        {
        case 1:
            if (pi_NbBitsPerBandPerPixel == 8)
                {
                HASSERT((pi_IsSigned == false) && (pi_IsFloat == false));
                m_Signed = pi_IsSigned;
                result = GDT_Byte;
                }
            else if (pi_NbBitsPerBandPerPixel == 16)
                {
                HASSERT(pi_IsFloat == false);
                m_Signed = pi_IsSigned;

                if (pi_IsSigned == false)
                    {
                    result = GDT_UInt16;
                    }
                else
                    {
                    result = GDT_Int16;
                    }
                }
            else if (pi_NbBitsPerBandPerPixel == 32)
                {
                if (pi_IsFloat == true)
                    {
                    HASSERT(pi_IsSigned == true);
                    result = GDT_Float32;
                    }
                }
            break;

        case 3:
            switch (pi_NbBitsPerBandPerPixel)
                {

                case 8:
                    m_Signed = false;
                    result = GDT_Byte;
                    break;

                case 16:
                    if (!pi_HasPalette)
                        {
                        m_Signed = false;
                        result = GDT_UInt16;
                        }
                    break;

                case 32:
                    if (!pi_HasPalette)
                        {
                        m_Signed = false;
                        result = GDT_UInt32;
                        }
                    break;

                default:
                    break;

                }
            break;

        case 4:
            switch (pi_NbBitsPerBandPerPixel)
                {
                case 8:
                    m_Signed = false;
                    result = GDT_Byte;
                    break;

                case 16:
                    m_Signed = false;
                    result = GDT_UInt16;
                    break;

                default:
                    break;
                }
            break;

        default:
            break;

        }

    HASSERT(result != GDT_Unknown);

    return result;
    }

//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFGdalSupportedFile::GetScanLineOrientation()const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// Protected
// ThrowExBasedOnGDALErrorCode
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::ThrowExBasedOnGDALErrorCode(uint32_t pi_GetLastErrorCode)
    {
    switch (pi_GetLastErrorCode)
        {
        case CPLE_OutOfMemory :
            throw HFCOutOfMemoryException();
            break;
        case CPLE_FileIO :
            throw HFCFileIOErrorException(GetURL()->GetURL());
            break;
        case CPLE_OpenFailed :
            throw HFCCannotOpenFileException(GetURL()->GetURL());
            break;
        case CPLE_NotSupported :
            throw HFCFileNotSupportedException(GetURL()->GetURL());
            break;
        case CPLE_AssertionFailed : //Do nothing
            break;
        case CPLE_NoWriteAccess :
            throw HFCFileReadOnlyException(GetURL()->GetURL());
            break;
        case CPLE_UserInterrupt :
            throw HFCFileReadOnlyException(GetURL()->GetURL());
            break;
        case CPLE_IllegalArg :
        case CPLE_ObjectNull : //There is currently no exception in Imagepp which can maps to CPLE_ObjectNull.
        case CPLE_AppDefined : //It seems to be the default error code.
        default :
            throw HRFGenericException(GetURL()->GetURL());
            break;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFGdalSupportedFile::HRFGdalSupportedFile(const char*           pi_pDriverName, 
                                           const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t              pi_Offset)

    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    m_pGdalDriver = GetGDALDriverManager()->GetDriverByName(pi_pDriverName);
    if(NULL == m_pGdalDriver)
        {
        HASSERT(!"HRFGdalSupportedFile: GDAL Driver not found");
        throw HFCFileNotSupportedException( pi_rURL->GetURL());
        }

    // The ancestor store the access mode
    m_IsOpen        = false;

    m_poDataset     = NULL;

    m_GeoRefInfo.m_A00 = 1.0;
    m_GeoRefInfo.m_A01 = 0.0;
    m_GeoRefInfo.m_A10 = 0.0;
    m_GeoRefInfo.m_A11 = 1.0;
    m_GeoRefInfo.m_Tx = 0.0;
    m_GeoRefInfo.m_Ty = 0.0;

    m_Signed                        = false;
    m_IsGrayScale                   = false;
    m_HasPalette                    = false;
    m_IsReadPixelReal               = false;
    m_IsUPixelTypeForSignedData     = false;
    m_IsIntegerPixelTypeForRealData = false;

    m_DisplayRep      = UNDEFINED;

    m_IsGeoReference  = false;
    m_GTModelType     = TIFFGeo_Undefined;
    m_GTRasterType    = 0;

    m_NbBands             = 0;
    m_BlockWidth          = 0;
    m_BlockHeight         = 0;
    m_BitsPerPixelPerBand = 0;

    m_GrayBandInd     = -1;
    m_RedBandInd      = -1;
    m_GreenBandInd    = -1;
    m_BlueBandInd     = -1;
    m_AlphaBandInd    = -1;
    m_PaletteBandInd  = -1;
    m_ExtendedBandInd = -1;
    m_YbandInd        = -1;
    m_CbBandInd       = -1;
    m_CrBandInd       = -1;
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFGdalSupportedFile::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() > 0);

    HGF2DWorldIdentificator World;

    //If there is no georeference returns by GDAL the world should be
    //oriented in the SLO 4 direction.
    if ((m_IsGeoReference == true) ||
        (GetAccessMode().m_HasCreateAccess == true))
        {
        switch (m_GTModelType)
            {
            case TIFFGeo_ModelTypeProjected:
                World = HGF2DWorld_HMRWORLD;
                break;

            case TIFFGeo_ModelTypeGeographic:
                World = HGF2DWorld_GEOGRAPHIC;
                break;

            default:
                World = HGF2DWorld_GEOTIFFUNKNOWN;
                break;
            }
        }
    else
        {
        World = HGF2DWorld_UNKNOWNWORLD;
        }

    return World;
    }

//-----------------------------------------------------------------------------
// Protected
// GetGDALDataset
//-----------------------------------------------------------------------------
GDALDataset* HRFGdalSupportedFile::GetDataSet() const
    {
    return m_poDataset;
    }

//-----------------------------------------------------------------------------
// Protected
// GetDispRep
// Get how the image should be displayed
//-----------------------------------------------------------------------------

DISPLAY_REP HRFGdalSupportedFile::GetDispRep() const
    {
    return m_DisplayRep;
    }


//-----------------------------------------------------------------------------
// Protected
// GetNbDisplayableBands
// Get the numbers of displayable bands.
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetNbDisplayableBands() const
    {
    int NbDisplayableBands = 0;
    switch (m_DisplayRep)
        {
        case MONO :
        case PALETTE :
            NbDisplayableBands = 1;
            break;
        case RGB :
            if ((m_AlphaBandInd != -1) || (m_ExtendedBandInd != -1))
                {
                NbDisplayableBands = 4;
                }
            else
                {
                NbDisplayableBands = 3;
                }
            break;
        case YCC :
            NbDisplayableBands = 3;
            break;
        default :
            HASSERT(0);
            break;
        }

    return NbDisplayableBands;
    }


//-----------------------------------------------------------------------------
// Protected
// GetHeaderSize
// Get the header size.
//-----------------------------------------------------------------------------

uint32_t HRFGdalSupportedFile::GetBitsPerPixelPerBand() const
    {
    return m_BitsPerPixelPerBand;
    }

//-----------------------------------------------------------------------------
// Protected
// GetTotalRowBytes
// Get the number of bytes per row
//-----------------------------------------------------------------------------

uint32_t HRFGdalSupportedFile::GetTotalRowBytes() const
    {
    return m_BitsPerPixelPerBand/8 * GetImageWidth();
    }

//-----------------------------------------------------------------------------
// Protected
// IsReadPixelSigned
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsReadPixelSigned() const
    {
    return m_Signed;
    }

//-----------------------------------------------------------------------------
// Protected
// IsUnsignedPixelTypeForSignedData
// Return true if the unsigned pixel type is used to represent signed data.
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsUnsignedPixelTypeForSignedData() const
    {
    return m_IsUPixelTypeForSignedData;
    }

//-----------------------------------------------------------------------------
// Protected
// IsIntegerPixelTypeForRealData
// Return true if an integer pixel type is used to represent real data.
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsIntegerPixelTypeForRealData() const
    {
    return m_IsIntegerPixelTypeForRealData;
    }

//-----------------------------------------------------------------------------
// Protected
// IsGrayScale
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsGrayScale() const
    {
    return m_IsGrayScale;
    }


//-----------------------------------------------------------------------------
// Protected
// IsGrayScale
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::IsReadPixelReal() const
    {
    return m_IsReadPixelReal;
    }

//-----------------------------------------------------------------------------
// Protected
// SetHasPalette
//
//-----------------------------------------------------------------------------
void HRFGdalSupportedFile::SetHasPalette(bool pi_hasPalette)
    {
    m_HasPalette = pi_hasPalette;
    }

//-----------------------------------------------------------------------------
// Protected
// HasPalette
//
//-----------------------------------------------------------------------------
bool HRFGdalSupportedFile::HasPalette() const
    {
    return m_HasPalette;
    }

//-----------------------------------------------------------------------------
// Protected
// GetBlockWidth
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetBlockWidth() const
    {
    return m_BlockWidth;
    }

//-----------------------------------------------------------------------------
// Protected
// GetBlockHeight
//-----------------------------------------------------------------------------
int HRFGdalSupportedFile::GetBlockHeight() const
    {
    return m_BlockHeight;
    }
