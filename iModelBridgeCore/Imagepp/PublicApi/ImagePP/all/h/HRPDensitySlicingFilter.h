//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPDensitySlicingFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPDensitySlicingFilter
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPDensitySlicingFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_DensitySlicing, HRPFunctionFilter)
    

public:                       // Primary methods
    HRPDensitySlicingFilter();
    HRPDensitySlicingFilter(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPDensitySlicingFilter(const HRPDensitySlicingFilter& pi_rFilter);

    virtual              ~HRPDensitySlicingFilter();

    // Cloning
    HRPFilter*     Clone() const override;

    // Get/Set methods
    virtual int32_t    AddSlice (int32_t pi_StartIndex,
                                int32_t pi_EndIndex,
                                uint32_t pi_StartColor,
                                uint32_t pi_EndColor,
                                int32_t pi_Opacity);

    virtual bool     ModifySlice (int32_t pi_StartIndex,
                                   int32_t pi_EndIndex,
                                   uint32_t pi_StartColor,
                                   uint32_t pi_EndColor,
                                   int32_t pi_Opacity);

    virtual int32_t    GetSliceIndex (int32_t pi_StartIndex,
                                     int32_t pi_EndIndex) const;

    virtual bool     GetSliceInfo(int32_t pi_SliceIndex,
                                   int32_t* po_pStartIndex,
                                   int32_t* po_pEndIndex,
                                   int32_t* po_pStartColor,
                                   int32_t* po_pEndColor,
                                   int32_t* po_pOpacity) const;

    virtual void      RemoveSlice (int32_t pi_StartIndex,
                                   int32_t pi_EndIndex);

    virtual void      RemoveSlice (int32_t pi_SliceIndex);

    virtual void      RemoveAll();

    virtual int32_t    GetSliceCount() const;

    IMAGEPP_EXPORT  virtual void      SetDesaturationFactor(double pi_DesaturationFactor);
    virtual double   GetDesaturationFactor() const;

protected:
    // methods
    virtual void      Function ( const void*  pi_pSrcRawData,
                                 void*  po_pDestRawData,
                                 uint32_t pi_PixelsCount) const;
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

    int                m_Channels;
    int                m_ChannelWidth;
    int                m_MaxSampleValue;

    double            m_DesaturationFactor;

private:
    HRPDensitySlicingFilter&
    operator=(const HRPDensitySlicingFilter& pi_rFilter);
    void            DeepCopy(const HRPDensitySlicingFilter& pi_rSrc);

    void              FunctionN8( const void*  pi_pSrcRawData,
                                  void*  po_pDestRawData,
                                  uint32_t pi_PixelsCount) const;

    void              FunctionN16( const void*  pi_pSrcRawData,
                                   void*  po_pDestRawData,
                                   uint32_t pi_PixelsCount) const;

    void              FunctionPalette( const void*  pi_pSrcRawData,
                                       void*  po_pDestRawData,
                                       uint32_t pi_PixelsCount) const;
    };
END_IMAGEPP_NAMESPACE
