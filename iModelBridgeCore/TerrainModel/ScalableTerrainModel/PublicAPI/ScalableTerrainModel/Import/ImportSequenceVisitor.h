/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/ImportSequenceVisitor.h $
|    $RCSfile: ImportSequenceVisitor.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/10/21 17:32:10 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ImportCommandBase;

struct ImportAllCommand;
struct ImportAllToLayerCommand;
struct ImportAllToLayerTypeCommand;
struct ImportAllToTypeCommand;

struct ImportLayerCommand;
struct ImportLayerToLayerCommand;
struct ImportLayerToLayerTypeCommand;
struct ImportLayerToTypeCommand;

struct ImportLayerTypeCommand;
struct ImportLayerTypeToLayerCommand;
struct ImportLayerTypeToLayerTypeCommand;
struct ImportLayerTypeToTypeCommand;

struct ImportTypeCommand;
struct ImportTypeToLayerCommand;
struct ImportTypeToLayerTypeCommand;
struct ImportTypeToTypeCommand;




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IImportSequenceVisitor
    {
    virtual                     ~IImportSequenceVisitor        () = 0 {}


    //virtual void                _Visit                         (const ImportCommandBase&                    command) { /* Do nothing*/ }

    virtual void                _Visit                         (const ImportAllCommand&                     command) = 0;
    virtual void                _Visit                         (const ImportAllToLayerCommand&              command) = 0;
    virtual void                _Visit                         (const ImportAllToLayerTypeCommand&          command) = 0;
    virtual void                _Visit                         (const ImportAllToTypeCommand&               command) = 0;

    virtual void                _Visit                         (const ImportLayerCommand&                   command) = 0;
    virtual void                _Visit                         (const ImportLayerToLayerCommand&            command) = 0;
    virtual void                _Visit                         (const ImportLayerToLayerTypeCommand&        command) = 0;
    virtual void                _Visit                         (const ImportLayerToTypeCommand&             command) = 0;

    virtual void                _Visit                         (const ImportLayerTypeCommand&               command) = 0;
    virtual void                _Visit                         (const ImportLayerTypeToLayerCommand&        command) = 0;
    virtual void                _Visit                         (const ImportLayerTypeToLayerTypeCommand&    command) = 0;
    virtual void                _Visit                         (const ImportLayerTypeToTypeCommand&         command) = 0;

    virtual void                _Visit                         (const ImportTypeCommand&                    command) = 0;
    virtual void                _Visit                         (const ImportTypeToLayerCommand&             command) = 0;
    virtual void                _Visit                         (const ImportTypeToLayerTypeCommand&         command) = 0;
    virtual void                _Visit                         (const ImportTypeToTypeCommand&              command) = 0;
    };

typedef IImportSequenceVisitor  IImportCommandVisitor;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportSequenceVisitor : public IImportSequenceVisitor
    {
    virtual                     ~ImportSequenceVisitor         () = 0 {}

    virtual void                _Visit                         (const ImportAllCommand&                     command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportAllToLayerCommand&              command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportAllToLayerTypeCommand&          command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportAllToTypeCommand&               command) override { /* Do nothing*/ }

    virtual void                _Visit                         (const ImportLayerCommand&                   command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerToLayerCommand&            command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerToLayerTypeCommand&        command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerToTypeCommand&             command) override { /* Do nothing*/ }

    virtual void                _Visit                         (const ImportLayerTypeCommand&               command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerTypeToLayerCommand&        command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerTypeToLayerTypeCommand&    command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportLayerTypeToTypeCommand&         command) override { /* Do nothing*/ }

    virtual void                _Visit                         (const ImportTypeCommand&                    command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportTypeToLayerCommand&             command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportTypeToLayerTypeCommand&         command) override { /* Do nothing*/ }
    virtual void                _Visit                         (const ImportTypeToTypeCommand&              command) override { /* Do nothing*/ }


    };



typedef ImportSequenceVisitor   ImportCommandVisitor;

END_BENTLEY_MRDTM_IMPORT_NAMESPACE