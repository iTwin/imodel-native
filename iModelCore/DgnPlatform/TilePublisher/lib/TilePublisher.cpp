/*--------------------------------------------------------------------------------------+                                                                                                                                      
|
|     $Source: TilePublisher/lib/TilePublisher.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include "Constants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;

#define WIP_2D_SUPPORT

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
BatchIdMap::BatchIdMap(TileSource source) : m_source(source)
    {
    // Invalid ID always maps to the first batch table index
    GetBatchId(BeInt64Id());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t BatchIdMap::GetBatchId(BeInt64Id elemId)
    {
    auto found = m_map.find(elemId);
    if (m_map.end() == found)
        {
        auto batchId = static_cast<uint16_t>(m_list.size());
        if (batchId == 0xffff)
            return 0;   // ###TODO: avoid hitting this limit...

        m_list.push_back(elemId);
        found = m_map.insert(bmap<BeInt64Id, uint16_t>::value_type(elemId, batchId)).first;
        }

    return found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchIdMap::ToJson(Json::Value& value, DgnDbR db) const
    {
    switch (m_source)
        {
        case TileSource::None:
            return;
        case TileSource::Model:
            {
            Json::Value modelIds(Json::arrayValue);
            for (auto idIter = m_list.begin(); idIter != m_list.end(); ++idIter)
                modelIds.append(idIter->ToString());

            value["model"] = modelIds;
            return;
            }
        case TileSource::Element:
            {
            // ###TODO: Assumes 3d-only...
            // There's no longer a simple way to query the category of an arbitrary geometric element without knowing whether it's 2d or 3d...
            static const Utf8CP s_sql = "SELECT e.ModelId,g.CategoryId FROM " BIS_TABLE(BIS_CLASS_Element) " AS e, " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g "
                "WHERE e.Id=? AND g.ElementId=e.Id";

            BeSQLite::Statement stmt;
            stmt.Prepare(db, s_sql);

            Json::Value elementIds(Json::arrayValue);
            Json::Value modelIds(Json::arrayValue);
            Json::Value categoryIds(Json::arrayValue);

            for (auto elemIter = m_list.begin(); elemIter != m_list.end(); ++elemIter)
                {
                elementIds.append(elemIter->ToString());    // NB: Javascript doesn't support full range of 64-bit integers...must convert to strings...
                DgnModelId modelId;
                DgnCategoryId categoryId;

                stmt.BindId(1, *elemIter);
                if (BeSQLite::BE_SQLITE_ROW == stmt.Step())
                    {
                    modelId = stmt.GetValueId<DgnModelId>(0);
                    categoryId = stmt.GetValueId<DgnCategoryId>(1);
                    }

                modelIds.append(modelId.ToString());
                categoryIds.append(categoryId.ToString());
                stmt.Reset();
                }

            value["element"] = elementIds;
            value["model"] = modelIds;
            value["category"] = categoryIds;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::TilePublisher(TileNodeCR tile, PublisherContext& context)
    : m_batchIds(tile.GetSource()), m_centroid(tile.GetTileCenter()), m_tile(tile), m_context(context)
    {
#define CESIUM_RTC_ZERO
#ifdef CESIUM_RTC_ZERO
    m_centroid = DPoint3d::FromXYZ(0,0,0);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::IncrementalStatus   TilePublisher::IncrementalGenerate (TileModelDeltaCR modelDelta)
    {
    TileReader          tileReader;
    TileMeshList        oldMeshes, newMeshes;
                                                        
    if (modelDelta.DoIncremental(m_tile) &&
        TileReader::Status::Success != tileReader.ReadTile (oldMeshes, GetBinaryDataFileName()))
        return IncrementalStatus::Regenerate;

    bool        geometryRemoved = false;
    DRange3d    publishedRange = DRange3d::NullRange();

    if (!modelDelta.GetDeleted().empty())
        for (auto& mesh : oldMeshes)
            geometryRemoved |= mesh->RemoveEntityGeometry(modelDelta.GetDeleted());

    if (!modelDelta.GetAdded().empty())
        newMeshes =  m_tile.GenerateMeshes(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, false, m_context.WantPolylines(), &modelDelta);

    if (newMeshes.empty())
        {
        if (!geometryRemoved)
            {
            for (auto& oldMesh : oldMeshes)
                publishedRange.Extend (oldMesh->GetRange());

            m_tile.SetPublishedRange (publishedRange);
            return IncrementalStatus::UsePrevious;
            }
        m_meshes = oldMeshes;
        }
    else
        {
        // Merge old meshes with new ones.
        bmap<TileMeshMergeKey, TileMeshPtr>  meshMap;
    
        for (auto& oldMesh : oldMeshes)
            if (!oldMesh->IsEmpty())
                meshMap.Insert(TileMeshMergeKey(*oldMesh), oldMesh);

        for (auto& newMesh : newMeshes)
            {
            TileMeshMergeKey    key(*newMesh);
            auto const&         found = meshMap.find(key);

            if (meshMap.find(key) == meshMap.end())
                meshMap.Insert (key, newMesh);
            else
                found->second->AddMesh (*newMesh);
            }
        for (auto& curr : meshMap)
            {
            m_meshes.push_back (curr.second);
            }
        }
    for (auto& mesh : m_meshes)
        publishedRange.Extend (mesh->GetRange());

    m_tile.SetPublishedRange (publishedRange);
    return IncrementalStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBoundingVolume(Json::Value& val, DRange3dCR range)
    {
    BeAssert (!range.IsNull());
    DPoint3d    center = DPoint3d::FromInterpolate (range.low, .5, range.high);
    DVec3d      diagonal = range.DiagonalVector();

    // Range identified by center point and axes
    // Axes are relative to origin of box, not center
    static double      s_minSize = .001;   // Meters...  Don't allow degenerate box.

    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    AppendPoint(box, center);
    AppendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, diagonal.x), 0.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, diagonal.y), 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, diagonal.z)));
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
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName  TilePublisher::GetBinaryDataFileName() const
    {
    WString rootName;
    BeFileName dataDir = m_context.GetDataDirForModel(m_tile.GetModel(), &rootName);

    return BeFileName(nullptr, dataDir.c_str(), m_tile.GetFileName (rootName.c_str(), s_binaryDataExtension).c_str(), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilePublisher::Publish()
    {
    TileModelDeltaCP     modelDelta;

    if (nullptr != (modelDelta = m_tile.GetModelDelta()))
        {
        switch (IncrementalGenerate(*modelDelta))
            {
            case IncrementalStatus::UsePrevious:        // There are no changes within this tile - use previously generated tile.
                return PublisherContext::Status::Success;

            case IncrementalStatus::Regenerate:
                m_meshes = m_tile.GenerateMeshes(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, false, m_context.WantPolylines(), nullptr);
                break;

             case IncrementalStatus::Success:
                break;
            }
        }
    else
        {
        m_meshes = m_tile.GenerateMeshes(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, false, m_context.WantPolylines(), nullptr);
        }

    if (m_meshes.empty())
        return PublisherContext::Status::NoGeometry;       // Nothing to write...Ignore this tile (it will be omitted when writing tileset data as its published range will be NullRange.

    
    // .b3dm file
    Json::Value sceneJson(Json::objectValue);

    ProcessMeshes(sceneJson);

    Utf8String sceneStr = Json::FastWriter().write(sceneJson);

    Json::Value batchTableJson(Json::objectValue);
    m_batchIds.ToJson(batchTableJson, m_context.GetDgnDb());
    Utf8String batchTableStr = Json::FastWriter().write(batchTableJson);
    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());

    std::FILE*  outputFile = _wfopen(GetBinaryDataFileName().c_str(), L"wb");

    if (nullptr == outputFile)
        {
        BeAssert (false && "Unable to open output file");
        return PublisherContext::Status::CantOpenOutputFile;
        }

    // GLTF header = 5 32-bit values
    static const size_t s_gltfHeaderSize = 20;
    uint32_t sceneStrLength = static_cast<uint32_t>(sceneStr.size());
    uint32_t gltfLength = s_gltfHeaderSize + sceneStrLength + m_binaryData.GetSize();

    // B3DM header = 6 32-bit values
    // Header immediately followed by batch table json
    static const size_t s_b3dmHeaderSize = 24;
    uint32_t b3dmNumBatches = m_batchIds.Count(), zero = 0;
    uint32_t b3dmLength = gltfLength + s_b3dmHeaderSize + batchTableStrLen;

    std::fwrite(s_b3dmMagic, 1, 4, outputFile);
    std::fwrite(&s_b3dmVersion, 1, 4, outputFile);
    std::fwrite(&b3dmLength, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&batchTableStrLen, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile); // length of binary portion of batch table - we have no binary batch table data
    std::fwrite(&b3dmNumBatches, 1, sizeof(uint32_t), outputFile);
    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, outputFile);
    std::fwrite(&s_gltfMagic, 1, 4, outputFile);
    std::fwrite(&s_gltfVersion, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&gltfLength, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&sceneStrLength, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&s_gltfSceneFormat, 1, sizeof(uint32_t), outputFile);

    std::fwrite(sceneStr.data(), 1, sceneStrLength, outputFile);
    if (!m_binaryData.empty())
        std::fwrite(m_binaryData.data(), 1, m_binaryData.size(), outputFile);

    std::fclose(outputFile);

    return PublisherContext::Status::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::ProcessMeshes(Json::Value& val)
    {
    AddExtensions(val);

    val["meshes"]["mesh_0"]["primitives"] = Json::arrayValue;

    DRange3d    publishedRange = DRange3d::NullRange();

    for (size_t i = 0; i < m_meshes.size(); i++)
        {
        AddMesh(val, *m_meshes[i], i);
        AddPolylines(val, *m_meshes[i], i); 
        publishedRange.Extend (m_meshes[i]->GetRange());
        }

    m_tile.SetPublishedRange (publishedRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddExtensions(Json::Value& rootNode)
    {
    rootNode["extensionsUsed"] = Json::arrayValue;
    rootNode["extensionsUsed"].append("KHR_binary_glTF");
    rootNode["extensionsUsed"].append("CESIUM_RTC");
    rootNode["extensionsUsed"].append("WEB3D_quantized_attributes");

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
* @bsimethod                                                    Ray.Bentley     10/02016
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t  roundToMultipleOfTwo (int32_t value)
    {
    static          double  s_closeEnoughRatio = .85;       // Don't round up if already within .85 of value.
    int32_t         rounded = 2;
    int32_t         closeEnoughValue = (int32_t) ((double) value * s_closeEnoughRatio);
    
    while (rounded < closeEnoughValue && rounded < 0x01000000)
        rounded <<= 1;

    return rounded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/02016
+---------------+---------------+---------------+---------------+---------------+------*/
 Utf8String TilePublisher::AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, TileMeshCR mesh, Utf8CP  suffix)
    {
    auto const& found = m_textureImages.find (&textureImage);

    if (found != m_textureImages.end())
        return found->second;

    bool        hasAlpha = textureImage.GetImageSource().GetFormat() == ImageSource::Format::Png;


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

    DRange3d    range = mesh.GetRange(), uvRange = mesh.GetUVRange();
    Image       image (textureImage.GetImageSource(), hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);

    // This calculation should actually be made for each triangle and maximum used. 
    static      double      s_requiredSizeRatio = 2.0, s_sizeLimit = 1024.0;
    double      requiredSize = std::min (s_sizeLimit, s_requiredSizeRatio * range.DiagonalDistance () / (m_tile.GetTolerance() * std::min (1.0, uvRange.DiagonalDistance())));
    DPoint2d    imageSize = { (double) image.GetWidth(), (double) image.GetHeight() };

    rootNode["bufferViews"][bvImageId] = Json::objectValue;
    rootNode["bufferViews"][bvImageId]["buffer"] = "binary_glTF";

    Point2d     targetImageSize, currentImageSize = { (int32_t) image.GetWidth(), (int32_t) image.GetHeight() };

    if (requiredSize < std::min (currentImageSize.x, currentImageSize.y))
        {
        static      int32_t s_minImageSize = 64;
        static      int     s_imageQuality = 60;
        int32_t     targetImageMin = std::max(s_minImageSize, (int32_t) requiredSize);
        ByteStream  targetImageData;

        if (imageSize.x > imageSize.y)
            {
            targetImageSize.y = targetImageMin;
            targetImageSize.x = (int32_t) ((double) targetImageSize.y * imageSize.x / imageSize.y);
            }
        else
            {
            targetImageSize.x = targetImageMin;
            targetImageSize.y = (int32_t) ((double) targetImageSize.x * imageSize.y / imageSize.x);
            }
        targetImageSize.x = roundToMultipleOfTwo (targetImageSize.x);
        targetImageSize.y = roundToMultipleOfTwo (targetImageSize.y);
        }
    else
        {
        targetImageSize.x = roundToMultipleOfTwo (currentImageSize.x);
        targetImageSize.y = roundToMultipleOfTwo (currentImageSize.y);
        }

    ImageSource         imageSource = textureImage.GetImageSource();
    static const int    s_imageQuality = 50;

    if (targetImageSize.x != imageSize.x || targetImageSize.y != imageSize.y)
        {
        Image           targetImage = Image::FromResizedImage (targetImageSize.x, targetImageSize.y, image);

        imageSource = ImageSource (targetImage, textureImage.GetImageSource().GetFormat(), s_imageQuality);
        }

    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = targetImageSize.x;
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = targetImageSize.y;

    ByteStream const& imageData = imageSource.GetByteStream();
    rootNode["bufferViews"][bvImageId]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bvImageId]["byteLength"] = imageData.size();

    AddBinaryData (imageData.data(), imageData.size());

    m_textureImages.Insert (&textureImage, textureId);

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddUnlitShaderTechnique (Json::Value& rootNode)
    {
    Utf8String      s_techniqueName = "unlitTechnique";

    if (rootNode.isMember("techniques") &&
        rootNode["techniques"].isMember(s_techniqueName.c_str()))
        return s_techniqueName;

    Json::Value     technique = Json::objectValue;

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    static char         *s_programName                    = "unlitProgram",
                        *s_vertexShaderName               = "unlitVertexShader",
                        *s_fragmentShaderName             = "unlitFragmentShader",
                        *s_vertexShaderBufferViewName     = "unlitVertexShaderBufferView",
                        *s_fragmentShaderBufferViewName   = "unlitFragmentShaderBufferView";

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

    AddBufferView(bufferViews, s_vertexShaderBufferViewName, s_unlitVertexShader);
    AddBufferView(bufferViews, s_fragmentShaderBufferViewName, s_unlitFragmentShader); 

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
Utf8String TilePublisher::AddMeshMaterial (Json::Value& rootNode, bool& isTextured, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix)
    {
    Utf8String      materialName = Utf8String ("Material_") + suffix;

    if (nullptr == displayParams)
        return materialName;


    RgbFactor       specularColor = { 1.0, 1.0, 1.0 };
    double          specularExponent = s_qvFinish * s_qvExponentMultiplier;
    uint32_t        rgbInt  = displayParams->GetFillColor();
    double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
    Json::Value&    materialValue = rootNode["materials"][materialName.c_str()] = Json::objectValue;
    bool            isUnlit = displayParams->GetIgnoreLighting();
    RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);

    if (!isUnlit && displayParams->GetMaterialId().IsValid())
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

            DgnMaterialCPtr material = DgnMaterial::Get(m_context.GetDgnDb(), displayParams->GetMaterialId());

            if (material.IsValid())
                materialValue["name"] = material->GetMaterialName().c_str();

            materialValue["materialId"] = displayParams->GetMaterialId().ToString();
            }
        }

    TileTextureImageCP      textureImage = nullptr;

    displayParams->ResolveTextureImage(m_context.GetDgnDb());
    if (false != (isTextured = (nullptr != (textureImage = displayParams->GetTextureImage()))))
        {
        materialValue["technique"] = AddMeshShaderTechnique (rootNode, true, alpha < 1.0, displayParams->GetIgnoreLighting()).c_str();
        materialValue["values"]["tex"] = AddTextureImage (rootNode, *textureImage, mesh, suffix);
        }
    else
        {
        auto& materialColor = materialValue["values"]["color"] = Json::arrayValue;

        materialColor.append(rgb.red);
        materialColor.append(rgb.green);
        materialColor.append(rgb.blue);
        materialColor.append(alpha);

        materialValue["technique"] = isUnlit ? AddUnlitShaderTechnique (rootNode).c_str() : AddMeshShaderTechnique(rootNode, false, alpha < 1.0, false).c_str();
        }

    if (! isUnlit)
        {
        materialValue["values"]["specularExponent"] = specularExponent;

        auto& materialSpecularColor = materialValue["values"]["specularColor"] = Json::arrayValue;
        materialSpecularColor.append (specularColor.red);
        materialSpecularColor.append (specularColor.green);
        materialSpecularColor.append (specularColor.blue);
        }
    return materialName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddPolylineMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix)
    {
    Utf8String      materialName = Utf8String ("Material_") + suffix;

    if (nullptr == displayParams)
        return materialName;

    uint32_t        rgbInt  = displayParams->GetFillColor();
    double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
    Json::Value&    materialValue = rootNode["materials"][materialName.c_str()] = Json::objectValue;
    RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);

    auto& materialColor = materialValue["values"]["color"] = Json::arrayValue;

    materialColor.append(rgb.red);
    materialColor.append(rgb.green);
    materialColor.append(rgb.blue);
    materialColor.append(alpha);

    Utf8String      s_techniqueName = "polylineTechnique";

    if (!rootNode.isMember("techniques") ||
        !rootNode["techniques"].isMember(s_techniqueName.c_str()))
        {
        Json::Value     technique = Json::objectValue;

        AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
        AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
        AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
        AddTechniqueParameter(technique, "previous", GLTF_FLOAT_VEC3, "PREVIOUS");
        AddTechniqueParameter(technique, "next", GLTF_FLOAT_VEC3, "NEXT");
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");
        AddTechniqueParameter(technique, "width", GLTF_FLOAT, "WIDTH");

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

        techniqueAttributes["a_next"] = "next";
        techniqueAttributes["a_batchId"] = "batch";
        techniqueAttributes["a_pos"]  = "pos";
        techniqueAttributes["a_previous"] = "previous";
        techniqueAttributes["a_next"] = "next";
        techniqueAttributes["a_width"] = "width";


        auto& techniqueUniforms = technique["uniforms"];
        techniqueUniforms["u_mv"] = "mv";
        techniqueUniforms["u_proj"] = "proj";

        auto& rootProgramNode = (rootNode["programs"][s_programName] = Json::objectValue);
        rootProgramNode["attributes"] = Json::arrayValue;
        AppendProgramAttribute(rootProgramNode, "a_pos");
        AppendProgramAttribute(rootProgramNode, "a_previous");
        AppendProgramAttribute(rootProgramNode, "a_next");
        AppendProgramAttribute(rootProgramNode, "a_width");
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
        }

    materialValue["technique"] = s_techniqueName;
    return materialName;
    }      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    TilePublisher::AddBinaryData (void const* data, size_t size)
    {
    size_t currentBufferSize = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + size);
    memcpy(m_binaryData.data() + currentBufferSize, data, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshVertexAttribute (Json::Value& rootNode, double const* values, Utf8StringCR bufferViewId, Utf8StringCR accesorId, size_t nComponents, size_t nAttributes, char* accessorType, bool quantize, double const* min, double const* max)
    {
    size_t              nValues = nComponents * nAttributes;
    size_t              dataSize;
    size_t              byteOffset = m_binaryData.size();
    Json::Value         bufferViews = Json::objectValue;
    Json::Value         accessor   = Json::objectValue;

    if (quantize)
        {
        double      range = (double) (0xffff);
        
        accessor["componentType"] = GLTF_UNSIGNED_SHORT;
            
        auto&       quantizeExtension = accessor["extensions"]["WEB3D_quantized_attributes"];
        auto&       decodeMatrix = quantizeExtension["decodeMatrix"] = Json::arrayValue;
         
        for (size_t i=0; i<nComponents; i++)
            {
            for (size_t j=0; j<nComponents; j++)
                decodeMatrix.append ((i==j) ? ((max[i] - min[i]) / range) : 0.0);

            decodeMatrix.append (0.0);
            }
        for (size_t i=0; i<nComponents; i++)
            decodeMatrix.append (min[i]);

        decodeMatrix.append (1.0);
        
        for (size_t i=0; i<nComponents; i++)
            {
            quantizeExtension["decodedMin"].append (min[i]);
            quantizeExtension["decodedMax"].append (max[i]);
            }

        bvector <unsigned short>    quantizedValues;

        for (size_t i=0; i<nValues; i++)
            {
            size_t  componentIndex = i % nComponents;
            quantizedValues.push_back ((unsigned short) (.5 + (values[i] - min[componentIndex]) * range / (max[componentIndex] - min[componentIndex])));
            }
        AddBinaryData (quantizedValues.data(), dataSize = nValues * sizeof (unsigned short));
        }
    else
        {
        bvector <float>     floatValues;

        accessor["componentType"] = GLTF_FLOAT;

        for (size_t i=0; i<nValues; i++)
            floatValues.push_back ((float) values[i]);

        AddBinaryData (floatValues.data(), dataSize = nValues * sizeof (float));
        }

    bufferViews["buffer"] = "binary_glTF";
    bufferViews["byteOffset"] = byteOffset;
    bufferViews["byteLength"] = dataSize;
    bufferViews["target"] = GLTF_ARRAY_BUFFER;

    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = nAttributes;
    accessor["type"] = accessorType;

    rootNode["bufferViews"][bufferViewId] = bufferViews;
    rootNode["accessors"][accesorId] = accessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshBatchIds (Json::Value& rootNode, Json::Value& primitive, TileMeshR mesh, Utf8StringCR idStr)
    {
    Utf8String  bvBatchId        = Concat("bvBatch_", idStr),
                accBatchId       = Concat("accBatch_", idStr);

    if (mesh.ValidIdsPresent())
        {
        bvector<uint16_t>   batchIds;

        for (auto const& elemId : mesh.EntityIds())
            batchIds.push_back(m_batchIds.GetBatchId(elemId));

        primitive["attributes"]["BATCHID"] = accBatchId;

        auto nBatchIdBytes = batchIds.size() * sizeof(uint16_t);
        rootNode["bufferViews"][bvBatchId] = Json::objectValue;
        rootNode["bufferViews"][bvBatchId]["buffer"] = "binary_glTF";
        rootNode["bufferViews"][bvBatchId]["byteOffset"] = m_binaryData.size();
        rootNode["bufferViews"][bvBatchId]["byteLength"] = nBatchIdBytes;
        rootNode["bufferViews"][bvBatchId]["target"] = GLTF_ARRAY_BUFFER;

        AddBinaryData (batchIds.data(), nBatchIdBytes);
        rootNode["accessors"][accBatchId] = Json::objectValue;
        rootNode["accessors"][accBatchId]["bufferView"] = bvBatchId;
        rootNode["accessors"][accBatchId]["byteOffset"] = 0;
        rootNode["accessors"][accBatchId]["componentType"] = GLTF_UNSIGNED_SHORT;
        rootNode["accessors"][accBatchId]["count"] = batchIds.size();
        rootNode["accessors"][accBatchId]["type"] = "SCALAR";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshIndices(Json::Value& rootNode, Json::Value& primitive, bvector<uint32_t> const& indices, Utf8StringCR idStr)
    {
    Utf8String          accIndexId       = Concat("accIndex_", idStr),
                        bvIndexId        = Concat("bvIndex_", idStr);
    bool                useShortIndices    = true;

    for (auto& index : indices)
        {
        if (index > 0xffff)
            {
            useShortIndices = false;
            break;
            }
        }
    primitive["indices"] = accIndexId;

    rootNode["bufferViews"][bvIndexId] = Json::objectValue;
    rootNode["bufferViews"][bvIndexId]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bvIndexId]["byteOffset"] = m_binaryData.size();
    rootNode["bufferViews"][bvIndexId]["byteLength"] = indices.size() * (useShortIndices ? sizeof(uint16_t) : sizeof(uint32_t));
    rootNode["bufferViews"][bvIndexId]["target"] =  GLTF_ELEMENT_ARRAY_BUFFER;

    if (useShortIndices)
        {
        bvector<uint16_t>   shortIndices;

        for (auto& index : indices)
            shortIndices.push_back ((uint16_t) index);

        AddBinaryData (shortIndices.data(), shortIndices.size()*sizeof(uint16_t));
        }
    else
        {
        AddBinaryData (indices.data(),  indices.size()*sizeof(uint32_t));
        }

    rootNode["accessors"][accIndexId] = Json::objectValue;
    rootNode["accessors"][accIndexId]["bufferView"] = bvIndexId;
    rootNode["accessors"][accIndexId]["byteOffset"] = 0;
    rootNode["accessors"][accIndexId]["componentType"] = useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32;
    rootNode["accessors"][accIndexId]["count"] = indices.size();
    rootNode["accessors"][accIndexId]["type"] = "SCALAR";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange)
    {
    positionValue["min"] = Json::arrayValue;
    positionValue["min"].append(pointRange.low.x);
    positionValue["min"].append(pointRange.low.y);
    positionValue["min"].append(pointRange.low.z);
    positionValue["max"] = Json::arrayValue;
    positionValue["max"].append(pointRange.high.x);
    positionValue["max"].append(pointRange.high.y);
    positionValue["max"].append(pointRange.high.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMesh(Json::Value& rootNode, TileMeshR mesh, size_t index)
    {
    if (mesh.Triangles().empty())
        return;

    Utf8String          idStr(std::to_string(index).c_str()),
                        bvPositionId     = Concat("bvPosition_", idStr),
                        bvParamId        = Concat("bvParam_", idStr),
                        bvNormalId       = Concat("bvNormal_", idStr),
                        accPositionId    = Concat("accPosition_", idStr),
                        accParamId       = Concat("accParam_", idStr),
                        accNormalId      = Concat("accNormal_", idStr);
    bvector<uint32_t>   indices;

    BeAssert (mesh.Triangles().empty() || mesh.Polylines().empty());        // Meshes should contain either triangles or polylines but not both.

    if (!mesh.Triangles().empty())
        {
        for (auto const& tri : mesh.Triangles())
            {
           indices.push_back(tri.m_indices[0]);
           indices.push_back(tri.m_indices[1]);
           indices.push_back(tri.m_indices[2]);
           }
        }

    Json::Value         primitive = Json::objectValue;

    AddMeshBatchIds(rootNode, primitive, mesh, idStr);

    DRange3d        pointRange = DRange3d::From(mesh.Points());
    static bool     s_doQuantize = true;
    bool            quantizePositions = s_doQuantize, quantizeParams = s_doQuantize, quantizeNormals = s_doQuantize, isTextured = false;

    primitive["material"] = AddMeshMaterial (rootNode, isTextured, mesh.GetDisplayParams(), mesh, idStr.c_str());
    primitive["mode"] = GLTF_TRIANGLES;

    primitive["attributes"]["POSITION"] = accPositionId;
    AddMeshVertexAttribute (rootNode, &mesh.Points().front().x, bvPositionId, accPositionId, 3, mesh.Points().size(), "VEC3", quantizePositions, &pointRange.low.x, &pointRange.high.x);

    BeAssert (isTextured == !mesh.Params().empty());
    if (!mesh.Params().empty() && isTextured)
        {
        primitive["attributes"]["TEXCOORD_0"] = accParamId;

        DRange3d        paramRange = DRange3d::From(mesh.Params(), 0.0);
        AddMeshVertexAttribute (rootNode, &mesh.Params().front().x, bvParamId, accParamId, 2, mesh.Params().size(), "VEC2", quantizeParams, &paramRange.low.x, &paramRange.high.x);
        }


    if (!mesh.Normals().empty() &&
        nullptr != mesh.GetDisplayParams() && !mesh.GetDisplayParams()->GetIgnoreLighting())        // No normals if ignoring lighting (reality meshes).
        {
        DRange3d        normalRange = DRange3d::From (-1.0, -1.0, -1.0, 1.0, 1.0, 1.0); 

        primitive["attributes"]["NORMAL"] = accNormalId;
        AddMeshVertexAttribute (rootNode, &mesh.Normals().front().x, bvNormalId, accNormalId, 3, mesh.Normals().size(), "VEC3", quantizeNormals, &normalRange.low.x, &normalRange.high.x);
        }


    AddMeshIndices (rootNode, primitive, indices, idStr);
    AddMeshPointRange(rootNode["accessors"][accPositionId], pointRange);

    rootNode["meshes"]["mesh_0"]["primitives"].append(primitive);
    rootNode["buffers"]["binary_glTF"]["byteLength"] = m_binaryData.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddPolylines(Json::Value& rootNode, TileMeshR mesh, size_t index)
    {
    if (mesh.Polylines().empty())
        return;

    Utf8String idStr(std::to_string(index).c_str());

    Utf8String bvPositionId     = Concat("bvPosition_", idStr),
               bvPreviousId     = Concat("bvPrev_", idStr),
               bvNextId         = Concat("bvNext_", idStr),
               bvWidthId        = Concat("bvWidth_", idStr),
               accPositionId    = Concat("accPosition_", idStr),
               accPreviousId    = Concat("accPrevious_", idStr),
               accNextId        = Concat("accNext_", idStr),
               accWidthId       = Concat("accWidth_", idStr);

    bvector<double>             widths;
    bvector<DPoint3d>           points, previous, next;
    bvector<DPoint3d> const&    meshPoints = mesh.Points();
    static double               s_degenerateSegmentTolerance = 1.0E-5;
    static double               s_testWidth = 5;
    bvector<uint32_t>           indices;

    BeAssert (mesh.Polylines().empty());        // Meshes should contain either triangles or polylines but not both.

    for (auto const& polyline : mesh.Polylines())
        {
        for (size_t i=0; i<polyline.m_indices.size()-1; i++)
            {
            DPoint3d        p0 = meshPoints[polyline.m_indices[i]], 
                            p1 = meshPoints[polyline.m_indices[i+1]];

            if (p0.IsEqual(p1, s_degenerateSegmentTolerance))
                continue;

            indices.push_back(points.size());
            indices.push_back(points.size() + 2);
            indices.push_back(points.size() + 1);

            indices.push_back(points.size() + 1);
            indices.push_back(points.size() + 2);
            indices.push_back(points.size() + 3);

            widths.push_back(-s_testWidth);
            widths.push_back(s_testWidth);
            widths.push_back(-s_testWidth);
            widths.push_back(s_testWidth);

            points.push_back(p0);
            points.push_back(p0);
            points.push_back(p1);
            points.push_back(p1);

            DPoint3d    previousPoint = (i == 0) ? DPoint3d::FromSumOf (p0, 2.0, p1, -1.0) : meshPoints[polyline.m_indices[i-1]];
            DPoint3d    nextPoint     = (i == polyline.m_indices.size() - 2) ? DPoint3d::FromSumOf(p1, 2.0, p0, -1.0) : meshPoints[polyline.m_indices[i+2]];

            previous.push_back (previousPoint);
            previous.push_back (previousPoint);
            previous.push_back (p0);
            previous.push_back (p0);

            next.push_back (p1);
            next.push_back (p1);
            next.push_back (nextPoint);
            next.push_back (nextPoint);
            }
        }

    Json::Value         primitive = Json::objectValue;
    AddMeshBatchIds(rootNode, primitive, mesh, idStr);

    DRange3d        pointRange = DRange3d::From(points);

    pointRange.Extend (previous.front());
    pointRange.Extend (next.back());

    static bool     s_doQuantize = false;

    primitive["material"] = AddPolylineMaterial (rootNode, mesh.GetDisplayParams(), mesh, idStr.c_str());
    primitive["mode"] = GLTF_TRIANGLES;

    primitive["attributes"]["POSITION"] = accPositionId;
    primitive["attributes"]["PREVIOUS"] = accPreviousId;
    primitive["attributes"]["NEXT"] = accNextId;
    primitive["attributes"]["WIDTH"] = accWidthId;
    AddMeshVertexAttribute (rootNode, &points.front().x, bvPositionId, accPositionId, 3, points.size(), "VEC3", s_doQuantize, &pointRange.low.x, &pointRange.high.x);
    AddMeshVertexAttribute (rootNode, &previous.front().x, bvPreviousId, accPreviousId, 3,previous.size(), "VEC3", s_doQuantize, &pointRange.low.x, &pointRange.high.x);
    AddMeshVertexAttribute (rootNode, &next.front().x, bvNextId, accNextId, 3, next.size(), "VEC3", s_doQuantize, &pointRange.low.x, &pointRange.high.x);
    AddMeshVertexAttribute (rootNode, &widths.front(), bvWidthId, accWidthId, 1, widths.size(), "SCALAR", false, nullptr, nullptr);

    AddMeshIndices (rootNode, primitive, indices, idStr);
    AddMeshPointRange(rootNode["accessors"][accPositionId], pointRange);

    rootNode["meshes"]["mesh_0"]["primitives"].append(primitive);
    rootNode["buffers"]["binary_glTF"]["byteLength"] = m_binaryData.size();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint3d  cartesianFromRadians (double longitude, double latitude, double height = 0.0)
    {
    DPoint3d    s_wgs84RadiiSquared = DPoint3d::From (6378137.0 * 6378137.0, 6378137.0 * 6378137.0, 6356752.3142451793 * 6356752.3142451793);
    double      cosLatitude = cos(latitude);
    DPoint3d    normal, scratchK;

    normal.x = cosLatitude * cos(longitude);
    normal.y = cosLatitude * sin(longitude);
    normal.z = sin(latitude);

    normal.Normalize();
    scratchK.x = normal.x * s_wgs84RadiiSquared.x;
    scratchK.y = normal.y * s_wgs84RadiiSquared.y;
    scratchK.z = normal.z * s_wgs84RadiiSquared.z;

    double  gamma = sqrt(normal.DotProduct (scratchK));

    DPoint3d    earthPoint = DPoint3d::FromScale(scratchK, 1.0 / gamma);
    DPoint3d    heightDelta = DPoint3d::FromScale (normal, height);

    return DPoint3d::FromSumOf (earthPoint, heightDelta);
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherContext::IsGeolocated () const
    {
    return nullptr != GetDgnDb().Units().GetDgnGCS();
    }
    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::PublisherContext(ViewControllerR view, BeFileNameCR outputDir, WStringCR tilesetName,  GeoPointCP geoLocation, bool publishPolylines, size_t maxTilesetDepth, bool publishIncremental)
    : m_viewController(view), m_outputDir(outputDir), m_rootName(tilesetName), m_publishPolylines (publishPolylines), m_maxTilesetDepth (maxTilesetDepth), m_publishIncremental (publishIncremental)
    {
    // By default, output dir == data dir. data dir is where we put the json/b3dm files.
    m_outputDir.AppendSeparator();
    m_dataDir = m_outputDir;

    // For now use view center... maybe should use the DgnDb range center.
    DPoint3d        origin = m_viewController.GetCenter ();

    m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);

    DgnGCS*         dgnGCS = m_viewController.GetDgnDb().Units().GetDgnGCS();
    DPoint3d        ecfOrigin, ecfNorth;

    if (nullptr == dgnGCS)
        {
        double  longitude = -75.686844444444444444444444444444, latitude = 40.065702777777777777777777777778;

        if (nullptr != geoLocation)
            {
            longitude = geoLocation->longitude;
            latitude  = geoLocation->latitude;
            }

        // NB: We have to translate to surface of globe even if we're not using the globe, because
        // Cesium's camera freaks out if it approaches the origin (aka the center of the earth)

        ecfOrigin = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, latitude * msGeomConst_radiansPerDegree);
        ecfNorth  = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, 1.0E-4 + latitude * msGeomConst_radiansPerDegree);
        }
    else
        {
        GeoPoint        originLatLong, northLatLong;
        DPoint3d        north = origin;
    
        north.y += 100.0;

        dgnGCS->LatLongFromUors (originLatLong, origin);
        dgnGCS->XYZFromLatLong(ecfOrigin, originLatLong);

        dgnGCS->LatLongFromUors (northLatLong, north);
        dgnGCS->XYZFromLatLong(ecfNorth, northLatLong);
        }


    DVec3d      zVector, yVector;
    RotMatrix   rMatrix;

    zVector.Normalize ((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference (ecfNorth, ecfOrigin);

    rMatrix.SetColumn (yVector, 1);
    rMatrix.SetColumn (zVector, 2);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

    m_tileToEcef =  Transform::From (rMatrix, ecfOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::InitializeDirectories(BeFileNameCR dataDir)
    {
    // Ensure directories exist and are writable
    if (m_outputDir != dataDir && BeFileNameStatus::Success != BeFileName::CheckAccess(m_outputDir, BeFileNameAccess::Write))
        return Status::CantWriteToBaseDirectory;

    bool dataDirExists = BeFileName::DoesPathExist(dataDir);
    if (dataDirExists && !m_publishIncremental && BeFileNameStatus::Success != BeFileName::EmptyDirectory(dataDir.c_str()))
        return Status::CantCreateSubDirectory;
    else if (!dataDirExists && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dataDir))
        return Status::CantCreateSubDirectory;

    if (BeFileNameStatus::Success != BeFileName::CheckAccess(dataDir, BeFileNameAccess::Write))
        return Status::CantCreateSubDirectory;

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::CleanDirectories(BeFileNameCR dataDir)
    {
    BeFileName::EmptyAndRemoveDirectory (dataDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::ConvertStatus(TileGeneratorStatus input)
    {
    switch (input)
        {
        case TileGeneratorStatus::Success:        return Status::Success;
        case TileGeneratorStatus::NoGeometry:     return Status::NoGeometry;
        default: BeAssert(TileGeneratorStatus::Aborted == input); return Status::Aborted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PublisherContext::ConvertStatus(Status input)
    {
    switch (input)
        {
        case Status::Success:       return TileGeneratorStatus::Success;
        case Status::NoGeometry:    return TileGeneratorStatus::NoGeometry;
        default:                    return TileGeneratorStatus::Aborted;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteMetadataTree (DRange3dR range, Json::Value& root, TileNodeCR tile, size_t depth)
    {
    if (tile.GetIsEmpty())
        {
        range = DRange3d::NullRange();
        return;
        }

    WString rootName;
    BeFileName dataDir = GetDataDirForModel(tile.GetModel(), &rootName);

    DRange3d        contentRange, publishedRange = tile.GetPublishedRange();

    // If we are publishing standalone datasets then the tiles are all published before we write the metadata tree.
    // In that case we can trust the published ranges and use them to only write non-empty nodes and branches.
    // In the server case we don't have this information and have to trust the tile ranges.  
    if (!_AllTilesPublished() && publishedRange.IsNull())
        publishedRange = tile.GetTileRange();
    
    // the published range represents the actual range of the published meshes. - This may be smaller than the 
    // range estimated when we built the tile tree. -- However we do not clip the meshes to the tile range.
    // so start the range out as the intersection of the tile range and the published range.
    range = contentRange = DRange3d::FromIntersection (tile.GetTileRange(), publishedRange, true);

    if (!tile.GetChildren().empty())
        {
        root[JSON_Children] = Json::arrayValue;
#ifdef LIMIT_TILESET_DEPTH                     // I believe this should not be necessary now that we are not combining models into a single tileset. 
        if (0 == --depth)
            {
            // Write children as seperate tilesets.
            for (auto& childTile : tile.GetChildren())
                {
                Json::Value         childTileset;
                DRange3d            childRange;

                childTileset["asset"]["version"] = "0.0";

                auto&       childRoot = childTileset[JSON_Root];
                WString     metadataRelativePath = childTile->GetFileName(rootName.c_str(), s_metadataExtension);
                BeFileName  metadataFileName (nullptr, dataDir.c_str(), metadataRelativePath.c_str(), nullptr);

                WriteMetadataTree (childRange, childRoot, *childTile, GetMaxTilesetDepth());
                if (!childRange.IsNull())
                    {
                    TileUtil::WriteJsonToFile (metadataFileName.c_str(), childTileset);

                    Json::Value         child;

                    child["refine"] = "replace";
                    child[JSON_GeometricError] = childTile->GetTolerance();
                    TilePublisher::WriteBoundingVolume(child, childRange);

                    child[JSON_Content]["url"] = Utf8String (metadataRelativePath.c_str()).c_str();
                    root[JSON_Children].append(child);
                    range.Extend (childRange);
                    }
                }
            }
        else
#endif
            {
            // Append children to this tileset.
            for (auto& childTile : tile.GetChildren())
                {
                Json::Value         child;
                DRange3d            childRange;

                WriteMetadataTree (childRange, child, *childTile, depth);
                if (!childRange.IsNull())
                    {
                    root[JSON_Children].append(child);
                    range.Extend (childRange);
                    }
                }
            }
        }
    if (range.IsNull())
        return;

    root["refine"] = "replace";
    root[JSON_GeometricError] = tile.GetTolerance();
    TilePublisher::WriteBoundingVolume(root, range);

    if (!contentRange.IsNull())
        {
        root[JSON_Content]["url"] = Utf8String(GetTileUrl(tile, s_binaryDataExtension));
        TilePublisher::WriteBoundingVolume (root[JSON_Content], contentRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteTileset (BeFileNameCR metadataFileName, TileNodeCR rootTile, size_t maxDepth)
    {
    Json::Value val;

    val["asset"]["version"] = "0.0";

    auto&       root = val[JSON_Root];
    DRange3d    rootRange;

    WriteMetadataTree (rootRange, root, rootTile, maxDepth);
    TileUtil::WriteJsonToFile (metadataFileName.c_str(), val);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherContext::_DoIncrementalModelPublish (BeFileNameR dataDirectory, DgnModelCR model)
    {
    if (!m_publishIncremental)
        return false;

    dataDirectory = GetDataDirForModel(model);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PublisherContext::_BeginProcessModel(DgnModelCR model)
    {
    return Status::Success == InitializeDirectories(GetDataDirForModel(model)) ? TileGeneratorStatus::Success : TileGeneratorStatus::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PublisherContext::_EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status)
    {
    if (TileGeneratorStatus::Success == status)
        {
        BeAssert(nullptr != rootTile);
        BeMutexHolder lock(m_mutex);
        m_modelRoots.push_back(rootTile);
        }
    else if (!m_publishIncremental)
        {
        CleanDirectories(GetDataDirForModel(model));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName PublisherContext::GetDataDirForModel(DgnModelCR model, WStringP pTilesetName) const
    {
    WString tmpTilesetName;
    WStringR tilesetName = nullptr != pTilesetName ? *pTilesetName : tmpTilesetName;
    tilesetName = TileUtil::GetRootNameForModel(model);

    BeFileName dataDir = m_dataDir;
    dataDir.AppendToPath(tilesetName.c_str());

    return dataDir;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status   PublisherContext::PublishViewModels (TileGeneratorR generator, DRange3dR rootRange, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter)
    {
    auto spatialView = m_viewController._ToSpatialView();
    auto drawingView = m_viewController._ToDrawingView();

#ifndef WIP_2D_SUPPORT
    drawingView = nullptr;
#endif
    
    if (nullptr == spatialView && nullptr == drawingView)
        {
        BeAssert(false);
        return Status::NoGeometry;
        }

    DgnModelIdSet viewedModels;

    if (nullptr != spatialView)
        viewedModels = spatialView->GetViewedModels();
    else
        viewedModels.insert (drawingView->GetViewedModelId());

    Json::Value     value;
    value["asset"]["version"] = "0.0";

    auto& root = value[JSON_Root];

    if (!GetTileToEcef().IsIdentity())
        {
        DMatrix4d   matrix  = DMatrix4d::From (GetTileToEcef());
        auto&       transformValue = root[JSON_Transform];

        for (size_t i=0;i<4; i++)
            for (size_t j=0; j<4; j++)
                transformValue.append (matrix.coff[j][i]);
        }

    root["refine"] = "replace";
    root[JSON_GeometricError] = 1.E6;

    rootRange = DRange3d::NullRange();

    static size_t           s_maxPointsPerTile = 250000;
    auto status = generator.GenerateTiles(*this, viewedModels, toleranceInMeters, s_maxPointsPerTile);
    if (TileGeneratorStatus::Success != status)
        return ConvertStatus(status);

    if (m_modelRoots.empty())
        return Status::NoGeometry;

    for (auto childRootTile : m_modelRoots)
        {
        Json::Value childRoot;

        rootRange.Extend(childRootTile->GetTileRange());
        childRoot["refine"] = "replace";
        childRoot[JSON_GeometricError] = childRootTile->GetTolerance();
        TilePublisher::WriteBoundingVolume(childRoot, childRootTile->GetTileRange());

        WString modelRootName;
        BeFileName modelDataDir = GetDataDirForModel(childRootTile->GetModel(), &modelRootName);

        BeFileName      childTilesetFileName (nullptr, nullptr, modelRootName.c_str(), s_metadataExtension);
        childRoot[JSON_Content]["url"] = Utf8String (modelRootName + L"/" + childTilesetFileName.c_str()).c_str();

        WriteTileset (BeFileName(nullptr, modelDataDir.c_str(), childTilesetFileName.c_str(), nullptr), *childRootTile, GetMaxTilesetDepth());

        root[JSON_Children].append(childRoot);
        }

    m_modelRoots.clear();

    TilePublisher::WriteBoundingVolume(root, rootRange);

    BeFileName  metadataFileName (nullptr, GetDataDirectory().c_str(), m_rootName.c_str(), s_metadataExtension);

    TileUtil::WriteJsonToFile (metadataFileName.c_str(), value);

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetModelsJson (DgnModelIdSet const& modelIds)
    {
    Json::Value     modelJson (Json::objectValue);
    
    for (auto& modelId : modelIds)
        {
        auto const&  model = GetDgnDb().Models().GetModel (modelId);
        if (model.IsValid())
            modelJson[modelId.ToString()] = model->GetName();
        }

    return modelJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetCategoriesJson (DgnCategoryIdSet const& categoryIds)
    {
    Json::Value categoryJson (Json::objectValue); 
    
    for (auto& categoryId : categoryIds)
        {
        auto const& category = SpatialCategory::Get(GetDgnDb(), categoryId);

        if (category.IsValid())
            categoryJson[categoryId.ToString()] = category->GetCategoryName();
        }

    return categoryJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::GetViewJson (Json::Value& json, ViewDefinitionCR view, TransformCR transform)
    {
    CameraViewDefinitionCP          cameraView = view.ToCameraView();

#ifndef WIP_2D_SUPPORT
    OrthographicViewDefinitionCP    orthographicView = nullptr == cameraView ? view.ToOrthographicView() : nullptr;
    if (nullptr == cameraView && nullptr == orthographicView)
        {
        BeAssert(false && "unsupported view type");
        return;
        }
#endif

    json["name"] = view.GetName();

    auto spatialView = view.ToSpatialView();
    auto view2d = nullptr == spatialView ? dynamic_cast<ViewDefinition2dCP>(&view) : nullptr;
    if (nullptr != spatialView)
        {
        auto selectorId = spatialView->GetModelSelectorId().ToString();
        json["modelSelector"] = selectorId;
        }
    else if (nullptr != view2d)
        {
        auto fakeModelSelectorId = view2d->GetBaseModelId().ToString();
        fakeModelSelectorId.append("_2d");
        json["modelSelector"] = fakeModelSelectorId;
        }

    json["categorySelector"] = view.GetCategorySelectorId().ToString();
    json["displayStyle"] = view.GetDisplayStyleId().ToString();

    DPoint3d viewOrigin = view.GetOrigin();
    transform.Multiply(viewOrigin);
    json["origin"] = PointToJson(viewOrigin);
    
    DVec3d viewExtents = view.GetExtents();
    json["extents"] = PointToJson(viewExtents);

    DVec3d xVec, yVec, zVec;
    view.GetRotation().GetRows(xVec, yVec, zVec);
    transform.MultiplyMatrixOnly(xVec);
    transform.MultiplyMatrixOnly(yVec);
    transform.MultiplyMatrixOnly(zVec);

    RotMatrix columnMajorRotation = RotMatrix::FromColumnVectors(xVec, yVec, zVec);
    auto& rotJson = (json["rotation"] = Json::arrayValue);
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            rotJson.append(columnMajorRotation.form3d[i][j]);

    if (nullptr != cameraView)
        {
        json["type"] = "camera";

        DPoint3d eyePoint = cameraView->GetEyePoint();
        transform.Multiply(eyePoint);
        json["eyePoint"] = PointToJson(eyePoint);

        json["lensAngle"] = cameraView->GetLensAngle();
        json["focusDistance"] = cameraView->GetFocusDistance();
        }
    else
        {
        json["type"] = "ortho";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::GetViewsetJson(Json::Value& json, TransformCR transform, DPoint3dCR groundPoint)
    {
    Utf8String rootNameUtf8(m_rootName.c_str());
    json["name"] = rootNameUtf8;

    if (!m_tileToEcef.IsIdentity())
        {
        DPoint3d groundEcefPoint;
        transform.Multiply(groundEcefPoint, groundPoint);
        json["groundPoint"] = PointToJson(groundEcefPoint);
        }

    DgnViewId defaultViewId;
    DgnElementIdSet allModelSelectors;
    DgnElementIdSet allCategorySelectors;
    DgnElementIdSet allDisplayStyles;
    DgnModelIdSet all2dModelIds;

    auto& viewsJson = (json["views"] = Json::objectValue);
    for (auto& view : ViewDefinition::MakeIterator(GetDgnDb()))
        {
        auto viewDefinition = ViewDefinition::Get(GetDgnDb(), view.GetId());
        if (!viewDefinition.IsValid())
            continue;

        auto spatialView = viewDefinition->ToSpatialView();

#ifndef WIP_2D_SUPPORT
        if (nullptr == spatialView)
            continue;
#endif
    
        auto view2d = nullptr == spatialView ? dynamic_cast<ViewDefinition2dCP>(viewDefinition.get()) : nullptr;
        if (nullptr != spatialView)
            allModelSelectors.insert(spatialView->GetModelSelectorId());
        else if (nullptr != view2d)
            all2dModelIds.insert(view2d->GetBaseModelId());

        Json::Value entry(Json::objectValue);
 
        allCategorySelectors.insert(viewDefinition->GetCategorySelectorId());
        allDisplayStyles.insert(viewDefinition->GetDisplayStyleId());

        GetViewJson(entry, *viewDefinition, transform);
        viewsJson[view.GetId().ToString()] = entry;

        // If for some reason the default view is not in the published set, we'll use the first view as the default
        if (!defaultViewId.IsValid() || view.GetId() == GetViewController().GetViewId())
            defaultViewId = view.GetId();
        }

    if (!defaultViewId.IsValid())
        return Status::NoGeometry;

    json["defaultView"] = defaultViewId.ToString();

    WriteModelsJson(json, allModelSelectors, all2dModelIds);
    WriteCategoriesJson(json, allCategorySelectors);
    json["displayStyles"] = GetDisplayStylesJson(allDisplayStyles);

    AxisAlignedBox3d projectExtents = GetDgnDb().Units().GetProjectExtents();
    transform.Multiply(projectExtents, projectExtents);
    json["projectExtents"]["low"] = PointToJson(projectExtents.low);
    json["projectExtents"]["high"] = PointToJson(projectExtents.high);

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteModelsJson(Json::Value& json, DgnElementIdSet const& allModelSelectors, DgnModelIdSet const& all2dModels)
    {
    DgnModelIdSet allModels = all2dModels;
    Json::Value& selectorsJson = (json["modelSelectors"] = Json::objectValue);
    for (auto const& selectorId : allModelSelectors)
        {
        auto selector = GetDgnDb().Elements().Get<ModelSelector>(selectorId);
        if (selector.IsValid())
            {
            auto models = selector->GetModels();
            selectorsJson[selectorId.ToString()] = IdSetToJson(models);
            allModels.insert(models.begin(), models.end());
            }
        }

    // create a fake model selector for each 2d model
    for (auto const& modelId : all2dModels)
        {
        DgnModelIdSet modelIdSet;
        modelIdSet.insert(modelId);
        selectorsJson[modelId.ToString()+"_2d"] = IdSetToJson(modelIdSet);
        }

    json["models"] = GetModelsJson(allModels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteCategoriesJson(Json::Value& json, DgnElementIdSet const& selectorIds)
    {
    DgnCategoryIdSet allCategories;
    Json::Value& selectorsJson = (json["categorySelectors"] = Json::objectValue);
    for (auto const& selectorId : selectorIds)
        {
        auto selector = GetDgnDb().Elements().Get<CategorySelector>(selectorId);
        if (selector.IsValid())
            {
            auto cats = selector->GetCategories();
            selectorsJson[selectorId.ToString()] = IdSetToJson(cats);
            allCategories.insert(cats.begin(), cats.end());
            }
        }

    json["categories"] = GetCategoriesJson(allCategories);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetDisplayStylesJson(DgnElementIdSet const& styleIds)
    {
    Json::Value json(Json::objectValue);
    for (auto const& styleId : styleIds)
        {
        auto style = GetDgnDb().Elements().Get<DisplayStyle>(styleId);
        if (style.IsValid())
            json[styleId.ToString()] = GetDisplayStyleJson(*style);
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetDisplayStyleJson(DisplayStyleCR style)
    {
    Json::Value json(Json::objectValue);

    ColorDef bgColor = style.GetBackgroundColor();
    auto& bgColorJson = (json["backgroundColor"] = Json::objectValue);
    bgColorJson["red"] = bgColor.GetRed() / 255.0;
    bgColorJson["green"] = bgColor.GetGreen() / 255.0;
    bgColorJson["blue"] = bgColor.GetBlue() / 255.0;

    // ###TODO: skybox, ground plane, view flags...

    return json;
    }


