/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/GeoCoords/TransfoModelCombined.hpp $
|    $RCSfile: TransfoModelCombined.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:27:00 $
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
struct TransfoModelCombined : public TransfoModelMixinBase<TransfoModelCombined>
    {
private:       
    TransfoModelBaseCPtr                m_0P;
    TransfoModelBaseCPtr                m_1P;

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return BaseHandler::IsConvertibleToMatrix(*m_0P) && BaseHandler::IsConvertibleToMatrix(*m_1P);
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return BaseHandler::ConvertToMatrix(*m_1P) * BaseHandler::ConvertToMatrix(*m_0P);
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        return (TransfoModel::S_SUCCESS == BaseHandler::Transform(*m_0P, sourcePt, targetPt) &&
                TransfoModel::S_SUCCESS == BaseHandler::Transform(*m_1P, targetPt, targetPt)) ? 
                    TransfoModel::S_SUCCESS : TransfoModel::S_ERROR;

        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        return (TransfoModel::S_SUCCESS == BaseHandler::Transform(*m_0P, sourcePtP, sourcePtQty, targetPtP) &&
                TransfoModel::S_SUCCESS == BaseHandler::Transform(*m_1P, targetPtP, sourcePtQty, targetPtP)) ? 
                    TransfoModel::S_SUCCESS : TransfoModel::S_ERROR;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        std::auto_ptr<TransfoModelBase> inv0P(BaseHandler::CreateInverse(*m_0P));

        if (0 == inv0P.get())
            return 0;

        std::auto_ptr<TransfoModelBase> inv1P(BaseHandler::CreateInverse(*m_1P));

        if (0 == inv1P.get())
            return 0;

        return new TransfoModelCombined(inv1P.release(), inv0P.release());
        }
protected:
    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelCombined           (const TransfoModelCombined&     rhs)
        :   m_0P(rhs.m_0P),
            m_1P(rhs.m_1P)
        {
        }

    explicit                            TransfoModelCombined           (const TransfoModel&             t0,
                                                                        const TransfoModel&             t1)
        :   m_0P(Handler::GetBasePtr(t0)), 
            m_1P(Handler::GetBasePtr(t1))
        {
        }


    explicit                            TransfoModelCombined           (TransfoModelBase*               t0P,
                                                                        TransfoModelBase*               t1P)
        :   m_0P(t0P), 
            m_1P(t1P)
        {
        }
public:
    static TransfoModelCombined*        CreateFrom                     (const TransfoModel&             t0,
                                                                        const TransfoModel&             t1)
        {
        return new TransfoModelCombined(t0, t1);
        }

    static TransfoModelCombined*        CreateFrom                     (TransfoModelBase*               t0P,
                                                                        TransfoModelBase*               t1P)
        {
        return new TransfoModelCombined(t0P, t1P);
        }

    };

} // END unnamed namespace
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE