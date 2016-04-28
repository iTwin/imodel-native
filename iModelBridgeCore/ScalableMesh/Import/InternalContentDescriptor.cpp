/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/InternalContentDescriptor.cpp $
|    $RCSfile: InternalContentDescriptor.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/05 00:12:41 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include "InternalContentDescriptor.h"

#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Import/ContentDescriptor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
namespace Internal {



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDesc::LayerDesc (const ILayerDescriptor&      layerDesc)
    :   m_id(0),
        m_gcs(layerDesc.GetGCS()),
        m_extentP(!layerDesc.GetExtent().IsNull() ? &layerDesc.GetExtent() : 0),
        m_types()
    {
    m_types.insert(m_types.end(), layerDesc.GetTypes().begin(), layerDesc.GetTypes().end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDesc::LayerDesc (const ILayerDescriptor&      layerDesc,
                      uint32_t                        layerID)
    :   m_id(layerID),
        m_gcs(layerDesc.GetGCS()),
        m_extentP(!layerDesc.GetExtent().IsNull() ? &layerDesc.GetExtent() : 0),
        m_types()
    {
    m_types.insert(m_types.end(), layerDesc.GetTypes().begin(), layerDesc.GetTypes().end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDesc::ContentDesc (const ContentDescriptor&  contentDesc)    
    :   m_layers()
    {
    for (auto it = contentDesc.LayersBegin(); it != contentDesc.LayersEnd(); it++)
        m_layers.push_back(LayerDesc(**it));
    for (uint32_t layerID = 0; layerID < m_layers.size(); ++layerID)
        m_layers[layerID].SetID(layerID);
    }

} // END namespace Internal
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
