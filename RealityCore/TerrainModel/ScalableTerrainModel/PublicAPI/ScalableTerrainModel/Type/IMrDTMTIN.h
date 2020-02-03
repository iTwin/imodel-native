/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableTerrainModel/Import/DataType.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeV0.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeFamilyV0.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

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
    BENTLEYSTM_EXPORT explicit                    TINTypeFamilyCreator                           ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TINType3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEYSTM_EXPORT explicit                    TINType3d64fCreator                            ();
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEYSTM_EXPORT explicit                    TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator   ();
    };


END_BENTLEY_MRDTM_NAMESPACE