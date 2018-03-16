/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/TilePublisher.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/TilePublisher.h>
#include <BeSqLite/BeSqLite.h>
#include "Constants.h"
#include <Geom/OperatorOverload.h>
#include <DgnPlatform/AutoRestore.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_TILETREE_IO

// Vector classifiers are original implementation -- we'll continue to write them until
// the batched model classifiers are functional and merged into master.
static bool s_writeVectorClassifier = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void    padTo4ByteBoundary(std::FILE* outputFile)
    {
    std::fseek(outputFile, 0, SEEK_END);
    long        position = ftell(outputFile), padBytes = (4 - position % 4);

    if (0 != padBytes)
        {
        uint64_t    zero = 0;

        std::fwrite(&zero, 1, padBytes, outputFile);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void    padTo4ByteBoundary(ByteStream& byteStream)
    {
    size_t      padBytes = (4 - byteStream.size() % 4);

    if (0 != padBytes)
        {
        uint64_t    zero = 0;
        byteStream.Append((uint8_t const*) & zero, padBytes);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      getJsonString(Json::Value const& value)
    {
    Utf8String      string =  Json::FastWriter().write(value);

    // Pad to 4 byte boundary...
    while (0 != string.size() % 4)
        string = string + " ";

    return string;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PublishTileData::GetJsonString() const
    {
    // TFS#734860: Missing padding following feature table json in i3dm tiles...
    return getJsonString(m_json);
    }


//=======================================================================================
// Flat (non-hierarchical) batch table builder.
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct BatchTableBuilder
{
private:
    Json::Value                             m_json;
    DgnDbR                                  m_db;
    bool                                    m_is3d;
    DgnCategoryId                           m_uncategorized;
    FeatureAttributesMapCR                  m_attrs;
    bmap<DgnElementId, DgnElementId>        m_assemblyIds;
    bmap<DgnSubCategoryId, DgnCategoryId>   m_categoryIds;
                            
    bool IsUncategorized(DgnCategoryId id) const { return id.IsValid() && id == m_uncategorized; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId QueryCategoryId(DgnSubCategoryId subCategoryId)
    {
    auto    found = m_categoryIds.find(subCategoryId);

    if (found != m_categoryIds.end())
        return found->second;

    DgnCategoryId       categoryId;

    DgnSubCategoryCPtr subCategory = m_db.Elements().Get<DgnSubCategory> (subCategoryId);

    if (subCategory.IsValid())
        categoryId = subCategory->GetCategoryId();

    m_categoryIds.Insert(subCategoryId, categoryId);

    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId QueryAssemblyId(DgnElementId childId) 
    {
    auto    found = m_assemblyIds.find(childId);

    if (found != m_assemblyIds.end())
        return found->second;

    DgnElementId        assemblyId;
    DgnCategoryId       assemblyCategoryId;

    assemblyId = childId;
    if (!childId.IsValid())
        return assemblyId;

    // Get this element's category and parent element
    // Recurse until no more parents (or we find a non-geometric parent)
    static constexpr Utf8CP s_3dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
    static constexpr Utf8CP s_2dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE ECInstanceId=?";

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db.GetPreparedECSqlStatement(m_is3d ? s_3dsql : s_2dsql);
    stmt->BindId(1, childId);

    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto thisCatId = stmt->GetValueId<DgnCategoryId>(1);
        if (assemblyCategoryId.IsValid() && IsUncategorized(thisCatId) && !IsUncategorized(assemblyCategoryId))
            break; // yuck. if have children with valid categories, stop before first uncategorized parent (V8 complex header).

        assemblyCategoryId = thisCatId;
        assemblyId = childId;

        childId = stmt->GetValueId<DgnElementId>(0);
        if (!childId.IsValid())
            break;

        // Try to get the parent's category. If parent is not geometric, this will fail and we will treat current child as the assembly root.
        stmt->Reset();
        stmt->BindId(1, childId);
        }
    m_assemblyIds.Insert(childId, assemblyId);

    return assemblyId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void InitUncategorizedCategory()
    {
    // This is dumb. See OfficeBuilding.dgn - cells have no level in V8, which translates to 'Uncategorized' (2d and 3d variants) in DgnDb
    // We don't want to create an 'Uncategorized' assembly if its children belong to a real category.
    // We only can detect this because for whatever reason, "Uncategorized" is not a localized string.
    DefinitionModelR dictionary = m_db.GetDictionaryModel();
    DgnCode code = m_is3d ? SpatialCategory::CreateCode(dictionary, "Uncategorized") : DrawingCategory::CreateCode(dictionary, "Uncategorized");

    m_uncategorized = DgnCategory::QueryCategoryId(m_db, code);
    }

public:
    Json::Value& GetJson()  { return m_json; }
    Utf8String ToString()   { return getJsonString(m_json); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BatchTableBuilder(FeatureAttributesMapCR attrs, DgnDbR db, bool is3d, PublisherContext& context) : m_json(Json::objectValue), m_db(db), m_is3d(is3d), m_attrs(attrs)
    {
    InitUncategorizedCategory();

    Json::Value             geomClasses    = Json::arrayValue, 
                            elementIds     = Json::arrayValue, 
                            assemblyIds    = Json::arrayValue, 
                            categoryIds    = Json::arrayValue,
                            subCategoryIds = Json::arrayValue;

    bool                    validLabelsFound = false;
                        
    for (auto const& kvp : attrs)
        {
        FeatureAttributesCR attr = kvp.first;                                                                                                                                    
        uint32_t            index = kvp.second;
        DgnElementId        elementId = attr.GetElementId();

        geomClasses[index]    = (int) attr.GetClass();
        elementIds[index]     = elementId.ToString();
        assemblyIds[index]    = QueryAssemblyId(elementId).ToString();
        subCategoryIds[index] = attr.GetSubCategoryId().ToString();
        categoryIds[index]    = QueryCategoryId(attr.GetSubCategoryId()).ToString();

        }
    
    m_json["geomClass"]   = std::move(geomClasses);
    m_json["element"]     = std::move(elementIds);
    m_json["assembly"]    = std::move(assemblyIds);
    m_json["subCategory"] = std::move(subCategoryIds);
    m_json["category"]    = std::move(categoryIds);

    context.AddBatchTableAttributes (m_json, attrs);
    }


};  // BatchTableBuilder


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublish::ColorIndex::ComputeDimensions(uint16_t nColors)
    {
    // Minimum texture size in WebGL is 64x64. For 16-bit color indices, we need at least 256x256.
    // At the risk of pessimization, let's not assume more than that.
    // Let's assume non-power-of-2 textures are not a big deal (we're not mipmapping them or anything).
    uint16_t height = 1;
    uint16_t width = std::min(nColors, GetMaxWidth());
    if (width < nColors)
        {
        height = nColors / width;
        if (height*width < nColors)
            ++height;
        }

    BeAssert(height*width >= nColors);
    m_width = width;
    m_height = height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void fillColorIndex(ByteStream& texture, ColorIndexMapCR map, uint16_t nColors, T getColorDef)
    {
    constexpr size_t bytesPerColor = 4;
    texture.Resize(bytesPerColor * nColors);

    for (auto const& kvp : map)
        {
        ColorDef fill = getColorDef(kvp.first);
        auto pColor = texture.GetDataP() + kvp.second * bytesPerColor;
        pColor[0] = fill.GetRed();
        pColor[1] = fill.GetGreen();
        pColor[2] = fill.GetBlue();
        pColor[3] = fill.GetAlpha();    // already inverted...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublish::ColorIndex::Build(TileMeshCR mesh, TileMaterial const& mat)
    {
    // Possibilities:
    //  - Material does not override color or alpha. Copy colors directly from ColorIndexMap (unless only one color - then use uniform color).
    //  - Material overrides both color and alpha. Every vertex has same color. Don't use indexed colors in that case.
    //  - Material overrides RGB only. Vertices may have differing alphas. ###TODO: If alpha does not vary, don't use indexed colors.
    //  - Material overrides alpha only. Vertices may have differing RGB values. ###TODO: Detect that case and don't use indexed colors?
    ColorIndexMapCR map = mesh.GetColorIndexMap();
    uint16_t nColors = map.GetNumIndices();
    ComputeDimensions(nColors);
    BeAssert(0 < m_width && 0 < m_height);

    if (!mat.OverridesAlpha() && !mat.OverridesRgb())
        {
        fillColorIndex(m_texture, map, nColors, [](uint32_t color)
            {
            ColorDef fill(color);
            fill.SetAlpha(255 - fill.GetAlpha());
            return fill;
            });
        }
    else if (!mat.OverridesAlpha())
        {
        RgbFactor fRgb = mat.GetRgbOverride();
        ColorDef rgb(static_cast<uint8_t>(fRgb.red*255), static_cast<uint8_t>(fRgb.green*255), static_cast<uint8_t>(fRgb.blue*255));
        fillColorIndex(m_texture, map, nColors, [=](uint32_t color)
            {
            ColorDef input(color);
            ColorDef output = rgb;
            output.SetAlpha(255 - input.GetAlpha());
            return output;
            });
        }
    else if (!mat.OverridesRgb())
        {
        uint8_t alpha = 255 - static_cast<uint8_t>(mat.GetAlphaOverride() * 255);
        fillColorIndex(m_texture, map, nColors, [=](uint32_t color)
            {
            ColorDef def(color);
            def.SetAlpha(alpha);
            return def;
            });
        }
    else
        {
        uint8_t alpha = 255 - static_cast<uint8_t>(mat.GetAlphaOverride() * 255);
        RgbFactor fRgb = mat.GetRgbOverride();
        ColorDef rgba(static_cast<uint8_t>(fRgb.red*255), static_cast<uint8_t>(fRgb.green*255), static_cast<uint8_t>(fRgb.blue*255), alpha);
        fillColorIndex(m_texture, map, nColors, [=](uint32_t color) { return rgba; });
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePublisher::TilePublisher(TileNodeCR tile, PublisherContext& context) : m_centroid(tile.GetTileCenter()), m_tile(tile), m_context(context)
    {
#define CESIUM_RTC_ZERO
#ifdef CESIUM_RTC_ZERO
    m_centroid = DPoint3d::FromXYZ(0,0,0);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::PublisherContext::Statistics::RecordPointCloud (size_t nPoints)
    {
    m_pointCloudTotalPoints += nPoints;
    m_pointCloudCount++;
    if (0 == m_pointCloudMinPoints || nPoints < m_pointCloudMinPoints)
        m_pointCloudMinPoints = nPoints;

    if (nPoints > m_pointCloudMaxPoints)
        m_pointCloudMaxPoints = nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/17
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::PublisherContext::Statistics::~Statistics()
    {
#define STATISTICS
#ifdef STATISTICS
    if (0.0 != m_textureCompressionMegaPixels)
        {
        LOG.infov ("Total Compression: %f megapixels, %f seconds (%f minutes, %f hours)\n", m_textureCompressionMegaPixels, m_textureCompressionSeconds, m_textureCompressionSeconds / 60.0, m_textureCompressionSeconds / 3600.0);
        LOG.infov ("Compression Rate: %f megapixels/second\n", m_textureCompressionMegaPixels / m_textureCompressionSeconds);
        }
    if (0 != m_pointCloudCount)
        {
        LOG.infov ("Point Cloud count: %g, Total Points: %g, Min Points: %g, Max Points: %g, Average: %g\n", (double) m_pointCloudCount, (double) m_pointCloudTotalPoints, (double) m_pointCloudMinPoints,(double) m_pointCloudMaxPoints, (double) m_pointCloudTotalPoints / (double) m_pointCloudCount);
        }
#endif
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
    AppendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, diagonal.x)/2.0, 0.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, diagonal.y)/2.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, diagonal.z/2.0)));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddTechniqueParameter(Json::Value& technique, Utf8CP name, Gltf::DataType type, Utf8CP semantic)
    {
    auto& param = technique["parameters"][name];
    param["type"] = static_cast<int32_t>(type);
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
void TilePublisher::AddShader(Json::Value& shaders, Utf8CP name, int32_t type, Utf8CP buffer)
    {
    auto& shader = (shaders[name] = Json::objectValue);
    shader["type"] = type;
    shader["extensions"]["KHR_binary_glTF"]["bufferView"] = buffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void PublishTileData::AddBufferView(Utf8CP name, T const& bufferData)
    {
    Json::Value&    views = m_json["bufferViews"];

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
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PublishTileData::AddBinaryData (void const* data, size_t size) 
    {
    m_binaryData.Append (static_cast<uint8_t const*> (data), size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PublishTileData::PadBinaryDataToBoundary(size_t boundarySize)
    {
    uint8_t        zero = 0;
    while (0 != (m_binaryData.size() % boundarySize))
        m_binaryData.Append(&zero, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString     PublisherContext::GetTileExtension (TileNodeCR tile) const
    {
    return (s_writeVectorClassifier && DoPublishAsClassifier()) ? L"vctr" : tile.GetFileExtension();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::RecordUsage(FeatureAttributesMapCR attributes)
    {
    BeMutexHolder lock(m_mutex);

    for (auto& attribute : attributes)
        m_usedSubCategories.insert (attribute.first.GetSubCategoryId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName  PublisherContext::GetBinaryDataFileName(TileNodeCR tile) const
    {
    WString     rootName = GetRootName (tile.GetModel().GetModelId(), GetCurrentClassifier());
    BeFileName  modelDir = GetModelDataDirectory(tile.GetModel().GetModelId(), GetCurrentClassifier());

    return  BeFileName(nullptr, modelDir.c_str(), tile.GetFileName (rootName.c_str(), GetTileExtension(tile).c_str()).c_str(), nullptr);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void extendRange(DRange3dR range, TileMeshList const& meshes, TransformCP transform)
    {
    for (auto& mesh : meshes)
        {
        if (nullptr == transform)
            {
            range.Extend(mesh->GetRange());
            }
        else
            {
            for (auto& point : mesh->Points())
                {
                DPoint3d    transformedPoint;

                transform->Multiply(transformedPoint, point);
                range.Extend(transformedPoint);
                }
            }
        }
    }

template<typename T> static void FWriteValue (T const& value, std::FILE* file) { fwrite(&value, 1, sizeof(value), file); }
template<typename T> static void FWrite(T const& value, std::FILE* file) { if (!value.empty()) fwrite(value.data(), 1, value.size() * sizeof(*value.data()), file); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilePublisher::Publish()
    {
    PublishableTileGeometry publishableGeometry = m_tile.GeneratePublishableGeometry(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, m_context.WantSurfacesOnly(), !m_context.DoPublishAsClassifier() /* no instancing for classifiers */, m_context.GetGenerationFilter());

    if (publishableGeometry.IsEmpty())
        return PublisherContext::Status::NoGeometry;                            // Nothing to write...Ignore this tile (it will be omitted when writing tileset data as its published range will be NullRange.

    BeFileName      fileName = m_context.GetBinaryDataFileName(m_tile);

    std::FILE*  outputFile = _wfopen(fileName.c_str(), L"wb");

    if (nullptr == outputFile)
        {
        BeAssert (false && "Unable to open output file");
        return PublisherContext::Status::CantOpenOutputFile;
        }
    m_context.RecordUsage (m_tile.GetAttributes());
    
    if (m_context.DoPublishAsClassifier())
        {
        WriteClassifier(outputFile, publishableGeometry, m_context.GetCurrentClassifier()->m_classifier, m_context.GetCurrentClassifier()->m_classifiedRange.m_range);
        }
    else if (publishableGeometry.Parts().empty())
        {
        BeAssert (publishableGeometry.PointClouds().empty() || publishableGeometry.Meshes().empty());   // We don't expect point clouds with meshes (although these could be handled as a composite if necessary).
        if (!publishableGeometry.PointClouds().empty())
            WritePointCloud(outputFile, *publishableGeometry.PointClouds().front());
        else
            WriteTileMeshes(outputFile, publishableGeometry);
        }
    else
        {
        // Composite header.
        uint32_t        tileCount = (publishableGeometry.Meshes().empty() ? 0 : 1) + publishableGeometry.Parts().size();

        FWriteValue(Format::Composite, outputFile);
        FWriteValue(Composite::Version, outputFile);
        long    compositeSizeLocation = ftell (outputFile);
        FWriteValue((uint32_t) 0, outputFile);                   // Filled in below...
        FWriteValue(tileCount, outputFile);

        WriteTileMeshes(outputFile, publishableGeometry);

        uint32_t    compositeSize = std::ftell(outputFile);
        std::fseek (outputFile, compositeSizeLocation, SEEK_SET);
        FWriteValue(compositeSize, outputFile);
        }
    std::fclose(outputFile);

    return PublisherContext::Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value  TilePublisher::CreateMesh (TileMeshList const& tileMeshes, PublishTileData& tileData, size_t& primitiveIndex)
    {
    Json::Value     jsonMesh = Json::objectValue;
    Json::Value     primitives;
    bool            doBatchIds = false;

    for (auto& tileMesh : tileMeshes)
        if (tileMesh->ValidIdsPresent())
            {
            doBatchIds = true;
            break;
            }

    for (auto& tileMesh : tileMeshes)
        {
        if (!tileMesh->Triangles().empty())
            AddMeshPrimitive(primitives, tileData, *tileMesh, primitiveIndex++, doBatchIds);

        if (!tileMesh->Polylines().empty())
            AddPolylinePrimitive(primitives, tileData, *tileMesh, primitiveIndex++, doBatchIds);
        }
    BeAssert (!primitives.empty());
    jsonMesh["primitives"] = primitives;
    return jsonMesh;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteTileMeshes (std::FILE* outputFile, PublishableTileGeometryR publishableGeometry)
    {
    DRange3d    publishedRange = DRange3d::NullRange();

    if (!publishableGeometry.Meshes().empty())
        {
        extendRange (publishedRange, publishableGeometry.Meshes(), nullptr);

        bool        validIdsPresent = false;
        Utf8String  batchTableStr;

        for (auto& mesh : publishableGeometry.Meshes())
            {
            if (mesh->ValidIdsPresent())
                {
                BatchTableBuilder batchTableBuilder(m_tile.GetAttributes(), m_context.GetDgnDb(), m_tile.GetModel().Is3d(), m_context);
                batchTableStr = batchTableBuilder.ToString();
                break;
                }
            }

        WriteBatched3dModel (outputFile, publishableGeometry.Meshes(), batchTableStr);
        }

    for (auto& part: publishableGeometry.Parts())
        WritePartInstances(outputFile, publishedRange, part);

    m_tile.SetPublishedRange (publishedRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WritePointCloud (std::FILE* outputFile, TileMeshPointCloudR pointCloud) 
    {
    long            startPosition = ftell (outputFile);
    Json::Value     featureTable;
    bool            rgbPresent = pointCloud.Colors().size() == pointCloud.Points().size();

    featureTable["POINTS_LENGTH"] = pointCloud.Points().size();
    featureTable["POSITION_QUANTIZED"]["byteOffset"] = 0;

    DRange3d        positionRange = DRange3d::NullRange();

    positionRange.Extend(pointCloud.Points());

    DVec3d positionScale = DVec3d::FromStartEnd(positionRange.low, positionRange.high);

    featureTable["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.x);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.y);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.z);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(positionScale.x);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(positionScale.y);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(positionScale.z);

    if (rgbPresent)
        featureTable["RGB"]["byteOffset"] = pointCloud.Points().size() * 3 * sizeof(int16_t);

    Utf8String      featureTableStr =  getJsonString(featureTable);

    uint32_t        zero = 0, 
                    featureTableStrLen = featureTableStr.size(),
                    featureTableBinaryLength = pointCloud.Points().size() * (3 * sizeof(int16_t) + (rgbPresent ?  sizeof(TileMeshPointCloud::Rgb) : 0));

    FWriteValue(Format::PointCloud, outputFile);
    FWriteValue(PointCloud::Version, outputFile);                                                                                                                                  
    long    lengthDataPosition = ftell(outputFile);
    FWriteValue((int32_t) 0, outputFile);    // Total length filled in below.
    FWriteValue(featureTableStrLen, outputFile);          
    FWriteValue(featureTableBinaryLength, outputFile);    

    FWriteValue(zero, outputFile);    // No batch for now.
    FWriteValue(zero, outputFile);    // No batch for now.

    std::fwrite(featureTableStr.data(), 1, featureTableStrLen, outputFile);
    for (auto& point : pointCloud.Points())
        {
        int16_t             quantizedPosition[3];
        static double       range = (double) (0xffff);

        quantizedPosition[0] = (uint16_t) (.5 + range * (point.x - positionRange.low.x) / positionScale.x);
        quantizedPosition[1] = (uint16_t) (.5 + range * (point.y - positionRange.low.y) / positionScale.y);
        quantizedPosition[2] = (uint16_t) (.5 + range * (point.z - positionRange.low.z) / positionScale.z);

        std::fwrite (quantizedPosition, 1, sizeof(quantizedPosition), outputFile);
        }     
    if (rgbPresent)
        std::fwrite (pointCloud.Colors().data(), pointCloud.Colors().size(), sizeof(TileMeshPointCloud::Rgb), outputFile);

    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    FWriteValue(dataSize, outputFile);
    std::fseek(outputFile, 0, SEEK_END);


    m_tile.SetPublishedRange (positionRange);

    m_context.m_statistics.RecordPointCloud(pointCloud.Points().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::_AddBatchTableAttributes(Json::Value& batchTableJson, FeatureAttributesMapCR attrs)
    {
    if (!m_scheduleEntryMaps.empty())
        {
        bvector<Json::Value>    schedules;

        for (auto& scheduleMap : m_scheduleEntryMaps)
            schedules.push_back(Json::arrayValue);

        for (auto const& kvp : attrs)
            {
            for (size_t i=0; i<m_scheduleEntryMaps.size(); i++)
                {
                auto&   scheduleMap = m_scheduleEntryMaps.at(i);
                auto    found = scheduleMap.find(kvp.first.GetElementId());

                schedules.at(i)[kvp.second] = (found == scheduleMap.end()) ? Json::Value::GetNull() : found->second;
                }
            }
        for (size_t i=0; i<schedules.size(); i++)
            batchTableJson[Utf8PrintfString("schedule%d", i).c_str()] = std::move(schedules[i]);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WritePartInstances(std::FILE* outputFile, DRange3dR publishedRange, TileMeshPartPtr& part)
    {
    PublishTileData     featureTableData, partData;
    bvector<uint16_t>   attributeIndices;
    bool                rotationPresent = false;

    featureTableData.m_json["INSTANCES_LENGTH"] = part->Instances().size();

    bvector<float>          upFloats, rightFloats, scales;
    FeatureAttributesMap    attributesSet;
    DRange3d                positionRange = DRange3d::NullRange();

    bool validIdsPresent = false;
    bool invalidIdsPresent = false;
    bool scaleRequired = false;
    for (auto& instance : part->Instances())
        {
        DPoint3d    translation;
        DVec3d      right, up;
        RotMatrix   rMatrix, rigidMatrix;
        double      scale;

        instance.GetTransform().GetTranslation(translation);
        instance.GetTransform().GetMatrix(rMatrix);
        if (!rMatrix.IsRigidScale(rigidMatrix, scale))
            {
            BeAssert (false && "Part instance contains non-rigid scale");
            continue;
            }


        positionRange.Extend(translation);
        rotationPresent |= !rigidMatrix.IsIdentity();

        rigidMatrix.GetColumn(right, 0);
        rigidMatrix.GetColumn(up, 1);

        rightFloats.push_back(right.x);
        rightFloats.push_back(right.y);
        rightFloats.push_back(right.z);

        upFloats.push_back(up.x);
        upFloats.push_back(up.y);
        upFloats.push_back(up.z);

        scales.push_back(scale);
        scaleRequired |= fabs(scale - 1.0) > 1.0E-8;

        extendRange (publishedRange, part->Meshes(), &instance.GetTransform());
        attributeIndices.push_back(attributesSet.GetIndex(instance.GetAttributes()));
        bool isValidId = (0 != attributeIndices.back());
        validIdsPresent |= isValidId;
        invalidIdsPresent |= !isValidId;
        }

    AddExtensions(partData);
    AddDefaultScene(partData);
    AddMeshes (partData, part->Meshes());

#ifdef DO_QUANTIZE_INSTANCE_TRANSLATION
    // Quantizing the instance translation would seem to be a reasonable optimization...however
    // in some I-Models (area234 from Hatch) the instance origins are far from the parts and therefore
    // quantization can produce excessive error.
    DVec3d              positionScale;
    bvector<uint16_t>   quantizedPosition;
    double              range = (double) (0xffff);

    positionScale = DVec3d::FromStartEnd(positionRange.low, positionRange.high);
    for (auto& instance : part->Instances())
        {
        DPoint3d    translation;

        instance.GetTransform().GetTranslation(translation);

        quantizedPosition.push_back((uint16_t) (.5 + (range * (translation.x - positionRange.low.x) / positionScale.x)));
        quantizedPosition.push_back((uint16_t) (.5 + (range * (translation.y - positionRange.low.y) / positionScale.y)));
        quantizedPosition.push_back((uint16_t) (.5 + (range * (translation.z - positionRange.low.z) / positionScale.z)));
        }


    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.x);
    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.y);
    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.z);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.x);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.y);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.z);

    featureTableData.m_json["POSITION_QUANTIZED"]["byteOffset"] = featureTableData.BinaryDataSize();
    featureTableData.AddBinaryData(quantizedPosition.data(), quantizedPosition.size()*sizeof(uint16_t));
#else
    bvector<FPoint3d>       positions;
    for (auto& instance : part->Instances())
        {
        DPoint3d        translation;

        instance.GetTransform().GetTranslation(translation);
        positions.push_back(FPoint3d::From(translation));
        }

    featureTableData.m_json["POSITION"]["byteOffset"] = featureTableData.BinaryDataSize();
    featureTableData.AddBinaryData(positions.data(), positions.size()*sizeof(FPoint3d));
#endif

    if (validIdsPresent)
        {
        if (!invalidIdsPresent)
            {
            // Cesium's instanced models require that indices range from [0, nInstances). Must remove the "undefined" entry for that to work.
            attributesSet.RemoveUndefined();
            for (auto& index : attributeIndices)
                index--;
            }

        featureTableData.m_json["BATCH_ID"]["byteOffset"] = featureTableData.BinaryDataSize();
        featureTableData.AddBinaryData(attributeIndices.data(), attributeIndices.size()*sizeof(uint16_t));
        }

    featureTableData.PadBinaryDataToBoundary(4);
    if (rotationPresent)
        {
        featureTableData.m_json["NORMAL_UP"]["byteOffset"] = featureTableData.BinaryDataSize();
        featureTableData.AddBinaryData(upFloats.data(), upFloats.size()*sizeof(float));

        featureTableData.m_json["NORMAL_RIGHT"]["byteOffset"] = featureTableData.BinaryDataSize();
        featureTableData.AddBinaryData(rightFloats.data(), rightFloats.size()*sizeof(float));

        if (scaleRequired)
            {
            featureTableData.m_json["SCALE"]["byteOffset"] = featureTableData.BinaryDataSize();
            featureTableData.AddBinaryData(scales.data(), scales.size()*sizeof(float));
            }
        }

    BatchTableBuilder batchTableBuilder(attributesSet, m_context.GetDgnDb(), m_tile.GetModel().Is3d(), m_context);
    Utf8String      batchTableStr = batchTableBuilder.ToString();

    Utf8String      featureTableStr = featureTableData.GetJsonString();
    uint32_t        batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    uint32_t        featureTableJsonLength = static_cast<uint32_t> (featureTableStr.size());
    uint32_t        featureTableBinarySize = featureTableData.BinaryDataSize(), gltfFormat = 1, zero = 0;

    uint32_t        padBytes = (featureTableJsonLength + featureTableBinarySize + batchTableStrLen) % 8;
    uint64_t        padZero;

    featureTableData.AddBinaryData (&padZero, padBytes);
    featureTableBinarySize += padBytes;

    long            startPosition = ftell(outputFile);

    FWriteValue(Format::I3dm, outputFile);
    FWriteValue(I3dm::Version, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    FWriteValue(zero, outputFile);        // Filled in later.
    FWriteValue(featureTableJsonLength, outputFile);
    FWriteValue(featureTableBinarySize, outputFile);
    FWriteValue(batchTableStrLen, outputFile);
    FWriteValue((uint32_t) 0, outputFile);         // Batch table binary (not used).
    FWriteValue(gltfFormat, outputFile);
    std::fwrite(featureTableStr.data(), 1, featureTableJsonLength, outputFile);
    std::fwrite(featureTableData.BinaryData(), 1, featureTableData.BinaryDataSize(), outputFile);
    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, outputFile);

    WriteGltf(outputFile, partData);
    padTo4ByteBoundary (outputFile);
    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    FWriteValue(dataSize, outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBatched3dModel(std::FILE* outputFile, TileMeshList const& meshes, Utf8StringCR batchTableStr)
    {
    PublishTileData     tileData;

    uint32_t        batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    Json::Value     featureTable;

    AddExtensions(tileData);
    AddDefaultScene(tileData);
    AddMeshes(tileData, meshes);


    featureTable["BATCH_LENGTH"] = batchTableStr.empty() ? 0 : m_tile.GetAttributes().size();
    Utf8String      featureTableStr = getJsonString(featureTable);

    long    startPosition = ftell (outputFile);
    FWriteValue(Format::B3dm, outputFile);
    FWriteValue(B3dm::Version, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    FWriteValue(0, outputFile);    // Filled in below.
    FWriteValue((uint32_t) featureTableStr.size(), outputFile);   
    FWriteValue(0, outputFile);    // Feature table binary (none)
    FWriteValue((uint32_t) batchTableStr.size(), outputFile);
    FWriteValue(0, outputFile);    // length of binary portion of batch table (none).
    FWrite(featureTableStr, outputFile);
    FWrite(batchTableStr, outputFile);

    WriteGltf (outputFile, tileData);

    padTo4ByteBoundary (outputFile);
    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    FWriteValue(dataSize, outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct  VectorPosition
    {
    uint16_t  m_x;
    uint16_t  m_y;

    VectorPosition() : m_x(0), m_y(0) { }
    VectorPosition(DPoint3dCR dPoint, DRange3dCR range)
        {
        m_x = (uint16_t) ((double) 0x7fff * (dPoint.x - range.low.x) / (range.high.x - range.low.x));
        m_y = (uint16_t) ((double) 0x7fff * (dPoint.y - range.low.y) / (range.high.y - range.low.y));
        }
    };

uint16_t    zigZagEncode(int32_t value) { return (uint16_t) (((value << 1) ^ (value >> 15)) & 0xffff); }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct ClassifierTileWriter
{
    typedef bmap<DPoint3d, uint32_t, TileUtil::PointComparator> T_PointMap;

    ModelSpatialClassifierCR        m_classifier;
    bmap<DgnElementId, uint32_t>    m_elementColors;
    PublisherContext&               m_context;


    static constexpr double         s_pointTolerance = 1.0E-6;

    virtual void _AddClosedMesh(PolyfaceHeaderR polyface, uint16_t batchId, TileDisplayParamsCR displayParams)  = 0;
    virtual void _InitMesh()  { }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ClassifierTileWriter( ModelSpatialClassifierCR classifier, PublisherContext& context) : m_context(context),  m_classifier(classifier) 
    { 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr   BuildPolyfaceFromTriangles(bvector<DPoint3d> const& meshPoints, bvector<TileTriangle> const& triangles)
    {
    PolyfaceHeaderPtr           polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    PolyfaceCoordinateMapPtr    coordinateMap = PolyfaceCoordinateMap::Create(*polyface);    

    for (auto& triangle : triangles)
        {
        int32_t         indices[3];

        for (size_t i=0; i<3; i++)
            {
            indices[i] = 1 + coordinateMap->AddPoint(meshPoints[triangle.m_indices[i]]);
            if (!triangle.m_edgeVisible[i])
                indices[i] = -indices[i];
            }

        polyface->AddIndexedFacet(3, indices);
        }
    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr   BuildPolyfaceFromPolylines(DVec3dR normal, bvector<DPoint3d> const& meshPoints, bvector<TilePolyline> const& polylines, double expandDistance)
    {
    PolyfaceHeaderPtr           polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    PolyfaceCoordinateMapPtr    coordinateMap = PolyfaceCoordinateMap::Create(*polyface);   
    bvector<DPoint3d>           points;
    DPoint3d                    origin;
    static constexpr double     s_tolerance = 1.0E-5;
    static constexpr double     s_maxMiterDot = .75;

    for (auto& polyline : polylines)
        for (auto& index : polyline.m_indices)
            points.push_back (meshPoints[index]);

    if (!bsiGeom_planeThroughPointsTol(&normal, &origin, points.data(), points.size(), s_tolerance))
        normal = DVec3d::From(0.0, 0.0, 1.0);
    else
        normal.Normalize();

    for (auto& polyline : polylines)
        {
        if (polyline.m_indices.empty())
            {
            BeAssert(false);
            continue;
            }
        size_t  segmentCount = polyline.m_indices.size()-1;

        if (0 == segmentCount)
            {
            static      constexpr  size_t s_circlePoints = 8;
            auto&       center = meshPoints[polyline.m_indices[0]];
            int32_t     indices[s_circlePoints];
            double      angleDelta = msGeomConst_2pi / (double) s_circlePoints;

            for (size_t i=0; i<s_circlePoints; i++)
                {
                double  angle = i * angleDelta;
                indices[i] = 1 + coordinateMap->AddPoint(DPoint3d::From (center.x + expandDistance * cos(angle), center.y + expandDistance * sin(angle), center.z));
                }
            polyface->AddIndexedFacet(s_circlePoints, indices);
            }

        for (size_t i=0; i<segmentCount; i++)
            {
            DPoint3d    start = meshPoints[polyline.m_indices[i]],
                        end = meshPoints[polyline.m_indices[i+1]];
            DVec3d      delta = DVec3d::FromStartEnd(start, end);
            DVec3d      perp = DVec3d::FromCrossProduct(normal, delta);
                                                                                                                                                                                                                                                                                                                    
            int32_t     indices0[3], indices1[3];
            if (perp.Normalize() < s_tolerance ||
                delta.Normalize() < s_tolerance)
                continue;

            DVec3d      perp0 = perp, perp1 = perp;
            double      distance0 = expandDistance, distance1 = expandDistance;

            if (i > 0)
                {
                DVec3d      dir = delta, prevDir = DVec3d::FromStartEndNormalize(start, meshPoints[polyline.m_indices[i-1]]);
                double      dot = prevDir.DotProduct(dir);

                if (dot > -.9999 && dot < s_maxMiterDot)
                    {
                    perp0.Normalize(DVec3d::FromSumOf(dir, prevDir));
                    distance0 /= perp0.DotProduct(perp);
                    }
                }
            if (i < segmentCount - 1)
                {
                DVec3d      dir = delta, nextDir = DVec3d::FromStartEndNormalize(end, meshPoints[polyline.m_indices[i+2]]);
                dir.Negate();
                double      dot = nextDir.DotProduct(dir);

                if (dot > -.9999 && dot < s_maxMiterDot)
                    {
                    perp1.Normalize(DVec3d::FromSumOf(dir, nextDir));
                    distance1 /= perp1.DotProduct(perp);
                    }
                }

    
            indices0[0]               = 1 + coordinateMap->AddPoint(DPoint3d::FromSumOf(start, perp0, -distance0));
            indices0[2] = indices1[0] = 1 + coordinateMap->AddPoint(DPoint3d::FromSumOf(start, perp0,  distance0));
            indices0[1] = indices1[1] = 1 + coordinateMap->AddPoint(DPoint3d::FromSumOf(end,   perp1, -distance1));
            indices1[2]               = 1 + coordinateMap->AddPoint(DPoint3d::FromSumOf(end,   perp1,  distance1));

            polyface->AddIndexedFacet(3, indices0);
            polyface->AddIndexedFacet(3, indices1);
            }
        }

    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsSolid(bvector<TileTriangle> const& triangles) 
    {
    for (auto& triangle : triangles)
        if (!triangle.IsSingleSided())
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsPolygon(DVec3dR normal, bvector<TileTriangle> const& triangles, TileMeshCR mesh) 
    {
    bvector<uint32_t>       indices;
    bool                    first = true;
    bvector<DPoint3d>       solidPoints;
    bvector<TileTriangle>   solidTriangles;

    for (auto& triangle : triangles)
        {
        DVec3d  triangleNormal = mesh.GetTriangleNormal(triangle);
        if (first)
            {
            normal = triangleNormal;
            }
        else if (!normal.IsParallelTo(triangleNormal))
            {
            return false;
            }
        }
    return true;
    }
    

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddExtrudedPolygon(PolyfaceHeaderR polyface, DVec3d normal, DRange3dCR classifiedRange, double expandDistance, uint16_t batchId, TileDisplayParamsCR displayParams)
    {
    DRay3d              ray = DRay3d::FromOriginAndVector(*polyface.GetPointCP(), normal);
    DRange1d            classifiedProjection = classifiedRange.GetCornerRange(ray);
    PolyfaceHeaderPtr   offsetPolyface = polyface.ComputeOffset(PolyfaceHeader::OffsetOptions(), classifiedProjection.high, classifiedProjection.low);

    if (offsetPolyface.IsValid())
        {
        offsetPolyface->Triangulate();
        AddClosedMesh(*offsetPolyface, expandDistance, batchId, displayParams);
        }
    else
        BeAssert(false && "classifier offset error");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshes (TileMeshCR mesh, bvector<TileTriangle> const& triangles, DRange3dCR classifiedRange)
    {
    // Need to process all triangles that have the same attribute value as a single "mesh".
    bmap <uint16_t, bvector<TileTriangle>> triangleMap;

    for (auto& triangle : triangles)
        {
        bvector<TileTriangle>   triangleVector(1, triangle);

        auto    insertPair = triangleMap.Insert(mesh.Attributes().at(triangle.m_indices[0]), triangleVector);

        if (!insertPair.second)
            insertPair.first->second.push_back(triangle);
        }
    
    for (auto& curr : triangleMap)
        {
        uint16_t                batchId = curr.first;
        PolyfaceHeaderPtr       polyface = BuildPolyfaceFromTriangles(mesh.Points(), curr.second);
        DVec3d                  polygonNormal;

        _InitMesh(); 

        if (IsSolid(curr.second))
            AddClosedMesh(*polyface, m_classifier.GetExpandDistance(), batchId, mesh.GetDisplayParams());
        else if (IsPolygon(polygonNormal, curr.second, mesh))
            AddExtrudedPolygon(*polyface, polygonNormal, classifiedRange, m_classifier.GetExpandDistance(), batchId, mesh.GetDisplayParams());
        else
            BeAssert (false && "invalid classifier geometry");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddPolylines (TileMeshCR mesh, bvector<TilePolyline> const& polylines, DRange3dCR classifiedRange)
    {
    // Need to process all polylines that have the same attribute value as a single "mesh".
    bmap <uint16_t, bvector<TilePolyline>> polylineMap;
    
    for (auto& polyline : polylines)
        {
        bvector<TilePolyline>   polylineVector(1, polyline);

        auto    insertPair = polylineMap.Insert(mesh.Attributes().at(polyline.m_indices[0]), polylineVector);

        if (!insertPair.second)
            insertPair.first->second.push_back(polyline);
        }
    
    for (auto& curr : polylineMap)
        {
        uint16_t                batchId = curr.first;
        DVec3d                  polylineNormal;
        PolyfaceHeaderPtr       polyface = BuildPolyfaceFromPolylines(polylineNormal, mesh.Points(), curr.second, m_classifier.GetExpandDistance());

        _InitMesh();

        AddExtrudedPolygon(*polyface, polylineNormal, classifiedRange, 0.0, batchId, mesh.GetDisplayParams());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddGeometry(PublishableTileGeometryR geometry, DRange3dCR classifiedRange, FeatureAttributesMapCR attributes)
    {
    for (auto& mesh : geometry.Meshes())
        {
        if (!mesh->Triangles().empty())
            AddMeshes(*mesh, mesh->Triangles(), classifiedRange);

        if (!mesh->Polylines().empty())
            AddPolylines(*mesh, mesh->Polylines(), classifiedRange);

        switch (m_classifier.GetInsideDisplay())
            {
            case ModelSpatialClassifier::DISPLAY_ElementColor:
                {
                bmap<uint16_t, DgnElementId>    attributeIndexToElementId;
                bmap<uint16_t, uint32_t>        colorIndexToColor;
                             
                for (auto& curr : attributes)
                    attributeIndexToElementId[curr.second] = curr.first.GetElementId();

                for (auto& curr :  mesh->GetColorIndexMap())
                    colorIndexToColor[curr.second] = curr.first;

                for (size_t i=0; i<mesh->Colors().size(); i++)
                    m_elementColors[attributeIndexToElementId[mesh->Attributes()[i]]] = colorIndexToColor[mesh->Colors()[i]];

                break;
                }

            default:
                {
                static ColorDef s_colors[] = { ColorDef::DarkGrey(),  /* Off */ ColorDef::White() /* On */, ColorDef::DarkGrey() /* Dimmed */, ColorDef::Magenta() /* Hilite*/ };

                for (auto& curr : attributes)
                    m_elementColors[curr.first.GetElementId()] = s_colors[m_classifier.GetInsideDisplay()].GetValue();
                    
                break;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddLabelsToBatchTable(Json::Value& json, FeatureAttributesMap const& attrs, char const* labelECProperty, DgnDbR db)
    {
    bool            validLabelsFound = false;
    Json::Value     labels = Json::arrayValue;

    for (auto const& kvp : attrs)
        {
        uint32_t            index = kvp.second;
        DgnElementId        elementId = kvp.first.GetElementId();

        ECN::ECValue    value;

        auto element = db.Elements().Get<DgnElement> (elementId);
        if (element.IsValid() &&

            DgnDbStatus::Success == element->GetPropertyValue(value, labelECProperty))
            {
            labels[index] = value.GetUtf8CP();
            validLabelsFound = true;
            }
        else
            labels[index] = Json::Value::GetNull();
        }
    if (validLabelsFound)
        json["label"] = std::move(labels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddColorsToBatchTable(Json::Value& json, FeatureAttributesMap const& attrs)
    {
    Json::Value     colors = Json::arrayValue;

    for (auto const& kvp : attrs)
        colors[kvp.second] = m_elementColors[kvp.first.GetElementId()];

    json["classifierColor"] = std::move(colors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddClosedMesh(PolyfaceHeaderR polyface, double expandDistance, uint16_t batchId, TileDisplayParamsCR displayParams) 
    {
    if (0.0 != expandDistance)
        {
        PolyfaceHeaderPtr           offsetPolyface = polyface.ComputeOffset(PolyfaceHeader::OffsetOptions(), expandDistance, 0.0, true, false, false);
        if (offsetPolyface.IsValid())
            {
            _AddClosedMesh(*offsetPolyface, batchId, displayParams);
            return;
            }
        else
            {
            BeAssert(false && "classifier offset error");
            }
        }
    else
        {
        _AddClosedMesh(polyface, batchId, displayParams);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  GetBatchTableString(FeatureAttributesMapCR featureAttributes, DgnDbR db, bool is3d)
    {
    BatchTableBuilder   batchTableBuilder(featureAttributes, db, is3d, m_context);

    AddLabelsToBatchTable(batchTableBuilder.GetJson(), featureAttributes, "Name", db);
    AddColorsToBatchTable(batchTableBuilder.GetJson(), featureAttributes);

    return  batchTableBuilder.ToString();
    }


};  // ClassifierTileWriter

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct VectorClassifierTileWriter : ClassifierTileWriter
{
    Json::Value                     m_featureTable;
    bvector<uint32_t>               m_meshIndices;
    bvector<FPoint3d>               m_meshPositions;
    bvector<uint32_t>               m_meshIndexCountBuffer;
    bvector<uint32_t>               m_meshIndexOffsetBuffer;
    ByteStream                      m_featureBinary;
    ByteStream                      m_batchIdsBuffer;
    VectorPosition                  m_lastPosition;
    DRange3d                        m_range;
    uint32_t                        m_nextMeshPointIndex;
    T_PointMap                      m_meshPointMap;

    virtual void _InitMesh() override { m_meshIndexOffsetBuffer.push_back(m_meshIndices.size()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VectorClassifierTileWriter(DRange3dCR range,  ModelSpatialClassifierCR classifier, PublisherContext& context) : ClassifierTileWriter(classifier, context), m_range(range), m_meshPointMap(TileUtil::PointComparator(s_pointTolerance)), m_nextMeshPointIndex(0)
    {
    m_featureTable["RTC_CENTER"][0] = 0.0; m_featureTable["RTC_CENTER"][1] = 0.0; m_featureTable["RTC_CENTER"][2] = 0.0;
    m_featureTable["MINIMUM_HEIGHT"] = -1000.0;
    m_featureTable["MAXIMUM_HEIGHT"] =  1000.0;
    m_featureTable["FORMAT"] = 1;  // Cartesian coordinates.

    auto& rectangle     = m_featureTable["RECTANGLE"];
    rectangle[0]  = range.low.x;
    rectangle[1]  = range.low.y;         
    rectangle[2]  = range.high.x;
    rectangle[3]  = range.high.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _AddClosedMesh(PolyfaceHeaderR polyface, uint16_t batchId, TileDisplayParamsCR displayParams) override
    {
    size_t      initialIndexSize = m_meshIndices.size();
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        for (size_t i=0; i<3; i++)
            m_meshIndices.push_back(m_meshPositions.size() + (uint32_t) visitor->GetClientPointIndexCP()[i]);

    for (size_t i=0; i<polyface.GetPointCount(); i++)
        m_meshPositions.push_back(FPoint3d::From(polyface.GetPointCP()[i]));

    uint32_t      indexCount = m_meshIndices.size() - initialIndexSize;
    m_batchIdsBuffer.Append((uint16_t)(batchId /* Subtract 1 to produce zero based batchIds - workaround this dependence in vector tiles */ - 1));
    m_meshIndexCountBuffer.push_back(indexCount);
    m_featureBinary.Append(indexCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Write(std::FILE* outputFile, TileNodeCR tile, DgnDbR db)
    {
    FeatureAttributesMap    featureAttributes = tile.GetAttributes();

    m_featureTable["MESHES_LENGTH"] = m_meshIndexCountBuffer.size();
    m_featureTable["MESH_POSITION_COUNT"] = m_meshPositions.size();
    m_featureTable["MESHES_COUNT"]["byteOffset"] = 0;
    m_featureTable["MESH_INDEX_COUNTS"]["byteOffset"] = m_featureBinary.size();
    m_featureBinary.Append((uint8_t const*) m_meshIndexCountBuffer.data(), m_meshIndexCountBuffer.size()*sizeof(uint32_t));
    m_featureTable["MESH_INDEX_OFFSETS"]["byteOffset"] = m_featureBinary.size();
    m_featureBinary.Append((uint8_t const*) m_meshIndexOffsetBuffer.data(), m_meshIndexOffsetBuffer.size()*sizeof(uint32_t));
    m_featureTable["MESH_BATCH_IDS"]["byteOffset"] = m_featureBinary.size();
    m_featureBinary.Append(m_batchIdsBuffer.data(), m_batchIdsBuffer.size());
    
    padTo4ByteBoundary(m_featureBinary);
    featureAttributes.RemoveUndefined();

    Utf8String          batchTableStr = GetBatchTableString(featureAttributes, db, tile.GetModel().Is3d()),
                        featureTableStr = getJsonString(m_featureTable);

    long    startPosition = ftell (outputFile);
    FWriteValue(Format::Vector, outputFile);
    FWriteValue(Vector::Version, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    FWriteValue((uint32_t) 0, outputFile);    // Filled in below.
    FWriteValue((uint32_t) featureTableStr.size(), outputFile);                                                                 // Feature table Json.
    FWriteValue((uint32_t) m_featureBinary.size(), outputFile);                                                                 // Feature Table binary.
    FWriteValue((uint32_t) batchTableStr.size(), outputFile);                                                                   // Batch table Json.
    FWriteValue((uint32_t) 0, outputFile);                                                                                      // No binary batch data.
    FWriteValue((uint32_t) (m_meshIndices.size() * sizeof(uint32_t)), outputFile);                                              // Indices byte length.
    FWriteValue((uint32_t) (m_meshPositions.size() * sizeof(FPoint3d)), outputFile);                                            // Positions byte length.
    FWriteValue((uint32_t) 0, outputFile);                                                                                      // No polygons positions.
    FWriteValue((uint32_t) 0, outputFile);                                                                                      // No point positions.
    FWrite(featureTableStr, outputFile);                                                                                        // Feature table Json.
    FWrite(m_featureBinary, outputFile);                                                                                        // Feature table binary.
    FWrite(batchTableStr, outputFile);                                                                                          // Batch table Json.
    FWrite(m_meshIndices, outputFile);
    FWrite(m_meshPositions, outputFile);

    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    FWriteValue(dataSize, outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }

};  // VectorClassifierTileWriter


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct BatchedClassifierTileWriter : ClassifierTileWriter
{
    TileMeshPtr         m_mesh;
    DRange3dR           m_outputContentRange;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BatchedClassifierTileWriter(DRange3dR outputContentRange,  ModelSpatialClassifierCR classifier, PublisherContext& context) : ClassifierTileWriter(classifier, context), m_outputContentRange(outputContentRange)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AddClosedMesh(PolyfaceHeaderR polyface, uint16_t batchId, TileDisplayParamsCR displayParams) override
    {
    if (!m_mesh.IsValid())
        m_mesh = TileMesh::Create(displayParams);
    
    m_outputContentRange.Extend (polyface.GetPointCP(), polyface.GetPointCount());
    uint32_t      pointCount = m_mesh->Points().size();

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        m_mesh->AddTriangle(TileTriangle((uint32_t) (pointCount + visitor->GetClientPointIndexCP()[0]), 
                                         (uint32_t) (pointCount + visitor->GetClientPointIndexCP()[1]), 
                                         (uint32_t) (pointCount + visitor->GetClientPointIndexCP()[2]), true));

    for (size_t i=0; i<polyface.GetPointCount(); i++)
        {
        m_mesh->AddVertex(polyface.GetPointCP()[i], nullptr, nullptr, (uint32_t) batchId, 0);
        m_mesh->SetValidIdsPresent(true);
        }
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   06/2017                                                                                                                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteClassifier(std::FILE* outputFile, PublishableTileGeometryR geometry, ModelSpatialClassifierCR classifier, DRange3dCR classifiedRange)
    {
    DRange3d        contentRange = DRange3d::NullRange();

    if (s_writeVectorClassifier)
        {
        VectorClassifierTileWriter      writer(contentRange, classifier, m_context);

        writer.AddGeometry(geometry, classifiedRange, m_tile.GetAttributes());
        writer.Write(outputFile, m_tile, m_context.GetDgnDb());

        for (auto& mesh : geometry.Meshes())
            contentRange.Extend(mesh->Points());

        }
    else
        {
        BatchedClassifierTileWriter     writer(contentRange, classifier, m_context);

        writer.AddGeometry(geometry, classifiedRange, m_tile.GetAttributes());

        if (writer.m_mesh.IsValid())
            {
            bvector<TileMeshPtr>    meshes(1, writer.m_mesh);
            WriteBatched3dModel (outputFile, meshes, writer.GetBatchTableString(m_tile.GetAttributes(), m_context.GetDgnDb(), m_tile.GetModel().Is3d()));
            }
        }
    (const_cast<TileNodeR> (m_tile)).SetTileRange(contentRange);      // Need to account for expansion by height.
    m_tile.SetPublishedRange (contentRange);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteGltf(std::FILE* outputFile, PublishTileData const& tileData)
    {
    Utf8String  sceneStr = tileData.GetJsonString();
    uint32_t    sceneStrLength = static_cast<uint32_t>(sceneStr.size());

    long    startPosition = ftell(outputFile);
    FWriteValue(Format::Gltf, outputFile);
    FWriteValue(Gltf::Version, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    FWriteValue((uint32_t) 0, outputFile);        // Filled in below.
    FWriteValue(sceneStrLength, outputFile);
    FWriteValue(Gltf::SceneFormat, outputFile);

    std::fwrite(sceneStr.data(), 1, sceneStrLength, outputFile);
    if (!tileData.m_binaryData.empty())
        std::fwrite(tileData.m_binaryData.data(), 1, tileData.BinaryDataSize(), outputFile);

    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    FWriteValue(dataSize, outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshes(PublishTileData& tileData, TileMeshList const& meshList)
    {
    size_t          meshIndex = 0, primitiveIndex = 0;
    Json::Value     meshes     = Json::objectValue;
    Json::Value     nodes      = Json::objectValue;
    Json::Value     rootNode   = Json::objectValue;

    Utf8PrintfString    meshName("Mesh_%d", meshIndex++);

    meshes[meshName] = CreateMesh (meshList, tileData, primitiveIndex);
    rootNode["meshes"].append (meshName);
    rootNode["matrix"] = PublisherContext::TransformToJson(Transform::FromIdentity());
    nodes["rootNode"] = rootNode;
    tileData.m_json["meshes"] = meshes;
    tileData.m_json["nodes"]  = nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddExtensions(PublishTileData& tileData)
    {
    tileData.m_json["extensionsUsed"] = Json::arrayValue;
    tileData.m_json["extensionsUsed"].append("KHR_binary_glTF");
    tileData.m_json["extensionsUsed"].append("CESIUM_RTC");
    tileData.m_json["extensionsUsed"].append("WEB3D_quantized_attributes");

    tileData.m_json["glExtensionsUsed"] = Json::arrayValue;
    tileData.m_json["glExtensionsUsed"].append("OES_element_index_uint");

    tileData.m_json["extensions"]["CESIUM_RTC"]["center"] = Json::arrayValue;
    tileData.m_json["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.x);
    tileData.m_json["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.y);
    tileData.m_json["extensions"]["CESIUM_RTC"]["center"].append(m_centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/02016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddDefaultScene (PublishTileData& tileData)
    {
    tileData.m_json["scene"] = "defaultScene";
    tileData.m_json["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    tileData.m_json["scenes"]["defaultScene"]["nodes"].append("rootNode");
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
Utf8String TilePublisher::AddTextureImage (PublishTileData& tileData, TileTextureImageCPtr& textureImage, TileMeshCR mesh, Utf8CP  suffix)
    {
    auto const& found = m_textureImages.find (textureImage);

    // For composite tiles, we must ensure that the texture is defined for each individual tile - cannot share
    bool textureExists = found != m_textureImages.end();
    if (textureExists && tileData.m_json.isMember("textures") && tileData.m_json["textures"].isMember(found->second.c_str()))
        return found->second;

    bool        hasAlpha = textureImage->GetImageSource().GetFormat() == ImageSource::Format::Png;

    Utf8String  textureId = Utf8String ("texture_") + suffix;
    Utf8String  imageId   = Utf8String ("image_")   + suffix;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + suffix;

    tileData.m_json["textures"][textureId] = Json::objectValue;
    tileData.m_json["textures"][textureId]["format"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    tileData.m_json["textures"][textureId]["internalFormat"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    tileData.m_json["textures"][textureId]["sampler"] = "sampler_0";
    tileData.m_json["textures"][textureId]["source"] = imageId;

    tileData.m_json["images"][imageId] = Json::objectValue;


    DRange3d    range = mesh.GetRange(), uvRange = mesh.GetUVRange();
    Image       image (textureImage->GetImageSource(), hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);

    // This calculation should actually be made for each triangle and maximum used.
    static      double      s_requiredSizeRatio = 2.0, s_sizeLimit = 1024.0;
    double      requiredSize = std::min (s_sizeLimit, s_requiredSizeRatio * range.DiagonalDistance () / (m_tile.GetTolerance() * std::min (1.0, uvRange.DiagonalDistance())));
    DPoint2d    imageSize = { (double) image.GetWidth(), (double) image.GetHeight() };

    tileData.m_json["bufferViews"][bvImageId] = Json::objectValue;
    tileData.m_json["bufferViews"][bvImageId]["buffer"] = "binary_glTF";

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

    ImageSource         imageSource = textureImage->GetImageSource();
    static const int    s_imageQuality = 50;


    if (targetImageSize.x != imageSize.x || targetImageSize.y != imageSize.y)
        {
        Image           targetImage = Image::FromResizedImage (targetImageSize.x, targetImageSize.y, image);

        imageSource = ImageSource (targetImage, textureImage->GetImageSource().GetFormat(), s_imageQuality);
        }

    if (m_context.GetTextureMode() == PublisherContext::External ||
        m_context.GetTextureMode() == PublisherContext::Compressed)
        {
        WString     name = WString(imageId.c_str(), true) + L"_" + m_tile.GetNameSuffix(), extension;

#ifdef COMPRESSED_TEXTURE_SUPPORT
        if (m_context.GetTextureMode() == PublisherContext::Compressed) 
            {
            extension = L"crn";

            static crn_uint32       s_qualityLevel = 128;
            static crn_dxt_quality  s_dxtQuality   = cCRNDXTQualitySuperFast;

            Image       image (imageSource, Image::Format::Rgba);

            crn_comp_params     compressParams;                                                                                                                 
            
            compressParams.m_width  = image.GetWidth(); 
            compressParams.m_height = image.GetHeight();
            compressParams.m_quality_level = s_qualityLevel; 
            compressParams.m_dxt_quality = s_dxtQuality;
            compressParams.m_file_type = cCRNFileTypeCRN;

            compressParams.m_dxt_compressor_type = cCRNDXTCompressorCRN;
            compressParams.m_pImages[0][0] = reinterpret_cast <crn_uint32 const*> (image.GetByteStream().GetData());

            crn_uint32      compressedSize;
            StopWatch       timer(true);
            
            void*   compressedData = crn_compress (compressParams, compressedSize);

            BeMutexHolder lock(m_context.m_statistics.m_mutex);
            m_context.m_statistics.m_textureCompressionSeconds    += timer.GetCurrentSeconds();
            m_context.m_statistics.m_textureCompressionMegaPixels += (double) (image.GetWidth() * image.GetHeight()) / (1024.0 * 1024.0);

             std::FILE*  outputFile = _wfopen(BeFileName(nullptr, m_context.GetDataDirForModel(m_tile.GetModel()).c_str(), name.c_str(), extension.c_str()).c_str(), L"wb");
            fwrite (compressedData, 1, compressedSize, outputFile);
            crn_free_block (compressedData);
            fclose (outputFile);
            }
        else
#endif
            {
            extension = ImageSource::Format::Jpeg == imageSource.GetFormat() ? L"jpg" : L"png";
            std::FILE*  outputFile = _wfopen(BeFileName(nullptr, m_context.GetModelDataDirectory(m_tile.GetModel().GetModelId(), nullptr).c_str(), name.c_str(), extension.c_str()).c_str(), L"wb");
            fwrite (imageSource.GetByteStream().GetData(), 1, imageSource.GetByteStream().GetSize(), outputFile);
            fclose (outputFile);
            }
        tileData.m_json["images"][imageId]["uri"] = Utf8String(BeFileName(nullptr, nullptr, name.c_str(), extension.c_str()).c_str());
        tileData.m_json["images"][imageId]["name"] = imageId; 
        }
    else
        {
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = imageSource.GetFormat() == ImageSource::Format::Png ? "image/png" : "image/jpeg";


        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = targetImageSize.x;
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = targetImageSize.y;

        ByteStream const& imageData = imageSource.GetByteStream();
        tileData.m_json["bufferViews"][bvImageId]["byteOffset"] = tileData.BinaryDataSize();
        tileData.m_json["bufferViews"][bvImageId]["byteLength"] = imageData.size();
        tileData.AddBinaryData (imageData.data(), imageData.size());
        }

    if (!textureExists)
        m_textureImages.Insert (textureImage, textureId);

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddColorIndex(PublishTileData& tileData, TilePublish::ColorIndex& colorIndex, TileMeshCR mesh, Utf8CP suffix)
    {
    Utf8String textureId("texture_"),
               imageId("image_"),
               bvImageId("imageBufferView");

    textureId.append(suffix);
    imageId.append(suffix);
    bvImageId.append(suffix);

    auto& texture = tileData.m_json["textures"][textureId] = Json::objectValue;
    texture["format"] = static_cast<int32_t>(Gltf::DataType::Rgba);
    texture["internalFormat"] = static_cast<int32_t>(Gltf::DataType::Rgba);
    texture["sampler"] = "sampler_1";
    texture["source"] = imageId;

    auto& bufferView = tileData.m_json["bufferViews"][bvImageId] = Json::objectValue;
    bufferView["buffer"] = "binary_glTF";

    auto& image = tileData.m_json["images"][imageId] = Json::objectValue;
    auto& imageExtensions = image["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    imageExtensions["bufferView"] = bvImageId;
    imageExtensions["mimeType"] = "image/png";
    imageExtensions["width"] = colorIndex.GetWidth();
    imageExtensions["height"] = colorIndex.GetHeight();

    ImageSource imageSource(colorIndex.ExtractImage(), ImageSource::Format::Png);
    ByteStream const& imageData = imageSource.GetByteStream();
    bufferView["byteOffset"] = tileData.BinaryDataSize();
    bufferView["byteLength"] = imageData.size();
    tileData.AddBinaryData(imageData.data(), imageData.size());

#if defined(DEBUG_COLOR_INDEX)
    WString name = WString(imageId.c_str(), true) + L"_" + m_tile.GetNameSuffix(), extension;
    std::FILE* outputFile = _wfopen(BeFileName(nullptr, m_context.GetDataDirForModel(m_tile.GetModel()).c_str(), name.c_str(), L"png").c_str(), L"wb");
    fwrite(imageData.GetData(), 1, imageData.GetSize(), outputFile);
    fclose(outputFile);
#endif

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void addTransparencyToTechnique (Json::Value& technique)
    {
    technique["states"]["enable"].append (3042);  // BLEND

    auto&   techniqueFunctions =    technique["states"]["functions"] = Json::objectValue;

    techniqueFunctions["blendEquationSeparate"] = Json::arrayValue;
    techniqueFunctions["blendFuncSeparate"]     = Json::arrayValue;

    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (rgb)
    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (alpha)

    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcRGB)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstRGB)
    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcAlpha)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstAlpha)

    techniqueFunctions["depthMask"] = "false";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddMeshShaderTechnique(PublishTileData& data, MeshMaterial const& mat, bool doBatchIds)
    {
    Utf8String prefix = mat.GetTechniqueNamePrefix();

    if (!doBatchIds)
        prefix.append("NoId");

    Utf8String techniqueName(prefix);
    techniqueName.append("Technique");

    if (data.m_json.isMember("techniques") && data.m_json["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value technique(Json::objectValue);

    AddTechniqueParameter(technique, "mv", Gltf::DataType::FloatMat4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", Gltf::DataType::FloatMat4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", Gltf::DataType::FloatVec3, "POSITION");
    if (!mat.IgnoresLighting())
        {
        AddTechniqueParameter(technique, "n", Gltf::DataType::IntVec2, "NORMAL");
        AddTechniqueParameter(technique, "nmx", Gltf::DataType::FloatMat3, "MODELVIEWINVERSETRANSPOSE");
        }

    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", Gltf::DataType::Float, "_BATCHID");

    if (!mat.IsTextured())
        AddTechniqueParameter(technique, "colorIndex", Gltf::DataType::Float, "_COLORINDEX");


    Utf8String         programName               = prefix + "Program";
    Utf8String         vertexShader              = prefix + "VertexShader";
    Utf8String         fragmentShader            = prefix + "FragmentShader";
    Utf8String         vertexShaderBufferView    = vertexShader + "BufferView";
    Utf8String         fragmentShaderBufferView  = fragmentShader + "BufferView";

    technique["program"] = programName.c_str();

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(Gltf::DepthTest);
    techniqueStates["disable"].append(Gltf::CullFace);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";
    if (doBatchIds)
        techniqueAttributes["a_batchId"] = "batch";

    if (!mat.IsTextured())
        techniqueAttributes["a_colorIndex"] = "colorIndex";

    if(!mat.IgnoresLighting())
        techniqueAttributes["a_n"] = "n";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";
    if (!mat.IgnoresLighting())
        techniqueUniforms["u_nmx"] = "nmx";

    auto& rootProgramNode = (data.m_json["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");

    if (doBatchIds)
        AppendProgramAttribute(rootProgramNode, "a_batchId");
    if (!mat.IgnoresLighting())
        AppendProgramAttribute(rootProgramNode, "a_n");

    rootProgramNode["vertexShader"]   = vertexShader.c_str();
    rootProgramNode["fragmentShader"] = fragmentShader.c_str();

    auto& shaders = data.m_json["shaders"];
    AddShader(shaders, vertexShader.c_str(), Gltf::VertexShader, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), Gltf::FragmentShader, fragmentShaderBufferView.c_str());

    bool color2d = false;
    std::string vertexShaderString = s_shaderPrecision;
    if (doBatchIds)
        vertexShaderString.append(s_batchIdShaderAttribute);

    vertexShaderString.append(mat.GetVertexShaderString());

    data.AddBufferView(vertexShaderBufferView.c_str(),  vertexShaderString);
    data.AddBufferView(fragmentShaderBufferView.c_str(), mat.GetFragmentShaderString());

    mat.AddTechniqueParameters(technique, rootProgramNode, data);

    data.m_json["techniques"][techniqueName.c_str()] = technique;

    return techniqueName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineMaterial::PolylineMaterial(TileMeshCR mesh, bool is3d, Utf8CP suffix)
    : TileMaterial(Utf8String("PolylineMaterial_")+suffix)
    {
    TileDisplayParamsCR displayParams = mesh.GetDisplayParams();

    m_type = displayParams.GetRasterWidth() <= 1 ? PolylineType::Simple : PolylineType::Tesselated;

    ColorIndexMapCR map = mesh.GetColorIndexMap();
    m_hasAlpha = map.HasTransparency();         // || IsTesselated(); // Turn this on if we use alpha in tesselated polylines.
    m_colorDimension = ColorIndex::CalcDimension(map.GetNumIndices());
    m_width = static_cast<double> (displayParams.GetRasterWidth());

    if (0 != displayParams.GetLinePixels())
        {
        static uint32_t         s_height = 2;
        ByteStream              bytes(32 * 4 * s_height);
        uint32_t*               dataP = (uint32_t*) bytes.GetDataP();

        for (uint32_t y=0, mask = 0x0001; y < s_height; y++)
            for (uint32_t x=0, mask = 0x0001; x < 32; x++, mask = mask << 1)
                *dataP++ = (0 == (mask & displayParams.GetLinePixels())) ? 0 : 0xffffffff;

        Render::Image   image (32, s_height, std::move(bytes), Image::Format::Rgba);

        m_texture = TileTextureImage::Create (Render::ImageSource(image, Render::ImageSource::Format::Png));
        m_textureLength = -32;          // Negated to denote cosmetic.
        }
    m_adjustColorForBackground = !is3d;

#ifdef  TEST_IMAGE 
    FILE*       pFile;
    if (nullptr != (pFile = fopen("d:\\tmp\\RailRoad.png", "rb")))
        {
        fseek (pFile, 0, SEEK_END);
        
        size_t      fileSize = ftell(pFile);
        ByteStream  bytes(fileSize);
        fseek (pFile, 0, SEEK_SET);
        fread (bytes.data(), 1, fileSize, pFile);
        fclose(pFile);

        ImageSource imageSource (ImageSource::Format::Png, std::move(bytes));
        Image       image(imageSource, Image::Format::Rgba);  
        static      uint32_t s_minDimension = 64;

        Point2d     sourceImageSize = { (int) image.GetWidth(), (int) image.GetHeight() },
                    targetImageSize =  { roundToMultipleOfTwo(std::max(image.GetWidth(), s_minDimension)), roundToMultipleOfTwo(std::max(image.GetHeight(), s_minDimension)) };

        if (targetImageSize.x != image.GetWidth() || targetImageSize.y != image.GetHeight())
            imageSource = ImageSource (Image::FromResizedImage (targetImageSize.x, targetImageSize.y, image), ImageSource::Format::Png);
        m_texture = TileTextureImage::Create (imageSource);
        m_textureLength = -(double) sourceImageSize.x;
        m_width = (double) sourceImageSize.y / 2;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolylineMaterial::GetTechniqueNamePrefix() const
    {
    Utf8String prefix = PolylineType::Simple == GetType() ? "Simple" : "Tesselated";

    prefix.append (IsTextured() ? "Textured" : "Solid");
    prefix.append("Polyline");
    prefix.append(std::to_string(static_cast<uint8_t>(GetColorIndexDimension())).c_str());
    return prefix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string PolylineMaterial::_GetVertexShaderString() const
    {
    std::string const* list = nullptr;
    if (IsTextured())
        list = IsTesselated() ? s_tesselatedTexturedPolylineVertexShaders : s_simpleTexturedPolylineVertexShaders;
    else
        list = IsTesselated() ? s_tesselatedSolidPolylineVertexShaders  : s_simpleSolidPolylineVertexShaders;

    auto index = static_cast<uint8_t>(GetColorIndexDimension());

    return list[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string TileMaterial::GetVertexShaderString() const
    {
    std::string vs = _GetVertexShaderString();

    if (!m_adjustColorForBackground)
        return vs;

    Utf8String vs2d(s_adjustBackgroundColorContrast.c_str());
    vs2d.append(vs.c_str());
    vs2d.ReplaceAll("v_color = computeColor()", "v_color = adjustContrast(computeColor())");
    return vs2d.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string const& PolylineMaterial::GetFragmentShaderString() const
    {
    return IsTextured () ? s_texturedPolylineFragmentShader : s_solidPolylineFragmentShader;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineMaterial::AddTechniqueParameters(Json::Value& tech, Json::Value& prog, PublishTileData& data) const
    {
    AddColorIndexTechniqueParameters(tech, prog, data);
    if (IsTextured())
        AddTextureTechniqueParameters(tech, prog, data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshMaterial::MeshMaterial(TileMeshCR mesh, bool is3d, Utf8CP suffix, DgnDbR db) : TileMaterial(Utf8String("Material_")+suffix)
    {
    TileDisplayParamsCR params = mesh.GetDisplayParams();
    m_ignoreLighting = params.GetIgnoreLighting();

    params.ResolveTextureImage(db);
    m_texture = params.GetTextureImage();

    uint32_t rgbInt = params.GetColor();
    double alpha = 1.0 - ((uint8_t*)&rgbInt)[3] / 255.0;

    if (params.GetRenderMaterialId().IsValid())
        {
        m_material = RenderMaterial::Get(db, params.GetRenderMaterialId());
        if (m_material.IsValid())
            {
            auto jsonMat = &m_material->GetRenderingAsset();
            m_overridesRgb = jsonMat->GetBool(RENDER_MATERIAL_FlagHasBaseColor, false);
            m_overridesAlpha = jsonMat->GetBool(RENDER_MATERIAL_FlagHasTransmit, false);

            if (m_overridesRgb)
                m_rgbOverride = jsonMat->GetColor(RENDER_MATERIAL_Color);

            if (m_overridesAlpha)
                {
                m_alphaOverride = jsonMat->GetDouble(RENDER_MATERIAL_Transmit, 0.0);
                alpha = m_alphaOverride;
                }
            else if (m_overridesRgb)
                {                                                                                                                  
                // Apparently overriding RGB without specifying transmit => opaque.
                m_alphaOverride = 0.0;
                alpha = m_alphaOverride;
                m_overridesAlpha = true;
                }

            if (jsonMat->GetBool(RENDER_MATERIAL_FlagHasSpecularColor, false))
                m_specularColor = jsonMat->GetColor(RENDER_MATERIAL_SpecularColor);

            constexpr double s_finishScale = 15.0;
            if (jsonMat->GetBool(RENDER_MATERIAL_FlagHasFinish, false))
                m_specularExponent = jsonMat->GetDouble(RENDER_MATERIAL_Finish, s_qvSpecular) * s_finishScale;
            }
        }

    if (IsTextured() || m_overridesAlpha)
        m_hasAlpha = alpha > 0.0;
    else
        m_hasAlpha = mesh.GetColorIndexMap().HasTransparency();

    m_adjustColorForBackground = !is3d && !params.GetIsColorFromBackground();

    if (m_overridesAlpha && m_overridesRgb)
        m_colorDimension = ColorIndex::Dimension::Zero;
    else
        m_colorDimension = params.GetIsColorFromBackground() ? ColorIndex::Dimension::Background : ColorIndex::CalcDimension(mesh.GetColorIndexMap().GetNumIndices());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string MeshMaterial::_GetVertexShaderString() const
    {
    if (IsTextured())
        return IgnoresLighting() ? s_unlitTextureVertexShader : s_texturedVertexShader;

    auto index = static_cast<uint8_t>(GetColorIndexDimension());
    BeAssert(index < _countof(s_untexturedVertexShaders));

    std::string const* list = IgnoresLighting() ? s_unlitVertexShaders : s_untexturedVertexShaders;
    return list[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string const& MeshMaterial::GetFragmentShaderString() const
    {
    if (IsTextured())
        return IgnoresLighting() ? s_unlitTextureFragmentShader : s_texturedFragShader;
    else
        return IgnoresLighting() ? s_unlitFragmentShader : s_untexturedFragShader;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MeshMaterial::GetTechniqueNamePrefix() const
    {
    Utf8String prefix = IsTextured() ? "Textured" : "Untextured";
    if (HasTransparency())
        prefix.append("Transparent");

    if (IgnoresLighting())
        prefix.append("Unlit");

    prefix.append(std::to_string(static_cast<uint8_t>(GetColorIndexDimension())).c_str());

    return prefix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMaterial::AddColorIndexTechniqueParameters(Json::Value& technique, Json::Value& program, PublishTileData& data) const
    {
    auto dim = GetColorIndexDimension();
    auto& techniqueUniforms = technique["uniforms"];
    if (TilePublish::ColorIndex::Dimension::Zero != dim)
        {
        TilePublisher::AddTechniqueParameter(technique, "tex", Gltf::DataType::Sampler2d, "_3DTILESDIFFUSE");
        TilePublisher::AddTechniqueParameter(technique, "colorIndex", Gltf::DataType::Float, "_COLORINDEX");

        techniqueUniforms["u_tex"] = "tex";
        techniqueUniforms["u_texStep"] = "texStep";

        technique["attributes"]["a_colorIndex"] = "colorIndex";
        TilePublisher::AppendProgramAttribute(program, "a_colorIndex");

        auto& sampler = data.m_json["samplers"]["sampler_1"];
        sampler["minFilter"] = Gltf::Nearest;
        sampler["maxFilter"] = Gltf::Nearest;
        sampler["wrapS"] = Gltf::ClampToEdge;
        sampler["wrapT"] = Gltf::ClampToEdge;

        if (TilePublish::ColorIndex::Dimension::Two == dim)
            {
            TilePublisher::AddTechniqueParameter(technique, "texWidth", Gltf::DataType::Float, nullptr);
            TilePublisher::AddTechniqueParameter(technique, "texStep", Gltf::DataType::FloatVec4, nullptr);

            techniqueUniforms["u_texWidth"] = "texWidth";
            }
        else
            {
            TilePublisher::AddTechniqueParameter(technique, "texStep", Gltf::DataType::FloatVec2, nullptr);
            }
        }
    else
        {
        TilePublisher::AddTechniqueParameter(technique, "color", Gltf::DataType::FloatVec4, "_3DTILESDIFFUSE");
        techniqueUniforms["u_color"] = "color";
        }

    if (HasTransparency())
        addTransparencyToTechnique(technique);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMaterial::AddTextureTechniqueParameters(Json::Value& technique, Json::Value& program, PublishTileData& data) const
    {
    BeAssert (IsTextured());
    if (IsTextured())
        {
        TilePublisher::AddTechniqueParameter(technique, "tex", Gltf::DataType::Sampler2d, "_3DTILESDIFFUSE");
        TilePublisher::AddTechniqueParameter(technique, "texc", Gltf::DataType::FloatVec2, "TEXCOORD_0");

        data.m_json["samplers"]["sampler_0"] = Json::objectValue;
        data.m_json["samplers"]["sampler_0"]["minFilter"] = Gltf::Linear;
        data.m_json["samplers"]["sampler_0"]["magFilter"] = Gltf::Linear;
        if (!m_texture->GetRepeat())
            {
            data.m_json["samplers"]["sampler_0"]["wrapS"] = Gltf::ClampToEdge;
            data.m_json["samplers"]["sampler_0"]["wrapT"] = Gltf::ClampToEdge;
            }
        technique["uniforms"]["u_tex"] = "tex";
        technique["attributes"]["a_texc"] = "texc";
        TilePublisher::AppendProgramAttribute(program, "a_texc");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshMaterial::AddTechniqueParameters(Json::Value& technique, Json::Value& program, PublishTileData& data) const
    {
    if (IsTextured())
        AddTextureTechniqueParameters(technique, program, data);
    else
        AddColorIndexTechniqueParameters(technique, program, data);

    if (!IgnoresLighting())
        {
        // Specular...
        TilePublisher::AddTechniqueParameter(technique, "specularColor", Gltf::DataType::FloatVec3, nullptr);
        technique["uniforms"]["u_specularColor"] = "specularColor";

        TilePublisher::AddTechniqueParameter(technique, "specularExponent", Gltf::DataType::Float, nullptr);
        technique["uniforms"]["u_specularExponent"] = "specularExponent";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    TilePublisher::AddMaterialColor(Json::Value& matJson, TileMaterial& mat, PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix)
    {
    auto dim = mat.GetColorIndexDimension();
    if (TilePublish::ColorIndex::Dimension::Zero != dim)
        {
        TilePublish::ColorIndex colorIndex(mesh, mat);
        matJson["values"]["tex"] = AddColorIndex(tileData, colorIndex, mesh, suffix);

        uint16_t width = colorIndex.GetWidth();
        double stepX = 1.0 / width;
        double stepY = 1.0 / colorIndex.GetHeight();

        auto& texStep = matJson["values"]["texStep"] = Json::arrayValue;
        texStep.append(stepX);
        texStep.append(stepX * 0.5);    // centerX

        if (TilePublish::ColorIndex::Dimension::Two == mat.GetColorIndexDimension())
            {
            texStep.append(stepY);
            texStep.append(stepY * 0.5);    // centerY

            matJson["values"]["texWidth"] = width;
            }
        }
    else
        {
        BeAssert(1 == mesh.GetColorIndexMap().GetNumIndices() || (mat.OverridesRgb() && mat.OverridesAlpha()));
        ColorDef baseDef(mesh.GetColorIndexMap().begin()->first);
        RgbFactor rgb = mat.OverridesRgb() ? mat.GetRgbOverride() : RgbFactor::FromIntColor(baseDef.GetValue());
        double alpha = mat.OverridesAlpha() ? mat.GetAlphaOverride() : baseDef.GetAlpha()/255.0;

        auto& matColor = matJson["values"]["color"];
        matColor.append(rgb.red);
        matColor.append(rgb.green);
        matColor.append(rgb.blue);
        matColor.append(1.0 - alpha);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshMaterial TilePublisher::AddMeshMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    MeshMaterial mat(mesh,  m_tile.GetModel().Is3d(), suffix, m_context.GetDgnDb());

    Json::Value& matJson = tileData.m_json["materials"][mat.GetName().c_str()];

    auto matId = mesh.GetDisplayParams().GetRenderMaterialId();
    if (matId.IsValid())
        matJson["materialId"] = matId.ToString(); // Do we actually use this?

    if (nullptr != mat.GetDgnMaterial())
        matJson["name"] = mat.GetDgnMaterial()->GetMaterialName().c_str();

    if (mat.IsTextured())
        {
        TileTextureImageCPtr    texture = mat.GetTexture();
        matJson["values"]["tex"] = AddTextureImage(tileData, texture, mesh, suffix);
        }
    else
        AddMaterialColor (matJson, mat, tileData, mesh, suffix);

    matJson["technique"] = AddMeshShaderTechnique(tileData, mat, doBatchIds).c_str();

    if (!mat.IgnoresLighting())
        {
        matJson["values"]["specularExponent"] = mat.GetSpecularExponent();

        auto& specColor = matJson["values"]["specularColor"];
        specColor.append(mat.GetSpecularColor().red);
        specColor.append(mat.GetSpecularColor().green);
        specColor.append(mat.GetSpecularColor().blue);
        }

    return mat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineMaterial TilePublisher::AddTesselatedPolylineMaterial (PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    PolylineMaterial mat = AddPolylineMaterial(tileData, mesh, suffix, doBatchIds);
    BeAssert(mat.IsTesselated());

    Json::Value& matValues = tileData.m_json["materials"][mat.GetName()]["values"];
    matValues["width"] = mat.GetWidth();;
    return mat;
    }      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineMaterial TilePublisher::AddSimplePolylineMaterial (PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    PolylineMaterial mat = AddPolylineMaterial(tileData, mesh, suffix, doBatchIds);
    BeAssert(mat.IsSimple());
    return mat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineMaterial TilePublisher::AddPolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    PolylineMaterial mat(mesh,  m_tile.GetModel().Is3d(), suffix);

    auto& matJson = tileData.m_json["materials"][mat.GetName().c_str()];
    matJson["technique"] = AddPolylineTechnique(tileData, mat, doBatchIds);


    if (mat.IsTextured())
        {
        TileTextureImageCPtr    texture = mat.GetTexture();

        matJson["values"]["texLength"] = mat.GetTextureLength();
        matJson["values"]["tex"] = AddTextureImage(tileData, texture, mesh, suffix);
        }

    AddMaterialColor (matJson, mat, tileData, mesh, suffix);

    return mat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddPolylineTechnique(PublishTileData& tileData, PolylineMaterial const& mat, bool doBatchIds)
    {
    bool is3d = m_tile.GetModel().Is3d();
    Utf8String prefix(mat.GetTechniqueNamePrefix());
    prefix.append(is3d ? "3d" : "2d");
    if (!doBatchIds)
        prefix.append("NoId");

    Utf8String techniqueName = prefix + "Technique";

    if (tileData.m_json.isMember("techniques") && tileData.m_json["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value technique(Json::objectValue);
    AddTechniqueParameter(technique, "mv", Gltf::DataType::FloatMat4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", Gltf::DataType::FloatMat4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", Gltf::DataType::FloatVec3, "POSITION");
    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", Gltf::DataType::Float, "_BATCHID");

    auto& enableStates = technique["states"]["enable"] = Json::arrayValue;
    enableStates.append(Gltf::DepthTest);

    auto& attributes = technique["attributes"];
    attributes["a_pos"] = "pos";
    if (doBatchIds)
        attributes["a_batchId"] = "batch";

    auto& uniforms = technique["uniforms"];
    uniforms["u_mv"] = "mv";
    uniforms["u_proj"] = "proj";

    Utf8String programName = prefix + "Program",
               vertexShaderName = prefix + "VertexShader",
               fragmentShaderName = prefix + "FragmentShader",
               vertexShaderBufferViewName = vertexShaderName + "BufferView",
               fragmentShaderBufferViewName = fragmentShaderName + "BufferView";

    technique["program"] = programName;
    auto& programRoot = tileData.m_json["programs"][programName.c_str()];
    programRoot["vertexShader"] = vertexShaderName;
    programRoot["fragmentShader"] = fragmentShaderName;

    auto& shaders = tileData.m_json["shaders"];
    AddShader(shaders, vertexShaderName.c_str(), Gltf::VertexShader, vertexShaderBufferViewName.c_str());
    AddShader(shaders, fragmentShaderName.c_str(), Gltf::FragmentShader, fragmentShaderBufferViewName.c_str());

    std::string vertexShaderString(s_shaderPrecision);
    if (doBatchIds)
        vertexShaderString.append(s_batchIdShaderAttribute);

    vertexShaderString.append(mat.GetVertexShaderString());
    tileData.AddBufferView(vertexShaderBufferViewName.c_str(), vertexShaderString);
    tileData.AddBufferView(fragmentShaderBufferViewName.c_str(), mat.GetFragmentShaderString());

    programRoot["attributes"] = Json::arrayValue;
    AppendProgramAttribute(programRoot, "a_pos");
    if (doBatchIds)
        AppendProgramAttribute(programRoot, "a_batchId");

    mat.AddTechniqueParameters(technique, programRoot, tileData);


    if (mat.IsTesselated())
        {
        // NB: reference to "attributes" and "uniforms" declared above is potentially invalid after adding to parent node...due to use of btree instead of bmap in Json::Value...
        AddTechniqueParameter(technique, "color", Gltf::DataType::FloatVec4, nullptr);
        AddTechniqueParameter(technique, "width", Gltf::DataType::Float, nullptr);
        technique["uniforms"]["u_color"] = "color";
        technique["uniforms"]["u_width"] = "width";

        AddTechniqueParameter(technique, "prev", Gltf::DataType::FloatVec3, "PREV");
        AddTechniqueParameter(technique, "next", Gltf::DataType::FloatVec3, "NEXT");
        AddTechniqueParameter(technique, "param", Gltf::DataType::FloatVec2, "PARAM");


        technique["attributes"]["a_prev"] = "prev";
        technique["attributes"]["a_next"] = "next";
        technique["attributes"]["a_param"] = "param";
        AppendProgramAttribute(programRoot, "a_prev");
        AppendProgramAttribute(programRoot, "a_next");
        AppendProgramAttribute(programRoot, "a_param");
        }

    if (mat.IsTextured())
        {
        AddTechniqueParameter(technique, "texLength", Gltf::DataType::Float, nullptr);
        technique["uniforms"]["u_texLength"] = "texLength";

        AddTechniqueParameter(technique, "texScalePnt", Gltf::DataType::FloatVec3, "TEXSCALEPNT");
        technique["attributes"]["a_texScalePnt"] = "texScalePnt";
        AppendProgramAttribute(programRoot, "a_texScalePnt");

        technique["attributes"]["a_distance"] = "distance";
        AppendProgramAttribute(programRoot, "a_distance");
        AddTechniqueParameter(technique, "distance", Gltf::DataType::Float, "DISTANCE");
        }


    tileData.m_json["techniques"][techniqueName] = technique;
    return techniqueName;
    }

static double   clamp(double value, double min, double max)  { return value < min ? min : (value > max ? max : value);  }
static double   signNotZero(double value) { return value < 0.0 ? -1.0 : 1.0; }
static uint16_t toSNorm(double value) { return static_cast <uint16_t> (.5 + (clamp(value, -1.0, 1.0) * 0.5 + 0.5) * 255.0); }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static uint16_t octEncodeNormal (DVec3dCR vector)
    {
    DPoint2d    result;
    double      denom = fabs(vector.x) + fabs(vector.y) + fabs(vector.z);

    result.x = vector.x / denom;
    result.y = vector.y / denom;

    if (vector.z < 0) 
        {
        double x = result.x;
        double y = result.y;
        result.x = (1.0 - fabs(y)) * signNotZero(x);
        result.y = (1.0 - fabs(x)) * signNotZero(y);
        }
    return toSNorm(result.y) << 8 | toSNorm(result.x);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddMeshVertexAttributes (PublishTileData& tileData, double const* values, Utf8CP name, Utf8CP id, size_t nComponents, size_t nAttributes, char const* accessorType, VertexEncoding encoding, double const* min, double const* max)
    {
    tileData.PadBinaryDataToBoundary(4);

    Utf8String          nameId =  Concat(name, id),
                        accessorId = Concat ("acc", nameId),
                        bufferViewId = Concat ("bv", nameId);
    size_t              nValues = nComponents * nAttributes;
    size_t              dataSize = 0;
    size_t              byteOffset = tileData.BinaryDataSize();
    Json::Value         bufferViews = Json::objectValue;
    Json::Value         accessor   = Json::objectValue;


    switch (encoding)
        {
        case VertexEncoding::StandardQuantization:
            {
            double      range = (double) (0xffff);
        
            accessor["componentType"] = static_cast<int32_t>(Gltf::DataType::UnsignedShort);
                
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
            bvector<float> testVals;

            for (size_t i=0; i<nValues; i++)
                {
                size_t  componentIndex = i % nComponents;
                quantizedValues.push_back ((unsigned short) (.5 + (values[i] - min[componentIndex]) * range / (max[componentIndex] - min[componentIndex])));

                testVals.push_back( min[componentIndex] + (max[componentIndex] -  min[componentIndex]) * (double) quantizedValues.back() / range);
                }
            tileData.AddBinaryData (quantizedValues.data(), dataSize = nValues * sizeof (unsigned short));
            break;
            }
    
        case VertexEncoding::UnquantizedDoubles:
            {
            bvector <float>     floatValues;

            accessor["componentType"] = static_cast<int32_t>(Gltf::DataType::Float);

            for (size_t i=0; i<nValues; i++)
                floatValues.push_back ((float) values[i]);

            tileData.AddBinaryData (floatValues.data(), dataSize = nValues * sizeof (float));
            break;
            }

        case VertexEncoding::OctEncodedNormals:
            {
            bvector<uint16_t>   octEncodedNormals;
            DVec3dCP            normals = reinterpret_cast<DVec3dCP> (values);

            for (size_t i=0; i<nAttributes; i++)
                octEncodedNormals.push_back(octEncodeNormal(normals[i]));

            accessor["componentType"] = static_cast<int32_t>(Gltf::DataType::UnsignedByte);
            for (size_t i=0; i<3; i++)
                {
                accessor["min"].append (0);
                accessor["max"].append (255);
                }

            tileData.AddBinaryData (octEncodedNormals.data(), dataSize = nAttributes * sizeof (uint16_t));
            break;
            }
        }

    bufferViews["buffer"] = "binary_glTF";
    bufferViews["byteOffset"] = byteOffset;
    bufferViews["byteLength"] = dataSize;
    bufferViews["target"] = Gltf::ArrayBuffer;

    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = nAttributes;
    accessor["type"] = accessorType;

    tileData.m_json["bufferViews"][bufferViewId] = bufferViews;
    tileData.m_json["accessors"][accessorId] = accessor;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshUInt16Attributes(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& attributes16, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic)
    {
    Utf8String suffix(name);
    suffix.append(idStr);

    Utf8String bvId = Concat("bv", suffix);
    Utf8String accId = Concat("acc", suffix);

    primitive["attributes"][semantic] = accId;

    // Use uint8 if possible to save space in tiles and memory in browser
    bvector<uint8_t> attributes8;
    auto componentType = Gltf::DataType::UnsignedByte;
    for (auto attribute : attributes16)
        {
        if (attribute > 0xff)
            {
            componentType = Gltf::DataType::UnsignedShort;
            break;
            }
        }

    size_t nBytes = attributes16.size() * sizeof(uint16_t);
    if (Gltf::DataType::UnsignedByte == componentType)
        {
        attributes8.reserve(attributes16.size());
        for (auto attribute : attributes16)
            attributes8.push_back(static_cast<uint8_t>(attribute));

        nBytes /= 2;
        }

    auto& bv = tileData.m_json["bufferViews"][bvId];
    bv["buffer"] = "binary_glTF";
    bv["byteOffset"] = tileData.BinaryDataSize();
    bv["byteLength"] = nBytes;
    bv["target"] = Gltf::ArrayBuffer;

    if (Gltf::DataType::UnsignedByte == componentType)
        tileData.AddBinaryData(attributes8.data(), nBytes);
    else
        tileData.AddBinaryData(attributes16.data(), nBytes);

    auto& acc = tileData.m_json["accessors"][accId];
    acc["bufferView"] = bvId;
    acc["byteOffset"] = 0;
    acc["componentType"] = static_cast<int32_t>(componentType);
    acc["count"] = attributes16.size();
    acc["type"] = "SCALAR";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshUInt32Attributes(PublishTileData& tileData, Json::Value& primitive, bvector<uint32_t> const& attributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic)
    {
    Utf8String suffix(name);
    suffix.append(idStr);

    Utf8String bvId = Concat("bv", suffix);
    Utf8String accId = Concat("acc", suffix);

    primitive["attributes"][semantic] = accId;

    uint32_t      maxAttribute = 0;

    for (auto attribute : attributes)
        if (attribute > maxAttribute)
            maxAttribute = attribute;

    Gltf::DataType componentType;

    if (maxAttribute > 0xffff)
        componentType = Gltf::DataType::Float;
    else if (maxAttribute > 0xff)
        componentType = Gltf::DataType::UnsignedShort;
    else
        componentType = Gltf::DataType::UnsignedByte;

    bvector<uint8_t> attributes8;
    bvector<uint16_t> attributes16;
    bvector<uint32_t> attributesFloat;

    void*       pData = nullptr;
    size_t      dataSize = 0;

    switch (componentType)
        {
        case Gltf::DataType::Float:
            dataSize = sizeof(float);
            attributesFloat.reserve(attributes.size());
            for (auto attribute : attributes)
                attributesFloat.push_back(static_cast<float>(attribute));
            pData = attributesFloat.data();
            break;

        case Gltf::DataType::UnsignedShort:
            dataSize = sizeof(uint16_t);
            attributes16.reserve(attributes.size());
            for (auto attribute : attributes)
                attributes16.push_back(static_cast<uint16_t>(attribute));
            pData = attributes16.data();
            break;

        case Gltf::DataType::UnsignedByte:
            dataSize = sizeof(uint8_t);
            attributes8.reserve(attributes.size());
            for (auto attribute : attributes)
                attributes8.push_back(static_cast<uint8_t>(attribute));
            pData = attributes8.data();
            break;
        }


    auto&   bv = tileData.m_json["bufferViews"][bvId];
    size_t  nBytes = attributes.size() * dataSize;

    bv["buffer"] = "binary_glTF";
    bv["byteOffset"] = tileData.BinaryDataSize();
    bv["byteLength"] = nBytes;
    bv["target"] = Gltf::ArrayBuffer;
    tileData.AddBinaryData(pData, nBytes);

    auto& acc = tileData.m_json["accessors"][accId];
    acc["bufferView"] = bvId;
    acc["byteOffset"] = 0;
    acc["componentType"] = static_cast<int32_t>(componentType);
    acc["count"] = attributes.size();
    acc["type"] = "SCALAR";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshBatchIds (PublishTileData& tileData, Json::Value& primitive, bvector<uint32_t> const& batchIds, Utf8StringCR idStr)
    {
    AddMeshUInt32Attributes(tileData, primitive, batchIds, idStr, "Batch_", "_BATCHID");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshColors(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& colorIds, Utf8StringCR idStr)
    {
    AddMeshUInt16Attributes(tileData, primitive, colorIds, idStr, "ColorIndex_", "_COLORINDEX");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddMeshIndices(PublishTileData& tileData, Utf8CP name, bvector<uint32_t> const& indices, Utf8StringCR idStr)
    {
    Utf8String          nameId           = Concat(name, idStr),
                        accIndexId       = Concat("acc", nameId),
                        bvIndexId        = Concat("bv", nameId);
    bool                useShortIndices    = true;

    for (auto& index : indices)
        {
        if (index > 0xffff)
            {
            useShortIndices = false;
            break;
            }
        }
    tileData.m_json["bufferViews"][bvIndexId] = Json::objectValue;
    tileData.m_json["bufferViews"][bvIndexId]["buffer"] = "binary_glTF";
    tileData.m_json["bufferViews"][bvIndexId]["byteOffset"] = tileData.BinaryDataSize();
    tileData.m_json["bufferViews"][bvIndexId]["byteLength"] = indices.size() * (useShortIndices ? sizeof(uint16_t) : sizeof(uint32_t));
    tileData.m_json["bufferViews"][bvIndexId]["target"] =  Gltf::ElementArrayBuffer;

    if (useShortIndices)
        {
        bvector<uint16_t>   shortIndices;

        for (auto& index : indices)
            shortIndices.push_back ((uint16_t) index);

        tileData.AddBinaryData (shortIndices.data(), shortIndices.size()*sizeof(uint16_t));
        }
    else
        {
        tileData.AddBinaryData (indices.data(),  indices.size()*sizeof(uint32_t));
        }

    tileData.m_json["accessors"][accIndexId] = Json::objectValue;
    tileData.m_json["accessors"][accIndexId]["bufferView"] = bvIndexId;
    tileData.m_json["accessors"][accIndexId]["byteOffset"] = 0;
    tileData.m_json["accessors"][accIndexId]["componentType"] = static_cast<int32_t>(useShortIndices ? Gltf::DataType::UnsignedShort : Gltf::DataType::UInt32);
    tileData.m_json["accessors"][accIndexId]["count"] = indices.size();
    tileData.m_json["accessors"][accIndexId]["type"] = "SCALAR";

    return accIndexId;
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
void TilePublisher::AddMeshPrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds)
    {                                                                                                                                     
    if (mesh.Triangles().empty())
        return;

    Utf8String          idStr(std::to_string(index).c_str());
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

    if (doBatchIds)
        AddMeshBatchIds(tileData, primitive, mesh.Attributes(), idStr);

    DRange3d        pointRange = DRange3d::From(mesh.Points());

    MeshMaterial meshMat = AddMeshMaterial(tileData, mesh, idStr.c_str(), doBatchIds);
    primitive["material"] = meshMat.GetName();
    primitive["mode"] = static_cast<int32_t>(Gltf::PrimitiveType::Triangles);

    Utf8String      accPositionId =  AddMeshVertexAttributes (tileData, &mesh.Points().front().x, "Position", idStr.c_str(), 3, mesh.Points().size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
    primitive["attributes"]["POSITION"] = accPositionId;

    bool isTextured = meshMat.IsTextured();
    BeAssert (isTextured == !mesh.Params().empty());
    if (!mesh.Params().empty() && isTextured)
        {
        DRange3d        paramRange = DRange3d::From(mesh.Params(), 0.0);
        primitive["attributes"]["TEXCOORD_0"] = AddMeshVertexAttributes (tileData, &mesh.Params().front().x, "Param", idStr.c_str(), 2, mesh.Params().size(), "VEC2", VertexEncoding::StandardQuantization, &paramRange.low.x, &paramRange.high.x);
        }

    BeAssert(isTextured == mesh.Colors().empty());
    if (!mesh.Colors().empty() && !isTextured && TilePublish::ColorIndex::Dimension::Zero != meshMat.GetColorIndexDimension())
        AddMeshColors(tileData, primitive, mesh.Colors(), idStr);

    if (!mesh.Normals().empty() && !mesh.GetDisplayParams().GetIgnoreLighting())        // No normals if ignoring lighting (reality meshes).
        {
        primitive["attributes"]["NORMAL"] = AddMeshVertexAttributes (tileData, &mesh.Normals().front().x, "Normal", idStr.c_str(), 3, mesh.Normals().size(), "VEC2", VertexEncoding::OctEncodedNormals, nullptr, nullptr);
        }

    primitive["indices"] = AddMeshIndices (tileData, "Indices", indices, idStr);
    AddMeshPointRange(tileData.m_json["accessors"][accPositionId], pointRange);

    primitivesNode.append(primitive);
    tileData.m_json["buffers"]["binary_glTF"]["byteLength"] = tileData.BinaryDataSize();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     011/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds)
    {
    if (mesh.Polylines().empty())
        return;

    if (mesh.GetDisplayParams().GetRasterWidth() <= 1)
        AddSimplePolylinePrimitive(primitivesNode, tileData, mesh, index, doBatchIds);
    else
        AddTesselatedPolylinePrimitive(primitivesNode, tileData, mesh, index, doBatchIds);
    }    
  

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2017
+===============+===============+===============+===============+===============+======*/
struct  PolylineTesselation
    {
    bvector<DPoint3d>           m_points;
    bvector<DVec3d>             m_prevDirs;
    bvector<DVec3d>             m_nextDirs;
    bvector<double>             m_distances;
    bvector<DPoint2d>           m_params;
    bvector<DPoint3d>           m_scalePoints;
    bvector<uint32_t>           m_attributes;
    bvector<uint16_t>           m_colors;
    bvector<uint32_t>           m_indices;

    /* Delta contains the following parameters.
        x - the distance along the curve (used for parameterization of textures (0 - length).
        y - direction to project along perpendicular (-1 or 1).
        z - 0.0 == start, 4.0 == end.  (1-2) z-1 == start joint param. (5-6) z-5 == end joint params).  Joint parameter is the interpolation factor between perpendicular and bisector. */

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddPoint (DPoint3dCR point, DVec3dCR prev, DVec3dCR next, double delta, DVec2dCR param, uint32_t attrib, uint16_t color, DPoint3dCR center)
        {
        m_points.push_back(point);
        m_prevDirs.push_back(prev);
        m_nextDirs.push_back(next);
        m_distances.push_back(delta);
        m_params.push_back(param);
        m_attributes.push_back(attrib);
        m_colors.push_back(color);
        m_scalePoints.push_back(center);
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddTriangle(uint32_t baseIndex, uint32_t index0, uint32_t index1, uint32_t index2)
        {
        m_indices.push_back(baseIndex + index0);
        m_indices.push_back(baseIndex + index1);
        m_indices.push_back(baseIndex + index2);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     02/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddJointTriangles(uint32_t index, double length, DPoint3dCR p, DVec3dCR prev, DVec3dCR next, uint32_t attribute, uint16_t color, double param, DPoint3dCR center)
        {
        static size_t   s_nPoints = 4;
        double          paramDelta = 1.0 / (double) (s_nPoints - 1);
        
        for (size_t i=0; i < s_nPoints - 1; i++)
            AddTriangle(0, index, m_points.size() + i + 1, m_points.size() + i);

        for (size_t i=0; i < s_nPoints; i++)
            AddPoint(p, prev, next, length, DVec2d::From(1.0, param + (double) i * paramDelta), attribute, color, center); 
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     011/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void gatherPolyline(bvector<DPoint3d>& polylinePoints,  bvector<uint16_t>&  polylineColors, bvector<uint32_t>& polylineAttributes, TilePolylineCR polyline, TileMeshCR mesh, bool doColors, bool doBatchIds)
    {
    static double               s_degenerateSegmentTolerance = 1.0E-5;

    for (auto& index : polyline.m_indices)        // Gather polyline points, omitting degneeraet segments.
        {
        DPoint3d    thisPoint = mesh.Points().at(index);

        if (polylinePoints.empty() || thisPoint.Distance(polylinePoints.back()) > s_degenerateSegmentTolerance)
            {
            polylinePoints.push_back(thisPoint);
            if (doColors)
                polylineColors.push_back(mesh.Colors().at(index));

            if (doBatchIds)
                polylineAttributes.push_back(mesh.Attributes().at(index));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     011/2016
*
*  6 points are generated for each line segment and these are used to generate 4 triangles.
*  The triangles on the interior of a joint are truncated at the miter line - else
*  we would be able to use only two triangles and 4 points.  (See PolylineTesselation.dgn)
*  At each point we include the point location on at the segment center line as well as
*  the previous and next points.   The previous and next points are used to calculate the 
*  the miter direction.  For a start or end point the previous and next directions are 
*  along the line segment and the miter direction is therefore perpendicular to the line segment.
*  Param.x is the direction to offset along the miter line -- (0, -1 or 1).   
*  Param.y is an inelegant horrible conglomeration --
*       0 indicates the start point with miter, 1.0 = start point with joint, 
*       4 indicates end point with miter, 5 indicates end point with joint
*
*  The joint geometry fills the void between the two miter triangles.  Currently we are just
*  Creating two triangles -- but we could create a single triangle (as QVision does) or additional
*  triangles to make a more rounded joint.
*    param.y values from 2-3 indicate start joint points, 6-7 indicate end joints. these are calculated as a linear combination of 
*    the segment perpendicular and the miter direction.
*
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddTesselatedPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds)
    {
    Utf8String idStr(std::to_string(index).c_str());

    static double               s_degenerateSegmentTolerance = 1.0E-5;
    BeAssert (mesh.Triangles().empty());        // Meshes should contain either triangles or polylines but not both.

    PolylineMaterial            mat = AddTesselatedPolylineMaterial(tileData, mesh, idStr.c_str(), doBatchIds);
    PolylineTesselation         tesselation;
    bool                        doColors = TilePublish::ColorIndex::Dimension::Zero != mat.GetColorIndexDimension();
    double                      minLength = 0.0, maxLength = 0.0;

    for (auto const& polyline : mesh.Polylines())
        {
        bvector<DPoint3d>       polylinePoints;
        bvector<uint16_t>       polylineColors;
        bvector<uint32_t>       polylineAttributes;

        gatherPolyline (polylinePoints, polylineColors, polylineAttributes, polyline, mesh, doColors, doBatchIds);

        if (polylinePoints.size() < 2)
            continue;

        DRange3d        polylineRange = DRange3d::From(polylinePoints);
        DPoint3d        rangeCenter = DPoint3d::FromInterpolate(polylineRange.low, .5, polylineRange.high);
        double          cumulativeLength = 0.0;
        bool            isClosed = polylinePoints.front().IsEqual(polylinePoints.back());
        
        for (size_t i=0, last = polylinePoints.size()-1; i<last; i++)
            {
            static              double s_maxJointDot = -.90;
            DPoint3d            p0 = polylinePoints[i], p1 = polylinePoints[i+1];
            double              thisLength = p0.Distance(p1), 
                                length0 = cumulativeLength, 
                                length1 = (cumulativeLength += thisLength);
            bool                isStart  = (i == 0),
                                isEnd    = (i == last - 1);
            uint16_t            colors0 = 0, colors1 = 0;
            uint32_t            attributes0 = 0, attributes1 = 1;
            static double       s_extendDistance = .1;
            DVec3d              thisDir = DVec3d::FromStartEndNormalize(p0, p1), negatedThisDir = DVec3d::FromScale(thisDir, -1.0),
                                prevDir0 = isStart ? (isClosed ?  DVec3d::FromStartEndNormalize(p0,  polylinePoints[last-1]) : negatedThisDir) : DVec3d::FromStartEndNormalize(p0,  polylinePoints[i-1]),
                                nextDir0 = thisDir,
                                prevDir1 = negatedThisDir,
                                nextDir1 = isEnd ? (isClosed ? DVec3d::FromStartEndNormalize(p1, polylinePoints[1]) : thisDir) : DVec3d::FromStartEndNormalize(p1, polylinePoints[i+2]);
            size_t              baseIndex = tesselation.m_points.size();
            bool                jointAt0 = prevDir0.DotProduct(nextDir0) > s_maxJointDot, jointAt1 = prevDir1.DotProduct(nextDir1) > s_maxJointDot;

                        
            if (doColors)
                {
                colors0 = polylineColors[i];
                colors1 = polylineColors[i+1];
                }

            if (doBatchIds)
                {
                attributes0 = polylineAttributes[i];
                attributes1 = polylineAttributes[i+1];
                }

            if (jointAt0 || jointAt1)
                {
                tesselation.AddTriangle(baseIndex, 2, 4, 1);
                tesselation.AddTriangle(baseIndex, 2, 1, 0);
                tesselation.AddTriangle(baseIndex, 0, 1, 5);
                tesselation.AddTriangle(baseIndex, 0, 5, 3);

                for (size_t j=0; j<6; j++)
                    {
                    static bool     basePoints[6]  = {true, false, true, true, false, false};
                    static double   deltaYs[6] = {0.0, 0.0, -1.0, 1.0, -1.0, 1.0 };
                    bool            basePoint = basePoints[j];
                
                    tesselation.AddPoint (basePoint ? p0 : p1,
                                          basePoint ? prevDir0 : prevDir1,
                                          basePoint ? nextDir0 : nextDir1,
                                          basePoint ? length0 : length1,
                                          DVec2d::From(deltaYs[j], basePoint ? (jointAt0 ? 1.0 : 0.0) : (jointAt1 ? 5.01 : 4.01)),
                                          basePoint ? attributes0 : attributes1,
                                          basePoint ? colors0 : colors1,
                                          rangeCenter);
                    }
                if (jointAt0)
                    tesselation.AddJointTriangles(baseIndex, length0, p0, prevDir0, nextDir0, attributes0, colors0, 2.0, rangeCenter);

                if (jointAt1)
                    tesselation.AddJointTriangles(baseIndex+1, length1, p1, prevDir1, nextDir1, attributes1, colors1, 6.0, rangeCenter);
                }
            else
                {
                tesselation.AddTriangle(baseIndex, 0, 2, 1);
                tesselation.AddTriangle(baseIndex, 1, 2, 3);

                for (size_t j=0; j<4; j++)
                    {
                    bool        basePoint = j<2;
                    double      deltaY = (0 == (j & 0x01)) ? -1.0 : 1.0;
                    tesselation.AddPoint (basePoint ? p0 : p1,
                                          basePoint ? prevDir0 : prevDir1,
                                          basePoint ? nextDir0 : nextDir1,
                                          basePoint ? length0 : length1,
                                          DVec2d::From(deltaY, basePoint ? 0.0 : 4.001),
                                          basePoint ? attributes0 : attributes1,
                                          basePoint ? colors0 : colors1,
                                          rangeCenter);
                    }
                }
            }
        maxLength = std::max(maxLength, cumulativeLength);
        }
    if (tesselation.m_points.empty())
        return;


    Json::Value     primitive = Json::objectValue;
    DRange3d        pointRange = DRange3d::From(tesselation.m_points), paramRange = DRange3d::From(tesselation.m_params, 0.0);

    primitive["material"] = mat.GetName();
    primitive["mode"] = static_cast<int32_t>(Gltf::PrimitiveType::Triangles);

    Utf8String  accPositionId = AddMeshVertexAttributes (tileData, &tesselation.m_points.front().x, "Position", idStr.c_str(), 3, tesselation.m_points.size(), "VEC3",  VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
    primitive["attributes"]["POSITION"]  = accPositionId;
    primitive["attributes"]["PREV"] = AddMeshVertexAttributes (tileData, &tesselation.m_prevDirs.front().x, "Prev", idStr.c_str(), 3, tesselation.m_prevDirs.size(), "VEC2",  VertexEncoding::OctEncodedNormals, nullptr, nullptr);
    primitive["attributes"]["NEXT"] = AddMeshVertexAttributes (tileData, &tesselation.m_nextDirs.front().x, "Next", idStr.c_str(), 3, tesselation.m_nextDirs.size(), "VEC2",  VertexEncoding::OctEncodedNormals, nullptr, nullptr);
    primitive["attributes"]["PARAM"]  = AddMeshVertexAttributes (tileData, &tesselation.m_params.front().x, "Param", idStr.c_str(), 2, tesselation.m_params.size(), "VEC2", VertexEncoding::StandardQuantization, &paramRange.low.x, &paramRange.high.x);
    primitive["indices"] = AddMeshIndices (tileData, "Index", tesselation.m_indices, idStr);

    if (mat.IsTextured())
        {
        primitive["attributes"]["DISTANCE"]  = AddMeshVertexAttributes (tileData, &tesselation.m_distances.front(), "Distance", idStr.c_str(), 1, tesselation.m_distances.size(), "SCALAR", VertexEncoding::StandardQuantization, &minLength, &maxLength);
        primitive["attributes"]["TEXSCALEPNT"]  = AddMeshVertexAttributes (tileData, &tesselation.m_scalePoints.front().x, "TexScalePnt", idStr.c_str(), 3, tesselation.m_scalePoints.size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
        }


    if (doBatchIds)
        AddMeshBatchIds(tileData, primitive, tesselation.m_attributes, idStr);

    if (doColors)
        AddMeshColors(tileData, primitive, tesselation.m_colors, idStr);

    AddMeshPointRange(tileData.m_json["accessors"][accPositionId], pointRange);

    primitivesNode.append(primitive);
    tileData.m_json["buffers"]["binary_glTF"]["byteLength"] = tileData.BinaryDataSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     011/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddSimplePolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds)
    {
    if (mesh.Polylines().empty())
        return;

    Utf8String idStr(std::to_string(index).c_str());
    PolylineMaterial            mat = AddSimplePolylineMaterial(tileData, mesh, idStr.c_str(), doBatchIds);
    bvector<DPoint3d>           points, scalePoints;
    bvector<uint16_t>           colors;
    bvector<uint32_t>           attributes, indices;
    bvector<double>             distances;
    double                      minLength = 0.0, maxLength = 0.0;
    bool                        doColors = TilePublish::ColorIndex::Dimension::Zero != mat.GetColorIndexDimension();

    for (auto const& polyline : mesh.Polylines())
        {
        bvector<DPoint3d>       polylinePoints;
        bvector<uint16_t>       polylineColors;
        bvector<uint32_t>       polylineAttributes;

        gatherPolyline (polylinePoints, polylineColors, polylineAttributes, polyline, mesh, doColors, doBatchIds);

        DRange3d        polylineRange = DRange3d::From(polylinePoints);
        DPoint3d        rangeCenter = DPoint3d::FromInterpolate(polylineRange.low, .5, polylineRange.high);
        double          cumulativeLength = 0.0;
        size_t          baseIndex = points.size();

        for (size_t i=0; i<polylinePoints.size(); i++)
            {
            DPoint3d            p0 = polylinePoints[i];

            if (i > 0)
                {
                cumulativeLength += p0.Distance(polylinePoints[i-1]);
                indices.push_back(baseIndex + i - 1);
                indices.push_back(baseIndex + i);
                }

            points.push_back (p0);
            if (doBatchIds)
                attributes.push_back(polylineAttributes[i]);

            if (doColors)
                colors.push_back(polylineColors[i]);

            distances.push_back(cumulativeLength);
            scalePoints.push_back(rangeCenter);
            }
        maxLength = std::max(maxLength, cumulativeLength);
        }

    if (indices.empty())
        return;

    Json::Value     primitive = Json::objectValue;
    DRange3d        pointRange = DRange3d::From(points);

    primitive["material"] = mat.GetName();
    primitive["mode"] = static_cast<int32_t>(Gltf::PrimitiveType::Lines);

    Utf8String  accPositionId = AddMeshVertexAttributes (tileData, &points.front().x, "Position", idStr.c_str(), 3, points.size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
    primitive["attributes"]["POSITION"]  = accPositionId;
    primitive["indices"] = AddMeshIndices (tileData, "Index", indices, idStr);

    if (mat.IsTextured())
        {
        primitive["attributes"]["DISTANCE"]  = AddMeshVertexAttributes (tileData, &distances.front(), "Distance", idStr.c_str(), 1, distances.size(), "SCALAR", VertexEncoding::StandardQuantization, &minLength, &maxLength);
        primitive["attributes"]["TEXSCALEPNT"]  = AddMeshVertexAttributes (tileData, &scalePoints.front().x, "TexScalePnt", idStr.c_str(), 3, scalePoints.size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
        }

    if (doBatchIds)
        AddMeshBatchIds(tileData, primitive, attributes, idStr);

    if (doColors)
        AddMeshColors(tileData, primitive, colors, idStr);

    AddMeshPointRange(tileData.m_json["accessors"][accPositionId], pointRange);

    primitivesNode.append(primitive);
    tileData.m_json["buffers"]["binary_glTF"]["byteLength"] = tileData.BinaryDataSize();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherContext::IsGeolocated () const
    {
    return m_isGeoLocated;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::PublisherContext(DgnDbR db, DgnViewIdSet const& viewIds, BeFileNameCR outputDir, WStringCR tilesetName,  AxisAlignedBox3dCR projectExtents, GeoPointCP geoLocation, bool publishSurfacesOnly, size_t maxTilesetDepth, TextureMode textureMode, GlobeMode globeMode)
    : m_db(db), m_viewIds(viewIds), m_outputDir(outputDir), m_rootName(tilesetName), m_projectExtents(projectExtents), m_publishSurfacesOnly (publishSurfacesOnly), m_maxTilesetDepth (maxTilesetDepth), m_textureMode(textureMode), m_generationFilter(nullptr), m_currentClassifier(nullptr), m_globeMode(globeMode)
    {
        {
        // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
        m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();
        }

    // By default, output dir == data dir. data dir is where we put the json/b3dm files.
    m_outputDir.AppendSeparator();
    m_dataDir = m_outputDir;
    m_isEcef = false;

    // ###TODO: Probably want a separate db-to-tile per model...will differ for non-spatial models...
    DPoint3d        origin = m_projectExtents.GetCenter();
    if (m_isEcef)
        m_dbToTile.InitIdentity();
    else
        m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);

    DgnGCS*         dgnGCS = db.GeoLocation().GetDgnGCS();
    DPoint3d        ecfOrigin, ecfNorth;

    // Some user might want to override the DgnGCS by specifying a geolocation on cmd line...
    m_isGeoLocated = nullptr != dgnGCS || nullptr != geoLocation;


    // Create a WGS84 GCS to convert the WGS84 Lat/Long to ECEF/XYZ
    WString     warningMsg;
    StatusInt   warning;
    auto        wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
    GeoPoint    originLatLong;

    wgs84GCS ->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary


    if (nullptr == dgnGCS || nullptr != geoLocation)
        {
        if (nullptr != geoLocation)
            {
            originLatLong = *geoLocation;
            }
        else
            {
            // NB: We have to translate to surface of globe even if we're not using the globe, because
            // Cesium's camera freaks out if it approaches the origin (aka the center of the earth)

            originLatLong.longitude = -75.686844444444444444444444444444;
            originLatLong.latitude = 40.065702777777777777777777777778;        // Arbitrary location (Exton flapole).
            }
        }
    else
        {
        dgnGCS->LatLongFromUors (originLatLong, origin);

        // If the current GCS does not use WGS84, need to convert as XYZFromLatLong expects WGS84 Lat/Long... (TFS# 799148). 
        if (0 != wcscmp (dgnGCS->GetDatumName(), L"WGS84"))
            {
            auto        wgs84Datum = GeoCoordinates::Datum::CreateDatum (L"WGS84");
            auto        thisDatum = GeoCoordinates::Datum::CreateDatum(dgnGCS->GetDatumName());
            auto        datumConverter = GeoCoordinates::DatumConverter::Create (*thisDatum, *wgs84Datum);

            if (nullptr != datumConverter)
                {
                datumConverter->ConvertLatLong3D(originLatLong, originLatLong);

                // Eww?
                datumConverter->Destroy();
                datumConverter = nullptr;
                }
            }
        }
    GeoPoint    northLatLong = originLatLong;

    northLatLong.latitude += 3.0 / (360.0 * 60.0 * 60.0);   // To get a point north of origin move 3 minutes of latitude (per Alain Robert)                  


    wgs84GCS->XYZFromLatLong(ecfOrigin, originLatLong);
    wgs84GCS->XYZFromLatLong(ecfNorth, northLatLong);

    RotMatrix   rMatrix;
    rMatrix.InitIdentity();

    DVec3d      zVector, yVector;

    zVector.Normalize ((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference (ecfNorth, ecfOrigin);

    rMatrix.SetColumn (yVector, 1);
    rMatrix.SetColumn (zVector, 2);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

    if (m_isEcef)
        m_spatialToEcef.InitIdentity(); // ###TODO: ecfNorth...
    else
        m_spatialToEcef =  Transform::From (rMatrix, ecfOrigin);
    }

#include <Bentley\BeDirectoryIterator.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileNameStatus clearNonHistoryDirectories(BeFileNameCR dataDir)
    {
    WString path2;
    BeFileNameStatus stat = BeFileName::FixPathName(path2, dataDir, false);
    if (BeFileNameStatus::Success != stat)
        return  stat;

    BeFileName path(path2.c_str());

    if (!BeFileName::DoesPathExist(path))
        return  BeFileNameStatus::FileNotFound;

    BeFileName filename;
    bool       isDir;
    for (BeDirectoryIterator dir(path); dir.GetCurrentEntry(filename, isDir, true) == SUCCESS; dir.ToNext())
        {
        if (isDir)
            {
            WString     name;

            if (filename.GetBaseName() != L"History")
                {
                stat = BeFileName::EmptyAndRemoveDirectory(filename);
                if (BeFileNameStatus::Success != stat)
                    return  stat;
                }
            }
        else
            {
            BeFileName::SetFileReadOnly(filename, false);
            stat = BeFileName::BeDeleteFile(filename);
            if (stat != BeFileNameStatus::Success)
                return  stat;
            }
        }

    return BeFileNameStatus::Success;
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

    if (dataDirExists && BeFileNameStatus::Success != clearNonHistoryDirectories(dataDir))
        {
        return Status::CantCreateSubDirectory;
        }
    else if (!dataDirExists && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dataDir))
        {
        return Status::CantCreateSubDirectory;
        }

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
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetViewAttachmentsJson(Sheet::ModelCR sheet, DgnModelIdSet& attachedModels)
    {
    bvector<DgnElementId> attachmentIds = sheet.GetSheetAttachmentIds();
    Json::Value attachmentsJson(Json::arrayValue);
    for (DgnElementId attachmentId : attachmentIds)
        {
        auto attachment = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachmentId);
        ViewDefinitionCPtr view = attachment.IsValid() ? GetDgnDb().Elements().Get<ViewDefinition>(attachment->GetAttachedViewId()) : nullptr;
        if (view.IsNull())
            continue;

        // Handle wacky 'spatial' views created for 2d models by DgnV8Converter...
        DgnModelIdSet viewedModels;
        GetViewedModelsFromView(viewedModels, attachment->GetAttachedViewId());
        BeAssert(1 == viewedModels.size() || view->IsSpatialView());
        if (viewedModels.empty())
            continue;

        // ###TODO: Spatial view attachments with multiple models unhandled...
        DgnModelId baseModelId = *viewedModels.begin();
        attachedModels.insert(baseModelId);

        Json::Value viewJson;
        viewJson["baseModelId"] = baseModelId.ToString();
        viewJson["categorySelector"] = view->GetCategorySelectorId().ToString();
        viewJson["displayStyle"] = view->GetDisplayStyleId().ToString();

        DPoint3d            viewOrigin = view->GetOrigin();
        AxisAlignedBox3d    sheetRange = attachment->GetPlacement().CalculateRange();
        double              sheetScale = sheetRange.XLength() / view->GetExtents().x;

        // HACK: For some reason, attachments to drawings of 3D models, and attachments with any scaling factor, end up as 'spatial' views of 'spatial' models in the converter.
        // Revert the ECEF transform we apply to such models so they will render on the sheet, which is not itself transformed.
        Transform ecefToSheet;
        if (view->IsSpatialView() && !m_spatialToEcef.IsIdentity())
            ecefToSheet.InverseOf(m_spatialToEcef);
        else
            ecefToSheet.InitIdentity();

        Transform           subtractViewOrigin = Transform::From(DPoint3d::From(-viewOrigin.x, -viewOrigin.y, -viewOrigin.z)),
                            viewRotation = Transform::From(view->GetRotation()),
                            scaleToSheet = Transform::FromScaleFactors (sheetScale, sheetScale, sheetScale),
                            addSheetOrigin = Transform::FromProduct(ecefToSheet, Transform::From(DPoint3d::From(sheetRange.low.x, sheetRange.low.y, attachment->GetDisplayPriority()/500.0))),
                            tileToSheet = Transform::FromProduct(Transform::FromProduct(addSheetOrigin, scaleToSheet), Transform::FromProduct(viewRotation, subtractViewOrigin));

        viewJson["transform"] = TransformToJson(tileToSheet);

        attachmentsJson.append(std::move(viewJson));
        }

    return attachmentsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteModelMetadataTree (DRange3dR range, Json::Value& root, TileNodeCR tile, size_t depth)
    {
    if (tile.GetIsEmpty() && tile.GetChildren().empty())
        {
        range = DRange3d::NullRange();
        return;
        }

    WString         rootName = GetRootName (tile.GetModel().GetModelId(), GetCurrentClassifier());
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
        if (0 == --depth)
            {
            // Write children as seperate tilesets.
            for (auto& childTile : tile.GetChildren())
                {
                Json::Value         childTileset;
                DRange3d            childRange;

                childTileset["asset"]["version"] = "0.0";
                childTileset["asset"]["gltfUpAxis"] = "Z";

                auto&       childRoot = childTileset[JSON_Root];
                WString     metadataRelativePath = childTile->GetFileName(rootName.c_str(), s_metadataExtension);
                BeFileName  dataDirectory = GetModelDataDirectory(tile.GetModel().GetModelId(), GetCurrentClassifier());
                BeFileName  metadataFileName (nullptr, dataDirectory.c_str(), metadataRelativePath.c_str(), nullptr);

                WriteModelMetadataTree (childRange, childRoot, *childTile, GetMaxTilesetDepth());
                if (!childRange.IsNull())
                    {
                    TileUtil::WriteJsonToFile (metadataFileName.c_str(), childTileset);

                    Json::Value         child;

                    child["refine"] = "REPLACE";
                    child[JSON_GeometricError] = childTile->GetTolerance();
                    TilePublisher::WriteBoundingVolume(child, childRange);

                    child[JSON_Content]["url"] = Utf8String (metadataRelativePath.c_str()).c_str();

                    Json::Value tileCustomMetadata;
                    Utf8String metadataName;
                    childTile->GetTileCustomMetadata(metadataName, tileCustomMetadata);
                    if (!tileCustomMetadata.isNull())
                        child[metadataName.c_str()] = tileCustomMetadata;

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

                WriteModelMetadataTree (childRange, child, *childTile, depth);
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

    if (!contentRange.IsNull() && !tile.GetIsEmpty())
        {
        root[JSON_Content]["url"] = Utf8String(GetTileUrl(tile, GetTileExtension(tile).c_str(), GetCurrentClassifier()));
        TilePublisher::WriteBoundingVolume (root[JSON_Content], contentRange);
        }

    Json::Value tileCustomMetadata;
    Utf8String metadataName;
    tile.GetTileCustomMetadata(metadataName, tileCustomMetadata);
    if (!tileCustomMetadata.isNull())
        root[metadataName.c_str()] = tileCustomMetadata;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::TransformToJson(TransformCR tf)
    {
    auto matrix = DMatrix4d::From(tf);
    Json::Value json(Json::arrayValue);
    for (size_t i=0;i<4; i++)
        for (size_t j=0; j<4; j++)
            json.append (matrix.coff[j][i]);


    return json;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteTileset (BeFileNameCR metadataFileName, TileNodeCR rootTile, size_t maxDepth)
    {
    Json::Value val, modelRoot;

    val["asset"]["version"] = "0.0";
    val["asset"]["gltfUpAxis"] = "Z";
 
    DRange3d    rootRange;
    WriteModelMetadataTree (rootRange, modelRoot, rootTile, maxDepth);

    // The modelRanges come from initial tile ranges which may be slopply - particularly
    // when coming from reality models.   Reset them here to the actual published ranges
    // which are generally more accurate (improves classifier fit (Berkeley Campus)).
    if (!rootRange.IsNull())
        m_modelRanges[rootTile.GetModel().GetModelId()] = ModelRange(rootRange, false);


    val[JSON_Root] = std::move(modelRoot);

    if (rootTile.GetModel().IsSpatialModel())
        val[JSON_Root][JSON_Transform] = TransformToJson(m_spatialToEcef);

    Utf8String name;
    Json::Value modelCustomMetadata;
    ExtractModelCustomPublishMetadata(rootTile.GetModel(), name, modelCustomMetadata);
    if (!modelCustomMetadata.isNull())
        val[JSON_Root][name.c_str()] = modelCustomMetadata;

    TileUtil::WriteJsonToFile (metadataFileName.c_str(), val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::ExtractModelCustomPublishMetadata(DgnModelCR model, Utf8StringR name, Json::Value& metadata) const
    {
    auto const& publishProperties = model.GetJsonProperties("publishing");
    if (!publishProperties.isNull())
        {
        name = publishProperties["name"].asString();
        metadata = publishProperties["properties"];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PublisherContext::_BeginProcessModel(DgnModelCR model)
    {
    return Status::Success == InitializeDirectories(GetModelDataDirectory(model.GetModelId(), GetCurrentClassifier(), false)) ? TileGeneratorStatus::Success : TileGeneratorStatus::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PublisherContext::_EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status)
    {
    if (TileGeneratorStatus::Success == status && nullptr != rootTile && (!rootTile->GetIsEmpty() || !rootTile->GetChildren().empty()))
        {
            {
            BeMutexHolder lock(m_mutex);
            m_modelRanges[model.GetModelId()] = ModelRange(rootTile->GetTileRange(), false);
            }

        WriteModelTileset(*rootTile);
        }
    else
        {
        CleanDirectories(GetModelDataDirectory(model.GetModelId(), GetCurrentClassifier(), false));
        }

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteModelTileset(TileNodeCR tile)
    {
    WriteTileset(GetTilesetFileName(tile.GetModel().GetModelId(), GetCurrentClassifier()), tile, GetMaxTilesetDepth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName  PublisherContext::GetModelDataDirectory(DgnModelId modelId, ClassifierInfo const* classifier, bool appendSeperator) const
    {
    BeFileName  modelDir = m_dataDir;

    modelDir.AppendToPath( GetRootName(modelId, classifier).c_str());
    if (appendSeperator)
        modelDir.AppendSeparator();

    return modelDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    PublisherContext::GetViewedModelsFromView (DgnModelIdSet& viewedModels, DgnViewId viewId, bool includeViewAttachments)
    {
    SpatialViewDefinitionPtr spatialView = nullptr;
    auto view2d = GetDgnDb().Elements().Get<ViewDefinition2d>(viewId);
    if (view2d.IsValid())
        {
        viewedModels.insert(view2d->GetBaseModelId());
        auto sheet = includeViewAttachments ? view2d->ToSheetView() : nullptr;
        if (nullptr != sheet)
            {
            auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
            stmt->BindId(1, view2d->GetBaseModelId());
            while (BE_SQLITE_ROW == stmt->Step())
                {
                auto attachId = stmt->GetValueId<DgnElementId>(0);
                auto attach = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachId);
                if (attach.IsValid())
                    GetViewedModelsFromView(viewedModels, attach->GetAttachedViewId(), false); // view attachments don't nest...
                }
            }
        }
    else if ((spatialView = GetDgnDb().Elements().GetForEdit<SpatialViewDefinition>(viewId)).IsValid())
        {
        for (auto& modelId : spatialView->GetModelSelector().GetModels())
            viewedModels.insert(modelId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status   PublisherContext::PublishViewModels (TileGeneratorR generator, DRange3dR rootRange, double toleranceInMeters, bool surfacesOnly, ITileGenerationProgressMonitorR progressMeter)
    {
    DgnModelIdSet viewedModels;

#if defined(WIP_PUBLISH_VIEW_ATTACHMENTS)
    bool includeAttachments = true;
#else
    bool includeAttachments = false;
#endif

    for (auto const& viewId : m_viewIds)
        GetViewedModelsFromView (viewedModels, viewId, includeAttachments);

    auto status = generator.GenerateTiles(*this, viewedModels, toleranceInMeters, surfacesOnly, s_maxPointsPerTile);
    if (TileGeneratorStatus::Success != status)
        return ConvertStatus(status);

    rootRange = DRange3d::NullRange();
    for (auto const& kvp : m_modelRanges)
        {
        auto model = GetDgnDb().Models().GetModel(kvp.first);

        if (model->IsSpatialModel())
            rootRange.Extend(kvp.second.m_range);
        }
    return PublishClassifiers(viewedModels, generator, toleranceInMeters, progressMeter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status   PublisherContext::PublishClassifiers (DgnModelIdSet const& viewedModels, TileGeneratorR generator, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter)
    {
    uint32_t    index = 0;
    Status      status = Status::Success;

    for (auto& modelId : viewedModels)
        {
        auto const&                 foundRange = m_modelRanges.find(modelId);

        if (foundRange == m_modelRanges.end())
            continue;

        
        auto                        model = GetDgnDb().Models().GetModel(modelId).get();
        ModelSpatialClassifiers     classifiers;

        if (nullptr != model && 
            nullptr != model->ToSpatialModel() &&
            SUCCESS == model->ToSpatialModel()->GetSpatialClassifiers(classifiers) &&
            !classifiers.empty())
            {
            T_ClassifierInfos       classifierInfos;

            for (auto& classifier : classifiers)
                {
                Status                  thisStatus;
                ClassifierInfo          classifierInfo(classifier, foundRange->second, index++);

                if (Status::Success != (thisStatus = PublishClassifier (classifierInfo,  generator, toleranceInMeters, progressMeter)))
                    status = thisStatus;

                classifierInfos.push_back(classifierInfo);
                }
            m_classifierMap[modelId] = classifierInfos;
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WString PublisherContext::GetRootName (DgnModelId modelId, ClassifierInfo const* classifier) const
    {
    return (nullptr == classifier) ? TileUtil::GetRootNameForModel(modelId, false) : classifier->GetRootName();
    }

//=======================================================================================
// @bsistruct                                                   Ray>Bentley     08/2017
//=======================================================================================
struct ClassifierFilter : ITileGenerationFilter
{

    DgnDbR                  m_db;
    ModelSpatialClassifier  m_classifier;

    ClassifierFilter(ModelSpatialClassifier const& classifier, DgnDbR db) : m_classifier(classifier), m_db(db) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _AcceptElement(DgnElementId elementId, TileDisplayParamsCR displayParams) const override
    {
    switch (m_classifier.GetType())
        {
        case ModelSpatialClassifier::TYPE_Model:
            return true;

        case ModelSpatialClassifier::TYPE_Category:
            return displayParams.GetCategoryId() == m_classifier.GetCategoryId();
        
        default:
            BeAssert(false);
            return true;
        }
    }
    

};  // ClasssifierFilter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status   PublisherContext::PublishClassifier(ClassifierInfo& classifierInfo, TileGeneratorR generator, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter)
    {
    ClassifierFilter                    classifierFilter(classifierInfo.m_classifier, GetDgnDb());
    AutoRestore<ITileGenerationFilter*> saveGenerationFilter(&m_generationFilter, &classifierFilter);
    AutoRestore<ClassifierInfo*>        saveCurrentClassifier(&m_currentClassifier, &classifierInfo);
    DgnModelIdSet                       singleClassifierModelIdSet;

    singleClassifierModelIdSet.insert(classifierInfo.m_classifier.GetModelId());
    
    return ConvertStatus(generator.GenerateTiles(*this, singleClassifierModelIdSet, toleranceInMeters, false, s_maxPointsPerTile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName PublisherContext::GetTilesetFileName(DgnModelId modelId, ClassifierInfo const* classifier)
    {
    WString     rootName = GetRootName(modelId, classifier);
    BeFileName  modelDir = GetModelDataDirectory(modelId, classifier);

    return BeFileName(nullptr, modelDir.c_str(), rootName.c_str(), s_metadataExtension);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  PublisherContext::GetTilesetURL(DgnModelId modelId, ClassifierInfo const* classifier)
    {
    if (nullptr == classifier)
        {
        auto urlIter = m_directUrls.find(modelId);
        if (m_directUrls.end() != urlIter)
            return urlIter->second;
        }
    BeFileName      tilesetFilePath = GetTilesetFileName(modelId, classifier);
    auto            utf8FileName = tilesetFilePath.GetNameUtf8();
    auto            url = Utf8String(utf8FileName.c_str() + m_outputDir.size());
    
    url.ReplaceAll("\\", "//");
    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetModelsJson (DgnModelIdSet const& modelIds)
    {
    Json::Value modelsJson(Json::objectValue);
    for (auto const& modelId : modelIds)
        AddModelJson(modelsJson, modelId, modelIds);

    return modelsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::AddModelJson(Json::Value& modelsJson, DgnModelId modelId, DgnModelIdSet const& modelIds)
    {
    auto const&  model = GetDgnDb().Models().GetModel (modelId);
    if (model.IsValid())
        {
        auto spatialModel = model->ToSpatialModel();
        auto model2d = nullptr == spatialModel ? dynamic_cast<GraphicalModel2dCP>(model.get()) : nullptr;
        if (nullptr == spatialModel && nullptr == model2d)
            {
            BeAssert(false && "Unsupported model type");
            return;
            }

        auto modelRangeIter = m_modelRanges.find(modelId);
        if (m_modelRanges.end() == modelRangeIter)
            return; // this model produced no tiles. ignore it.

        ModelRange modelRange = modelRangeIter->second;
        if (modelRange.m_range.IsNull())
            {
            BeAssert(false && "Null model range");
            return;
            }

        Json::Value modelJson(Json::objectValue);

        auto        sheetModel = model->ToSheetModel();

        // The reality models (Point Clouds and Reality meshes) do not contain elements and therefore
        // no categories etc.   They unfortunately do not have their own base class and therefore no
        // good way to detect - except that they do not extend physical model.
        bool        isRealityModel = nullptr != spatialModel && nullptr == model->ToPhysicalModel();

        modelJson["name"] = model->GetName();
        modelJson["type"] = nullptr != spatialModel ? (isRealityModel ? "reality" : "spatial") : (nullptr != sheetModel ? "sheet" : "drawing");

        if (nullptr != spatialModel)
            {
            if (modelRange.m_isEcef)
                {
                modelJson["transform"] = TransformToJson(Transform::FromIdentity());
                }
            else
                {
                m_spatialToEcef.Multiply(modelRange.m_range, modelRange.m_range);
                modelJson["transform"] = TransformToJson(m_spatialToEcef);
                }
            }
        else if (nullptr != sheetModel)
            {
            DgnModelIdSet attachedModels;
            modelJson["attachedViews"] = GetViewAttachmentsJson(*sheetModel, attachedModels);

            // Ensure all attached models are included in the models array
            // NB: No in-place version of std::set_difference...
            for (auto const& attachedModelId : attachedModels)
                if (modelIds.end() == modelIds.find(attachedModelId))
                    AddModelJson(modelsJson, attachedModelId, modelIds);
            }

        modelJson["extents"] = RangeToJson(modelRange.m_range);
        modelJson["tilesetUrl"] = GetTilesetURL(modelId, false);

        modelsJson[modelId.ToString()] = modelJson;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetCategoriesJson (DgnCategoryIdSet const& categoryIds)
    {
    Json::Value categoryJson (Json::objectValue); 
    
    for (auto& categoryId : categoryIds)
        {
        auto const& category = DgnCategory::Get(GetDgnDb(), categoryId);

        if (category.IsValid())
            categoryJson[categoryId.ToString()] = category->GetCategoryName();
        }

    return categoryJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherContext::GetViewJson(Json::Value& json, ViewDefinitionCR view, TransformCR transform)
    {
    auto                spatialView = view.ToSpatialView();
    ViewDefinition2dCP  view2d;
    if (nullptr != spatialView)
        {
        auto selectorId = spatialView->GetModelSelectorId().ToString();
        json["modelSelector"] = selectorId;
        }
    else if (nullptr != (view2d = view.ToDrawingView()) ||
             nullptr != (view2d = view.ToSheetView()))
        {
        auto modelId = view2d->GetBaseModelId();
        if (m_modelRanges.end() == m_modelRanges.find(modelId))
            return false; // model produced no tiles

        auto fakeModelSelectorId = modelId.ToString();
        fakeModelSelectorId.append("_2d");
        json["modelSelector"] = fakeModelSelectorId;
        }
    else
        {
        BeAssert(false && "Unexpected view type");
        return false;
        }

    json["name"] = view.GetName();
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

    auto cameraView = nullptr != spatialView ? view.ToView3d() : nullptr;
    if (nullptr != cameraView)
        {
        json["type"] = "camera";
        json["isCameraOn"] = cameraView->IsCameraOn();
        DPoint3d eyePoint = cameraView->GetEyePoint();
        transform.Multiply(eyePoint);
        json["eyePoint"] = PointToJson(eyePoint);

        json["lensAngle"] = cameraView->GetLensAngle().Radians();
        json["focusDistance"] = cameraView->GetFocusDistance();
        }
    else if (nullptr != spatialView)
        {
        json["type"] = "ortho";
        }
    else
        {
        json["type"] = nullptr != view.ToDrawingView() ? "drawing" : "sheet";;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PublisherContext::ExtractViewSelectors(DgnViewId& defaultViewId, DgnElementIdSet& allModelSelectors, T_CategorySelectorMap& allCategorySelectors, DgnElementIdSet& allDisplayStyles, DgnModelIdSet&   all2dModelIds)
    {
    for (auto const& viewId : m_viewIds)
        {
        auto viewDefinition = ViewDefinition::Get(GetDgnDb(), viewId);
        if (!viewDefinition.IsValid())
            continue;

        auto                        spatialView = viewDefinition->ToSpatialView();
        DrawingViewDefinitionCP     drawingView;
        SheetViewDefinitionCP       sheetView;

        if (nullptr != spatialView)
            {
            allModelSelectors.insert(spatialView->GetModelSelectorId());
            }
        else if (nullptr != (drawingView = viewDefinition->ToDrawingView()))    
            {
            all2dModelIds.insert(drawingView->GetBaseModelId());
            }
        else if (nullptr != (sheetView = viewDefinition->ToSheetView()))
            {
            all2dModelIds.insert(sheetView->GetBaseModelId());

            auto const&  model = GetDgnDb().Models().GetModel (sheetView->GetBaseModelId());

            if (model.IsValid() && nullptr != model->ToSheetModel())
                {
                auto   attachedViews = model->ToSheetModel()->GetSheetAttachmentViews(GetDgnDb());
                for (auto& attachedView : attachedViews)
                    {
                    auto    insertPair = allCategorySelectors.Insert(attachedView->GetCategorySelectorId(), T_ViewDefs());
                    insertPair.first->second.push_back(attachedView);

                    allDisplayStyles.insert(attachedView->GetDisplayStyleId());
                    if (nullptr != attachedView->ToView2d())
                        all2dModelIds.insert(attachedView->ToView2d()->GetBaseModelId());
                    }
                }
            }

        Json::Value entry(Json::objectValue);
 
        bvector<ViewDefinitionCPtr>  viewDefs;
        auto insertPair = allCategorySelectors.Insert(viewDefinition->GetCategorySelectorId(), viewDefs);
        insertPair.first->second.push_back(viewDefinition);

        allDisplayStyles.insert(viewDefinition->GetDisplayStyleId());

        // If for some reason the default view is not in the published set, we'll use the first view as the default
        if (!defaultViewId.IsValid())
            defaultViewId = viewId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetViewDefinitionsJson()
    {
    Json::Value     viewsJson = Json::objectValue;
    Transform       spatialTransform = Transform::FromProduct(m_spatialToEcef, m_dbToTile);

    for (auto const& viewId : m_viewIds)
        {
        auto viewDefinition = ViewDefinition::Get(GetDgnDb(), viewId);
        if (!viewDefinition.IsValid())
            continue;

        Json::Value entry(Json::objectValue);
 
        if (GetViewJson(entry, *viewDefinition, nullptr != viewDefinition->ToSpatialView() ? spatialTransform : Transform::FromIdentity()))
            viewsJson[viewId.ToString()] = entry;
        }
    return viewsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId, GlobeMode globeMode)
    {
    Utf8String rootNameUtf8(m_rootName.c_str());
    json["name"] = rootNameUtf8;

    Transform spatialTransform = Transform::FromProduct(m_spatialToEcef, m_dbToTile);

    if (!m_spatialToEcef.IsIdentity())
        {
        DPoint3d groundEcefPoint;
        spatialTransform.Multiply(groundEcefPoint, groundPoint);
        json["groundPoint"] = PointToJson(groundEcefPoint);
        }

    DgnElementIdSet         allModelSelectors;
    T_CategorySelectorMap   allCategorySelectors;
    DgnElementIdSet         allDisplayStyles;
    DgnModelIdSet           all2dModelIds;
    
    ExtractViewSelectors (defaultViewId, allModelSelectors, allCategorySelectors, allDisplayStyles, all2dModelIds);

    if (!defaultViewId.IsValid())
        return Status::NoGeometry;

    json["views"] = GetViewDefinitionsJson();
    json["defaultView"] = defaultViewId.ToString();

    WriteModelsJson(json, allModelSelectors, all2dModelIds);
    WriteCategoriesJson(json, allCategorySelectors, true);
    json["displayStyles"] = GetDisplayStylesJson(allDisplayStyles);

    AxisAlignedBox3d projectExtents = m_projectExtents;
    spatialTransform.Multiply(projectExtents, projectExtents);
    json["projectExtents"] = RangeToJson(projectExtents);
    json["projectTransform"] = TransformToJson(m_spatialToEcef);
    json["projectOrigin"] = PointToJson(m_projectExtents.GetCenter());

    
    if (!m_schedulesJson.isNull() && m_schedulesJson.size() > 0)
        {
        json["timeline"] = true;
        json["timelineVisible"] = true;
        json["schedules"] = std::move(m_schedulesJson);
        }

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
    json["classifiers"] = GetAllClassifiersJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetAllClassifiersJson()
    {
    Json::Value classifiersValue = Json::objectValue;

    for (auto& curr : m_classifierMap)
        {
        size_t      index = 0;
        for (auto& classifierInfo : curr.second)
            {
            auto&           classifier = classifierInfo.m_classifier;
            Json::Value     classifierValue = classifier.ToJson();

            classifierValue["tilesetUrl"] = GetTilesetURL(classifier.GetModelId(), &classifierInfo);
            classifiersValue[Utf8String(classifierInfo.GetRootName())] = classifierValue;
            }
        }
    return classifiersValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherContext::CategoryOnInAnyView(DgnCategoryId categoryId, PublisherContext::T_ViewDefs views, bool testUsed) const
    {
    auto const& category = DgnCategory::Get(GetDgnDb(), categoryId);

    if (!category.IsValid())
        return false;

    auto            subCategories = category->MakeSubCategoryIterator();
    bool            anySubCategories = false, anySubCategoriesOn = false;
    auto            subCategoryIds = subCategories.BuildIdSet<DgnSubCategoryId>();
    auto            name = category->GetCategoryName();

    for (auto& subCategoryId : subCategoryIds)
        {
        auto const& subcategory = DgnSubCategory::Get(GetDgnDb(), subCategoryId);

        if (!subcategory.IsValid())
            continue;

        auto        subName = subcategory->GetSubCategoryName();

        anySubCategories = true;
        
        for (auto& view : views)
            {
            ViewControllerPtr viewController = view->LoadViewController();

            if ((!testUsed || IsSubCategoryUsed(subCategoryId)) && !viewController->GetSubCategoryAppearance(subCategoryId).IsInvisible())
                anySubCategoriesOn = true;
            }
        }
    return !anySubCategories || anySubCategoriesOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteCategoriesJson(Json::Value& json, T_CategorySelectorMap const& selectorIds, bool testUsed)
    {
    DgnCategoryIdSet    allCategories;                                                                                                                      
    Json::Value&        selectorsJson = (json["categorySelectors"] = Json::objectValue);

    for (auto const& selectorId : selectorIds)
        {
        auto    selector = GetDgnDb().Elements().Get<CategorySelector>(selectorId.first);

        if (selector.IsValid())
            {
            auto                cats = selector->GetCategories();
            DgnCategoryIdSet    onInAnyViewCats;

            for (auto const& cat : cats)
                if (CategoryOnInAnyView(cat, selectorId.second, testUsed))
                    onInAnyViewCats.insert(cat);

            selectorsJson[selectorId.first.ToString()] = IdSetToJson(onInAnyViewCats);
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

    json["viewFlags"] = style.GetViewFlags().ToJson();

    auto style3d = style.ToDisplayStyle3d();
    switch (m_globeMode)
        {
        case GlobeMode::Off:
            json["isGlobeVisible"] = false;
            break;

        case GlobeMode::On:
            json["isGlobeVisible"] = true;
            break;

        case GlobeMode::FromDisplayStyle:
            if (nullptr != style3d)
                json["isGlobeVisible"] = style3d->IsGroundPlaneEnabled() || IsGeolocated();
        }

    return json;
    }

