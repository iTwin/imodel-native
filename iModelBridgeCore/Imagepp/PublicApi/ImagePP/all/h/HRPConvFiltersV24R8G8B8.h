//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPConvFiltersV24R8G8B8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPConvFiltersV24R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPConvFilterV24R8G8B8.h"

//-----------------------------------------------------------------------------
// HRPSmoothFilter
// Smooth filter definition
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRPSmoothFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Smooth, HRPConvFilterV24R8G8B8)
    

public:

    IMAGEPP_EXPORT                     HRPSmoothFilter();
    IMAGEPP_EXPORT virtual             ~HRPSmoothFilter();

    virtual HRPFilter* Clone() const override;

private:
    HRPSmoothFilter(const HRPSmoothFilter& pi_rFilter);

    // Disabled methods
    HRPSmoothFilter& operator =(const HRPSmoothFilter& pi_rObj);

    static int32_t WeightMatrix[3][3];
    };

//-----------------------------------------------------------------------------
// HRPSharpenFilter
// Sharpen filter definition
//-----------------------------------------------------------------------------
class HRPSharpenFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Sharpen, HRPConvFilterV24R8G8B8)
    

public:
    IMAGEPP_EXPORT                     HRPSharpenFilter();
    IMAGEPP_EXPORT                     HRPSharpenFilter(Byte pi_Intensity);
    IMAGEPP_EXPORT virtual             ~HRPSharpenFilter();

    virtual HRPFilter* Clone() const override;

    virtual Byte      GetIntensity() const;
    virtual void       SetIntensity(Byte pi_Intensity);

private:
    HRPSharpenFilter(const HRPSharpenFilter& pi_rFilter);

    // Disabled methods
    HRPSharpenFilter& operator =(const HRPSharpenFilter& pi_rObj);

    Byte  m_Intensity;
    };

//-----------------------------------------------------------------------------
// HRPDetailFilter
// Detail filter definition
//-----------------------------------------------------------------------------
class HRPDetailFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Detail, HRPConvFilterV24R8G8B8)
    

public:
    IMAGEPP_EXPORT                     HRPDetailFilter();
    IMAGEPP_EXPORT virtual             ~HRPDetailFilter();

    virtual HRPFilter* Clone() const override;

private:
    HRPDetailFilter(const HRPDetailFilter& pi_rFilter);

    // Disabled methods
    HRPDetailFilter& operator =(const HRPDetailFilter& pi_rObj);

    static int32_t WeightMatrix[3][3];
    };

//-----------------------------------------------------------------------------
// HRPEdgeEnhanceFilter
// Edgen enhance filter definition
//-----------------------------------------------------------------------------

class HRPEdgeEnhanceFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_EdgeEnhance, HRPConvFilterV24R8G8B8)
    

public:

    IMAGEPP_EXPORT                 HRPEdgeEnhanceFilter();
    IMAGEPP_EXPORT virtual         ~HRPEdgeEnhanceFilter();

    virtual HRPFilter* Clone() const override;

private:
    HRPEdgeEnhanceFilter(const HRPEdgeEnhanceFilter& pi_rFilter);

    // Disabled methods
    HRPEdgeEnhanceFilter& operator =(const HRPEdgeEnhanceFilter& pi_rObj);

    static int32_t WeightMatrix[3][3];
    };

//-----------------------------------------------------------------------------
// HRPFindEdgesFilter
// Find edges filter definition
//-----------------------------------------------------------------------------

class HRPFindEdgesFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_FindEdges, HRPConvFilterV24R8G8B8)
    

public:

    IMAGEPP_EXPORT                     HRPFindEdgesFilter();
    IMAGEPP_EXPORT virtual             ~HRPFindEdgesFilter();

    virtual HRPFilter* Clone() const override;

private:
    HRPFindEdgesFilter(const HRPFindEdgesFilter& pi_rFilter);

    // Disabled methods
    HRPFindEdgesFilter& operator =(const HRPFindEdgesFilter& pi_rObj);

    static int32_t WeightMatrix[3][3];
    };

//-----------------------------------------------------------------------------
// HRPBlurFilter
// Blur filter definition
//-----------------------------------------------------------------------------

class HRPBlurFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Blur, HRPConvFilterV24R8G8B8)
    

public:
    IMAGEPP_EXPORT                     HRPBlurFilter();
    IMAGEPP_EXPORT                     HRPBlurFilter(Byte pi_Intensity);

    IMAGEPP_EXPORT virtual             ~HRPBlurFilter();

    virtual HRPFilter* Clone() const override;

    virtual Byte      GetIntensity() const;
    virtual void       SetIntensity(Byte pi_Intensity);

private:
    HRPBlurFilter(const HRPBlurFilter& pi_rFilter);

    // Disabled methods
    HRPBlurFilter& operator =(const HRPBlurFilter& pi_rObj);

    Byte  m_Intensity;
    };


//-----------------------------------------------------------------------------
// HRPAverageFilter
// Average filter definition
//-----------------------------------------------------------------------------

class HRPAverageFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Average, HRPConvFilterV24R8G8B8)
    

public:

    IMAGEPP_EXPORT                     HRPAverageFilter();
    IMAGEPP_EXPORT virtual             ~HRPAverageFilter();

    virtual HRPFilter* Clone() const override;

private:
    HRPAverageFilter(const HRPAverageFilter& pi_rFilter);

    // Disabled methods
    HRPAverageFilter& operator =(const HRPAverageFilter& pi_rObj);

    static int32_t WeightMatrix[2][2];
    };
END_IMAGEPP_NAMESPACE

