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
        elementIds.append(elemIter->ToString());    // NB: Javascript doesn't support full range of 64-bit integers...must convert to strings...

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
    m_meshes = m_tile._GenerateMeshes(geomCache, m_tile.GetTolerance(), TileGeometry::NormalMode::Always, true);
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
void TilePublisher::WriteMetadata(Json::Value& val, TileNodeCR tile, double tolerance, WStringCR b3dmPath)
    {
    val["asset"]["version"] = "0.0";
    val[JSON_GeometricError] = tile.GetTolerance();

    static double       s_toleranceRatio = 1.0;

    auto& root = val[JSON_Root];
    root["refine"] = "replace";
    root[JSON_GeometricError] = tile.GetTolerance() * s_toleranceRatio;
    WriteBoundingVolume(root, tile);

    if (0 == m_tile.GetDepth() && !m_context.GetTileToEcef().IsIdentity())
        {
        DMatrix4d   matrix  = DMatrix4d::From (m_context.GetTileToEcef());
        auto&       transformValue = val[JSON_Root][JSON_Transform];

        for (size_t i=0;i<4; i++)
            for (size_t j=0; j<4; j++)
                transformValue.append (matrix.coff[j][i]);
        }

    root[JSON_Content]["url"] = Utf8String(BeFileName::GetFileNameAndExtension(b3dmPath.c_str()).c_str()).c_str();
    if (!tile.GetChildren().empty())
        root[JSON_Children] = Json::arrayValue;

    for (auto& childTile : tile.GetChildren())
        {
        Json::Value         child;
        WString             path = GetTileMetadataName(childTile);

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
    shader["extensions"]["KHR_binary_glTF"]["bufferView"] = buffer;
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
    memcpy(m_binaryData.data() + binaryDataSize, bufferData.data(), bufferDataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilePublisher::Publish()
    {
    BeFileName  b3dmPath(nullptr, GetDataDirectory().c_str(), (GetPrefix() + GetNodeNameSuffix(m_tile)).c_str(), L"b3dm");
    Json::Value val;

    WriteMetadata(val, m_tile, m_tile.GetTolerance(), b3dmPath);

    Utf8String metadataStr = Json::FastWriter().write(val);

    WString metadataPath = GetTileMetadataName(m_tile);
    m_outputFile.open(Utf8String(metadataPath.c_str()).c_str(), std::ios_base::trunc);
    m_outputFile.write(metadataStr.data(), metadataStr.size());
    m_outputFile.close();

    // .b3dm file
    Json::Value sceneJson(Json::objectValue);
    ProcessMeshes(sceneJson);
    Utf8String sceneStr = Json::FastWriter().write(sceneJson);

    Json::Value batchTableJson(Json::objectValue);
    m_batchIds.ToJson(batchTableJson);
    Utf8String batchTableStr = Json::FastWriter().write(batchTableJson);
    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());

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
    m_outputFile.write(batchTableStr.data(), batchTableStrLen);

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

    val["meshes"]["mesh_0"]["primitives"] = Json::arrayValue;
    for (size_t i = 0; i < m_meshes.size(); i++)
        AddMesh(val, *m_meshes[i], i);
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
* @bsimethod                                                    Ray.Bentley     08/02016
+---------------+---------------+---------------+---------------+---------------+------*/
 Utf8String TilePublisher::AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, Utf8CP  suffix)
    {
    bool        hasAlpha = textureImage.GetHasAlpha();

    Utf8String  textureId = Utf8String ("texture_") + suffix;
    Utf8String  imageId   = Utf8String ("image_")   + suffix;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + suffix;

    rootNode["textures"][textureId] = Json::objectValue;
    rootNode["textures"][textureId]["format"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    rootNode["textures"][textureId]["internalFormat"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    rootNode["textures"][textureId]["sampler"] = "sampler_0";
    rootNode["textures"][textureId]["source"] = imageId;

    rootNode["images"][imageId] = Json::objectValue;
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = "image/jpeg";

    static const    int s_jpegQuality = 50;
    ImageSource     imageSource (textureImage.GetImage(), hasAlpha ? ImageSource::Format::Png : ImageSource::Format::Jpeg, s_jpegQuality);

    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = textureImage.GetHeight();
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = textureImage.GetWidth();

    ByteStream const& imageData = imageSource.GetByteStreamR();

    rootNode["bufferViews"][bvImageId] = Json::objectValue;
    rootNode["bufferViews"][bvImageId]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bvImageId]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bvImageId]["byteLength"] = imageData.size();

    size_t current_buffer_size = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + imageData.size());
    memcpy(m_binaryData.data() + current_buffer_size, imageData.data(), imageData.size());

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddPolylineShaderTechnique (Json::Value& rootNode)
    {
    Utf8String      s_techniqueName = "polylineTechnique";

    if (rootNode.isMember("techniques") &&
        rootNode["techniques"].isMember(s_techniqueName.c_str()))
        return s_techniqueName;

    Json::Value     technique = Json::objectValue;

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    static char         *s_programName                    = "polylineProgram",
                        *s_vertexShaderName               = "polylineVertexShader",
                        *s_fragmentShaderName             = "polylineFragmentShader",
                        *s_vertexShaderBufferViewName     = "polylineVertexShaderBufferView",
                        *s_fragmentShaderBufferViewName   = "polylineFragmentShaderBufferView";

    technique["program"] = s_programName;

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(GLTF_DEPTH_TEST);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";
    techniqueAttributes["a_batchId"] = "batch";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";

    auto& rootProgramNode = (rootNode["programs"][s_programName] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");
    AppendProgramAttribute(rootProgramNode, "a_batchId");

    rootProgramNode["vertexShader"]   = s_vertexShaderName;
    rootProgramNode["fragmentShader"] = s_fragmentShaderName;

    auto& shaders = rootNode["shaders"];
    AddShader (shaders, s_vertexShaderName, GLTF_VERTEX_SHADER, s_vertexShaderBufferViewName);
    AddShader (shaders, s_fragmentShaderName, GLTF_FRAGMENT_SHADER, s_fragmentShaderBufferViewName);

    auto& bufferViews = rootNode["bufferViews"];

    AddBufferView(bufferViews, s_vertexShaderBufferViewName, s_polylineVertexShader);
    AddBufferView(bufferViews, s_fragmentShaderBufferViewName, s_polylineFragmentShader); 

    AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
    techniqueUniforms["u_color"] = "color";

    rootNode["techniques"][s_techniqueName.c_str()] = technique;

    return s_techniqueName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String     TilePublisher::AddMeshShaderTechnique (Json::Value& rootNode, bool textured, bool transparent, bool ignoreLighting)
    {
    Utf8String  prefix = textured ? "textured" : "untextured";

    if (transparent)
        prefix = prefix + "Transparent";

    if (ignoreLighting)
        prefix = prefix + "Unlit";

    Utf8String  techniqueName = prefix + "Technique";
    
    if (rootNode.isMember("techniques") &&
        rootNode["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value     technique = Json::objectValue;

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (!ignoreLighting)
        {
        AddTechniqueParameter(technique, "n", GLTF_FLOAT_VEC3, "NORMAL");
        AddTechniqueParameter(technique, "nmx", GLTF_FLOAT_MAT3, "MODELVIEWINVERSETRANSPOSE");
        }
    AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    Utf8String         programName               = prefix + "Program";
    Utf8String         vertexShader              = prefix + "VertexShader";
    Utf8String         fragmentShader            = prefix + "FragmentShader";
    Utf8String         vertexShaderBufferView    = vertexShader + "BufferView";
    Utf8String         fragmentShaderBufferView  = fragmentShader + "BufferView";

    technique["program"] = programName.c_str();

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(GLTF_DEPTH_TEST);
    techniqueStates["disable"].append(GLTF_CULL_FACE);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";
    techniqueAttributes["a_batchId"] = "batch";
    if(!ignoreLighting)
        techniqueAttributes["a_n"] = "n";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";
    if (!ignoreLighting)
        techniqueUniforms["u_nmx"] = "nmx";

    auto& rootProgramNode = (rootNode["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");
    AppendProgramAttribute(rootProgramNode, "a_batchId");
    if (!ignoreLighting)
        AppendProgramAttribute(rootProgramNode, "a_n");

    rootProgramNode["vertexShader"]   = vertexShader.c_str();
    rootProgramNode["fragmentShader"] = fragmentShader.c_str();

    auto& shaders = rootNode["shaders"];
    AddShader(shaders, vertexShader.c_str(), GLTF_VERTEX_SHADER, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), GLTF_FRAGMENT_SHADER, fragmentShaderBufferView.c_str());

    auto& bufferViews = rootNode["bufferViews"];


    AddBufferView(bufferViews, vertexShaderBufferView.c_str(),   ignoreLighting ? s_unlitTextureVertexShader  : (textured ? s_texturedVertexShader : s_untexturedVertexShader));
    AddBufferView(bufferViews, fragmentShaderBufferView.c_str(), ignoreLighting ? s_unlitTextureFragmentShader: (textured ? s_texturedFragShader    : s_untexturedFragShader)); 

    // Diffuse...
    if (textured)
        {
        AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        AddTechniqueParameter(technique, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");

        rootNode["samplers"]["sampler_0"] = Json::objectValue;
        rootNode["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;

        technique["uniforms"]["u_tex"] = "tex";
        technique["attributes"]["a_texc"] = "texc";
        AppendProgramAttribute(rootProgramNode, "a_texc");
        }
    else
        {
        AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
        techniqueUniforms["u_color"] = "color";
        }
    if (!ignoreLighting)
       {
        // Specular...
        AddTechniqueParameter(technique, "specularColor", GLTF_FLOAT_VEC3, nullptr);
        techniqueUniforms["u_specularColor"] = "specularColor";

        AddTechniqueParameter(technique, "specularExponent", GLTF_FLOAT, nullptr);
        techniqueUniforms["u_specularExponent"] = "specularExponent";
        }

    // Transparency requires blending extensions...
    if (transparent)
        {
        technique["states"]["enable"].append (3042);  // BLEND

        auto&   techniqueFunctions =    technique["states"]["functions"] = Json::objectValue;

        techniqueFunctions["blendEquationSeparate"] = Json::arrayValue;
        techniqueFunctions["blendFuncSeparate"]     = Json::arrayValue;

        techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (rgb)
        techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (aphla)
    
        techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcRGB)
        techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstRGB)
        techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcAlpha)
        techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstAlpha)
        
        techniqueFunctions["depthMask"] = "false";
        }


    rootNode["techniques"][techniqueName.c_str()] = technique;

    return techniqueName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, bool isPolyline, Utf8CP suffix)
    {
    RgbFactor       specularColor = { 1.0, 1.0, 1.0 };
    double          specularExponent = s_qvFinish * s_qvExponentMultiplier;
    uint32_t        rgbInt  = 0xffffff;
    RgbFactor       rgb;
    double          alpha = 1.0;
    Utf8String      materialName = Utf8String ("Material_") + suffix;
    Json::Value&    materialValue = rootNode["materials"][materialName.c_str()] = Json::objectValue;

    if (nullptr != displayParams)
        {
        rgbInt = displayParams->GetFillColor();

        if (!isPolyline && displayParams->GetMaterialId().IsValid())
            {
            JsonRenderMaterial  jsonMaterial;

            if (SUCCESS == jsonMaterial.Load (displayParams->GetMaterialId(), m_context.GetDgnDb()))
                {
                static double       s_finishScale = 15.0;

                if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasSpecularColor, false))
                    specularColor = jsonMaterial.GetColor (RENDER_MATERIAL_SpecularColor);

                if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasFinish, false))
                    specularExponent = jsonMaterial.GetDouble (RENDER_MATERIAL_Finish, s_qvSpecular) * s_finishScale;

                if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasBaseColor, false))
                    rgb = jsonMaterial.GetColor (RENDER_MATERIAL_Color);

                if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasTransmit, false))
                    alpha = 1.0 - jsonMaterial.GetDouble (RENDER_MATERIAL_Transmit, 0.0);
                }
            }

        TileTextureImageCP      textureImage;

        if (!isPolyline && nullptr != (textureImage = displayParams->GetTextureImage()))
            {
            materialValue["technique"] = AddMeshShaderTechnique (rootNode, true, alpha < 1.0, displayParams->GetIgnoreLighting()).c_str();
            materialValue["values"]["tex"] = AddTextureImage (rootNode, *textureImage, suffix);
            }
        else
            {
            RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);
            double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
            auto&           materialColor = materialValue["values"]["color"] = Json::arrayValue;

            materialColor.append(rgb.red);
            materialColor.append(rgb.green);
            materialColor.append(rgb.blue);
            materialColor.append(alpha);

            materialValue["technique"] = isPolyline ? AddPolylineShaderTechnique (rootNode).c_str() : AddMeshShaderTechnique(rootNode, false, alpha < 1.0, false).c_str();
            }

        if (!isPolyline)
            {
            materialValue["values"]["specularExponent"] = specularExponent;

            auto& materialSpecularColor = materialValue["values"]["specularColor"] = Json::arrayValue;
            materialSpecularColor.append (specularColor.red);
            materialSpecularColor.append (specularColor.green);
            materialSpecularColor.append (specularColor.blue);
            }
        }
    return materialName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMesh(Json::Value& rootNode, TileMeshR mesh, size_t index)
    {
    Utf8String idStr(std::to_string(index).c_str());

    Utf8String bv_pos_id    = Concat("bv_pos_", idStr),
               bv_ind_id    = Concat("bv_ind_", idStr),
               bv_uv_id     = Concat("bv_uv_", idStr),
               bv_n_id      = Concat("bv_n_", idStr),
               bv_bat_id    = Concat("bv_bat_", idStr),
               acc_pos_id   = Concat("acc_pos_", idStr),
               acc_ind_id   = Concat("acc_ind_", idStr),
               acc_uv_id    = Concat("acc_uv_", idStr),
               acc_n_id     = Concat("acc_n_", idStr)
               ,acc_bat_id  = Concat("acc_bat_", idStr);

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

    BeAssert (mesh.Triangles().empty() || mesh.Polylines().empty());        // Meshes should contain either triangles or polylines but not both.

    if (!mesh.Triangles().empty())
        {
        indices.reserve(mesh.Triangles().size() * 3);
        for (auto const& tri : mesh.Triangles())
            {
            indices.push_back((unsigned int)tri.m_indices[0]);
            indices.push_back((unsigned int)tri.m_indices[1]);
            indices.push_back((unsigned int)tri.m_indices[2]);
            }
        }
    else if (!mesh.Polylines().empty())
        {
        for (auto const& polyline : mesh.Polylines())
            {
            for (size_t i=0; i<polyline.m_indices.size()-1; i++)
                {
                indices.push_back (polyline.m_indices[i]);
                indices.push_back (polyline.m_indices[i+1]);
                }
            }
        }

    bvector<float> uvs;
    uvs.reserve(mesh.Params().size() * 2);
    for (auto const& uv : mesh.Params())
        {
        uvs.push_back((float)uv.x);
        uvs.push_back((float)(1.0 - uv.y));         // Ick... Either our images are flipped or our v- convention is mismatched.
        }

    bvector<float> normals;
    normals.reserve(mesh.Normals().size() * 3);
    for (auto const& norm : mesh.Normals())
        {
        normals.push_back((float)norm.x);
        normals.push_back((float)norm.y);
        normals.push_back((float)norm.z);
        }

    bvector<uint16_t> batchIds;
    batchIds.reserve(mesh.ElementIds().size());
    for (auto const& elemId : mesh.ElementIds())
        batchIds.push_back(m_batchIds.GetBatchId(elemId));

    Json::Value attr = Json::objectValue;

    attr["attributes"]["POSITION"] = acc_pos_id;

    if (!uvs.empty())
        attr["attributes"]["TEXCOORD_0"] = acc_uv_id;

    if (!normals.empty())
        attr["attributes"]["NORMAL"] = acc_n_id;

    attr["attributes"]["BATCHID"] = acc_bat_id;
    attr["indices"] = acc_ind_id;

    attr["material"] = AddMaterial (rootNode, mesh.GetDisplayParams(), mesh.Triangles().empty(), idStr.c_str());

    attr["mode"] = mesh.Triangles().empty() ? GLTF_LINES : GLTF_TRIANGLES;
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

    auto nBatchIdBytes = batchIds.size() * sizeof(uint16_t);
    rootNode["bufferViews"][bv_bat_id] = Json::objectValue;
    rootNode["bufferViews"][bv_bat_id]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bv_bat_id]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bv_bat_id]["byteLength"] = nBatchIdBytes;
    rootNode["bufferViews"][bv_bat_id]["target"] = GLTF_ARRAY_BUFFER;

    current_buffer_size = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + nBatchIdBytes);
    memcpy(m_binaryData.data() + current_buffer_size, batchIds.data(), nBatchIdBytes);

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

    rootNode["accessors"][acc_bat_id] = Json::objectValue;
    rootNode["accessors"][acc_bat_id]["bufferView"] = bv_bat_id;
    rootNode["accessors"][acc_bat_id]["byteOffset"] = 0;
    rootNode["accessors"][acc_bat_id]["componentType"] = GLTF_UNSIGNED_SHORT;
    rootNode["accessors"][acc_bat_id]["count"] = batchIds.size();
    rootNode["accessors"][acc_bat_id]["type"] = "SCALAR";

    if (!uvs.empty())
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

    if (!normals.empty())
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

    // For now use view center... maybe should use the DgnDb range center.
    DPoint3d        origin = m_viewController.GetCenter ();

    m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);

    DgnGCS*         dgnGCS = m_viewController.GetDgnDb().Units().GetDgnGCS();

    if (nullptr == dgnGCS)
        {
        m_tileToEcef    = Transform::FromIdentity ();   
        }
    else
        {
        GeoPoint        originLatLong, northLatLong;
        DPoint3d        ecfOrigin, ecfNorth, north = origin;
    
        north.y += 100.0;

        dgnGCS->LatLongFromUors (originLatLong, origin);
        dgnGCS->XYZFromLatLong(ecfOrigin, originLatLong);

        dgnGCS->LatLongFromUors (northLatLong, north);
        dgnGCS->XYZFromLatLong(ecfNorth, northLatLong);

        DVec3d      zVector, yVector;
        RotMatrix   rMatrix;

        zVector.Normalize ((DVec3dCR) ecfOrigin);
        yVector.NormalizedDifference (ecfNorth, ecfOrigin);

        rMatrix.SetColumn (yVector, 1);
        rMatrix.SetColumn (zVector, 2);
        rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

        m_tileToEcef =  Transform::From (rMatrix, ecfOrigin);
        }

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::ConvertStatus(TileGenerator::Status input)
    {
    switch (input)
        {
        case TileGenerator::Status::Success:        return Status::Success;
        case TileGenerator::Status::NoGeometry:     return Status::NoGeometry;
        case TileGenerator::Status::NotImplemented: return Status::NoGeometry;  // "NotImplemented" means "the viewed model is not an IPublishModelTiles therefore no tiles to publish"...not an error.
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
TilesetPublisher::Status TilesetPublisher::WriteWebApp (TransformCR transform, bvector<WString>& viewedTileSetNames)
    {
    // Set up initial view based on view controller settings
    DVec3d xVec, yVec, zVec;
    m_viewController.GetRotation().GetRows(xVec, yVec, zVec);

    auto cameraView = m_viewController._ToCameraView();
    bool useCamera = nullptr != cameraView && cameraView->IsCameraOn();
    DPoint3d viewDest = useCamera ? cameraView->GetControllerCamera().GetEyePoint() : m_viewController.GetCenter();
    if (!useCamera)
        {
        static const double s_zRatio = 1.5;
        DVec3d viewDelta = m_viewController.GetDelta();
        viewDest = DPoint3d::FromSumOf(viewDest, zVec, std::max(viewDelta.x, viewDelta.y) * s_zRatio);
        }

    transform.Multiply(viewDest);
    transform.MultiplyMatrixOnly(yVec);
    transform.MultiplyMatrixOnly(zVec);

    yVec.Normalize();
    zVec.Normalize();
    zVec.Negate();      // Towards target.

    bool geoLocated = !m_tileToEcef.IsIdentity();
    Utf8CP viewOptionString = geoLocated ? "" : "globe: false, scene3DOnly:true, skyBox: false, skyAtmosphere: false";
    Utf8CP viewFrameString = geoLocated ? s_geoLocatedViewingFrameJs : s_3dOnlyViewingFrameJs; 

    // Produce the html file contents
    Utf8PrintfString html(s_viewerHtml, viewOptionString, m_rootName.c_str(), m_rootName.c_str(), viewFrameString, viewDest.x, viewDest.y, viewDest.z, zVec.x, zVec.y, zVec.z, yVec.x, yVec.y, yVec.z);

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
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status   TilesetPublisher::PublishViewedModel (WStringR tileSetName, DgnModelR model)
    {
    IPublishModelTiles*     publishTiles;

    if (NULL == (publishTiles = dynamic_cast <IPublishModelTiles*> (&model)))
        return TileGenerator::Status::NotImplemented;

    return publishTiles->_PublishModelTiles (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilesetPublisher::Status TilesetPublisher::Publish()
    {
    auto status = Setup();
    if (Status::Success != status)
        return status;

    ProgressMeter progressMeter(*this);
    TileGenerator generator (m_dbToTile, &progressMeter);

    static double       s_toleranceInMeters = 0.01;
    bvector<WString>    viewedTileSetNames;

    status = ConvertStatus(generator.LoadGeometry(m_viewController, s_toleranceInMeters));

    m_generator = &generator;
    if (Status::Success == status)
        {
        static const size_t     s_maxPointsPerTile = 20000;
        TileNode                rootNode;

        status = ConvertStatus(generator.GenerateTiles (rootNode, s_toleranceInMeters, s_maxPointsPerTile));
        if (Status::Success == status)
            {
            if (Status::Success == (status = ConvertStatus (generator.CollectTiles(rootNode, *this))))
                viewedTileSetNames.push_back (m_rootName);
            }
        }
    if (status != Status::Success &&
        status != Status::NoGeometry)      // If no root geometry there still may be viewed models.
        return status;

    for (auto& modelId : m_viewController.GetViewedModels())
        {
        if (modelId == m_viewController.GetBaseModelId())
            continue;

        WString         tileSetName;
        DgnModelPtr     viewedModel = m_viewController.GetDgnDb().Models().GetModel (modelId);

        if (viewedModel.IsValid() && TileGenerator::Status::Success == PublishViewedModel (tileSetName, *viewedModel))
            viewedTileSetNames.push_back (tileSetName);
        }

    m_generator = nullptr;

    if (Status::Success != status)
        return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;

    OutputStatistics(generator.GetStatistics());

    return WriteWebApp (Transform::FromProduct (m_tileToEcef, m_dbToTile), viewedTileSetNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::OutputStatistics(TileGenerator::Statistics const& stats) const
    {
    printf("Statistics:\n"
           "Tile count: %u\n"
           "Tile depth: %u\n"
           "Geometry collection time: %.4f seconds\n"
           "Tile creation: %.4f seconds Average per-tile: %.4f seconds\n",
           static_cast<uint32_t>(stats.m_tileCount),
           static_cast<uint32_t>(stats.m_tileDepth),
           stats.m_collectionTime,
           stats.m_tileCreationTime,
           0 != stats.m_tileCount ? stats.m_tileCreationTime / stats.m_tileCount : 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_IndicateProgress(uint32_t completed, uint32_t total)
    {
    if (m_lastNumCompleted == completed)
        {
        printf("...");
        }
    else
        {
        m_lastNumCompleted = completed;
        uint32_t pctComplete = static_cast<double>(completed)/total * 100;
        printf("\n%s: %u%% (%u/%u)%s", m_taskName.c_str(), pctComplete, completed, total, completed == total ? "\n" : "");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_SetTaskName(TileGenerator::TaskName task)
    {
    Utf8String newTaskName = (TileGenerator::TaskName::CreatingTiles == task) ? "Creating Tiles" : "Collecting Geometry";
    if (!m_taskName.Equals(newTaskName))
        {
        m_lastNumCompleted = 0xffffffff;
        m_taskName = newTaskName;
        }
    }

