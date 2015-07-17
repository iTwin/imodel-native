/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/ContentConfigVisitor.h $
|    $RCSfile: ContentConfigVisitor.h,v $
|   $Revision: 1.10 $
|       $Date: 2011/11/18 15:51:22 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/ContentConfig.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ContentConfigComponentBase;

struct LayerConfig;
struct GCSConfig;
struct GCSExtendedConfig;
struct TypeConfig;
struct GCSLocalAdjustmentConfig;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IContentConfigVisitor
    {
    virtual                     ~IContentConfigVisitor         () = 0 {}


    //virtual void                _Visit                         (const ContentConfigComponentBase&   config) { /* Do nothing*/ }

    virtual void                _Visit                         (const LayerConfig&                  config) = 0;
    virtual void                _Visit                         (const GCSConfig&                    config) = 0;
    virtual void                _Visit                         (const GCSExtendedConfig&            config) = 0;
    virtual void                _Visit                         (const TypeConfig&                   config) = 0;
    virtual void                _Visit                         (const GCSLocalAdjustmentConfig&     config) = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigVisitor : public IContentConfigVisitor
    {
    virtual                     ~ContentConfigVisitor          () = 0 {}

    virtual void                _Visit                         (const LayerConfig&                  config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const GCSConfig&                    config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const GCSExtendedConfig&            config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const TypeConfig&                   config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const GCSLocalAdjustmentConfig&     config) override { /* Do nothing*/ }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ILayerConfigVisitor
    {
    virtual                     ~ILayerConfigVisitor           () = 0 {}

    //virtual void                _Visit                         (const ContentConfigComponentBase&   config) { /* Do nothing*/ }

    virtual void                _Visit                         (const GCSConfig&                    config) = 0;
    virtual void                _Visit                         (const GCSExtendedConfig&            config) = 0;
    virtual void                _Visit                         (const TypeConfig&                   config) = 0;
    virtual void                _Visit                         (const GCSLocalAdjustmentConfig&     config) = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerConfigVisitor : public ILayerConfigVisitor
    {
    virtual                     ~LayerConfigVisitor            () = 0 {}

    virtual void                _Visit                         (const GCSConfig&                    config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const GCSExtendedConfig&            config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const TypeConfig&                   config) override { /* Do nothing*/ }
    virtual void                _Visit                         (const GCSLocalAdjustmentConfig&     config) override { /* Do nothing*/ }
    };

END_BENTLEY_MRDTM_IMPORT_NAMESPACE
