/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/TilePublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include "Constants.h"
#include <crunch/CrnLib.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;

//=======================================================================================
// We use a hierarchical batch table to organize features by element and subcategory,
// and subcategories by category
// Each feature has a batch table class corresponding to its DgnGeometryClass.
// The feature classes have no properties, only parents for classification.
// The element, category, and subcategory classes each have an ID property.
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct BatchTableBuilder
{
private:
    struct SubcatInfo { uint32_t m_index; uint32_t m_catIndex; };

    enum ClassIndex
    {
        kClass_Primary,
        kClass_Construction,
        kClass_Dimension,
        kClass_Pattern,
        kClass_Element,
        kClass_SubCategory,
        kClass_Category,

        kClass_COUNT,
        kClass_FEATURE_COUNT = kClass_Pattern+1,
    };

    static constexpr Utf8CP s_classNames[kClass_COUNT] =
        {
        "Primary", "Construction", "Dimension", "Pattern", "Element", "SubCategory", "Category"
        };

    Json::Value                         m_json; // "HIERARCHY": object
    DgnDbR                              m_db;
    FeatureAttributesMapCR              m_attrs;
    bmap<DgnElementId, uint32_t>        m_elems;
    bmap<DgnSubCategoryId, SubcatInfo>  m_subcats;
    bmap<DgnCategoryId, uint32_t>       m_cats;

    template<typename T, typename U> static auto Find(T& map, U const& key) -> typename T::iterator
        {
        return map.find(key);
        }
    template<typename T, typename U> static uint32_t FindOrInsert(T& map, U const& key)
        {
        auto iter = Find(map, key);
        if (iter != map.end())
            return iter->second;

        uint32_t index = static_cast<uint32_t>(map.size());
        map[key] = index;
        return index;
        }
    template<typename T, typename U> static uint32_t GetIndex(T& map, U const& key)
        {
        auto iter = Find(map, key);
        BeAssert(iter != map.end());
        return iter->second;
        }

    uint32_t MapElementIndex(DgnElementId id) { return FindOrInsert(m_elems, id); }
    uint32_t GetElementIndex(DgnElementId id) { return GetIndex(m_elems, id); }
    uint32_t MapCategoryIndex(DgnCategoryId id) { return FindOrInsert(m_cats, id); }
    uint32_t GetCategoryIndex(DgnCategoryId id) { return GetIndex(m_cats, id); }
    SubcatInfo MapSubCategoryInfo(DgnSubCategoryId id);
    SubcatInfo GetSubCategoryInfo(DgnSubCategoryId id);

    Json::Value& GetClass(ClassIndex idx) { return m_json["classes"][idx]; }
    static ClassIndex GetFeatureClassIndex(DgnGeometryClass geomClass);

    void DefineClasses();
    void MapFeatures();
    void MapParents();
    void MapElements(uint32_t offset);
    void MapSubCategories(uint32_t offset, uint32_t categoriesOffset);
    void MapCategories(uint32_t offset);

    void Build();
public:
    BatchTableBuilder(FeatureAttributesMapCR attrs, DgnDbR db)
        : m_json(Json::objectValue), m_db(db), m_attrs(attrs)
        {
        Build();
        }

    Json::Value& GetHierarchy() { return m_json; }
    Utf8String ToString()
        {
        Json::Value json;
        json["HIERARCHY"] = GetHierarchy();
        return Json::FastWriter().write(json);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::Build()
    {
    m_json["parentCounts"] = Json::arrayValue;
    m_json["classIds"] = Json::arrayValue;
    m_json["parentIds"] = Json::arrayValue;

    // Set up the "classes" array. For each member, defines "name" property only. "length" and "instances" property TBD.
    DefineClasses();

    // Makes sure every instance of every class is assigned an index into the "classIds" array (relative to the index of
    // the first instance of that class).
    // Adds index for each Feature into "classIds" (index == batch ID)
    // Sets "parentCounts" for each Feature (all == 2)
    // Sets "length" for each of the Feature classes
    MapFeatures();

    // Populates "classes" for all instances of abstract (parent) classes
    // Populates the "parentIds" and "parentCounts" arrays for all instances of all classes
    // Sets the "length" and "instances" property of "classes" member for each abstract (parent) class
    // Sets "instancesLength" to the total number of instances of all classes.
    MapParents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::GetFeatureClassIndex(DgnGeometryClass geomClass) -> ClassIndex
    {
    switch (geomClass)
        {
        case DgnGeometryClass::Primary:         return kClass_Primary;
        case DgnGeometryClass::Construction:    return kClass_Construction;
        case DgnGeometryClass::Dimension:       return kClass_Dimension;
        case DgnGeometryClass::Pattern:         return kClass_Pattern;
        default:
            BeAssert(false);
            return kClass_Primary;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapSubCategories(uint32_t offset, uint32_t categoriesOffset)
    {
    Json::Value& subcats = GetClass(kClass_SubCategory);
    subcats["length"] = m_subcats.size();

    Json::Value &instances = (subcats["instances"] = Json::objectValue),
                &subcat_id = (instances["subcategory"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"],
                &parentIds = m_json["parentIds"];

    uint32_t parentIdsOffset = static_cast<uint32_t>(m_attrs.size()) * 2; // 2 parents per feature followed by 1 parent per subcategory
    for (auto const& kvp : m_subcats)
        {
        Json::Value::ArrayIndex index = kvp.second.m_index;
        subcat_id[index] = kvp.first.ToString();

        classIds[index + offset] = kClass_SubCategory;
        parentCounts[index + offset] = 1;
        parentIds[index + parentIdsOffset] = kvp.second.m_catIndex + categoriesOffset;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapCategories(uint32_t offset)
    {
    Json::Value& cats = GetClass(kClass_Category);
    cats["length"] = m_cats.size();

    Json::Value &instances = (cats["instances"] = Json::objectValue),
                &cat_id = (instances["category"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"];

    for (auto const& kvp : m_cats)
        {
        classIds[offset + kvp.second] = kClass_Category;
        parentCounts[offset + kvp.second] = 0;
        cat_id[kvp.second] = kvp.first.ToString();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapElements(uint32_t offset)
    {
    Json::Value& elements = GetClass(kClass_Element);
    elements["length"] = m_elems.size();

    Json::Value& instances = (elements["instances"] = Json::objectValue);
    Json::Value& elem_id = (instances["element"] = Json::arrayValue);
    Json::Value& classIds = m_json["classIds"];
    Json::Value& parentCounts = m_json["parentCounts"];
    for (auto const& kvp : m_elems)
        {
        classIds[offset + kvp.second] = kClass_Element;
        parentCounts[offset + kvp.second] = 0;
        elem_id[kvp.second] = kvp.first.ToString();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapParents()
    {
    uint32_t elementsOffset = static_cast<uint32_t>(m_attrs.size());
    uint32_t subcatsOffset = elementsOffset + static_cast<uint32_t>(m_elems.size());
    uint32_t catsOffset = subcatsOffset + static_cast<uint32_t>(m_subcats.size());
    uint32_t totalInstances = catsOffset + static_cast<uint32_t>(m_cats.size());

    m_json["instancesLength"] = totalInstances;

    // Now that every instance of every class has an index into "classIds", we can map parent IDs
    Json::Value& parentIds = m_json["parentIds"];
    for (auto const& kvp : m_attrs)
        {
        FeatureAttributesCR attr = kvp.first;
        uint32_t index = kvp.second * 2; // 2 parents per feature

        parentIds[index] = elementsOffset + GetElementIndex(attr.GetElementId());
        parentIds[index+1] = subcatsOffset + GetSubCategoryInfo(attr.GetSubCategoryId()).m_index;
        }

    // Set "instances" and "length" to Element class, and add elements to "classIds"
    MapElements(elementsOffset);

    // Set "instances" and "length" to SubCategory class, and add subcategories to "classIds" and "parentIds"
    MapSubCategories(subcatsOffset, catsOffset);
    MapCategories(catsOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapFeatures()
    {
    Json::Value& classIds = m_json["classIds"];
    Json::Value& parentCounts = m_json["parentCounts"];

    uint32_t instanceCounts[kClass_FEATURE_COUNT] = { 0 };

    for (auto const& kvp : m_attrs)
        {
        FeatureAttributesCR attr = kvp.first;
        uint32_t index = kvp.second;
        ClassIndex classIndex = GetFeatureClassIndex(attr.GetClass());

        classIds[index] = classIndex;
        parentCounts[index] = 2; // element, subcategory

        ++instanceCounts[classIndex];

        // Ensure all parent instances are mapped
        MapElementIndex(attr.GetElementId());
        MapSubCategoryInfo(attr.GetSubCategoryId());
        }

    // Set the number of instances of each class
    for (uint8_t classIndex = 0; classIndex < kClass_FEATURE_COUNT; classIndex++)
        GetClass(static_cast<ClassIndex>(classIndex))["length"] = instanceCounts[classIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::DefineClasses()
    {
    auto& classes = (m_json["classes"] = Json::arrayValue);
    for (uint8_t i = 0; i < kClass_COUNT; i++)
        {
        auto& cls = (classes[i] = Json::objectValue);
        cls["name"] = s_classNames[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::MapSubCategoryInfo(DgnSubCategoryId id) -> SubcatInfo
    {
    auto iter = Find(m_subcats, id);
    if (iter != m_subcats.end())
        return iter->second;

    DgnCategoryId catId = DgnSubCategory::QueryCategoryId(m_db, id);
    SubcatInfo info;
    info.m_index = static_cast<uint32_t>(m_subcats.size());
    info.m_catIndex = MapCategoryIndex(catId);
    m_subcats[id] = info;
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::GetSubCategoryInfo(DgnSubCategoryId id) -> SubcatInfo
    {
    auto iter = Find(m_subcats, id);
    BeAssert(iter != m_subcats.end());
    return iter->second;
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
        printf ("Total Compression: %f megapixels, %f seconds (%f minutes, %f hours)\n", m_textureCompressionMegaPixels, m_textureCompressionSeconds, m_textureCompressionSeconds / 60.0, m_textureCompressionSeconds / 3600.0);
        printf ("Compression Rate: %f megapixels/second\n", m_textureCompressionMegaPixels / m_textureCompressionSeconds);
        }
    if (0 != m_pointCloudCount)
        {
        printf ("Point Cloud count: %g, Total Points: %g, Min Points: %g, Max Points: %g, Average: %g\n", (double) m_pointCloudCount, (double) m_pointCloudTotalPoints, (double) m_pointCloudMinPoints,(double) m_pointCloudMaxPoints, (double) m_pointCloudTotalPoints / (double) m_pointCloudCount);
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
BeFileName  TilePublisher::GetBinaryDataFileName() const
    {
    WString rootName;
    BeFileName dataDir = m_context.GetDataDirForModel(m_tile.GetModel(), &rootName);

    return  BeFileName(nullptr, dataDir.c_str(), m_tile.GetFileName (rootName.c_str(), m_tile.GetFileExtension().c_str()).c_str(), nullptr);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilePublisher::Publish()
    {
    PublishableTileGeometry publishableGeometry = m_tile.GeneratePublishableGeometry(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, false, m_context.WantSurfacesOnly(), nullptr);

    if (publishableGeometry.IsEmpty())
        return PublisherContext::Status::NoGeometry;                            // Nothing to write...Ignore this tile (it will be omitted when writing tileset data as its published range will be NullRange.

    BeFileName      fileName = GetBinaryDataFileName();

    std::FILE*  outputFile = _wfopen(fileName.c_str(), L"wb");

    if (nullptr == outputFile)
        {
        BeAssert (false && "Unable to open output file");
        return PublisherContext::Status::CantOpenOutputFile;
        }
    
        
    if (publishableGeometry.Parts().empty())
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
        uint32_t        tileCount = (publishableGeometry.Meshes().empty() ? 0 : 1) + publishableGeometry.Parts().size(), zero = 0;

        std::fwrite(s_compositeTileMagic, 1, 4, outputFile);
        std::fwrite(&s_compositeTileVersion, 1, 4, outputFile);
        long    compositeSizeLocation = ftell (outputFile);
        std::fwrite(&zero, 1, 4, outputFile);                   // Filled in below...
        std::fwrite(&tileCount, 1, 4, outputFile);

        WriteTileMeshes(outputFile, publishableGeometry);

        uint32_t    compositeSize = std::ftell(outputFile);
        std::fseek (outputFile, compositeSizeLocation, SEEK_SET);
        std::fwrite (&compositeSize, 1, 4, outputFile);
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
static void    padTo4ByteBoundary(std::FILE* outputFile)
    {
    std::fseek(outputFile, 0, SEEK_END);
    long        position = ftell(outputFile), padBytes = (4 - position % 4);

    if (0 != padBytes)
        {
        uint64_t    zero = 0;

        std::fwrite (&zero, 1, padBytes, outputFile);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void    padTo4ByteBoundary(Utf8String& string)
    {
    while (0 != string.size() % 4)
        string = string + " ";
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
        WriteBatched3dModel (outputFile, publishableGeometry.Meshes());
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

    Utf8String      featureTableStr =  Json::FastWriter().write(featureTable);

    padTo4ByteBoundary(featureTableStr);

    uint32_t        zero = 0, 
                    featureTableStrLen = featureTableStr.size(),
                    featureTableBinaryLength = pointCloud.Points().size() * (3 * sizeof(int16_t) + (rgbPresent ?  sizeof(TileMeshPointCloud::Rgb) : 0));

    std::fwrite(s_pointCloudMagic, 1, 4, outputFile);
    std::fwrite(&s_pointCloudVersion, 1, 4, outputFile);                                                                                                                                  
    long    lengthDataPosition = ftell(outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile);    // Total length filled in below.
    std::fwrite(&featureTableStrLen, 1, sizeof(uint32_t), outputFile);          
    std::fwrite(&featureTableBinaryLength, 1, sizeof(uint32_t), outputFile);    

    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile);    // No batch for now.
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile);    // No batch for now.

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
    std::fwrite(&dataSize, 1, sizeof(uint32_t), outputFile);
    std::fseek(outputFile, 0, SEEK_END);


    m_tile.SetPublishedRange (positionRange);

    m_context.m_statistics.RecordPointCloud(pointCloud.Points().size());
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

    bvector<float>          upFloats, rightFloats;
    FeatureAttributesMap    attributesSet;
    DRange3d                positionRange = DRange3d::NullRange();

    bool validIdsPresent = false;
    for (auto& instance : part->Instances())
        {
        DPoint3d    translation;
        DVec3d      right, up;
        RotMatrix   rMatrix;

        instance.GetTransform().GetTranslation(translation);
        instance.GetTransform().GetMatrix(rMatrix);

        positionRange.Extend(translation);
        rotationPresent |= !rMatrix.IsIdentity();

        rMatrix.GetColumn(right, 0);
        rMatrix.GetColumn(up, 1);

        rightFloats.push_back(right.x);
        rightFloats.push_back(right.y);
        rightFloats.push_back(right.z);


        upFloats.push_back(up.x);
        upFloats.push_back(up.y);
        upFloats.push_back(up.z);

        extendRange (publishedRange, part->Meshes(), &instance.GetTransform());
        attributeIndices.push_back(attributesSet.GetIndex(instance.GetAttributes()));
        validIdsPresent |= (0 != attributeIndices.back());
        }
    DVec3d              positionScale;
    bvector<uint16_t>   quantizedPosition;
    double              range = (double) (0xffff);

    positionScale = DVec3d::FromStartEnd(positionRange.low, positionRange.high);
    for (auto& instance : part->Instances())
        {
        DPoint3d    translation;

        instance.GetTransform().GetTranslation(translation);

        quantizedPosition.push_back((uint16_t) (.5 + range * (translation.x - positionRange.low.x) / positionScale.x));
        quantizedPosition.push_back((uint16_t) (.5 + range * (translation.y - positionRange.low.y) / positionScale.y));
        quantizedPosition.push_back((uint16_t) (.5 + range * (translation.z - positionRange.low.z) / positionScale.z));
        }

    AddExtensions(partData);
    AddDefaultScene(partData);
    AddMeshes (partData, part->Meshes());

    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.x);
    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.y);
    featureTableData.m_json["QUANTIZED_VOLUME_OFFSET"].append(positionRange.low.z);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.x);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.y);
    featureTableData.m_json["QUANTIZED_VOLUME_SCALE"].append(positionScale.z);

    featureTableData.m_json["POSITION_QUANTIZED"]["byteOffset"] = featureTableData.BinaryDataSize();
    featureTableData.AddBinaryData(quantizedPosition.data(), quantizedPosition.size()*sizeof(uint16_t));

    if (validIdsPresent)
        {
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
        }

    BatchTableBuilder batchTableBuilder(attributesSet, m_context.GetDgnDb());
    Utf8String      batchTableStr = batchTableBuilder.ToString();
    Utf8String      featureTableStr = Json::FastWriter().write(featureTableData.m_json);

    // Pad the feature table string to insure that the binary is 4 byte aligned.
    padTo4ByteBoundary(featureTableStr);

    uint32_t        batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    uint32_t        featureTableJsonLength = static_cast<uint32_t> (featureTableStr.size());
    uint32_t        featureTableBinarySize = featureTableData.BinaryDataSize(), gltfFormat = 1, zero = 0;

    uint32_t        padBytes = (featureTableJsonLength + featureTableBinarySize + batchTableStrLen) % 8;
    uint64_t        padZero;

    featureTableData.AddBinaryData (&padZero, padBytes);
    featureTableBinarySize += padBytes;


    long            startPosition = ftell(outputFile);

    std::fwrite(s_instanced3dMagic, 1, 4, outputFile);
    std::fwrite(&s_instanced3dVersion, 1, 4, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    std::fwrite (&zero, 1, sizeof(uint32_t),outputFile);        // Filled in later.
    std::fwrite(&featureTableJsonLength, 1, sizeof(uint32_t),outputFile);
    std::fwrite(&featureTableBinarySize, 1, sizeof(uint32_t),outputFile);
    std::fwrite(&batchTableStrLen, 1, sizeof(uint32_t),outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t),outputFile);         // Batch table binary (not used).
    std::fwrite(&gltfFormat, 1, sizeof(uint32_t), outputFile);
    std::fwrite(featureTableStr.data(), 1, featureTableJsonLength, outputFile);
    std::fwrite(featureTableData.BinaryData(), 1, featureTableData.BinaryDataSize(), outputFile);
    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, outputFile);

    WriteGltf(outputFile, partData);
    padTo4ByteBoundary (outputFile);
    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    std::fwrite(&dataSize, 1, sizeof(uint32_t), outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteBatched3dModel(std::FILE* outputFile, TileMeshList const& meshes)
    {
    PublishTileData     tileData;
    bool                validIdsPresent = meshes.front()->ValidIdsPresent();

    AddExtensions(tileData);
    AddDefaultScene(tileData);
    AddMeshes(tileData, meshes);

    FeatureAttributesMapCR attributes = m_tile.GetAttributes();
    Utf8String batchTableStr;
    if (validIdsPresent)
        {
        BatchTableBuilder batchTableBuilder(attributes, m_context.GetDgnDb());
        batchTableStr = batchTableBuilder.ToString();
        }

    uint32_t batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    uint32_t zero = 0;
    uint32_t b3dmNumBatches = validIdsPresent ? attributes.size() : 0;

    long    startPosition = ftell (outputFile);
    std::fwrite(s_b3dmMagic, 1, 4, outputFile);
    std::fwrite(&s_b3dmVersion, 1, 4, outputFile);
    long    lengthDataPosition = ftell(outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile);    // Filled in below.
    std::fwrite(&batchTableStrLen, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile); // length of binary portion of batch table - we have no binary batch table data
    std::fwrite(&b3dmNumBatches, 1, sizeof(uint32_t), outputFile);
    std::fwrite(batchTableStr.data(), 1, batchTableStrLen, outputFile);

    WriteGltf (outputFile, tileData);

    padTo4ByteBoundary (outputFile);
    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    std::fwrite(&dataSize, 1, sizeof(uint32_t), outputFile);
    std::fseek(outputFile, 0, SEEK_END);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::WriteGltf(std::FILE* outputFile, PublishTileData tileData)
    {
    Utf8String  sceneStr = Json::FastWriter().write(tileData.m_json);
    uint32_t    sceneStrLength = static_cast<uint32_t>(sceneStr.size()), zero = 0;

    long    startPosition = ftell(outputFile);
    std::fwrite(&s_gltfMagic, 1, 4, outputFile);
    std::fwrite(&s_gltfVersion, 1, sizeof(uint32_t), outputFile);
    long    lengthDataPosition = ftell(outputFile);
    std::fwrite(&zero, 1, sizeof(uint32_t), outputFile);        // Filled in below.
    std::fwrite(&sceneStrLength, 1, sizeof(uint32_t), outputFile);
    std::fwrite(&s_gltfSceneFormat, 1, sizeof(uint32_t), outputFile);

    std::fwrite(sceneStr.data(), 1, sceneStrLength, outputFile);
    if (!tileData.m_binaryData.empty())
        std::fwrite(tileData.m_binaryData.data(), 1, tileData.BinaryDataSize(), outputFile);

    uint32_t    dataSize = static_cast<uint32_t> (ftell(outputFile) - startPosition);
    std::fseek(outputFile, lengthDataPosition, SEEK_SET);
    std::fwrite(&dataSize, 1, sizeof(uint32_t), outputFile);
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
 Utf8String TilePublisher::AddTextureImage (PublishTileData& tileData, TileTextureImageCR textureImage, TileMeshCR mesh, Utf8CP  suffix)
    {
    auto const& found = m_textureImages.find (&textureImage);

    if (found != m_textureImages.end())
        return found->second;

    bool        hasAlpha = textureImage.GetImageSource().GetFormat() == ImageSource::Format::Png;

    Utf8String  textureId = Utf8String ("texture_") + suffix;
    Utf8String  imageId   = Utf8String ("image_")   + suffix;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + suffix;

    tileData.m_json["textures"][textureId] = Json::objectValue;
    tileData.m_json["textures"][textureId]["format"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    tileData.m_json["textures"][textureId]["internalFormat"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    tileData.m_json["textures"][textureId]["sampler"] = "sampler_0";
    tileData.m_json["textures"][textureId]["source"] = imageId;

    tileData.m_json["images"][imageId] = Json::objectValue;


    DRange3d    range = mesh.GetRange(), uvRange = mesh.GetUVRange();
    Image       image (textureImage.GetImageSource(), hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);

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

    ImageSource         imageSource = textureImage.GetImageSource();
    static const int    s_imageQuality = 50;


    if (targetImageSize.x != imageSize.x || targetImageSize.y != imageSize.y)
        {
        Image           targetImage = Image::FromResizedImage (targetImageSize.x, targetImageSize.y, image);

        imageSource = ImageSource (targetImage, textureImage.GetImageSource().GetFormat(), s_imageQuality);
        }

    if (m_context.GetTextureMode() == PublisherContext::External ||
        m_context.GetTextureMode() == PublisherContext::Compressed)
        {
        WString     name = WString(imageId.c_str(), true) + L"_" + m_tile.GetNameSuffix(), extension;

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
            {
            extension = ImageSource::Format::Jpeg == imageSource.GetFormat() ? L"jpg" : L"png";
            std::FILE*  outputFile = _wfopen(BeFileName(nullptr, m_context.GetDataDirForModel(m_tile.GetModel()).c_str(), name.c_str(), extension.c_str()).c_str(), L"wb");
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
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = "image/jpeg";


        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = targetImageSize.x;
        tileData.m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = targetImageSize.y;

        ByteStream const& imageData = imageSource.GetByteStream();
        tileData.m_json["bufferViews"][bvImageId]["byteOffset"] = tileData.BinaryDataSize();
        tileData.m_json["bufferViews"][bvImageId]["byteLength"] = imageData.size();
        tileData.AddBinaryData (imageData.data(), imageData.size());
        }

    m_textureImages.Insert (&textureImage, textureId);

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddUnlitShaderTechnique (PublishTileData& tileData, bool doBatchIds)
    {
    Utf8String      s_techniqueName = "unlitTechnique";

    if (tileData.m_json.isMember("techniques") &&
        tileData.m_json["techniques"].isMember(s_techniqueName.c_str()))
        return s_techniqueName;

    Json::Value     technique = Json::objectValue;

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

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
    if (doBatchIds)
        techniqueAttributes["a_batchId"] = "batch";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";

    auto& rootProgramNode = (tileData.m_json["programs"][s_programName] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");

    if (doBatchIds)
        AppendProgramAttribute(rootProgramNode, "a_batchId");

    rootProgramNode["vertexShader"]   = s_vertexShaderName;
    rootProgramNode["fragmentShader"] = s_fragmentShaderName;

    auto& shaders = tileData.m_json["shaders"];
    AddShader (shaders, s_vertexShaderName, GLTF_VERTEX_SHADER, s_vertexShaderBufferViewName);
    AddShader (shaders, s_fragmentShaderName, GLTF_FRAGMENT_SHADER, s_fragmentShaderBufferViewName);

    std::string vertexShaderString = s_shaderPrecision + (doBatchIds ? s_batchIdShaderAttribute : "") + s_unlitVertexShader;

    tileData.AddBufferView(s_vertexShaderBufferViewName, vertexShaderString);
    tileData.AddBufferView(s_fragmentShaderBufferViewName, s_unlitFragmentShader); 

    AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
    techniqueUniforms["u_color"] = "color";

    tileData.m_json["techniques"][s_techniqueName.c_str()] = technique;

    return s_techniqueName;
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
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String     TilePublisher::AddMeshShaderTechnique (PublishTileData& tileData, bool textured, bool transparent, bool ignoreLighting, bool doBatchIds)
    {
    Utf8String  prefix = textured ? "textured" : "untextured";

    if (transparent)
        prefix = prefix + "Transparent";

    if (ignoreLighting)
        prefix = prefix + "Unlit";

    Utf8String  techniqueName = prefix + "Technique";
    
    if (tileData.m_json.isMember("techniques") &&
        tileData.m_json["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value     technique = Json::objectValue;

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (!ignoreLighting)
        {
        AddTechniqueParameter(technique, "n", GLTF_INT_VEC2, "NORMAL");
        AddTechniqueParameter(technique, "nmx", GLTF_FLOAT_MAT3, "MODELVIEWINVERSETRANSPOSE");
        }
    if (doBatchIds)
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
    if (doBatchIds)
        techniqueAttributes["a_batchId"] = "batch";

    if(!ignoreLighting)
        techniqueAttributes["a_n"] = "n";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";
    if (!ignoreLighting)
        techniqueUniforms["u_nmx"] = "nmx";

    auto& rootProgramNode = (tileData.m_json["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");
    if (doBatchIds)
        AppendProgramAttribute(rootProgramNode, "a_batchId");
    if (!ignoreLighting)
        AppendProgramAttribute(rootProgramNode, "a_n");

    rootProgramNode["vertexShader"]   = vertexShader.c_str();
    rootProgramNode["fragmentShader"] = fragmentShader.c_str();

    auto& shaders = tileData.m_json["shaders"];
    AddShader(shaders, vertexShader.c_str(), GLTF_VERTEX_SHADER, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), GLTF_FRAGMENT_SHADER, fragmentShaderBufferView.c_str());

    std::string batchIdString = doBatchIds ? s_batchIdShaderAttribute : std::string();
    std::string vertexShaderString = s_shaderPrecision + batchIdString + (ignoreLighting ? s_unlitTextureVertexShader  : (textured ? s_texturedVertexShader : s_untexturedVertexShader));

    tileData.AddBufferView(vertexShaderBufferView.c_str(),  vertexShaderString);
    tileData.AddBufferView(fragmentShaderBufferView.c_str(), ignoreLighting ? s_unlitTextureFragmentShader: (textured ? s_texturedFragShader    : s_untexturedFragShader)); 

    // Diffuse...
    if (textured)
        {
        AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        AddTechniqueParameter(technique, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");

        tileData.m_json["samplers"]["sampler_0"] = Json::objectValue;
        tileData.m_json["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;

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
        addTransparencyToTechnique (technique);

    tileData.m_json["techniques"][techniqueName.c_str()] = technique;

    return techniqueName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddMeshMaterial (PublishTileData& tileData, bool& isTextured, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    Utf8String      materialName = Utf8String ("Material_") + suffix;

    if (nullptr == displayParams)
        return materialName;


    RgbFactor       specularColor = { 1.0, 1.0, 1.0 };
    double          specularExponent = s_qvFinish * s_qvExponentMultiplier;
    uint32_t        rgbInt  = displayParams->GetFillColor();
    double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
    Json::Value&    materialValue = tileData.m_json["materials"][materialName.c_str()] = Json::objectValue;
    bool            isUnlit = displayParams->GetIgnoreLighting();
    RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);

    if (!isUnlit && displayParams->GetMaterialId().IsValid())
        {
        auto jsonMaterial = RenderingAsset::Load(displayParams->GetMaterialId(), m_context.GetDgnDb());

        if (nullptr != jsonMaterial)
            {
            static double       s_finishScale = 15.0;

            if (jsonMaterial->GetBool (RENDER_MATERIAL_FlagHasSpecularColor, false))
                specularColor = jsonMaterial->GetColor (RENDER_MATERIAL_SpecularColor);

            if (jsonMaterial->GetBool (RENDER_MATERIAL_FlagHasFinish, false))
                specularExponent = jsonMaterial->GetDouble (RENDER_MATERIAL_Finish, s_qvSpecular) * s_finishScale;

            if (jsonMaterial->GetBool (RENDER_MATERIAL_FlagHasBaseColor, false))
                rgb = jsonMaterial->GetColor (RENDER_MATERIAL_Color);

            if (jsonMaterial->GetBool (RENDER_MATERIAL_FlagHasTransmit, false))
                alpha = 1.0 - jsonMaterial->GetDouble (RENDER_MATERIAL_Transmit, 0.0);

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
        materialValue["technique"] = AddMeshShaderTechnique (tileData, true, alpha < 1.0, displayParams->GetIgnoreLighting(), doBatchIds).c_str();
        materialValue["values"]["tex"] = AddTextureImage (tileData, *textureImage, mesh, suffix);
        }
    else
        {
        auto& materialColor = materialValue["values"]["color"] = Json::arrayValue;

        materialColor.append(rgb.red);
        materialColor.append(rgb.green);
        materialColor.append(rgb.blue);
        materialColor.append(alpha);

        materialValue["technique"] = isUnlit ? AddUnlitShaderTechnique (tileData, doBatchIds).c_str() : AddMeshShaderTechnique(tileData, false, alpha < 1.0, false, doBatchIds).c_str();
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
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TilePublisher::AddPolylineMaterial (PublishTileData& tileData, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds)
    {
    Utf8String      materialName = Utf8String ("PolylineMaterial_") + suffix;

    if (nullptr == displayParams)
        return materialName;

    uint32_t        rgbInt  = displayParams->GetFillColor();
    double          alpha = 1.0 - ((uint8_t*)&rgbInt)[3]/255.0;
    Json::Value&    materialValue = tileData.m_json["materials"][materialName.c_str()] = Json::objectValue;
    RgbFactor       rgb     = RgbFactor::FromIntColor (rgbInt);
    static double   s_minLineWidth = 1.0;
    static double   s_featherPixels = 1.0;

    auto& materialColor = materialValue["values"]["color"] = Json::arrayValue;

    materialColor.append(rgb.red);
    materialColor.append(rgb.green);
    materialColor.append(rgb.blue);
    materialColor.append(alpha);

    Utf8String      s_techniqueName = "polylineTechnique";

    double          halfWidthPixels = std::max(s_minLineWidth, (double) displayParams->GetRasterWidth()) / 2.0, featherPixels;
    halfWidthPixels += (featherPixels = std::min(halfWidthPixels/2.0, s_featherPixels));
    materialValue["values"]["width"]   = halfWidthPixels;
    materialValue["values"]["feather"] = featherPixels/halfWidthPixels;

    if (!tileData.m_json.isMember("techniques") ||
        !tileData.m_json["techniques"].isMember(s_techniqueName.c_str()))
        {
        Json::Value     technique = Json::objectValue;

        AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
        AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
        AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
        AddTechniqueParameter(technique, "direction", GLTF_FLOAT_VEC3, "DIRECTION");
        AddTechniqueParameter(technique, "vertexDelta", GLTF_FLOAT_VEC3, "VERTEXDELTA");
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

        static char const   *s_programName                    = "polylineProgram",
                            *s_vertexShaderName               = "polylineVertexShader",
                            *s_fragmentShaderName             = "polylineFragmentShader",
                            *s_vertexShaderBufferViewName     = "polylineVertexShaderBufferView",
                            *s_fragmentShaderBufferViewName   = "polylineFragmentShaderBufferView";

        technique["program"] = s_programName;

        auto&   techniqueStates = technique["states"];
        techniqueStates["enable"] = Json::arrayValue;
        techniqueStates["enable"].append(GLTF_DEPTH_TEST);

        auto& techniqueAttributes = technique["attributes"];

        if (doBatchIds)
            techniqueAttributes["a_batchId"] = "batch";

        techniqueAttributes["a_pos"]  = "pos";
        techniqueAttributes["a_direction"]  = "direction";
        techniqueAttributes["a_vertexDelta"] = "vertexDelta";

        auto& techniqueUniforms = technique["uniforms"];
        techniqueUniforms["u_mv"] = "mv";
        techniqueUniforms["u_proj"] = "proj";

        auto& rootProgramNode = (tileData.m_json["programs"][s_programName] = Json::objectValue);
        rootProgramNode["attributes"] = Json::arrayValue;
        AppendProgramAttribute(rootProgramNode, "a_pos");
        AppendProgramAttribute(rootProgramNode, "a_direction");
        AppendProgramAttribute(rootProgramNode, "a_vertexDelta");

        if (doBatchIds)
            AppendProgramAttribute(rootProgramNode, "a_batchId");

        rootProgramNode["vertexShader"]   = s_vertexShaderName;
        rootProgramNode["fragmentShader"] = s_fragmentShaderName;

        auto& shaders = tileData.m_json["shaders"];
        AddShader (shaders, s_vertexShaderName, GLTF_VERTEX_SHADER, s_vertexShaderBufferViewName);
        AddShader (shaders, s_fragmentShaderName, GLTF_FRAGMENT_SHADER, s_fragmentShaderBufferViewName);
        
        std::string     vertexShaderString = s_shaderPrecision + (doBatchIds ? s_batchIdShaderAttribute : "") + s_polylineVertexShader;

        tileData.AddBufferView(s_vertexShaderBufferViewName, vertexShaderString);
        tileData.AddBufferView(s_fragmentShaderBufferViewName, s_polylineFragmentShader); 


        AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
        techniqueUniforms["u_color"] = "color";
        AddTechniqueParameter(technique, "width", GLTF_FLOAT, nullptr);
        techniqueUniforms["u_width"] = "width";
        AddTechniqueParameter(technique, "feather", GLTF_FLOAT, nullptr);
        techniqueUniforms["u_feather"] = "feather";

        addTransparencyToTechnique (technique);
        tileData.m_json["techniques"][s_techniqueName] = technique;
        }

    materialValue["technique"] = s_techniqueName;
    return materialName;
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
            tileData.AddBinaryData (quantizedValues.data(), dataSize = nValues * sizeof (unsigned short));
            break;
            }
    
        case VertexEncoding::UnquantizedDoubles:
            {
            bvector <float>     floatValues;

            accessor["componentType"] = GLTF_FLOAT;

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

            accessor["componentType"] = GLTF_UNSIGNED_BYTE;
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
    bufferViews["target"] = GLTF_ARRAY_BUFFER;

    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = nAttributes;
    accessor["type"] = accessorType;

    tileData.m_json["bufferViews"][bufferViewId] = bufferViews;
    tileData.m_json["accessors"][accessorId] = accessor;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshBatchIds (PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& batchIds, Utf8StringCR idStr)
    {
    Utf8String  bvBatchId        = Concat("bvBatch_", idStr),
                accBatchId       = Concat("accBatch_", idStr);

    primitive["attributes"]["BATCHID"] = accBatchId;

    auto nBatchIdBytes = batchIds.size() * sizeof(uint16_t);
    tileData.m_json["bufferViews"][bvBatchId] = Json::objectValue;
    tileData.m_json["bufferViews"][bvBatchId]["buffer"] = "binary_glTF";
    tileData.m_json["bufferViews"][bvBatchId]["byteOffset"] = tileData.BinaryDataSize();
    tileData.m_json["bufferViews"][bvBatchId]["byteLength"] = nBatchIdBytes;
    tileData.m_json["bufferViews"][bvBatchId]["target"] = GLTF_ARRAY_BUFFER;

    tileData.AddBinaryData (batchIds.data(), nBatchIdBytes);
    tileData.m_json["accessors"][accBatchId] = Json::objectValue;
    tileData.m_json["accessors"][accBatchId]["bufferView"] = bvBatchId;
    tileData.m_json["accessors"][accBatchId]["byteOffset"] = 0;
    tileData.m_json["accessors"][accBatchId]["componentType"] = GLTF_UNSIGNED_SHORT;
    tileData.m_json["accessors"][accBatchId]["count"] = batchIds.size();
    tileData.m_json["accessors"][accBatchId]["type"] = "SCALAR";
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
    tileData.m_json["bufferViews"][bvIndexId]["target"] =  GLTF_ELEMENT_ARRAY_BUFFER;

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
    tileData.m_json["accessors"][accIndexId]["componentType"] = useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32;
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
    bool            isTextured = false;

    primitive["material"] = AddMeshMaterial (tileData, isTextured, mesh.GetDisplayParams(), mesh, idStr.c_str(), doBatchIds);
    primitive["mode"] = GLTF_TRIANGLES;

    Utf8String      accPositionId =  AddMeshVertexAttributes (tileData, &mesh.Points().front().x, "Position", idStr.c_str(), 3, mesh.Points().size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
    primitive["attributes"]["POSITION"] = accPositionId;

    BeAssert (isTextured == !mesh.Params().empty());
    if (!mesh.Params().empty() && isTextured)
        {
        DRange3d        paramRange = DRange3d::From(mesh.Params(), 0.0);
        primitive["attributes"]["TEXCOORD_0"] = AddMeshVertexAttributes (tileData, &mesh.Params().front().x, "Param", idStr.c_str(), 2, mesh.Params().size(), "VEC2", VertexEncoding::StandardQuantization, &paramRange.low.x, &paramRange.high.x);
        }


    if (!mesh.Normals().empty() &&
        nullptr != mesh.GetDisplayParams() && !mesh.GetDisplayParams()->GetIgnoreLighting())        // No normals if ignoring lighting (reality meshes).
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

    Utf8String idStr(std::to_string(index).c_str());

    bvector<DPoint3d>           points, directions;
    bvector<DPoint3d> const&    meshPoints = mesh.Points();
    bvector<uint16_t>           attributes;
    bvector<uint32_t>           indices;
    bvector<DPoint3d>           vertexDeltas;
    static double               s_degenerateSegmentTolerance = 1.0E-5;

    BeAssert (mesh.Triangles().empty());        // Meshes should contain either triangles or polylines but not both.

    for (auto const& polyline : mesh.Polylines())
        {
        for (size_t i=0; i<polyline.m_indices.size()-1; i++)
            {
            DPoint3d        p0 = meshPoints[polyline.m_indices[i]], 
                            p1 = meshPoints[polyline.m_indices[i+1]];
            DVec3d          direction = DVec3d::FromStartEnd (p0, p1);
            bool            isStart  (i == 0),
                            isEnd    (i == polyline.m_indices.size()-2);
            if (direction.Magnitude() < s_degenerateSegmentTolerance)
                continue;

            indices.push_back(points.size());
            indices.push_back(points.size() + 2);
            indices.push_back(points.size() + 1);

            indices.push_back(points.size() + 1);
            indices.push_back(points.size() + 2);
            indices.push_back(points.size() + 3);

            points.push_back(p0);
            points.push_back(p0);
            points.push_back(p1);
            points.push_back(p1);

            for (size_t j=0; j<4; j++)
                {
                DPoint2d    delta;
                double      uParam;
                
                if (j < 2)
                    {
                    uParam = 0.0;
                    delta.x = isStart ? 0.0 : -1.0;
                    }
                else
                    {
                    uParam = 1.0;
                    delta.x = isEnd ? 0.0 : 1.0;
                    }
                
                delta.y = (0 == (j & 0x0001)) ? -1.0 : 1.0;
                vertexDeltas.push_back (DPoint3d::From(delta.x, delta.y, uParam));
                directions.push_back(direction);
                }

            if (mesh.ValidIdsPresent())
                {
                auto&   attribute0 = mesh.Attributes().at(polyline.m_indices[i]);
                auto&   attribute1 = mesh.Attributes().at(polyline.m_indices[i+1]);

                attributes.push_back (attribute0);
                attributes.push_back (attribute0);
                attributes.push_back (attribute1);
                attributes.push_back (attribute1);
                }
            }
        }

    Json::Value     primitive = Json::objectValue;
    DRange3d        pointRange = DRange3d::From(points), directionRange = DRange3d::From(directions);
    DRange3d        vertexDeltaRange = DRange3d::From (-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);

    primitive["material"] = AddPolylineMaterial (tileData, mesh.GetDisplayParams(), mesh, idStr.c_str(), mesh.ValidIdsPresent());
    primitive["mode"] = GLTF_TRIANGLES;

    Utf8String  accPositionId = AddMeshVertexAttributes (tileData, &points.front().x, "Position", idStr.c_str(), 3, points.size(), "VEC3", VertexEncoding::StandardQuantization, &pointRange.low.x, &pointRange.high.x);
    primitive["attributes"]["POSITION"]  = accPositionId;
    primitive["attributes"]["DIRECTION"] = AddMeshVertexAttributes (tileData, &directions.front().x, "Direction", idStr.c_str(), 3, directions.size(), "VEC3", VertexEncoding::StandardQuantization, &directionRange.low.x, &directionRange.high.x);
    primitive["attributes"]["VERTEXDELTA"]  = AddMeshVertexAttributes (tileData, &vertexDeltas.front().x, "VertexDelta", idStr.c_str(), 3, vertexDeltas.size(), "VEC3", VertexEncoding::StandardQuantization, &vertexDeltaRange.low.x, &vertexDeltaRange.high.x);
    primitive["indices"] = AddMeshIndices (tileData, "Index", indices, idStr);

    if (doBatchIds)
        AddMeshBatchIds(tileData, primitive, attributes, idStr);

    AddMeshPointRange(tileData.m_json["accessors"][accPositionId], pointRange);

    primitivesNode.append(primitive);
    tileData.m_json["buffers"]["binary_glTF"]["byteLength"] = tileData.BinaryDataSize();
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
    return nullptr != GetDgnDb().GeoLocation().GetDgnGCS();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::PublisherContext(DgnDbR db, DgnViewIdSet const& viewIds, BeFileNameCR outputDir, WStringCR tilesetName,  GeoPointCP geoLocation, bool publishSurfacesOnly, size_t maxTilesetDepth, TextureMode textureMode)
    : m_db(db), m_viewIds(viewIds), m_outputDir(outputDir), m_rootName(tilesetName), m_publishSurfacesOnly (publishSurfacesOnly), m_maxTilesetDepth (maxTilesetDepth), m_textureMode(textureMode)
    {
    // By default, output dir == data dir. data dir is where we put the json/b3dm files.
    m_outputDir.AppendSeparator();
    m_dataDir = m_outputDir;

    // ###TODO: Probably want a separate db-to-tile per model...will differ for non-spatial models...
    DPoint3d        origin = db.GeoLocation().GetProjectExtents().GetCenter();
    m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);

    DgnGCS*         dgnGCS = db.GeoLocation().GetDgnGCS();
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

    RotMatrix   rMatrix;
    rMatrix.InitIdentity();
    m_nonSpatialToEcef = Transform::From(rMatrix, ecfOrigin);

    DVec3d      zVector, yVector;

    zVector.Normalize ((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference (ecfNorth, ecfOrigin);

    rMatrix.SetColumn (yVector, 1);
    rMatrix.SetColumn (zVector, 2);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

    m_spatialToEcef =  Transform::From (rMatrix, ecfOrigin);
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
    if (dataDirExists && BeFileNameStatus::Success != BeFileName::EmptyDirectory(dataDir.c_str()))
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
        root[JSON_Content]["url"] = Utf8String(GetTileUrl(tile, tile.GetFileExtension().c_str()));
        TilePublisher::WriteBoundingVolume (root[JSON_Content], contentRange);
        }
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
    Json::Value val;

    val["asset"]["version"] = "0.0";

    auto&       root = val[JSON_Root];

    auto const& ecefTransform = rootTile.GetModel().IsSpatialModel() ? m_spatialToEcef : m_nonSpatialToEcef;
    if (!ecefTransform.IsIdentity())
        root[JSON_Transform] = TransformToJson(ecefTransform);

    DRange3d    rootRange;
    WriteMetadataTree (rootRange, root, rootTile, maxDepth);
    TileUtil::WriteJsonToFile (metadataFileName.c_str(), val);
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
            {
            BeMutexHolder lock(m_mutex);
            m_modelRanges[model.GetModelId()] = rootTile->GetTileRange();
            }

        WriteModelTileset(*rootTile);
        }
    else
        {
        CleanDirectories(GetDataDirForModel(model));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteModelTileset(TileNodeCR tile)
    {
    Json::Value root;
    root["refine"] = "replace";
    root[JSON_GeometricError] = tile.GetTolerance();
    TilePublisher::WriteBoundingVolume(root, tile.GetTileRange());

    WString modelRootName;
    BeFileName modelDataDir = GetDataDirForModel(tile.GetModel(), &modelRootName);

    BeFileName tilesetFileName(nullptr, nullptr, modelRootName.c_str(), s_metadataExtension);
    root[JSON_Content]["url"] = Utf8String(modelRootName + L"/" + tilesetFileName.c_str()).c_str();

    WriteTileset(BeFileName(nullptr, modelDataDir.c_str(), tilesetFileName.c_str(), nullptr), tile, GetMaxTilesetDepth());
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
PublisherContext::Status   PublisherContext::PublishViewModels (TileGeneratorR generator, DRange3dR rootRange, double toleranceInMeters, bool surfacesOnly, ITileGenerationProgressMonitorR progressMeter)
    {
    DgnModelIdSet viewedModels;
    for (auto const& viewId : m_viewIds)
        {
        SpatialViewDefinitionPtr spatialView = nullptr;
        auto drawingView = GetDgnDb().Elements().Get<DrawingViewDefinition>(viewId);
        if (drawingView.IsValid())
            {
            surfacesOnly = false;           // Always publish lines, text etc. in sheets.
            viewedModels.insert(drawingView->GetBaseModelId());
            }
        else if ((spatialView = GetDgnDb().Elements().GetForEdit<SpatialViewDefinition>(viewId)).IsValid())
            {
            auto spatialModels = spatialView->GetModelSelector().GetModels();
            viewedModels.insert(spatialModels.begin(), spatialModels.end());
            }
        }

    static size_t           s_maxPointsPerTile = 250000;
    auto status = generator.GenerateTiles(*this, viewedModels, toleranceInMeters, surfacesOnly, s_maxPointsPerTile);
    if (TileGeneratorStatus::Success != status)
        return ConvertStatus(status);

    rootRange = DRange3d::NullRange();
    for (auto const& kvp : m_modelRanges)
        rootRange.Extend(kvp.second);

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetModelsJson (DgnModelIdSet const& modelIds)
    {
    Json::Value     modelsJson (Json::objectValue);
    
    for (auto& modelId : modelIds)
        {
        auto const&  model = GetDgnDb().Models().GetModel (modelId);
        if (model.IsValid())
            {
            auto spatialModel = model->ToSpatialModel();
            auto drawingModel = nullptr == spatialModel ? dynamic_cast<DrawingModelCP>(model.get()) : nullptr;
            if (nullptr == spatialModel && nullptr == drawingModel)
                {
                BeAssert(false && "Unsupported model type");
                continue;
                }

            DRange3d modelRange;
            auto modelRangeIter = m_modelRanges.find(modelId);
            if (m_modelRanges.end() != modelRangeIter)
                modelRange = modelRangeIter->second;
            else
                modelRange = model->ToGeometricModel()->QueryModelRange(); // This gives a much larger range...

            if (modelRange.IsNull())
                {
                BeAssert(false && "Null model range");
                continue;
                }

            Json::Value modelJson(Json::objectValue);

            modelJson["name"] = model->GetName();
            modelJson["type"] = nullptr != spatialModel ? "spatial" : "drawing";


            auto const& modelTransform = nullptr != spatialModel ? m_spatialToEcef : m_nonSpatialToEcef;
            modelTransform.Multiply(modelRange, modelRange);
 			modelJson["extents"] = RangeToJson(modelRange);

            if (nullptr == spatialModel && !modelTransform.IsIdentity())
                modelJson["transform"] = TransformToJson(modelTransform);   // ###TODO? This may end up varying per model...

            // ###TODO: Shouldn't have to compute this twice...
            WString modelRootName;
            BeFileName modelDataDir = GetDataDirForModel(*model, &modelRootName);

            BeFileName relativePath (nullptr, m_rootName.c_str(), modelRootName.c_str(), nullptr);  // RootDir/ModelDir/
            relativePath.AppendToPath(modelRootName.c_str());                                       // RootDir/ModelDir/ModelName
            relativePath.AppendExtension(s_metadataExtension);                                      // RootDir/ModelDir/ModelName.json

            auto utf8FileName = relativePath.GetNameUtf8();
            utf8FileName.ReplaceAll("\\", "//");
            modelJson["tilesetUrl"] = utf8FileName;

            modelsJson[modelId.ToString()] = modelJson;
            }
        }

    return modelsJson;
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
    auto spatialView = view.ToSpatialView();
    auto drawingView = nullptr == spatialView ? view.ToDrawingView() : nullptr;
    if (nullptr != spatialView)
        {
        auto selectorId = spatialView->GetModelSelectorId().ToString();
        json["modelSelector"] = selectorId;
        }
    else if (nullptr != drawingView)
        {
        auto fakeModelSelectorId = drawingView->GetBaseModelId().ToString();
        fakeModelSelectorId.append("_2d");
        json["modelSelector"] = fakeModelSelectorId;
        }
    else
        {
        BeAssert(false && "Unexpected view type");
        return; // ###TODO sheets - should not end up here
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

    auto cameraView = nullptr != spatialView ? view.ToCameraView() : nullptr;
    if (nullptr != cameraView)
        {
        json["type"] = "camera";

        DPoint3d eyePoint = cameraView->GetEyePoint();
        transform.Multiply(eyePoint);
        json["eyePoint"] = PointToJson(eyePoint);

        json["lensAngle"] = cameraView->GetLensAngle();
        json["focusDistance"] = cameraView->GetFocusDistance();
        }
    else if (nullptr != spatialView)
        {
        json["type"] = "ortho";
        }
    else
        {
        json["type"] = "drawing";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId)
    {
    Utf8String rootNameUtf8(m_rootName.c_str());
    json["name"] = rootNameUtf8;

    Transform spatialTransform = Transform::FromProduct(m_spatialToEcef, m_dbToTile);
    Transform nonSpatialTransform = Transform::FromProduct(m_nonSpatialToEcef, m_dbToTile);

    if (!m_spatialToEcef.IsIdentity())
        {
        DPoint3d groundEcefPoint;
        spatialTransform.Multiply(groundEcefPoint, groundPoint);
        json["groundPoint"] = PointToJson(groundEcefPoint);
        }

    DgnElementIdSet allModelSelectors;
    DgnElementIdSet allCategorySelectors;
    DgnElementIdSet allDisplayStyles;
    DgnModelIdSet all2dModelIds;

    auto& viewsJson = (json["views"] = Json::objectValue);
    for (auto const& viewId : m_viewIds)
        {
        auto viewDefinition = ViewDefinition::Get(GetDgnDb(), viewId);
        if (!viewDefinition.IsValid())
            continue;

        auto spatialView = viewDefinition->ToSpatialView();
        auto drawingView = nullptr == spatialView ? viewDefinition->ToDrawingView() : nullptr;
        if (nullptr != spatialView)
            allModelSelectors.insert(spatialView->GetModelSelectorId());
        else if (nullptr != drawingView)
            all2dModelIds.insert(drawingView->GetBaseModelId());
        else
            continue;   // ###TODO: Sheets

        Json::Value entry(Json::objectValue);
 
        allCategorySelectors.insert(viewDefinition->GetCategorySelectorId());
        allDisplayStyles.insert(viewDefinition->GetDisplayStyleId());

        GetViewJson(entry, *viewDefinition, nullptr != spatialView ? spatialTransform : nonSpatialTransform);
        viewsJson[viewId.ToString()] = entry;

        // If for some reason the default view is not in the published set, we'll use the first view as the default
        if (!defaultViewId.IsValid())
            defaultViewId = viewId;
        }

    if (!defaultViewId.IsValid())
        return Status::NoGeometry;

    json["defaultView"] = defaultViewId.ToString();

    WriteModelsJson(json, allModelSelectors, all2dModelIds);
    WriteCategoriesJson(json, allCategorySelectors);
    json["displayStyles"] = GetDisplayStylesJson(allDisplayStyles);

    AxisAlignedBox3d projectExtents = GetDgnDb().GeoLocation().GetProjectExtents();
    spatialTransform.Multiply(projectExtents, projectExtents);
    json["projectExtents"] = RangeToJson(projectExtents);
    json["projectTransform"] = TransformToJson(m_spatialToEcef);
    
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


