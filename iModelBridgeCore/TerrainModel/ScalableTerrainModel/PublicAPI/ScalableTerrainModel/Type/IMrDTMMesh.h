/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Type/IMrDTMMesh.h $
|    $RCSfile: IMrDTMMesh.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/10 15:21:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableTerrainModel/Import/DataType.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeV0.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeFamilyV0.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

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
    BENTLEYSTM_EXPORT explicit                    MeshTypeFamilyCreator                          ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshType3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEYSTM_EXPORT explicit                    MeshType3d64fCreator                           ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEYSTM_EXPORT explicit                    MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator  ();
    };

END_BENTLEY_MRDTM_NAMESPACE