/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ImporterCommandVisitor.h $
|    $RCSfile: ImporterCommandVisitor.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/23 19:49:08 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include "ImporterImpl.h"
#include <ScalableTerrainModel/Import/ImportSequenceVisitor.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
namespace Internal {


class Config;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CommandVisitor : public IImportSequenceVisitor
    {
    ImporterImpl&                   m_impl;
    const Config&                   m_config;

    virtual void                    _Visit                     (const ImportAllCommand&                     command) override;
    virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override;
    virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override;
    virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override;

    virtual void                    _Visit                     (const ImportLayerCommand&                   command) override;
    virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override;
    virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override;
    virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override;

    virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override;
    virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override;
    virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override;
    virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override;

    virtual void                    _Visit                     (const ImportTypeCommand&                    command) override;
    virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override;
    virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override;
    virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override;




    void                            ImportLayer                (UInt                                        sourceLayer) const;

    void                            ImportLayerToLayer         (UInt                                        sourceLayer,
                                                                UInt                                        targetLayer) const;

    void                            ImportLayerToLayerType     (UInt                                        sourceLayer,
                                                                UInt                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerToType          (UInt                                        sourceLayer,
                                                                const DataTypeFamily&                       targetType) const;


    void                            ImportLayerType            (UInt                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType) const;
    
    void                            ImportLayerTypeToLayer     (UInt                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                UInt                                        targetLayer) const;

    void                            ImportLayerTypeToLayerType (UInt                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                UInt                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerTypeToLayerType (UInt                                        sourceLayer,
                                                                const DataType&                             sourceType,
                                                                UInt                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerTypeToType      (UInt                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                const DataTypeFamily&                       targetType) const;


    void                            ImportType                 (const DataTypeFamily&                       sourceType) const;

    void                            ImportTypeToLayer          (const DataTypeFamily&                       sourceType,
                                                                UInt                                        targetLayer) const;

    void                            ImportTypeToLayerType      (const DataTypeFamily&                       sourceType,
                                                                UInt                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportTypeToType           (const DataTypeFamily&                       sourceType,
                                                                const DataTypeFamily&                       targetType) const;



public:
    explicit                        CommandVisitor             (ImporterImpl&                               impl,
                                                                const Config&                               config);
    };  


} // END namespace Internal
END_BENTLEY_MRDTM_IMPORT_NAMESPACE