//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessDensitySlicingFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

BEGIN_IMAGEPP_NAMESPACE
// Forward declaration.
class HGFLightnessColorSpace;

class HRPLigthnessDensitySlicingFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessDensitySlicing, HRPFunctionFilter)
    

public:                       // Primary methods
    HRPLigthnessDensitySlicingFilter();
    HRPLigthnessDensitySlicingFilter(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter(const HRPLigthnessDensitySlicingFilter& pi_rFilter);

    virtual              ~HRPLigthnessDensitySlicingFilter();

    // Cloning
    HRPFilter*     Clone() const override;

    // Get/Set methods
    IMAGEPP_EXPORT int32_t    AddSlice (int32_t pi_StartIndex,
                                       int32_t pi_EndIndex,
                                       uint32_t pi_StartColor,
                                       uint32_t pi_EndColor,
                                       int32_t pi_Opacity);

    bool     ModifySlice (int32_t pi_StartIndex,
                           int32_t pi_EndIndex,
                           uint32_t pi_StartColor,
                           uint32_t pi_EndColor,
                           int32_t pi_Opacity);

    int32_t    GetSliceIndex (int32_t pi_StartIndex,
                             int32_t pi_EndIndex) const;

    bool     GetSliceInfo(int32_t pi_SliceIndex,
                           int32_t* po_pStartIndex,
                           int32_t* po_pEndIndex,
                           int32_t* po_pStartColor,
                           int32_t* po_pEndColor,
                           int32_t* po_pOpacity) const;

    void      RemoveSlice (int32_t pi_StartIndex,
                           int32_t pi_EndIndex);

    void      RemoveSlice (int32_t pi_SliceIndex);

    void      RemoveAll();

    int32_t    GetSliceCount() const;

    IMAGEPP_EXPORT  void      SetDesaturationFactor(double pi_DesaturationFactor);
    double   GetDesaturationFactor() const;

protected:
    // methods
    virtual void      Function ( const void*  pi_pSrcRawData,
                                 void*  po_pDestRawData,
                                 uint32_t pi_PixelsCount) const override;
    struct SliceInfo
        {
        int32_t m_StartIndex;
        int32_t m_EndIndex;

        uint32_t m_StartColor;
        uint32_t m_EndColor;

        int32_t m_Opacity;
        };

    // members
    vector<SliceInfo > m_SliceList;

    uint32_t           m_Channels;
    unsigned short     m_ChannelWidth;
    int                m_MaxSampleValue;

    double            m_DesaturationFactor;
    HGFLightnessColorSpace* m_pColorSpaceConverter;

private:
    HRPLigthnessDensitySlicingFilter&
    operator=(const HRPLigthnessDensitySlicingFilter& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter& pi_rSrc);

    void            FunctionN8( const void*  pi_pSrcRawData,
                                void*  po_pDestRawData,
                                uint32_t pi_PixelsCount) const;

    void            FunctionN16( const void*  pi_pSrcRawData,
                                 void*  po_pDestRawData,
                                 uint32_t pi_PixelsCount) const;

    void            FunctionPalette( const void*  pi_pSrcRawData,
                                     void*  po_pDestRawData,
                                     uint32_t pi_PixelsCount) const;
    };
END_IMAGEPP_NAMESPACE


