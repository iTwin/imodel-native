/*--------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/TileWriter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>


USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       
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
struct RenderSystem : Render::System
{

    RenderSystem()  { }
    ~RenderSystem() { }

    virtual MaterialPtr _GetMaterial(DgnMaterialId, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const override { return nullptr; } 
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { BeAssert(false); return nullptr; }
//  virtual TexturePtr _CreateTexture(ImageCR image, Render::Texture::CreateParams const& params) const override {return new Texture(image, params);}
//  virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp, Texture::CreateParams const& params) const override {return new Texture(source, targetFormat, bottomUp, params); }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateVisibleEdges(MeshEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateSilhouetteEdges(SilhouetteEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return nullptr; }
    virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features) const override {return nullptr; }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return 0xffffffff; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, Texture::CreateParams const& params=Texture::CreateParams())  const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, Texture::CreateParams const& params=Texture::CreateParams())  const override {return nullptr; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override 
    { 
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb)  const override 
    { 
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const override 
    {
    return  nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const override 
    { 
    return nullptr;
    }
};  // RenderSystem


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct  Writer
{
    Json::Value         m_json;
    ByteStream          m_binaryData;
    StreamBufferR       m_buffer;
    RenderSystem        m_renderSystem;
    DgnModelCR          m_model;

    Writer(StreamBufferR buffer, DgnModelR model) : m_buffer(buffer), m_model(model) { }

    size_t BinaryDataSize() const { return m_binaryData.size(); }
    void const* BinaryData() const { return m_binaryData.data(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddBinaryData (void const* data, size_t size) 
    {
    m_binaryData.Append (static_cast<uint8_t const*> (data), size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PadBinaryDataToBoundary(size_t boundarySize)
    {
    uint8_t        zero = 0;
    while (0 != (m_binaryData.size() % boundarySize))
        m_binaryData.Append(&zero, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PadTo4ByteBoundary()
    {
    long        position = m_buffer.GetSize(), padBytes = (4 - position % 4);

    if (0 != padBytes)
        {
        uint8_t     zero = 0;

        m_buffer.Append(&zero, 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void AddBufferView(Utf8CP name, T const& bufferData)
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
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddExtensions(DPoint3dCR centroid)
    {
    m_json["extensionsUsed"] = Json::arrayValue;
    m_json["extensionsUsed"].append("KHR_binary_glTF");
    m_json["extensionsUsed"].append("CESIUM_RTC");
    m_json["extensionsUsed"].append("WEB3D_quantized_attributes");

    m_json["glExtensionsUsed"] = Json::arrayValue;
    m_json["glExtensionsUsed"].append("OES_element_index_uint");

    m_json["extensions"]["CESIUM_RTC"]["center"] = Json::arrayValue;
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.x);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.y);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddDefaultScene ()
    {
    m_json["scene"] = "defaultScene";
    m_json["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    m_json["scenes"]["defaultScene"]["nodes"].append("rootNode");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTriMesh(Json::Value& primitivesNode, MeshArgs const& meshArgs, MeshCR mesh, size_t& index)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddPolyline(Json::Value& primitivesNode, PolylineArgs const& polylineArgs, MeshCR mesh, size_t& index)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshGraphics(Json::Value& primitivesNode, MeshCR mesh, size_t& index)
    { 
    GetMeshGraphicsArgs         graphicsArgs;
    bvector<Render::GraphicPtr> graphics;

    mesh.GetGraphics (graphics, m_renderSystem, graphicsArgs, m_model.GetDgnDb());

    AddTriMesh(primitivesNode, graphicsArgs.m_meshArgs, mesh, index);
    AddPolyline(primitivesNode, graphicsArgs.m_polylineArgs, mesh, index);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshes(Render::Primitives::GeometryCollectionCR geometry)
    {
    Json::Value     meshes     = Json::objectValue;
    Json::Value     mesh       = Json::objectValue;
    Json::Value     nodes      = Json::objectValue;
    Json::Value     rootNode   = Json::objectValue;
    Json::Value     primitives = Json::objectValue;
    Utf8String      meshName   = "Mesh";
    size_t          primitiveIndex = 0;

    for (auto& mesh : geometry.Meshes())
        AddMeshGraphics(primitives, *mesh, primitiveIndex);

    mesh["primitives"] = primitives;
    meshes[meshName] = mesh;
    rootNode["meshes"].append (meshName);
    nodes["rootNode"] = rootNode;
    m_json["meshes"] = meshes;
    m_json["nodes"]  = nodes;
    }

};  // Writer

           
//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct GltfWriter : Writer
{

public:
    GltfWriter(StreamBufferR streamBuffer, DgnModelR model) : Writer(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteGltf(Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid)
    {
    AddExtensions(centroid);
    AddDefaultScene();
    AddMeshes (geometry);

    return SUCCESS;
    }


};  // GltfWriter

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct BatchedModelWriter : GltfWriter
{

public:
    BatchedModelWriter(StreamBufferR streamBuffer, DgnModelR model) : GltfWriter(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteBatchedModel(Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid)
    {
    Utf8String          batchTableStr = BatchTableBuilder (geometry.Meshes().FeatureTable(), m_model.GetDgnDb(), m_model.Is3d()).ToString();
    uint32_t            batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    uint32_t            zero = 0;
    uint32_t            b3dmNumBatches = geometry.Meshes().FeatureTable().size();

    long    startPosition = m_buffer.GetSize();
    m_buffer.Append((const uint8_t *) s_b3dmMagic, 4);
    m_buffer.Append((const uint8_t *) &s_b3dmVersion, 4);
    long    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((const uint8_t *) &zero, sizeof(uint32_t));    // Filled in below.
    m_buffer.Append((const uint8_t *) &batchTableStrLen, sizeof(uint32_t));
    m_buffer.Append((const uint8_t *) &zero, sizeof(uint32_t)); // length of binary portion of batch table - we have no binary batch table data
    m_buffer.Append((const uint8_t *) &b3dmNumBatches, sizeof(uint32_t));
    m_buffer.Append((const uint8_t *) batchTableStr.data(), batchTableStrLen);

    if (SUCCESS != WriteGltf (geometry, centroid))
        return ERROR;

    PadTo4ByteBoundary ();
    uint32_t    dataSize = static_cast<uint32_t> (m_buffer.GetSize() - startPosition);
    memcpy(m_buffer.GetDataP() + lengthDataPosition, &dataSize, sizeof(uint32_t));
    return SUCCESS;
    }
};  // BatchedModelWriter


   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileIO::WriteTile(StreamBufferR streamBuffer, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid)
    {
    return BatchedModelWriter(streamBuffer, model).WriteBatchedModel(geometry, centroid);
    }
