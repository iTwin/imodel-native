/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransfoModelMixinBase.h $
|    $RCSfile: TransfoModelMixinBase.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:56 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TransfoModelBase.h"

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct TransfoModelMixinBase : public TransfoModelBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                     _GetClassID                    () const override
        {
        return s_GetClassID();
        }
public:
    virtual                             ~TransfoModelMixinBase         () = 0 {}

    static ClassID                      s_GetClassID                   () 
        {
        static const ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }
    };

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
