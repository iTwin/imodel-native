/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransformationUtils.h $
|    $RCSfile: TransformationUtils.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/07 14:26:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/GeoCoords/Transformation.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE


namespace {

struct UniformScaleFunctor
    {
    const double                    m_scale;

    explicit                        UniformScaleFunctor            (double                          scaleFactor) 
        :   m_scale(scaleFactor) {}

    DPoint3d                        operator ()                    (const DPoint3d&                 point) const
        {
        DPoint3d out = {point.x * m_scale, point.y * m_scale, point.z * m_scale};
        return out;
        }
    };


struct ScaleFunctor
    {
    const double                    m_x, m_y, m_z;

    explicit                        ScaleFunctor                   (double                          xFactor,
                                                                    double                          yFactor,
                                                                    double                          zFactor) 
        :   m_x(xFactor), m_y(yFactor), m_z(zFactor) {}

    DPoint3d                        operator ()                    (const DPoint3d&                 point) const
        {
        DPoint3d out = {point.x * m_x, point.y * m_y, point.z * m_z};
        return out;
        }
    };     


struct TranslateFunctor
    {
    const double                    m_x, m_y, m_z;

    explicit                        TranslateFunctor               (double                          xTranslation,
                                                                    double                          yTranslation,
                                                                    double                          zTranslation) 
        :   m_x(xTranslation), m_y(yTranslation), m_z(zTranslation) {}

    DPoint3d                        operator ()                    (const DPoint3d&                 point) const
        {
        DPoint3d out = {point.x + m_x, point.y + m_y, point.z + m_z};
        return out;
        }
    };     


struct ScaleTranslateFunctor
    {
    const double                    m_xScale, m_yScale, m_zScale;
    const double                    m_xTrans, m_yTrans, m_zTrans;

    explicit                        ScaleTranslateFunctor          (double                          xScaling,
                                                                    double                          yScaling,
                                                                    double                          zScaling,
                                                                    double                          xTranslation,
                                                                    double                          yTranslation,
                                                                    double                          zTranslation) 
        :   m_xScale(xScaling), m_yScale(yScaling), m_zScale(zScaling),
            m_xTrans(xTranslation), m_yTrans(yTranslation), m_zTrans(zTranslation) {}

    DPoint3d                        operator ()                    (const DPoint3d&                 point) const
        {
        DPoint3d out = {point.x*m_xScale + m_xTrans, point.y*m_yScale + m_yTrans, point.z*m_zScale + m_zTrans};
        return out;
        }
    };   


struct TransformFunctor
    {
    TransfoMatrix                   m;

    explicit                        TransformFunctor               (const TransfoMatrix&            matrix)
        :   m(matrix)
        {
        }

    DPoint3d                        operator ()                    (const DPoint3d&                 p) const
        {
        return m*p;
        }
    };

}

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
