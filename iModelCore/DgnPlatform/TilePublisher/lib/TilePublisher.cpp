/*--------------------------------------------------------------------------------------+                       
|
|     $Source: TilePublisher/lib/TilePublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/TilePublisher.h>
#include <BeSqLite/BeSqLite.h>
#include "Constants.h"
#include <Geom/OperatorOverload.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct  PublishTileData
{
    Json::Value         m_json;
    ByteStream          m_binaryData;

    size_t BinaryDataSize() const { return m_binaryData.size(); }
    void const* BinaryData() const { return m_binaryData.data(); }
    void AddBinaryData(void const* data, size_t size);
    void PadBinaryDataToBoundary(size_t boundarySize);
    template<typename T> void AddBufferView(Utf8CP name, T const& bufferData);
};


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
    struct ElemInfo { uint32_t m_index; uint32_t m_parentIndex; };
    struct AssemInfo { uint32_t m_index; uint32_t m_catIndex; };
    struct Assembly { DgnElementId m_elemId; DgnCategoryId m_catId; };

    enum ClassIndex
    {
        kClass_Primary,
        kClass_Construction,
        kClass_Dimension,
        kClass_Pattern,
        kClass_Element,
        kClass_Assembly,
        kClass_SubCategory,
        kClass_Category,

        kClass_COUNT,
        kClass_FEATURE_COUNT = kClass_Pattern+1,
    };

    static constexpr Utf8CP s_classNames[kClass_COUNT] =
        {
        "Primary", "Construction", "Dimension", "Pattern", "Element", "Assembly", "SubCategory", "Category"
        };

    Json::Value                         m_json; // "HIERARCHY": object
    DgnDbR                              m_db;
    FeatureAttributesMapCR              m_attrs;
    bmap<DgnElementId, ElemInfo>        m_elems;
    bmap<DgnElementId, AssemInfo>       m_assemblies;
    bmap<DgnSubCategoryId, uint32_t>    m_subcats;
    bmap<DgnCategoryId, uint32_t>       m_cats;
    DgnCategoryId                       m_uncategorized;
    bool                                m_is3d;

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

    ElemInfo MapElementInfo(DgnElementId id);
    ElemInfo GetElementInfo(DgnElementId id);
    AssemInfo MapAssemblyInfo(Assembly assem);
    uint32_t MapCategoryIndex(DgnCategoryId id) { return FindOrInsert(m_cats, id); }
    uint32_t GetCategoryIndex(DgnCategoryId id) { return GetIndex(m_cats, id); }
    uint32_t MapSubCategoryIndex(DgnSubCategoryId id) { return FindOrInsert(m_subcats, id); }
    uint32_t GetSubCategoryIndex(DgnSubCategoryId id) { return GetIndex(m_subcats, id); }

    Json::Value& GetClass(ClassIndex idx) { return m_json["classes"][idx]; }
    static ClassIndex GetFeatureClassIndex(DgnGeometryClass geomClass);

    Assembly QueryAssembly(DgnElementId) const;
    void DefineClasses();
    void MapFeatures();
    void MapParents();
    void MapElements(uint32_t offset, uint32_t assembliesOffset);
    void MapAssemblies(uint32_t offset, uint32_t categoriesOffset);
    void MapSubCategories(uint32_t offset);
    void MapCategories(uint32_t offset);

    void Build();
    void InitUncategorizedCategory();
    bool IsUncategorized(DgnCategoryId id) const { return id.IsValid() && id == m_uncategorized; }
public:
    BatchTableBuilder(FeatureAttributesMapCR attrs, DgnDbR db, bool is3d)
        : m_json(Json::objectValue), m_db(db), m_attrs(attrs), m_is3d(is3d)
        {
        InitUncategorizedCategory();
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
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::InitUncategorizedCategory()
    {
    // This is dumb. See OfficeBuilding.dgn - cells have no level in V8, which translates to 'Uncategorized' (2d and 3d variants) in DgnDb
    // We don't want to create an 'Uncategorized' assembly if its children belong to a real category.
    // We only can detect this because for whatever reason, "Uncategorized" is not a localized string.
    DefinitionModelR dictionary = m_db.GetDictionaryModel();
    DgnCode code = m_is3d ? SpatialCategory::CreateCode(dictionary, "Uncategorized") : DrawingCategory::CreateCode(dictionary, "Uncategorized");
    m_uncategorized = DgnCategory::QueryCategoryId(m_db, code);
    }

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
void BatchTableBuilder::MapSubCategories(uint32_t offset)
    {
    Json::Value& subcats = GetClass(kClass_SubCategory);
    subcats["length"] = m_subcats.size();

    Json::Value &instances = (subcats["instances"] = Json::objectValue),
                &subcat_id = (instances["subcategory"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"];

    for (auto const& kvp : m_subcats)
        {
        classIds[offset + kvp.second] = kClass_SubCategory;
        parentCounts[offset + kvp.second] = 0;
        subcat_id[kvp.second] = kvp.first.ToString();
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
void BatchTableBuilder::MapElements(uint32_t offset, uint32_t assembliesOffset)
    {
    Json::Value& elements = GetClass(kClass_Element);
    elements["length"] = m_elems.size();

    Json::Value& instances = (elements["instances"] = Json::objectValue);
    Json::Value& elem_id = (instances["element"] = Json::arrayValue);
    Json::Value& classIds = m_json["classIds"];
    Json::Value& parentCounts = m_json["parentCounts"];
    Json::Value& parentIds = m_json["parentIds"];

    for (auto const& kvp : m_elems)
        {
        Json::Value::ArrayIndex index = kvp.second.m_index;
        elem_id[index] = kvp.first.ToString();

        classIds[offset + index] = kClass_Element;
        parentCounts[offset + index] = 1;
        parentIds.append(kvp.second.m_parentIndex + assembliesOffset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapAssemblies(uint32_t offset, uint32_t categoriesOffset)
    {
    Json::Value& assems = GetClass(kClass_Assembly);
    assems["length"] = m_assemblies.size();

    Json::Value &instances = (assems["instances"] = Json::objectValue),
                &assem_id = (instances["assembly"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"],
                &parentIds = m_json["parentIds"];

    for (auto const& kvp : m_assemblies)
        {
        Json::Value::ArrayIndex index = kvp.second.m_index;
        classIds[offset+index] = kClass_Assembly;
        assem_id[index] = kvp.first.ToString();
        parentCounts[offset+index] = 1;
        parentIds.append(kvp.second.m_catIndex + categoriesOffset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapParents()
    {
    uint32_t elementsOffset = static_cast<uint32_t>(m_attrs.size());
    uint32_t assembliesOffset = elementsOffset + static_cast<uint32_t>(m_elems.size());
    uint32_t subcatsOffset = assembliesOffset + static_cast<uint32_t>(m_assemblies.size());
    uint32_t catsOffset = subcatsOffset + static_cast<uint32_t>(m_subcats.size());
    uint32_t totalInstances = catsOffset + static_cast<uint32_t>(m_cats.size());

    m_json["instancesLength"] = totalInstances;

    // Now that every instance of every class has an index into "classIds", we can map parent IDs
    Json::Value& parentIds = m_json["parentIds"];
    for (auto const& kvp : m_attrs)
        {
        FeatureAttributesCR attr = kvp.first;
        uint32_t index = kvp.second * 2; // 2 parents per feature

        parentIds[index] = elementsOffset + GetElementInfo(attr.GetElementId()).m_index;
        parentIds[index+1] = subcatsOffset + GetSubCategoryIndex(attr.GetSubCategoryId());
        }

    // Set "instances" and "length" to Element class, and add elements to "classIds" and assemblies to "parentIds"
    MapElements(elementsOffset, assembliesOffset);
    MapAssemblies(assembliesOffset, catsOffset);

    // Set "instances" and "length" to SubCategory class, and add subcategories to "classIds" and "parentIds"
    MapSubCategories(subcatsOffset);
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
        MapElementInfo(attr.GetElementId());
        MapSubCategoryIndex(attr.GetSubCategoryId());
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
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::MapElementInfo(DgnElementId id) -> ElemInfo
    {
    auto iter = Find(m_elems, id);
    if (iter != m_elems.end())
        return iter->second;

    Assembly assem = QueryAssembly(id);
    ElemInfo info;
    info.m_index = static_cast<uint32_t>(m_elems.size());
    info.m_parentIndex = MapAssemblyInfo(assem).m_index;
    m_elems[id] = info;
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::MapAssemblyInfo(Assembly assem) -> AssemInfo
    {
    auto iter = Find(m_assemblies, assem.m_elemId);
    if (iter != m_assemblies.end())
        return iter->second;

    AssemInfo info;
    info.m_index = static_cast<uint32_t>(m_assemblies.size());
    info.m_catIndex = MapCategoryIndex(assem.m_catId);
    m_assemblies[assem.m_elemId] = info;
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::GetElementInfo(DgnElementId id) -> ElemInfo
    {
    auto iter = Find(m_elems, id);
    BeAssert(iter != m_elems.end());
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::QueryAssembly(DgnElementId childId) const -> Assembly
    {
    Assembly assem;
    assem.m_elemId = childId;
    if (!childId.IsValid())
        return assem;

    // Get this element's category and parent element
    // Recurse until no more parents (or we find a non-geometric parent)
    static constexpr Utf8CP s_3dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
    static constexpr Utf8CP s_2dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE ECInstanceId=?";

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db.GetPreparedECSqlStatement(m_is3d ? s_3dsql : s_2dsql);
    stmt->BindId(1, childId);

    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto thisCatId = stmt->GetValueId<DgnCategoryId>(1);
        if (assem.m_catId.IsValid() && IsUncategorized(thisCatId) && !IsUncategorized(assem.m_catId))
            break; // yuck. if have children with valid categories, stop before first uncategorized parent (V8 complex header).

        assem.m_catId = thisCatId;
        assem.m_elemId = childId;

        childId = stmt->GetValueId<DgnElementId>(0);
        if (!childId.IsValid())
            break;

        // Try to get the parent's category. If parent is not geometric, this will fail and we will treat current child as the assembly root.
        stmt->Reset();
        stmt->BindId(1, childId);
        }

    BeAssert(assem.m_catId.IsValid());
    return assem;
    }

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
    PublishableTileGeometry publishableGeometry = m_tile.GeneratePublishableGeometry(m_context.GetDgnDb(), TileGeometry::NormalMode::Always, m_context.WantSurfacesOnly(), nullptr);

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
    bool invalidIdsPresent = false;
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
        bool isValidId = (0 != attributeIndices.back());
        validIdsPresent |= isValidId;
        invalidIdsPresent |= !isValidId;
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
        }

    BatchTableBuilder batchTableBuilder(attributesSet, m_context.GetDgnDb(), m_tile.GetModel().Is3d());
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
    bool                validIdsPresent = false;

    for (auto& mesh : meshes)
        if (mesh->ValidIdsPresent())
            validIdsPresent = true;

    AddExtensions(tileData);
    AddDefaultScene(tileData);
    AddMeshes(tileData, meshes);
                         
    FeatureAttributesMapCR attributes = m_tile.GetAttributes();
    Utf8String batchTableStr;
    if (validIdsPresent)
        {
        BatchTableBuilder batchTableBuilder(attributes, m_context.GetDgnDb(), m_tile.GetModel().Is3d());
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
    tileData.m_json["textures"][textureId]["format"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    tileData.m_json["textures"][textureId]["internalFormat"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
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
    texture["format"] = GLTF_RGBA;
    texture["internalFormat"] = GLTF_RGBA;
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

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (!mat.IgnoresLighting())
        {
        AddTechniqueParameter(technique, "n", GLTF_INT_VEC2, "NORMAL");
        AddTechniqueParameter(technique, "nmx", GLTF_FLOAT_MAT3, "MODELVIEWINVERSETRANSPOSE");
        }

    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    if (!mat.IsTextured())
        AddTechniqueParameter(technique, "colorIndex", GLTF_FLOAT, "_COLORINDEX");

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
    AddShader(shaders, vertexShader.c_str(), GLTF_VERTEX_SHADER, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), GLTF_FRAGMENT_SHADER, fragmentShaderBufferView.c_str());

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

    if (params.GetMaterialId().IsValid())
        {
        m_material = DgnMaterial::Get(db, params.GetMaterialId());
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
                m_alphaOverride = 1.0;
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
        TilePublisher::AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        TilePublisher::AddTechniqueParameter(technique, "colorIndex", GLTF_FLOAT, "_COLORINDEX");

        techniqueUniforms["u_tex"] = "tex";
        techniqueUniforms["u_texStep"] = "texStep";

        technique["attributes"]["a_colorIndex"] = "colorIndex";
        TilePublisher::AppendProgramAttribute(program, "a_colorIndex");

        auto& sampler = data.m_json["samplers"]["sampler_1"];
        sampler["minFilter"] = GLTF_NEAREST;
        sampler["maxFilter"] = GLTF_NEAREST;
        sampler["wrapS"] = GLTF_CLAMP_TO_EDGE;
        sampler["wrapT"] = GLTF_CLAMP_TO_EDGE;

        if (TilePublish::ColorIndex::Dimension::Two == dim)
            {
            TilePublisher::AddTechniqueParameter(technique, "texWidth", GLTF_FLOAT, nullptr);
            TilePublisher::AddTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC4, nullptr);

            techniqueUniforms["u_texWidth"] = "texWidth";
            }
        else
            {
            TilePublisher::AddTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC2, nullptr);
            }
        }
    else
        {
        TilePublisher::AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
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
        TilePublisher::AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        TilePublisher::AddTechniqueParameter(technique, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");

        data.m_json["samplers"]["sampler_0"] = Json::objectValue;
        data.m_json["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;
        if (!m_texture->GetRepeat())
            {
            data.m_json["samplers"]["sampler_0"]["wrapS"] = GLTF_CLAMP_TO_EDGE;
            data.m_json["samplers"]["sampler_0"]["wrapT"] = GLTF_CLAMP_TO_EDGE;
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
        TilePublisher::AddTechniqueParameter(technique, "specularColor", GLTF_FLOAT_VEC3, nullptr);
        technique["uniforms"]["u_specularColor"] = "specularColor";

        TilePublisher::AddTechniqueParameter(technique, "specularExponent", GLTF_FLOAT, nullptr);
        technique["uniforms"]["u_specularExponent"] = "specularExponent";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    TilePublisher::AddMaterialColor(Json::Value& matJson, TileMaterial& mat, PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix)
    {
    auto dim = TileColorIndex::CalcDimension(mesh.GetColorTable().GetNumIndices());

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

    auto matId = mesh.GetDisplayParams().GetMaterialId();
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
    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    auto& enableStates = technique["states"]["enable"] = Json::arrayValue;
    enableStates.append(GLTF_DEPTH_TEST);

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
    AddShader(shaders, vertexShaderName.c_str(), GLTF_VERTEX_SHADER, vertexShaderBufferViewName.c_str());
    AddShader(shaders, fragmentShaderName.c_str(), GLTF_FRAGMENT_SHADER, fragmentShaderBufferViewName.c_str());

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
        AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
        AddTechniqueParameter(technique, "width", GLTF_FLOAT, nullptr);
        technique["uniforms"]["u_color"] = "color";
        technique["uniforms"]["u_width"] = "width";

        AddTechniqueParameter(technique, "prev", GLTF_FLOAT_VEC3, "PREV");
        AddTechniqueParameter(technique, "next", GLTF_FLOAT_VEC3, "NEXT");
        AddTechniqueParameter(technique, "param", GLTF_FLOAT_VEC2, "PARAM");


        technique["attributes"]["a_prev"] = "prev";
        technique["attributes"]["a_next"] = "next";
        technique["attributes"]["a_param"] = "param";
        AppendProgramAttribute(programRoot, "a_prev");
        AppendProgramAttribute(programRoot, "a_next");
        AppendProgramAttribute(programRoot, "a_param");
        }

    if (mat.IsTextured())
        {
        AddTechniqueParameter(technique, "texLength", GLTF_FLOAT, nullptr);
        technique["uniforms"]["u_texLength"] = "texLength";

        AddTechniqueParameter(technique, "texScalePnt", GLTF_FLOAT_VEC3, "TEXSCALEPNT");
        technique["attributes"]["a_texScalePnt"] = "texScalePnt";
        AppendProgramAttribute(programRoot, "a_texScalePnt");

        technique["attributes"]["a_distance"] = "distance";
        AppendProgramAttribute(programRoot, "a_distance");
        AddTechniqueParameter(technique, "distance", GLTF_FLOAT, "DISTANCE");
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
    auto componentType = GLTF_UNSIGNED_BYTE;
    for (auto attribute : attributes16)
        {
        if (attribute > 0xff)
            {
            componentType = GLTF_UNSIGNED_SHORT;
            break;
            }
        }

    size_t nBytes = attributes16.size() * sizeof(uint16_t);
    if (GLTF_UNSIGNED_BYTE == componentType)
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
    bv["target"] = GLTF_ARRAY_BUFFER;

    if (GLTF_UNSIGNED_BYTE == componentType)
        tileData.AddBinaryData(attributes8.data(), nBytes);
    else
        tileData.AddBinaryData(attributes16.data(), nBytes);

    auto& acc = tileData.m_json["accessors"][accId];
    acc["bufferView"] = bvId;
    acc["byteOffset"] = 0;
    acc["componentType"] = componentType;
    acc["count"] = attributes16.size();
    acc["type"] = "SCALAR";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TilePublisher::AddMeshBatchIds (PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& batchIds, Utf8StringCR idStr)
    {
    AddMeshUInt16Attributes(tileData, primitive, batchIds, idStr, "Batch_", "BATCHID");
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

    MeshMaterial meshMat = AddMeshMaterial(tileData, mesh, idStr.c_str(), doBatchIds);
    primitive["material"] = meshMat.GetName();
    primitive["mode"] = GLTF_TRIANGLES;

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
    bvector<uint16_t>           m_attributes;
    bvector<uint16_t>           m_colors;
    bvector<uint32_t>           m_indices;

    /* Delta contains the following parameters.
        x - the distance along the curve (used for parameterization of textures (0 - length).
        y - direction to project along perpendicular (-1 or 1).
        z - 0.0 == start, 4.0 == end.  (1-2) z-1 == start joint param. (5-6) z-5 == end joint params).  Joint parameter is the interpolation factor between perpendicular and bisector. */

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddPoint (DPoint3dCR point, DVec3dCR prev, DVec3dCR next, double delta, DVec2dCR param, uint16_t attrib, uint16_t color, DPoint3dCR center)
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
    void AddJointTriangles(uint32_t index, double length, DPoint3dCR p, DVec3dCR prev, DVec3dCR next, uint16_t attribute, uint16_t color, double param, DPoint3dCR center)
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
static void gatherPolyline(bvector<DPoint3d>& polylinePoints,  bvector<uint16_t>&  polylineColors, bvector<uint16_t>& polylineAttributes, TilePolylineCR polyline, TileMeshCR mesh, bool doColors, bool doBatchIds)
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
        bvector<uint16_t>       polylineColors, polylineAttributes;

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
            uint16_t            colors0 = 0, colors1 = 0, attributes0 = 0, attributes1 = 1;
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

    Json::Value     primitive = Json::objectValue;
    DRange3d        pointRange = DRange3d::From(tesselation.m_points), paramRange = DRange3d::From(tesselation.m_params, 0.0);

    primitive["material"] = mat.GetName();
    primitive["mode"] = GLTF_TRIANGLES;

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
    bvector<uint16_t>           attributes, colors;
    bvector<uint32_t>           indices;
    bvector<double>             distances;
    double                      minLength = 0.0, maxLength = 0.0;
    bool                        doColors = TilePublish::ColorIndex::Dimension::Zero != mat.GetColorIndexDimension();

    for (auto const& polyline : mesh.Polylines())
        {
        bvector<DPoint3d>       polylinePoints;
        bvector<uint16_t>       polylineColors, polylineAttributes;

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

    Json::Value     primitive = Json::objectValue;
    DRange3d        pointRange = DRange3d::From(points);

    primitive["material"] = mat.GetName();
    primitive["mode"] = GLTF_LINES;

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
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublisherContext::GetViewAttachmentsJson(Sheet::ModelCR sheet)
    {
    bvector<DgnElementId> attachmentIds = sheet.GetSheetAttachmentIds();
    Json::Value attachmentsJson(Json::arrayValue);
    for (DgnElementId attachmentId : attachmentIds)
        {
        auto attachment = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachmentId);
        DrawingViewDefinitionCPtr view = attachment.IsValid() ? GetDgnDb().Elements().Get<DrawingViewDefinition>(attachment->GetAttachedViewId()) : nullptr;
        if (view.IsNull())
            continue;

        Json::Value viewJson;
        viewJson["baseModelId"] = view->GetBaseModelId().ToString();
        viewJson["categorySelector"] = view->GetCategorySelectorId().ToString();
        viewJson["displayStyle"] = view->GetDisplayStyleId().ToString();

        DPoint3d            viewOrigin = view->GetOrigin();
        AxisAlignedBox3d    sheetRange = attachment->GetPlacement().CalculateRange();
        double              sheetScale = sheetRange.XLength() / view->GetExtents().x;
        Transform           subtractViewOrigin = Transform::From(DPoint3d::From(-viewOrigin.x, -viewOrigin.y, -viewOrigin.z)),
                            viewRotation = Transform::From(view->GetRotation()),
                            scaleToSheet = Transform::FromScaleFactors (sheetScale, sheetScale, sheetScale),
                            addSheetOrigin = Transform::From(DPoint3d::From(sheetRange.low.x, sheetRange.low.y, attachment->GetDisplayPriority()/500.0)),
                            tileToSheet = Transform::FromProduct(Transform::FromProduct(addSheetOrigin, scaleToSheet), Transform::FromProduct(viewRotation, subtractViewOrigin)),
                            sheetToTile;

        sheetToTile.InverseOf(tileToSheet);
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

    WString         rootName;
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
                WString     metadataRelativePath = rootName + L"/" + childTile->GetFileName(rootName.c_str(), s_metadataExtension);
                BeFileName  metadataFileName (nullptr, m_dataDir.c_str(), metadataRelativePath.c_str(), nullptr);

                WriteModelMetadataTree (childRange, childRoot, *childTile, GetMaxTilesetDepth());
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

    root["refine"] = "replace";
    root[JSON_GeometricError] = tile.GetTolerance();
    TilePublisher::WriteBoundingVolume(root, range);

    if (!contentRange.IsNull() && !tile.GetIsEmpty())
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
    Json::Value val, modelRoot;

    val["asset"]["version"] = "0.0";
    val["asset"]["gltfUpAxis"] = "Z";
 
    DRange3d    rootRange;
    WriteModelMetadataTree (rootRange, modelRoot, rootTile, maxDepth);

    val[JSON_Root] = std::move(modelRoot);

    if (rootTile.GetModel().IsSpatialModel())
        val[JSON_Root][JSON_Transform] = TransformToJson(m_spatialToEcef);

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
    if (TileGeneratorStatus::Success == status && nullptr != rootTile && (!rootTile->GetIsEmpty() || !rootTile->GetChildren().empty()))
        {
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
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName PublisherContext::GetModelTilesetName(DgnModelCR model)
    {
    return BeFileName(nullptr, m_dataDir.c_str(), TileUtil::GetRootNameForModel(model).c_str(), s_metadataExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::WriteModelTileset(TileNodeCR tile)
    {
    WriteTileset(GetModelTilesetName(tile.GetModel()), tile, GetMaxTilesetDepth());
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
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::AddViewedModel(DgnModelIdSet& viewedModels, DgnModelId modelId)
    {
    viewedModels.insert(modelId);

    // Scan for viewAttachments...
    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, modelId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto attach   = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachId);

        if (!attach.IsValid())
            {
            BeAssert(false);
            continue;
            }

        GetViewedModelsFromView (viewedModels, attach->GetAttachedViewId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    PublisherContext::GetViewedModelsFromView (DgnModelIdSet& viewedModels, DgnViewId viewId)
    {
    SpatialViewDefinitionPtr spatialView = nullptr;
    auto view2d = GetDgnDb().Elements().Get<ViewDefinition2d>(viewId);
    if (view2d.IsValid())
        {
        AddViewedModel (viewedModels, view2d->GetBaseModelId()); 
        }
    else if ((spatialView = GetDgnDb().Elements().GetForEdit<SpatialViewDefinition>(viewId)).IsValid())
        {
        for (auto& modelId : spatialView->GetModelSelector().GetModels())
            AddViewedModel (viewedModels, modelId);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status   PublisherContext::PublishViewModels (TileGeneratorR generator, DRange3dR rootRange, double toleranceInMeters, bool surfacesOnly, ITileGenerationProgressMonitorR progressMeter)
    {
    DgnModelIdSet viewedModels;

    for (auto const& viewId : m_viewIds)
        GetViewedModelsFromView (viewedModels, viewId);

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
            auto model2d = nullptr == spatialModel ? dynamic_cast<GraphicalModel2dCP>(model.get()) : nullptr;
            if (nullptr == spatialModel && nullptr == model2d)
                {
                BeAssert(false && "Unsupported model type");
                continue;
                }

            auto modelRangeIter = m_modelRanges.find(modelId);
            if (m_modelRanges.end() == modelRangeIter)
                continue; // this model produced no tiles. ignore it.

            DRange3d modelRange = modelRangeIter->second;
            if (modelRange.IsNull())
                {
                BeAssert(false && "Null model range");
                continue;
                }

            Json::Value modelJson(Json::objectValue);

            auto sheetModel = model->ToSheetModel();
            modelJson["name"] = model->GetName();
            modelJson["type"] = nullptr != spatialModel ? "spatial" : (nullptr != sheetModel ? "sheet" : "drawing");

            if (nullptr != spatialModel)
                {
                m_spatialToEcef.Multiply(modelRange, modelRange);
                modelJson["transform"] = TransformToJson(m_spatialToEcef);
                }
            else if (nullptr != sheetModel)
                {
                modelJson["attachedViews"] = GetViewAttachmentsJson(*sheetModel);
                }

            modelJson["extents"] = RangeToJson(modelRange);

            // ###TODO: Shouldn't have to compute this twice...
            WString modelRootName = TileUtil::GetRootNameForModel(*model);

            BeFileName tilesetFileName (nullptr, m_rootName.c_str(), modelRootName.c_str(), s_metadataExtension);

            auto utf8FileName = tilesetFileName.GetNameUtf8();
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
        auto const& category = DgnCategory::Get(GetDgnDb(), categoryId);

        if (category.IsValid())
            categoryJson[categoryId.ToString()] = category->GetCategoryName();
        }

    return categoryJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PublisherContext::GetViewJson(Json::Value& json, ViewDefinitionCR view, TransformCR transform)
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
        auto fakeModelSelectorId = view2d->GetBaseModelId().ToString();
        fakeModelSelectorId.append("_2d");
        json["modelSelector"] = fakeModelSelectorId;
        }
    else
        {
        BeAssert(false && "Unexpected view type");
        return;
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status PublisherContext::GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId)
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

    DgnElementIdSet allModelSelectors;
    DgnElementIdSet allCategorySelectors;
    DgnElementIdSet allDisplayStyles;
    DgnModelIdSet   all2dModelIds;

    auto& viewsJson = (json["views"] = Json::objectValue);
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
                    allCategorySelectors.insert(attachedView->GetCategorySelectorId());
                    allDisplayStyles.insert(attachedView->GetDisplayStyleId());
                    if (nullptr != attachedView->ToView2d())
                        all2dModelIds.insert(attachedView->ToView2d()->GetBaseModelId());
                    }
                }
            }

        Json::Value entry(Json::objectValue);
 
        allCategorySelectors.insert(viewDefinition->GetCategorySelectorId());
        allDisplayStyles.insert(viewDefinition->GetDisplayStyleId());

        GetViewJson(entry, *viewDefinition, nullptr != spatialView ? spatialTransform : Transform::FromIdentity());
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

    json["viewFlags"] = style.GetViewFlags().ToJson();

    auto style3d = style.ToDisplayStyle3d();
    if (nullptr != style3d)
        json["isGlobeVisible"] = style3d->IsGroundPlaneEnabled();

    return json;
    }


