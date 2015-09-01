//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFErdasImgEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFErdasImgEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFErdasImgEditor.h>
#include <Imagepp/all/h/HRFErdasImgFile.h>
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

HRFErdasImgEditor::HRFErdasImgEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFGdalSupportedFileEditor(pi_rpRasterFile,
                                 pi_Page,
                                 pi_Resolution,
                                 pi_AccessMode)
    {
    HRFErdasImgFile* pErdasFile = (HRFErdasImgFile*)m_pRasterFile.GetPtr();

    if ((GetRasterFile()->GetAccessMode().m_HasCreateAccess == false) &&
        (pErdasFile->AreDataNeedToBeScaled() == true))
        {
        HASSERT(pErdasFile != 0);

        switch (pErdasFile->GetDispRep())
            {
            case MONO:
                {
                double MonoBandMax = 0;
                double MonoBandMin = 0;
                int32_t   Err;

                //Disable the linear scaling if there is only one band in case the data are DEM data.
                if (!pErdasFile->IsReadPixelReal() &&
                    (m_pRasterBands[GRAY_BAND]->GetMetadataItem("STATISTICS_MINIMUM") != 0 ||
                     m_pRasterBands[GRAY_BAND]->GetMetadataItem("STATISTICS_MAXIMUM") != 0))
                    {
                    MonoBandMin = m_pRasterBands[GRAY_BAND]->GetMinimum(&Err);
                    MonoBandMax = m_pRasterBands[GRAY_BAND]->GetMaximum(&Err);

                    if ((MonoBandMax - MonoBandMin) != 0)
                        SetSingleBandLinearScaling(MonoBandMin, MonoBandMax);
                    }
                }
            break;

            case RGB:
                {
                double RedBandMax = 0;
                double GreenBandMax = 0;
                double BlueBandMax = 0;
                double RedBandMin = 0;
                double GreenBandMin = 0;
                double BlueBandMin = 0;
                int32_t   Err;

                if (m_pRasterBands[RED_BAND]->GetMetadataItem("STATISTICS_MINIMUM") != NULL ||
                    m_pRasterBands[RED_BAND]->GetMetadataItem("STATISTICS_MAXIMUM") != NULL ||
                    m_pRasterBands[GREEN_BAND]->GetMetadataItem("STATISTICS_MINIMUM") != NULL ||
                    m_pRasterBands[GREEN_BAND]->GetMetadataItem("STATISTICS_MAXIMUM") != NULL ||
                    m_pRasterBands[BLUE_BAND]->GetMetadataItem("STATISTICS_MINIMUM") != NULL ||
                    m_pRasterBands[BLUE_BAND]->GetMetadataItem("STATISTICS_MAXIMUM") != NULL)
                    {
                    RedBandMin =  m_pRasterBands[RED_BAND]->GetMinimum(&Err);
                    RedBandMax =  m_pRasterBands[RED_BAND]->GetMaximum(&Err);
                    GreenBandMin =  m_pRasterBands[GREEN_BAND]->GetMinimum(&Err);
                    GreenBandMax =  m_pRasterBands[GREEN_BAND]->GetMaximum(&Err);
                    BlueBandMin =  m_pRasterBands[BLUE_BAND]->GetMinimum(&Err);
                    BlueBandMax =  m_pRasterBands[BLUE_BAND]->GetMaximum(&Err);

                    if (pErdasFile->GetBandInd(GCI_AlphaBand) != -1)
                        {
                        double AlphaBandMin =  m_pRasterBands[ALPHA_EXT_BAND]->GetMinimum(&Err);
                        double AlphaBandMax =  m_pRasterBands[ALPHA_EXT_BAND]->GetMaximum(&Err);

                        SetRGBABandLinearScaling(RedBandMin,   RedBandMax,
                                                 GreenBandMin, GreenBandMax,
                                                 BlueBandMin,  BlueBandMax,
                                                 &AlphaBandMin, &AlphaBandMax);
                        }
                    else
                        {
                        SetRGBABandLinearScaling(RedBandMin,   RedBandMax,
                                                 GreenBandMin, GreenBandMax,
                                                 BlueBandMin,  BlueBandMax);
                        }
                    }
                else if (pErdasFile->IsReadPixelReal() == true)
                    {
                    double* pBandMins = new double[m_NbBands];
                    double* pBandMaxs = new double[m_NbBands];

                    FindRealPVminMax(pBandMins, pBandMaxs);

                    if (pErdasFile->GetBandInd(GCI_AlphaBand) != -1)
                        {
                        double AlphaBandMin =  m_pRasterBands[ALPHA_EXT_BAND]->GetMinimum(&Err);
                        double AlphaBandMax =  m_pRasterBands[ALPHA_EXT_BAND]->GetMaximum(&Err);

                        SetRGBABandLinearScaling(pBandMins[RED_BAND],   pBandMaxs[RED_BAND],
                                                 pBandMins[GREEN_BAND], pBandMaxs[GREEN_BAND],
                                                 pBandMins[BLUE_BAND],  pBandMaxs[BLUE_BAND],
                                                 &AlphaBandMin, &AlphaBandMax);
                        }
                    else
                        {
                        SetRGBABandLinearScaling(pBandMins[RED_BAND],   pBandMaxs[RED_BAND],
                                                 pBandMins[GREEN_BAND], pBandMaxs[GREEN_BAND],
                                                 pBandMins[BLUE_BAND],  pBandMaxs[BLUE_BAND]);
                        }

                    delete pBandMins;
                    delete pBandMaxs;
                    }
                }
            break;
            }
        }

    DetermineScalingMethod();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFErdasImgEditor::~HRFErdasImgEditor()
    {
    }