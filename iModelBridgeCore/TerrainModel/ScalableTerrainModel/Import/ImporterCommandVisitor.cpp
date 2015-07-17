/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ImporterCommandVisitor.cpp $
|    $RCSfile: ImporterCommandVisitor.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/26 18:46:52 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "ImporterCommandVisitor.h"
#include <ScalableTerrainModel/Import/Command/All.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
namespace Internal {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CommandVisitor::CommandVisitor (ImporterImpl&       impl,
                                const Config&       config)
    :   m_impl(impl),
        m_config(config)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayer   (UInt sourceLayer) const
    {
    const UInt targetLayer = m_config.HasDefaultTargetLayer() ? m_config.GetDefaultTargetLayer() : sourceLayer;
    ImportLayerToLayer(sourceLayer, targetLayer);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::ImportLayerToLayer    (UInt    sourceLayer,
                                            UInt    targetLayer) const
    {
    if (m_config.HasDefaultTargetType())
        {
        ImportLayerToLayerType(sourceLayer, targetLayer, m_config.GetDefaultTargetType());
        }
    else
        {
        if (!m_impl.m_sourceDesc.IsValidLayer(sourceLayer))
            throw CustomException(L"Invalid command data");

        const LayerDesc& layerDesc = m_impl.m_sourceDesc.GetLayer(sourceLayer);

        for (LayerDesc::TypeCIterator typeIt = layerDesc.TypesBegin(), typesEnd = layerDesc.TypesEnd(); typeIt != typesEnd; ++typeIt)
            {
            ImportLayerTypeToLayerType(layerDesc.GetID(), *typeIt, targetLayer, typeIt->GetFamily());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::ImportLayerToLayerType    (UInt                    sourceLayer,
                                                UInt                    targetLayer,
                                                const DataTypeFamily&   targetType) const
    {
    if (!m_impl.m_sourceDesc.IsValidLayer(sourceLayer))
        throw CustomException(L"Invalid command data");

    const LayerDesc& layerDesc = m_impl.m_sourceDesc.GetLayer(sourceLayer);

    for (LayerDesc::TypeCIterator typeIt = layerDesc.TypesBegin(), typesEnd = layerDesc.TypesEnd(); typeIt != typesEnd; ++typeIt)
        {
        ImportLayerTypeToLayerType(layerDesc.GetID(), *typeIt, targetLayer, targetType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerToType  (UInt                    sourceLayer,
                                                const DataTypeFamily&   targetType) const
    {
    const UInt targetLayer = m_config.HasDefaultTargetLayer() ? m_config.GetDefaultTargetLayer() : sourceLayer;
    ImportLayerToLayerType(sourceLayer, targetLayer, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerType    (UInt                    sourceLayer,
                                                const DataTypeFamily&   sourceType) const
    {
    const UInt targetLayer = m_config.HasDefaultTargetLayer() ? m_config.GetDefaultTargetLayer() : sourceLayer;
    const DataTypeFamily& targetType = m_config.HasDefaultTargetType() ? m_config.GetDefaultTargetType() : sourceType;

    ImportLayerTypeToLayerType(sourceLayer, sourceType, 
                               targetLayer, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerTypeToLayer (UInt                    sourceLayer,
                                                    const DataTypeFamily&   sourceType,
                                                    UInt                    targetLayer) const
    {
    const DataTypeFamily& targetType = m_config.HasDefaultTargetType() ? m_config.GetDefaultTargetType() : sourceType;
    ImportLayerTypeToLayerType(sourceLayer, sourceType, 
                               targetLayer, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerTypeToLayerType     (UInt                    sourceLayer,
                                                            const DataTypeFamily&   sourceType,
                                                            UInt                    targetLayer,
                                                            const DataTypeFamily&   targetType) const
    {
    m_impl.Import(sourceLayer, sourceType, 
                  targetLayer, targetType, 
                  m_config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerTypeToLayerType (UInt                    sourceLayer,
                                                        const DataType&         sourceType,
                                                        UInt                    targetLayer,
                                                        const DataTypeFamily&   targetType) const
    {
    m_impl.Import(sourceLayer, sourceType, 
                  targetLayer, targetType, 
                  m_config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportLayerTypeToType  (UInt                    sourceLayer,
                                                    const DataTypeFamily&   sourceType,
                                                    const DataTypeFamily&   targetType) const
    {
    const UInt targetLayer = m_config.HasDefaultTargetLayer() ? m_config.GetDefaultTargetLayer() : sourceLayer;
    ImportLayerTypeToLayerType(sourceLayer, sourceType, 
                               targetLayer, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportType  (const DataTypeFamily&   sourceType) const
    {
    const DataTypeFamily& targetType = m_config.HasDefaultTargetType() ? m_config.GetDefaultTargetType() : sourceType;
    ImportTypeToType(sourceType, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void CommandVisitor::ImportTypeToLayer  (const DataTypeFamily&   sourceType,
                                                UInt                    targetLayer) const
    {
    const DataTypeFamily& targetType = m_config.HasDefaultTargetType() ? m_config.GetDefaultTargetType() : sourceType;
    ImportTypeToLayerType(sourceType, targetLayer, targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::ImportTypeToLayerType     (const DataTypeFamily&   sourceType,
                                                UInt                    targetLayer,
                                                const DataTypeFamily&   targetType) const
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayerTypeToLayerType(layerIt->GetID(), sourceType, targetLayer, targetType);
        }  
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::ImportTypeToType  (const DataTypeFamily&   sourceType,
                                        const DataTypeFamily&   targetType) const
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayerTypeToType(layerIt->GetID(), sourceType, targetType);
        } 
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportAllCommand&             command)
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayer(layerIt->GetID());
        }  
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportAllToLayerCommand&      command)
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayerToLayer(layerIt->GetID(), command.GetTargetLayer());
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportAllToLayerTypeCommand&  command)
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayerToLayerType(layerIt->GetID(), command.GetTargetLayer(), command.GetTargetType());
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportAllToTypeCommand&       command)
    {
    for (ContentDesc::LayerCIter layerIt = m_impl.m_sourceDesc.LayersBegin(), layersEnd = m_impl.m_sourceDesc.LayersEnd();
         layerIt != layersEnd;
         ++layerIt)
        {
        ImportLayerToType(layerIt->GetID(), command.GetTargetType());
        }   
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerCommand&                   command)
    {
    ImportLayer(command.GetSourceLayer());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerToLayerCommand&            command)
    {
    ImportLayerToLayer(command.GetSourceLayer(), command.GetTargetLayer());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerToLayerTypeCommand&        command)
    {
    ImportLayerToLayerType(command.GetSourceLayer(), command.GetTargetLayer(), command.GetTargetType());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerToTypeCommand&             command)
    {
    ImportLayerToType(command.GetSourceLayer(), command.GetTargetType());
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerTypeCommand&               command)
    {
    ImportLayerType(command.GetSourceLayer(), command.GetSourceType());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerTypeToLayerCommand&        command)
    {
    ImportLayerTypeToLayer(command.GetSourceLayer(), command.GetSourceType(), command.GetTargetLayer());
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerTypeToLayerTypeCommand&    command)
    {
    ImportLayerTypeToLayerType(command.GetSourceLayer(), command.GetSourceType(), 
                               command.GetTargetLayer(), command.GetTargetType());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportLayerTypeToTypeCommand&         command)
    {
    ImportLayerTypeToType(command.GetSourceLayer(), command.GetSourceType(), command.GetTargetType());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportTypeCommand&                    command)
    {
    ImportType(command.GetSourceType());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportTypeToLayerCommand&             command)
    {
    ImportTypeToLayer(command.GetSourceType(), command.GetTargetLayer());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportTypeToLayerTypeCommand&         command)
    {
    ImportTypeToLayerType(command.GetSourceType(), command.GetTargetLayer(), command.GetTargetType());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CommandVisitor::_Visit (const ImportTypeToTypeCommand&              command)
    {
    ImportTypeToType(command.GetSourceType(), command.GetTargetType());
    }




} // END namespace Internal
END_BENTLEY_MRDTM_IMPORT_NAMESPACE
