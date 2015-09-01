//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGdalSupportedFileEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGdalSupportedFileEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGdalSupportedFile.h>
#include <Imagepp/all/h/HRFGdalSupportedFileEditor.h>
#include <Imagepp/all/h/HTIFFUtils.h>

//PixelType
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

#include <ImagePP-GdalLib/gdal_priv.h>

//-----------------------------------------------------------------------------
//                              Notes
//-----------------------------------------------------------------------------
//
//  Scaling data with a linear factor IS defenitly NOT be the best ways to
//  enhance data visualization.  Because they often provide  a meam AND a
//  standard diviation for a given dataset, we suppose the data should normally
//  follow a kind of a bell curve (wich can be define as a gaussian function...).
//
//  In that case, the smartest ways of scaling these data to spread their values
//  evenly across all the band width should be using a gaussian function.
//
//  We have provided a gaussian lookup table, and to see wich value the pixel
//  will be scaled, is to find it's proper z quote value.
//
//        X      : Original (data) pixel value.
//        X_m    : Given all pixel (data) mean.
//        st_dev : Given standard diviation.
//
//        Z      : Quote to find in the lookup table. The index will represent
//                 the new scaled pixel value.
//
//        Z = (X - X_m) / st_dev
//
//  Find where Z is in the look up, the index will be the scaled value.

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

#define RASTER_FILE ((HFCPtr<HRFGdalSupportedFile>&)m_pRasterFile)

HRFGdalSupportedFileEditor::HRFGdalSupportedFileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                       uint32_t              pi_Page,
                                                       unsigned short       pi_Resolution,
                                                       HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    HPRECONDITION(RASTER_FILE->GetDataSet() != 0);

    m_pBlockType = RASTER_FILE->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType();

    m_BlockWidth = RASTER_FILE->GetBlockWidth();
    m_BlockHeight = RASTER_FILE->GetBlockHeight();

    m_UseLinearBandScaling = false;

    m_pScalePixelFnc = 0;

    bool HasWriteAccess = (m_AccessMode.m_HasCreateAccess == true) ||
                           (m_AccessMode.m_HasWriteAccess == true);

    m_NbBitsPerBandPerPixel = RASTER_FILE->GetBitsPerPixelPerBand();
    m_NbBytePerBandPerPixel = (uint32_t)ceil(RASTER_FILE->GetBitsPerPixelPerBand() / 8.0);
    m_NbBands                = RASTER_FILE->GetNbDisplayableBands();
    m_BytesPerLineBand        = RASTER_FILE->GetBlockWidth() * RASTER_FILE->GetBitsPerPixelPerBand() / 8;
    m_NbPixelsPerBlock        = RASTER_FILE->GetBlockWidth() * RASTER_FILE->GetBlockHeight();

    m_pRasterBands = new GDALRasterBand*[m_NbBands];
    m_pBandMinimum = new double[m_NbBands];
    m_pBandScaling = new double[m_NbBands];

    m_PixelSpaceInBytes = m_NbBands;
    m_LineSpaceInBytes = m_NbBands * RASTER_FILE->GetBlockWidth();

    if (RASTER_FILE->GetDataSet() != 0)
        {
        switch (RASTER_FILE->GetDispRep())
            {
            case MONO :
                m_pRasterBands[GRAY_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_GrayIndex));
                break;

            case RGB :
                m_pRasterBands[RED_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_RedBand));
                m_pRasterBands[GREEN_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_GreenBand));
                m_pRasterBands[BLUE_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_BlueBand));

                if (RASTER_FILE->GetBandInd(GCI_AlphaBand) != -1)
                    {
                    m_pRasterBands[ALPHA_EXT_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_AlphaBand));
                    }
                else if (RASTER_FILE->GetBandInd(GCI_Undefined) != -1)
                    {
                    m_pRasterBands[ALPHA_EXT_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_Undefined));
                    }
                break;

            case YCC :
                m_pRasterBands[Y_BAND]  = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_YCbCr_YBand));
                m_pRasterBands[CB_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_YCbCr_CbBand));
                m_pRasterBands[CR_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_YCbCr_CrBand));

                HASSERT(RASTER_FILE->IsReadPixelSigned() == false);
                break;

            case PALETTE :

                switch (m_NbBitsPerBandPerPixel)
                    {
                    case 8:
                        m_pRasterBands[PALETTE_BAND] = RASTER_FILE->GetRasterBand(RASTER_FILE->GetBandInd(GCI_PaletteIndex));

                        HASSERT(RASTER_FILE->IsReadPixelSigned() == false);
                        break;
                    default:
                        HASSERT(0);
                    }
                break;
            default :
                HASSERT(0);
            }
        }

    m_GdalDataType =(unsigned short) m_pRasterBands[0]->GetRasterDataType();

    if (RASTER_FILE->IsUnsignedPixelTypeForSignedData() == true)
        {
        switch (RASTER_FILE->GetDispRep())
            {
            case MONO :
                SetSingleBandLinearScaling(HRFGdalSupportedFile::GetMinimumPossibleValue(m_GdalDataType),
                                           HRFGdalSupportedFile::GetMaximumPossibleValue(m_GdalDataType));
                break;

            case RGB :
                double MinValue = HRFGdalSupportedFile::GetMinimumPossibleValue(m_GdalDataType);
                double MaxValue = HRFGdalSupportedFile::GetMaximumPossibleValue(m_GdalDataType);

                SetRGBABandLinearScaling(MinValue, MaxValue,
                                         MinValue, MaxValue,
                                         MinValue, MaxValue,
                                         m_NbBands == 4 ? &MinValue : 0,
                                         m_NbBands == 4 ? &MaxValue : 0);
                break;
            }

        if (HasWriteAccess == true)
            {
            m_pTempSignedBuffer = new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()];
            }
        }

    if (RASTER_FILE->IsReadPixelReal() == true)
        {
        m_pReadBlockFnc = (ReadBlockFncPtr)&HRFGdalSupportedFileEditor::ReadRealBlock;

        if (m_GdalDataType == GDT_Float32)
            {
            //A temporary buffer is needed if the integer pixel type is
            //used to read the real data.
            if (RASTER_FILE->IsIntegerPixelTypeForRealData() == true)
                {
                m_pTempRealBuffer = (Byte*)new float[(size_t)(m_NbBands * m_NbPixelsPerBlock)];
                }
            m_PixelSpaceInBytes *= sizeof(float);
            m_LineSpaceInBytes  *= sizeof(float);
            }
        else
            {
            HASSERT(RASTER_FILE->IsIntegerPixelTypeForRealData() == true);
            m_pTempRealBuffer = (Byte*)new double[(size_t)(m_NbBands * m_NbPixelsPerBlock)];
            m_PixelSpaceInBytes *= sizeof(double);
            m_LineSpaceInBytes  *= sizeof(double);
            }
        }
    else
        {
        m_pReadBlockFnc = (ReadBlockFncPtr)&HRFGdalSupportedFileEditor::ReadIntegerBlock;

        m_PixelSpaceInBytes *= m_NbBytePerBandPerPixel;
        m_LineSpaceInBytes  *= m_NbBytePerBandPerPixel;
        }

    if (m_UseLinearBandScaling == true)
        {
        DetermineScalingMethod();
        }
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFGdalSupportedFileEditor::~HRFGdalSupportedFileEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Set and activate a linear scaling operation
// Linear Scaling
//-----------------------------------------------------------------------------

void HRFGdalSupportedFileEditor::SetRGBABandLinearScaling(double  pi_MinRedValue,
                                                          double  pi_MaxRedValue,
                                                          double  pi_MinGreenValue,
                                                          double  pi_MaxGreenValue,
                                                          double  pi_MinBlueValue,
                                                          double  pi_MaxBlueValue,
                                                          double* pi_pMinAlphaOrExtendValue,
                                                          double* pi_pMaxAlphaOrExtendValue)
    {
    HPRECONDITION(pi_MinRedValue < pi_MaxRedValue);
    HPRECONDITION(pi_MinGreenValue < pi_MaxGreenValue);
    HPRECONDITION(pi_MinBlueValue < pi_MaxBlueValue);

#ifdef HVERIFYCONTRACT
    if ((pi_pMinAlphaOrExtendValue != 0) &&
        (pi_pMaxAlphaOrExtendValue != 0))
        {
        HASSERT(*pi_pMinAlphaOrExtendValue < *pi_pMaxAlphaOrExtendValue);
        }
#endif

    uint64_t MaxPossibleValue = 0;

    HASSERT(m_NbBitsPerBandPerPixel <= 32);

    MaxPossibleValue = ((uint64_t)1 << m_NbBitsPerBandPerPixel) - 1;

    m_pBandScaling[0] = MaxPossibleValue / (pi_MaxRedValue - pi_MinRedValue);
    m_pBandMinimum[0] = pi_MinRedValue;

    m_pBandScaling[1] = MaxPossibleValue / (pi_MaxGreenValue - pi_MinGreenValue);
    m_pBandMinimum[1] = pi_MinGreenValue;

    m_pBandScaling[2] = MaxPossibleValue / (pi_MaxBlueValue - pi_MinBlueValue);
    m_pBandMinimum[2] = pi_MinBlueValue;

    if ((pi_pMinAlphaOrExtendValue != 0) && (pi_pMinAlphaOrExtendValue != 0))
        {
        m_pBandScaling[3] = MaxPossibleValue / (*pi_pMaxAlphaOrExtendValue - *pi_pMinAlphaOrExtendValue);
        m_pBandMinimum[3] = *pi_pMinAlphaOrExtendValue;
        }

    m_UseLinearBandScaling = true;
    }


//-----------------------------------------------------------------------------
// public
// Set and activate a linear scaling operation for a single band image
// (grayscale or palette)
// Linear Scaling
//-----------------------------------------------------------------------------

void HRFGdalSupportedFileEditor::SetSingleBandLinearScaling(double pi_MinGrayValue, double pi_MaxGrayValue)
    {
    HPRECONDITION(pi_MinGrayValue < pi_MaxGrayValue);

    uint64_t MaxPossibleValue = 0;

    HASSERT(m_NbBitsPerBandPerPixel <=32);

    MaxPossibleValue = ((uint64_t)1 << m_NbBitsPerBandPerPixel) - 1;

    m_pBandScaling[0] = MaxPossibleValue / (pi_MaxGrayValue - pi_MinGrayValue);
    m_pBandMinimum[0] = pi_MinGrayValue;

    m_UseLinearBandScaling = true;
    }

//-----------------------------------------------------------------------------
// protected
// DetermineGdalDataType
// Determine the GDAL data type.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::DetermineGdalDataType()
    {
    if(RASTER_FILE->IsReadPixelReal() == false)
        {
        switch (m_NbBitsPerBandPerPixel)
            {
            case 8 :
                m_GdalDataType = GDT_Byte;
                break;
            case 16 :
                m_GdalDataType = (unsigned short)(RASTER_FILE->IsReadPixelSigned() ? GDT_Int16 : GDT_UInt16);
                break;
            case 32 :
                m_GdalDataType = (unsigned short)(RASTER_FILE->IsReadPixelSigned() ? GDT_Int32 : GDT_UInt32);
                break;
            }
        }
    else
        {
        m_GdalDataType = (unsigned short)m_pRasterBands[GRAY_BAND]->GetRasterDataType();
        }
    }

//-----------------------------------------------------------------------------
// protected
// DetermineScalingMethod
// Determine the best scaling method if required.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::DetermineScalingMethod()
    {
    if ((m_UseLinearBandScaling == true) &&
        (RASTER_FILE->IsReadPixelReal() == false))
        {
        switch (m_NbBitsPerBandPerPixel)
            {
            case 8 :
                m_pScalePixelFnc = (ScalePixelFncPtr)&HRFGdalSupportedFileEditor::Scaling8BitsBlock;
                break;
            case 16 :
                m_pScalePixelFnc = (ScalePixelFncPtr)&HRFGdalSupportedFileEditor::Scaling16BitsBlock;
                break;
            case 32 :
                m_pScalePixelFnc = (ScalePixelFncPtr)&HRFGdalSupportedFileEditor::Scaling32BitsBlock;
                break;
            default :
                HASSERT(0);
            }
        }
    else
        {
        m_pScalePixelFnc = 0;
        }
    }

//-----------------------------------------------------------------------------
// protected
// Read uncompressed 32 bit RGBA Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::ReadIntegerBlock(uint64_t pi_PosBlockX,
                                                     uint64_t pi_PosBlockY,
                                                     Byte* po_pData,
                                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    ComputeIOblockSize((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    Byte* pData = po_pData;

    for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
        {
        Status = ReadBandBlock(pData, BandInd, pi_PosBlockX, pi_PosBlockY);

        if (Status != H_SUCCESS)
            break;

        pData += m_NbBytePerBandPerPixel;
        }

    SisterFileLock.ReleaseKey();

    if (m_UseLinearBandScaling == true)
        {
        (this->*m_pScalePixelFnc)(po_pData);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// protected
// ComputeIOblockSize
// Compute the size of the block to read or write.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::ComputeIOblockSize(uint32_t pi_PosBlockX, uint32_t pi_PosBlockY)
    {
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    if ((pi_PosBlockX + RASTER_FILE->GetBlockWidth()) > (uint32_t)RASTER_FILE->GetImageWidth())
        {
        m_WidthToRead = RASTER_FILE->GetImageWidth() - pi_PosBlockX;
        }
    else
        {
        m_WidthToRead = RASTER_FILE->GetBlockWidth();
        }

    if ((pi_PosBlockY + RASTER_FILE->GetBlockHeight()) > (uint32_t)RASTER_FILE->GetImageHeight())
        {
        m_HeightToRead = RASTER_FILE->GetImageHeight() - pi_PosBlockY;
        }
    else
        {
        m_HeightToRead = RASTER_FILE->GetBlockHeight();
        }
    }


//-----------------------------------------------------------------------------
// protected
// Read uncompressed 96 bit real RGB Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::ReadRealBlock(uint64_t pi_PosBlockX,
                                                  uint64_t pi_PosBlockY,
                                                  Byte* po_pData,
                                                  HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(RASTER_FILE->IsReadPixelReal() == true); //INT not tested
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    ComputeIOblockSize((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_GdalDataType == GDT_Float32)
        {
        HASSERT((m_pTempRealBuffer == 0) ||
                (RASTER_FILE->IsUnsignedPixelTypeForSignedData() == true));
        //Linear scaling must not be done on single band float data image
        //because the data can represent elevation.
        HASSERT((m_UseLinearBandScaling == false) || (m_NbBands > 1));
        float* pTempBuffer;

        if (m_pTempRealBuffer != 0)
            {
            pTempBuffer = (float*)m_pTempRealBuffer.get();
            }
        else
            {
            pTempBuffer = (float*)po_pData;
            }

        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            Status = ReadBandBlock((Byte*)pTempBuffer, BandInd, pi_PosBlockX, pi_PosBlockY);

            if (Status != H_SUCCESS)
                break;

            pTempBuffer++;
            }
        SisterFileLock.ReleaseKey();

        if (Status == H_SUCCESS && m_UseLinearBandScaling == true)
            {
            ScalingFloatBlock(po_pData);
            }
        }
    else
        {
        HASSERT(m_GdalDataType == GDT_Float64);

        double* pTempBuffer = (double*)m_pTempRealBuffer.get();

        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            Status = ReadBandBlock((Byte*)pTempBuffer, BandInd, pi_PosBlockX, pi_PosBlockY);

            if (Status != H_SUCCESS)
                break;

            pTempBuffer++;
            }

        SisterFileLock.ReleaseKey();

        if (Status == H_SUCCESS && m_UseLinearBandScaling == true)
            {
            ScalingDoubleBlock(po_pData);
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                              uint64_t pi_PosBlockY,
                                              Byte* po_pData,
                                              HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(m_pReadBlockFnc != 0);

    return (this->*m_pReadBlockFnc)(pi_PosBlockX,
                                    pi_PosBlockY,
                                    po_pData,
                                    pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// protected
// Scaling8BitsBlock
// Scale a 8 bits block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::Scaling8BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);

    Byte* pData = (Byte*) po_pData;

    for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
        {
        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            *pData = (Byte)((*pData - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
            ++pData;
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// Scaling16BitsBlock
// Scale a 16 bits block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::Scaling16BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);

    if(RASTER_FILE->IsReadPixelSigned())
        {
        short*  pOriData    = (short*)po_pData;
        unsigned short* pScaledData = (unsigned short*)po_pData;

        for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
            {
            for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                {
                *pScaledData = (unsigned short)((*pOriData - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                pScaledData++;
                pOriData++;
                }
            }
        }
    else
        {
        unsigned short* pData = (unsigned short*)po_pData;

        for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
            {
            for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                {
                *pData = (unsigned short)((*pData - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                ++pData;
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// RevertScaling16BitsBlock
// Revert the scaling of a 16 bits block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::RevertScaling16BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(RASTER_FILE->IsReadPixelSigned());

    unsigned short* pScaledData   = (unsigned short*)po_pData;
    short*  pUnscaledData = (short*)po_pData;

    for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
        {
        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            *pUnscaledData = (short)((*pScaledData / m_pBandScaling[BandInd]) + m_pBandMinimum[BandInd]);
            pScaledData++;
            pUnscaledData++;
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// Scaling32BitsBlock
// Scale a 32 bits block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::Scaling32BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);

    if(RASTER_FILE->IsReadPixelSigned())
        {
        int32_t* pOriData = (int32_t*)po_pData;
        uint32_t* pScaledData = (uint32_t*)po_pData;

        for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
            {
            for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                {
                *pScaledData = (uint32_t)((*pOriData - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                pScaledData++;
                pOriData++;
                }
            }
        }
    else
        {
        uint32_t* pData = (uint32_t*) po_pData;

        for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
            {
            for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                {
                *pData = (uint32_t)((*pData - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                ++pData;
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// RevertScaling32BitsBlock
// Revert the scaling of a 32 bits block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::RevertScaling32BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(RASTER_FILE->IsReadPixelSigned());

    uint32_t* pScaledData   = (uint32_t*)po_pData;
    int32_t* pUnscaledData = (int32_t*)po_pData;

    for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
        {
        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            *pUnscaledData = (int32_t)((*pScaledData / m_pBandScaling[BandInd]) + m_pBandMinimum[BandInd]);
            pScaledData++;
            pUnscaledData++;
            }
        }
    }

//-----------------------------------------------------------------------------
// protected
// ScalingFloatBlock
// Scale a float block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::ScalingFloatBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pTempRealBuffer != 0);

    switch (m_NbBitsPerBandPerPixel)
        {
        case 16 :
            {
            uint16_t* pScaledData = (uint16_t*)po_pData;
            float*  pTempRealBuffer = (float*)m_pTempRealBuffer.get();

            for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
                {
                for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                    {
                    *pScaledData = (uint16_t)((*pTempRealBuffer - m_pBandMinimum[BandInd]) * m_pBandScaling[BandInd]);
                    pScaledData++;
                    pTempRealBuffer++;
                    }
                }
            break;
            }
        case 32 :
            {
            uint32_t* pScaledData = (uint32_t*)po_pData;
            float*  pTempRealBuffer = (float*)m_pTempRealBuffer.get();

            for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
                {
                for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                    {
                    *pScaledData = (uint32_t)((*pTempRealBuffer - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                    pScaledData++;
                    pTempRealBuffer++;
                    }
                }
            break;
            }
        default :
            HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// protected
// ScalingDoubleBlock
// Scale a double block.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::ScalingDoubleBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pTempRealBuffer != 0);

    switch (m_NbBitsPerBandPerPixel)
        {
        case 16 :
            {
            uint16_t* pScaledData = (uint16_t*)po_pData;
            double* pTempRealBuffer = (double*)m_pTempRealBuffer.get();

            for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
                {
                for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                    {
                    *pScaledData = (uint16_t)((*pTempRealBuffer - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                    pScaledData++;
                    pTempRealBuffer++;
                    }
                }
            break;
            }
        case 32 :
            {
            uint32_t* pScaledData = (uint32_t*)po_pData;
            double* pTempRealBuffer = (double*)m_pTempRealBuffer.get();

            for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
                {
                for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                    {
                    *pScaledData = (uint32_t)((*pTempRealBuffer - m_pBandMinimum[BandInd])* m_pBandScaling[BandInd]);
                    pScaledData++;
                    }
                }
            break;
            }
        default :
            HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// protected
// Find the minimum and maximum pixel values in an float image.
//-----------------------------------------------------------------------------
void HRFGdalSupportedFileEditor::FindRealPVminMax(double*   po_pMin,
                                                  double*   po_pMax)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(m_NbBands > 0);
    HPRECONDITION(RASTER_FILE->IsReadPixelReal() == true);

    for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
        {
        po_pMin[BandInd] = DBL_MAX;
        po_pMax[BandInd] = -DBL_MAX;
        }

    //Disable the scaling to read only the real data.
    bool IsRequiredScaling = m_UseLinearBandScaling;
    m_UseLinearBandScaling = false;

    uint32_t BlockPosX = 0;
    uint32_t BlockPosY = 0;
    double PixelValue = 0;
    uint64_t PixelInd = 0;

    while (BlockPosX  < (uint32_t)RASTER_FILE->GetImageWidth())
        {
        while (BlockPosY  < (uint32_t)RASTER_FILE->GetImageHeight())
            {
            //The read data will be put in the temporary buffer.
            ReadRealBlock(BlockPosX, BlockPosY, 0, 0);

            if (m_GdalDataType == GDT_Float32)
                {
                for (int RowIndex = 0; RowIndex < m_HeightToRead; RowIndex++)
                    {
                    PixelInd = RowIndex * m_BlockWidth * m_NbBands;

                    for (int ColIndex = 0; ColIndex < m_WidthToRead; ColIndex++)
                        {
                        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                            {
                            PixelValue = ((float*)m_pTempRealBuffer.get())[PixelInd];

                            if (po_pMin[BandInd] > PixelValue)
                                {
                                po_pMin[BandInd] = PixelValue;
                                }

                            if (po_pMax[BandInd] < PixelValue)
                                {
                                po_pMax[BandInd] = PixelValue;
                                }

                            PixelInd++;
                            }
                        }
                    }
                }
            else
                {
                for (int RowIndex = 0; RowIndex < m_HeightToRead; RowIndex++)
                    {
                    PixelInd = RowIndex * m_BlockWidth * m_NbBands;

                    for (int ColIndex = 0; ColIndex < m_WidthToRead; ColIndex++)
                        {
                        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
                            {
                            PixelValue = ((double*)m_pTempRealBuffer.get())[PixelInd];

                            if (po_pMin[BandInd] > PixelValue)
                                {
                                po_pMin[BandInd] = PixelValue;
                                }

                            if (po_pMax[BandInd] < PixelValue)
                                {
                                po_pMax[BandInd] = PixelValue;
                                }

                            PixelInd++;
                            }
                        }
                    }
                }

            BlockPosY += RASTER_FILE->GetBlockHeight();
            }

        BlockPosX += RASTER_FILE->GetBlockWidth();
        }

    m_UseLinearBandScaling = IsRequiredScaling;
    }

//-----------------------------------------------------------------------------
// public
// Write Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                               uint64_t              pi_PosBlockY,
                                               const Byte*          pi_pData,
                                               HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    const Byte* pDataToWrite = pi_pData;

    if ((pi_PosBlockX + RASTER_FILE->GetBlockWidth()) > (uint32_t)RASTER_FILE->GetImageWidth())
        {
        m_WidthToRead = RASTER_FILE->GetImageWidth() - (uint32_t)pi_PosBlockX;
        }
    else
        {
        m_WidthToRead = RASTER_FILE->GetBlockWidth();
        }

    if ((pi_PosBlockY + RASTER_FILE->GetBlockHeight()) > (uint32_t)RASTER_FILE->GetImageHeight())
        {
        m_HeightToRead = RASTER_FILE->GetImageHeight() - (uint32_t)pi_PosBlockY;
        }
    else
        {
        m_HeightToRead = RASTER_FILE->GetBlockHeight();
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (RASTER_FILE->IsUnsignedPixelTypeForSignedData() == true)
        {
        BeStringUtilities::Memcpy(m_pTempSignedBuffer.get(),
                                  m_pResolutionDescriptor->GetBlockSizeInBytes(),
                                  pi_pData,
                                  m_pResolutionDescriptor->GetBlockSizeInBytes());

        //Only when the data are signed that it is possible to edit
        //them.
        if (m_GdalDataType == GDT_Int32)
            {
            RevertScaling32BitsBlock(m_pTempSignedBuffer.get());
            }
        else
            {
            //There should be only to GDAL data type that are
            //signed and are supported in write mode,
            //GDT_Int32 and GDT_Int16.
            HASSERT(m_GdalDataType == GDT_Int16);
            RevertScaling16BitsBlock(m_pTempSignedBuffer.get());
            }

        pDataToWrite = m_pTempSignedBuffer.get();
        }

    for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
        {
        Status = WriteBandBlock(pDataToWrite, BandInd, pi_PosBlockX, pi_PosBlockY);

        if (Status != H_SUCCESS)
            break;

        pDataToWrite += m_NbBytePerBandPerPixel;
        }

    SisterFileLock.ReleaseKey();

    return Status;
    };


//-----------------------------------------------------------------------------
// Protected
// Read band block
// Read a specified band block using GDAL
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::ReadBandBlock  (Byte*           po_pOutBuffer,
                                                    const uint64_t  pi_BandIndex,
                                                    const uint64_t  pi_PosBlockX,
                                                    const uint64_t  pi_PosBlockY)
    {
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    CPLErrorReset();
    m_pRasterBands[pi_BandIndex]->RasterIO(GF_Read,
                                            (uint32_t)pi_PosBlockX,
                                            (uint32_t)pi_PosBlockY,
                                            m_WidthToRead,
                                            m_HeightToRead,
                                            po_pOutBuffer,
                                            m_WidthToRead,
                                            m_HeightToRead,
                                            (GDALDataType)m_GdalDataType,
                                            m_PixelSpaceInBytes,
                                            m_LineSpaceInBytes);


    if ((CPLGetLastErrorNo() != CPLE_None) &&
        ((CPLGetLastErrorType() == CE_Failure) ||
         (CPLGetLastErrorType() == CE_Fatal)))
        {
        switch (CPLGetLastErrorNo())
            {
            case CPLE_OutOfMemory :
                Status = H_NOT_ENOUGH_MEMORY;
                break;
            case CPLE_FileIO :
                Status = H_IOERROR;
                break;
            case CPLE_AssertionFailed : // No Error
                break;
            default :
                Status = H_READ_ERROR;  // Because we're reading,
                // any unmappable error is
                // a read error
                break;
            }

        // Found raster IO read error!
        HASSERT(H_SUCCESS != Status);
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// Protected
// Write band block
// Write a specified band block using GDAL
//-----------------------------------------------------------------------------
HSTATUS HRFGdalSupportedFileEditor::WriteBandBlock (const Byte*    pi_pInBuffer,
                                                    const uint64_t  pi_BandIndex,
                                                    const uint64_t  pi_PosBlockX,
                                                    const uint64_t  pi_PosBlockY)
    {
    HPRECONDITION(pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    CPLErrorReset();
    m_pRasterBands[pi_BandIndex]->RasterIO(GF_Write,
                                           (uint32_t)pi_PosBlockX,
                                           (uint32_t)pi_PosBlockY,
                                           m_WidthToRead,
                                           m_HeightToRead,
                                           const_cast<Byte*>(pi_pInBuffer),
                                           m_WidthToRead,
                                           m_HeightToRead,
                                           (GDALDataType)m_GdalDataType,
                                           m_PixelSpaceInBytes,
                                           m_LineSpaceInBytes);

    if ((CPLGetLastErrorNo() != CPLE_None) &&
        ((CPLGetLastErrorType() == CE_Failure) ||
         (CPLGetLastErrorType() == CE_Fatal)))
        {


            switch (CPLGetLastErrorNo())
            {
            case CPLE_OutOfMemory :
                Status = H_NOT_ENOUGH_MEMORY;
                break;
            case CPLE_FileIO :
                Status = H_IOERROR;
                break;
            case CPLE_AssertionFailed : // No Error
                break;
            default :
                Status = H_WRITE_ERROR; // Because we're writing,
                // any unmappable error is
                // a write error
                break;
            }

        // Found raster IO write error!
        HASSERT(H_SUCCESS != Status);
        }

    return Status;

    }