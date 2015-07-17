/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/GeoCoords/TransfoModelScalingTranslating.hpp $
|    $RCSfile: TransfoModelScalingTranslating.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:53 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef INCLUDED_FROM_TRANSFORMATION_CPP 
    #error "See note" 
#endif

// NOTE: This header is expected to be included only inside specific compilation units and 
// so has access to all this compilation unit included headers (as long as they are included
// prior to inclusion of this one.

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
namespace Internal {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelScalingTranslating : public TransfoModelMixinBase<TransfoModelScalingTranslating>
    {
private:
    ScaleTranslateFunctor               m_scaleTransFn;

    explicit                            TransfoModelScalingTranslating (double                          xScaling,
                                                                        double                          yScaling,
                                                                        double                          zScaling,
                                                                        double                          xTranslation,
                                                                        double                          yTranslation,
                                                                        double                          zTranslation)
        :   m_scaleTransFn(xScaling, yScaling, zScaling, 
                           xTranslation, yTranslation, zTranslation)
        {
        assert(!EqEps(xScaling, 0.0));
        assert(!EqEps(yScaling, 0.0));
        assert(!EqEps(zScaling, 0.0));
        }

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelScalingTranslating (const TransfoModelScalingTranslating&   
                                                                                                        rhs)
        :   m_scaleTransFn(rhs.m_scaleTransFn)
        {
        }

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return TransfoMatrix(m_scaleTransFn.m_xScale, 0.0,                     0.0,                     m_scaleTransFn.m_xTrans,
                             0.0,                     m_scaleTransFn.m_yScale, 0.0,                     m_scaleTransFn.m_yTrans,
                             0.0,                     0.0,                     m_scaleTransFn.m_zScale, m_scaleTransFn.m_zTrans);
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_scaleTransFn(sourcePt);
        return TransfoModel::S_SUCCESS;
        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_scaleTransFn);
        return TransfoModel::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        return new TransfoModelScalingTranslating(1.0/m_scaleTransFn.m_xScale, 
                                                  1.0/m_scaleTransFn.m_yScale, 
                                                  1.0/m_scaleTransFn.m_zScale,
                                                  (-m_scaleTransFn.m_xTrans)/m_scaleTransFn.m_xScale, 
                                                  (-m_scaleTransFn.m_yTrans)/m_scaleTransFn.m_yScale, 
                                                  (-m_scaleTransFn.m_zTrans)/m_scaleTransFn.m_zScale);
        }

public:
    static TransfoModelScalingTranslating*         
                                        CreateFrom                     (double                          xScaling,
                                                                        double                          yScaling,
                                                                        double                          zScaling,
                                                                        double                          xTranslation,
                                                                        double                          yTranslation,
                                                                        double                          zTranslation)
        {
        return new TransfoModelScalingTranslating(xScaling, yScaling, zScaling, 
                                                  xTranslation, yTranslation, zTranslation);
        }

    };

} 
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE