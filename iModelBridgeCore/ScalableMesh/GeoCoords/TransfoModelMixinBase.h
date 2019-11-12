/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    virtual                             ~TransfoModelMixinBase         () {}

    static ClassID                      s_GetClassID                   () 
        {
        static const ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }
    };

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
