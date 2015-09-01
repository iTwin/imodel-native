//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFNitfEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFNitfEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFNitfEditor.h>
#include <Imagepp/all/h/HRFNitfFile.h>
#include <Imagepp/all/h/HTIFFUtils.h>

#include <ImagePP-GdalLib/gdal_priv.h>

#define BAND_1 0
#define BAND_2 1
#define BAND_3 2
#define BAND_4 3


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
HRFNitfEditor::HRFNitfEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             unsigned short       pi_Resolution,
                             HFCAccessMode         pi_AccessMode)
    : HRFGdalSupportedFileEditor(pi_rpRasterFile,
                                 pi_Page,
                                 pi_Resolution,
                                 pi_AccessMode)
    {
    HRFNitfFile* pNitfFile = (HRFNitfFile*)m_pRasterFile.GetPtr();
    HASSERT(pNitfFile != 0);

    bool UseLinearBandScaling = m_UseLinearBandScaling;

    if(pNitfFile->IsReadPixelReal() && pNitfFile->IsIntegerPixelTypeForRealData())
        {
        switch (pNitfFile->GetDispRep())
            {
            case MONO:
                {
                double GrayBandMax = 0;
                double GrayBandMin = 0;

                FindRealPVminMax(&GrayBandMin, &GrayBandMax);
                SetSingleBandLinearScaling(GrayBandMin, GrayBandMax);
                }
            break;

            case RGB:
                {
                HASSERT(m_NbBands == 3);
                double BandMins[3];
                double BandMaxs[3];

                FindRealPVminMax(BandMins, BandMaxs);

                SetRGBABandLinearScaling(BandMins[RED_BAND],   BandMaxs[RED_BAND],
                                         BandMins[GREEN_BAND], BandMaxs[GREEN_BAND],
                                         BandMins[BLUE_BAND],  BandMaxs[BLUE_BAND]);
                }
            break;
            }
        }
    else
        {
        const char* pNITF_ABPP_String = pNitfFile->GetDataSet()->GetMetadataItem("NITF_ABPP");
        const char* pNITF_PJUST_String = pNitfFile->GetDataSet()->GetMetadataItem("NITF_PJUST");
        
        uint32_t NbSignificatifBits = pNITF_ABPP_String != NULL ? atoi(pNITF_ABPP_String) : 0;

        if (NbSignificatifBits < m_NbBitsPerBandPerPixel && pNITF_PJUST_String != NULL && pNITF_PJUST_String[0] == 'R')
            {
            if (m_pResolutionDescriptor->GetPixelType()->CountIndexBits() == 0)
                {
                HASSERT((m_NbBitsPerBandPerPixel == 16) && ((m_NbBands == 1) || (m_NbBands == 3)));

                double MaxValue = (double)(((uint64_t)1 << NbSignificatifBits) - 1);
                double MinValue = 0;

                if (pNitfFile->IsUnsignedPixelTypeForSignedData())
                    {
                    MaxValue = floor(MaxValue / 2);
                    MinValue = (MaxValue + 1) * -1;
                    }

                if (m_NbBands == 1)
                    {
                    HASSERT(!pNitfFile->IsUnsignedPixelTypeForSignedData());
                    /*Removed because single band unsigned data might be used to represent DEM.
                    SetSingleBandLinearScaling(MinValue, MaxValue);
                    */
                    }
                else if (m_NbBands == 3)
                    {
                    SetRGBABandLinearScaling(MinValue, MaxValue,    //Red Band
                                             MinValue, MaxValue,    //Green Band
                                             MinValue, MaxValue);   //Blue Band
                    }
                }
            else
                {
                HASSERT((m_NbBitsPerBandPerPixel == 8) && (m_NbBands == 1));
                m_Mask = (1 << NbSignificatifBits) - 1;
                m_pScalePixelFnc = (ScalePixelFncPtr)&HRFNitfEditor::Masking8BitsBlock;
                m_UseLinearBandScaling = true;
                //The scaling method is determined here, so set UseLinearBandScaling
                //to true to ensure that DetermineScalingMethod won't be called at
                //the end of the constructor.
                UseLinearBandScaling = true;
                }
            }
        }

    if ((m_UseLinearBandScaling == true) && (UseLinearBandScaling == false))
        {
        DetermineScalingMethod();
        }
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFNitfEditor::~HRFNitfEditor()
    {
    }

//-----------------------------------------------------------------------------
// protected
// Masking8BitsBlock
//-----------------------------------------------------------------------------
void HRFNitfEditor::Masking8BitsBlock(Byte* po_pData)
    {
    HPRECONDITION(po_pData != 0);

    Byte* pData = (Byte*) po_pData;

    for (uint64_t PixelIndex = 0; PixelIndex < m_NbPixelsPerBlock; PixelIndex++)
        {
        for (uint32_t BandInd = 0; BandInd < m_NbBands; BandInd++)
            {
            *pData &= (Byte)m_Mask;
            ++pData;
            }
        }
    }