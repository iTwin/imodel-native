/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelIdentity.hpp $
|    $RCSfile: TransfoModelIdentity.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:58 $
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
struct TransfoModelIdentity : public TransfoModelMixinBase<TransfoModelIdentity>
    {
private:          
    explicit                            TransfoModelIdentity           () {}

    // TDORAY: Same as default. Use new C++0x feature
                                        TransfoModelIdentity           (const TransfoModelIdentity&     rhs) {}

    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        return true;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        return TransfoMatrix(1.0, 0.0, 0.0, 0.0,
                             0.0, 1.0, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0);
        }

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        targetPt = sourcePt;
        return TransfoModel::S_SUCCESS;
        }


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        memcpy(targetPtP, sourcePtP, sizeof(*targetPtP)*sourcePtQty);
        return TransfoModel::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        return new TransfoModelIdentity;
        }
public:
    static TransfoModelIdentity*        CreateFrom                     ()
        {
        return new TransfoModelIdentity;
        }
    };

} 
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
