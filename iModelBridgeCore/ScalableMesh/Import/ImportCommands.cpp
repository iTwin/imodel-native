/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportCommands.cpp $
|    $RCSfile: ImportCommands.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/10/21 17:32:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/Command/All.h>
#include "ImportCommandMixinBaseImpl.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllCommand::ImportAllCommand (const ImportAllCommand& rhs)
    :   super_class(rhs)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllCommand::ImportAllCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllCommand::~ImportAllCommand ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerCommand::ImportAllToLayerCommand   (const ImportAllToLayerCommand&  rhs)
    :   super_class(rhs),
        m_targetLayer(rhs.m_targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerCommand::ImportAllToLayerCommand   (uint32_t targetLayer)
    :   m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerCommand::~ImportAllToLayerCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportAllToLayerCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerTypeCommand::ImportAllToLayerTypeCommand   (const ImportAllToLayerTypeCommand&    rhs)
    :   super_class(rhs),
        m_targetType(rhs.m_targetType), m_targetLayer(rhs.m_targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerTypeCommand::ImportAllToLayerTypeCommand   (uint32_t                    targetLayer,
                                                            const DataTypeFamily&   targetType)
    :   m_targetType(targetType), m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToLayerTypeCommand::~ImportAllToLayerTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportAllToLayerTypeCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportAllToLayerTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToTypeCommand::ImportAllToTypeCommand (const ImportAllToTypeCommand& rhs)
    :   super_class(rhs),
        m_targetType(rhs.m_targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToTypeCommand::ImportAllToTypeCommand (const DataTypeFamily& targetType)
    :   m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportAllToTypeCommand::~ImportAllToTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportAllToTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerCommand::ImportLayerCommand (uint32_t layer)
    :   m_sourceLayer(layer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerCommand::ImportLayerCommand (const ImportLayerCommand& rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerCommand::~ImportLayerCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerCommand::ImportLayerToLayerCommand (const ImportLayerToLayerCommand&  rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_targetLayer(rhs.m_targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerCommand::ImportLayerToLayerCommand   (uint32_t    sourceLayer,
                                                        uint32_t    targetLayer)
    :   m_sourceLayer(sourceLayer), m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerCommand::~ImportLayerToLayerCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerToLayerCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerToLayerCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerTypeCommand::ImportLayerToLayerTypeCommand (const ImportLayerToLayerTypeCommand& rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_targetLayer(rhs.m_targetLayer), m_targetType(rhs.m_targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerTypeCommand::ImportLayerToLayerTypeCommand   (uint32_t                    sourceLayer,
                                                                uint32_t                    targetLayer,
                                                                const DataTypeFamily&   targetType)
    :   m_sourceLayer(sourceLayer), m_targetLayer(targetLayer), m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToLayerTypeCommand::~ImportLayerToLayerTypeCommand  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerToLayerTypeCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerToLayerTypeCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerToLayerTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToTypeCommand::ImportLayerToTypeCommand (const ImportLayerToTypeCommand&         rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_targetType(rhs.m_targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToTypeCommand::ImportLayerToTypeCommand (uint32_t                    sourceLayer,
                                                    const DataTypeFamily&   targetType)
    :   m_sourceLayer(sourceLayer), m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerToTypeCommand::~ImportLayerToTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerToTypeCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerToTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeCommand::ImportLayerTypeCommand (const ImportLayerTypeCommand&   rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_sourceType(rhs.m_sourceType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeCommand::ImportLayerTypeCommand (uint32_t                    sourceLayer,
                                                const DataTypeFamily&   sourceType)
    :   m_sourceLayer(sourceLayer), m_sourceType(sourceType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeCommand::~ImportLayerTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerCommand::ImportLayerTypeToLayerCommand   (const ImportLayerTypeToLayerCommand& rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_sourceType(rhs.m_sourceType), m_targetLayer(rhs.m_targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerCommand::ImportLayerTypeToLayerCommand   (uint32_t                    sourceLayer,
                                                                const DataTypeFamily&   sourceType,
                                                                uint32_t                    targetLayer)
    :   m_sourceLayer(sourceLayer), m_sourceType(sourceType), m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerCommand::~ImportLayerTypeToLayerCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeToLayerCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeToLayerCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeToLayerCommand::GetTargetLayer () const
    {
    return m_sourceLayer;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerTypeCommand::ImportLayerTypeToLayerTypeCommand     (const ImportLayerTypeToLayerTypeCommand& rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_sourceType(rhs.m_sourceType),
        m_targetLayer(rhs.m_targetLayer), m_targetType(rhs.m_targetType)

    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerTypeCommand::ImportLayerTypeToLayerTypeCommand   (uint32_t                    sourceLayer,
                                                                        const DataTypeFamily&   sourceType,
                                                                        uint32_t                    targetLayer,
                                                                        const DataTypeFamily&   targetType)
    :   m_sourceLayer(sourceLayer), m_sourceType(sourceType),
        m_targetLayer(targetLayer), m_targetType(targetType)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToLayerTypeCommand::~ImportLayerTypeToLayerTypeCommand  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeToLayerTypeCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeToLayerTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeToLayerTypeCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeToLayerTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToTypeCommand::ImportLayerTypeToTypeCommand (const ImportLayerTypeToTypeCommand& rhs)
    :   super_class(rhs),
        m_sourceLayer(rhs.m_sourceLayer), m_sourceType(rhs.m_sourceType), m_targetType(rhs.m_targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToTypeCommand::ImportLayerTypeToTypeCommand (uint32_t                    sourceLayer,
                                                            const DataTypeFamily&   sourceType,
                                                            const DataTypeFamily&   targetType)
    :   m_sourceLayer(sourceLayer), m_sourceType(sourceType), m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportLayerTypeToTypeCommand::~ImportLayerTypeToTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportLayerTypeToTypeCommand::GetSourceLayer () const
    {
    return m_sourceLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeToTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportLayerTypeToTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeCommand::ImportTypeCommand (const ImportTypeCommand& rhs)
    :   super_class(rhs),
        m_sourceType(rhs.m_sourceType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeCommand::ImportTypeCommand (const DataTypeFamily& sourceType)
    :   m_sourceType(sourceType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeCommand::~ImportTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerCommand::ImportTypeToLayerCommand (const ImportTypeToLayerCommand&         rhs)
    :   super_class(rhs),
        m_sourceType(rhs.m_sourceType), m_targetLayer(rhs.m_targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerCommand::ImportTypeToLayerCommand (const DataTypeFamily&   sourceType,
                                                    uint32_t                    targetLayer)
    :   m_sourceType(sourceType), m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerCommand::~ImportTypeToLayerCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeToLayerCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportTypeToLayerCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToTypeCommand::ImportTypeToTypeCommand (const ImportTypeToTypeCommand& rhs)
    :   super_class(rhs),
        m_sourceType(rhs.m_sourceType), m_targetType(rhs.m_targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToTypeCommand::ImportTypeToTypeCommand   (const DataTypeFamily&   sourceType,
                                                    const DataTypeFamily&   targetType)
    :   m_sourceType(sourceType), m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToTypeCommand::~ImportTypeToTypeCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeToTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeToTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerTypeCommand::ImportTypeToLayerTypeCommand (const ImportTypeToLayerTypeCommand&    rhs)
    :   super_class(rhs),
        m_sourceType(rhs.m_sourceType), m_targetType(rhs.m_targetType),
        m_targetLayer(rhs.m_targetLayer)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerTypeCommand::ImportTypeToLayerTypeCommand (const DataTypeFamily&   sourceType,
                                                            uint32_t                    targetLayer,
                                                            const DataTypeFamily&   targetType)
    :   m_sourceType(sourceType), m_targetType(targetType),
        m_targetLayer(targetLayer)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportTypeToLayerTypeCommand::~ImportTypeToLayerTypeCommand    ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeToLayerTypeCommand::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ImportTypeToLayerTypeCommand::GetTargetLayer () const
    {
    return m_targetLayer;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& ImportTypeToLayerTypeCommand::GetTargetType () const
    {
    return m_targetType;
    }






END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
