//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageOpFunctionFilters.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HRAImageOpFunctionFilters.h>
#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>


#define CLAMP(A)((A)<=(0) ? (0) : (A)<(256) ? (A) : (255))



/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpPixelReplacerFilter-----------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t ChannelCount_T>
struct PixelReplacer_T
    {
    static inline bool AreEqual(Data_T const* pValue1, Data_T const* pValue2){/*Need to implement specialization*/}
    static inline void Copy(Data_T* pDest, Data_T const* pSrc) {/*Need to implement specialization*/}
    };

template<class Data_T>
struct PixelReplacer_T<Data_T, 1>
    {
    static inline bool AreEqual(Data_T const* pValue1, Data_T const* pValue2) {return *pValue1 == *pValue2;}
    static inline void Copy(Data_T* pDest, Data_T const* pSrc) {*pDest = *pSrc;}
    };

template<class Data_T>
struct PixelReplacer_T<Data_T, 3>
    {
    static inline bool AreEqual(Data_T const* pValue1, Data_T const* pValue2) {return pValue1[0] == pValue2[0] && pValue1[1] == pValue2[1] && pValue1[2] == pValue2[2];}
    static inline void Copy(Data_T* pDest, Data_T const* pSrc) {pDest[0] = pSrc[0]; pDest[1] = pSrc[1]; pDest[2] = pSrc[2];}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPixelReplacerFilterPtr HRAImageOpPixelReplacerFilter::CreatePixelReplacer(HRPPixelType const& pixeltype)
    {  
    HPRECONDITION(pixeltype.CountPixelRawDataBits() % 8 == 0);        // Must byte aligned.

    return new HRAImageOpPixelReplacerFilter(pixeltype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPixelReplacerFilter::HRAImageOpPixelReplacerFilter(HRPPixelType const& pixeltype) 
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
     m_pPixelType((HRPPixelType*)pixeltype.Clone())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPixelReplacerFilter::~HRAImageOpPixelReplacerFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpPixelReplacerFilter::SetValue(void const* pValue, size_t size)
    {
    HPRECONDITION(size == m_pPixelType->CountPixelRawDataBits() / 8);

    m_oldValue.resize(m_pPixelType->CountPixelRawDataBits() / 8);
    memcpy(m_oldValue.data(), pValue, m_oldValue.size());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpPixelReplacerFilter::SetNewValue(void const* pValue, size_t size)
    {
    HPRECONDITION(size == m_pPixelType->CountPixelRawDataBits() / 8);

    m_newValue.resize(m_pPixelType->CountPixelRawDataBits() / 8);
    memcpy(m_newValue.data(), pValue, m_newValue.size());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelReplacerFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = (HRPPixelType*)m_pPixelType->Clone();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelReplacerFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;   

    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = (HRPPixelType*)m_pPixelType->Clone();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelReplacerFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If OUTPUT is already provided they must be of same type.
    if(GetOutputPixelType() != NULL && !GetOutputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    if (pixelType->CountIndexBits() % 8 != 0 || !pixelType->IsCompatibleWith(m_pPixelType->GetClassID()))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pInputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelReplacerFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If INPUT is already provided they must be of same type.
    if(GetInputPixelType() != NULL && !GetInputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;
        
    if (pixelType->CountIndexBits() % 8 != 0 || !pixelType->IsCompatibleWith(m_pPixelType->GetClassID()))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pOutputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t Count_T> 
ImagePPStatus HRAImageOpPixelReplacerFilter::Process_T(HRAImageSampleR outData, HRAImageSampleCR inputData)
    {
#ifndef DISABLE_BUFFER_GETDATA
    HPRECONDITION(m_oldValue.size() == sizeof(Data_T)*Count_T);
    HPRECONDITION(m_newValue.size() == sizeof(Data_T)*Count_T);

    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    Data_T const* pOldValue = (Data_T const*)m_oldValue.data();
    Data_T const* pNewValue = (Data_T const*)m_newValue.data();

    for(uint32_t line=0; line < outHeight; ++line)
        {
        Data_T const* pInBufferLine  = (Data_T const*)(pInBuffer+line*inPitch);
        Data_T*       pOutBufferLine = (Data_T*)(pOutBuffer+line*outPitch);

        for(uint32_t column=0; column < outWidth; ++column)
            {
            if(PixelReplacer_T<Data_T, Count_T>::AreEqual(pInBufferLine + (column*Count_T), pOldValue))
                PixelReplacer_T<Data_T, Count_T>::Copy(pOutBufferLine + (column*Count_T), pNewValue);
            else
                PixelReplacer_T<Data_T, Count_T>::Copy(pOutBufferLine + (column*Count_T), pInBufferLine + (column*Count_T));
            }
        }
#endif
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpPixelReplacerFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth());
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight());
    HPRECONDITION(!m_oldValue.empty() && m_oldValue.size() == m_newValue.size());
    
    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    switch(m_pPixelType->GetClassID())
        {
        //***  We do not care about the pixel representation all we need is the pixelsize.

        // 8 bits
        case HRPPixelTypeId_I8R8G8B8:
        case HRPPixelTypeId_I8R8G8B8A8:
        case HRPPixelTypeId_I8VA8R8G8B8:    
        case HRPPixelTypeId_I8Gray8:
        case HRPPixelTypeId_V8Gray8:
        case HRPPixelTypeId_V8GrayWhite8:
            status = Process_T<uint8_t, 1>(outData, inputData);
            break;

        // 16 bits
        case HRPPixelTypeId_V16Gray16:
        case HRPPixelTypeId_V16PRGray8A8:
        case HRPPixelTypeId_V16Int16:
            status = Process_T<uint16_t, 1>(outData, inputData);
            break;
       
        // 24 bits
        case HRPPixelTypeId_V24R8G8B8:
        case HRPPixelTypeId_V24B8G8R8:
        case HRPPixelTypeId_V24PhotoYCC:
            status = Process_T<uint8_t, 3>(outData, inputData);
            break;

        // 32 bits-float.
        // We assumed a perfect bit pattern match. Might need to introduce a configurable epsilon at some point.
        case HRPPixelTypeId_V32Float32:  
            // FALL to 32 bits integer case.
        // 32 bits
        case HRPPixelTypeId_V32PRPhotoYCCA8:
        case HRPPixelTypeId_V32PR8PG8PB8A8:
        case HRPPixelTypeId_V32A8R8G8B8:
        case HRPPixelTypeId_V32CMYK:
        case HRPPixelTypeId_V32R8G8B8A8:
        case HRPPixelTypeId_V32B8G8R8X8:
        case HRPPixelTypeId_V32R8G8B8X8:
            status = Process_T<uint32_t, 1>(outData, inputData);
            break;

        // 48 bits
        case HRPPixelTypeId_V48R16G16B16:
            status = Process_T<uint16_t, 3>(outData, inputData);
            break;
    
        // 64 bits
        case HRPPixelTypeId_V64R16G16B16A16:
        case HRPPixelTypeId_V64R16G16B16X16:
            status = Process_T<uint64_t, 1>(outData, inputData);
            break;

        // 96 bits.
        case HRPPixelTypeId_V96R32G32B32:
            status = Process_T<uint32_t, 3>(outData, inputData);
            break;                  
        
        // not supported.
        //case HRPPixelTypeId_I1R8G8B8:
        //case HRPPixelTypeId_I1R8G8B8:
        //case HRPPixelTypeId_I1R8G8B8RLE:
        //case HRPPixelTypeId_I1R8G8B8A8:
        //case HRPPixelTypeId_I1R8G8B8A8RLE:
        //case HRPPixelTypeId_I2R8G8B8:
        //case HRPPixelTypeId_I4R8G8B8:
        //case HRPPixelTypeId_I4R8G8B8A8:
        //case HRPPixelTypeId_V1Gray1:
        //case HRPPixelTypeId_V1GrayWhite1:
        //case HRPPixelTypeId_V16B5G5R5:
        //case HRPPixelTypeId_V16R5G6B5:
        //case HRPPixelTypeId_I8R8G8B8Mask:
        default:
            status = IMAGEPP_STATUS_UnknownError;
            break;
        }
    
    return IMAGEPP_STATUS_Success;
    }


/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpColorReplacerFilter-----------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(const Byte pi_newRGBColor[3],
                                                                                 const RGBSetList& selectedRGBSet,
                                                                                 const RGBSetList& selectedRemoveRGBSet)
    {  
    HRAImageOpColorReplacerFilter* pFilter = new HRAImageOpColorReplacerFilter(pi_newRGBColor, selectedRGBSet, selectedRemoveRGBSet);
    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpColorReplacerFilter::HRAImageOpColorReplacerFilter(const Byte pi_newRGBColor[3],
                                                             const RGBSetList& selectedRGBSet,
                                                             const RGBSetList& selectedRemoveRGBSet) :
    HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
    m_RGBSetList(selectedRGBSet),
    m_RGBSetRemoveList(selectedRemoveRGBSet)
    {
    m_NewRGBColor[0] = pi_newRGBColor[0];
    m_NewRGBColor[1] = pi_newRGBColor[1];
    m_NewRGBColor[2] = pi_newRGBColor[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpColorReplacerFilter::~HRAImageOpColorReplacerFilter()
    {
    m_RGBCubeList.clear();
    m_RGBSetList.clear();
    m_LUVCubeList.clear();

    m_RGBCubeRemoveList.clear();
    m_RGBSetRemoveList.clear();
    m_LUVCubeRemoveList.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColorReplacerFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = new HRPPixelTypeV24R8G8B8();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColorReplacerFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;   

    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = new HRPPixelTypeV24R8G8B8();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColorReplacerFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If OUTPUT is already provided they must be of same type.
    if(GetOutputPixelType() != NULL && !GetOutputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    if (!pixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pInputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColorReplacerFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If INPUT is already provided they must be of same type.
    if(GetInputPixelType() != NULL && !GetInputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;
        
    if (!pixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pOutputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColorReplacerFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
#ifndef DISABLE_BUFFER_GETDATA
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth());
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight());

    // Can't operate on en empty list!
    HPRECONDITION(m_RGBCubeList.size() > 0 || (m_RGBSetList.size() > 0) || (m_LUVCubeList.size() > 0));

    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    bool  PixelFound;
    uint32_t Width = outData.GetWidth();
    uint32_t Height = outData.GetHeight();

    for (uint32_t row = 0; row < Height; ++row)
        {
        Byte const* pInBufferRow = pInBuffer + row*inPitch;
        Byte* pOutBufferRow = (Byte*)(pOutBuffer + row*outPitch);

        for (uint32_t column = 0; column < Width; ++column)
            {
            PixelFound = LookForColor(pInBufferRow[column * 3], pInBufferRow[column * 3 + 1], pInBufferRow[column * 3 + 2]);

            // If color has been found into the ColorSet , change it!
            if (PixelFound)
                {
                pOutBufferRow[column * 3] = m_NewRGBColor[0];
                pOutBufferRow[column * 3 + 1] = m_NewRGBColor[1];
                pOutBufferRow[column * 3 + 2] = m_NewRGBColor[2];
                }
            else
                {
                pOutBufferRow[column * 3] = pInBufferRow[column * 3];
                pOutBufferRow[column * 3 + 1] = pInBufferRow[column * 3 + 1];
                pOutBufferRow[column * 3 + 2] = pInBufferRow[column * 3 + 2];
                }
            }
        }
#endif
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::AddColors(const HGFRGBCube& pi_rCube)
    {
    m_RGBCubeList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::AddColors(const HGFRGBSet& pi_rCube)
    {
    m_RGBSetList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::AddColors(const HGFLUVCube& pi_rCube)
    {
    m_LUVCubeList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::RemoveColors(const HGFRGBCube& pi_rCube)
    {
    m_RGBCubeRemoveList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::RemoveColors(const HGFRGBSet& pi_rCube)
    {
    m_RGBSetRemoveList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::RemoveColors(const HGFLUVCube& pi_rCube)
    {
    m_LUVCubeRemoveList.push_back(pi_rCube);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpColorReplacerFilter::SetNewColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue)
    {
    m_NewRGBColor[0] = pi_Red;
    m_NewRGBColor[1] = pi_Green;
    m_NewRGBColor[2] = pi_Blue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte* HRAImageOpColorReplacerFilter::GetNewColor() const
    {
    return m_NewRGBColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageOpColorReplacerFilter::LookForColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const
    {
    bool PixelFound = false;

    RGBCubeList::const_iterator RGBCudeItr = m_RGBCubeList.begin();
    RGBSetList::const_iterator  RGBSetItr  = m_RGBSetList.begin();
    LUVCubeList::const_iterator LUVCubeItr = m_LUVCubeList.begin();

    RGBCubeList::const_iterator RGBCudeRemoveItr = m_RGBCubeRemoveList.begin();
    RGBSetList::const_iterator  RGBSetRemoveItr  = m_RGBSetRemoveList.begin();
    LUVCubeList::const_iterator LUVCubeRemoveItr = m_LUVCubeRemoveList.begin();

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Looking into RGBCubeList
    while(!PixelFound && (RGBCudeItr != m_RGBCubeList.end()))
        {
        PixelFound = (*RGBCudeItr).IsIn(pi_Red, pi_Green, pi_Blue);

        // Next ColorSet.
        RGBCudeItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Looking into RGBSetList if not already found
    while(!PixelFound && (RGBSetItr != m_RGBSetList.end()))
        {
        PixelFound = (*RGBSetItr).IsIn(pi_Red, pi_Green, pi_Blue);

        // Next ColorSet.
        RGBSetItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Looking into LUVCubeList if not already found
    while(!PixelFound && (LUVCubeItr != m_LUVCubeList.end()))
        {
        PixelFound = (*LUVCubeItr).IsIn(pi_Red, pi_Green, pi_Blue);

        // Next ColorSet.
        LUVCubeItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // If Color has been been found, verify if the user has removed it...
    while(PixelFound && (RGBCudeRemoveItr != m_RGBCubeRemoveList.end()))
        {
        PixelFound = !((*RGBCudeRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        RGBCudeRemoveItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // If Color has been been found, verify if the user has removed it...
    while(PixelFound && (RGBSetRemoveItr != m_RGBSetRemoveList.end()))
        {
        PixelFound = !((*RGBSetRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        RGBSetRemoveItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // If Color has been been found, verify if the user has removed it...
    while(PixelFound && (LUVCubeRemoveItr != m_LUVCubeRemoveList.end()))
        {
        PixelFound = !((*LUVCubeRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        LUVCubeRemoveItr++;
        }

    return PixelFound;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpColortwistFilter--------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpPtr HRAImageOpColortwistFilter::CreateColortwistFilter(const double matrix[4][4])
    {  
    HRAImageOpColortwistFilter* pFilter = new HRAImageOpColortwistFilter(matrix);
    return pFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpColortwistFilter::HRAImageOpColortwistFilter(const double matrix[4][4]) :
    HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0))
    {
    memcpy(m_matrix, matrix, 4 * 4 * sizeof(double));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpColortwistFilter::~HRAImageOpColortwistFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const double* HRAImageOpColortwistFilter::GetMatrix() const
    {
    return m_matrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColortwistFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;

    // If we have an output, input must be of same type.
    if(GetOutputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetOutputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = new HRPPixelTypeV24PhotoYCC();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColortwistFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;   

    // If we have an input, output must be of same type.
    if(GetInputPixelType() != NULL)
        {
        pixelType = (HRPPixelType*)GetInputPixelType()->Clone();
        return IMAGEPP_STATUS_Success;
        }
    
    pixelType = new HRPPixelTypeV24PhotoYCC();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColortwistFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If OUTPUT is already provided they must be of same type.
    if(GetOutputPixelType() != NULL && !GetOutputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;

    if (!pixelType->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pInputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColortwistFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    // If INPUT is already provided they must be of same type.
    if(GetInputPixelType() != NULL && !GetInputPixelType()->HasSamePixelInterpretation(*pixelType))
        return IMAGEOP_STATUS_InvalidPixelType;
        
    if (!pixelType->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID))
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pOutputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonathan.Bernier  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpColortwistFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
#ifndef DISABLE_BUFFER_GETDATA
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth());
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight());

    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t Width = outData.GetWidth();
    uint32_t Height = outData.GetHeight();

    for(uint32_t row=0; row < Height; ++row)
        {
        Byte const* pInBufferRow  = pInBuffer+row*inPitch;
        Byte* pOutBufferRow = (Byte*)(pOutBuffer+row*outPitch);

        for(uint32_t column=0; column < Width; ++column)
            {
            // normalize PhotoYCC
            double Lin  = 1.3584 * pInBufferRow[column*3];
            double C1in = 2.2179 * (pInBufferRow[column*3+1] - 156);
            double C2in = 1.8215 * (pInBufferRow[column*3+2] - 137);

            double Lout  = m_matrix[0] * Lin + m_matrix[1] * C1in + m_matrix[2] * C2in + m_matrix[3];
            double C1out = m_matrix[4] * Lin + m_matrix[5] * C1in + m_matrix[6] * C2in + m_matrix[7];
            double C2out = m_matrix[8] * Lin + m_matrix[9] * C1in + m_matrix[10] * C2in + m_matrix[11];

            // reconvert the output in PhotoYCC 8 
            int32_t L8 =  (int32_t)(Lout / 1.3584);
            int32_t C18 = (int32_t)((C1out / 2.2179) + 156);
            int32_t C28 = (int32_t)((C2out / 1.8215) + 137);

            pOutBufferRow[column*3]   = (Byte)CLAMP(L8);
            pOutBufferRow[column * 3 + 1] = (Byte)CLAMP(C18);
            pOutBufferRow[column * 3 + 2] = (Byte)CLAMP(C28);
            }
        }
#endif
    return IMAGEPP_STATUS_Success;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*------------------------------------General methods for alpha filters----------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static HFCPtr<HRPPixelType> s_GetSuitableOutputPixelTypeAlpha(const HFCPtr<HRPPixelType>& pixelType)
    {
    if (pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        // This pixel type already contains an alpha channel. So, this pixel type is suitable for output.
        return pixelType;
        }

    // Let's see if we can add an alpha channel to this pixel type and obtain a valid pixel type.
    // E.g. RGB->RGBA is ok.
    //      1bit-> ???. Not ok.
    HFCPtr<HRPChannelOrg> pChannelOrg(new HRPChannelOrg(pixelType->GetChannelOrg()));
    pChannelOrg->AddChannel(HRPChannelType(HRPChannelType::ALPHA, HRPChannelType::INT_CH, 8, 0));
    HFCPtr<HRPPixelType> pNewPixelType = HRPPixelTypeFactory::GetInstance()->Create(*pChannelOrg, 0);

    // Make a last check. For V24B8G8R8 pixel type, HRPPixelTypeFactory can't create a "V32B8G8R8A8" pixel type,
    // but a suitable output pixel type is RGBA for this type.
    if (pixelType->IsCompatibleWith(HRPPixelTypeV24B8G8R8::CLASS_ID))
        pNewPixelType = new HRPPixelTypeV32R8G8B8A8();

    // If a valid pixel type can't be created, pNewPixelType will be NULL
    return pNewPixelType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ImagePPStatus s_GetAvailableAlphaInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, HFCPtr<HRPPixelType> pixelTypeToMatch, const HFCPtr<HRPPixelType> pOutputPixelType)
    {
    if(pOutputPixelType != NULL)
        {
        // Output pixel type is set. Let's use the same one for input
        pixelType = (HRPPixelType*)pOutputPixelType->Clone();
        return IMAGEPP_STATUS_Success;
        }

    // No output pixel type is set. Use best match for pixelTypeToMatch
    if (index == 0)
        {
        if (pixelTypeToMatch != NULL)
            {
            if (pixelTypeToMatch->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
                {
                // The pixel type to match contains an alpha channel. Suggest an input with an alpha channel.
                pixelType = new HRPPixelTypeV32R8G8B8A8();
                return IMAGEPP_STATUS_Success;
                }
            else
                {
                // Suggest an input with no alpha channel.
                pixelType = new HRPPixelTypeV24R8G8B8();
                return IMAGEPP_STATUS_Success;
                }
            }
        else
            {
            // No pixel type to match
            pixelType = new HRPPixelTypeV32R8G8B8A8();
            return IMAGEPP_STATUS_Success;
            }
        }

    // We have no more suggestions
    return IMAGEOP_STATUS_NoMorePixelType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ImagePPStatus s_GetAvailableAlphaOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch, const HFCPtr<HRPPixelType> pInputPixelType) 
    {
    // We have only one suggestion.
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;

    if(pInputPixelType != NULL)
        {
        if (pInputPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            // Input pixel type already has an alpha channel. Let's use the same one for output
            pixelType = (HRPPixelType*)pInputPixelType->Clone();
            return IMAGEPP_STATUS_Success;
            }
        else
            {
            // Let's see if we can add an alpha channel to the input pixel type
            HFCPtr<HRPPixelType> pOutputPixelType = s_GetSuitableOutputPixelTypeAlpha(pInputPixelType);
            if (pOutputPixelType == NULL)
                // We can't add an alpha channel
                return IMAGEOP_STATUS_NoMorePixelType;

            pixelType = pOutputPixelType;
            return IMAGEPP_STATUS_Success;
            }
        }
 
    // No input pixel type is set. Let's use V32R8G8B8A8 as output
    pixelType = new HRPPixelTypeV32R8G8B8A8();
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ImagePPStatus s_SetInputPixelTypeAlpha(HFCPtr<HRPPixelType> pixelType, HFCPtr<HRPPixelType>& inputPixelType, const HFCPtr<HRPPixelType> pOutputPixelType)
    {
    if(pixelType == NULL)
        {
        inputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    if (pixelType->CountIndexBits() > 0)
        // We don't process indexed pixel types (i.e. with a palette)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Input pixel type must have RGB channels, because the alpha range is provided in RGB
    if(pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::RED, 0) == HRPChannelType::FREE ||
       pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GREEN, 0) == HRPChannelType::FREE ||
       pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::BLUE, 0) == HRPChannelType::FREE)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Only process 8 bits channels. We currently don't process larger channels, because ranges can only be compared with 
    // byte values, so we ignore other than 8 bits for now.
    // (Use index 0 because all channels have same size.)
    if(pixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() != 8)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Obtain a suitable output pixel type for the input type
    HFCPtr<HRPPixelType> pSuitableOutputPixelType = s_GetSuitableOutputPixelTypeAlpha(pixelType);

    if (pSuitableOutputPixelType == NULL)
        // Not possible to create a suitable output type by adding an alpha channel
        return IMAGEOP_STATUS_InvalidPixelType;

    inputPixelType = pixelType;
    
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ImagePPStatus s_SetOutputPixelTypeAlpha(HFCPtr<HRPPixelType> pixelType, HFCPtr<HRPPixelType>& outputPixelType, const HFCPtr<HRPPixelType> pInputPixelType)
    {
    if(pixelType == NULL)
        {
        outputPixelType = NULL;
        return IMAGEPP_STATUS_Success;
        }

    if (pixelType->CountIndexBits() > 0)
        // We don't process indexed pixel types (i.e. with a palette)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Output pixel type must have an alpha channel
    if(pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Output pixel type must have RGB channels, because the alpha range is provided in RGB
    if(pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::RED, 0) == HRPChannelType::FREE ||
       pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GREEN, 0) == HRPChannelType::FREE ||
       pixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::BLUE, 0) == HRPChannelType::FREE)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Only process 8 bit channels. We currently don't process larger channels, because ranges can only be compared with 
    // byte values, so we leave that out for now.
    // All channels have same size.
    if(pixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() != 8)
        return IMAGEOP_STATUS_InvalidPixelType;

    // Output pixel type is suitable
    outputPixelType = pixelType;
    return IMAGEPP_STATUS_Success;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpAlphaReplacerFilter--------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
HRAImageOpPtr HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges)
    {
    return new HRAImageOpAlphaReplacerFilter(defaultAlpha, pi_rRanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpAlphaReplacerFilter::HRAImageOpAlphaReplacerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges)
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
    m_defaultAlpha(defaultAlpha), m_ranges(pi_rRanges)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpAlphaReplacerFilter::~HRAImageOpAlphaReplacerFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    return s_GetAvailableAlphaInputPixelType(pixelType, index, pixelTypeToMatch, GetOutputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) 
    {
    return s_GetAvailableAlphaOutputPixelType(pixelType, index, pixelTypeToMatch, GetInputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    return s_SetInputPixelTypeAlpha(pixelType, m_pInputPixelType, GetOutputPixelType());
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    return s_SetOutputPixelTypeAlpha(pixelType, m_pOutputPixelType, GetInputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params)
    {
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaReplacerFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
#ifndef DISABLE_BUFFER_GETDATA
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth());
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight());

    // Find the corresponding channels (R, G, B, Alpha) between input and output raster
    uint32_t correspondingChannel[255];
    uint32_t nbChannelsOut = GetOutputPixelType()->GetChannelOrg().CountChannels();
    uint32_t nbChannelsIn = GetInputPixelType()->GetChannelOrg().CountChannels();
    uint32_t alphaChannelIndex=0;
    uint32_t redChannelIndex=0;
    uint32_t greenChannelIndex=0;
    uint32_t blueChannelIndex = 0;
    for (uint32_t i = 0; i < nbChannelsOut; i++)
        {
        const HRPChannelType* pOutputChannelType = GetOutputPixelType()->GetChannelOrg().GetChannelPtr(i);
        uint32_t inputChannelIndex = GetInputPixelType()->GetChannelOrg().GetChannelIndex(pOutputChannelType->GetRole(), 0);
        if (pOutputChannelType->GetRole() == HRPChannelType::ALPHA)
            alphaChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::RED)
            redChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::GREEN)
            greenChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::BLUE)
            blueChannelIndex = i;

        if (inputChannelIndex != HRPChannelType::FREE)
            correspondingChannel[i] = inputChannelIndex;
        else
            {
            // At this point, if there is no corresponding channel in input, it is necessarily the alpha channel (which may be absent from input and present in output only)
            HASSERT(pOutputChannelType->GetRole() == HRPChannelType::ALPHA);
            }
        }

    // Navigate the buffer, replacing alpha channel appropriately
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t nbColumns = outData.GetWidth();
    uint32_t nbRanges = (uint32_t)m_ranges.size();
    for(uint32_t row=0; row < outData.GetHeight(); ++row)
        {
        Byte const* pInBufferLine = pInBuffer+row*inPitch;
        Byte*       pOutBufferLine = pOutBuffer+row*outPitch;

        uint32_t outPosition = 0;
        uint32_t inPosition = 0;
        for(uint32_t column=0; column < nbColumns; column++)
            {
            for(uint32_t channelIdx=0; channelIdx < nbChannelsOut; channelIdx++)
                {
                if (channelIdx == alphaChannelIndex)
                    pOutBufferLine[outPosition + alphaChannelIndex] = m_defaultAlpha;
                else
                    pOutBufferLine[outPosition + channelIdx] = pInBufferLine[inPosition + correspondingChannel[channelIdx]]; 
                }

            // verify if the color is in the ranges list
            for(uint32_t rangeIdx = 0; rangeIdx < nbRanges; rangeIdx++)
                if(m_ranges[rangeIdx].IsIn(pOutBufferLine[outPosition + redChannelIndex], 
                                             pOutBufferLine[outPosition + greenChannelIndex], 
                                             pOutBufferLine[outPosition + blueChannelIndex]))
                    // if yes, set the new alpha value
                    pOutBufferLine[outPosition + alphaChannelIndex] = m_ranges[rangeIdx].GetAlphaValue();

            outPosition += nbChannelsOut;
            inPosition += nbChannelsIn;
            }
        }
#endif
    return IMAGEPP_STATUS_Success;
    }

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpAlphaComposerFilter--------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
HRAImageOpPtr HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges)
    {
    return new HRAImageOpAlphaComposerFilter(defaultAlpha, pi_rRanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpAlphaComposerFilter::HRAImageOpAlphaComposerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges)
    :HRAImageOp(new HRPPixelNeighbourhood(1,1,0,0)),
    m_defaultAlpha(defaultAlpha), m_ranges(pi_rRanges)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpAlphaComposerFilter::~HRAImageOpAlphaComposerFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    return s_GetAvailableAlphaInputPixelType(pixelType, index, pixelTypeToMatch, GetOutputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) 
    {
    return s_GetAvailableAlphaOutputPixelType(pixelType, index, pixelTypeToMatch, GetInputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    return s_SetInputPixelTypeAlpha(pixelType, m_pInputPixelType, GetOutputPixelType());
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    return s_SetOutputPixelTypeAlpha(pixelType, m_pOutputPixelType, GetInputPixelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params)
    {
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpAlphaComposerFilter::_Process(HRAImageSampleR outData, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
#ifndef DISABLE_BUFFER_GETDATA
    HPRECONDITION(GetInputPixelType() != NULL && GetOutputPixelType() != NULL);
    HPRECONDITION(inputData.GetBufferCP() != NULL && outData.GetBufferP() != NULL);
    HPRECONDITION(inputData.GetWidth() == outData.GetWidth());
    HPRECONDITION(inputData.GetHeight() == outData.GetHeight());

    // Find the corresponding channels (R, G, B, Alpha) between input and output raster
    uint32_t correspondingChannel[255];
    uint32_t nbChannelsOut = GetOutputPixelType()->GetChannelOrg().CountChannels();
    uint32_t nbChannelsIn = GetInputPixelType()->GetChannelOrg().CountChannels();
    uint32_t alphaChannelIndex = 0;
    uint32_t redChannelIndex = 0;
    uint32_t greenChannelIndex = 0;
    uint32_t blueChannelIndex = 0;
    bool inputHasAlphaChannel = true;

    for (uint32_t i = 0; i < nbChannelsOut; i++)
        {
        const HRPChannelType* pOutputChannelType = GetOutputPixelType()->GetChannelOrg().GetChannelPtr(i);
        uint32_t inputChannelIndex = GetInputPixelType()->GetChannelOrg().GetChannelIndex(pOutputChannelType->GetRole(), 0);
        if (pOutputChannelType->GetRole() == HRPChannelType::ALPHA)
            alphaChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::RED)
            redChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::GREEN)
            greenChannelIndex = i;
        else if (pOutputChannelType->GetRole() == HRPChannelType::BLUE)
            blueChannelIndex = i;

        // Set the corresponding channels (same role) between input and output
        if (inputChannelIndex != HRPChannelType::FREE)
            correspondingChannel[i] = inputChannelIndex;
        else
            {
            // At this point, if there is no corresponding channel in input, it is necessarily the alpha channel (which may be absent from input and present in output only)
            HASSERT(pOutputChannelType->GetRole() == HRPChannelType::ALPHA);
            inputHasAlphaChannel = false;
            }
        }

    HFCMath (*pQuotients) (HFCMath::GetInstance());

    // Navigate the buffer, replacing alpha channel appropriately
    size_t inPitch, outPitch;
    Byte const* pInBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
    Byte* pOutBuffer = outData.GetBufferP()->GetDataP(outPitch);

    uint32_t nbColumns = outData.GetWidth();
    uint32_t nbRanges = (uint32_t)m_ranges.size();
    for(uint32_t row=0; row < outData.GetHeight(); ++row)
        {
        Byte const* pInBufferLine = pInBuffer+row*inPitch;
        Byte*       pOutBufferLine = pOutBuffer+row*outPitch;

        uint32_t outPosition = 0;
        uint32_t inPosition = 0;
        for(uint32_t column=0; column < nbColumns; column++)
            {
            for(uint32_t channelIdx=0; channelIdx < nbChannelsOut; channelIdx++)
                {
                if (channelIdx == alphaChannelIndex)
                    {
                    if (inputHasAlphaChannel)
                        {
                        pOutBufferLine[outPosition + alphaChannelIndex] = (Byte)pQuotients->DivideBy255((uint32_t)m_defaultAlpha * (uint32_t)pInBufferLine[inPosition + correspondingChannel[channelIdx]]);
                        }
                    else
                        {
                        pOutBufferLine[outPosition + alphaChannelIndex] = m_defaultAlpha;
                        }
                    }
                else
                    pOutBufferLine[outPosition + channelIdx] = pInBufferLine[inPosition + correspondingChannel[channelIdx]]; 
                }

            // verify if the color is in the ranges list
            for(uint32_t rangeIdx = 0; rangeIdx < nbRanges; rangeIdx++)
                if(m_ranges[rangeIdx].IsIn(pOutBufferLine[outPosition + redChannelIndex], 
                                             pOutBufferLine[outPosition + greenChannelIndex], 
                                             pOutBufferLine[outPosition + blueChannelIndex]))
                    {
                    // if yes, set the new alpha value
                    if (inputHasAlphaChannel)
                        {
                        // Input data has alpha
                        pOutBufferLine[outPosition + alphaChannelIndex] = (Byte)pQuotients->DivideBy255((uint32_t)m_ranges[rangeIdx].GetAlphaValue() * (uint32_t)pInBufferLine[inPosition + correspondingChannel[alphaChannelIndex]]);
                        }
                    else
                        {
                        // Input data has no alpha
                        pOutBufferLine[outPosition + alphaChannelIndex] = m_ranges[rangeIdx].GetAlphaValue();
                        }
                    }

            outPosition += nbChannelsOut;
            inPosition += nbChannelsIn;
            }
        }
#endif
    return IMAGEPP_STATUS_Success;
    }








