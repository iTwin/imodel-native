/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/InternalContentDescriptor.cpp $
|    $RCSfile: InternalContentDescriptor.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/05 00:12:41 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "InternalContentDescriptor.h"

#include <ScalableTerrainModel/Import/Config/Content/All.h>
#include <ScalableTerrainModel/Import/ContentDescriptor.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
namespace Internal {



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDesc::LayerDesc (const LayerDescriptor&      layerDesc)
    :   m_id(0),
        m_gcs(layerDesc.GetGCS()),
        m_extentP(layerDesc.HasExtent() ? &layerDesc.GetExtent() : 0),
        m_types(layerDesc.TypesBegin(), layerDesc.TypesEnd())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDesc::LayerDesc (const LayerDescriptor&      layerDesc,
                      UInt                        layerID)
    :   m_id(layerID),
        m_gcs(layerDesc.GetGCS()),
        m_extentP(layerDesc.HasExtent() ? &layerDesc.GetExtent() : 0),
        m_types(layerDesc.TypesBegin(), layerDesc.TypesEnd())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDesc::ContentDesc (const ContentDescriptor&  contentDesc)    
    :   m_layers(contentDesc.LayersBegin(), contentDesc.LayersEnd())
    {
    for (UInt layerID = 0; layerID < m_layers.size(); ++layerID)
        m_layers[layerID].SetID(layerID);
    }

} // END namespace Internal
END_BENTLEY_MRDTM_IMPORT_NAMESPACE
