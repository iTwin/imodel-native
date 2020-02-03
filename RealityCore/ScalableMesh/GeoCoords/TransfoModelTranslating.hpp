/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    virtual SMStatus        _Transform(const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = m_translateFn(sourcePt);
        return SMStatus::S_SUCCESS;
        }


    virtual SMStatus       _Transform(const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        std::transform(sourcePtP, sourcePtP + sourcePtQty, targetPtP, m_translateFn);
        return SMStatus::S_SUCCESS;
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
