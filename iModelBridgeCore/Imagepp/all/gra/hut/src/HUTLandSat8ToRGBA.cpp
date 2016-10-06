//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTLandSat8ToRGBA.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFUtility implementation
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HUTLandSat8ToRGBA.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HIMFilteredImage.h>
#include <Imagepp/all/h/HRFRasterFileExtender.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRPContrastStretchFilter16.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Donald.Morissette 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t Count_T>
bool NativeScanHisto(vector<double>& minValues, vector<double>& maxValues, Byte const* pData, uint32_t width, uint32_t height,
                     double const* pNoDataValue=NULL, double HistoCut=0.0)
    {
    size_t pitch = width*sizeof(Data_T)*Count_T;

    Data_T NoDataValueTyped = pNoDataValue ? (Data_T) *pNoDataValue : 0;
    Data_T minValueTyped[Count_T];
    Data_T maxValueTyped[Count_T];
    bool isValidMin[Count_T];
    bool isValidMax[Count_T];

    // Compute histogram 8bit and 16bit only
    Data_T* Histo[Count_T];
    bool HistoOn = false;
    if (0.0 != HistoCut && sizeof(Data_T) <= 2)
        {
        HistoOn = true;
        for (int i=0; i<Count_T; ++i)
            {
            Histo[i] = new Data_T[(size_t)std::numeric_limits<Data_T>::max()];
            memset(Histo[i], 0, (size_t)std::numeric_limits<Data_T>::max()*sizeof(Data_T));
            }
        }

    for (uint32_t channel = 0; channel < Count_T; ++channel)
        {
        isValidMin[channel] = false;
        isValidMax[channel] = false;
        minValueTyped[channel] = std::numeric_limits<Data_T>::max();
        maxValueTyped[channel] = std::numeric_limits<Data_T>::lowest(); // min doesn't give negative value for float!
        }

    for (uint32_t line = 0; line < height; ++line)
        {
        Data_T const* pDataLine = (Data_T const*) (pData + line*pitch);

        for (uint32_t column = 0; column < width; ++column)
            {
            for (uint32_t channel = 0; channel < Count_T; ++channel)
                {
                if (HistoOn)
                    ++(Histo[channel][(size_t)pDataLine[(column*Count_T) + channel]]);

                if (pNoDataValue != NULL && NoDataValueTyped == pDataLine[(column*Count_T) + channel])
                    continue;

                if (!isValidMin[channel] || pDataLine[(column*Count_T) + channel] < minValueTyped[channel])
                    {
                    minValueTyped[channel] = pDataLine[(column*Count_T) + channel];
                    isValidMin[channel] = true;
                    }

                if (!isValidMax[channel] || pDataLine[(column*Count_T) + channel] > maxValueTyped[channel])
                    {
                    maxValueTyped[channel] = pDataLine[(column*Count_T) + channel];
                    isValidMax[channel] = true;
                    }
                }
            }
        }

    minValues.resize(Count_T);
    maxValues.resize(Count_T);

    bool allValid = true;
    for (uint32_t channel = 0; channel < Count_T; ++channel)
        {
        minValues[channel] = isValidMin[channel] ? minValueTyped[channel] : std::numeric_limits<Data_T>::lowest();
        maxValues[channel] = isValidMax[channel] ? maxValueTyped[channel] : std::numeric_limits<Data_T>::max();

        allValid = allValid && isValidMin[channel] && isValidMax[channel];

        if (HistoOn && allValid)
            { //NoDataValueTyped
            size_t MaxPixeltoRemove = (size_t)((width * height) * HistoCut);
            size_t SumPixel = 0;
            for (uint32_t i = (uint32_t)minValues[channel]; i < (uint32_t)std::numeric_limits<Data_T>::max(); ++i)
                {
                SumPixel += (size_t)Histo[channel][i];
                if (SumPixel < MaxPixeltoRemove)
                    minValues[channel] = (i+1);
                else
                    break;
                }

            SumPixel = 0;
            for (uint32_t i = (uint32_t)maxValues[channel]; i >= 1; --i)
                {
                SumPixel += (size_t)Histo[channel][i];
                if (SumPixel < MaxPixeltoRemove)
                    maxValues[channel] = (i-1);
                else
                    break;
                }
            }

        }

    if (HistoOn)
        {
        for (int i = 0; i < Count_T; ++i)
            delete Histo[i];
        }

    return allValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Donald.Morissette 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComputeMinMaxValuesHisto(vector<double>& minValues, vector<double>& maxValues, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber,
                              double  const* pNodataValue=NULL, double HistoCut=0.0)
    {
    // If Min.max already set in the file natively, we take them.
    HRFAttributeMinSampleValue const* pMinValueTag = pRasterFile->GetPageDescriptor(pageNumber)->FindTagCP<HRFAttributeMinSampleValue>();
    HRFAttributeMaxSampleValue const* pMaxValueTag = pRasterFile->GetPageDescriptor(pageNumber)->FindTagCP<HRFAttributeMaxSampleValue>();
    if (pMinValueTag != NULL && pMaxValueTag != NULL)
        {
        minValues = pMinValueTag->GetData();
        maxValues = pMaxValueTag->GetData();
        return true;
        }

    // Need to scan the file
    HFCPtr<HRFRasterFile> pTrueRasterFile = pRasterFile;
    while (pTrueRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pTrueRasterFile = ((HRFRasterFileExtender*)pTrueRasterFile.GetPtr())->GetOriginalFile();

    uint32_t preferedWidth = std::min<uint32_t>((uint32_t) pTrueRasterFile->GetPageDescriptor(pageNumber)->GetResolutionDescriptor(0)->GetWidth(), 1024);
    uint32_t preferedHeight = std::min<uint32_t>((uint32_t) pTrueRasterFile->GetPageDescriptor(pageNumber)->GetResolutionDescriptor(0)->GetWidth(), 1024);
    HFCPtr<HRFThumbnail> pThumbnail = HRFThumbnailMaker(pTrueRasterFile, pageNumber, &preferedWidth, &preferedHeight, false);

    bool succeeded = true; 
    switch (pThumbnail->GetPixelType()->GetClassID())
        {
            case HRPPixelTypeId_V16Gray16:
                succeeded = NativeScanHisto<uint16_t, 1>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), 
                                                     pNodataValue, HistoCut);
                break;

            case HRPPixelTypeId_V48R16G16B16:
                succeeded = NativeScanHisto<uint16_t, 3>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), 
                                                     pNodataValue, HistoCut);
                break;

            default:
                succeeded = false;
                break;
        }

    return succeeded;
    }


//-----------------------------------------------------------------------------
// public
// LandSat8ToRGBA
//
// .xch file is often used.
//
// UseSameMinMax --> if true, the min of all channel and the max of all channel will be used, instead of of each min max by channel.
// HistoCut      --> 0.0  : native scan
//               --> 0.01 : Computer histogram, increase the min and reduce the max value until (NbTotalPixel*0.01) is gotten, each side separatly.
//                                        min-->....<--max
//  
//-----------------------------------------------------------------------------            
HFCPtr<HRARaster> ImagePP::HUTLandSat8ToRGBA(HFCPtr<HRARaster>& pRaster, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber,
                                             double  const* pNodataValue, double HistoCut, bool UseSameMinMax)
    {
    HPRECONDITION(pRaster->GetPixelType()->GetChannelOrg().CountChannels() == 3);
    HPRECONDITION(pRaster->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize() == 16);

    vector<double> minValues, maxValues;

    if (!ComputeMinMaxValuesHisto(minValues, maxValues, pRasterFile, pageNumber, pNodataValue, HistoCut))
        return pRaster;

    if (UseSameMinMax)
        {
        if (pRaster->GetPixelType()->GetChannelOrg().CountChannels() == 3)
            {
            minValues[0] = min(minValues[0], min (minValues[1], minValues[2]));
            minValues[1] = minValues[0];
            minValues[2] = minValues[0];
    
            maxValues[0] = max(maxValues[0], max (maxValues[1], maxValues[2]));
            maxValues[1] = maxValues[0];
            maxValues[2] = maxValues[0];
            }
        }
    
    HFCPtr<HRPContrastStretchFilter16> pFilter = new HRPContrastStretchFilter16(pRaster->GetPixelType());
    double gamma = 1.0;
    for (unsigned short i = 0; i < pRaster->GetPixelType()->GetChannelOrg().CountChannels(); ++i)
        {
        pFilter->SetInterval(i, (int32_t) minValues[i], (int32_t) maxValues[i]);
        pFilter->SetGammaFactor(i, gamma);  
        }

    // 0,0,0 --> is necessary transparent by definition of landsat8 specification.
    ListHRPAlphaRange listAlphaRanges;
    HRPAlphaRange alphaRange(0,0,0, 0,0,0, 0);
    listAlphaRanges.push_back(alphaRange);
    HFCPtr<HRPAlphaComposer> pAlphaComposer(new HRPAlphaComposer(255, listAlphaRanges));
    HFCPtr<HRARaster> pFilteredImage = new HIMFilteredImage(pRaster, (HFCPtr<HRPFilter>&)pAlphaComposer);

    return new HIMFilteredImage(pFilteredImage, pFilter.GetPtr());
    }

