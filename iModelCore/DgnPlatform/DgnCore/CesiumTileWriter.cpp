/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/CesiumTileWriter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>
#include <folly/BeFolly.h>

#include "../TilePublisher/lib/Constants.h" // ###TODO: Move this stuff.


USING_NAMESPACE_TILETREE
USING_NAMESPACE_TILEWRITER
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       

static double s_minToleranceRatio = 512.0;

BEGIN_TILEWRITER_NAMESPACE

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
    FeatureTableCR                      m_featureTable;
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
    BatchTableBuilder(FeatureTableCR featureTable, DgnDbR db, bool is3d)
        : m_json(Json::objectValue), m_db(db), m_featureTable(featureTable), m_is3d(is3d)
        {
        InitUncategorizedCategory();
        Build();
        }

    Json::Value const& GetHierarchy() const { return m_json; }

    Utf8String ToString() const
        {
        Json::Value json;
        json["HIERARCHY"] = GetHierarchy();
        return getJsonString(json);
        }
};

// So dumb that this is required by linker...
constexpr Utf8CP BatchTableBuilder::s_classNames[];

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
    uint32_t elementsOffset = static_cast<uint32_t>(m_featureTable.size());
    uint32_t assembliesOffset = elementsOffset + static_cast<uint32_t>(m_elems.size());
    uint32_t subcatsOffset = assembliesOffset + static_cast<uint32_t>(m_assemblies.size());
    uint32_t catsOffset = subcatsOffset + static_cast<uint32_t>(m_subcats.size());
    uint32_t totalInstances = catsOffset + static_cast<uint32_t>(m_cats.size());

    m_json["instancesLength"] = totalInstances;

    // Now that every instance of every class has an index into "classIds", we can map parent IDs
    Json::Value& parentIds = m_json["parentIds"];
    for (auto const& kvp : m_featureTable)
        {
        FeatureCR attr = kvp.first;
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

    for (auto const& kvp : m_featureTable)
        {
        FeatureCR attr = kvp.first;
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



/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct MeshMaterial 
{ 
    Utf8String  m_name;

    MeshMaterial (Utf8StringCR suffix) : m_name("Material_" + suffix) { }

    bool IsTextured() const { return false; }
    bool IgnoresLighting() const { return false; }
    Utf8StringCR GetName() const { return m_name; }


};  // MeshMaterial


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct CesiumTileWriter : TileWriter::Writer
{
    CesiumTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : TileWriter::Writer(streamBuffer, model) { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, TriMeshArgs const& meshArgs, MeshMaterial const& meshMaterial, Utf8StringCR idStr)
    {
    primitiveJson["mode"] = GLTF_TRIANGLES;

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(meshArgs.m_points, meshArgs.m_numPoints, meshArgs.m_pointParams, "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    bool isTextured = meshMaterial.IsTextured();
    if (nullptr != meshArgs.m_textureUV && isTextured)
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (meshArgs.m_textureUV, meshArgs.m_numPoints, "Param", idStr.c_str());
    if (meshArgs.m_colors.m_numColors > 1)
        AddColors(primitiveJson, meshArgs.m_colors, meshArgs.m_numPoints, idStr);

    BeAssert (meshMaterial.IgnoresLighting() || nullptr != meshArgs.m_normals);

    if (nullptr != meshArgs.m_normals && !meshMaterial.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"] = AddNormals(meshArgs.m_normals, meshArgs.m_numPoints, "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshIndices ("Indices", (uint32_t const*) meshArgs.m_vertIndex, meshArgs.m_numIndices, idStr, meshArgs.m_numPoints);
    AddMeshPointRange(m_json["accessors"][accPositionId], meshArgs.m_pointParams.GetRange());

    return SUCCESS;
    }
                                                                                     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTriMesh(Json::Value& primitivesNode, TriMeshArgsCR meshArgs, ColorTableCR colorTable, MeshMaterial const& meshMaterial, size_t& index)
    {
    if (0 == meshArgs.m_numIndices)
        return;

    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         materialJson = Json::objectValue, primitiveJson = Json::objectValue;

    if (//SUCCESS == CreateMeshMaterialJson(materialJson, colorTable, meshMaterial, idStr) &&
        SUCCESS == CreateTriMesh(primitiveJson, meshArgs, meshMaterial, idStr))
        {
#ifdef NONDEFAULT_MATERIALS
        m_json["materials"][meshMaterial.GetName()] = materialJson;
#endif
        primitiveJson["material"] = meshMaterial.GetName();
        primitivesNode.append(primitiveJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginBatchedModel(uint32_t& startPosition, uint32_t& lengthDataPosition, Render::FeatureTableCR featureTable)
    {
    Utf8String          batchTableStr = BatchTableBuilder (featureTable, m_model.GetDgnDb(), m_model.Is3d()).ToString();
    Json::Value         featureTableJson;

    featureTableJson["BATCH_LENGTH"] = featureTable.size();
    Utf8String      featureTableStr = getJsonString(featureTableJson);

    startPosition = m_buffer.GetSize();
    m_buffer.Append((const uint8_t *) s_b3dmMagic, 4);
    m_buffer.Append(s_b3dmVersion);                                                          
    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);                                                      // total length - filled in later.
    m_buffer.Append(static_cast<uint32_t>(featureTableStr.size()));                        // feature table JSon length.
    m_buffer.Append((uint32_t) 0);                                                      // length of binary portion of feature table (zero).
    m_buffer.Append(static_cast<uint32_t>(batchTableStr.size()));                       // batch table JSon length.
    m_buffer.Append((uint32_t) 0);                                                      // length of binary portion of batch table (zero)
    m_buffer.Append((const uint8_t *) featureTableStr.data(), featureTableStr.size());  // Feature table Json.
    m_buffer.Append((const uint8_t *) batchTableStr.data(), batchTableStr.size());      // Batch table Json.

    PadToBoundary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void EndBatchedModel(uint32_t startPosition, uint32_t lengthDataPosition)
    {
    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  WriteBatchedModel(Render::FeatureTableCR featureTable, DPoint3dCR centroid)
    {
    uint32_t       startPosition = 0, lengthDataPosition = 0;

    BeginBatchedModel(startPosition, lengthDataPosition, featureTable);
    WriteGltf(centroid);
    EndBatchedModel(startPosition, lengthDataPosition);

    return SUCCESS;
    }


};  // CesiumTileWriter

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct RenderSystem : Render::System
{
    static  constexpr uint32_t          s_maxFeatures = 0xffff;

    mutable StreamBuffer                m_streamBuffer;
    mutable CesiumTileWriter            m_writer;
    mutable Json::Value                 m_primitives;
    mutable DRange3d                    m_range;
    mutable FeatureTable               m_featureTable;

    RenderSystem(GeometricModelR model) : m_writer(m_streamBuffer, model), m_range(DRange3d::NullRange()), m_featureTable(s_maxFeatures)  { }

    virtual MaterialPtr _GetMaterial(RenderMaterialId, DgnDbR) const override { return nullptr; }
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { return nullptr; }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual Render::TargetPtr _CreateOffscreenTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateVisibleEdges(MeshEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateSilhouetteEdges(SilhouetteEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return new Graphic(dgndb); }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return s_maxFeatures; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, Render::Texture::CreateParams const& params) const override {return new TileTexture(image, params);}
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, Texture::CreateParams const& params) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override  { return nullptr; }
    virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb)  const override {return nullptr; }
    virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const override { return nullptr; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& featureTable) const override 
    {
    m_featureTable = std::move(featureTable);
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const override
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR triMesh, DgnDbR db) const override 
    {
    ColorTable      colorTable;
    size_t          index = (size_t) m_primitives.size();
    Utf8String      idStr(std::to_string(index).c_str());
    MeshMaterial    meshMaterial(idStr);

    m_range.Extend(triMesh.m_pointParams.GetRange());
    m_writer.AddTriMesh(m_primitives, triMesh, colorTable, meshMaterial, index); 

    return new Graphic(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::WriteStatus WriteTile(PublishedTileR outputTile)
    {
    if (m_range.IsNull())
        return TileIO::WriteStatus::NoGeometry;

    m_writer.WriteBatchedModel(m_featureTable, m_range.LocalToGlobal(.5, .5, .5));

    BeFile          outputFile;

    if (BeFileStatus::Success != outputFile.Create (outputTile.GetFileName()) ||
        BeFileStatus::Success != outputFile.Write (nullptr, m_streamBuffer.data(), m_streamBuffer.size()))
        return TileIO::WriteStatus::UnableToOpenFile;
   
    outputFile.Close();
    outputTile.SetPublishedRange(m_range);

    return TileIO::WriteStatus::Success;
    }
        
};  // RenderSystem

   
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct Context
{
    TileTree::TilePtr   m_inputTile;
    PublishedTilePtr    m_outputTile;
    Transform           m_transformFromDgn;
    ICesiumPublisherP   m_publisher;
    double              m_leafTolerance;
    GeometricModelP     m_model;
    ClipVectorPtr       m_clip;

    Context(TileTree::TileP inputTile, PublishedTileP outputTile, TransformCR transformFromDgn, ClipVectorCP clip, ICesiumPublisher* publisher, double leafTolerance, GeometricModelP model) : 
            m_inputTile(inputTile), m_outputTile(outputTile), m_transformFromDgn(transformFromDgn), m_publisher(publisher), m_leafTolerance(leafTolerance), m_model(model) { if (nullptr != clip) m_clip = clip->Clone(nullptr); }

    Context(TileTree::TileP inputTile, PublishedTileP outputTile, Context const& inContext) :
            m_inputTile(inputTile), m_outputTile(outputTile), m_transformFromDgn(inContext.m_transformFromDgn), m_clip(inContext.m_clip), m_publisher(inContext.m_publisher), m_leafTolerance(inContext.m_leafTolerance), m_model(inContext.m_model) { }


};

typedef folly::Future<TileIO::WriteStatus> FutureWriteStatus;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTile::PublishedTile(TileTree::TileCR inputTile, BeFileNameCR outputDirectory) : m_publishedRange(DRange3d::NullRange())
    {
    WString         name = WString(inputTile._GetTileCacheKey().c_str(), true);

    name.ReplaceAll(L":", L"_");
    name.ReplaceAll(L".", L"_");
    name.ReplaceAll(L"/", L"_");

    m_tileRange = inputTile.GetRange();
    m_fileName = BeFileName (nullptr, outputDirectory.c_str(), name.c_str(), L".b3dm");
    m_tolerance = m_tileRange.DiagonalDistance() / s_minToleranceRatio;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::WriteStatus writeCesiumTile(Context context)
    {
    TileWriter::RenderSystem*    renderSystem = new TileWriter::RenderSystem(*context.m_model);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]
        {                                
        TileTree::TileLoadStatePtr      loadState;

        return context.m_inputTile->IsNotLoaded() ? context.m_inputTile->GetRootR()._RequestTile(*context.m_inputTile, loadState, renderSystem, BeTimePoint()) : SUCCESS;
        })
    .then([=](BentleyStatus status)
        {
        if (SUCCESS != status)
            return TileIO::WriteStatus::UnableToLoadTile;
        
        TileIO::WriteStatus writeStatus = renderSystem->WriteTile(*context.m_outputTile);
        delete renderSystem;
        return writeStatus;
        })
    .then([=](TileIO::WriteStatus status)
        {
        return status;
        }).get();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static FutureWriteStatus generateParentTile (Context context)
    {
    return folly::makeFuture (writeCesiumTile(context));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static FutureWriteStatus generateChildTiles (TileIO::WriteStatus parentStatus, Context context)
    {
    double      parentTolerance = context.m_outputTile->GetTolerance();

    if (parentTolerance < context.m_leafTolerance)
        return folly::makeFuture(TileIO::WriteStatus::Success);

    auto const& children = context.m_inputTile->_GetChildren(true);

    if (nullptr == children)
        return folly::makeFuture(parentStatus);

    std::vector<FutureWriteStatus> childFutures;

    for (auto& child : *children)
        {
        PublishedTilePtr    publishedTile = new PublishedTile(*child, context.m_publisher->_GetOutputDirectory(*context.m_model));
        Context             childContext(child.get(), publishedTile.get(), context);

        context.m_outputTile->GetChildren().push_back(publishedTile);
        auto childFuture = generateParentTile(childContext).then([=](FutureWriteStatus result) { return generateChildTiles(result.value(), childContext); });
        childFutures.push_back(std::move(childFuture));
        }

    return folly::unorderedReduce(childFutures, TileIO::WriteStatus::Success, [=](TileIO::WriteStatus reduced, TileIO::WriteStatus next)
        {
        return TileIO::WriteStatus::Aborted == reduced || TileIO::WriteStatus::Aborted == next ? TileIO::WriteStatus::Aborted : TileIO::WriteStatus::Success;
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
FutureWriteStatus writeCesiumTileset(ICesiumPublisher* publisher, GeometricModelP model, TransformCR transformFromDgn, double leafTolerance)
    {
    TileWriter::RenderSystem        renderSystem(*model);
    TileTree::RootCPtr              tileRoot = model->GetTileTree(&renderSystem);
    ClipVectorPtr                   clip;
    PublishedTilePtr                publishedRoot = new PublishedTile(*tileRoot->GetRootTile(), publisher->_GetOutputDirectory(*model));

    if (!tileRoot.IsValid())
        return folly::makeFuture(TileIO::WriteStatus::NoGeometry);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
        {
        return publisher->_BeginProcessModel(*model);
        })
    .then([=](TileIO::WriteStatus status)
        {
        if (TileIO::WriteStatus::Success != status)
            return folly::makeFuture(status);

        Transform           transformFromRoot = Transform::FromProduct(transformFromDgn, tileRoot->GetLocation());
        TileTree::TilePtr   inputTile = tileRoot->GetRootTile();
        Context             context(inputTile.get(), publishedRoot.get(), transformFromRoot, clip.get(), publisher, leafTolerance, model);
                                                                                                                
        return generateParentTile(context).then([=](FutureWriteStatus status) { return generateChildTiles(status.value(), context); });
        })
    .then([=](FutureWriteStatus status)
        {
        return folly::makeFuture(publisher->_EndProcessModel(*model, *publishedRoot, status.value()));   
        });
    }
END_TILEWRITER_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::WriteStatus ICesiumPublisher::WriteCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance)
    {
    return TileWriter::writeCesiumTileset(&publisher, const_cast<GeometricModelP> (&model), transformFromDgn, leafTolerance).get();
    }






