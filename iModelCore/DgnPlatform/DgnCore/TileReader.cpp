/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/TileReader.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileReader.h>

// Uncomment to turn on validation of logic for tile cache => MeshBuilderMap => GeometryCollection for debugging.
// #define TEST_TILE_REBUILDER

USING_NAMESPACE_TILE_IO
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

BEGIN_TILE_IO_NAMESPACE

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
BentleyStatus GltfReader::GetBufferView (void const*& pData, size_t& count, size_t& byteLength, Gltf::DataType& type, Json::Value& accessor, Json::Value const& primitiveValue, Utf8CP accessorName)
    {
    Json::Value     bufferView;

    if(SUCCESS != GetAccessorAndBufferView(accessor, bufferView, primitiveValue, accessorName))
        return ERROR;
    
    byteLength  = bufferView["byteLength"].asUInt();
    count       = accessor["count"].asUInt();
    pData = m_binaryData + bufferView["byteOffset"].asUInt() + accessor["byteOffset"].asUInt();
    type = Gltf::ToDataType(accessor["componentType"].asUInt());

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
    uint32_t count;
    auto data = ReadBufferData32(primitiveValue, accessorName, &count);
    if (!data.IsValid())
        return ERROR;

    switch (data.m_storageType)
        {
        case Gltf::DataType::UInt32:
            indices.resize(count);
            memcpy(indices.data(), data.m_data, count*sizeof(uint32_t));
            return SUCCESS;
        case Gltf::DataType::UnsignedShort:
        case Gltf::DataType::UnsignedByte:
            indices.reserve(count);
            for (uint32_t i = 0; i < count; i++)
                indices.push_back(data[i]);

            return SUCCESS;
        default:
            BeAssert(false && "unrecognized index type");
            return ERROR;

        }
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
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue, accessorName))
        return ERROR;

    size_t   nValues             = count * nComponents;



    switch (type)
        {
        case Gltf::DataType::UnsignedShort:
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

        case Gltf::DataType::Float:
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
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "BATCHID") ||
        type != Gltf::DataType::UnsignedShort)
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
BentleyStatus GltfReader::ReadVertices(QPoint3dListR vertexList, Json::Value const& primitiveValue)
    {
    void const*     pData;
    size_t          count, byteLength;
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "POSITION"))
        return ERROR;

    size_t              nValues             = count * 3;

    switch (static_cast<Gltf::DataType>(accessor["componentType"].asInt()))
        {
        case Gltf::DataType::UnsignedShort:
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

            vertexList.Assign(reinterpret_cast<QPoint3d const*>(pData), count, QPoint3d::Params(DRange3d::From(min[0].asDouble(), min[1].asDouble(), min[2].asDouble(), max[0].asDouble(), max[1].asDouble(), max[2].asDouble())));
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
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;

    switch (static_cast<Gltf::DataType>(accessor["componentType"].asInt()))
        {
        case Gltf::DataType::UnsignedByte:
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
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView(pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;


    switch (static_cast<Gltf::DataType>(accessor["componentType"].asInt()))
        {
        case Gltf::DataType::UnsignedByte:
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
template<typename T_Point> BentleyStatus GltfReader::ReadPoints(bvector<T_Point>& points, Json::Value const& value, Utf8CP accessorName)
    {
    void const*     pData;
    size_t          count, byteLength;
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, accessorName))
        return ERROR;

    switch (static_cast<Gltf::DataType>(accessor["componentType"].asInt()))
        {
        case Gltf::DataType::Float:
            {
            BeAssert (byteLength == count * sizeof(T_Point));
            points.resize(count);
            memcpy (points.data(), pData, count * sizeof(T_Point));
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
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, primitiveValue["attributes"], "_COLORINDEX"))
        return;

    switch (type)
        {
        case Gltf::DataType::UnsignedShort:
            {
            colors.resize(count);
            memcpy (colors.data(), pData, count * sizeof(uint16_t));
            break;
            }

        case Gltf::DataType::UnsignedByte:
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
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::ReadColorTable(ColorTableR colorTable, Json::Value const& primitiveValue)
    {
    return _ReadColorTable(colorTable, primitiveValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::_ReadColorTable(ColorTableR colorTable, Json::Value const& prim)
    {
    Json::Value matJson, matValues;
    Json::Value matName = prim["material"];
    if (!matName.isString() || !(matJson = m_materialValues[matName.asString()]).isObject() || !(matValues = matJson["values"]).isObject())
        return ERROR;

    Json::Value colorJson = matValues["color"];
    if (colorJson.isArray() && !colorJson.isNull()) // because apparently null things are considered arrays...
        {
        if (4 != colorJson.size())
            return ERROR;

        ColorDef color(static_cast<uint8_t>(colorJson[0].asDouble()*255),
                       static_cast<uint8_t>(colorJson[1].asDouble()*255),
                       static_cast<uint8_t>(colorJson[2].asDouble()*255),
                       255 - static_cast<uint8_t>(colorJson[3].asDouble()*255));

        colorTable.GetIndex(color.GetValue());

        BeAssert(colorTable.IsUniform());
        BeAssert(0 == colorTable.GetIndex(color.GetValue()));

        return SUCCESS;
        }

    // ###TODO: Ignoring existing of textured materials for now...
    Json::Value tex = matValues["tex"];
    if (!tex.isString())
        return ERROR;

    Image img = GetTextureImage(tex.asCString());
    if (!img.IsValid() || img.GetWidth() < 2)
        {
        BeAssert(false);
        return ERROR;
        }

    ByteStream const& bytes = img.GetByteStream();
    for (size_t i = 0; i < bytes.size(); i += 4)
        {
        ColorDef color(bytes[i], bytes[i+1], bytes[i+2], 255 - bytes[i+3]);
        colorTable.GetIndex(color.GetValue());
        }

    // NB: If color table is two-dimensional, there are probably unused entries in last row of texture...
    BeAssert(img.GetWidth() * img.GetHeight() == colorTable.size());
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnTileReader::_ReadColorTable(ColorTableR colorTable, Json::Value const& primitiveValue)
    {
    Json::Value colorTableJson = primitiveValue["colorTable"];

    if (!colorTableJson.isArray() || colorTableJson.isNull())
        return ERROR;

    for (uint32_t i=0; i<colorTableJson.size(); i++)
        colorTable.GetIndex(colorTableJson[i].asUInt());                                            
        
    return colorTable.size() > 0 ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource GltfReader::GetImageSource(Utf8CP name)
    {
    Json::Value extJson, gltfJson;
    Json::Value imgJson = m_images[name];
    if (!imgJson.isObject() || !(extJson = imgJson["extensions"]).isObject() || !(gltfJson = extJson["KHR_binary_glTF"]).isObject())
        return ImageSource();

    Json::Value bvJson;
    Json::Value bvName = gltfJson["bufferView"];
    if (!bvName.isString() || (bvJson = m_bufferViews[bvName.asCString()]).isNull())
        return ImageSource();

    size_t byteLength = bvJson["byteLength"].asUInt();
    void const* pData = m_binaryData + bvJson["byteOffset"].asUInt();

    auto mimeType = gltfJson["mimeType"].asString();
    auto format = mimeType.EndsWith("png") ? ImageSource::Format::Png : ImageSource::Format::Jpeg;
    return ImageSource(format, ByteStream(static_cast<uint8_t const*>(pData), static_cast<uint32_t>(byteLength)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Image GltfReader::GetImage(Utf8CP name)
    {
    return Image(GetImageSource(name));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource GltfReader::GetTextureImageSource(Utf8CP name)
    {
    Json::Value sourceJson;
    Json::Value textureJson = m_textures[name];
    if (!textureJson.isObject() || !(sourceJson = textureJson["source"]).isString())
        return ImageSource();

    return GetImageSource(sourceJson.asCString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Image GltfReader::GetTextureImage(Utf8CP name)
    {
    return Image(GetTextureImageSource(name));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TextureMapping GltfReader::GetTextureMapping(Utf8CP name, Json::Value const& paramsJson)
    {
    if (!m_namedTextures.isMember(name))
        return TextureMapping();

    TextureKey key;
    BeInt64Id textureId;
    if (BeStringUtilities::HasHexPrefix(name) && SUCCESS == BeInt64Id::FromString(textureId, name))
        key = TextureKey(DgnTextureId(textureId.GetValue()));
    else
        key = TextureKey(name);

    TexturePtr tex = m_renderSystem._FindTexture(key, m_model.GetDgnDb());
    if (tex.IsNull())
        {
        auto const& texJson = m_namedTextures[name];
        auto bvId = texJson["bufferView"].asString();
        if (!m_bufferViews.isMember(bvId))
            return TextureMapping();

        auto const& bvJson = m_bufferViews[bvId];
        uint32_t byteLength = bvJson["byteLength"].asUInt();
        void const* pData = m_binaryData + bvJson["byteOffset"].asUInt();

        auto format = static_cast<ImageSource::Format>(texJson["format"].asUInt());
        ImageSource img(format, ByteStream(static_cast<uint8_t const*>(pData), byteLength));
        Texture::CreateParams createParams(key);
        createParams.m_isGlyph = texJson["isGlyph"].asBool();
        tex = m_renderSystem._CreateTexture(img, Image::BottomUp::No, m_model.GetDgnDb(), createParams);
        if (tex.IsNull())
            return TextureMapping();
        }

    TextureMapping::Params params;
    params.m_mapMode = static_cast<TextureMapping::Mode>(paramsJson["mode"].asUInt());
    params.m_textureWeight = paramsJson["weight"].asDouble();
    params.m_worldMapping = paramsJson["worldMapping"].asBool();

    for (uint32_t i = 0; i < 2; i++)
        for (uint32_t j = 0; j < 3; j++)
            params.m_textureMat2x3.m_val[i][j] = paramsJson["transform"][i][j].asDouble();

    return TextureMapping(*tex, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::ReadPolylines(bvector<MeshPolyline>& polylines, Json::Value value, Utf8CP name, bool disjoint)
    {
    void const*     pData;
    size_t          count, byteLength;
    Gltf::DataType  type;
    Json::Value     accessor;

    if (SUCCESS != GetBufferView (pData, count, byteLength, type, accessor, value, name))
        return ERROR;

    for (size_t i=0; i<count; i++)
        {
        uint32_t                nIndices = 0;
        bvector<uint32_t>       indices;
        float                   startDistance;

        CopyAndIncrement(&startDistance, pData, sizeof(startDistance));
        CopyAndIncrement(&nIndices, pData, sizeof(nIndices));

        indices.resize(nIndices);
        if (Gltf::DataType::UnsignedShort == type)
            {
            for (size_t j=0; j<nIndices; j++)
                {
                uint16_t    index;

                CopyAndIncrement(&index, pData, sizeof(index));
                indices[j] = index;
                }

            // TFS#895336: We were previously failing to align polyline data.
            // Each polyline outputs:
            //  32-bit start distance
            //  32-bit num indices
            //  indices[nIndices] where every index is either 16- or 32-bits
            // In the case where indices are 16-bits and there are an odd number of them, we added 2 bytes of padding after the last index in order to
            // align to 32-bit boundary for next polyline. Need to skip that padding here.
            if (0 != (nIndices & 1))
                Increment(pData, sizeof(uint16_t));
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

        polylines.push_back(MeshPolyline(startDistance, std::move(indices)));
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
    bvector<uint32_t>   silhouetteIndices, visibleIndices;
    Json::Value const&  silhouettesValue = edgesValue["silhouettes"];

    if (SUCCESS == ReadIndices(visibleIndices, edgesValue, "visibles"))
        {
        meshEdges->m_visible.resize(visibleIndices.size()/2);
        memcpy (meshEdges->m_visible.data(), visibleIndices.data(), visibleIndices.size() * sizeof(uint32_t));
        }

    if (!silhouettesValue.isNull() &&
        SUCCESS == ReadNormalPairs(meshEdges->m_silhouetteNormals, silhouettesValue, "normalPairs") &&
        SUCCESS == ReadIndices(silhouetteIndices, silhouettesValue, "indices"))
        {
        meshEdges->m_silhouette.resize(silhouetteIndices.size()/2);
        memcpy (meshEdges->m_silhouette.data(), silhouetteIndices.data(), silhouetteIndices.size() * sizeof(uint32_t));
        }

    ReadPolylines(meshEdges->m_polylines, edgesValue, "polylines", false);

    return meshEdges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GltfReader::_ReadFeatures(bvector<uint32_t>& indices, Json::Value const& primitiveValue)
    {
    if (SUCCESS != ReadIndices(indices, primitiveValue["attributes"], "_BATCHID"))
        {
        // ###TODO: This can occur and is fine...e.g. reality models...but want to catch it for testing purposes.
        BeAssert(false && "Missing batch IDs");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnTileReader::_ReadFeatures(bvector<uint32_t>& indices, Json::Value const& primitiveValue)
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
    if (SUCCESS != _ReadFeatures(indices, primitiveValue) || (indices.size() > 1 && indices.size() != mesh.Points().size()))
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
MeshPtr GltfReader::ReadMeshPrimitive(Json::Value const& primitiveValue, FeatureTableP featureTable, size_t nodeIndex)
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
    MeshPtr                 mesh = Mesh::Create(*displayParams, featureTable, primitiveType, DRange3d::NullRange(), !m_model.Is3d(), isPlanar, nodeIndex);
    MeshEdgesPtr            meshEdges;

    if(SUCCESS != ReadVertices(mesh->VertsR(), primitiveValue))
        return nullptr;

    ReadColorTable(mesh->GetColorTableR(), primitiveValue);
    ReadColors(mesh->ColorsR(), primitiveValue);
    ReadFeatures(*mesh, primitiveValue);
#ifdef NOTNOW_DGNCLIENFX_AUXDATA_IMPLEMENTATION
    mesh->GetAuxData().m_displacementChannel = ReadAuxChannel<AuxDisplacementChannel, AuxDisplacementChannel::Data, FPoint3d> (primitiveValue["attributes"]["AUXDISPLACEMENTS"]);
    mesh->GetAuxData().m_paramChannel = ReadAuxChannel<AuxParamChannel, AuxParamChannel::Data, FPoint2d> (primitiveValue["attributes"]["AUXPARAMS"]);
    if (mesh->GetAuxData().m_paramChannel.IsValid() && mesh->ParamsR().empty())
        mesh->ParamsR() = mesh->GetAuxData().m_paramChannel->GetData().front()->GetValues(); 
#endif
    switch (primitiveType)
        {
        case Mesh::PrimitiveType::Mesh:
            {
            if (SUCCESS != ReadMeshIndices(*mesh, primitiveValue))
                return nullptr;

            if (!displayParams->IgnoresLighting() &&
                SUCCESS != ReadNormals(mesh->NormalsR(), primitiveValue["attributes"], "NORMAL"))
                return nullptr;
    
            ReadPoints(mesh->ParamsR(), primitiveValue["attributes"], "TEXCOORD_0");
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
ReadStatus  GltfReader::ReadGltf(Render::Primitives::GeometryCollectionR geometryCollection)
    {
    Json::Value meshValues;
    ReadStatus status = InitGltf(meshValues);
    if (ReadStatus::Success != status)
        return status;

    size_t      nodeIndex = 0;          // TBD.   Set by traversing nodes.
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
            
            if ((mesh = ReadMeshPrimitive(primitive, &geometryCollection.Meshes().FeatureTable(), nodeIndex)).IsValid())
                geometryCollection.Meshes().push_back(mesh);
            }
        }

    return ReadStatus::Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus GltfReader::InitGltf(Json::Value& meshValues)
    {
    Json::Reader    reader;          
    Gltf::Header    header;

    if (!header.Read(m_buffer))
        return ReadStatus::ReadError;

    m_binaryData = m_buffer.GetCurrent() + header.sceneStrLength;

    bvector<char>       sceneStrData(header.sceneStrLength);
    Json::Value         sceneValue;
    
    if(! m_buffer.ReadBytes(sceneStrData.data(), header.sceneStrLength))
        return ReadStatus::ReadError;

    if(!reader.parse(sceneStrData.data(), sceneStrData.data() + header.sceneStrLength, sceneValue))
        return ReadStatus::SceneParseError;

    meshValues     = sceneValue["meshes"];

    m_materialValues = sceneValue["materials"];
    m_accessors      = sceneValue["accessors"];
    m_bufferViews    = sceneValue["bufferViews"];
    m_textures       = sceneValue["textures"];
    m_images         = sceneValue["images"];
    m_namedTextures  = sceneValue["namedTextures"];

    if(!meshValues.isObject() || 
        !m_materialValues.isObject() ||
        !m_accessors.isObject() ||
        !m_bufferViews.isObject())
        return ReadStatus::SceneDataError;

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BufferData32 GltfReader::ReadBufferData32(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount)
    {
    BufferView view = GetBufferView(json, accessorName);
    if (!view.IsValid())
        return BufferData32();

    if (nullptr != pCount)
        *pCount = static_cast<uint32_t>(view.count);

    return BufferData32(view.pData, view.type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BufferData16 GltfReader::ReadBufferData16(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount)
    {
    BufferView view = GetBufferView(json, accessorName);
    if (!view.IsValid())
        return BufferData16();

    if (nullptr != pCount)
        *pCount = static_cast<uint32_t>(view.count);

    return BufferData16(view.pData, view.type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus B3dmReader::ReadTile(Render::Primitives::GeometryCollectionR geometry)
    {
    B3dm::Header header;
    if (!header.Read(m_buffer))
        return ReadStatus::InvalidHeader;

    uint32_t gltfStartPos = m_buffer.GetPos() + header.featureTableStrLen + header.featureTableBinaryLen + header.batchTableStrLen + header.batchTableBinaryLen;

    // feature table tells us how many entries are in the batch table
    if (0 != header.featureTableStrLen)
        {
        Json::Value featureTableJson;
        Json::Reader reader;
        Utf8CP featureTableStart = (Utf8CP)m_buffer.GetCurrent();
        if (!reader.parse(featureTableStart, featureTableStart + header.featureTableStrLen, featureTableJson))
            return ReadStatus::BatchTableParseError;

        uint32_t batchTableLen = featureTableJson["BATCH_LENGTH"].asUInt();
        if (0 != batchTableLen)
            {
            m_buffer.Advance(header.featureTableStrLen + header.featureTableBinaryLen);
            Json::Value batchTableJson;
            Utf8CP batchTableStart = (Utf8CP)m_buffer.GetCurrent();
            if (!reader.parse(batchTableStart, batchTableStart + header.batchTableStrLen, batchTableJson))
                return ReadStatus::BatchTableParseError;

            auto status = ReadFeatureTable(geometry.Meshes().FeatureTable(), batchTableJson, batchTableLen);
            if (ReadStatus::Success != status)
                return status;
            }
        }

    m_buffer.SetPos(gltfStartPos);
    return ReadGltf (geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus B3dmReader::ReadFeatureTable(Render::FeatureTableR features, Json::Value const& batch, uint32_t batchLen)
    {
    if (0 == batchLen)
        return ReadStatus::Success;

    Json::Value elems = batch["element"],
                subcats = batch["subCategory"],
                classes = batch["geomClass"];

    if (!elems.isArray() || !subcats.isArray() || !classes.isArray())
        return ReadStatus::BatchTableParseError;
    else if (elems.size() != batchLen || subcats.size() != batchLen || classes.size() != batchLen)
        return ReadStatus::BatchTableParseError;

    // NB: Index zero is an 'invalid' feature that's always present in batch table but never in batch IDs. Skip it.
    for (uint32_t i = 1; i < batchLen; i++)
        {
        Feature feature(DgnElementId(elems[i].asUInt64()), DgnSubCategoryId(subcats[i].asUInt64()), static_cast<DgnGeometryClass>(classes[i].asUInt()));
        features.Insert(feature, i);
        }

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static DisplayParamsCPtr displayParamsFromJson(Json::Value const& materialValue, DgnDbR db, Render::System& system, GltfReader& reader)
    {
    GradientSymbPtr     gradient;

    if (materialValue.isMember("gradient"))
        {
        gradient = GradientSymb::Create();

        if (SUCCESS != gradient->FromJson(materialValue["gradient"]))
            gradient = nullptr;
        }

    TextureMapping texMap;
    if (materialValue.isMember("texture"))
        texMap = reader.GetTextureMapping(materialValue["texture"]["name"].asCString(), materialValue["texture"]["params"]);

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
                                  db, system, texMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DgnTileReader::_CreateDisplayParams(Json::Value const& materialValue)
    {
    return displayParamsFromJson(materialValue, m_model.GetDgnDb(), m_renderSystem, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnTileReader::ReadFeatureTable(FeatureTableR featureTable)
    {
    uint32_t startPos = m_buffer.GetPos();

    DgnTile::FeatureTableHeader header;
    if (!header.Read(m_buffer))
        return ReadStatus::ReadError;

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
            return ReadStatus::FeatureTableError;
            }

        featureTable.Insert(Feature(DgnElementId(elementId), DgnSubCategoryId(subCategoryId), static_cast<DgnGeometryClass>(geometryClass)), index);
        }
    
    featureTable.SetModelId(m_model.GetModelId());

    m_buffer.SetPos(startPos + header.length);

    return ReadStatus::Success;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct StreamBufferScope
{
    StreamBufferR   m_buffer;

    explicit StreamBufferScope(StreamBufferR buffer) : m_buffer(buffer) { m_buffer.SetPos(0); }
    ~StreamBufferScope() { m_buffer.SetPos(0); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnTileReader::VerifyFeatureTable()
    {
    StreamBufferScope scope(m_buffer);

    DgnTile::Header tileHeader;
    DgnTile::FeatureTableHeader tableHeader;
    if (!tileHeader.Read(m_buffer) || !tableHeader.Read(m_buffer))
        {
        BeAssert(false);
        return false;
        }

    uint64_t elemId;
    uint32_t numBytesToSkip = sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t);
    CachedStatementPtr stmt = m_model.GetDgnDb().GetCachedStatement("SELECT ModelId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    for (size_t i = 0; i < tableHeader.count; i++)
        {
        if (!m_buffer.Read(elemId) || !m_buffer.Advance(numBytesToSkip))
            {
            BeAssert(false);
            return false;
            }

        stmt->BindId(1, DgnElementId(elemId));
        if (BE_SQLITE_ROW != stmt->Step())
            return false;

        stmt->Reset();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static bool processDeletedElements(DgnElementIdSet& deletedElemIds, StreamBufferR buffer, DgnModelR model, T func)
    {
    StreamBufferScope scope(buffer);

    DgnTile::Header tileHeader;
    DgnTile::FeatureTableHeader tableHeader;
    if (!tileHeader.Read(buffer) || !tableHeader.Read(buffer))
        {
        BeAssert(false);
        return false;
        }

    // Ugh, we must use ECSql because ECDb persists datetime as floating-point julian day value...
    CachedECSqlStatementPtr stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT LastMod FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECInstanceId=?");
    uint32_t numBytesToSkip = sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t);
    for (size_t i = 0; i < tableHeader.count; i++)
        {
        uint64_t elemId64;
        if (!buffer.Read(elemId64) || !buffer.Advance(numBytesToSkip))
            {
            BeAssert(false);
            return false;
            }

        DgnElementId elemId(elemId64);
        stmt->BindId(1, elemId);
        if (BE_SQLITE_ROW != stmt->Step())
            deletedElemIds.insert(elemId);
        else
            func(*stmt, elemId);

        stmt->Reset();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnTileReader::FindDeletedElements(DgnElementIdSet& deletedElemIds)
    {
    auto noop = [](CachedECSqlStatement& stmt, DgnElementId elemId) { };
    return processDeletedElements(deletedElemIds, m_buffer, m_model, noop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnTileReader::GetElements(DgnElementIdSet& deletedOrModified, DgnElementIdSet& unmodified, uint64_t lastModTime)
    {
    return processDeletedElements(deletedOrModified, m_buffer, m_model, [&](CachedECSqlStatement& stmt, DgnElementId elemId)
        {
        DateTime dt = stmt.GetValueDateTime(0);
        int64_t unixMillis;
        if (SUCCESS == dt.ToUnixMilliseconds(unixMillis))
            {
            auto& set = static_cast<uint64_t>(unixMillis) <= lastModTime ? unmodified : deletedOrModified;
            set.insert(elemId);
            }
        else
            {
            BeAssert(false);
            }
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnTileReader::ReadTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, bool& isLeaf)
    {
    DgnTile::Header header;
    if (!header.Read(m_buffer))
        return ReadStatus::InvalidHeader;

    ReadStatus status = ReadFeatureTable(geometry.Meshes().FeatureTable());
    if (ReadStatus::Success != status)
        return status;

    if (DgnTile::Flags::None != (header.flags & DgnTile::Flags::ContainsCurves))
        geometry.MarkCurved();

    if (DgnTile::Flags::None != (header.flags & DgnTile::Flags::Incomplete))
        geometry.MarkIncomplete();

    isLeaf = DgnTile::Flags::None != (header.flags & DgnTile::Flags::IsLeaf);
    contentRange = header.contentRange;

    return ReadGltf (geometry);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct DgnCacheTileRebuilder : GltfReader
{
private:
    struct Features
    {
        BufferData32    m_nonUniform;
        uint32_t        m_uniform = -1;

        Features() = default;
        explicit Features(uint32_t uniform) : m_uniform(uniform) { }
        explicit Features(BufferData32 nonUniform) : m_nonUniform(nonUniform) { }

        bool IsValid() const { return m_nonUniform.IsValid() || -1 != m_uniform; }
        bool IsUniform() const { BeAssert(IsValid()); return !IsNonUniform(); }
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

        ReadStatus Read(StreamBufferR buffer, DgnTile::FeatureTableHeader const& header, DgnElementIdSet const& skipElems);
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
        uint32_t        m_numIndices;
    };

    struct Polyline
    {
        PolylineHeader  m_header;
        BufferData32    m_indices;

        bool IsValid() const { return m_indices.IsValid(); }
    };

    struct MeshPrimitive
    {
        using Type = Render::Primitives::Mesh::PrimitiveType;

        // For mapping indices in mesh edges to indices in new mesh.
        // Key = old index (from cache)
        // Value = new index (if changed) or -1 (if vertex no longer exists)
        // Index values which did not change are not included in map
        using IndexMap = bmap<uint32_t, uint32_t>;

        uint32_t            m_numIndices;
        uint32_t            m_numVertices;
        DisplayParamsCPtr   m_displayParams;
        BufferData32        m_indices;
        QPoint3dCP          m_vertices;
        Features            m_features;
        uint16_t const*     m_normals = nullptr;
        FPoint2d const*     m_params = nullptr;
        bvector<uint32_t>   m_colorsByIndex;
        BufferData16        m_colorIndices;
        IndexMap            m_indexMap;
        Type                m_type;
        bool                m_isPlanar;

        OctEncodedNormalCP GetNormal(size_t index) const { return nullptr != m_normals ? reinterpret_cast<OctEncodedNormalCP>(m_normals+index) : nullptr; }
        FPoint2d const* GetParam(size_t index) const { return nullptr != m_params ? m_params+index : nullptr; }
        uint16_t GetColorIndex(size_t vertIndex) const
            {
            return m_colorIndices.IsValid() ? m_colorIndices[vertIndex] : 0;
            }
        uint32_t GetColorByIndex(uint16_t index) const
            {
            if (!m_colorIndices.IsValid())
                {
                BeAssert(1 == m_colorsByIndex.size());
                BeAssert(0 == index);
                return m_colorsByIndex[0];
                }
            else
                {
                return m_colorsByIndex[index];
                }
            }
        uint32_t GetVertexColor(uint32_t vertIndex) const { return GetColorByIndex(GetColorIndex(vertIndex)); }
        uint32_t RemapIndex(uint32_t oldIndex) const
            {
            auto iter = m_indexMap.find(oldIndex);
            return m_indexMap.end() != iter ? iter->second : oldIndex;
            }
    };

    Render::Primitives::MeshBuilderMapR m_builders;
    FeatureList m_featureList;

    DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue) override
        {
        return displayParamsFromJson(materialValue, m_model.GetDgnDb(), m_renderSystem, *this);
        }
    BentleyStatus _ReadFeatures(bvector<uint32_t>& featureIndices, Json::Value const& primitiveValue) override { BeAssert(false); return ERROR; } // unused...

    QPoint3d const* ReadVertices(uint32_t& numVertices, Json::Value const& json);
    BufferData16 ReadColorIndices(Json::Value const& json) { return ReadBufferData16(json["attributes"], "_COLORINDEX"); }
    Features ReadFeatures(Json::Value const& json);
    bvector<uint32_t> ReadColorIndex(Json::Value const& json);

    ReadStatus ReadFeatureTable(DgnElementIdSet const& skipElems);
    ReadStatus ReadGltf();
    ReadStatus ReadMeshPrimitive(Json::Value const& primitive);
    ReadStatus AddMeshPrimitive(Features features, Json::Value const& primitive);

    BufferData32 ReadMeshIndices(uint32_t& numIndices, Json::Value const& json) { return ReadBufferData32(json, "indices", &numIndices); }
    FPoint2d const* ReadPoints(Json::Value const&);
    uint16_t const* ReadNormals(Json::Value const&);
    void AddMesh(MeshPrimitive& mesh, Json::Value const&);
    uint32_t AddMeshVertex(MeshBuilderR, MeshPrimitive const&, uint32_t index, FeatureCR feature);
    void AddTriangle(MeshBuilderR, MeshPrimitive&, uint32_t indexOfFirstIndex, FeatureCP feature);

    void AddPolylines(MeshPrimitive const&, Json::Value const& json);
    Polyline ReadPolyline(void const*& pData, bool useShortIndices); // increments pData past the end of the polyline

    void AddMeshEdges(MeshBuilderR builder, MeshPrimitive const& mesh, Json::Value const& json);
    void AddPolylineEdges(MeshEdgesR, MeshPrimitive const&, Json::Value const&);
    void AddSilhouetteEdges(MeshEdgesR, MeshPrimitive const&, Json::Value const&);
    void AddVisibleEdges(MeshEdgesR, MeshPrimitive const&, Json::Value const&);
public:
    DgnCacheTileRebuilder(StreamBufferR buffer, DgnModelR model, Render::System& system, Render::Primitives::MeshBuilderMapR builders)
        : GltfReader(buffer, model, system), m_builders(builders) { }

    ReadStatus ReadTile(DgnTile::Flags& flags, DgnElementIdSet const& skipElems);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnCacheTileRebuilder::ReadFeatureTable(DgnElementIdSet const& skipElems)
    {
    uint32_t startPos = m_buffer.GetPos();

    DgnTile::FeatureTableHeader header;
    if (!header.Read(m_buffer))
        return ReadStatus::ReadError;

    m_builders.SetMaxFeatures(header.maxFeatures);

    auto status = m_featureList.Read(m_buffer, header, skipElems);
    m_buffer.SetPos(startPos + header.length);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnCacheTileRebuilder::FeatureList::Read(StreamBufferR buffer, DgnTile::FeatureTableHeader const& header, DgnElementIdSet const& skipElems)
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
            return ReadStatus::FeatureTableError;
            }

        BeAssert(index < m_entries.size());
        Feature feature(DgnElementId(elemId), DgnSubCategoryId(subCatId), static_cast<DgnGeometryClass>(geomClass));
        m_entries[index] = Entry(feature, skipElems.end() != skipElems.find(feature.GetElementId()));
        }

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnCacheTileRebuilder::AddMeshPrimitive(Features features, Json::Value const& json)
    {
    MeshPrimitive mesh;

    auto materialName = json["material"];
    if (!materialName.isString())
        return ReadStatus::ReadError;

    auto materialValue = m_materialValues[materialName.asString()];
    if (!materialValue.isObject())
        {
        BeAssert(false && "Material not found");
        return ReadStatus::ReadError;
        }

    mesh.m_displayParams = _CreateDisplayParams(materialValue);
    if (mesh.m_displayParams.IsNull())
        return ReadStatus::ReadError;

    mesh.m_vertices = ReadVertices(mesh.m_numVertices, json);
    mesh.m_features = features;
    mesh.m_type = static_cast<MeshPrimitive::Type>(json["type"].asUInt());
    mesh.m_isPlanar = json["isPlanar"].asBool();
    mesh.m_colorIndices = ReadColorIndices(json);
    mesh.m_colorsByIndex = ReadColorIndex(json);
    
    if (nullptr == mesh.m_vertices || mesh.m_colorsByIndex.empty())
        {
        BeAssert(false);
        return ReadStatus::ReadError;
        }

    switch (mesh.m_type)
        {
        case MeshPrimitive::Type::Mesh:
            {
            mesh.m_indices = ReadMeshIndices(mesh.m_numIndices, json);
            if (!mesh.m_indices.IsValid())
                return ReadStatus::ReadError;

            if (!mesh.m_displayParams->IgnoresLighting())
                {
                mesh.m_normals = ReadNormals(json);
                if (nullptr == mesh.m_normals)
                    return ReadStatus::ReadError;
                }

            mesh.m_params = ReadPoints(json);
            AddMesh(mesh, json); // also adds edges...
            break;
            }
        case MeshPrimitive::Type::Point:
        case MeshPrimitive::Type::Polyline:
            AddPolylines(mesh, json);
            break;
        default:
            BeAssert(false && "Invalid mesh primitive type");
            return ReadStatus::ReadError;
        }

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::Features DgnCacheTileRebuilder::ReadFeatures(Json::Value const& prim)
    {
    if (prim.isMember("featureID"))
        return Features(prim["featureID"].asUInt());

    BufferData32 bufView = ReadBufferData32(prim, "featureIDs");
    BeAssert(bufView.IsValid());
    return Features(bufView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
QPoint3dCP DgnCacheTileRebuilder::ReadVertices(uint32_t& numVertices, Json::Value const& json)
    {
    auto view = ReadBufferData16(json["attributes"], "POSITION", &numVertices);
    BeAssert(Gltf::DataType::UnsignedShort == view.m_storageType);
    return reinterpret_cast<QPoint3dCP>(view.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t const* DgnCacheTileRebuilder::ReadNormals(Json::Value const& json)
    {
    BufferData16 view = ReadBufferData16(json["attributes"], "NORMAL");
    BeAssert(Gltf::DataType::UnsignedByte == view.m_storageType); // Ray writes each normal as a Vec2 of 2 bytes...
    return reinterpret_cast<uint16_t const*>(view.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
FPoint2d const* DgnCacheTileRebuilder::ReadPoints(Json::Value const& json)
    {
    BufferView view = GetBufferView(json["attributes"], "TEXCOORD_0");
    if (view.IsValid())
        {
        BeAssert(Gltf::DataType::Float == Gltf::ToDataType(view.accessor["componentType"].asInt()));
        return reinterpret_cast<FPoint2d const*>(view.pData);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnCacheTileRebuilder::AddMeshVertex(MeshBuilderR builder, MeshPrimitive const& mesh, uint32_t indexIndex, FeatureCR feature)
    {
    uint32_t index = mesh.m_indices[indexIndex];

    QPoint3dCR pos = mesh.m_vertices[index];
    uint32_t fillColor = mesh.GetVertexColor(index);
    OctEncodedNormalCP normal = mesh.GetNormal(index);
    FPoint2dCP param = mesh.GetParam(index);

    VertexKey key(pos, feature, fillColor, normal, param);
    return builder.AddVertex(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddTriangle(MeshBuilderR builder, MeshPrimitive& mesh, uint32_t indexOfFirstIndex, FeatureCP feature)
    {
    uint32_t oldIndices[3] = { mesh.m_indices[indexOfFirstIndex], mesh.m_indices[indexOfFirstIndex+1], mesh.m_indices[indexOfFirstIndex+2] };
    uint32_t newIndices[3];

    if (nullptr == feature)
        {
        newIndices[0] = newIndices[1] = newIndices[2] = -1;
        }
    else
        {
        newIndices[0] = AddMeshVertex(builder, mesh, indexOfFirstIndex, *feature);
        newIndices[1] = AddMeshVertex(builder, mesh, indexOfFirstIndex+1, *feature);
        newIndices[2] = AddMeshVertex(builder, mesh, indexOfFirstIndex+2, *feature);
        builder.AddTriangle(Triangle(newIndices[0], newIndices[1], newIndices[2], false));
        }

    // If indices changed, map old to new index so we can remap edge indices later
    for (size_t i = 0; i < 3; i++)
        if (oldIndices[i] != newIndices[i])
            mesh.m_indexMap.Insert(oldIndices[i], newIndices[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddMesh(MeshPrimitive& mesh, Json::Value const& json)
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
#if defined(TEST_TILE_REBUILDER)
        BeAssert(nullptr != feature);
#endif
        AddTriangle(builder, mesh, i, feature);
        }

    AddMeshEdges(builder, mesh, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddMeshEdges(MeshBuilderR builder, MeshPrimitive const& mesh, Json::Value const& primJson)
    {
    if (nullptr == builder.GetMesh())
        return;

    auto edgesJson = primJson["edges"];
    if (edgesJson.isNull())
        return;

    // AddMesh() has removed any vertices belonging to features to be excluded from the reconstituted mesh;
    // and has populated a mapping from old to new indices (with -1 == removed).
    // All we need to do for edges is skip those which have been removed, and remap the indices of the rest.
    if (builder.GetMesh()->GetEdgesR().IsNull())
        builder.GetMesh()->GetEdgesR() = new MeshEdges();

    MeshEdgesPtr edges = builder.GetMesh()->GetEdgesR();
    AddVisibleEdges(*edges, mesh, edgesJson);
    AddPolylineEdges(*edges, mesh, edgesJson);
    AddSilhouetteEdges(*edges, mesh, edgesJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCacheTileRebuilder::Polyline DgnCacheTileRebuilder::ReadPolyline(void const*& pData, bool useShortIndices)
    {
    static_assert(sizeof(PolylineHeader) == sizeof(float)+sizeof(uint32_t), "Unexpected sizeof");

    Polyline polyline;
    CopyAndIncrement(&polyline.m_header, pData, sizeof(polyline.m_header));
    polyline.m_indices = BufferData32(pData, useShortIndices ? Gltf::DataType::UnsignedShort : Gltf::DataType::UInt32);

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

    bool useShortIndices = Gltf::DataType::UnsignedShort == view.type;
    BeAssert(useShortIndices || Gltf::DataType::UInt32 == view.type);

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
        uint32_t fillColor = mesh.GetVertexColor(firstIndex);
        builder.AddPolyline(points, *feature, fillColor, polyline.m_header.m_startDistance);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddPolylineEdges(MeshEdgesR edges, MeshPrimitive const& mesh, Json::Value const& json)
    {
    BufferView view = GetBufferView(json, "polylines");
    if (!view.IsValid())
        return;

    bool useShortIndices = Gltf::DataType::UnsignedShort == view.type;
    BeAssert(useShortIndices || Gltf::DataType::UInt32 == view.type);

    void const* pData = view.pData;
    bvector<uint32_t> newIndices;
    for (size_t i = 0; i < view.count; i++)
        {
        Polyline polyline = ReadPolyline(pData, useShortIndices);
        if (polyline.m_header.m_numIndices == 0)
            {
            BeAssert(false);
            continue;
            }

        // If any vertex was removed, the entire polyline edge was removed...
        uint32_t newFirstIndex = mesh.RemapIndex(polyline.m_indices[0]);
        if (-1 == newFirstIndex)
            continue;

        newIndices.resize(polyline.m_header.m_numIndices);
        newIndices[0] = newFirstIndex;
        for (uint32_t i = 1; i < polyline.m_header.m_numIndices; i++)
            newIndices[i] = mesh.RemapIndex(polyline.m_indices[i]);

        MeshPolyline mpl(polyline.m_header.m_startDistance, std::move(newIndices));
        edges.m_polylines.push_back(std::move(mpl));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddSilhouetteEdges(MeshEdgesR edges, MeshPrimitive const& mesh, Json::Value const& edgesJson)
    {
    auto json = edgesJson["silhouettes"];
    if (json.isNull())
        return;

    uint32_t numIndices;
    BufferData32 indexView = ReadBufferData32(json, "indices", &numIndices);
    if (!indexView.IsValid() || 0 == numIndices)
        return;

    // If any vertex was removed, the entire silhouette was removed...
    uint32_t newFirstIndex = mesh.RemapIndex(indexView[0]);
    if (-1 == newFirstIndex)
        return;

    if (SUCCESS != ReadNormalPairs(edges.m_silhouetteNormals, json, "normalPairs"))
        {
        BeAssert(false);
        return;
        }

    // m_silhouettes is a list of MeshEdge, which is a pair of indices...
    BeAssert(0 == (numIndices & 1));
    edges.m_silhouette.resize(numIndices/2);
    edges.m_silhouette[0] = MeshEdge(newFirstIndex, mesh.RemapIndex(indexView[1]));
    for (uint32_t i = 1; i < edges.m_silhouette.size(); i++)
        {
        uint32_t baseIndex = i * 2;
        edges.m_silhouette[i] = MeshEdge(mesh.RemapIndex(indexView[baseIndex]), mesh.RemapIndex(indexView[baseIndex+1]));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCacheTileRebuilder::AddVisibleEdges(MeshEdgesR edges, MeshPrimitive const& mesh, Json::Value const& edgesJson)
    {
    uint32_t numIndices;
    BufferData32 indexView = ReadBufferData32(edgesJson, "visibles", &numIndices);
    if (!indexView.IsValid() || 0 == numIndices)
        return;

    // If any vertex was removed, the entire silhouette was removed...
    uint32_t newFirstIndex = mesh.RemapIndex(indexView[0]);
    if (-1 == newFirstIndex)
        return;

    BeAssert(0 == (numIndices & 1));
    edges.m_visible.resize(numIndices/2);
    edges.m_visible[0] = MeshEdge(newFirstIndex, mesh.RemapIndex(indexView[1]));
    for (uint32_t i = 1; i < edges.m_visible.size(); i++)
        {
        uint32_t baseIndex = i * 2;
        edges.m_visible[i] = MeshEdge(mesh.RemapIndex(indexView[baseIndex]), mesh.RemapIndex(indexView[baseIndex+1]));
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
ReadStatus DgnCacheTileRebuilder::ReadMeshPrimitive(Json::Value const& prim)
    {
    Features features = ReadFeatures(prim);
    if (!features.IsValid())
        return ReadStatus::ReadError;

    if (!features.IsValid())
        {
        BeAssert(false);
        return ReadStatus::Success;
        }

    // If only one feature, can trivially skip this mesh
    if (features.IsUniform() && m_featureList.RejectFeature(features.m_uniform))
        return ReadStatus::Success;

    return AddMeshPrimitive(features, prim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnCacheTileRebuilder::ReadGltf()
    {
    Json::Value meshes;
    auto status = InitGltf(meshes);
    if (ReadStatus::Success != status)
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
            if (ReadStatus::Success != (status = ReadMeshPrimitive(primitive)))
                return status;
        }

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus DgnCacheTileRebuilder::ReadTile(DgnTile::Flags& flags, DgnElementIdSet const& skipElems)
    {
    DgnTile::Header header;
    if (!header.Read(m_buffer))
        return ReadStatus::InvalidHeader;

    flags = header.flags;

    ReadStatus status = ReadFeatureTable(skipElems);
    if (ReadStatus::Success != status)
        return status;

    return ReadGltf();
    }

#if defined(TEST_TILE_REBUILDER)

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/18
//=======================================================================================
struct MeshVertex
{
    QPoint3d            m_pos;
    OctEncodedNormal    m_normal = OctEncodedNormal::From(0);
    Feature             m_feature;
    ColorDef            m_color = ColorDef::Magenta();
    FPoint2d            m_param;

    MeshVertex(MeshCR mesh, uint32_t index)
        {
        BeAssert(index < mesh.Verts().size());
        m_pos = mesh.Verts()[index];

        if (!mesh.Normals().empty())
            m_normal = mesh.Normals()[index];

        if (!mesh.Params().empty())
            m_param = mesh.Params()[index];
        else
            m_param.x = m_param.y = 0.0;

        if (mesh.Colors().empty())
            {
            BeAssert(1 == mesh.GetColorTable().size());
            m_color = ColorDef(mesh.GetColorTable().begin()->first);
            }
        else
            {
            BeAssert(1 < mesh.GetColorTable().size());
            auto colorIndex = mesh.Colors()[index];
            if (!mesh.GetColorTable().FindByIndex(m_color, colorIndex))
                BeAssert(false);
            }

        auto const& featureIndices = mesh.GetFeatureIndices();
        uint32_t featureIndex = 0;
        if (featureIndices.empty())
            {
            if (!mesh.GetUniformFeatureIndex(featureIndex))
                BeAssert(false);
            }
        else
            {
            featureIndex = featureIndices[index];
            }

        BeAssert(nullptr != mesh.GetFeatureTable());
        if (!mesh.GetFeatureTable()->FindFeature(m_feature, featureIndex))
            BeAssert(false);
        }

    bool operator==(MeshVertex const& rhs) const
        {
        if (m_pos != rhs.m_pos || m_normal != rhs.m_normal || m_feature != rhs.m_feature || m_color != rhs.m_color)
            return false;

        static constexpr double tolerance = 0.1;
        return abs(m_param.x - rhs.m_param.x) < tolerance && abs(m_param.y - rhs.m_param.y);
        }

    void Compare(MeshVertex const& rhs) const
        {
        BeAssert(m_pos == rhs.m_pos);
        BeAssert(m_normal == rhs.m_normal);
        BeAssert(m_feature == rhs.m_feature);
        BeAssert(m_color == rhs.m_color);

        static constexpr double tolerance = 0.1;
        BeAssert(abs(m_param.x - rhs.m_param.x) < tolerance);
        BeAssert(abs(m_param.y - rhs.m_param.y) < tolerance);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareMeshEdges(MeshCR lhm, MeshCR rhm, bvector<MeshEdge> const& lhe, bvector<MeshEdge> const& rhe)
    {
    bool asserted = false;
    BeAssert(lhe.size() == rhe.size());
    auto const& lhv = lhm.Verts();
    auto const& rhv = rhm.Verts();
    for (size_t i = 0; i < lhe.size(); i++)
        {
        auto const& lh = lhe[i];
        auto const& rh = rhe[i];

        auto lhp0 = lhv[lh.m_indices[0]],
             lhp1 = lhv[lh.m_indices[1]],
             rhp0 = rhv[rh.m_indices[0]],
             rhp1 = rhv[rh.m_indices[1]];

        // possibly order of indices reversed due to index remapping...
        if (lhp0 != rhp0)
            std::swap(lhp0, lhp1);

        if (!asserted)
            {
            BeAssert(lhp0 == rhp0);
            BeAssert(lhp1 == rhp1);
            if (lhp0 != rhp0 || lhp1 != rhp1)
                asserted = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareVisibleEdges(MeshCR lhm, MeshCR rhm)
    {
    compareMeshEdges(lhm, rhm, lhm.GetEdges()->m_visible, rhm.GetEdges()->m_visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareSilhouettes(MeshCR lhm, MeshCR rhm)
    {
    compareMeshEdges(lhm, rhm, lhm.GetEdges()->m_silhouette, rhm.GetEdges()->m_silhouette);

    auto const& lhn = lhm.GetEdges()->m_silhouetteNormals;
    auto const& rhn = rhm.GetEdges()->m_silhouetteNormals;
    BeAssert(lhn.size() == rhn.size());

    bool asserted = false;
    for (size_t i = 0; i < lhn.size(); i++)
        {
        auto const& lhp = lhn[i];
        auto const& rhp = rhn[i];

        if (!asserted)
            {
            BeAssert(lhp.first == rhp.first);
            BeAssert(lhp.second == rhp.second);
            if (lhp.first != rhp.first || lhp.second != rhp.second)
                asserted = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareVertices(MeshCR lhm, uint32_t lhi, MeshCR rhm, uint32_t rhi)
    {
    MeshVertex lhv(lhm, lhi),
               rhv(rhm, rhi);

    lhv.Compare(rhv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void comparePolylines(MeshPolylineCR lhl, MeshCR lhm, MeshPolylineCR rhl, MeshCR rhm)
    {
    BeAssert(DoubleOps::AlmostEqual(lhl.GetStartDistance(), rhl.GetStartDistance()));

    auto const& lhIndices = lhl.GetIndices();
    auto const& rhIndices = rhl.GetIndices();
    BeAssert(lhIndices.size() == rhIndices.size());

    for (size_t i = 0; i < lhIndices.size(); i++)
        compareVertices(lhm, lhIndices[i], rhm, rhIndices[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void comparePolylineLists(PolylineList const& lhl, PolylineList const& rhl, MeshCR lhm, MeshCR rhm)
    {
    BeAssert(lhl.size() == rhl.size());
    for (size_t i = 0; i < lhl.size(); i++)
        comparePolylines(lhl[i], lhm, rhl[i], rhm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareEdges(MeshCR lhm, MeshCR rhm)
    {
    compareVisibleEdges(lhm, rhm);
    compareSilhouettes(lhm, rhm);
    comparePolylineLists(lhm.GetEdges()->m_polylines, rhm.GetEdges()->m_polylines, lhm, rhm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareTriangles(MeshCR lhs, MeshCR rhs)
    {
    auto const& lhv = lhs.Verts();
    auto const& rhv = rhs.Verts();
    auto const& lht = lhs.Triangles().Indices();
    auto const& rht = rhs.Triangles().Indices();
    BeAssert(lht.size() == rht.size());

    for (size_t i = 0; i < lht.size(); i++)
        compareVertices(lhs, lht[i], rhs, rht[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareMeshes(MeshCR lhs, MeshCR rhs)
    {
    BeAssert(lhs.GetDisplayParams().IsEqualTo(rhs.GetDisplayParams(), DisplayParams::ComparePurpose::Merge));

    auto const& lhct = lhs.GetColorTable();
    auto const& rhct = rhs.GetColorTable();
    BeAssert(lhct.size() == rhct.size());
    for (auto lhi = lhct.begin(), rhi = rhct.begin(); lhi != lhct.end(); ++lhi, ++rhi)
        BeAssert(lhi->first == rhi->first);

    auto const& lhv = lhs.Verts();
    auto const& rhv = rhs.Verts();
    BeAssert(lhv.size() == rhv.size());
    BeAssert(lhv.GetParams().GetOrigin().AlmostEqual(rhv.GetParams().GetOrigin()));
    BeAssert(lhv.GetParams().GetScale().AlmostEqual(rhv.GetParams().GetScale()));

    compareTriangles(lhs, rhs);
    comparePolylineLists(lhs.Polylines(), rhs.Polylines(), lhs, rhs);

    BeAssert(lhs.GetEdges().IsValid() == rhs.GetEdges().IsValid());
    if (lhs.GetEdges().IsValid() && rhs.GetEdges().IsValid())
        compareEdges(lhs, rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void compareMeshLists(MeshList const& lhs, MeshList const& rhs)
    {
    FeatureTable const& lhft = lhs.m_features;
    FeatureTable const& rhft = rhs.m_features;
    BeAssert(lhft.size() == rhft.size());
    for (auto lhi = lhft.begin(), rhi = rhft.begin(); lhi != lhft.end(); ++lhi, ++rhi)
        BeAssert(lhi->first == rhi->first);

    for (size_t i = 0; i < lhs.size(); i++)
        compareMeshes(*lhs[i], *rhs[i]);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf, DRange3dCR tileRange)
    {
#if defined(TEST_TILE_REBUILDER)
    // Tile data => MeshBuilderMap => GeometryCollection
    ReadDgnTile(contentRange, geometry, streamBuffer, model, renderSystem, isLeaf, tileRange, DgnElementIdSet());

    // Compare with ordinary deserialization path...
    ElementAlignedBox3d testContentRange;
    bool testIsLeaf;
    Render::Primitives::GeometryCollection testGeometry;
    streamBuffer.SetPos(0);
    ReadStatus testStatus = DgnTileReader(streamBuffer, model, renderSystem).ReadTile(testContentRange, testGeometry, testIsLeaf);

    BeAssert(ReadStatus::Success == testStatus);
    BeAssert(testContentRange.low.AlmostEqual(contentRange.low) && testContentRange.high.AlmostEqual(contentRange.high));
    BeAssert(testIsLeaf == isLeaf);
    BeAssert(testGeometry.Meshes().size() == geometry.Meshes().size());

    compareMeshLists(testGeometry.Meshes(), geometry.Meshes());

    return ReadStatus::Success;
#else
    return DgnTileReader(streamBuffer, model, renderSystem).ReadTile(contentRange, geometry, isLeaf);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf, DRange3dCR tileRange, DgnElementIdSet const& skipElems)
    {
    if (skipElems.empty())
        return ReadDgnTile(contentRange, geometry, streamBuffer, model, renderSystem, isLeaf, tileRange);

    DgnTile::Header header;
    header.Read(streamBuffer);
    streamBuffer.ResetPos();
    contentRange = header.contentRange;
    isLeaf = DgnTile::Flags::None != (header.flags & DgnTile::Flags::IsLeaf);

    geometry.Meshes().FeatureTable() = FeatureTable(model.GetModelId(), 100000); // maxFeatures will be read + updated in ReadFeatureTable()...
    Render::Primitives::MeshBuilderMap builders(0.0, &geometry.Meshes().FeatureTable(), tileRange, false);
    DgnTile::Flags flags;
    auto status = ReadDgnTile(builders, streamBuffer, model, renderSystem, flags, skipElems);
    if (ReadStatus::Success != status)
        { BeAssert(false); return status; }

    for (auto& builder : builders)
        {
        MeshP mesh = builder.second->GetMesh();
        if (nullptr != mesh && !mesh->IsEmpty())
            geometry.Meshes().push_back(mesh);
        }

    if (DgnTile::Flags::None != (flags & DgnTile::Flags::ContainsCurves))
        geometry.MarkCurved();

    if (DgnTile::Flags::None != (flags & DgnTile::Flags::Incomplete))
        geometry.MarkIncomplete();

    return ReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus ReadDgnTile(Render::Primitives::MeshBuilderMapR builders, StreamBufferR buffer, GeometricModelR model, Render::System& system, DgnTile::Flags& flags, DgnElementIdSet const& skipElems)
    {
    BeAssert(!builders.GetRange().IsNull());
    return DgnCacheTileRebuilder(buffer, model, system, builders).ReadTile(flags, skipElems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus ReadWebTile(Render::Primitives::GeometryCollectionR geom, StreamBufferR buffer, GeometricModelR model, Render::System& system)
    {
    TileHeader header;
    if (!header.Read(buffer))
        return ReadStatus::InvalidHeader;

    buffer.ResetPos();
    switch (header.format)
        {
        case Format::B3dm:
            return B3dmReader(buffer, model, system).ReadTile(geom);
        default:
            BeAssert(false && "Unsupported tile format");
            return ReadStatus::ReadError;
        }
    }

struct ExtFmtPair { Utf8CP ext; Format fmt; };
static constexpr ExtFmtPair s_extFmtPairs[] =
    {
        { "b3dm", Format::B3dm },
        { "i3dm", Format::I3dm },
        { "vctr", Format::Vector },
        { "pnts", Format::PointCloud },
        { "cmpt", Format::Composite },
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Format TileHeader::FormatFromFileExtension(Utf8CP ext)
    {
    for (auto const& pair : s_extFmtPairs)
        if (0 == BeStringUtilities::Stricmp(ext, pair.ext))
            return pair.fmt;

    return Format::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP TileHeader::FileExtensionFromFormat(Format fmt)
    {
    for (auto const& pair : s_extFmtPairs)
        if (pair.fmt == fmt)
            return pair.ext;

    return nullptr;
    }

END_TILE_IO_NAMESPACE
