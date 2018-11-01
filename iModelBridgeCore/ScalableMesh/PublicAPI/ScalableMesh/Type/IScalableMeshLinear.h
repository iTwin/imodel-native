/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Type/IScalableMeshLinear.h $
|    $RCSfile: IScalableMeshLinear.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/30 19:04:08 $
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


namespace LinearDimensionDef
    {
    enum 
        {
        ROLE_UNKNOWN,
        ROLE_XCOORDINATE,
        ROLE_YCOORDINATE,
        ROLE_ZCOORDINATE,
        ROLE_RED_COLOR_COMPONENT,
        ROLE_GREEN_COLOR_COMPONENT,
        ROLE_BLUE_COLOR_COMPONENT,
        ROLE_COLOR_INTENSITY,
        ROLE_GROUP_ID,
        ROLE_SIGNIFICANCE_LEVEL,
        ROLE_CLASSIFICATION,
        // NOTE: Previous roles must correspond to roles defined in PointDimensionDef

        ROLE_FEATURE_TYPE,
        ROLE_POINT_INDEX,
        ROLE_POINT_QTY,

        ROLE_QTY,
        };
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class LinearTypeFamilyCreator : public Import::Plugin::V0::StaticDataTypeFamilyCreatorBase
    {
    virtual const Import::DataTypeFamily&           _Create                                        () const override;
public:
    BENTLEY_SM_EXPORT explicit                    LinearTypeFamilyCreator                        ();
    };

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class LinearTypeTi32Pi32Pq32Gi32_3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEY_SM_EXPORT explicit                    LinearTypeTi32Pi32Pq32Gi32_3d64fCreator        ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEY_SM_EXPORT explicit                    LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator    ();
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
