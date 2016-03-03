/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImporterCommandVisitor.h $
|    $RCSfile: ImporterCommandVisitor.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/23 19:49:08 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include "ImporterImpl.h"
#include <ScalableMesh/Import/ImportSequenceVisitor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
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




    void                            ImportLayer                (uint32_t                                        sourceLayer) const;

    void                            ImportLayerToLayer         (uint32_t                                        sourceLayer,
                                                                uint32_t                                        targetLayer) const;

    void                            ImportLayerToLayerType     (uint32_t                                        sourceLayer,
                                                                uint32_t                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerToType          (uint32_t                                        sourceLayer,
                                                                const DataTypeFamily&                       targetType) const;


    void                            ImportLayerType            (uint32_t                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType) const;
    
    void                            ImportLayerTypeToLayer     (uint32_t                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                uint32_t                                        targetLayer) const;

    void                            ImportLayerTypeToLayerType (uint32_t                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                uint32_t                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerTypeToLayerType (uint32_t                                        sourceLayer,
                                                                const DataType&                             sourceType,
                                                                uint32_t                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportLayerTypeToType      (uint32_t                                        sourceLayer,
                                                                const DataTypeFamily&                       sourceType,
                                                                const DataTypeFamily&                       targetType) const;


    void                            ImportType                 (const DataTypeFamily&                       sourceType) const;

    void                            ImportTypeToLayer          (const DataTypeFamily&                       sourceType,
                                                                uint32_t                                        targetLayer) const;

    void                            ImportTypeToLayerType      (const DataTypeFamily&                       sourceType,
                                                                uint32_t                                        targetLayer,
                                                                const DataTypeFamily&                       targetType) const;

    void                            ImportTypeToType           (const DataTypeFamily&                       sourceType,
                                                                const DataTypeFamily&                       targetType) const;



public:
    explicit                        CommandVisitor             (ImporterImpl&                               impl,
                                                                const Config&                               config);
    };  


} // END namespace Internal
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
