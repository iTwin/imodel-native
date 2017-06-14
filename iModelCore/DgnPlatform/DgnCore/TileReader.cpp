/*--------------------------------------------------------------------------------------+                                                                                                                                      
|
|     $Source: DgnCore/TileReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>

#include <TilePublisher/Lib/Constants.h>

USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define BEGIN_TILEREADER_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace TileReader {
#define END_TILEREADER_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE

BEGIN_TILEREADER_NAMESPACE


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct Reader
{
    StreamBufferR       m_buffer;
    DgnModelR           m_model;
    Json::Value         m_batchData;
    Json::Value         m_materialValues;
    Json::Value         m_accessors;     
    Json::Value         m_bufferViews;
    uint8_t const*      m_binaryData;

    Reader(StreamBufferR buffer, DgnModelR model) : m_buffer(buffer), m_model(model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    GetAccessorAndBufferView(Json::Value& accessor, Json::Value& bufferView,  Json::Value const& rootValue, const char* accessorName)
    {
    Json::Value     accessorValue = rootValue[accessorName], bufferViewAccessorValue;

    if(!accessorValue.isString() ||
        !(accessor = m_accessors[accessorValue.asCString()]).isObject() ||
        !(bufferViewAccessorValue = accessor["bufferView"]).isString() ||
        !(bufferView = m_bufferViews[bufferViewAccessorValue.asCString()]).isObject())
        return ERROR;

    if(bufferView["buffer"].asString() != "binary_glTF")
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsPtr ReadDisplayParams(Json::Value const& primitiveValue)
    {
    BeAssert(false && "WIP");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadIndices(TileMeshR mesh, Json::Value const& primitiveValue)
    {
    Json::Value         accessor, bufferView;
    auto&               mode = primitiveValue["mode"];

    if(!mode.isInt())
        {
        BeAssert(false && "mesh data parse error");
        return ERROR;
        }

    // Indices....
    if(SUCCESS != GetAccessorAndBufferView(accessor, bufferView, primitiveValue, "indices") ||
        bufferView["target"].asInt() != GLTF_ELEMENT_ARRAY_BUFFER)
        return ERROR;
    

    size_t              indicesByteLength  = bufferView["byteLength"].asUInt(), 
                        indicesCount       = accessor["count"].asUInt(),
                        bufferByteOffset   = bufferView["byteOffset"].asUInt(),
                        accessorByteOffset = accessor["byteOffset"].asUInt();
    bvector<uint32_t>   indices;
    void const*         pData = m_binaryData + bufferByteOffset + accessorByteOffset;

    switch(accessor["componentType"].asInt())
        {
        case  GLTF_UNSIGNED_SHORT:
            {
            if(indicesCount * sizeof(uint16_t) != indicesByteLength)
                {
                BeAssert(false && "index count mismatch");
                return ERROR;
                }

            uint16_t const *pIndex = reinterpret_cast <uint16_t const*>(pData);
        
            for(auto const& pEnd = pIndex + indicesCount; pIndex < pEnd; )
                indices.push_back(*pIndex++);
            break;
            }

        case  GLTF_UINT32:
            {
            if(indicesCount * sizeof(uint32_t) != indicesByteLength)
                {
                BeAssert(false && "index count mismatch");
                return ERROR;
                }

            indices.resize(indicesCount);

            memcpy(indices.data(), pData, indicesCount);
            break;
            }
        default:
            BeAssert(false && "unrecognized index type");
            return ERROR;

        }

    switch(mode.asInt())
        {
        case GLTF_LINES:
            {
            size_t          lineVertexCount = 2*(indices.size()/2);
            TilePolyline    polyline;

            for(size_t i=0; i<lineVertexCount; i+= 2)
                {
                uint32_t    index = indices[i];

                if(!polyline.GetIndices().empty() && index != polyline.GetIndices().back())
                    {
                    mesh.AddPolyline(polyline);
                    polyline.Clear();
                    }
                else
                    {
                    polyline.AddIndex(indices[i]);
                    polyline.AddIndex(indices[i+1]);
                    }
                }
            break;
            }
        
        case GLTF_TRIANGLES:
            {
            size_t      triangleVertexCount = 3*(indices.size()/3);

            for(size_t i=0; i < triangleVertexCount; i+= 3)
                mesh.AddTriangle(TileTriangle(indices[i], indices[i+1], indices[i+2], false));

            break;
            }
    
        default:
            BeAssert(false && "invalid mesh mode");
            break;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* attributeName)
    {
    Json::Value         accessor, bufferView;

    if(!primitiveValue.isMember("attributes") ||
        SUCCESS != GetAccessorAndBufferView(accessor, bufferView, primitiveValue["attributes"], attributeName) ||
        !accessor.isMember("componentType") ||
        bufferView["target"].asInt() != GLTF_ARRAY_BUFFER)
        return ERROR;
        
    size_t              count               = accessor["count"].asUInt(),
                        dataSize            = bufferView["byteLength"].asUInt(),
                        bufferByteOffset    = bufferView["byteOffset"].asUInt(),
                        accessorByteOffset  = accessor["byteOffset"].asUInt(),
                        nValues             = count * nComponents;
    void const*         pData = m_binaryData + bufferByteOffset + accessorByteOffset;

    switch (accessor["componentType"].asInt())
        {
        case GLTF_UNSIGNED_SHORT:
            {
            if(nValues * sizeof(uint16_t) != dataSize)    
                {
                BeAssert(false && "Error reading vertex attribute");
                return ERROR;
                }
            uint16_t const*        pValue = reinterpret_cast <uint16_t const*>(pData);
            for(size_t i=0; i<nValues; i++)
                values.push_back((double) pValue[i]);

            Json::Value     extensions, quantized, decodedMinValue, decodedMaxValue;

            if(!(extensions = accessor["extensions"]).isNull() &&
                !(quantized  = extensions["WEB3D_quantized_attributes"]).isNull() &&
               (decodedMinValue = quantized["decodedMin"]).isArray() &&
               (decodedMaxValue = quantized["decodedMax"]).isArray())
                {
                for(uint32_t  j=0; j<nComponents; j++)
                    {
                    double  decodedMin = decodedMinValue[j].asDouble(), decodedMax = decodedMaxValue[j].asDouble();
                    double  scale =(decodedMax - decodedMin) /(double) 0xffff;
        
                    for(size_t i=0, k = 0; i<count; i++)
                        {
                        double& value = values[i * nComponents + j];

                        value = decodedMin + value * scale;
                        }
                    }
                }

            break;
            }

        case GLTF_FLOAT:
            {
            if(nValues * sizeof(float) != dataSize)    
                {
                BeAssert(false && "Error reading vertex attribute");
                return ERROR;
                }
            float const*        pValue = reinterpret_cast <float const*>(pData);
            for(size_t i=0; i<nValues; i++)
                values.push_back((double) pValue[i]);
            break;
            }

        default:
            BeAssert(false && "Unrecognized component type");
            return ERROR;
        }


    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadVertexBatchIds (bvector<uint16_t>& batchIds, Json::Value const& primitiveValue)
    {
    Json::Value         accessor, bufferView;

    if(!primitiveValue.isMember("attributes") ||
        SUCCESS != GetAccessorAndBufferView(accessor, bufferView, primitiveValue["attributes"], "BATCHID") ||
        bufferView["target"].asInt() != GLTF_ARRAY_BUFFER ||
        accessor["componentType"] != GLTF_UNSIGNED_SHORT)
        {
        return ERROR;
        }

    size_t              count               = accessor["count"].asUInt(),
                        dataSize            = bufferView["byteLength"].asUInt(),
                        bufferByteOffset    = bufferView["byteOffset"].asUInt(),
                        accessorByteOffset  = accessor["byteOffset"].asUInt();
    void const*         pData =(m_binaryData + bufferByteOffset + accessorByteOffset);

    if (dataSize != count * sizeof(uint16_t))
        {
        BeAssert(false && "Invalid batch id size");
        return ERROR;
        }

    batchIds.resize (count);
    memcpy (batchIds.data(), pData, dataSize);
    return SUCCESS;
    }

     


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void  ReadMeshPrimitive(Render::Primitives::GeometryCollectionR geometry, Json::Value const& primitiveValue)
    {
#ifdef WIP
    TileDisplayParamsPtr    displayParams;

    if(!(displayParams = ReadDisplayParams(primitiveValue)).IsValid())
        return nullptr;
        
    TileMeshPtr         mesh = TileMesh::Create(displayParams);
    bvector<uint16_t>   batchIds;

    if(SUCCESS != ReadIndices(*mesh, primitiveValue) ||
       SUCCESS != ReadVertexValues(mesh->PointsR(), primitiveValue, "POSITION"))
        return nullptr;

    ReadVertexValues(mesh->NormalsR(), primitiveValue, "NORMAL");
    ReadVertexValues(mesh->ParamsR(), primitiveValue, "TEXCOORD_0");
    ReadVertexBatchIds (batchIds, primitiveValue);

    if (batchIds.size() == mesh->Points().size() && m_batchData["element"].isArray())
        {
        mesh->SetValidIdsPresent(true);
        for (auto& batchId : batchIds)
            {
            DgnElementId       entityId;

            if (SUCCESS == DgnElementId::FromString  (entityId, m_batchData["element"][batchId].asCString()))
                mesh->EntityIdsR().push_back(entityId);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadBatchedModel(Render::Primitives::GeometryCollectionR geometry)
    {
    char            b3dmMagic[4];
    uint32_t        b3dmLength, b3dmVersion, batchTableStrLen, batchTableBinaryLen, b3dmNumBatches; 

    if (! m_buffer.ReadBytes(b3dmMagic, 4) ||  
        0 != memcmp(b3dmMagic, s_b3dmMagic, 4) ||
        !m_buffer.Read(b3dmVersion) ||  
        b3dmVersion != s_b3dmVersion ||
        !m_buffer.Read(b3dmLength) ||
        !m_buffer.Read(batchTableStrLen) ||
        !m_buffer.Read(batchTableBinaryLen) ||
        !m_buffer.Read(b3dmNumBatches))
        return TileIO::ReadStatus::InvalidHeader;

    bvector<char>       batchTableData(batchTableStrLen);
    Json::Value         batchTableValue;
    Json::Reader        reader;                                                                                                                        
    
    if(!m_buffer.ReadBytes(batchTableData.data(), batchTableStrLen))
        return TileIO::ReadStatus::ReadError;
    
    if(! reader.parse(batchTableData.data(), batchTableData.data() + batchTableStrLen, m_batchData))
        return TileIO::ReadStatus::BatchTableParseError;

    char            gltfMagic[4];
    uint32_t        gltfVersion, gltfLength, sceneStrLength, gltfSceneFormat;

    if(!m_buffer.ReadBytes(gltfMagic, 4) ||
        0 != memcmp(gltfMagic, s_gltfMagic, 4) ||
        !m_buffer.Read(gltfVersion) ||
        gltfVersion != s_gltfVersion ||
        !m_buffer.Read(gltfLength) ||
        !m_buffer.Read(sceneStrLength) ||
        !m_buffer.Read(gltfSceneFormat) ||
        gltfSceneFormat != s_gltfSceneFormat)
        return TileIO::ReadStatus::ReadError;

    m_binaryData = m_buffer.GetCurrent();

    bvector<char>       sceneStrData(sceneStrLength);
    Json::Value         sceneValue;
    
    if(! m_buffer.ReadBytes(sceneStrData.data(), sceneStrLength))
        return TileIO::ReadStatus::ReadError;

    if(!reader.parse(sceneStrData.data(), sceneStrData.data() + sceneStrLength, sceneValue))
        return TileIO::ReadStatus::SceneParseError;

    Json::Value         meshValues     = sceneValue["meshes"];

    m_materialValues = sceneValue["materials"];
    m_accessors      = sceneValue["accessors"];
    m_bufferViews    = sceneValue["bufferViews"];

    if(!meshValues.isObject() || 
        !m_materialValues.isObject() ||
        !m_accessors.isObject() ||
        !m_bufferViews.isObject())
        return TileIO::ReadStatus::SceneDataError;

    for(auto& mesh : meshValues)
        {
        auto& primitives = mesh["primitives"];

        if(primitives.isNull())
            {
            BeAssert(false);
            continue;
            }
        for(auto& primitive : primitives)
            ReadMeshPrimitive(geometry, primitive);
        }

    return TileIO::ReadStatus::Success; 
    }
};  // Reader

END_TILEREADER_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus TileIO::ReadTile(Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, DgnModelR model)
    {
    return TileReader::Reader(streamBuffer, model).ReadBatchedModel(geometry);
    }
