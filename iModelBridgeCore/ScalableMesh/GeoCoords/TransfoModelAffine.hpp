/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelAffine.hpp $
|    $RCSfile: TransfoModelAffine.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:27:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef INCLUDED_FROM_TRANSFORMATION_CPP 
    #error "See note" 
#endif

// NOTE: This header is expected to be included only inside specific compilation units and 
// so has access to all this compilation unit included headers (as long as they are included
// prior to inclusion of this one.

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
namespace Internal {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelAffine : public TransfoModelMixinBase<TransfoModelAffine>
    {
private:       
    TransformFunctor                    m_transFn;

    explicit                            TransfoModelAffine             (const TransfoMatrix&            affineMatrix)
        :   m_transFn(affineMatrix)
        {
        // TDORAY: Make some debug validations here
        }

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelAffine             (const TransfoModelAffine&       rhs)
        :   m_transFn(rhs.m_transFn)
        {
        }

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return m_transFn.m;
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_transFn(sourcePt);
        return TransfoModel::S_SUCCESS;
        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_transFn);
        return TransfoModel::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {

        Transform transform(ToTransform3d(m_transFn.m));

        if (!bsiTransform_invert(&transform, &transform))
            return 0;

        return new TransfoModelAffine(FromTransform3d(transform));
        }
public:
    static TransfoModelAffine*          CreateFrom                     (const TransfoMatrix&            affineMatrix)
        {
        return new TransfoModelAffine(affineMatrix);
        }
    };
}
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
