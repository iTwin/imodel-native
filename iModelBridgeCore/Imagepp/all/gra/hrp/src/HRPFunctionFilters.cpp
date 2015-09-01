//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPFunctionFilters.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for HRPFunctionFilters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HRAImageOp.h>
#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

#define CLAMP(A)((A)<=(0) ? (0) : (A)<(256) ? (A) : (255))

typedef list<HGFRGBSet>  RGBSetList;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPColortwistFilter
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColortwistFilter::HRPColortwistFilter() :
    HRPFunctionFilter(new HRPPixelTypeV24PhotoYCC())
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColortwistFilter::HRPColortwistFilter(const double pi_Matrix[4][4]) :
    HRPFunctionFilter(new HRPPixelTypeV24PhotoYCC())
    {
    memcpy(m_Matrix, pi_Matrix, 4 * 4 * sizeof(double));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColortwistFilter::HRPColortwistFilter(const HRPColortwistFilter& pi_rFilter)
    : HRPFunctionFilter(pi_rFilter)
    {
    memcpy(m_Matrix, pi_rFilter.m_Matrix, 4 * 4 * sizeof(double));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColortwistFilter::~HRPColortwistFilter()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPColortwistFilter::Clone() const
    {
    return new HRPColortwistFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPColortwistFilter::Function( const void* pi_pSrcRawData,
                                    void* po_pDestRawData,
                                    uint32_t PixelsCount) const
    {
    double* pMatrix = (double*)m_Matrix;
    Byte* pSrcRawData = (Byte*)pi_pSrcRawData;
    Byte* pDestRawData = (Byte*)po_pDestRawData;

    double Lin;
    double C1in;
    double C2in;
    double Lout;
    double C1out;
    double C2out;
    int32_t L8;
    int32_t C18;
    int32_t C28;

    while(PixelsCount)
        {
        // normalize PhotoYCC
        Lin  = 1.3584 * pSrcRawData[0];
        C1in = 2.2179 * (pSrcRawData[1] - 156);
        C2in = 1.8215 * (pSrcRawData[2] - 137);

        Lout  = pMatrix[0] * Lin + pMatrix[1] * C1in + pMatrix[2] * C2in + pMatrix[3];
        C1out = pMatrix[4] * Lin + pMatrix[5] * C1in + pMatrix[6] * C2in + pMatrix[7];
        C2out = pMatrix[8] * Lin + pMatrix[9] * C1in + pMatrix[10] * C2in + pMatrix[11];

        // reconvert the output in PhotoYCC 8 bits
        L8 =  (int32_t)(Lout / 1.3584);
        C18 = (int32_t)((C1out / 2.2179) + 156);
        C28 = (int32_t)((C2out / 1.8215) + 137);

        pDestRawData[0] = (Byte)(CLAMP(L8));
        pDestRawData[1] = (Byte)(CLAMP(C18));
        pDestRawData[2] = (Byte)(CLAMP(C28));

        pSrcRawData += 3;
        pDestRawData += 3;

        PixelsCount--;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const double* HRPColortwistFilter::GetMatrix() const
    {
    return m_Matrix;
    }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPAlphaReplacer
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaReplacer::HRPAlphaReplacer()
    :   HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
        m_Ranges()
    {
    m_DefaultAlpha = 255;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaReplacer::HRPAlphaReplacer(Byte pi_DefaultAlpha)
    :   HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
        m_Ranges()
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaReplacer::HRPAlphaReplacer(Byte pi_DefaultAlpha,
                                   const ListHRPAlphaRange& pi_rRanges)
    :    HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
         m_Ranges(pi_rRanges)
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaReplacer::HRPAlphaReplacer(const HRPAlphaReplacer& pi_rObj)
    : HRPFunctionFilter(pi_rObj),
      m_Ranges()
    {
    try
        {
        DeepCopy(pi_rObj);
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaReplacer::~HRPAlphaReplacer()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaReplacer::DeepCopy(const HRPAlphaReplacer& pi_rObj)
    {
    m_DefaultAlpha = pi_rObj.m_DefaultAlpha;
    m_Ranges = pi_rObj.m_Ranges;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaReplacer::DeepDelete()
    {
    m_Ranges.clear();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPAlphaReplacer::Clone() const
    {
    return new HRPAlphaReplacer(*this);
    }

//-----------------------------------------------------------------------------
// GetDefaultAlpha
//-----------------------------------------------------------------------------
Byte HRPAlphaReplacer::GetDefaultAlpha() const
    {
    return m_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
// SetDefaultAlpha
//-----------------------------------------------------------------------------
void HRPAlphaReplacer::SetDefaultAlpha(Byte pi_DefaultAlpha)
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
// GetAlphaRanges
//-----------------------------------------------------------------------------
ListHRPAlphaRange HRPAlphaReplacer::GetAlphaRanges() const
    {
    return m_Ranges;
    }

//-----------------------------------------------------------------------------
// SetAlphaRanges
//-----------------------------------------------------------------------------
void HRPAlphaReplacer::SetAlphaRanges(const ListHRPAlphaRange& pi_rRanges)
    {
    m_Ranges = pi_rRanges;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaReplacer::Function( const void* pi_pSrcRawData,
                                 void* po_pDestRawData,
                                 uint32_t PixelsCount) const
    {
    Byte* pSrcRawData = (Byte*)pi_pSrcRawData;
    Byte* pDestRawData = (Byte*)po_pDestRawData;

    uint32_t NumberOfRanges = (uint32_t)m_Ranges.size();

    // is there some alpha range defined?
    if(NumberOfRanges > 0)
        {
        // there are ranges

        while(PixelsCount != 0)
            {
            pDestRawData[0] = pSrcRawData[0];
            pDestRawData[1] = pSrcRawData[1];
            pDestRawData[2] = pSrcRawData[2];
            pDestRawData[3] = m_DefaultAlpha;

            // verify if the color is in the ranges list
            for(uint32_t Index = 0; Index < NumberOfRanges; Index++)
                if(m_Ranges[Index].IsIn(pDestRawData[0], pDestRawData[1], pDestRawData[2]))
                    // if yes, set the new alpha value
                    pDestRawData[3] = m_Ranges[Index].GetAlphaValue();

            pSrcRawData += 4;
            pDestRawData += 4;

            PixelsCount--;
            }
        }
    else
        {
        // there is no ranges, optimal case: all alphas to default value

        // parse the pixels and copy all the data and relpace the alpha value
        while(PixelsCount != 0)
            {
            pDestRawData[0] = pSrcRawData[0];
            pDestRawData[1] = pSrcRawData[1];
            pDestRawData[2] = pSrcRawData[2];
            pDestRawData[3] = m_DefaultAlpha;

            pSrcRawData += 4;
            pDestRawData += 4;

            PixelsCount--;
            }
        }
    }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPAlphaComposer
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaComposer::HRPAlphaComposer()
    :   HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
        m_Ranges()
    {
    m_DefaultAlpha = 255;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaComposer::HRPAlphaComposer(Byte pi_DefaultAlpha)
    :   HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
        m_Ranges()
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaComposer::HRPAlphaComposer(Byte pi_DefaultAlpha,
                                   const ListHRPAlphaRange& pi_rRanges)
    :    HRPFunctionFilter(new HRPPixelTypeV32R8G8B8A8()),
         m_Ranges(pi_rRanges)
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaComposer::HRPAlphaComposer(const HRPAlphaComposer& pi_rObj)
    : HRPFunctionFilter(pi_rObj),
      m_Ranges()
    {
    try
        {
        DeepCopy(pi_rObj);
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPAlphaComposer::~HRPAlphaComposer()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaComposer::DeepCopy(const HRPAlphaComposer& pi_rObj)
    {
    m_DefaultAlpha = pi_rObj.m_DefaultAlpha;
    m_Ranges = pi_rObj.m_Ranges;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaComposer::DeepDelete()
    {
    m_Ranges.clear();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPAlphaComposer::Clone() const
    {
    return new HRPAlphaComposer(*this);
    }

//-----------------------------------------------------------------------------
// GetDefaultAlpha
//-----------------------------------------------------------------------------
Byte HRPAlphaComposer::GetDefaultAlpha() const
    {
    return m_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
// SetDefaultAlpha
//-----------------------------------------------------------------------------
void HRPAlphaComposer::SetDefaultAlpha(Byte pi_DefaultAlpha)
    {
    m_DefaultAlpha = pi_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
// GetAlphaRanges
//-----------------------------------------------------------------------------
ListHRPAlphaRange HRPAlphaComposer::GetAlphaRanges() const
    {
    return m_Ranges;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPAlphaComposer::Function( const void* pi_pSrcRawData,
                                 void* po_pDestRawData,
                                 uint32_t PixelsCount) const
    {
    Byte* pSrcRawData = (Byte*)pi_pSrcRawData;
    Byte* pDestRawData = (Byte*)po_pDestRawData;

    uint32_t NumberOfRanges = (uint32_t)m_Ranges.size();
    Byte OriginalAlpha;

    // is there some alpha range defined?
    if(NumberOfRanges > 0)
        {
        // there are ranges
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(PixelsCount != 0)
            {
            OriginalAlpha = pDestRawData[3];

            pDestRawData[0] = pSrcRawData[0];
            pDestRawData[1] = pSrcRawData[1];
            pDestRawData[2] = pSrcRawData[2];
            pDestRawData[3] = pQuotients->DivideBy255ToByte((uint32_t)m_DefaultAlpha * (uint32_t)pSrcRawData[3]);


            // verify if the color is in the ranges list
            for(uint32_t Index = 0; Index < NumberOfRanges; Index++)
                if(m_Ranges[Index].IsIn(pDestRawData[0], pDestRawData[1], pDestRawData[2]))
                    // if yes, set the new alpha value
                    pDestRawData[3] = pQuotients->DivideBy255ToByte((uint32_t)m_Ranges[Index].GetAlphaValue()* (uint32_t)pSrcRawData[3]);

            pSrcRawData += 4;
            pDestRawData += 4;

            PixelsCount--;
            }
        }
    else
        {
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // there is no ranges, optimal case: all alphas to default value

        // parse the pixels and copy all the data and relpace the alpha value

        while(PixelsCount != 0)
            {
            pDestRawData[0] = pSrcRawData[0];
            pDestRawData[1] = pSrcRawData[1];
            pDestRawData[2] = pSrcRawData[2];
            pDestRawData[3] = pQuotients->DivideBy255ToByte((uint32_t)m_DefaultAlpha * (uint32_t)pSrcRawData[3]);

            pSrcRawData += 4;
            pDestRawData += 4;

            PixelsCount--;
            }
        }
    }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPColorReplacerFilter
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorReplacerFilter::HRPColorReplacerFilter()
    : HRPFunctionFilter(new HRPPixelTypeV24R8G8B8())
    {
    // Dont know yet what's the selected color,
    // but safer when initialized.
    m_NewRGBColor[0] = 0;
    m_NewRGBColor[1] = 0;
    m_NewRGBColor[2] = 0;

    // Used for debugging purpose only.
#ifdef __HMR_DEBUG_MEMBER
    m_NewColorInitialized = false;
#endif
    }

//-----------------------------------------------------------------------------
// HRPColorReplacerFilter copy constructor
//-----------------------------------------------------------------------------

HRPColorReplacerFilter::HRPColorReplacerFilter(const HRPColorReplacerFilter& pi_rFilter)
    : HRPFunctionFilter(new HRPPixelTypeV24R8G8B8()),
      m_RGBCubeList(pi_rFilter.m_RGBCubeList),
      m_RGBSetList(pi_rFilter.m_RGBSetList),
      m_LUVCubeList(pi_rFilter.m_LUVCubeList),
      m_RGBCubeRemoveList(pi_rFilter.m_RGBCubeRemoveList),
      m_RGBSetRemoveList(pi_rFilter.m_RGBSetRemoveList),
      m_LUVCubeRemoveList(pi_rFilter.m_LUVCubeRemoveList)
    {
#ifdef __HMR_DEBUG_MEMBER
    m_NewColorInitialized = pi_rFilter.m_NewColorInitialized;
#endif
    memcpy(m_NewRGBColor, pi_rFilter.m_NewRGBColor, 3);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorReplacerFilter::~HRPColorReplacerFilter()
    {
    m_RGBCubeList.clear();
    m_RGBSetList.clear();
    m_LUVCubeList.clear();

    m_RGBCubeRemoveList.clear();
    m_RGBSetRemoveList.clear();
    m_LUVCubeRemoveList.clear();
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPColorReplacerFilter::Clone() const
    {
    return new HRPColorReplacerFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::Function(  const void*  pi_pSrcRawData,
                                        void*  po_pDestRawData,
                                        uint32_t PixelsCount) const
    {
    // Can't operate on en empty list!
    HASSERT(m_RGBCubeList.size() > 0 || (m_RGBSetList.size() > 0) || (m_LUVCubeList.size() > 0));
    HASSERT(pi_pSrcRawData != 0);
    HASSERT(po_pDestRawData != 0);
    HASSERT(PixelsCount);

    uint32_t PixelIndex;
    bool  PixelFound;

    Byte* SrcRawData  = (Byte*) pi_pSrcRawData;
    Byte* DestRawData = (Byte*) po_pDestRawData;

    // For each pixel present into the source...
    for (PixelIndex = 0; PixelIndex < PixelsCount; PixelIndex++)
        {
        PixelFound = LookForColor(*(SrcRawData), *(SrcRawData + 1), *(SrcRawData + 2));

        // If color has been found into the ColorSet , change it!
        if (PixelFound)
            {
            // At this set it black! ;-)
            *(  DestRawData) = *(m_NewRGBColor    );
            *(++DestRawData) = *(m_NewRGBColor + 1);
            *(++DestRawData) = *(m_NewRGBColor + 2);
            ++DestRawData;
            }
        else
            {
            // Keep original pixel.
            *(  DestRawData) = *(SrcRawData    );
            *(++DestRawData) = *(SrcRawData + 1);
            *(++DestRawData) = *(SrcRawData + 2);
            ++DestRawData;
            }

        // Next RGB pixel 3 Bytes after..
        SrcRawData+= 3;
        }
    }

//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::SetNewColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue)
    {
    // Used for debugging purpose only.
#ifdef __HMR_DEBUG_MEMBER
    m_NewColorInitialized = true;
#endif

    m_NewRGBColor[0] = pi_Red;
    m_NewRGBColor[1] = pi_Green;
    m_NewRGBColor[2] = pi_Blue;
    }

//-----------------------------------------------------------------------------
// public GetNewColor
//-----------------------------------------------------------------------------
const Byte* HRPColorReplacerFilter::GetNewColor() const
    {
    return m_NewRGBColor;
    }

//-----------------------------------------------------------------------------
// public GetSelectedRGBSet
//-----------------------------------------------------------------------------
const RGBSetList& HRPColorReplacerFilter::GetSelectedRGBSet() const
    {
    return m_RGBSetList;
    }

//-----------------------------------------------------------------------------
// public GetSelectedRemoveRGBSet
//-----------------------------------------------------------------------------
const RGBSetList& HRPColorReplacerFilter::GetSelectedRemoveRGBSet() const
    {
    return m_RGBSetRemoveList;
    }

//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::AddColors(const HGFRGBCube& pi_rCube)
    {
    m_RGBCubeList.push_back(pi_rCube);
    }


//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::AddColors(const HGFRGBSet&  pi_rCube)
    {
    m_RGBSetList.push_back(pi_rCube);
    }

//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::AddColors(const HGFLUVCube& pi_rCube)
    {
    m_LUVCubeList.push_back(pi_rCube);
    }

//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::RemoveColors(const HGFRGBCube& pi_rCube)
    {
    m_RGBCubeRemoveList.push_back(pi_rCube);
    }


//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::RemoveColors(const HGFRGBSet&  pi_rCube)
    {
    m_RGBSetRemoveList.push_back(pi_rCube);
    }

//-----------------------------------------------------------------------------
// public SetNewColor
//-----------------------------------------------------------------------------

void HRPColorReplacerFilter::RemoveColors(const HGFLUVCube& pi_rCube)
    {
    m_LUVCubeRemoveList.push_back(pi_rCube);
    }

//-----------------------------------------------------------------------------
// private LookForColor
//
// Verrify on all cube of all kind if we had selected color...
//-----------------------------------------------------------------------------
inline bool HRPColorReplacerFilter::LookForColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const
    {
    // We should have selected a color before looking for it...
#ifdef __HMR_DEBUG_MEMBER
    HPRECONDITION(m_NewColorInitialized);
#endif

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
    // If Color has been been found, verriry if the user have remove it...
    while(PixelFound && (RGBCudeRemoveItr != m_RGBCubeRemoveList.end()))
        {
        PixelFound = !((*RGBCudeRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        RGBCudeRemoveItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // If Color has been been found, verriry if the user have remove it...
    while(PixelFound && (RGBSetRemoveItr != m_RGBSetRemoveList.end()))
        {
        PixelFound = !((*RGBSetRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        RGBSetRemoveItr++;
        }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // If Color has been been found, verriry if the user have remove it...
    while(PixelFound && (LUVCubeRemoveItr != m_LUVCubeRemoveList.end()))
        {
        PixelFound = !((*LUVCubeRemoveItr).IsIn(pi_Red, pi_Green, pi_Blue));

        // Next ColorSet.
        LUVCubeRemoveItr++;
        }

    return PixelFound;
    }
