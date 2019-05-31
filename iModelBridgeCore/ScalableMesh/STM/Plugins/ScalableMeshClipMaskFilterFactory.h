/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshClipMaskFilterFactory.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/09/01 14:07:05 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <TerrainModel/TerrainModel.h>

#include <ScalableMesh/Import/Plugin/CustomFilterFactoryV0.h>
#include <ImagePP/all/h/HVEClipShape.h>
#include <ScalableMesh/Import/CustomFilterFactory.h>

USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ClipMaskFilterFactory : public Import::Plugin::V0::CustomFilterFactoryBase
    {
    struct Impl;
    auto_ptr<Impl>                      m_pImpl;

    BENTLEY_SM_EXPORT explicit                            ClipMaskFilterFactory  (Impl*                               implP);
                                        ~ClipMaskFilterFactory ();

    virtual const FilterCreatorBase*    _FindCreatorFor        (const DataType&                     sourceType,
                                                                Log&                                log) const override;

public:
    BENTLEY_SM_EXPORT static Import::CustomFilterFactory  CreateFrom             (const HFCPtr<HVEClipShape>&         shapePtr);
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
