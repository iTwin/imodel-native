/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Type/IScalableMeshPoint.h $
|    $RCSfile: IScalableMeshPoint.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/30 19:04:06 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/DataTypeFamily.h>
#include <ScalableMesh/Import/Plugin/DataTypeV0.h>
#include <ScalableMesh/Import/Plugin/DataTypeFamilyV0.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace PointDimensionDef
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
        // NOTE: When updating this listing, also update LinearDimensionRef listing

        ROLE_QTY,
        };
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PointTypeFamilyCreator : public Import::Plugin::V0::StaticDataTypeFamilyCreatorBase
    {
    virtual const Import::DataTypeFamily&         _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointTypeFamilyCreator                         ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public: 
    BENTLEYSTM_EXPORT explicit                    PointType3d64fCreator                          ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64fM64fCreator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointType3d64fM64fCreator                      ();
    

    };

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64fG32Creator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointType3d64fG32Creator                       ();
    };


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64fM64fG32Creator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointType3d64fM64fG32Creator                   ();
    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64f_R16G16B16_I16Creator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&                 _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointType3d64f_R16G16B16_I16Creator            ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PointType3d64f_R16G16B16_I16_C8Creator : public Import::Plugin::V0::StaticDataTypeCreatorBase
    {
    virtual const Import::DataType&         _Create                                        () const override;
public:
    BENTLEYSTM_EXPORT explicit                    PointType3d64f_R16G16B16_I16_C8Creator         ();
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
