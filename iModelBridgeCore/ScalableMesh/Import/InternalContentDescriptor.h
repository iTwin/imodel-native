/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/InternalContentDescriptor.h $
|    $RCSfile: InternalContentDescriptor.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/05 00:12:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

#include <ScalableMesh/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ContentDescriptor;
struct ILayerDescriptor;
struct ExtractionConfig;

namespace Internal {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerDesc
    {
    friend class                    ContentDesc;

    uint32_t                            m_id;
    GCS                             m_gcs;
    const DRange3d*                 m_extentP;

    typedef bvector<DataType>       TypeList;
    bvector<DataType>               m_types;

    void                            SetID                      (uint32_t                        id) { m_id = id; }

public:
    typedef const DataType*         TypeCIterator;

                                    LayerDesc                  (const ILayerDescriptor&      layerDesc);

    explicit                        LayerDesc                  (const ILayerDescriptor&      layerDesc,
                                                                uint32_t                        layerID);

    uint32_t                            GetID                      () const { return m_id; }

    TypeCIterator                   TypesBegin                 () const { return &*m_types.begin(); }
    TypeCIterator                   TypesEnd                   () const { return &*m_types.end(); }

    const GCS&                      GetGCS                     () const { return m_gcs; }
    const DRange3d*                 GetExtentP                 () const { return m_extentP; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ContentDesc
    {
    typedef bvector<LayerDesc>      LayerList;
    LayerList                       m_layers;
public:
    typedef LayerList::const_iterator
                                    LayerCIter;

    explicit                        ContentDesc                (const ContentDescriptor&    contentDesc);


    bool                            IsValidLayer               (uint32_t                        layerId) const;

    uint32_t                            GetLayerCount              () { return (uint32_t) m_layers.size(); }

    const LayerDesc&                GetLayer                   (uint32_t                        layerId) const;
    LayerDesc&                      GetLayer                   (uint32_t                        layerId);


    LayerCIter                      LayersBegin                () const { return m_layers.begin(); }
    LayerCIter                      LayersEnd                  () const { return m_layers.end(); }

    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool ContentDesc::IsValidLayer (uint32_t layerId) const
    {
    return layerId < m_layers.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline const LayerDesc& ContentDesc::GetLayer (uint32_t layerId) const
    {
    assert(layerId < m_layers.size());
    return m_layers[layerId];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline LayerDesc& ContentDesc::GetLayer (uint32_t layerId)
    {
    assert(layerId < m_layers.size());
    return m_layers[layerId];
    }




} // END namespace Internal
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
