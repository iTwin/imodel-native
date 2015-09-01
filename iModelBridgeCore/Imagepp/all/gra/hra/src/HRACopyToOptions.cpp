//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRACopyToOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRACopyFromOptions
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRACopyToOptions.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HVEShape.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyToOptions::HRACopyToOptions()
    :m_alphaBlend(false),
    m_pShape(NULL),
    m_resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyToOptions::HRACopyToOptions(HRACopyToOptionsCR opts)
 :m_resampling(opts.m_resampling)
    {
    m_pShape = opts.m_pShape;
    m_pSrcReplacingCoordSys = opts.m_pSrcReplacingCoordSys;
    m_pSrcReplacingPixelType = opts.m_pSrcReplacingPixelType;
    m_alphaBlend = opts.m_alphaBlend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyToOptions::~HRACopyToOptions(){}

HFCPtr<HGF2DCoordSys> HRACopyToOptions::GetReplacingCoordSys() const {return m_pSrcReplacingCoordSys;}
void HRACopyToOptions::SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pCoordSys){m_pSrcReplacingCoordSys=pCoordSys;}

HFCPtr<HRPPixelType> HRACopyToOptions::GetReplacingPixelType() const {return m_pSrcReplacingPixelType;}
void HRACopyToOptions::SetReplacingPixelType(const HFCPtr<HRPPixelType>& pPixelType){m_pSrcReplacingPixelType=pPixelType;}

HVEShape const* HRACopyToOptions::GetShape() const{return m_pShape;}
void HRACopyToOptions::SetShape(HVEShape const* pShape){m_pShape=pShape;}

void HRACopyToOptions::SetResamplingMode(HGSResampling const& resampling) {m_resampling = resampling;}
HGSResampling const& HRACopyToOptions::GetResamplingMode() const {return m_resampling;}