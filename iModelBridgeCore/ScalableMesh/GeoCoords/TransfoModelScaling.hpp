/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelScaling.hpp $
|    $RCSfile: TransfoModelScaling.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef INCLUDED_FROM_TRANSFORMATION_CPP 
    #error "See note" 
#endif
#include <STMInternal/Foundations/FoundationsPrivateTools.h>
// NOTE: This header is expected to be included only inside specific compilation units and 
// so has access to all this compilation unit included headers (as long as they are included
// prior to inclusion of this one.

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
namespace Internal {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelScaling : public TransfoModelMixinBase<TransfoModelScaling>
    {
private:       
    ScaleFunctor                        m_scaleFn;

    explicit                            TransfoModelScaling            (double                          xFactor,
                                                                        double                          yFactor,
                                                                        double                          zFactor)
        :   m_scaleFn(xFactor, yFactor, zFactor)
        {
        assert(!EqEps(xFactor, 0.0));
        assert(!EqEps(yFactor, 0.0));
        assert(!EqEps(zFactor, 0.0));
        }

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelScaling            (const TransfoModelScaling&      rhs)
        :   m_scaleFn(rhs.m_scaleFn)
        {
        }

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return TransfoMatrix(m_scaleFn.m_x, 0.0, 0.0, 0.0,
                             0.0, m_scaleFn.m_y, 0.0, 0.0,
                             0.0, 0.0, m_scaleFn.m_z, 0.0);
        }

    virtual SMStatus        _Transform(const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_scaleFn(sourcePt);
        return SMStatus::S_SUCCESS;
        }


    virtual SMStatus        _Transform(const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_scaleFn);
        return SMStatus::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        return new TransfoModelScaling(1.0/m_scaleFn.m_x, 1.0/m_scaleFn.m_y, 1.0/m_scaleFn.m_z);
        }
public:
    static TransfoModelScaling*         CreateFrom                     (double                          xFactor,
                                                                        double                          yFactor,
                                                                        double                          zFactor)
        {
        return new TransfoModelScaling(xFactor, yFactor, zFactor);
        }

    };

}
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
