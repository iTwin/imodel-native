/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Type/IScalableMeshTIN.h $
|    $RCSfile: IScalableMeshTIN.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/10 15:21:35 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/Plugin/DataTypeV0.h>
#include <ScalableMesh/Import/Plugin/DataTypeFamilyV0.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace TINDimensionDef
    {
    enum 
        {
        ROLE_UNKNOWN,
        ROLE_CUSTOM,
        ROLE_QTY,
        };
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TINTypeFamilyCreator : public Import::Plugin::V0::StaticDataTypeFamilyCreatorBase
    {
    virtual const Import::DataTypeFamily&           _Create                                        () const override;
public:
    BENTLEY_SM_EXPORT explicit                    TINTypeFamilyCreator                           ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TINType3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEY_SM_EXPORT explicit                    TINType3d64fCreator                            ();
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEY_SM_EXPORT explicit                    TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator   ();
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
