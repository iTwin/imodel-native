//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsSDTSDEMEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFUSgsSDTSDEMEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFUSgsSDTSDEMEditor.h>

/*
#include <ImagePP-GdalLib_priv.h>

#define BAND_1 0
#define BAND_2 1
#define BAND_3 2
#define BAND_4 3
*/

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
HRFUSgsSDTSDEMEditor::HRFUSgsSDTSDEMEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                           uint32_t              pi_Page,
                                           unsigned short       pi_Resolution,
                                           HFCAccessMode         pi_AccessMode)
    : HRFGdalSupportedFileEditor(pi_rpRasterFile,
                                 pi_Page,
                                 pi_Resolution,
                                 pi_AccessMode)
    {
    //Return the raw data
    m_UseLinearBandScaling = false;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFUSgsSDTSDEMEditor::~HRFUSgsSDTSDEMEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFUSgsSDTSDEMEditor::ReadBlock(uint64_t                 pi_PosBlockX,
                                        uint64_t                 pi_PosBlockY,
                                        Byte*                   po_pData,
                                        HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HSTATUS ReadStatus = T_Super::ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
    if(ReadStatus != H_SUCCESS)
        return ReadStatus;

    HFCPtr<HRPPixelType> pPixelType = GetResolutionDescriptor()->GetPixelType();

    // For now threat background data as no data.
    if(pPixelType->GetChannelOrg().CountChannels() == 1 &&
       pPixelType->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        {
        const HRPChannelType* pChannelType = pPixelType->GetChannelOrg().GetChannelPtr(0);

        switch(pChannelType->GetSize())
            {
            case 16:
                switch(pChannelType->GetDataType())
                    {
                    case HRPChannelType::INT_CH:
                    case HRPChannelType::VOID_CH:
                        ReplacePixelsWithNoDataValue<uint16_t>(po_pData, (uint16_t)USGS_SDTS_BACKGROUND_VALUE, *pChannelType->GetNoDataValue());
                        break;
                    case HRPChannelType::SINT_CH:
                        struct ReplaceWithNoData
                            {
                            int16_t m_PositiveNoDataValue;
                            explicit ReplaceWithNoData (double pi_NoDataValue) : m_PositiveNoDataValue((int16_t)abs(pi_NoDataValue)) {}

                            bool operator () (int16_t pi_PixelValue) const
                                {
                                return pi_PixelValue == USGS_SDTS_BACKGROUND_VALUE || pi_PixelValue == m_PositiveNoDataValue;
                                }
                            };

                        ReplacePixelsWithNoDataValueIf<int16_t>(po_pData, ReplaceWithNoData(*pChannelType->GetNoDataValue()), *pChannelType->GetNoDataValue());
                        break;
                    default:
                        break;
                    }
                break;
            case 32:
                switch(pChannelType->GetDataType())
                    {
                    case HRPChannelType::INT_CH:
                    case HRPChannelType::VOID_CH:
                        ReplacePixelsWithNoDataValue<uint32_t>(po_pData, (uint32_t)USGS_SDTS_BACKGROUND_VALUE, *pChannelType->GetNoDataValue());
                        break;
                    case HRPChannelType::SINT_CH:
                        ReplacePixelsWithNoDataValue<int32_t>(po_pData, USGS_SDTS_BACKGROUND_VALUE, *pChannelType->GetNoDataValue());
                        break;
                    case HRPChannelType::FLOAT_CH:
                        ReplacePixelsWithNoDataValue<float>(po_pData, USGS_SDTS_BACKGROUND_VALUE, *pChannelType->GetNoDataValue());
                        break;
                    default:
                        break;
                    }
                break;
            default:
                break;
            }
        }

    return ReadStatus;
    }


//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
template<class T>
void HRFUSgsSDTSDEMEditor::ReplacePixelsWithNoDataValue(Byte* pio_pData, T pi_PixelValue, double pi_NoDataValue) const
    {
    // We assumed that HRFUSgsSDTS always have a block type line. If not, the following code need to be adapted to take into
    // account the possibility of an incomplete last block.
    HPRECONDITION(m_BlockWidth == m_NbPixelsPerBlock);
    HPRECONDITION(1 == m_NbBands);

    T* pData           = (T*)pio_pData;
    T  NoDataValue     = (T)pi_NoDataValue;

    std::replace(pData, pData + m_NbPixelsPerBlock, pi_PixelValue, NoDataValue);
    }


template<class T, class Pred>
void HRFUSgsSDTSDEMEditor::ReplacePixelsWithNoDataValueIf(Byte* pio_pData, Pred pi_Predicate, double pi_NoDataValue) const
    {
    // We assumed that HRFUSgsSDTS always have a block type line. If not, the following code need to be adapted to take into
    // account the possibility of an incomplete last block.
    HPRECONDITION(m_BlockWidth == m_NbPixelsPerBlock);
    HPRECONDITION(1 == m_NbBands);

    T* pData           = (T*)pio_pData;
    T  NoDataValue     = (T)pi_NoDataValue;

    std::replace_if(pData, pData + m_NbPixelsPerBlock, pi_Predicate, NoDataValue);
    }