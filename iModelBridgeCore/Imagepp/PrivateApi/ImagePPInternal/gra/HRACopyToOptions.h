//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/HRACopyToOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>

#include <ImagePP/all/h/HGSTypes.h> 

BEGIN_IMAGEPP_NAMESPACE 

class HGF2DCoordSys;
class HRPPixelType;
class HVEShape;


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct HRACopyToOptions
{
    HRACopyToOptions();
    HRACopyToOptions(HRACopyToOptions const& opts);
    ~HRACopyToOptions();

    //! Query if alpha blend is enabled
    bool ApplyAlphaBlend() const {return m_alphaBlend;}

    //! Enable or disabled alpha blending.
    void SetAlphaBlend(bool alphaBlend){m_alphaBlend=alphaBlend;}

    //! The source raster replacement coordinate system
    //! This coordinate system is the coordinate system to which the 
    //! source raster must be transported to relative to its own
    //! logical coordinate system
    HFCPtr<HGF2DCoordSys> GetReplacingCoordSys() const;

    //! The replacing coordinate system for the source raster relative
    //! to this source raster logical coordinate system
    void SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pCoordSys);

    //! The source raster replacing pixel type if any. A null pointer indicates
    //! no replacing pixel type is set.
    HFCPtr<HRPPixelType> GetReplacingPixelType() const;

    //! Sets the replacing pixel type for the source raster.
    //! The replacing pixel type must fit with the source raster
    //! memory nature of the source raster.
    void SetReplacingPixelType(const HFCPtr<HRPPixelType>& pPixelType);

    //! The shape of the copy to region. This shape is expressed in the 
    //! coordinate system of the replacing coordinate system if any
    //! or any applicable coordinate system otherwise.
    //! This implies that in the event a source replacing coordinate
    //! system is present in the options then transportation of the 
    //! shape must be performed to interact with source related geometry.
    HVEShape const* GetShape() const;

    //! Sets the shape of the copy to region. This shape is expressed in the 
    //! coordinate system of the replacing coordinate system if any
    //! or any applicable coordinate system otherwise.
    //! This implies that in the event a source replacing coordinate
    //! system is present in the options then transportation of the 
    //! shape must be performed to interact with source related geometry.
    //! The shape will be intersected with related source and destination
    //! raster geometry prior to being used so it need not be already
    //! clipped to object it will relate with.
    void SetShape(HVEShape const* pShape);

   HGSResampling const& GetResamplingMode() const;
   void                 SetResamplingMode(HGSResampling const& resampling);

   HRACopyToOptions& operator=(const HRACopyToOptions& pi_rObj) = delete;
private:
    HVEShape const*       m_pShape;                   // The effective copy region.        
    HFCPtr<HGF2DCoordSys> m_pSrcReplacingCoordSys;
    HFCPtr<HRPPixelType>  m_pSrcReplacingPixelType;
    bool                  m_alphaBlend;
    HGSResampling         m_resampling;
};


END_IMAGEPP_NAMESPACE
