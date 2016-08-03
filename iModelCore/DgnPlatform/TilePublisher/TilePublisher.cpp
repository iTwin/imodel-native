/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include "Constants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;

#ifdef TODO_TEXTURES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void convertToJpeg(bvector<uint8_t>& image, int width, int height)
    {
    // ###TODO: Image => ImageSource...
    // ###TODO: Use ByteStream, not bvector...
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TextureCache::PrepareMeshTextures(TileMeshList& meshes, bvector<uint32_t>& texIds, TileGeometryCacheCR geometryCache)
    {
    BeMutexHolder lock (m_mutex);

    for (auto& mesh : meshes)
        {
        auto matSymb = mesh->GetGraphicParams();
        TextureKey key(matSymb);

        uint32_t textureId = -1;
        auto found = m_textureMap.find(key);
        if (m_textureMap.end() != found)
            {
            textureId = found->second;
            }
        else
            {
#ifdef TODO_TEXTURES
            bvector<uint8_t>   image;
            Point2d         imageSize;

            if (NULL != matSymb && SUCCESS == geometryCache.GetTextureImage (image, imageSize, *matSymb))
                {
                convertToJpeg(image, imageSize.x, imageSize.y);
                textureId = static_cast<uint32_t>(m_textures.size());
                m_textures.push_back(Texture(std::move(image), static_cast<uint32_t>(imageSize.x), static_cast<uint32_t>(imageSize.y)));
                }
#endif

            m_textureMap[key] = textureId;
            }

        texIds.push_back(textureId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
BatchIdMap::BatchIdMap()
    {
    // "no element" always maps to the first batch table index
    GetBatchId(DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t BatchIdMap::GetBatchId(DgnElementId elemId)
    {
    auto found = m_map.find(elemId);
    if (m_map.end() == found)
        {
        auto batchId = static_cast<uint16_t>(m_list.size());
        if (batchId == 0xffff)
            return 0;   // ###TODO: avoid hitting this limit...

        m_list.push_back(elemId);
        found = m_map.insert(bmap<DgnElementId, uint16_t>::value_type(elemId, batchId)).first;
        }

    return found->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchIdMap::ToJson(Json::Value& value) const
    {
    Json::Value elementIds(Json::arrayValue);
    for (auto elemIter = m_list.begin(); elemIter != m_list.end(); ++elemIter)
        elementIds.append(elemIter->GetValueUnchecked());   // ###TODO: Convert to string...javascript doesn't support 64-bit integers...

    value["element"] = elementIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::TilePublisher(TileNodeCR tile, TilesetPublisher& context)
    : m_centroid(GetCentroid(tile)), m_tile(tile), m_context(context)
    {
#define CESIUM_RTC_ZERO
#ifdef CESIUM_RTC_ZERO
    m_centroid = DPoint3d::FromXYZ(0,0,0);
#endif

    TileGeometryCacheR geomCache = GetGeometryCache();
    m_meshes = m_tile.GenerateMeshes(geomCache, m_tile.GetTolerance(), TileGeometry::NormalMode::Always, true);
    GetTextureCache().PrepareMeshTextures(m_meshes, m_textureIds, geomCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d TilePublisher::GetCentroid(TileNodeCR tile)
    {
    DRange3dCR range = tile.GetRange();
    return DPoint3d::FromXYZ(range.low.x + range.XLength()*0.5,
                             range.low.y + range.YLength()*0.5,
                             range.low.z + range.ZLength()*0.5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AppendUInt32(uint32_t value)
    {
    m_outputFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBoundingVolume(Json::Value& val, TileNodeCR tile)
    {
    // Range identified by center point and axes
    // Axes are relative to origin of box, not center
    DPoint3d center = GetCentroid(tile);
    DPoint3d hi = tile.GetRange().high;
    hi.Subtract(tile.GetRange().low);

    static double      s_minSize = .001;   // Meters...  Don't allow degenerate box.

    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    AppendPoint(box, center);
    AppendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, hi.x), 0.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, hi.y), 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, hi.z)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteMetadata(Json::Value& val, TileNodeCR tile, double tolerance, WStringCR b3dmPath, const bvector<WString>& childrenFilePaths)
    {
    val["asset"]["version"] = "0.0";
    val[JSON_GeometricError] = tile.GetTolerance();
    val["refine"] = "replace";

    static double       s_toleranceRatio = 1.0;

    auto& root = val[JSON_Root];
    root[JSON_GeometricError] = tile.GetTolerance() * s_toleranceRatio;
    WriteBoundingVolume(root, tile);

    root[JSON_Content]["url"] = Utf8String(BeFileName::GetFileNameAndExtension(b3dmPath.c_str()).c_str()).c_str();
    if (!childrenFilePaths.empty())
        root[JSON_Children] = Json::arrayValue;

    for (auto& path : childrenFilePaths)
        {
        Json::Value child;
        TileNodeCR childTile = tile.GetChildren()[&path - &childrenFilePaths.front()];

        child[JSON_Content]["url"] = Utf8String(BeFileName::GetFileNameAndExtension(path.c_str()).c_str()).c_str();
        child[JSON_GeometricError] = childTile.GetTolerance();
        WriteBoundingVolume(child, childTile);

        root[JSON_Children].append(child);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddTechniqueParameter(Json::Value& technique, Utf8CP name, int type, Utf8CP semantic)
    {
    auto& param = technique["parameters"][name];
    param["type"] = type;
    if (nullptr != semantic)
        param["semantic"] = semantic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AppendProgramAttribute(Json::Value& program, Utf8CP attrName)
    {
    Json::Value obj;
    obj[attrName] = Json::objectValue;
    program["attributes"].append(obj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddShader(Json::Value& shaders, Utf8CP name, int type, Utf8CP buffer)
    {
    auto& shader = (shaders[name] = Json::objectValue);
    shader["type"] = type;
    shader["extensions"]["khr_binary_gltf"]["bufferview"] = buffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void TilePublisher::AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData)
    {
    auto bufferDataSize = bufferData.size() * sizeof(bufferData[0]);
    auto& view = (views[name] = Json::objectValue);
    view["buffer"] = "binary_glTF";
    view["byteOffset"] = m_binaryData.size();
    view["byteLength"] = bufferDataSize;

    size_t binaryDataSize = m_binaryData.size();
    m_binaryData.resize(binaryDataSize + bufferDataSize);
    memcpy(m_binaryData.data(), bufferData.data(), bufferDataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilePublisher::Publish()
    {
    BeFileName b3dmPath(nullptr, GetDataDirectory().c_str(), (GetPrefix() + GetNodeNameSuffix(m_tile)).c_str(), L"b3dm");

    // .json file
    bvector<WString> childrenPaths;
    for (auto const& child : m_tile.GetChildren())
        childrenPaths.push_back(GetTileMetadataName(child));

    Json::Value val;
    WriteMetadata(val, m_tile, m_tile.GetTolerance(), b3dmPath, childrenPaths);

    Utf8String metadataStr = Json::FastWriter().write(val);

    WString metadataPath = GetTileMetadataName(m_tile);
    m_outputFile.open(Utf8String(metadataPath.c_str()).c_str(), std::ios_base::trunc);
    m_outputFile.write(metadataStr.data(), metadataStr.size());
    m_outputFile.close();

    // .b3dm file
    Json::Value sceneJson(Json::objectValue);
    ProcessMeshes(sceneJson);
    Utf8String sceneStr = Json::FastWriter().write(sceneJson);

#if USE_BATCH_TABLE
    Json::Value batchTableJson(Json::objectValue);
    m_batchIds.ToJson(batchTableJson);
    Utf8String batchTableStr = Json::FastWriter().write(batchTableJson);
    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
#else
    uint32_t batchTableStrLen = 0;
#endif

    m_outputFile.open(Utf8String(b3dmPath.c_str()).c_str(), std::ios_base::trunc | std::ios_base::binary);

    // GLTF header = 5 32-bit values
    static const size_t s_gltfHeaderSize = 20;
    static const char s_gltfMagic[] = "glTF";
    static const uint32_t s_gltfVersion = 1;
    static const uint32_t s_gltfSceneFormat = 0;
    uint32_t sceneStrLength = static_cast<uint32_t>(sceneStr.size());
    uint32_t gltfLength = s_gltfHeaderSize + sceneStrLength + m_binaryData.GetSize();

    // B3DM header = 5 32-bit values
    // Header immediately followed by batch table json
    static const size_t s_b3dmHeaderSize = 20;
    static const char s_b3dmMagic[] = "b3dm";
    static const uint32_t s_b3dmVersion = 1;
    uint32_t b3dmNumBatches = m_batchIds.Count();
    uint32_t b3dmLength = gltfLength + s_b3dmHeaderSize + batchTableStrLen;

    m_outputFile.write(s_b3dmMagic, 4);
    AppendUInt32(s_b3dmVersion);
    AppendUInt32(b3dmLength);
    AppendUInt32(b3dmNumBatches);
    AppendUInt32(batchTableStrLen);

#if USE_BATCH_TABLE
    m_outputFile.write(batchTableStr.data(), batchTableStrLen);
#endif

    m_outputFile.write(s_gltfMagic, 4);
    AppendUInt32(s_gltfVersion);
    AppendUInt32(gltfLength);
    AppendUInt32(sceneStrLength);
    AppendUInt32(s_gltfSceneFormat);

    m_outputFile.write(sceneStr.data(), sceneStrLength);
    if (!m_binaryData.empty())
        m_outputFile.write(reinterpret_cast<const char*>(m_binaryData.data()), m_binaryData.size());

    m_outputFile.close();

    return TilesetPublisher::Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::ProcessMeshes(Json::Value& val)
    {
    if (m_meshes.empty())
        {
        val["scene"] = "defaultScene";
        val["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
        val["nodes"]["node_0"] = Json::objectValue;
        val["nodes"]["node_0"]["name"] = "";

        return;
        }

    AddExtensions(val);

    TextureIdToNameMap texNames;
    AddTextures(val, texNames);
    AddShaders(val, !texNames.empty());

    val["meshes"]["mesh_0"]["primitives"] = Json::arrayValue;
    for (size_t i = 0; i < m_meshes.size(); i++)
        AddMesh(val, *m_meshes[i], i, m_textureIds[i], texNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddExtensions(Json::Value& rootNode)
    {
    rootNode["extensionsUsed"] = Json::arrayValue;
    rootNode["extensionsUsed"].append("KHR_binary_glTF");
    rootNode["extensionsUsed"].append("CESIUM_RTC");

    rootNode["glExtensionsUsed"] = Json::arrayValue;
    rootNode["glExtensionsUsed"].append("OES_element_index_uint");

    rootNode["extensions"]["CESIUM_RTC"]["center"] = Json::arrayValue;
    rootNode["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.x);
    rootNode["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.y);
    rootNode["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.z);

    rootNode["scene"] = "defaultScene";
    rootNode["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    rootNode["scenes"]["defaultScene"]["nodes"].append("node_0");
    rootNode["nodes"]["node_0"]["meshes"] = Json::arrayValue;
    rootNode["nodes"]["node_0"]["meshes"].append("mesh_0");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddTextures(Json::Value& rootNode, TextureIdToNameMap& texNames)
    {
    uint32_t material_id = 0;

    // NB: Could move this inside the loop but the expense of repeatedly entering and leaving the critical section likely to outweight any
    // potential contention...
    BeMutexHolder lock(GetTextureCache().GetMutex());

    for (auto& tex : m_textureIds)
        {
        if (tex != -1)
            {
            texNames[tex] = Utf8PrintfString("mat_%d", (int)material_id);

            Utf8String textureId = (std::string("tex_") + std::to_string(material_id)).c_str();
            Utf8String materialId = (std::string("mat_") + std::to_string(material_id)).c_str();
            Utf8String imageId = (std::string("img_") + std::to_string(material_id)).c_str();
            Utf8String bvImageId = (std::string("bv_img_") + std::to_string(material_id)).c_str();

            rootNode["materials"][materialId] = Json::objectValue;
            rootNode["materials"][materialId]["technique"] = "tech_1";
            rootNode["materials"][materialId]["values"]["tex"] = textureId.c_str();

            rootNode["textures"][textureId] = Json::objectValue;
            rootNode["textures"][textureId]["format"] = GLTF_RGB;
            rootNode["textures"][textureId]["internalFormat"] = GLTF_RGB;
            rootNode["textures"][textureId]["sampler"] = "sampler_0";
            rootNode["textures"][textureId]["source"] = imageId;

            rootNode["images"][imageId] = Json::objectValue;
            rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
            rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
            rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = "image/jpeg";

            auto pTex = GetTextureCache().GetTextureJPEG(tex);
            BeAssert(nullptr != pTex);

            rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = pTex->GetHeight();
            rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = pTex->GetWidth();

            auto const& texData = pTex->GetData();

            rootNode["bufferViews"][bvImageId] = Json::objectValue;
            rootNode["bufferViews"][bvImageId]["buffer"] = "binary_glTF";
            rootNode["bufferViews"][bvImageId]["byteOffset"] = m_binaryData.size();
            rootNode["bufferViews"][bvImageId]["byteLength"] = texData.size();

            size_t current_buffer_size = m_binaryData.size();
            m_binaryData.resize(m_binaryData.size() + texData.size());
            memcpy(m_binaryData.data() + current_buffer_size, texData.data(), texData.size());
            material_id++;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddShaders(Json::Value& rootNode, bool isTextured)
    {
    auto& tech0 = (rootNode["techniques"]["tech_0"] = Json::objectValue);
    AddTechniqueParameter(tech0, "color", GLTF_FLOAT_VEC4, nullptr);
    AddTechniqueParameter(tech0, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(tech0, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(tech0, "pos", GLTF_FLOAT_VEC3, "POSITION");
    AddTechniqueParameter(tech0, "n", GLTF_FLOAT_VEC3, "NORMAL");
    AddTechniqueParameter(tech0, "nmx", GLTF_FLOAT_MAT3, "MODELVIEWINVERSETRANSPOSE");
#if USE_BATCH_TABLE
    AddTechniqueParameter(tech0, "batch", GLTF_FLOAT, "BATCHID");
#endif
    tech0["program"] = "prog_0";

    tech0["states"]["enable"] = Json::arrayValue;
    tech0["states"]["enable"].append(GLTF_DEPTH_TEST);
    tech0["states"]["enable"].append(GLTF_CULL_FACE);

    auto& tech0Attributes = tech0["attributes"];
    tech0Attributes["a_pos"] = "pos";
    tech0Attributes["a_n"] = "n";
#if USE_BATCH_TABLE
    tech0Attributes["a_batchId"] = "batch";
#endif

    auto& tech0Uniforms = tech0["uniforms"];
    tech0Uniforms["u_color"] = "color";
    tech0Uniforms["u_mv"] = "mv";
    tech0Uniforms["u_proj"] = "proj";
    tech0Uniforms["u_nmx"] = "nmx";

    auto& prog0 = (rootNode["programs"]["prog_0"] = Json::objectValue);
    prog0["attributes"] = Json::arrayValue;
    AppendProgramAttribute(prog0, "a_pos");
    AppendProgramAttribute(prog0, "a_n");
#if USE_BATCH_TABLE
    AppendProgramAttribute(prog0, "a_batchId");
#endif

    prog0["vertexShader"] = "vs";
    prog0["fragmentShader"] = "fs";

    auto& shaders = rootNode["shaders"];
    AddShader(shaders, "vs", GLTF_VERTEX_SHADER, "bv_vs");
    AddShader(shaders, "fs", GLTF_FRAGMENT_SHADER, "bv_fs");

    auto& bufferViews = rootNode["bufferViews"];
    AddBufferView(bufferViews, "bv_vs", s_untexturedVertexShader);
    AddBufferView(bufferViews, "bv_fs", s_untexturedFragShader);

    if (isTextured)
        {
        rootNode["samplers"]["sampler_0"] = Json::objectValue;
        rootNode["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;

        auto& tech1 = (rootNode["techniques"]["tech_1"] = Json::objectValue);
        AddTechniqueParameter(tech1, "tex", GLTF_SAMPLER_2D, nullptr);
        AddTechniqueParameter(tech1, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
        AddTechniqueParameter(tech1, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
        AddTechniqueParameter(tech1, "pos", GLTF_FLOAT_VEC3, "POSITION");
        AddTechniqueParameter(tech1, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");
#if USE_BATCH_TABLE
        AddTechniqueParameter(tech1, "batch", GLTF_FLOAT, "BATCHID");
#endif
        tech1["program"] = "prog_1";

        tech1["states"]["enable"] = Json::arrayValue;
        tech1["states"]["enable"].append(GLTF_DEPTH_TEST);
        tech1["states"]["enable"].append(GLTF_CULL_FACE);

        auto& tech1Attributes = tech1["attributes"];
        tech1Attributes["a_pos"] = "pos";
        tech1Attributes["a_texc"] = "texc";

        auto& tech1Uniforms = tech1["uniforms"];
        tech1Uniforms["u_tex"] = "tex";
        tech1Uniforms["u_mv"] = "mv";
        tech1Uniforms["u_proj"] = "proj";

        auto& prog1 = (rootNode["programs"]["prog_1"] = Json::objectValue);
        AppendProgramAttribute(prog1, "a_pos");
        AppendProgramAttribute(prog1, "a_texc");
#if USE_BATCH_TABLE
        AppendProgramAttribute(prog1, "a_batchId");
#endif

        prog1["vertexShader"] = "vs_tex";
        prog1["fragmentShader"] = "fs_tex";

        auto& prog1Shaders = rootNode["shaders"];
        AddShader(prog1Shaders, "vs_tex", GLTF_VERTEX_SHADER, "bv_vs_tex");
        AddShader(prog1Shaders, "fs_tex", GLTF_FRAGMENT_SHADER, "bv_fs_tex");

        AddBufferView(bufferViews, "bv_vs_tex", s_texturedVertexShader);
        AddBufferView(bufferViews, "bv_fs_tex", s_texturedFragShader);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMesh(Json::Value& rootNode, TileMeshR mesh, size_t id, uint32_t texId, TextureIdToNameMap& texNames)
    {
    Utf8String idStr(std::to_string(id).c_str());
    Utf8String bv_pos_id    = Concat("bv_pos_", idStr),
               bv_ind_id    = Concat("bv_ind_", idStr),
               bv_uv_id     = Concat("bv_uv_", idStr),
               bv_n_id      = Concat("bv_n_", idStr),
#if USE_BATCH_TABLE
               bv_bat_id    = Concat("bv_bat_", idStr),
#endif
               acc_pos_id   = Concat("acc_pos_", idStr),
               acc_ind_id   = Concat("acc_ind_", idStr),
               acc_uv_id    = Concat("acc_uv_", idStr),
               acc_n_id     = Concat("acc_n_", idStr)
#if USE_BATCH_TABLE
               ,acc_bat_id  = Concat("acc_bat_", idStr);
#else
               ;
#endif

    bvector<float> ptsVal;
    ptsVal.reserve(mesh.Points().size() * 3);
    for (auto const& pt : mesh.Points())
        {
        DPoint3d ptTrans = pt;
        ptTrans.DifferenceOf(ptTrans, m_centroid);
        ptsVal.push_back((float)ptTrans.x);
        ptsVal.push_back((float)ptTrans.y);
        ptsVal.push_back((float)ptTrans.z);
        }

    bvector<unsigned int> indices;
    indices.reserve(mesh.Triangles().size() * 3);
    for (auto const& tri : mesh.Triangles())
        {
        indices.push_back((unsigned int)tri.m_indices[0]);
        indices.push_back((unsigned int)tri.m_indices[1]);
        indices.push_back((unsigned int)tri.m_indices[2]);
        }

    bvector<float> uvs;
    uvs.reserve(mesh.Params().size() * 2);
    for (auto const& uv : mesh.Params())
        {
        uvs.push_back((float)uv.x);
        uvs.push_back((float)(1.0 - uv.y));         // Ick... Either our images are flipped or are v- convention is mismatched.
        }

    bvector<float> normals;
    normals.reserve(mesh.Normals().size() * 3);
    for (auto const& norm : mesh.Normals())
        {
        normals.push_back((float)norm.x);
        normals.push_back((float)norm.y);
        normals.push_back((float)norm.z);
        }

#if USE_BATCH_TABLE
    bvector<uint16_t> batchIds;
    batchIds.reserve(mesh.ElementIds().size());
    for (auto const& elemId : mesh.ElementIds())
        batchIds.push_back(m_batchIds.GetBatchId(elemId));
#endif

    Json::Value attr = Json::objectValue;
    attr["attributes"]["POSITION"] = acc_pos_id;
    if (texId != -1)
        attr["attributes"]["TEXCOORD_0"] = acc_uv_id;
    else
        attr["attributes"]["NORMAL"] = acc_n_id;

#if USE_BATCH_TABLE
    attr["attributes"]["BATCHID"] = acc_bat_id;
#endif
    attr["indices"] = acc_ind_id;

    int baseMatId = static_cast<int>(GetTextureCache().Count()); //make sure untextured materials work when you also have texture
    if (texId == -1)
        {
        Utf8String matName = Utf8String(WPrintfString(L"mat_%d", (int)id+baseMatId).c_str());
        rootNode["materials"][matName.c_str()] = Json::objectValue;
        rootNode["materials"][matName.c_str()]["technique"] = "tech_0";
        rootNode["materials"][matName.c_str()]["values"]["color"] = Json::arrayValue;
        uint32_t rgb = (NULL == mesh.GetGraphicParams()) ? 0 : mesh.GetGraphicParams()->GetFillColor().GetValue();
        rootNode["materials"][matName.c_str()]["values"]["color"].append(((uint8_t*)&rgb)[0]/255.0);
        rootNode["materials"][matName.c_str()]["values"]["color"].append(((uint8_t*)&rgb)[1]/255.0);
        rootNode["materials"][matName.c_str()]["values"]["color"].append(((uint8_t*)&rgb)[2]/255.0);
        double alpha = 1.0 - ((uint8_t*)&rgb)[3]/255.0;
        rootNode["materials"][matName.c_str()]["values"]["color"].append(alpha);
        attr["material"] = matName.c_str();
        }
    else
        {
        attr["material"] = texNames[texId];
        }

    attr["mode"] = GLTF_TRIANGLES;
    rootNode["meshes"]["mesh_0"]["primitives"].append(attr);

    rootNode["bufferViews"][bv_pos_id] = Json::objectValue;
    rootNode["bufferViews"][bv_pos_id]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bv_pos_id]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bv_pos_id]["byteLength"] = ptsVal.size() * sizeof(float);
    rootNode["bufferViews"][bv_pos_id]["target"] = GLTF_ARRAY_BUFFER;

    size_t current_buffer_size = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + ptsVal.size() *sizeof(float));
    memcpy(m_binaryData.data() + current_buffer_size, ptsVal.data(), ptsVal.size() * sizeof(float));

    rootNode["bufferViews"][bv_ind_id] = Json::objectValue;
    rootNode["bufferViews"][bv_ind_id]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bv_ind_id]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bv_ind_id]["byteLength"] = indices.size() * sizeof(unsigned int);
    rootNode["bufferViews"][bv_ind_id]["target"] = GLTF_ELEMENT_ARRAY_BUFFER;

    current_buffer_size = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + indices.size() *  sizeof(unsigned int));
    memcpy(m_binaryData.data() + current_buffer_size, indices.data(), indices.size() *sizeof(unsigned int));

    rootNode["accessors"][acc_pos_id] = Json::objectValue;
    rootNode["accessors"][acc_pos_id]["bufferView"] = bv_pos_id;
    rootNode["accessors"][acc_pos_id]["byteOffset"] = 0;
    rootNode["accessors"][acc_pos_id]["componentType"] = GLTF_FLOAT;
    rootNode["accessors"][acc_pos_id]["count"] = ptsVal.size();
    rootNode["accessors"][acc_pos_id]["type"] = "VEC3";

#if USE_BATCH_TABLE
    auto nBatchIdBytes = batchIds.size() * sizeof(uint16_t);
    rootNode["bufferViews"][bv_bat_id] = Json::objectValue;
    rootNode["bufferViews"][bv_bat_id]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bv_bat_id]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bv_bat_id]["byteLength"] = nBatchIdBytes;
    rootNode["bufferViews"][bv_bat_id]["target"] = GLTF_ARRAY_BUFFER;

    current_buffer_size = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + nBatchIdBytes);
    memcpy(m_binaryData.data() + current_buffer_size, batchIds.data(), nBatchIdBytes);
#endif

    DRange3d range = DRange3d::From(mesh.Points().data(), (int)mesh.Points().size());

    rootNode["accessors"][acc_pos_id]["min"] = Json::arrayValue;
    rootNode["accessors"][acc_pos_id]["min"].append(range.low.x);
    rootNode["accessors"][acc_pos_id]["min"].append(range.low.y);
    rootNode["accessors"][acc_pos_id]["min"].append(range.low.z);
    rootNode["accessors"][acc_pos_id]["max"] = Json::arrayValue;
    rootNode["accessors"][acc_pos_id]["max"].append(range.high.x);
    rootNode["accessors"][acc_pos_id]["max"].append(range.high.y);
    rootNode["accessors"][acc_pos_id]["max"].append(range.high.z);

    rootNode["accessors"][acc_ind_id] = Json::objectValue;
    rootNode["accessors"][acc_ind_id]["bufferView"] = bv_ind_id;
    rootNode["accessors"][acc_ind_id]["byteOffset"] = 0;
    rootNode["accessors"][acc_ind_id]["componentType"] = GLTF_UINT32;
    rootNode["accessors"][acc_ind_id]["count"] = indices.size();
    rootNode["accessors"][acc_ind_id]["type"] = "SCALAR";

#if USE_BATCH_TABLE
    rootNode["accessors"][acc_bat_id] = Json::objectValue;
    rootNode["accessors"][acc_bat_id]["bufferView"] = bv_bat_id;
    rootNode["accessors"][acc_bat_id]["byteOffset"] = 0;
    rootNode["accessors"][acc_bat_id]["componentType"] = GLTF_UNSIGNED_SHORT;
    rootNode["accessors"][acc_bat_id]["count"] = batchIds.size();
    rootNode["accessors"][acc_bat_id]["type"] = "SCALAR";
#endif

    if (texId != -1) //textured mesh
        {
        rootNode["bufferViews"][bv_uv_id] = Json::objectValue;
        rootNode["bufferViews"][bv_uv_id]["buffer"] = "binary_glTF";
        rootNode["bufferViews"][bv_uv_id]["byteOffset"] = m_binaryData.size();
        rootNode["bufferViews"][bv_uv_id]["byteLength"] = uvs.size() * sizeof(float);
        rootNode["bufferViews"][bv_uv_id]["target"] = GLTF_ARRAY_BUFFER;

        current_buffer_size = m_binaryData.size();
        m_binaryData.resize(m_binaryData.size() + uvs.size() * sizeof(float));
        memcpy(m_binaryData.data() + current_buffer_size, uvs.data(), uvs.size() * sizeof(float));

        rootNode["accessors"][acc_uv_id] = Json::objectValue;
        rootNode["accessors"][acc_uv_id]["bufferView"] = bv_uv_id;
        rootNode["accessors"][acc_uv_id]["byteOffset"] = 0;
        rootNode["accessors"][acc_uv_id]["componentType"] = GLTF_FLOAT;
        rootNode["accessors"][acc_uv_id]["count"] = uvs.size();
        rootNode["accessors"][acc_uv_id]["type"] = "VEC2";
        }
    else
        {
        rootNode["bufferViews"][bv_n_id] = Json::objectValue;
        rootNode["bufferViews"][bv_n_id]["buffer"] = "binary_glTF";
        rootNode["bufferViews"][bv_n_id]["byteOffset"] = m_binaryData.size();
        rootNode["bufferViews"][bv_n_id]["byteLength"] = normals.size() * sizeof(float);
        rootNode["bufferViews"][bv_n_id]["target"] = GLTF_ARRAY_BUFFER;

        current_buffer_size = m_binaryData.size();
        m_binaryData.resize(m_binaryData.size() + normals.size() * sizeof(float));
        memcpy(m_binaryData.data() + current_buffer_size, normals.data(), normals.size() *  sizeof(float));

        rootNode["accessors"][acc_n_id] = Json::objectValue;
        rootNode["accessors"][acc_n_id]["bufferView"] = bv_n_id;
        rootNode["accessors"][acc_n_id]["byteOffset"] = 0;
        rootNode["accessors"][acc_n_id]["componentType"] = GLTF_FLOAT;
        rootNode["accessors"][acc_n_id]["count"] = normals.size();
        rootNode["accessors"][acc_n_id]["type"] = "VEC3";
        }
    rootNode["buffers"]["binary_glTF"]["byteLength"] = m_binaryData.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString TilePublisher::GetNodeNameSuffix(TileNodeCR node)
    {
    WString suffix;
    if (nullptr != node.GetParent())
        {
        suffix = WPrintfString(L"%02d", static_cast<int>(node.GetSiblingIndex()));
        for (auto parent = node.GetParent(); nullptr != parent->GetParent(); parent = parent->GetParent())
            suffix = WPrintfString(L"%02d", static_cast<int>(parent->GetSiblingIndex())) + suffix;
        }

    return suffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString TilePublisher::GetTileMetadataName(TileNodeCR node) const
    {
    return BeFileName(nullptr, GetDataDirectory().c_str(), (GetPrefix() + GetNodeNameSuffix(node)).c_str(), L"json");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TilesetPublisher::_AcceptTile(TileNodeCR tile)
    {
    if (Status::Success != m_acceptTileStatus)
        return TileGenerator::Status::Aborted;

    TilePublisher publisher(tile, *this);
    auto publisherStatus = publisher.Publish();
    switch (publisherStatus)
        {
        case Status::Success:
        case Status::NoGeometry:    // ok for tile to have no geometry
            break;
        default:
            m_acceptTileStatus = publisherStatus;
            break;
        }

    return ConvertStatus(publisherStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::TilesetPublisher(ViewControllerR view, BeFileNameCR outputDir, WStringCR tilesetName)
    : m_viewController(view), m_outputDir(outputDir), m_rootName(tilesetName), m_dataDir(m_outputDir)
    {
    // m_outputDir holds the .html file + shared scripts
    // m_dataDir = m_outputDir/m_rootName/ - holds the json + b3dm files
    m_outputDir.AppendSeparator();
    m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::Setup()
    {
    // Ensure directories exist and are writable
    if (BeFileNameStatus::Success != BeFileName::CheckAccess(m_outputDir, BeFileNameAccess::Write))
        return Status::CantWriteToBaseDirectory;

    bool dataDirExists = BeFileName::DoesPathExist(m_dataDir);
    if (dataDirExists && BeFileNameStatus::Success != BeFileName::EmptyDirectory(m_dataDir.c_str()))
        return Status::CantCreateSubDirectory;
    else if (!dataDirExists && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(m_dataDir))
        return Status::CantCreateSubDirectory;

    if (BeFileNameStatus::Success != BeFileName::CheckAccess(m_dataDir, BeFileNameAccess::Write))
        return Status::CantCreateSubDirectory;

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::ConvertStatus(TileGenerator::Status input)
    {
    switch (input)
        {
        case TileGenerator::Status::Success:    return Status::Success;
        case TileGenerator::Status::NoGeometry: return Status::NoGeometry;
        default: BeAssert(TileGenerator::Status::Aborted == input); return Status::Aborted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TilesetPublisher::ConvertStatus(Status input)
    {
    switch (input)
        {
        case Status::Success:       return TileGenerator::Status::Success;
        case Status::NoGeometry:    return TileGenerator::Status::NoGeometry;
        default:                    return TileGenerator::Status::Aborted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::WriteWebApp()
    {
    Utf8PrintfString html(s_viewerHtml, m_rootName.c_str(), m_rootName.c_str());
    BeFileName htmlFileName = m_outputDir;
    htmlFileName.AppendString(m_rootName.c_str()).AppendExtension(L"html");

    std::ofstream htmlFile;
    htmlFile.open(Utf8String(htmlFileName.c_str()).c_str(), std::ios_base::trunc);
    htmlFile.write(html.data(), html.size());
    htmlFile.close();

    // ###TODO: Symlink Cesium scripts, if not present

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::Publish()
    {
    auto status = Setup();
    if (Status::Success != status)
        return status;

    static bool s_convertToYUp = false;
    RotMatrix rot = s_convertToYUp ? RotMatrix::From2Vectors(DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0)) : RotMatrix::FromIdentity();
    Transform transformFromDgn = Transform::From(rot);

    ProgressMeter progressMeter(*this);
    TileGenerator generator(transformFromDgn, &progressMeter);

    static double s_toleranceInMeters = 0.01;
    status = ConvertStatus(generator.LoadGeometry(m_viewController, s_toleranceInMeters));
    if (Status::Success != status)
        return status;

    static const size_t s_maxPointsPerTile = 250000;
    TileNode rootNode;
    status = ConvertStatus(generator.GenerateTiles(rootNode, s_toleranceInMeters, s_maxPointsPerTile));
    if (Status::Success != status)
        return status;

    m_generator = &generator;
    status = ConvertStatus(generator.CollectTiles(rootNode, *this));
    m_generator = nullptr;

    if (Status::Success != status)
        return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;

    return WriteWebApp();
    }

