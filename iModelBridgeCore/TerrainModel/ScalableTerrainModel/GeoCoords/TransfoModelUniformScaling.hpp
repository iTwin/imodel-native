/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/GeoCoords/TransfoModelUniformScaling.hpp $
|    $RCSfile: TransfoModelUniformScaling.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:50 $
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
struct TransfoModelUniformScaling : public TransfoModelMixinBase<TransfoModelUniformScaling>
    {
private:       
    UniformScaleFunctor                 m_scaleFn;

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return TransfoMatrix(m_scaleFn.m_scale, 0.0,                0.0,                0.0,
                             0.0,               m_scaleFn.m_scale,  0.0,                0.0,
                             0.0,               0.0,                m_scaleFn.m_scale,  0.0);
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_scaleFn(sourcePt);
        return TransfoModel::S_SUCCESS;
        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_scaleFn);
        return TransfoModel::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        return new TransfoModelUniformScaling(1.0/m_scaleFn.m_scale);
        }


protected:
    explicit                            TransfoModelUniformScaling     (double                          scaleFactor)
        :   m_scaleFn(scaleFactor)
        {
        assert(!EqZeroEps(scaleFactor));
        }

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelUniformScaling     (const TransfoModelUniformScaling&      
                                                                                                        rhs)
        :   m_scaleFn(rhs.m_scaleFn)
        {
        }
public:
    static TransfoModelUniformScaling*  CreateFrom                     (double                          scaleFactor)
        {
        return new TransfoModelUniformScaling(scaleFactor);
        }
    };

} 
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE