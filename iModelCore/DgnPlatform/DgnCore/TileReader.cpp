/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/TileReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>

#include "../TilePublisher/lib/Constants.h" // ###TODO: Move this stuff.

USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define BEGIN_TILEREADER_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace TileReader {
#define END_TILEREADER_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE

BEGIN_TILEREADER_NAMESPACE


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct BufferView
{
    void const* pData = nullptr;
    size_t      count;
    size_t      byteLength;
    uint32_t    type;
    Json::Value accessor;

    bool IsValid() const { return nullptr != pData; }
};

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

    void Increment(void const*& in, size_t size) { in = (uint8_t const*) in + size; }
    void CopyAndIncrement(void* out, void const*& in, size_t size)
        {
        memcpy(out, in, size);
        Increment(in, size);
        }

    virtual DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue)
        {
        BeAssert (false && "WIP - Create DisplayParams from GLTF Material");
        return nullptr;
        }

    BentleyStatus GetAccessorAndBufferView(Json::Value& accessor, Json::Value& bufferView, Json::Value const& rootValue, const char* accessorName);
    BentleyStatus GetBufferView (void const*& pData, size_t& count, size_t& byteLength, uint32_t& type, Json::Value& accessor, Json::Value const& primitiveValue, Utf8CP accessorName);
    BufferView GetBufferView(Json::Value const& json, Utf8CP accessorName);

    BentleyStatus ReadIndices(bvector<uint32_t>& indices, Json::Value const& primitiveValue, Utf8CP accessorName);
    BentleyStatus ReadMeshIndices(MeshR mesh, Json::Value const& primitiveValue);

    BentleyStatus ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* accessorName);
    BentleyStatus ReadVertexBatchIds (bvector<uint16_t>& batchIds, Json::Value const& primitiveValue);
    BentleyStatus ReadVertices(QVertex3dListR vertexList, Json::Value const& primitiveValue);

    BentleyStatus ReadNormalPairs(OctEncodedNormalPairListR pairs, Json::Value const& value, Utf8CP accessorName);
    BentleyStatus ReadNormals(OctEncodedNormalListR normals, Json::Value const& value, Utf8CP accessorName);

    BentleyStatus ReadParams(bvector<FPoint2d>& params, Json::Value const& value, Utf8CP accessorName);

    void ReadColors(bvector<uint16_t>& colors, Json::Value const& primitiveValue);
    BentleyStatus ReadColorTable(ColorTableR colorTable, Json::Value const& primitiveValue);

    BentleyStatus ReadPolylines(bvector<MeshPolyline>& polylines, Json::Value value, Utf8CP name, bool disjoint);
    MeshEdgesPtr ReadMeshEdges(Json::Value const& primitiveValue);

    BentleyStatus ReadFeatures(bvector<uint32_t>& featureIndices, Json::Value const& primitiveValue);
    BentleyStatus ReadFeatures(MeshR mesh, Json::Value const& primitiveValue);

    MeshPtr ReadMeshPrimitive(Json::Value const& primitiveValue, FeatureTableP featureTable);

    // Initializes the members m_binaryData, m_materialValues, m_accessors, and m_bufferViews; and returns the JSON representation of the meshes.
    TileIO::ReadStatus InitGltf(Json::Value& meshValues);

    // Reads gltf data into a GeometryCollection
    TileIO::ReadStatus ReadGltf(Render::Primitives::GeometryCollectionR geometryCollection);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    GltfReader::GetAccessorAndBufferView(Json::Value& accessor, Json::Value& bufferView,  Json::Value const& rootValue, const char* accessorName)
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
BentleyStatus GltfReader::GetBufferView (void const*& pData, size_t& count, size_t& byteLength, uint32_t& type, Json::Value& accessor, Json::Value const& primitiveValue, Utf8CP accessorName)
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
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BufferView GltfReader::GetBufferView(Json::Value const& json, Utf8CP accessorName)
    {
    BufferView view;
    auto stat = GetBufferView(view.pData, view.count, view.byteLength, view.type, view.accessor, json, accessorName);
    BeAssert((SUCCESS == stat) == (view.IsValid()));
    return view;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GltfReader::ReadIndices(bvector<uint32_t>& indices, Json::Value const& primitiveValue, Utf8CP accessorName)
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
BentleyStatus GltfReader::ReadMeshIndices(MeshR mesh, Json::Value const& primitiveValue)
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
BentleyStatus GltfReader::ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* accessorName)
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
        
                    for(size_t i=0; i<count; i++)
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
BentleyStatus GltfReader::ReadVertexBatchIds (bvector<uint16_t>& batchIds, Json::Value const& primitiveValue)
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
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::ReadVertices(QVertex3dListR vertexList, Json::Value const& primitiveValue)
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
BentleyStatus GltfReader::ReadNormals(OctEncodedNormalListR normals, Json::Value const& value, Utf8CP accessorName)
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
BentleyStatus GltfReader::ReadNormalPairs(OctEncodedNormalPairListR pairs, Json::Value const& value, Utf8CP accessorName)
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
BentleyStatus GltfReader::ReadParams(bvector<FPoint2d>& params, Json::Value const& value, Utf8CP accessorName)
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
void GltfReader::ReadColors(bvector<uint16_t>& colors, Json::Value const& primitiveValue)
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
BentleyStatus GltfReader::ReadColorTable(ColorTableR colorTable, Json::Value const& primitiveValue)
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
BentleyStatus GltfReader::ReadPolylines(bvector<MeshPolyline>& polylines, Json::Value value, Utf8CP name, bool disjoint)
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

        if (!disjoint && indices.size() < 2)
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
MeshEdgesPtr GltfReader::ReadMeshEdges(Json::Value const& primitiveValue)
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
   
    ReadPolylines(meshEdges->m_polylines, edgesValue, "polylines", false);

    return meshEdges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::ReadFeatures(bvector<uint32_t>& indices, Json::Value const& primitiveValue)
    {
    if (primitiveValue.isMember("featureID"))
        {
        indices.push_back(primitiveValue["featureID"].asUInt());
        return SUCCESS;
        }

    if (SUCCESS != ReadIndices(indices, primitiveValue, "featureIDs"))
        {
        BeAssert(false && "Missing feature IDs");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus     GltfReader::ReadFeatures(MeshR mesh, Json::Value const& primitiveValue)
    {
    bvector <uint32_t>  indices;
    if (SUCCESS != ReadFeatures(indices, primitiveValue) || (indices.size() > 1 && indices.size() != mesh.Points().size()))
        {
        BeAssert(false && "Missing feature IDs");
        return ERROR;
        }

    mesh.SetFeatureIndices(std::move(indices));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
MeshPtr GltfReader::ReadMeshPrimitive(Json::Value const& primitiveValue, FeatureTableP featureTable)
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
            if (SUCCESS != ReadPolylines(mesh->PolylinesR(), primitiveValue, "indices", Mesh::PrimitiveType::Point == primitiveType))
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
TileIO::ReadStatus  GltfReader::ReadGltf(Render::Primitives::GeometryCollectionR geometryCollection)
    {
    Json::Value meshValues;
    TileIO::ReadStatus status = InitGltf(meshValues);
    if (TileIO::ReadStatus::Success != status)
        return status;

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus GltfReader::InitGltf(Json::Value& meshValues)
    {
    char            gltfMagic[4];
    uint32_t        gltfVersion, gltfLength, sceneStrLength, gltfSceneFormat;
    Json::Reader    reader;          
    ByteCP          startPosition = m_buffer.GetCurrent();

    if(!m_buffer.ReadBytes(gltfMagic, 4) ||
        0 != memcmp(gltfMagic, s_gltfMagic, 4) ||
        !m_buffer.Read(gltfVersion) ||
        (gltfVersion != s_gltfVersion && gltfVersion != s_gltfVersion2) ||
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

    meshValues     = sceneValue["meshes"];

    m_materialValues = sceneValue["materials"];
    m_accessors      = sceneValue["accessors"];
    m_bufferViews    = sceneValue["bufferViews"];

    if(!meshValues.isObject() || 
        !m_materialValues.isObject() ||
        !m_accessors.isObject() ||
        !m_bufferViews.isObject())
        return TileIO::ReadStatus::SceneDataError;

    return TileIO::ReadStatus::Success;
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static DisplayParamsCPtr displayParamsFromJson(Json::Value const& materialValue, DgnDbR db, Render::System& system)
    {
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
                                  RenderMaterialId(materialValue["materialId"].asUInt64()),
                                  ColorDef(materialValue["lineColor"].asUInt()),
                                  ColorDef(materialValue["fillColor"].asUInt()),
                                  materialValue["lineWidth"].asUInt(),
                                  (LinePixels) materialValue["linePixels"].asUInt(),
                                  (FillFlags) materialValue["fillFlags"].asUInt(),
                                  (DgnGeometryClass) materialValue["class"].asUInt(),
                                  materialValue["ignoreLighting"].asBool(),
                                  db, system);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct DgnTileHeader
{
    char                magic[4];
    uint32_t            version;
    uint32_t            flags;
    ElementAlignedBox3d contentRange;
    uint32_t            length;

    bool Read(StreamBufferR buffer)
        {
        return buffer.ReadBytes(magic, 4) && 0 == memcmp(magic, s_dgnTileMagic, 4)
            && buffer.Read(version) && version == s_dgnTileVersion
            && buffer.Read(flags) && buffer.Read(contentRange) && buffer.Read(length);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct FeatureTableHeader
{
    uint32_t    length;
    uint32_t    maxFeatures;
    uint32_t    count;

    bool Read(StreamBufferR buffer)
        {
        return buffer.Read(length) && buffer.Read(maxFeatures) && buffer.Read(count);
        }
};

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
    return displayParamsFromJson(materialValue, m_model.GetDgnDb(), m_renderSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadFeatureTable(FeatureTableR featureTable)
    {
    uint32_t startPos = m_buffer.GetPos();

    FeatureTableHeader header;
    if (!header.Read(m_buffer))
        return TileIO::ReadStatus::ReadError;

    featureTable.SetMaxFeatures(header.maxFeatures);

    for (size_t i=0;i<header.count; i++)
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
    
    m_buffer.SetPos(startPos + header.length);

    return TileIO::ReadStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus  ReadTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, bool& isLeaf)
    {
    DgnTileHeader header;
    if (!header.Read(m_buffer))
        return TileIO::ReadStatus::InvalidHeader;

    TileIO::ReadStatus status = ReadFeatureTable(geometry.Meshes().FeatureTable());
    if (TileIO::ReadStatus::Success != status)
        return status;

    if (0 != (header.flags & TileIO::Flags::ContainsCurves))
        geometry.MarkCurved();

    if (0 != (header.flags & TileIO::Flags::Incomplete))
        geometry.MarkIncomplete();

    isLeaf = 0 != (header.flags & TileIO::Flags::IsLeaf);
    contentRange = header.contentRange;

    return ReadGltf (geometry);
    }
};  // DgnCacheTileReader

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct DgnCacheTileRebuilder : GltfReader
{
private:
    struct BufferView32
    {
        void const* m_data;
        bool        m_short;

        BufferView32() : m_data(nullptr) { }
        BufferView32(void const* data, bool isShort) : m_data(data), m_short(isShort) { }

        uint32_t operator[](size_t index) const
            {
            if (m_short)
                return (reinterpret_cast<uint16_t const*>(m_data))[index];
            else
                return reinterpret_cast<uint32_t const*>(m_data)[index];
            }

        bool IsValid() const { return nullptr != m_data; }
    };

    struct BufferView16
    {
        void const* m_data;
        bool        m_byte;

        BufferView16() : m_data(nullptr) { }
        BufferView16(void const* data, bool isByte) : m_data(data), m_byte(isByte) { }

        uint16_t operator[](size_t index) const
            {
            if (m_byte)
                return (reinterpret_cast<uint8_t const*>(m_data))[index];
            else
                return (reinterpret_cast<uint16_t const*>(m_data))[index];
            }

        bool IsValid() const { return nullptr != m_data; }
    };

    struct Features
    {
        BufferView32    m_nonUniform;
        uint32_t        m_uniform = 0;

        Features() = default;
        explicit Features(uint32_t uniform) : m_uniform(uniform) { }
        explicit Features(BufferView32 nonUniform) : m_nonUniform(nonUniform) { }

        bool IsValid() const { return 0 == m_uniform && !m_nonUniform.IsValid(); }
        bool IsUniform() const { BeAssert(IsValid()); return 0 != m_uniform; }
        bool IsNonUniform() const { BeAssert(IsValid()); return m_nonUniform.IsValid(); }

        uint32_t GetFeatureId(uint32_t index) const
            {
            BeAssert(IsValid());
            return IsUniform() ? m_uniform : m_nonUniform[index];
            }
    };

    struct FeatureList
    {
        struct Entry
        {
            Feature m_feature;
            bool    m_omit;

            Entry() { }
            Entry(FeatureCR feature, bool omit) : m_feature(feature), m_omit(omit) { }
        };

        bvector<Entry>  m_entries;

        TileIO::ReadStatus Read(StreamBufferR buffer, FeatureTableHeader const& header, DgnElementIdSet const& skipElems);
        bool RejectFeature(uint32_t index) const { BeAssert(index < m_entries.size()); return m_entries[index].m_omit; }

        // Returns nullptr if the feature is to be excluded
        FeatureCP GetFeature(uint32_t index) const
            {
            BeAssert(index < m_entries.size());
            auto const& entry = m_entries[index];
            return entry.m_omit ? nullptr : &entry.m_feature;
            }
    };

    struct PolylineHeader
    {
        float           m_startDistance;
        FPoint3d        m_rangeCenter;
        uint32_t        m_numIndices;
    };

    struct Polyline
    {
        PolylineHeader  m_header;
        BufferView32    m_indices;

        bool IsValid() const { return m_indices.IsValid(); }
    };

    struct MeshPrimitive
    {
        using Type = Render::Primitives::Mesh::PrimitiveType;

        uint32_t            m_numIndices;
        uint32_t            m_numVertices;
        DisplayParamsCPtr   m_displayParams;
        BufferView32        m_indices;
        QPoint3dCP          m_vertices;
        Features            m_features;
        uint16_t const*     m_normals;
        FPoint2d const*     m_params;
        bvector<uint32_t>   m_colorsByIndex;
        BufferView16        m_colorIndices;
        Type                m_type;
        bool                m_isPlanar;

        OctEncodedNormalCP GetNormal(size_t index) const { return nullptr != m_normals ? reinterpret_cast<OctEncodedNormalCP>(m_normals+index) : nullptr; }
        FPoint2d const* GetParam(size_t index) const { return nullptr != m_params ? m_params+index : nullptr; }
    };

    Render::Primitives::MeshBuilderMapR m_builders;
    FeatureList m_featureList;

    DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue) override
        {
        return displayParamsFromJson(materialValue, m_model.GetDgnDb(), m_renderSystem);
        }

    BufferView32 ReadBufferView32(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount=nullptr);
    BufferView16 ReadBufferView16(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount=nullptr);

    QPoint3d const* ReadVertices(uint32_t& numVertices, Json::Value const& json);
    BufferView16 ReadColorIndices(Json::Value const& json) { return ReadBufferView16(json["attributes"], "_COLORINDEX"); }
    Features ReadFeatures(Json::Value const& json);
    bvector<uint32_t> ReadColorIndex(Json::Value const& json);

    TileIO::ReadStatus ReadFeatureTable(DgnElementIdSet const& skipElems);
    TileIO::ReadStatus ReadGltf();
    TileIO::ReadStatus ReadMeshPrimitive(Json::Value const& primitive);
    TileIO::ReadStatus AddMeshPrimitive(Features features, Json::Value const& primitive);

    BufferView32 ReadMeshIndices(uint32_t& numIndices, Json::Value const& json) { return ReadBufferView32(json, "indices", &numIndices); }
    FPoint2d const* ReadParams(Json::Value const&);
    uint16_t const* ReadNormals(Json::Value const&);
    void AddMesh(MeshPrimitive const& mesh);
    uint32_t AddMeshVertex(MeshBuilderR, MeshPrimitive const&, uint32_t index, FeatureCR feature);

    void AddPolylines(MeshPrimitive const&, Json::Value const& json);
    Polyline ReadPolyline(void const*& pData, bool useShortIndices); // increments pData past the end of the polyline
public:
    DgnCacheTileRebuilder(StreamBufferR buffer, DgnModelR model, Render::System& system, Render::Primitives::MeshBuilderMapR builders)
        : GltfReader(buffer, model, system), m_builders(builders) { }

    TileIO::ReadStatus ReadTile(TileIO::Flags& flags, DgnElementIdSet const& skipElems);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::ReadFeatureTable(DgnElementIdSet const& skipElems)
    {
    uint32_t startPos = m_buffer.GetPos();

    FeatureTableHeader header;
    if (!header.Read(m_buffer))
        return TileIO::ReadStatus::ReadError;

    m_builders.SetMaxFeatures(header.maxFeatures);

    auto status = m_featureList.Read(m_buffer, header, skipElems);
    m_buffer.SetPos(startPos);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::FeatureList::Read(StreamBufferR buffer, FeatureTableHeader const& header, DgnElementIdSet const& skipElems)
    {
    m_entries.resize(header.count);
    for (uint32_t i = 0; i < header.count; i++)
        {
        uint64_t elemId;
        uint64_t subCatId;
        uint32_t geomClass;
        uint32_t index;

        if (!buffer.Read(elemId) || !buffer.Read(subCatId) || !buffer.Read(geomClass) || !buffer.Read(index))
            {
            BeAssert(false);
            return TileIO::ReadStatus::FeatureTableError;
            }

        BeAssert(index < m_entries.size());
        Feature feature(DgnElementId(elemId), DgnSubCategoryId(subCatId), static_cast<DgnGeometryClass>(geomClass));
        m_entries[index] = Entry(feature, skipElems.end() != skipElems.find(feature.GetElementId()));
        }

    return TileIO::ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::AddMeshPrimitive(Features features, Json::Value const& json)
    {
    MeshPrimitive mesh;

    auto materialName = json["material"];
    if (!materialName.isString())
        return TileIO::ReadStatus::ReadError;

    auto materialValue = m_materialValues[materialName.asString()];
    if (!materialValue.isObject())
        {
        BeAssert(false && "Material not found");
        return TileIO::ReadStatus::ReadError;
        }

    mesh.m_displayParams = _CreateDisplayParams(materialValue);
    if (mesh.m_displayParams.IsNull())
        return TileIO::ReadStatus::ReadError;

    mesh.m_vertices = ReadVertices(mesh.m_numVertices, json);
    mesh.m_features = features;
    mesh.m_type = static_cast<MeshPrimitive::Type>(json["type"].asUInt());
    mesh.m_isPlanar = json["isPlanar"].asBool();
    mesh.m_colorIndices = ReadColorIndices(json);
    mesh.m_colorsByIndex = ReadColorIndex(json);
    
    if (nullptr == mesh.m_vertices || mesh.m_colorsByIndex.empty())
        {
        BeAssert(false);
        return TileIO::ReadStatus::ReadError;
        }

    switch (mesh.m_type)
        {
        case MeshPrimitive::Type::Mesh:
            {
            mesh.m_indices = ReadMeshIndices(mesh.m_numIndices, json);
            if (!mesh.m_indices.IsValid())
                return TileIO::ReadStatus::ReadError;

            if (!mesh.m_displayParams->IgnoresLighting())
                {
                mesh.m_normals = ReadNormals(json);
                if (nullptr == mesh.m_normals)
                    return TileIO::ReadStatus::ReadError;
                }

            mesh.m_params = ReadParams(json);
            // ###TODO: Edges...

            AddMesh(mesh);
            break;
            }
        case MeshPrimitive::Type::Point:
        case MeshPrimitive::Type::Polyline:
            AddPolylines(mesh, json);
            break;
        default:
            BeAssert(false && "Invalid mesh primitive type");
            return TileIO::ReadStatus::ReadError;
        }

    return TileIO::ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::Features DgnCacheTileRebuilder::ReadFeatures(Json::Value const& prim)
    {
    if (prim.isMember("featureID"))
        return Features(prim["featureID"].asUInt());

    BufferView32 bufView = ReadBufferView32(prim, "featureIDs");
    BeAssert(bufView.IsValid());
    return Features(bufView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
QPoint3dCP DgnCacheTileRebuilder::ReadVertices(uint32_t& numVertices, Json::Value const& json)
    {
    auto view = ReadBufferView16(json["attributes"], "POSITION", &numVertices);
    BeAssert(!view.m_byte);
    return reinterpret_cast<QPoint3dCP>(view.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t const* DgnCacheTileRebuilder::ReadNormals(Json::Value const& json)
    {
    BufferView16 view = ReadBufferView16(json["attributes"], "NORMAL");
    BeAssert(!view.m_byte);
    return reinterpret_cast<uint16_t const*>(view.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
FPoint2d const* DgnCacheTileRebuilder::ReadParams(Json::Value const& json)
    {
    BufferView view = GetBufferView(json["attributes"], "TEXCOORD_0");
    if (view.IsValid())
        {
        BeAssert(GLTF_FLOAT == view.accessor["componentType"].asInt());
        return reinterpret_cast<FPoint2d const*>(view.pData);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnCacheTileRebuilder::AddMeshVertex(MeshBuilderR builder, MeshPrimitive const& mesh, uint32_t index, FeatureCR feature)
    {
    QPoint3dCR pos = mesh.m_vertices[index];
    uint16_t colorIdx = mesh.m_colorIndices[index];
    uint32_t fillColor = mesh.m_colorsByIndex[colorIdx];
    OctEncodedNormalCP normal = mesh.GetNormal(index);
    FPoint2dCP param = mesh.GetParam(index);

    VertexKey key(pos, feature, fillColor, normal, param);
    return builder.AddVertex(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddMesh(MeshPrimitive const& mesh)
    {
    BeAssert(MeshPrimitive::Type::Mesh == mesh.m_type);

    // Add triangles, skipping those which belong to excluded features
    // ###TODO: Optimize for the case in which this mesh contains no excluded features...in particular, if feature table is uniform we *know* no triangles will be omitted.
    MeshBuilderMap::Key key(*mesh.m_displayParams, nullptr != mesh.m_normals, mesh.m_type, mesh.m_isPlanar);
    MeshBuilderR builder = m_builders[key];
    for (uint32_t i = 0; i < mesh.m_numIndices; i += 3)
        {
        uint32_t index = mesh.m_indices[i];
        uint32_t featureId = mesh.m_features.GetFeatureId(index);
        FeatureCP feature = m_featureList.GetFeature(featureId);
        if (nullptr == feature)
            continue;

        uint32_t i0 = AddMeshVertex(builder, mesh, i, *feature);
        uint32_t i1 = AddMeshVertex(builder, mesh, i+1, *feature);
        uint32_t i2 = AddMeshVertex(builder, mesh, i+2, *feature);
        builder.AddTriangle(Triangle(i0, i1, i2, false));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::Polyline DgnCacheTileRebuilder::ReadPolyline(void const*& pData, bool useShortIndices)
    {
    static_assert(sizeof(PolylineHeader) == sizeof(float)+sizeof(FPoint3d)+sizeof(uint32_t), "Unexpected sizeof");

    Polyline polyline;
    CopyAndIncrement(&polyline.m_header, pData, sizeof(polyline.m_header));
    polyline.m_indices = BufferView32(pData, useShortIndices);

    size_t indexSize = useShortIndices ? sizeof(uint16_t) : sizeof(uint32_t);
    Increment(pData, indexSize * polyline.m_header.m_numIndices);

    return polyline;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddPolylines(MeshPrimitive const& mesh, Json::Value const& json)
    {
    BeAssert(MeshPrimitive::Type::Mesh != mesh.m_type);

    BufferView view = GetBufferView(json, "indices");
    if (!view.IsValid())
        {
        BeAssert(false);
        return;
        }

    bool useShortIndices = GLTF_UNSIGNED_SHORT == view.type;
    BeAssert(useShortIndices || GLTF_UINT32 == view.type);

    // Add line/point strings, skipping those which belong to excluded features
    bvector<QPoint3d> points;
    MeshBuilderMap::Key key(*mesh.m_displayParams, false, mesh.m_type, mesh.m_isPlanar);
    MeshBuilderR builder = m_builders[key];

    void const* pData = view.pData;
    for (size_t i = 0; i < view.count; i++)
        {
        Polyline polyline = ReadPolyline(pData, useShortIndices);
        if (polyline.m_header.m_numIndices == 0)
            {
            BeAssert(false);
            continue;
            }

        // The feature ID will be the same for each individual point/line string...
        uint32_t firstIndex = polyline.m_indices[0];
        uint32_t featureId = mesh.m_features.GetFeatureId(firstIndex);
        FeatureCP feature = m_featureList.GetFeature(featureId);
        if (nullptr == feature)
            continue;

        points.resize(polyline.m_header.m_numIndices);
        for (size_t j = 0; j < polyline.m_header.m_numIndices; j++)
            {
            uint32_t vertIdx = polyline.m_indices[j];
            points[j] = mesh.m_vertices[vertIdx];
            }

        // Fill color will also be the same for all vertices
        uint16_t colorIdx = mesh.m_colorIndices[firstIndex];
        uint32_t fillColor = mesh.m_colorsByIndex[colorIdx];
        builder.AddPolyline(points, *feature, fillColor, polyline.m_header.m_startDistance, polyline.m_header.m_rangeCenter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint32_t> DgnCacheTileRebuilder::ReadColorIndex(Json::Value const& json)
    {
    bvector<uint32_t> colors;
    auto colorTable = json["colorTable"];
    if (!colorTable.isArray())
        return colors;

    colors.reserve(colorTable.size());
    for (size_t i = 0; i < colorTable.size(); i++)
        colors.push_back(colorTable[static_cast<int>(i)].asUInt());

    return colors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::BufferView32 DgnCacheTileRebuilder::ReadBufferView32(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount)
    {
    BufferView view = GetBufferView(json, accessorName);
    if (!view.IsValid())
        return BufferView32();

    if (nullptr != pCount)
        *pCount = static_cast<uint32_t>(view.count);

    bool isShort = GLTF_UNSIGNED_SHORT == view.type;
    BeAssert(isShort || GLTF_UINT32 == view.type);
    return BufferView32(view.pData, isShort);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::BufferView16 DgnCacheTileRebuilder::ReadBufferView16(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount)
    {
    BufferView view = GetBufferView(json, accessorName);
    if (!view.IsValid())
        return BufferView16();

    if (nullptr != pCount)
        *pCount = static_cast<uint32_t>(view.count);

    bool isByte = GLTF_UNSIGNED_BYTE == view.type;
    BeAssert(isByte || GLTF_UNSIGNED_SHORT == view.type);
    return BufferView16(view.pData, isByte);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::ReadMeshPrimitive(Json::Value const& prim)
    {
    Features features = ReadFeatures(prim);
    if (!features.IsValid())
        return TileIO::ReadStatus::ReadError;

    if (!features.IsValid())
        {
        BeAssert(false);
        return TileIO::ReadStatus::Success;
        }

    // If only one feature, can trivially skip this mesh
    if (features.IsUniform() && m_featureList.RejectFeature(features.m_uniform))
        return TileIO::ReadStatus::Success;

    return AddMeshPrimitive(features, prim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::ReadGltf()
    {
    Json::Value meshes;
    auto status = InitGltf(meshes);
    if (TileIO::ReadStatus::Success != status)
        return status;

    for (auto& mesh : meshes)
        {
        auto& primitives = mesh["primitives"];
        if (primitives.isNull())
            {
            BeAssert(false);
            continue;
            }

        for (auto& primitive : primitives)
            if (TileIO::ReadStatus::Success != (status = ReadMeshPrimitive(primitive)))
                return status;
        }

    return TileIO::ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus DgnCacheTileRebuilder::ReadTile(TileIO::Flags& flags, DgnElementIdSet const& skipElems)
    {
    DgnTileHeader header;
    if (!header.Read(m_buffer))
        return TileIO::ReadStatus::InvalidHeader;

    flags = static_cast<TileIO::Flags>(header.flags);

    TileIO::ReadStatus status = ReadFeatureTable(skipElems);
    if (TileIO::ReadStatus::Success != status)
        return status;

    return ReadGltf();
    }

END_TILEREADER_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus TileIO::ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf)
    {
    return TileReader::DgnCacheTileReader(streamBuffer, model, renderSystem).ReadTile(contentRange, geometry, isLeaf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::ReadStatus TileIO::ReadDgnTile(Render::Primitives::MeshBuilderMapR builders, StreamBufferR buffer, GeometricModelR model, Render::System& system, TileIO::Flags& flags, DgnElementIdSet const& skipElems)
    {
    return TileReader::DgnCacheTileRebuilder(buffer, model, system, builders).ReadTile(flags, skipElems);
    }

