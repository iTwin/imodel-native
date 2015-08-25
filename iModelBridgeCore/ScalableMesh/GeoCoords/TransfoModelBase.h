/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelBase.h $
|    $RCSfile: TransfoModelBase.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:27:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/GeoCoords/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

struct TransfoModelBase;

typedef SharedPtrTypeTrait<TransfoModelBase>::type 
                                        TransfoModelBasePtr;
typedef SharedPtrTypeTrait<const TransfoModelBase>::type 
                                        TransfoModelBaseCPtr;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelBase : private Unassignable, public ShareableObjectTypeTrait<TransfoModelBase>::type
    {
protected:
    typedef const std::type_info*       ClassID;

private:
    friend struct                       TransfoModel;
    friend struct                       TransfoModelHandler;
    friend struct                       TransfoModelBaseHandler;

    virtual ClassID                     _GetClassID                    () const = 0;

    virtual bool                        _IsConvertibleToMatrix         () const = 0;

    virtual TransfoMatrix               _ConvertToMatrix               () const = 0;

    virtual TransfoModel::Status        _Transform                     (const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const = 0;


    virtual TransfoModel::Status        _Transform                     (const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const = 0;


    virtual TransfoModelBase*           _CreateInverse                 () const = 0;

public:
    virtual                             ~TransfoModelBase              () = 0 {}
    };


END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
