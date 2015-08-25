/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelTranslating.hpp $
|    $RCSfile: TransfoModelTranslating.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:51 $
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
struct TransfoModelTranslating : public TransfoModelMixinBase<TransfoModelTranslating>
    {
private:
    TranslateFunctor                    m_translateFn;

    explicit                            TransfoModelTranslating        (double                          xTranslation,
                                                                        double                          yTranslation,
                                                                        double                          zTranslation)
        :   m_translateFn(xTranslation, yTranslation, zTranslation)
        {
        }

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelTranslating        (const TransfoModelTranslating&      rhs)
        :   m_translateFn(rhs.m_translateFn)
        {
        }

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return TransfoMatrix(1.0, 0.0, 0.0, m_translateFn.m_x,
                             0.0, 1.0, 0.0, m_translateFn.m_y,
                             0.0, 0.0, 1.0, m_translateFn.m_z);
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_translateFn(sourcePt);
        return TransfoModel::S_SUCCESS;
        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_translateFn);
        return TransfoModel::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        return new TransfoModelTranslating(-m_translateFn.m_x, -m_translateFn.m_y, -m_translateFn.m_z);
        }
public:
    static TransfoModelTranslating*     CreateFrom                     (double                          xTranslation,
                                                                        double                          yTranslation,
                                                                        double                          zTranslation)
        {
        return new TransfoModelTranslating(xTranslation, yTranslation, zTranslation);
        }
    };


} 
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
