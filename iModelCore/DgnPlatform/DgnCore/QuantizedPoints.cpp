/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/QuantizedPoints.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"


static const double     s_quantizeRangeScale   = (double) 0xffff;
static const double     s_unquantizeRangeScale = 1.0 / (double) 0xffff;
static const double     s_halfRangeScale       = (double) 0xffff / 2.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Render::QuantizedPoint::QuantizedPoint(DRange3dCR range, DPoint3dCR value)
    {
    DVec3d              diagonal = range.DiagonalVector();
    
    m_x = (uint16_t) (.5 + (value.x - range.low.x) * s_quantizeRangeScale / diagonal.x); 
    m_y = (uint16_t) (.5 + (value.y - range.low.y) * s_quantizeRangeScale / diagonal.y); 
    m_z = (uint16_t) (.5 + (value.z - range.low.z) * s_quantizeRangeScale / diagonal.z); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Render::QuantizedPoint::Unquantized(DRange3dCR range) const
    {
    DVec3d              diagonal = range.DiagonalVector();

    return DPoint3d::From(range.low.x + m_x * diagonal.x * s_unquantizeRangeScale,
                          range.low.y + m_y * diagonal.y * s_unquantizeRangeScale,
                          range.low.z + m_z * diagonal.z * s_unquantizeRangeScale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
FPoint3d Render::QuantizedPoint::UnquantizedAboutCenter(DRange3dCR range) const
    {
    DVec3d              diagonal = range.DiagonalVector();
    FPoint3d            fPoint;
    
    fPoint.x = (float) ((double) m_x - 0x7fff) * diagonal.x * s_unquantizeRangeScale;
    fPoint.y = (float) ((double) m_y - 0x7fff) * diagonal.y * s_unquantizeRangeScale;
    fPoint.z = (float) ((double) m_z - 0x7fff) * diagonal.z * s_unquantizeRangeScale;

    return fPoint;
    }
