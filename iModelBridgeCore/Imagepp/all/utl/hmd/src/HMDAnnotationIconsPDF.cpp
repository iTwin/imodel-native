//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDAnnotationIconsPDF.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDAnnotationIconsPDF.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDAnnotationIconsPDF::HMDAnnotationIconsPDF()
    : HMDMetaDataContainer(HMD_ANNOT_ICON_RASTERING_INFO)
    {
    m_RasterizeIcon = true;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDAnnotationIconsPDF::~HMDAnnotationIconsPDF()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDAnnotationIconsPDF::HMDAnnotationIconsPDF(const HMDAnnotationIconsPDF& pi_rObj)
    : HMDMetaDataContainer(pi_rObj)
    {
    m_RasterizeIcon = pi_rObj.m_RasterizeIcon;
    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HFCPtr<HMDMetaDataContainer> HMDAnnotationIconsPDF::Clone() const
    {
    return HFCPtr<HMDMetaDataContainer>(new HMDAnnotationIconsPDF(*this));
    }

//-----------------------------------------------------------------------------
// Public
// Set the annotation icon related rasterization operation (rasterize or not)
//-----------------------------------------------------------------------------
void HMDAnnotationIconsPDF::SetRasterization(bool pi_RasterizeIcon)
    {
    m_RasterizeIcon = pi_RasterizeIcon;
    }

//-----------------------------------------------------------------------------
// Public
// Get the annotation icon related rasterization operation (rasterize or not)
//-----------------------------------------------------------------------------
bool HMDAnnotationIconsPDF::GetRasterization()
    {
    return m_RasterizeIcon;
    }