/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "TilePublisher/TilePublisher.h"
#include "Constants.h"
#include <limits>

//#include "..\STM\ScalableMeshQuery.h"


//USING_NAMESPACE_BENTLEY_DGN
//USING_NAMESPACE_BENTLEY_RENDER
//using namespace BentleyApi::Dgn::Render::Tile3d;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

BentleyStatus resize_image (ByteStream&    outputImage, Point2dCR outputSize, Byte const*  inputImage, Point2dCR  inputSize, bool isRGBA);

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
void BatchIdMap::ToJson(Json::Value& value) const
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
        assert(false); // not implemented;
            //if (db)
            //    {
            //    //// ###TODO: Assumes 3d-only...
            //    //// There's no longer a simple way to query the category of an arbitrary geometric element without knowing whether it's 2d or 3d...
            //    //static const Utf8CP s_sql = "SELECT e.ModelId,g.CategoryId FROM " BIS_TABLE(BIS_CLASS_Element) " AS e, " BIS_TABLE(BIS_CLASS_GeometricElement3d) " AS g "
            //    //    "WHERE e.Id=? AND g.ElementId=e.Id";
            //    //
            //    //BeSQLite::Statement stmt;
            //    //stmt.Prepare(db, s_sql);
            //    //
            //    //Json::Value elementIds(Json::arrayValue);
            //    //Json::Value modelIds(Json::arrayValue);
            //    //Json::Value categoryIds(Json::arrayValue);
            //    //
            //    //for (auto elemIter = m_list.begin(); elemIter != m_list.end(); ++elemIter)
            //    //    {
            //    //    elementIds.append(elemIter->ToString());    // NB: Javascript doesn't support full range of 64-bit integers...must convert to strings...
            //    //    DgnModelId modelId;
            //    //    DgnCategoryId categoryId;
            //    //
            //    //    stmt.BindId(1, *elemIter);
            //    //    if (BeSQLite::BE_SQLITE_ROW == stmt.Step())
            //    //        {
            //    //        modelId = stmt.GetValueId<DgnModelId>(0);
            //    //        categoryId = stmt.GetValueId<DgnCategoryId>(1);
            //    //        }
            //    //
            //    //    modelIds.append(modelId.ToString());
            //    //    categoryIds.append(categoryId.ToString());
            //    //    stmt.Reset();
            //    //    }
            //    //
            //    //value["element"] = elementIds;
            //    //value["model"] = modelIds;
            //    //value["category"] = categoryIds;
            //    }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::TilePublisher(TileNodeCR tile, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS)
    : m_batchIds(TileSource::None), m_centroid(tile.GetTileCenter()), m_tile(&tile), m_context(nullptr)
    {
    m_centroid = DPoint3d::From(0, 0, 0);
    m_meshes = m_tile->GenerateMeshes();
#if 0
    // 1. CESIUM_RTC does not work when root tileset contains a non-identity transform
    // 2. Point to point reprojection is not necessary
    // 3. TODO: Must investigate the use of RTC_CENTER instead
    if (!m_meshes.empty())
        {
        if (sourceGCS != nullptr && sourceGCS != destinationGCS && !destinationGCS->IsEquivalent(*sourceGCS))
            {
            m_meshes[0]->ReprojectPoints(sourceGCS, destinationGCS);

            GeoPoint inLatLong, outLatLong;
            if (sourceGCS->LatLongFromCartesian(inLatLong, m_centroid) != SUCCESS)
                assert(false);
            if (sourceGCS->LatLongFromLatLong(outLatLong, inLatLong, *destinationGCS) != SUCCESS)
                assert(false);
            if (destinationGCS->XYZFromLatLong(m_centroid, outLatLong) != SUCCESS)
                assert(false);
            }

        // Convert points to follow Y-up convention and translate to zero (avoids jittering for distant datasets)
        Transform transform = Transform::FromRowValues(1, 0, 0, -m_centroid.x,
                                                       0, 1, 0, -m_centroid.y,
                                                       0, 0, 1, -m_centroid.z);
        
        m_meshes[0]->ApplyTransform(transform);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::TilePublisher(TileNodeCR tile, PublisherContext& context)
    : m_batchIds(tile.GetSource()), m_centroid(tile.GetTileCenter()), m_tile(&tile), m_context(&context), m_outputFile(NULL)
    {
    m_meshes = m_tile->GenerateMeshes(/*context->GetCache(), context->GetDgnDb(),*/ TileGeometry::NormalMode::Always, false, context.WantPolylines());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AppendUInt32(uint32_t value)
    {
    std::fwrite(&value, 1, sizeof(value), m_outputFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList ScalableMeshTileNode::_GenerateMeshes(TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines) const
    {   
    TileMeshList        tileMeshes;
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(m_outputTexture, false, m_coverageID != -1);
    auto meshP = m_node->GetMeshUnderClip2(flags, m_clips, m_coverageID, m_isClipBoundary);
    if (!meshP.IsValid() || meshP->GetNbFaces() == 0) return tileMeshes;

    TileMeshBuilderPtr      builder;
    TileDisplayParamsPtr    displayParams;
    
    if (m_node->IsTextured() && m_outputTexture)
        {
        auto textureP = m_node->GetTextureCompressed();
        ImageSource jpgTex(ImageSource::Format::Jpeg, ByteStream(textureP->GetData(), (uint32_t)textureP->GetSize()));
        TileTextureImagePtr     tileTexture = TileTextureImage::Create(jpgTex);
        displayParams = TileDisplayParams::Create(0xffffff, tileTexture, true);
        }
    else
        {
        TileTextureImagePtr     tileTexture = nullptr;
        displayParams = TileDisplayParams::Create(0x007700, tileTexture, false);
        }
    builder = TileMeshBuilder::Create(displayParams, 0.0);
    builder->AddPolyface(*meshP->GetPolyfaceQuery(), false);

    tileMeshes.push_back(builder->GetMesh());

    for (auto& mesh : tileMeshes)
        {
        mesh->ApplyTransform(GetTransformFromDgn());
        }
    return tileMeshes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBoundingVolume(Json::Value& val, DRange3dCR range)
    {
    BeAssert (!range.IsNull());
    DPoint3d    center = DPoint3d::FromInterpolate (range.low, .5, range.high);
    DVec3d      diagonal = range.IsNull() ? DVec3d::From(0, 0, 0) : DVec3d::FromStartEnd(range.low, range.high);

    // Range identified by center point and axes
    // Axes are relative to origin of box, not center
    static double      s_minSize = .001;   // Meters...  Don't allow degenerate box.

    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    AppendPoint(box, center);
    AppendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, diagonal.x)/2.0, 0.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, diagonal.y)/2.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, diagonal.z)/2.0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBoundingVolume(Json::Value& val, DPoint3dCR center, RotMatrixCR halfAxes)
    {
    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    AppendPoint(box, center);
    AppendPoint(box, DPoint3d::FromXYZ (halfAxes.form3d[0][0], halfAxes.form3d[0][1], halfAxes.form3d[0][2]));
    AppendPoint(box, DPoint3d::FromXYZ (halfAxes.form3d[1][0], halfAxes.form3d[1][1], halfAxes.form3d[1][2]));
    AppendPoint(box, DPoint3d::FromXYZ (halfAxes.form3d[2][0], halfAxes.form3d[2][1], halfAxes.form3d[2][2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteJsonToFile (WCharCP fileName, Json::Value& value)
    {
    Utf8String  metadataStr = Json::FastWriter().write(value);
    auto        outputFile = std::fopen(Utf8String(fileName).c_str(), "w");

    std::fwrite(metadataStr.data(), 1, metadataStr.size(), outputFile);
    std::fclose(outputFile);
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
    program["attributes"].append(attrName);
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
    view["byteOffset"] = Json::Value(m_binaryData.size());
    view["byteLength"] = Json::Value(bufferDataSize);

    size_t binaryDataSize = m_binaryData.size();
    m_binaryData.resize(binaryDataSize + bufferDataSize);
    memcpy(m_binaryData.data() + binaryDataSize, bufferData.data(), bufferDataSize);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   08/16
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status TilePublisher::Publish()
//    {
//    if (m_meshes.empty())
//        return PublisherContext::Status::NoGeometry;       // Nothing to write...Ignore this tile (it will be omitted when writing tileset data as its published range will be NullRange.
//
//    BeFileName  binaryDataFileName (nullptr, GetDataDirectory().c_str(), m_tile->GetRelativePath (m_context->GetRootName().c_str(), s_binaryDataExtension).c_str(), nullptr);
//    
//    // .b3dm file
//    Json::Value sceneJson(Json::objectValue);
//
//    ProcessMeshes(sceneJson);
//
//    Utf8String sceneStr = Json::FastWriter().write(sceneJson);
//
//    Json::Value batchTableJson(Json::objectValue);
//#ifndef VANCOUVER_API
//    m_batchIds.ToJson(batchTableJson, m_context->GetDgnDb());
//#else
//    m_batchIds.ToJson(batchTableJson);
//#endif
//    Utf8String batchTableStr = Json::FastWriter().write(batchTableJson);
//    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
//
//    m_outputFile = std::fopen(Utf8String(binaryDataFileName.c_str()).c_str(), "wb");
//
//    // GLTF header = 5 32-bit values
//    static const size_t s_gltfHeaderSize = 20;
//    //static const char s_gltfMagic[] = "glTF";
//    //static const uint32_t s_gltfVersion = 1;
//    //static const uint32_t s_gltfSceneFormat = 0;
//    uint32_t sceneStrLength = static_cast<uint32_t>(sceneStr.size());
//    uint32_t gltfLength = s_gltfHeaderSize + sceneStrLength + m_binaryData.GetSize();
//
//    // B3DM header = 6 32-bit values
//    // Header immediately followed by batch table json
//    static const size_t s_b3dmHeaderSize = 24;
//    //static const char s_b3dmMagic[] = "b3dm";
//    //static const uint32_t s_b3dmVersion = 1;
//    uint32_t b3dmNumBatches = m_batchIds.Count();
//    uint32_t b3dmLength = gltfLength + s_b3dmHeaderSize + batchTableStrLen;
//
//    std::fwrite(s_b3dmMagic, 1, 4, m_outputFile);
//    AppendUInt32(s_b3dmVersion);
//    AppendUInt32(b3dmLength);
//    AppendUInt32(batchTableStrLen);
//    AppendUInt32(0); // length of binary portion of batch table - we have no binary batch table data
//    AppendUInt32(b3dmNumBatches);
//    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, m_outputFile);
//
//    std::fwrite(s_gltfMagic, 1, 4, m_outputFile);
//    AppendUInt32(s_gltfVersion);
//    AppendUInt32(gltfLength);
//    AppendUInt32(sceneStrLength);
//    AppendUInt32(s_gltfSceneFormat);
//
//    std::fwrite(sceneStr.data(), 1, sceneStrLength, m_outputFile);
//    if (!m_binaryData.empty())
//        std::fwrite(m_binaryData.data(), 1, m_binaryData.size(), m_outputFile);
//
//    std::fclose(m_outputFile);
//    m_outputFile = NULL;
//
//    return PublisherContext::Status::Success;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   08/16
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status TilePublisher::Publish(TileMeshR mesh, Utf8StringR sceneStr)
//    {
//
//    BeFileName  binaryDataFileName (nullptr, GetDataDirectory().c_str(), m_tile.IsValid() ? m_tile->GetRelativePath (m_context->GetRootName().c_str(), s_binaryDataExtension).c_str() : m_context->GetRootName().c_str(), nullptr);
//    
//    // .b3dm file
//    Json::Value sceneJson(Json::objectValue);
//
//    m_meshes.push_back(TileMeshPtr(&mesh));
//    ProcessMeshes(sceneJson);
//
//    sceneStr = Json::FastWriter().write(sceneJson);
//
//    Json::Value batchTableJson(Json::objectValue);
//    m_batchIds.ToJson(batchTableJson, m_context->GetDgnDb());
//    Utf8String batchTableStr = Json::FastWriter().write(batchTableJson);
//    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
//
//    m_outputFile = std::fopen(Utf8String(binaryDataFileName.c_str()).c_str(), "wb");
//
//    // GLTF header = 5 32-bit values
//    static const size_t s_gltfHeaderSize = 20;
//    static const char s_gltfMagic[] = "glTF";
//    static const uint32_t s_gltfVersion = 1;
//    static const uint32_t s_gltfSceneFormat = 0;
//    uint32_t sceneStrLength = static_cast<uint32_t>(sceneStr.size());
//    uint32_t gltfLength = s_gltfHeaderSize + sceneStrLength + m_binaryData.GetSize();
//
//    // B3DM header = 6 32-bit values
//    // Header immediately followed by batch table json
//    static const size_t s_b3dmHeaderSize = 24;
//    static const char s_b3dmMagic[] = "b3dm";
//    static const uint32_t s_b3dmVersion = 1;
//    uint32_t b3dmNumBatches = m_batchIds.Count();
//    uint32_t b3dmLength = gltfLength + s_b3dmHeaderSize + batchTableStrLen;
//
//    std::fwrite(s_b3dmMagic, 1, 4, m_outputFile);
//    AppendUInt32(s_b3dmVersion);
//    AppendUInt32(b3dmLength);
//    AppendUInt32(batchTableStrLen);
//    AppendUInt32(0); // length of binary portion of batch table - we have no binary batch table data
//    AppendUInt32(b3dmNumBatches);
//    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, m_outputFile);
//
//    std::fwrite(s_gltfMagic, 1, 4, m_outputFile);
//    AppendUInt32(s_gltfVersion);
//    AppendUInt32(gltfLength);
//    AppendUInt32(sceneStrLength);
//    AppendUInt32(s_gltfSceneFormat);
//
//    std::fwrite(sceneStr.data(), 1, sceneStrLength, m_outputFile);
//    if (!m_binaryData.empty())
//        std::fwrite(m_binaryData.data(), 1, m_binaryData.size(), m_outputFile);
//
//    std::fclose(m_outputFile);
//    m_outputFile = NULL;
//
//    return PublisherContext::Status::Success;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilePublisher::Publish(bvector<Byte>& outData)
    {
    if (m_meshes.empty()) return PublisherContext::Status::Success;
    if (m_meshes[0]->Points().empty()) return PublisherContext::Status::Success;
    return this->Publish(*m_meshes[0], outData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilePublisher::Publish(TileMeshR mesh, bvector<Byte>& outData)
    {
    // .b3dm file
    Json::Value sceneJson(Json::objectValue);
    ProcessMeshes(sceneJson);
    Utf8String sceneStr = Json::FastWriter().write(sceneJson);

    while (0 != sceneStr.size() % 4)
        sceneStr = sceneStr + " ";

    Json::Value featureTableJson(Json::objectValue);
    featureTableJson["BATCH_LENGTH"] = 0;

    Utf8String featureTableStr = Json::FastWriter().write(featureTableJson);
    uint32_t featureTableBinaryByteLength = 0;

    Json::Value batchTableJson(Json::objectValue);
    m_batchIds.ToJson(batchTableJson);
    Utf8String batchTableStr = batchTableJson.empty() ? Utf8String() : Json::FastWriter().write(batchTableJson);
    uint32_t batchTableJSONByteLength = static_cast<uint32_t>(batchTableStr.size());
    uint32_t batchTableBinaryByteLength = 0;

    // GLTF header = 5 32-bit values
    static const size_t s_gltfHeaderSize = 20;
    //static const uint32_t s_gltfSceneFormat = 0;
    uint32_t sceneStrLength = static_cast<uint32_t>(sceneStr.size());
    uint32_t gltfLength = s_gltfHeaderSize + sceneStrLength + m_binaryData.GetSize();

    // B3DM header = 6 32-bit values
    // Header immediately followed by batch table json
    static const size_t s_b3dmHeaderSize = 28;
    //static const char s_b3dmMagic[] = "b3dm";
    //static const uint32_t s_b3dmVersion = 1;

    // Pad to 8 byte boundary
    while ((s_b3dmHeaderSize + featureTableStr.size()) % 8 > 0)
        featureTableStr.append(" ");

    uint32_t featureTableJSONByteLength = static_cast<uint32_t>(featureTableStr.size());

    uint32_t b3dmLength = s_b3dmHeaderSize + featureTableJSONByteLength + featureTableBinaryByteLength + batchTableJSONByteLength + gltfLength;


    outData.resize(b3dmLength);
    uint32_t dataOffset = 0;

    // b3dm header
    memcpy(outData.data() + dataOffset, &s_b3dmMagic, 4);
    dataOffset += 4;
    memcpy(outData.data() + dataOffset, &s_b3dmVersion, sizeof(s_b3dmVersion));
    dataOffset += sizeof(s_b3dmVersion);
    memcpy(outData.data() + dataOffset, &b3dmLength, sizeof(b3dmLength));
    dataOffset += sizeof(b3dmLength);
    memcpy(outData.data() + dataOffset, &featureTableJSONByteLength, sizeof(featureTableJSONByteLength));
    dataOffset += sizeof(featureTableJSONByteLength);
    memcpy(outData.data() + dataOffset, &featureTableBinaryByteLength, sizeof(featureTableBinaryByteLength));
    dataOffset += sizeof(featureTableBinaryByteLength);
    memcpy(outData.data() + dataOffset, &batchTableJSONByteLength, sizeof(batchTableJSONByteLength));
    dataOffset += sizeof(batchTableJSONByteLength);
    memcpy(outData.data() + dataOffset, &batchTableBinaryByteLength, sizeof(batchTableBinaryByteLength));
    dataOffset += sizeof(batchTableBinaryByteLength);

    // feature table
    memcpy(outData.data() + dataOffset, featureTableStr.c_str(), featureTableJSONByteLength);
    dataOffset += featureTableJSONByteLength;

    memset(outData.data() + dataOffset, 0, featureTableBinaryByteLength);
    dataOffset += featureTableBinaryByteLength;

    // batch table
    memcpy(outData.data() + dataOffset, batchTableStr.c_str(), batchTableJSONByteLength);
    dataOffset += batchTableJSONByteLength;

    // gltf
    memcpy(outData.data() + dataOffset, &s_gltfMagic, 4);
    dataOffset += 4;
    memcpy(outData.data() + dataOffset, &s_gltfVersion, sizeof(s_gltfVersion));
    dataOffset += sizeof(s_gltfVersion);
    memcpy(outData.data() + dataOffset, &gltfLength, sizeof(gltfLength));
    dataOffset += sizeof(gltfLength);
    memcpy(outData.data() + dataOffset, &sceneStrLength, sizeof(sceneStrLength));
    dataOffset += sizeof(sceneStrLength);
    memcpy(outData.data() + dataOffset, &s_gltfSceneFormat, sizeof(s_gltfSceneFormat));
    dataOffset += sizeof(s_gltfSceneFormat);

    memcpy(outData.data() + dataOffset, sceneStr.c_str(), sceneStrLength);
    dataOffset += sceneStrLength;

    if (!m_binaryData.empty())
        {
        memcpy(outData.data() + dataOffset, m_binaryData.data(), m_binaryData.size());
        dataOffset += sizeof(m_binaryData.size());
        }

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
        publishedRange.Extend (m_meshes[i]->GetRange());
        }
    if (m_tile != nullptr)
        m_tile->SetPublishedRange(publishedRange);
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

    rootNode["asset"]["version"] = "1.0";
    rootNode["scene"] = "defaultScene";
    rootNode["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    rootNode["scenes"]["defaultScene"]["nodes"].append("node_0");
    rootNode["nodes"]["node_0"]["meshes"] = Json::arrayValue;
    rootNode["nodes"]["node_0"]["meshes"].append("mesh_0");
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     10/02016
//+---------------+---------------+---------------+---------------+---------------+------*/
//static int32_t  roundToMultipleOfTwo (int32_t value)
//    {
//    int32_t rounded = 2;
//    
//    while (rounded < value && rounded < 0x01000000)
//        rounded <<= 1;
//
//    return rounded;
//    }

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
    Image       image (textureImage.GetImageSource(), hasAlpha ? Image::Format::Rgba : Image::Format::Rgb, Image::BottomUp::No, true);


    rootNode["bufferViews"][bvImageId] = Json::objectValue;
    rootNode["bufferViews"][bvImageId]["buffer"] = "binary_glTF";
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = image.GetHeight();
    rootNode["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = image.GetWidth();

    ByteStream const& imageData = textureImage.GetImageSource().GetByteStream();
    rootNode["bufferViews"][bvImageId]["byteOffset"] = Json::Value(m_binaryData.size());
    rootNode["bufferViews"][bvImageId]["byteLength"] = Json::Value(imageData.size());

    AddBinaryData(imageData.data(), imageData.size());

    m_textureImages.Insert (&textureImage, textureId);

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
    //AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "_BATCHID");

    static char const   *s_programName                    = "unlitProgram",
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
    //techniqueAttributes["a_batchId"] = "batch";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";

    auto& rootProgramNode = (rootNode["programs"][s_programName] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");
    //AppendProgramAttribute(rootProgramNode, "a_batchId");

    rootProgramNode["vertexShader"]   = s_vertexShaderName;
    rootProgramNode["fragmentShader"] = s_fragmentShaderName;

    auto& shaders = rootNode["shaders"];
    AddShader (shaders, s_vertexShaderName, GLTF_VERTEX_SHADER, s_vertexShaderBufferViewName);
    AddShader (shaders, s_fragmentShaderName, GLTF_FRAGMENT_SHADER, s_fragmentShaderBufferViewName);

    auto& bufferViews = rootNode["bufferViews"];
    std::string vertexShaderString = s_shaderPrecision + s_unlitVertexShader;
    AddBufferView(bufferViews, s_vertexShaderBufferViewName, vertexShaderString);
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
    //AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "_BATCHID");

    Utf8String         programName               = prefix + "Program";
    Utf8String         vertexShader              = prefix + "VertexShader";
    Utf8String         fragmentShader            = prefix + "FragmentShader";
    Utf8String         vertexShaderBufferView    = vertexShader + "BufferView";
    Utf8String         fragmentShaderBufferView  = fragmentShader + "BufferView";

    technique["program"] = programName.c_str();

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(GLTF_DEPTH_TEST);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";
    //techniqueAttributes["a_batchId"] = "batch";
    if(!ignoreLighting)
        techniqueAttributes["a_n"] = "n";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";
    if (!ignoreLighting)
        techniqueUniforms["u_nmx"] = "nmx";

    auto& rootProgramNode = (rootNode["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::Value();
    AppendProgramAttribute(rootProgramNode, "a_pos");
    //AppendProgramAttribute(rootProgramNode, "a_batchId");
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
Utf8String TilePublisher::AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix)
    {
    Utf8String      materialName = Utf8String ("Material_") + suffix;

    if (nullptr == displayParams)
        return materialName;


    RgbFactor       specularColor = { 1.0, 1.0, 1.0 };
    double          specularExponent = s_qvFinish * s_qvExponentMultiplier;
    uint32_t        rgbInt  = displayParams->GetFillColor();
    double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
    Json::Value&    materialValue = rootNode["materials"][materialName.c_str()] = Json::objectValue;
    bool            isPolyline = mesh.Triangles().empty();
    RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);

    //if (!isPolyline && displayParams->GetMaterialId().IsValid())
    //    {
    //    JsonRenderMaterial  jsonMaterial;
    //
    //    if (SUCCESS == jsonMaterial.Load (displayParams->GetMaterialId(), m_context.GetDgnDb()))
    //        {
    //        static double       s_finishScale = 15.0;
    //
    //        if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasSpecularColor, false))
    //            specularColor = jsonMaterial.GetColor (RENDER_MATERIAL_SpecularColor);
    //
    //        if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasFinish, false))
    //            specularExponent = jsonMaterial.GetDouble (RENDER_MATERIAL_Finish, s_qvSpecular) * s_finishScale;
    //
    //        if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasBaseColor, false))
    //            rgb = jsonMaterial.GetColor (RENDER_MATERIAL_Color);
    //
    //        if (jsonMaterial.GetBool (RENDER_MATERIAL_FlagHasTransmit, false))
    //            alpha = 1.0 - jsonMaterial.GetDouble (RENDER_MATERIAL_Transmit, 0.0);
    //        }
    //    }
    //
    TileTextureImageCP      textureImage;

    if (!isPolyline && nullptr != (textureImage = displayParams->GetTextureImage()))
        {
        materialValue["technique"] = AddMeshShaderTechnique (rootNode, true, alpha < 1.0, displayParams->GetIgnoreLighting()).c_str();
        materialValue["values"]["tex"] = AddTextureImage (rootNode, *textureImage, mesh, suffix);
        }
    else
        {
        auto&           materialColor = materialValue["values"]["color"] = Json::arrayValue;

        materialColor.append(rgb.red);
        materialColor.append(rgb.green);
        materialColor.append(rgb.blue);
        materialColor.append(alpha);

        materialValue["technique"] = isPolyline ? AddPolylineShaderTechnique (rootNode).c_str() : AddMeshShaderTechnique(rootNode, false, alpha < 1.0, displayParams->GetIgnoreLighting()).c_str();
        }

    if (!isPolyline && !displayParams->GetIgnoreLighting())
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
void    TilePublisher::AddBinaryData (void const* data, size_t size)
    {
    size_t currentBufferSize = m_binaryData.size();
    m_binaryData.resize(m_binaryData.size() + size);
    memcpy(m_binaryData.data() + currentBufferSize, data, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    TilePublisher::PadBinaryDataToBoundary(size_t boundarySize)
    {
    uint8_t        zero = 0;
    while (0 != (m_binaryData.size() % boundarySize))
        m_binaryData.Append(&zero, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshVertexAttribute (Json::Value& rootNode, double const* values, Utf8StringCR bufferViewId, Utf8StringCR accesorId, size_t nComponents, size_t nAttributes, char* accessorType, bool quantize, double const* min, double const* max, unsigned short* qmin, unsigned short* qmax)
    {
    PadBinaryDataToBoundary(4);

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
            if (qmin && qmax)
                {
                qmin[i] = std::numeric_limits<unsigned short>::max();
                qmax[i] = std::numeric_limits<unsigned short>::min();
                }
            }

        bvector <unsigned short>    quantizedValues;

        for (size_t i=0; i<nValues; i++)
            {
            size_t  componentIndex = i % nComponents;
            auto qVal = (unsigned short)(.5 + (values[i] - min[componentIndex]) * range / (max[componentIndex] - min[componentIndex]));
            if (qmin && qVal < qmin[componentIndex]) qmin[componentIndex] = qVal;
            if (qmax && qVal > qmax[componentIndex]) qmax[componentIndex] = qVal;
            quantizedValues.push_back (qVal);
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
    bufferViews["byteOffset"] = Json::Value(byteOffset);
    bufferViews["byteLength"] = Json::Value(dataSize);
    bufferViews["target"] = GLTF_ARRAY_BUFFER;

    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = Json::Value(nAttributes);
    accessor["type"] = accessorType;

    rootNode["bufferViews"][bufferViewId] = bufferViews;
    rootNode["accessors"][accesorId] = accessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMesh(Json::Value& rootNode, TileMeshR mesh, size_t index)
    {
    Utf8String idStr(std::to_string(index).c_str());

    Utf8String bvPositionId     = Concat("bvPosition_", idStr),
               bvIndexId        = Concat("bvIndex_", idStr),
               bvParamId        = Concat("bvParam_", idStr),
               bvNormalId       = Concat("bvNormal_", idStr),
               bvBatchId        = Concat("bvBatch_", idStr),
               accPositionId    = Concat("accPosition_", idStr),
               accIndexId       = Concat("accIndex_", idStr),
               accParamId       = Concat("accParam_", idStr),
               accNormalId      = Concat("accNormal_", idStr),
               accBatchId       = Concat("accBatch_", idStr);

    bvector<uint32_t>   indices;
    bvector<uint16_t>   shortIndices;
    bool                useShortIndices = mesh.Points().size() <= 0xffff;

    BeAssert (mesh.Triangles().empty() || mesh.Polylines().empty());        // Meshes should contain either triangles or polylines but not both.

    if (!mesh.Triangles().empty())
        {
        for (auto const& tri : mesh.Triangles())
            {
            if (useShortIndices)
                {
                shortIndices.push_back((uint16_t)tri.m_indices[0]);
                shortIndices.push_back((uint16_t)tri.m_indices[1]);
                shortIndices.push_back((uint16_t)tri.m_indices[2]);
                }
            else
                {
                indices.push_back(tri.m_indices[0]);
                indices.push_back(tri.m_indices[1]);
                indices.push_back(tri.m_indices[2]);
                }
            }
        }
    else if (!mesh.Polylines().empty())
        {
        for (auto const& polyline : mesh.Polylines())
            {
            for (size_t i=0; i<polyline.m_indices.size()-1; i++)
                {
                if (useShortIndices)
                    {
                    shortIndices.push_back ((uint16_t)polyline.m_indices[i]);
                    shortIndices.push_back ((uint16_t)polyline.m_indices[i+1]);
                    }
                else
                    {
                    indices.push_back (polyline.m_indices[i]);
                    indices.push_back (polyline.m_indices[i+1]);
                    }
                }
            }
        }
    else
        {
        BeAssert(!"Nothing to publish...");
        }


    bvector<uint16_t>   batchIds;
    Json::Value         attr = Json::objectValue;

    if (mesh.ValidIdsPresent())
        {
        batchIds.reserve(mesh.EntityIds().size());
        for (auto const& elemId : mesh.EntityIds())
            batchIds.push_back(m_batchIds.GetBatchId(elemId));

        attr["attributes"]["BATCHID"] = accBatchId;
        }

    DRange3d        pointRange = DRange3d::From(mesh.Points());
    static bool     s_doQuantize = true;
    bool            quantizePositions = s_doQuantize, quantizeParams = s_doQuantize, quantizeNormals = s_doQuantize;

    attr["indices"] = accIndexId;
    attr["material"] = AddMaterial (rootNode, mesh.GetDisplayParams(), mesh, idStr.c_str());
    attr["mode"] = mesh.Triangles().empty() ? GLTF_LINES : GLTF_TRIANGLES;

    attr["attributes"]["POSITION"] = accPositionId;

    unsigned short qmin[3], qmax[3];
    AddMeshVertexAttribute (rootNode, &mesh.Points().front().x, bvPositionId, accPositionId, 3, mesh.Points().size(), (char*)"VEC3", quantizePositions, &pointRange.low.x, &pointRange.high.x, &qmin[0], &qmax[0]);

    if (!mesh.Params().empty())
        {
        attr["attributes"]["TEXCOORD_0"] = accParamId;

        size_t i=0;
        bvector<DPoint2d> flippedUvs (mesh.Params().size());
        for (auto const& uv : mesh.Params())
            flippedUvs[i++] = DPoint2d::From (uv.x, 1.0 - uv.y);      // Needs work - flip textures rather than params.

        DRange3d        paramRange = DRange3d::From(flippedUvs, 0.0);
        AddMeshVertexAttribute (rootNode, &flippedUvs.front().x, bvParamId, accParamId, 2, mesh.Params().size(), (char*)"VEC2", quantizeParams, &paramRange.low.x, &paramRange.high.x);
        }


    if (!mesh.Normals().empty() &&
        nullptr != mesh.GetDisplayParams() && !mesh.GetDisplayParams()->GetIgnoreLighting())        // No normals if ignoring lighting (reality meshes).
        {
        DRange3d        normalRange = DRange3d::From (-1.0, -1.0, -1.0, 1.0, 1.0, 1.0); 
    
        attr["attributes"]["NORMAL"] = accNormalId;
        AddMeshVertexAttribute (rootNode, &mesh.Normals().front().x, bvNormalId, accNormalId, 3, mesh.Normals().size(), (char*)"VEC3", quantizeNormals, &normalRange.low.x, &normalRange.high.x);
        }

    rootNode["meshes"]["mesh_0"]["primitives"].append(attr);

    rootNode["bufferViews"][bvIndexId] = Json::objectValue;
    rootNode["bufferViews"][bvIndexId]["buffer"] = "binary_glTF";
    rootNode["bufferViews"][bvIndexId]["byteOffset"] = Json::Value(m_binaryData.size());
    rootNode["bufferViews"][bvIndexId]["byteLength"] = Json::Value(useShortIndices ? (shortIndices.size() * sizeof(uint16_t)) : (indices.size() * sizeof(uint32_t)));
    rootNode["bufferViews"][bvIndexId]["target"] =  GLTF_ELEMENT_ARRAY_BUFFER;

    if (useShortIndices)
        AddBinaryData (shortIndices.data(),  shortIndices.size() *sizeof(uint16_t));
    else
        AddBinaryData (indices.data(),  indices.size() *sizeof(uint32_t));

    if (mesh.ValidIdsPresent())
        {
        auto nBatchIdBytes = batchIds.size() * sizeof(uint16_t);
        rootNode["bufferViews"][bvBatchId] = Json::objectValue;
        rootNode["bufferViews"][bvBatchId]["buffer"] = "binary_glTF";
        rootNode["bufferViews"][bvBatchId]["byteOffset"] = Json::Value(m_binaryData.size());
        rootNode["bufferViews"][bvBatchId]["byteLength"] = Json::Value(nBatchIdBytes);
        rootNode["bufferViews"][bvBatchId]["target"] = GLTF_ARRAY_BUFFER;

        AddBinaryData (batchIds.data(), nBatchIdBytes);
        rootNode["accessors"][accBatchId] = Json::objectValue;
        rootNode["accessors"][accBatchId]["bufferView"] = bvBatchId;
        rootNode["accessors"][accBatchId]["byteOffset"] = 0;
        rootNode["accessors"][accBatchId]["componentType"] = GLTF_UNSIGNED_SHORT;
        rootNode["accessors"][accBatchId]["count"] = Json::Value(batchIds.size());
        rootNode["accessors"][accBatchId]["type"] = "SCALAR";
        }
    
    rootNode["accessors"][accPositionId]["min"] = Json::arrayValue;
    rootNode["accessors"][accPositionId]["max"] = Json::arrayValue;
    if (quantizePositions)
        {
        rootNode["accessors"][accPositionId]["min"].append(qmin[0]);
        rootNode["accessors"][accPositionId]["min"].append(qmin[1]);
        rootNode["accessors"][accPositionId]["min"].append(qmin[2]);
        rootNode["accessors"][accPositionId]["max"].append(qmax[0]);
        rootNode["accessors"][accPositionId]["max"].append(qmax[1]);
        rootNode["accessors"][accPositionId]["max"].append(qmax[2]);
        }
    else
        {
        rootNode["accessors"][accPositionId]["min"].append(pointRange.low.x);
        rootNode["accessors"][accPositionId]["min"].append(pointRange.low.y);
        rootNode["accessors"][accPositionId]["min"].append(pointRange.low.z);
        rootNode["accessors"][accPositionId]["max"].append(pointRange.high.x);
        rootNode["accessors"][accPositionId]["max"].append(pointRange.high.y);
        rootNode["accessors"][accPositionId]["max"].append(pointRange.high.z);
        }


    rootNode["accessors"][accIndexId] = Json::objectValue;
    rootNode["accessors"][accIndexId]["bufferView"] = bvIndexId;
    rootNode["accessors"][accIndexId]["byteOffset"] = 0;
    rootNode["accessors"][accIndexId]["componentType"] = useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32;
    rootNode["accessors"][accIndexId]["count"] = Json::Value(useShortIndices ? shortIndices.size() : indices.size());
    rootNode["accessors"][accIndexId]["type"] = "SCALAR";

    //AddMeshPointRange(tileData.m_json["accessors"][accPositionId], pointRange);


    rootNode["buffers"]["binary_glTF"]["byteLength"] = Json::Value(m_binaryData.size());
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



///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool PublisherContext::IsGeolocated () const
//    {
//    return nullptr != GetDgnDb().Units().GetDgnGCS();
//    }
    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::PublisherContext(/*ViewControllerR view,*/ BeFileNameCR outputDir, WStringCR tilesetName,  GeoPointCP geoLocation, bool publishPolylines, size_t maxTilesetDepth, size_t maxTilesPerDirectory)
    : /*m_viewController(view),*/ m_outputDir(outputDir), m_rootName(tilesetName), m_publishPolylines (publishPolylines), m_maxTilesetDepth (maxTilesetDepth), m_maxTilesPerDirectory (maxTilesPerDirectory)
    {
    // By default, output dir == data dir. data dir is where we put the json/b3dm files.
    m_outputDir.AppendSeparator();
    m_dataDir = m_outputDir;

//    // For now use view center... maybe should use the DgnDb range center.
//    DPoint3d        origin = m_viewController.GetCenter ();

//    m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);
    m_tilesetTransform = Transform::FromIdentity();

//    DgnGCS*         dgnGCS = m_viewController.GetDgnDb().Units().GetDgnGCS();
    DPoint3d        ecfOrigin, ecfNorth;

    if (true/*nullptr == dgnGCS*/)
        {
        double  longitude = -75.686844444444444444444444444444, latitude = 40.065702777777777777777777777778;

        if (nullptr != geoLocation)
            {
            longitude = geoLocation->longitude;
            latitude  = geoLocation->latitude;
            }
        ecfOrigin = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, latitude * msGeomConst_radiansPerDegree);
        ecfNorth  = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, 1.0E-4 + latitude * msGeomConst_radiansPerDegree);
        }
//    else
//        {
//        GeoPoint        originLatLong, northLatLong;
//        DPoint3d        north = origin;
//    
//        north.y += 100.0;
//
//        dgnGCS->LatLongFromUors (originLatLong, origin);
//        dgnGCS->XYZFromLatLong(ecfOrigin, originLatLong);
//
//        dgnGCS->LatLongFromUors (northLatLong, north);
//        dgnGCS->XYZFromLatLong(ecfNorth, northLatLong);
//        }


    DVec3d      zVector, yVector;
    RotMatrix   rMatrix;

    zVector.Normalize ((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference (ecfNorth, ecfOrigin);

    rMatrix.SetColumn (yVector, 1);
    rMatrix.SetColumn (zVector, 2);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

    m_tileToEcef =  Transform::From (rMatrix, ecfOrigin);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   08/16
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status PublisherContext::Setup()
//    {
//    // Ensure directories exist and are writable
//    if (m_outputDir != m_dataDir && BeFileNameStatus::Success != BeFileName::CheckAccess(m_outputDir, BeFileNameAccess::Write))
//        return Status::CantWriteToBaseDirectory;
//
//    bool dataDirExists = BeFileName::DoesPathExist(m_dataDir);
//    if (dataDirExists && BeFileNameStatus::Success != BeFileName::EmptyDirectory(m_dataDir.c_str()))
//        return Status::CantCreateSubDirectory;
//    else if (!dataDirExists && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(m_dataDir))
//        return Status::CantCreateSubDirectory;
//
//    if (BeFileNameStatus::Success != BeFileName::CheckAccess(m_dataDir, BeFileNameAccess::Write))
//        return Status::CantCreateSubDirectory;
//
//    return Status::Success;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   08/16
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status PublisherContext::ConvertStatus(TileGenerator::Status input)
//    {
//    switch (input)
//        {
//        case TileGenerator::Status::Success:        return Status::Success;
//        case TileGenerator::Status::NoGeometry:     return Status::NoGeometry;
//        case TileGenerator::Status::NotImplemented: return Status::NoGeometry;  // "NotImplemented" means "the viewed model is not an IPublishModelTiles therefore no tiles to publish"...not an error.
//        default: BeAssert(TileGenerator::Status::Aborted == input); return Status::Aborted;
//        }
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   08/16
//+---------------+---------------+---------------+---------------+---------------+------*/
//TileGenerator::Status PublisherContext::ConvertStatus(Status input)
//    {
//    switch (input)
//        {
//        case Status::Success:       return TileGenerator::Status::Success;
//        case Status::NoGeometry:    return TileGenerator::Status::NoGeometry;
//        default:                    return TileGenerator::Status::Aborted;
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteMetadataTree (DRange3dR range, Json::Value& root, TileNodeCR tile, size_t depth)
    {
    if (_OmitFromTileset(tile))
        {
        range = DRange3d::NullRange();
        return;
        }

    DRange3d        contentRange, publishedRange = tile.GetPublishedRange();

    // If we are publishing standalone datasets then the tiles are all published before we write the metadata tree.
    // In that case we can trust the published ranges and use them to only write non-empty nodes and branches.
    // In the server case we don't have this information and have to trust the tile ranges.  
    if (!_AllTilesPublished() && publishedRange.IsNull())
        publishedRange = tile.GetTileRange();
    
    // the published range represents the actual range of the published meshes. - This may be smaller than the 
    // range estimated when we built the tile tree. -- However we do not clip the meshes to the tile range.
    // so start the range out as the intersection of the tile range and the published range.
    contentRange.IntersectionOf (tile.GetTileRange(), publishedRange);
    range = contentRange;

    if (!tile.GetChildren().empty())
        {
        root[JSON_Children] = Json::arrayValue;
        if (0 == --depth)
            {
            // Write children as seperate tilesets.
            for (auto& childTile : tile.GetChildren())
                {
                Json::Value         childTileset;
                DRange3d            childRange;

                childTileset["asset"]["version"] = "0.0";

                auto&       childRoot = childTileset[JSON_Root];
                WString     metadataRelativePath = childTile->GetRelativePath(GetRootName().c_str(), s_metadataExtension);
                BeFileName  metadataFileName (nullptr, GetDataDirectory().c_str(), metadataRelativePath.c_str(), nullptr);

                WriteMetadataTree (childRange, childRoot, *childTile, GetMaxTilesetDepth());
                if (!childRange.IsNull())
                    {
                    TilePublisher::WriteJsonToFile (metadataFileName.c_str(), childTileset);

                    Json::Value         child;

                    child["refine"] = "REPLACE";
                    child[JSON_GeometricError] = childTile->GetTolerance();
                    TilePublisher::WriteBoundingVolume(child, childRange);

                    child[JSON_Content]["url"] = Utf8String (metadataRelativePath.c_str()).c_str();
                    root[JSON_Children].append(child);
                    range.Extend (childRange);
                    }
                }
            }
        else
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

    root["refine"] = "REPLACE";
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

    if (!GetTilesetTransform().IsIdentity())
        {
        DMatrix4d   matrix  = DMatrix4d::From (GetTilesetTransform());
        auto&       transformValue = val[JSON_Root][JSON_Transform];

        for (size_t i=0;i<4; i++)
            for (size_t j=0; j<4; j++)
                transformValue.append (matrix.coff[j][i]);
        }

    auto&       root = val[JSON_Root];
    DRange3d    rootRange;

    WriteMetadataTree (rootRange, root, rootTile, maxDepth);
    TilePublisher::WriteJsonToFile (metadataFileName.c_str(), val);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     08/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status   PublisherContext::CollectOutputTiles (Json::Value& rootJson, DRange3dR rootRange, TileNodeR rootTile, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector)
//    {
//    Status                      status;
//    AutoRestore <WString>       saveRootName (&m_rootName, WString (name.c_str()));
//
//    if (0 != m_maxTilesPerDirectory)
//        rootTile.GenerateSubdirectories (m_maxTilesPerDirectory, m_dataDir);
//
//    if (Status::Success == (status = ConvertStatus  (generator.CollectTiles (rootTile, collector))))
//        {
//        Json::Value         child;
//
//        rootRange.Extend (rootTile.GetTileRange());
//        rootJson["refine"] = "REPLACE";
//        rootJson[JSON_GeometricError] = rootTile.GetTolerance();
//        TilePublisher::WriteBoundingVolume(rootJson, rootTile.GetTileRange());
//
//        rootJson[JSON_Content]["url"] = Utf8String (rootTile.GetRelativePath (name.c_str(), s_metadataExtension).c_str());
//
//        WriteTileset (BeFileName(nullptr, GetDataDirectory().c_str(), rootTile.GetRelativePath ((GetRootName() + L"").c_str(), s_metadataExtension).c_str(), nullptr), rootTile, GetMaxTilesetDepth());
//        }
//    return status;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     08/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status   PublisherContext::DirectPublishModel (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, DgnModelR model, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter)
//    {
//    IGenerateMeshTiles*         generateMeshTiles;
//    TileNodePtr                 rootTile;
//    Status                      status;
//
//    if (nullptr == (generateMeshTiles = dynamic_cast <IGenerateMeshTiles*> (&model)))
//        return Status::NotImplemented;
//
//    progressMeter._SetModel (&model);
//    progressMeter._SetTaskName (ITileGenerationProgressMonitor::TaskName::GeneratingTileNodes);       // Needs work -- meter progress in model publisher.
//    progressMeter._IndicateProgress (0, 1);
//                                                                            
//    if (Status::Success != (status = ConvertStatus (generateMeshTiles->_GenerateMeshTiles (rootTile, m_dbToTile))))
//        return status;
//
//    return CollectOutputTiles (rootJson, rootRange, *rootTile, name, generator, collector); 
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     08/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status   PublisherContext::PublishElements (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters)
//    {
//    AutoRestore <WString>   saveRootName (&m_rootName, WString (name.c_str()));
//    TileNodePtr             rootTile;
//    Status                  status;
//    static size_t           s_maxPointsPerTile = 200000;
//
//    if (Status::Success != (status = ConvertStatus(generator.GenerateTiles (rootTile, s_maxPointsPerTile))))
//        return status;
//        
//    return CollectOutputTiles (rootJson, rootRange, *rootTile, name, generator, collector); 
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     08/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//PublisherContext::Status   PublisherContext::PublishViewModels (TileGeneratorR generator, TileGenerator::ITileCollector& collector, DRange3dR rootRange, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter)
//    {
//    Json::Value         realityModelTilesets, elementTileSet;
//
//    rootRange = DRange3d::NullRange();
//    // First go through and collect tilesets for any (reality) models.   These will produce tileset from the HLOD trees directly and therefore don't
//    // won't be included by collecting through their elements.
//    for (auto& modelId : m_viewController.GetViewedModels())
//        {
//        DgnModelPtr     viewedModel = m_viewController.GetDgnDb().Models().GetModel (modelId);
//        WString         tilesetName;
//        Json::Value     tileValue;
//
//        if (viewedModel.IsValid())
//            {
//            tilesetName = L"RealityModel_" + WString (viewedModel->GetName().c_str(), true);
//
//            if (Status::Success == DirectPublishModel (tileValue, rootRange, tilesetName, *viewedModel, generator, collector, toleranceInMeters, progressMeter))
//                realityModelTilesets.append (tileValue);
//            }
//        }
//
//    if (realityModelTilesets.empty())
//        m_tilesetTransform = m_tileToEcef;       // If we are not creating a seperate root tile - apply the ECEF transform directly to the element tileset.
//
//    progressMeter._SetModel (m_viewController.GetTargetModel());
//
//    WString     elementTileSetName = realityModelTilesets.empty() ? GetRootName() : L"Elements";
//    Status      elementPublishStatus = PublishElements (elementTileSet, rootRange, elementTileSetName, generator, collector, toleranceInMeters);
//
//    m_tilesetTransform = Transform::FromIdentity();
//    if (realityModelTilesets.empty())
//        return elementPublishStatus;
//
//    
//    // We have relity models... create a tile set that includes both the reality models and the elements.
//    Json::Value     value;
//    value["asset"]["version"] = "0.0";
//
//    auto& root = value[JSON_Root];
//
//    if (!GetTileToEcef().IsIdentity())
//        {
//        DMatrix4d   matrix  = DMatrix4d::From (GetTileToEcef());
//        auto&       transformValue = root[JSON_Transform];
//
//        for (size_t i=0;i<4; i++)
//            for (size_t j=0; j<4; j++)
//                transformValue.append (matrix.coff[j][i]);
//        }
//
//    root["refine"] = "REPLACE";
//
//    root[JSON_GeometricError] = 1.E6;
//    TilePublisher::WriteBoundingVolume(root, rootRange);
//    root[JSON_Children] = realityModelTilesets;
//    if (Status::Success == elementPublishStatus)
//        root[JSON_Children].append (elementTileSet);
//
//    BeFileName  metadataFileName (nullptr, GetDataDirectory().c_str(), m_rootName.c_str(), s_metadataExtension);
//
//    TilePublisher::WriteJsonToFile (metadataFileName.c_str(), value);
//
//    return Status::Success;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     09/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//Json::Value PublisherContext::GetModelsJson (DgnModelIdSet const& modelIds)
//    {
//    Json::Value     modelJson (Json::objectValue);
//    
//    for (auto& modelId : modelIds)
//        {
//        auto const&  model = GetDgnDb().Models().GetModel (modelId);
//        if (model.IsValid())
//            modelJson[modelId.ToString()] = model->GetName();
//        }
//
//    return modelJson;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     09/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//Json::Value PublisherContext::GetCategoriesJson (DgnCategoryIdSet const& categoryIds)
//    {
//    Json::Value categoryJson (Json::objectValue); 
//    
//    for (auto& categoryId : categoryIds)
//        {
//        auto const& category = DgnCategory::QueryCategory (categoryId, GetDgnDb());
//
//        if (category.IsValid())
//            categoryJson[categoryId.ToString()] = category->GetCategoryName();
//        }
//
//    return categoryJson;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     09/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//void PublisherContext::GetSpatialViewJson (Json::Value& json, SpatialViewDefinitionCR view, TransformCR transform)
//    {
//    OrthographicViewDefinitionCP    orthographicView;
//    CameraViewDefinitionCP          cameraView;
//    DVec3d                          xVec, yVec, zVec;
//    RotMatrix                       rotation;
//    DPoint3d                        eyePoint;
//
//    if (nullptr != (cameraView = dynamic_cast <CameraViewDefinitionCP> (&view)))
//        {
//        // The camera may not be centered -- and Cesium doesn't handle uncentered windows well.
//        // Simulate by pointing the camera toward the center of the viewed volume.
//        eyePoint = cameraView->GetEyePoint();
//        rotation =  cameraView->GetViewDirection().ToRotMatrix();
//
//        DPoint3d    viewOrigin, viewEyePoint, target, viewTarget;
//
//        rotation.Multiply(viewOrigin, cameraView->GetOrigin());
//        rotation.Multiply(viewEyePoint, eyePoint);
//
//        auto extents = cameraView->GetExtents();
//        viewTarget.x = viewOrigin.x + extents.x/2.0;
//        viewTarget.y = viewOrigin.y + extents.y/2.0;
//        viewTarget.z = viewEyePoint.z - cameraView->GetFocusDistance();
//
//        rotation.MultiplyTranspose (target, viewTarget);
//
//        rotation.GetRows(xVec, yVec, zVec);
//        zVec.NormalizedDifference (eyePoint, target);
//
//        xVec.NormalizedCrossProduct (yVec, zVec);
//        yVec.NormalizedCrossProduct (zVec, xVec);
//
//        json["fov"]   =  2.0 * atan2 (extents.x/2.0, cameraView->GetFocusDistance());
//        }
//    else if (nullptr != (orthographicView = dynamic_cast <OrthographicViewDefinitionCP> (&view)))
//        {
//        // Simulate orthographic with a small field of view.
//        static const    double s_orthographicFieldOfView = .01;
//        DVec3d          extents = orthographicView->GetExtents();
//        DPoint3d        backCenter;
//
//        rotation = orthographicView->GetViewDirection().ToRotMatrix();
//        rotation.GetRows(xVec, yVec, zVec);
//
//        rotation.Multiply (backCenter, orthographicView->GetOrigin());
//        backCenter.SumOf (backCenter, extents, .5);
//        rotation.MultiplyTranspose (backCenter);
//
//        double  zDistance = extents.x / tan (s_orthographicFieldOfView / 2.0);
//
//        eyePoint.SumOf (backCenter, zVec, zDistance);
//        json["fov"] = s_orthographicFieldOfView;
//        }
//    else
//        {
//        BeAssert (false && "unsuppored view type");
//        }
//
//    transform.Multiply(eyePoint);
//    transform.MultiplyMatrixOnly(yVec);
//    transform.MultiplyMatrixOnly(zVec);
//
//    yVec.Normalize();
//    zVec.Normalize();
//    zVec.Negate();      // Towards target.
//
//    // View orientation
//    json["dest"] = PointToJson(eyePoint);
//    json["dir"] = PointToJson(zVec);
//    json["up"] = PointToJson(yVec);
//    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
