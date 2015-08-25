/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ImportConfigVisitor.h $
|    $RCSfile: ImportConfigVisitor.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/21 17:32:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ImportConfigComponentBase;

struct AttachmentsConfig;
struct DefaultSourceGCSConfig;
struct DefaultTargetGCSConfig;
struct DefaultTargetLayerConfig;
struct DefaultTargetTypeConfig;
struct DefaultTargetScalableMeshConfig;
struct ImportExtractionConfig;
struct ImportFilteringConfig;
struct SourceFiltersConfig;
struct TargetFiltersConfig;

/*---------------------------------------------------------------------------------**//**
* @description  
* @see ImportConfig
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct IImportConfigVisitor
    {

    virtual                             ~IImportConfigVisitor          () = 0 {}


    //virtual void                        _Visit                         (const ImportConfigComponentBase&    config) { /* Do nothing*/ }

    virtual void                        _Visit                         (const AttachmentsConfig&            config) = 0;
    virtual void                        _Visit                         (const DefaultSourceGCSConfig&       config) = 0;
    virtual void                        _Visit                         (const DefaultTargetGCSConfig&       config) = 0;
    virtual void                        _Visit                         (const DefaultTargetLayerConfig&     config) = 0;
    virtual void                        _Visit                         (const DefaultTargetTypeConfig&      config) = 0;
    virtual void                        _Visit                           (const DefaultTargetScalableMeshConfig& config) = 0;
    virtual void                        _Visit                         (const ImportExtractionConfig&       config) = 0;
    virtual void                        _Visit                         (const ImportFilteringConfig&        config) = 0;
    virtual void                        _Visit                         (const SourceFiltersConfig&          config) = 0;
    virtual void                        _Visit                         (const TargetFiltersConfig&          config) = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfigVisitor : public IImportConfigVisitor
    {
    virtual                             ~ImportConfigVisitor           () = 0 {}

    virtual void                        _Visit                         (const AttachmentsConfig&            config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const DefaultSourceGCSConfig&       config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const DefaultTargetGCSConfig&       config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const DefaultTargetLayerConfig&     config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const DefaultTargetTypeConfig&      config) override { /* Do nothing*/ }
    virtual void                        _Visit                           (const DefaultTargetScalableMeshConfig& config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const ImportExtractionConfig&       config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const ImportFilteringConfig&        config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const SourceFiltersConfig&          config) override { /* Do nothing*/ }
    virtual void                        _Visit                         (const TargetFiltersConfig&          config) override { /* Do nothing*/ }
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
