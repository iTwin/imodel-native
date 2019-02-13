/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/CacheTileWriter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>

USING_NAMESPACE_TILE_IO
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

BEGIN_TILE_IO_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct DgnCacheTileWriter : Tile::IO::Writer
{

    DEFINE_T_SUPER(Tile::IO::Writer);

public:
    DgnCacheTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : Tile::IO::Writer(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateColorTable(ColorTableCR colorTable)
    {
    Json::Value     jsonTable = Json::arrayValue;

    for (auto& entry : colorTable)
        jsonTable[entry.second] = entry.first;

    return jsonTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateMeshEdges(MeshEdgesCR meshEdges, size_t maxIndex, Utf8StringCR idStr)
    {
    Json::Value     edgesValue= Json::objectValue;

    if (!meshEdges.m_visible.empty())
        edgesValue["visibles"] = AddMeshIndices("visibles", meshEdges.m_visible.front().m_indices, 2 * meshEdges.m_visible.size(), idStr, maxIndex);

    if (!meshEdges.m_silhouette.empty())
        {
        edgesValue["silhouettes"]["indices"]  = AddMeshIndices("silhouettes", meshEdges.m_silhouette.front().m_indices, 2 * meshEdges.m_silhouette.size(), idStr, maxIndex);
        edgesValue["silhouettes"]["normalPairs"] = AddNormalPairs(meshEdges.m_silhouetteNormals.data(), meshEdges.m_silhouetteNormals.size(), "normalPairs", idStr.c_str());
        }

    if (!meshEdges.m_polylines.empty())
        edgesValue["polylines"] = AddPolylines(meshEdges.m_polylines, maxIndex, "polyEdge", idStr);

    return edgesValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    DisplayParamsCR     displayParams = mesh.GetDisplayParams();

    if (!mesh.Params().empty() && displayParams.IsTextured())
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (mesh.Params().data(), mesh.Params().size(), "Param", idStr.c_str());

    BeAssert(displayParams.IgnoresLighting() || !mesh.Normals().empty());

    if (!mesh.Normals().empty() && !displayParams.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"]  = AddNormals(mesh.Normals().data(), mesh.Normals().size(), "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshTriangleIndices ("Indices", mesh.Triangles(), idStr, mesh.Points().size());

    if (mesh.GetEdges().IsValid())
        primitiveJson["edges"] = CreateMeshEdges(*mesh.GetEdges(), mesh.Points().size(), idStr);

#ifdef NOTNOW_DGNCLIENFX_AUXDATA_IMPLEMENTATION
    if (mesh.GetAuxData().m_displacementChannel.IsValid())
        {
        Json::Value     displacementValues = Json::arrayValue;
        uint32_t        index = 0;

        for (auto& data : mesh.GetAuxData().m_displacementChannel->GetData())
            {
            Json::Value     dataValue = Json::objectValue;                       

            dataValue["input"] = data->GetInput();
            dataValue["values"] = AddPointAttribute(data->GetValues().data(), data->GetValues().size(), "AuxDisp", Utf8PrintfString("%s%u", idStr.c_str(), index++));
            displacementValues.append(std::move(dataValue));
            }
        primitiveJson["attributes"]["AUXDISPLACEMENTS"] = std::move(displacementValues);
        }

    if (mesh.GetAuxData().m_paramChannel.IsValid())
        {
        Json::Value     paramValues = Json::arrayValue;
        uint32_t        index = 0;

        for (auto& data : mesh.GetAuxData().m_paramChannel->GetData())
            {
            Json::Value     dataValue = Json::objectValue;                       

            dataValue["input"] = data->GetInput();
            dataValue["values"] = AddParamAttribute(data->GetValues().data(), data->GetValues().size(), "AuxParam", Utf8PrintfString("%s%u", idStr.c_str(), index++));
            paramValues.append(std::move(dataValue));
            }
        primitiveJson["attributes"]["AUXPARAMS"] = std::move(paramValues);
        }
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreatePolylines(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    primitiveJson["indices"] = AddPolylines(mesh.Polylines(), mesh.Points().size(), "polyline", idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  CreateDisplayParamJson(Json::Value& matJson, MeshCR mesh,  DisplayParamsCR displayParams, Utf8StringCR suffix) 
    {
    matJson["type"] = (uint8_t) displayParams.GetType();
	
    // GeomParams...
    if (displayParams.GetCategoryId().IsValid())
        matJson["categoryId"] = displayParams.GetCategoryId().ToHexStr();
    
    if (displayParams.GetSubCategoryId().IsValid())
        matJson["subCategoryId"] = displayParams.GetSubCategoryId().ToHexStr();

    // ###TODO: Support non-persistent materials if/when necessary...
    auto material = displayParams.GetSurfaceMaterial().GetMaterial();
    if (nullptr != material && material->GetKey().IsPersistent())
        {
        matJson["materialId"] = material->GetKey().GetId().ToHexStr();
        AddMaterialJson(*material);
        }

    matJson["class"] = (uint16_t) displayParams.GetClass();
    matJson["ignoreLighting"] = displayParams.IgnoresLighting();

    // GraphicsParams...
    matJson["fillColor"] = displayParams.GetFillColor();
    matJson["fillFlags"] = static_cast<uint32_t> (displayParams.GetFillFlags());

    // Are these needed for meshes (Edges??)
    matJson["lineColor"]  = displayParams.GetLineColor();     // Edges?
    matJson["lineWidth"]  = displayParams.GetLineWidth();
    matJson["linePixels"] = (uint32_t) displayParams.GetLinePixels();     // Edges?

    if (nullptr != displayParams.GetSurfaceMaterial().GetGradient())
        matJson["gradient"] = displayParams.GetSurfaceMaterial().GetGradient()->ToJson();

    TextureCP texture = displayParams.GetSurfaceMaterial().GetTextureMapping().GetTexture();
    if (nullptr != texture)
        {
        if (texture->GetKey().IsValid() && SUCCESS != AddTextureJson(displayParams.GetSurfaceMaterial().GetTextureMapping(), matJson))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nate.Rex        06/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AddMaterialJson(Render::Material material)
    {
    BeAssert(material.GetKey().IsValid());	// Assume that each persistent material will contain an id
    Utf8String id = material.GetKey().GetId().ToHexStr();

    if (!m_json.isMember("renderMaterials") || !m_json["renderMaterials"].isMember(id))
        {
        Json::Value& materialJson = m_json["renderMaterials"][id];

        // Add texture contained by material.
        if (material.HasTextureMapping())
            {
            Json::Value materialTextureJson;
            AddTextureJson(material.GetTextureMapping(), materialTextureJson);	// This will cause it to get added to "namedTextures" as well
            materialJson["textureMapping"] = materialTextureJson;
            }

        RenderMaterialCPtr matElem = RenderMaterial::Get(m_model.GetDgnDb(), material.GetKey().GetId());
        RenderingAssetCP asset = matElem.IsValid() ? &matElem->GetRenderingAsset() : nullptr;
        BeAssert(asset != nullptr);
        if (asset != nullptr)
            {
            if (asset->GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_Color);
                materialJson["diffuseColor"][0] = rgb.red;
                materialJson["diffuseColor"][1] = rgb.green;
                materialJson["diffuseColor"][2] = rgb.blue;
                }
            if (asset->GetBool(RENDER_MATERIAL_FlagHasSpecularColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_SpecularColor);
                materialJson["specularColor"][0] = rgb.red;
                materialJson["specularColor"][1] = rgb.green;
                materialJson["specularColor"][2] = rgb.blue;
                }
            if (asset->GetBool(RENDER_MATERIAL_FlagHasFinish, false))
                {
                materialJson["specularExponent"] = asset->GetDouble(RENDER_MATERIAL_Finish, Material::Defaults::SpecularExponent());
                }
            if (asset->GetBool(RENDER_MATERIAL_FlagHasTransmit, false))
                materialJson["transparency"] = asset->GetDouble(RENDER_MATERIAL_Transmit, 0.0);

            if (asset->GetBool(RENDER_MATERIAL_FlagHasDiffuse, false))
                materialJson["diffuse"] = asset->GetDouble(RENDER_MATERIAL_Diffuse, Material::Defaults::Diffuse());

            double specular;
            if (asset->GetBool(RENDER_MATERIAL_FlagHasSpecular, false))
                specular = asset->GetDouble(RENDER_MATERIAL_Specular, Material::Defaults::Specular());
            else
                specular = 0.0;     // Lack of specular means 0.0 -- not default (painting overspecular in PhotoRealistic Rendering
            materialJson["specular"] = specular;

            if (asset->GetBool(RENDER_MATERIAL_FlagHasReflect, false))
                {
                // Reflectance stored as fraction of specular in V8 material settings.
                materialJson["reflect"] = specular * asset->GetDouble(RENDER_MATERIAL_Reflect, Material::Defaults::Reflect());
                }

            if (asset->GetBool(RENDER_MATERIAL_FlagHasReflectColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_ReflectColor);
                materialJson["reflectColor"][0] = rgb.red;
                materialJson["reflectColor"][1] = rgb.green;
                materialJson["reflectColor"][2] = rgb.blue;
                }


            if (asset->GetBool(RENDER_MATERIAL_FlagHasRefract, false))
                materialJson["refract"] = asset->GetDouble(RENDER_MATERIAL_Refract, RenderingAsset::Default::Refract());
            materialJson["shadows"] = !(asset->GetBool(RENDER_MATERIAL_FlagNoShadows, false));
            materialJson["ambient"] = asset->GetDouble(RENDER_MATERIAL_Ambient, RenderingAsset::Default::Ambient());
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Paul.Connelly   01/18
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AddTextureJson(TextureMappingCR mapping, Json::Value& matJson)
    {
    // NB: I am specifically not using the same representation we use for textures in Cesium tiles because
    // it includes much extra data and indirection which we don't need; and doesn't include some of
    // the mapping params we need.
    BeAssert(mapping.IsValid());
    TextureCR texture = *mapping.GetTexture();

    // Identifier will be name for named textures, and hex id string for persistent textures (if we are supposed to embed persistents)
    Utf8String name = "";
    if (texture.GetKey().IsNamed())
        {
        name = texture.GetKey().GetName();
        }
    else
        {
        name = texture.GetKey().GetId().ToHexStr();
        }

    if (!m_json.isMember("namedTextures") || !m_json["namedTextures"].isMember(name))
        {
        ImageSourceCP img = texture.GetImageSource();
        if (nullptr == img || !img->IsValid())
            {
            BeAssert(false);
            return ERROR;
            }

        AddBufferView(name.c_str(), img->GetByteStream());

        Json::Value& json = m_json["namedTextures"][name];
        json["format"] = static_cast<uint32_t>(img->GetFormat());
        json["bufferView"] = name;
        json["isGlyph"] = texture.IsGlyph();

        auto dimensions = texture.GetDimensions();
        json["width"] = dimensions.width;
        json["height"] = dimensions.height;
        }

    auto const& params = mapping.GetParams();
    Json::Value& paramsJson = matJson["texture"]["params"];
    paramsJson["mode"] = static_cast<uint32_t>(params.m_mapMode);
    paramsJson["weight"] = params.m_textureWeight;
    paramsJson["worldMapping"] = params.m_worldMapping;
    for (uint32_t i = 0; i < 2; i++)
        for (uint32_t j = 0; j < 3; j++)
            paramsJson["transform"][i][j] = params.m_textureMat2x3.m_val[i][j];

    matJson["texture"]["name"] = name;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Ray.Bentley     06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
void AddFeatures (MeshCR mesh, Json::Value& primitiveJson, Utf8StringCR idStr)
    {
    FeatureIndex    featureIndex;

    mesh.ToFeatureIndex(featureIndex);
    if(featureIndex.IsEmpty())
        BeAssert(false && "Empty feature index");
    else if (featureIndex.IsUniform())
        primitiveJson["featureID"]  = featureIndex.m_featureID;
    else
        primitiveJson["featureIDs"] = AddMeshIndices("featureIDs", featureIndex.m_featureIDs, mesh.Points().size(), idStr, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteFeatureTable(FeatureTableCR featureTable)
    {
    uint32_t      startPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.

    m_buffer.Append(featureTable.GetMaxFeatures());
    m_buffer.Append((uint32_t) featureTable.size());
    for (auto& feature : featureTable)
        {
        m_buffer.Append(feature.first.GetElementId().GetValue());
        m_buffer.Append(feature.first.GetSubCategoryId().GetValue());
        m_buffer.Append(static_cast<uint32_t> (feature.first.GetClass()));
        m_buffer.Append(feature.second);
        }
    WriteLength(startPosition, startPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index)
    { 
    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         materialJson = Json::objectValue, primitiveJson = Json::objectValue;

    if (SUCCESS != CreateDisplayParamJson(materialJson, mesh, mesh.GetDisplayParams(), idStr))
        return ERROR;

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(mesh.Points().data(), mesh.Points().size(), mesh.Verts().GetParams(), "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    if (!mesh.GetColorTable().IsUniform())
        AddMeshUInt16Attributes(primitiveJson, mesh.Colors().data(), mesh.Colors().size(), idStr, "ColorIndex_", "_COLORINDEX");

    AddFeatures (mesh, primitiveJson, idStr);
    AddMeshPointRange(m_json["accessors"][accPositionId], mesh.Verts().GetParams().GetRange());
 
    primitiveJson["colorTable"] = CreateColorTable(mesh.GetColorTable());
    primitiveJson["type"] = (uint32_t) mesh.GetType();
    primitiveJson["isPlanar"] = mesh.IsPlanar();

    if ((!mesh.Triangles().Empty() && SUCCESS == CreateTriMesh(primitiveJson, mesh, idStr)) ||
        (!mesh.Polylines().empty() && SUCCESS == CreatePolylines(primitiveJson, mesh, idStr)))
        {
        Utf8String  materialName = "Material" + idStr;
        m_json["materials"][materialName] = materialJson;
        primitiveJson["material"] = materialName;
        primitivesNode.append(primitiveJson);

        return SUCCESS;
        }

    return ERROR;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshes(Render::Primitives::GeometryCollectionCR geometry)
    {
    Json::Value     primitives = Json::arrayValue;
    size_t          primitiveIndex = 0;

    for (auto& geomMesh : geometry.Meshes())
        AddMesh(primitives, *geomMesh, primitiveIndex);

    AddPrimitivesJson(primitives);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteTile(ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, bool isLeaf, double const* zoomFactor)
    {
    uint32_t        startPosition = m_buffer.GetSize();
    DgnTile::Flags  flags = isLeaf ? DgnTile::Flags::IsLeaf : DgnTile::Flags::None;

    if (nullptr != zoomFactor)
        flags |= DgnTile::Flags::HasZoomFactor;

    if (geometry.ContainsCurves())
        flags |=  DgnTile::Flags::ContainsCurves;

    if (!geometry.IsComplete())
        flags |= DgnTile::Flags::Incomplete;

    m_buffer.Append(Format::Dgn);
    m_buffer.Append(DgnTile::Version);
    m_buffer.Append(static_cast<uint32_t>(flags));
    m_buffer.Append(contentRange);
    m_buffer.Append(nullptr != zoomFactor ? *zoomFactor : 0.0);

    uint32_t    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.
    WriteFeatureTable(geometry.Meshes().FeatureTable());
    PadToBoundary ();

    AddMeshes(geometry);
    if (SUCCESS != WriteGltf ())
        return ERROR;

    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);

    return SUCCESS;
    }
};  // DgnCacheTileWriter

END_TILE_IO_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Tile::IO::WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, bool isLeaf, double const* zoomFactor)
    {
    return Tile::IO::DgnCacheTileWriter(streamBuffer, model).WriteTile(contentRange, geometry, isLeaf, zoomFactor);
    }

