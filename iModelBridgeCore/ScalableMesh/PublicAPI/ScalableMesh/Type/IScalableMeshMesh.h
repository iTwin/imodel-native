/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/Plugin/DataTypeV0.h>
#include <ScalableMesh/Import/Plugin/DataTypeFamilyV0.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace MeshDimensionDef
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
class MeshTypeFamilyCreator : public Import::Plugin::V0::StaticDataTypeFamilyCreatorBase
    {
    virtual const Import::DataTypeFamily&           _Create                                        () const override;
public:
    BENTLEY_SM_EXPORT explicit                    MeshTypeFamilyCreator                          ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshType3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEY_SM_EXPORT explicit                    MeshType3d64fCreator                           ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEY_SM_EXPORT explicit                    MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator  ();
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
