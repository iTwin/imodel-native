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
struct GltfReader
{
    StreamBufferR       m_buffer;
    DgnModelR           m_model;
    Json::Value         m_batchData;
    Json::Value         m_materialValues;
    Json::Value         m_accessors;     
    Json::Value         m_bufferViews;
    uint8_t const*      m_binaryData;
    Render::System&     m_renderSystem;

    GltfReader(StreamBufferR buffer, DgnModelR model, Render::System& renderSystem) : m_buffer(buffer), m_model(model), m_renderSystem(renderSystem) { }

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

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetBufferView (void const*& pData, size_t& count, size_t& byteLength, uint32_t& type, Json::Value& accessor, Json::Value const& primitiveValue, Utf8CP accessorName)
    {
    Json::Value     bufferView;

    if(SUCCESS != GetAccessorAndBufferView(accessor, bufferView, primitiveValue, accessorName))
        return ERROR;
    
    byteLength  = bufferView["byteLength"].asUInt();
    count       = accessor["count"].asUInt();
    pData = m_binaryData + bufferView["byteOffset"].asUInt() + accessor["byteOffset"].asUInt();
    type = accessor["componentType"].asUInt();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ReadIndices(bvector<uint32_t>& indices, Json::Value const& primitiveValue, Utf8CP accessorName)
    {
    void const*     pData;
    size_t          indicesCount, indicesByteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, indicesCount, indicesByteLength, type, accessor, primitiveValue, accessorName))
        return ERROR;

    switch(type)
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

            memcpy(indices.data(), pData, indicesByteLength);
            break;
            }
        default:
            BeAssert(false && "unrecognized index type");
            return ERROR;

        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadMeshIndices(MeshR mesh, Json::Value const& primitiveValue)
    {
    bvector<uint32_t>   indices;

    if (SUCCESS != ReadIndices (indices, primitiveValue, "indices"))
        {
        BeAssert(false && "indices read error");
        return ERROR;
        }

    size_t      triangleVertexCount = 3*(indices.size()/3);

    for(size_t i=0; i < triangleVertexCount; i+= 3)
        mesh.AddTriangle(Triangle(indices[i], indices[i+1], indices[i+2], false));


    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* accessorName)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue, accessorName))
        return ERROR;

    size_t   nValues             = count * nComponents;



    switch (type)
        {
        case GLTF_UNSIGNED_SHORT:
            {
            if(nValues * sizeof(uint16_t) != byteLength)    
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
            if(nValues * sizeof(float) != byteLength)    
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
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "BATCHID") ||
        type != GLTF_UNSIGNED_SHORT)
        return ERROR;

    if (byteLength != count * sizeof(uint16_t))
        {
        BeAssert(false && "Invalid batch id size");
        return ERROR;
        }

    batchIds.resize (count);
    memcpy (batchIds.data(), pData, byteLength);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue)
    {
    BeAssert (false && "WIP - Create DisplayParams from GLTF Material");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadVertices(QVertex3dListR vertexList, Json::Value const& primitiveValue)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "POSITION"))
        return ERROR;

    size_t              nValues             = count * 3;

    switch (accessor["componentType"].asInt())
        {
        case GLTF_UNSIGNED_SHORT:
            {
            Json::Value     extensions, quantized, min, max;

            if(nValues * sizeof(uint16_t) != byteLength)    
                {
                BeAssert(false && "Error reading vertex attribute");
                return ERROR;
                }

            if ((extensions = accessor["extensions"]).isNull() ||
                (quantized  = extensions["WEB3D_quantized_attributes"]).isNull() ||
                !(min = quantized["decodedMin"]).isArray() ||
                !(max = quantized["decodedMax"]).isArray())
                {
                BeAssert(false);
                return ERROR;
                }

            vertexList.Init(DRange3d::From(min[0].asDouble(), min[1].asDouble(), min[2].asDouble(), max[0].asDouble(), max[1].asDouble(), max[2].asDouble()), reinterpret_cast <QPoint3d const*>(pData), count); 
            return SUCCESS;
            }

        default:
            BeAssert(false && "Unsupported component type");
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadNormals(OctEncodedNormalListR normals, Json::Value const& value, Utf8CP accessorName)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;

    switch (accessor["componentType"].asInt())
        {
        case GLTF_UNSIGNED_BYTE:
                {
                normals.resize(count);
                memcpy (normals.data(), pData, count * sizeof(OctEncodedNormal));
                return SUCCESS;
                }

        default:
            BeAssert(false && "Unsupported component type");
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadNormalPairs(OctEncodedNormalPairListR pairs, Json::Value const& value, Utf8CP accessorName)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView(pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;


    switch (accessor["componentType"].asInt())
        {
        case GLTF_UNSIGNED_BYTE:
            {
            pairs.resize(count);
            memcpy(pairs.data(), pData, count * sizeof(OctEncodedNormalPair));
            return SUCCESS;
            }
        default:
            BeAssert(false && "Unsupported component type");
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadParams(bvector<FPoint2d>& params, Json::Value const& value, Utf8CP accessorName)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;

    switch (accessor["componentType"].asInt())
        {
        case GLTF_FLOAT:
            {
            BeAssert (byteLength == count * sizeof(FPoint2d));
            params.resize(count);
            memcpy (params.data(), pData, count * sizeof(FPoint2d));
            return SUCCESS;
            }

        default:
            BeAssert(false && "Unsupported component type");
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ReadColors(bvector<uint16_t>& colors, Json::Value const& primitiveValue)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "_COLORINDEX"))
        return;

    switch (type)
        {
        case GLTF_UNSIGNED_SHORT:
            {
            colors.resize(count);
            memcpy (colors.data(), pData, count * sizeof(uint16_t));
            break;
            }

        case GLTF_UNSIGNED_BYTE:
            {
            uint8_t const*  pByteData = static_cast<uint8_t const*> (pData);
            colors.resize(count);
            for (size_t i=0; i<count; i++)
                colors[i] = *pByteData++;

            break;
            }

        default:
            BeAssert(false && "Unsupported component type");
            break;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadColorTable(ColorTableR colorTable, Json::Value const& primitiveValue)
    {
    Json::Value colorTableJson = primitiveValue["colorTable"];

    if (!colorTableJson.isArray())
        return ERROR;

    for (uint32_t i=0; i<colorTableJson.size(); i++)
        colorTable.GetIndex(colorTableJson[i].asUInt());                                            
        
    return colorTable.size() > 0 ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    CopyAndIncrement(void* out, void const*& in, size_t size)
    {
    memcpy (out, in, size);
    in = (uint8_t const*) in + size;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReadPolylines(bvector<MeshPolyline>& polylines, Json::Value value, Utf8CP name)
    {
    void const*     pData;
    size_t          count, byteLength;
    uint32_t        type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, name))
        return ERROR;

    for (size_t i=0; i<count; i++)
        {
        uint32_t                nIndices = 0;
        bvector<uint32_t>       indices;
        float                   startDistance;
        FPoint3d                rangeCenter;

        CopyAndIncrement(&startDistance, pData, sizeof(startDistance));
        CopyAndIncrement(&rangeCenter, pData, sizeof(rangeCenter));
        CopyAndIncrement(&nIndices, pData, sizeof(nIndices));

        indices.resize(nIndices);
        if (GLTF_UNSIGNED_SHORT == type)
            {
            for (size_t j=0; j<nIndices; j++)
                {
                uint16_t    index;

                CopyAndIncrement(&index, pData, sizeof(index));
                indices[j] = index;
                }
            }
        else
            {
            CopyAndIncrement(indices.data(), pData, nIndices * sizeof(uint32_t));
            }
        if (indices.size() < 2)
            {
            BeAssert(false);
            continue;
            }

        polylines.push_back(MeshPolyline(startDistance, rangeCenter, std::move(indices)));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesPtr ReadMeshEdges(Json::Value const& primitiveValue)
    {
    Json::Value   edgesValue = primitiveValue["edges"];

    if (edgesValue.isNull())
        return nullptr;

    MeshEdgesPtr        meshEdges = new MeshEdges();
    bvector<uint32_t>   indices;
    Json::Value const&  silhouettesValue = edgesValue["silhouettes"];

    if (!silhouettesValue.isNull() &&
        SUCCESS == ReadNormalPairs(meshEdges->m_silhouetteNormals, silhouettesValue, "normalPairs") &&
        SUCCESS == ReadIndices(indices, silhouettesValue, "indices"))
        {
        meshEdges->m_silhouette.resize(indices.size()/2);
        memcpy (meshEdges->m_silhouette.data(), indices.data(), indices.size() * sizeof(uint32_t));
        }
   
    ReadPolylines(meshEdges->m_polylines, edgesValue, "polylines");

    return meshEdges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     ReadFeatures(MeshR mesh, Json::Value const& primitiveValue)
    {
    bvector <uint32_t>  indices;
    if (primitiveValue.isMember("featureID"))
        {
        indices.push_back(primitiveValue["featureID"].asUInt());
        }
    else
        {
        if (SUCCESS != ReadIndices(indices, primitiveValue, "featureIDs") || indices.size() != mesh.Points().size())
            {
            BeAssert(false && "Missing feature IDs");
            return ERROR;
            }
        }
    mesh.SetFeatureIndices(std::move(indices));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
MeshPtr ReadMeshPrimitive(Json::Value const& primitiveValue, FeatureTableP featureTable)
    {
    Json::Value     materialName = primitiveValue["material"], materialValue;
        
    if (!materialName.isString() ||
        !(materialValue = m_materialValues[materialName.asString()]).isObject())
        {
        BeAssert(false && "Material not found");
        return nullptr;
        }

    DisplayParamsCPtr    displayParams = _CreateDisplayParams(materialValue);
    if (displayParams.IsNull())
        return nullptr;

    Mesh::PrimitiveType     primitiveType = (Mesh::PrimitiveType) primitiveValue["type"].asUInt();
    bool                    isPlanar = primitiveValue["isPlanar"].asBool();
    MeshPtr                 mesh = Mesh::Create(*displayParams, featureTable, primitiveType, DRange3d::NullRange(), !m_model.Is3d(), isPlanar);
    MeshEdgesPtr            meshEdges;

    if(SUCCESS != ReadVertices(mesh->VertsR(), primitiveValue))
        return nullptr;

    ReadColorTable(mesh->GetColorTableR(), primitiveValue);
    ReadColors(mesh->ColorsR(), primitiveValue);
    ReadFeatures(*mesh, primitiveValue);

    switch (primitiveType)
        {
        case Mesh::PrimitiveType::Mesh:
            {
            if (SUCCESS != ReadMeshIndices(*mesh, primitiveValue))
                return nullptr;

            if (!displayParams->IgnoresLighting() &&
                SUCCESS != ReadNormals(mesh->NormalsR(), primitiveValue["attributes"], "NORMAL"))
                return nullptr;
    
            ReadParams(mesh->ParamsR(), primitiveValue["attributes"], "TEXCOORD_0");
            mesh->GetEdgesR() = ReadMeshEdges(primitiveValue);
            break;
            }

        case Mesh::PrimitiveType::Polyline:
        case Mesh::PrimitiveType::Point:
            {
            if (SUCCESS != ReadPolylines(mesh->PolylinesR(), primitiveValue, "indices"))
                {
                BeAssert(false);
                return nullptr;
                }
            break;
            }
        }


    // TBD... Batch ids.
    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadGltf(Render::Primitives::GeometryCollectionR geometryCollection)
    {
    char            gltfMagic[4];
    uint32_t        gltfVersion, gltfLength, sceneStrLength, gltfSceneFormat;
    Json::Reader    reader;          
    ByteCP          startPosition = m_buffer.GetCurrent();

    if(!m_buffer.ReadBytes(gltfMagic, 4) ||
        0 != memcmp(gltfMagic, s_gltfMagic, 4) ||
        !m_buffer.Read(gltfVersion) ||
        gltfVersion != s_gltfVersion ||
        !m_buffer.Read(gltfLength) ||
        !m_buffer.Read(sceneStrLength) ||
        !m_buffer.Read(gltfSceneFormat) ||
        gltfSceneFormat != s_gltfSceneFormat)
        return TileIO::ReadStatus::ReadError;

    m_binaryData = startPosition + gltfLength;

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
            {
            MeshPtr     mesh;
            
            if ((mesh = ReadMeshPrimitive(primitive, &geometryCollection.Meshes().FeatureTable())).IsValid())
                geometryCollection.Meshes().push_back(mesh);
            }
        }

    return TileIO::ReadStatus::Success; 
    }

};  // GltfReader

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct BatchedModelReader : GltfReader
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadTile(Render::Primitives::GeometryCollectionR geometry)
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

    return ReadGltf (geometry);
    }
};  // BatchedModelReader

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct DgnCacheTileReader : GltfReader
{
    DgnCacheTileReader(StreamBufferR buffer, DgnModelR model, Render::System& renderSystem) : GltfReader(buffer, model, renderSystem) { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue) override
    {
    // ###TODO: Use a DisplayParamsCache?
    GradientSymbPtr     gradient;

    if (materialValue.isMember("gradient"))
        {
        gradient = GradientSymb::Create();

        if (SUCCESS != gradient->FromJson(materialValue["gradient"]))
            gradient = nullptr;
        }

    return  DisplayParams::Create((DisplayParams::Type) materialValue["type"].asUInt(),
                                  DgnCategoryId(materialValue["categoryId"].asUInt64()),
                                  DgnSubCategoryId(materialValue["subCategoryId"].asUInt64()),
                                  gradient.get(),
                                  DgnMaterialId(materialValue["materialId"].asUInt64()),
                                  ColorDef(materialValue["lineColor"].asUInt()),
                                  ColorDef(materialValue["fillColor"].asUInt()),
                                  materialValue["lineWidth"].asUInt(),
                                  (LinePixels) materialValue["linePixels"].asUInt(),
                                  (FillFlags) materialValue["fillFlags"].asUInt(),
                                  (DgnGeometryClass) materialValue["class"].asUInt(),
                                  materialValue["ignoreLighting"].asBool(),
                                  m_model.GetDgnDb(), m_renderSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadFeatureTable(FeatureTableR featureTable)
    {
    uint32_t    featureTableLength, maxFeatures, featureCount;
    uint32_t    startPos = m_buffer.GetPos();

    if (!m_buffer.Read(featureTableLength) ||
        !m_buffer.Read(maxFeatures) ||
        !m_buffer.Read(featureCount))
        return TileIO::ReadStatus::ReadError;

    featureTable.SetMaxFeatures(maxFeatures);

    for (size_t i=0;i<featureCount; i++)
        {
        uint64_t    elementId = 0;
        uint64_t    subCategoryId = 0;
        uint32_t    geometryClass = 0;
        uint32_t    index = 0;

        if (!m_buffer.Read(elementId) ||
            !m_buffer.Read(subCategoryId) ||
            !m_buffer.Read(geometryClass) ||
            !m_buffer.Read(index))
            {
            BeAssert(false);
            return TileIO::ReadStatus::FeatureTableError;;
            }

        featureTable.Insert(Feature(DgnElementId(elementId), DgnSubCategoryId(subCategoryId), static_cast<DgnGeometryClass>(geometryClass)), index);
        }
    
    m_buffer.SetPos(startPos + featureTableLength);

    return TileIO::ReadStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, bool& isLeaf)
    {
    char                dgnTileMagic[4];
    uint32_t            dgnTileLength, dgnTileVersion, flags;
    TileIO::ReadStatus  status;

    if (! m_buffer.ReadBytes(dgnTileMagic, 4) ||  
        0 != memcmp(dgnTileMagic, s_dgnTileMagic, 4) ||
        !m_buffer.Read(dgnTileVersion) ||  
        dgnTileVersion != s_dgnTileVersion ||
        !m_buffer.Read(flags) ||
        !m_buffer.Read(contentRange) || 
        !m_buffer.Read(dgnTileLength))
        return TileIO::ReadStatus::InvalidHeader;
    
    if (TileIO::ReadStatus::Success != (status = ReadFeatureTable(geometry.Meshes().FeatureTable())))
        return status;

    if (0 != (flags & TileIO::Flags::ContainsCurves))
        geometry.MarkCurved();

    if (0 != (flags & TileIO::Flags::Incomplete))
        geometry.MarkIncomplete();

    isLeaf = 0 != (flags & TileIO::Flags::IsLeaf);

    return ReadGltf (geometry);
    }
};  // BatchedModelReader

END_TILEREADER_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus TileIO::ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, DgnModelR model, Render::System& renderSystem, bool& isLeaf)
    {
    return TileReader::DgnCacheTileReader(streamBuffer, model, renderSystem).ReadTile(contentRange, geometry, isLeaf);
    }
