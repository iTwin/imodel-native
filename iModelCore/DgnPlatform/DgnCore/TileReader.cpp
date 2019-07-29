/*-------------------------------------------------------------------------------------+                                                                                           
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileReader.h>

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
    MeshPtr                 mesh = Mesh::Create(*displayParams, featureTable, primitiveType, DRange3d::NullRange(), !m_model.Is3d(), isPlanar, nodeIndex, nullptr);
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf, DRange3dCR tileRange)
    {
    return DgnTileReader(streamBuffer, model, renderSystem).ReadTile(contentRange, geometry, isLeaf);
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
