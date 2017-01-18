/*--------------------------------------------------------------------------------------+                                                                                                                                      
|
|     $Source: TilePublisher/lib/TileReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include "Constants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    TileReader::GetAccessorAndBufferView(Json::Value& accessor, Json::Value& bufferView,  Json::Value const& rootValue, const char* accessorName)
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
TileDisplayParamsPtr TileReader::ReadDisplayParams(Json::Value const& primitiveValue)
    {
    auto&           materialName = primitiveValue["material"];
    Json::Value     materialValue, values, colorValue, materialIdValue;
    uint32_t        color = 0;
    BeInt64Id       id;
    DgnMaterialId   materialId;

    if(!materialName.isString() ||
        !(materialValue = m_materialValues[materialName.asCString()]).isObject() ||
       (values = materialValue["values"]).isNull())
        {
        BeAssert(false && "material not found");
        return nullptr;
        }

    if((materialIdValue = values["materialId"]).isString() &&
       SUCCESS == BeInt64Id::FromString  (id, materialIdValue.asCString()))
       materialId = DgnMaterialId(id.GetValue());

    if((colorValue = values["color"]).isArray())
        {
        RgbFactor       rgbFactor;
        double          alpha = colorValue[3].asDouble();

        rgbFactor.red   = colorValue[0].asDouble();
        rgbFactor.green = colorValue[1].asDouble();
        rgbFactor.blue  = colorValue[2].asDouble();
        
        ColorDef    colorDef = ColorUtil::FromRgbFactor(rgbFactor);
        colorDef.SetAlpha((Byte) (1.0 - std::min(1.0, alpha)) * 255.0);

        color = colorDef.GetValue();
        }
    return TileDisplayParams::Create(color, materialId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileReader::ReadIndices(TileMeshR mesh, Json::Value const& primitiveValue)
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
    void const*         pData =(m_binaryData.GetData() + bufferByteOffset + accessorByteOffset);

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
BentleyStatus TileReader::ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* attributeName)
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
    void const*         pData =(m_binaryData.GetData() + bufferByteOffset + accessorByteOffset);

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
BentleyStatus TileReader::ReadVertexBatchIds (bvector<uint16_t>& batchIds, Json::Value const& primitiveValue)
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
    void const*         pData =(m_binaryData.GetData() + bufferByteOffset + accessorByteOffset);

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
TileMeshPtr  TileReader::ReadMeshPrimitive(Json::Value const& primitiveValue)
    {
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

    return mesh;
    }

TileReader::~TileReader() { if(m_file) std::fclose(m_file); m_file = nullptr; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileReader::Status  TileReader::ReadTile(TileMeshList& meshes, BeFileNameCR fileName)
    {
    if(nullptr ==(m_file = _wfopen(fileName.c_str(), L"rb")))
        return Status::UnableToOpenFile;

    char            b3dmMagic[4];
    uint32_t        b3dmLength, b3dmVersion, batchTableStrLen, batchTableBinaryLen, b3dmNumBatches; 

    if(1 != std::fread(&b3dmMagic, 4, 1, m_file) ||
        0 != memcmp(b3dmMagic, s_b3dmMagic, 4) ||
        1 != std::fread(&b3dmVersion, sizeof(b3dmVersion), 1, m_file) ||
        b3dmVersion != s_b3dmVersion ||
        1 != std::fread(&b3dmLength, sizeof(b3dmLength), 1, m_file) ||
        1 != std::fread(&batchTableStrLen, sizeof(batchTableStrLen), 1, m_file) ||
        1 != std::fread(&batchTableBinaryLen, sizeof(batchTableBinaryLen), 1, m_file) ||
        1 != std::fread(&b3dmNumBatches, sizeof(b3dmNumBatches), 1, m_file))
        return Status::InvalidHeader;

    bvector<char>       batchTableData(batchTableStrLen);
    Json::Value         batchTableValue;
    Json::Reader        reader;                                                                                                                        

    
    if(1 != fread(batchTableData.data(), batchTableStrLen, 1, m_file))
        return Status::ReadError;
    
    if(! reader.parse(batchTableData.data(), batchTableData.data() + batchTableStrLen, m_batchData))
        return Status::BatchTableParseError;

    char            gltfMagic[4];
    uint32_t        gltfVersion, gltfLength, sceneStrLength, gltfSceneFormat;

    if(1 != std::fread(&gltfMagic, 4, 1, m_file) ||
        0 != memcmp(gltfMagic, s_gltfMagic, 4) ||
        1 != std::fread(&gltfVersion, sizeof(gltfVersion), 1, m_file) ||
        gltfVersion != s_gltfVersion ||
        1 != std::fread(&gltfLength, sizeof(gltfLength), 1, m_file) ||
        1 != std::fread(&sceneStrLength, sizeof(sceneStrLength), 1, m_file) ||
        1 != std::fread(&gltfSceneFormat, sizeof(gltfSceneFormat), 1, m_file) ||
        gltfSceneFormat != s_gltfSceneFormat)
        return Status::ReadError;

    bvector<char>       sceneStrData(sceneStrLength);
    Json::Value         sceneValue;
    
    if(1 != fread(sceneStrData.data(), sceneStrLength, 1, m_file))
        return Status::ReadError;

    if(!reader.parse(sceneStrData.data(), sceneStrData.data() + sceneStrLength, sceneValue))
        return Status::SceneParseError;

    size_t              binaryDataSize = gltfLength - sceneStrLength - 5 * sizeof(uint32_t);
    m_binaryData.resize(binaryDataSize);

    if(1 != fread(m_binaryData.data(), binaryDataSize, 1, m_file))
        return Status::ReadError;

    Json::Value         meshValues     = sceneValue["meshes"];

    m_materialValues = sceneValue["materials"];
    m_accessors      = sceneValue["accessors"];
    m_bufferViews    = sceneValue["bufferViews"];

    if(!meshValues.isObject() || 
        !m_materialValues.isObject() ||
        !m_accessors.isObject() ||
        !m_bufferViews.isObject())
        return Status::SceneDataError;

    for(auto& mesh : meshValues)
        {
        auto& primitives = mesh["primitives"];

        if(primitives.isNull())
            {
            BeAssert(false);
            continue;
            }
        for(auto& primitive : primitives)
            {
            TileMeshPtr    mesh = ReadMeshPrimitive(primitive);

            if(mesh.IsValid())
                meshes.push_back(mesh);
            }
        }

    return Status::Success; 
    }
#ifdef  WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileReader::Status  TileReader::ReadTileSet(Json::Value& tileSet, BeFileNameCR fileName)
    {
    std::filebuf fb;
    if (nullptr == fb.open (Utf8String(fileName.c_str()).c_str, std::ios::in))
        return Status::UnableToOpenFile;


    Json::Reader        reader;                                                                                                                        
    std::istream        is(&fb);

    return reader.parse (&is, tileSet) ? Status::Success : Status::TileSetParseError;
    }


#endif
