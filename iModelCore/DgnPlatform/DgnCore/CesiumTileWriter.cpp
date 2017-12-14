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

USING_NAMESPACE_TILETREE_IO
USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       

static double s_minToleranceRatio = 512.0;

BEGIN_TILETREE_IO_NAMESPACE

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
#if defined(ERROR_UNUSED_FIELD)
    FeatureTableCR                          m_attrs;
#endif
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
    Json::Value& GetJson()          { return m_json; }
    Utf8String ToString() const     { return getJsonString(m_json); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BatchTableBuilder(FeatureTableCR attrs, DgnDbR db, bool is3d) : m_json(Json::objectValue), m_db(db), m_is3d(is3d)
#if defined(ERROR_UNUSED_FIELD)
, m_attrs(attrs)
#endif
    {
    InitUncategorizedCategory();

    Json::Value             geomClasses    = Json::arrayValue, 
                            elementIds     = Json::arrayValue, 
                            assemblyIds    = Json::arrayValue, 
                            categoryIds    = Json::arrayValue,
                            subCategoryIds = Json::arrayValue;

#if defined(ERROR_UNUSED_VARIABLE)
    bool                    validLabelsFound = false;
#endif
                        
    for (auto const& kvp : attrs)
        {
        FeatureCR           attr = kvp.first;                                                                                                                                    
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

#ifdef WIP
    context.AddBatchTableAttributes (m_json, attrs);
#endif
    }


};  // BatchTableBuilder


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
struct CesiumTileWriter : TileTree::IO::Writer
{
    CesiumTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : TileTree::IO::Writer(streamBuffer, model) { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, TriMeshArgs const& meshArgs, MeshMaterial const& meshMaterial, Utf8StringCR idStr)
    {
    primitiveJson["mode"] = static_cast<int32_t>(Gltf::PrimitiveType::Triangles);

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
    m_buffer.Append(Format::B3dm);
    m_buffer.Append(B3dm::Version);                                                          
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

    RenderSystem(GeometricModelR model) : m_writer(m_streamBuffer, model), m_range(DRange3d::NullRange()), m_featureTable(model.GetModelId(), s_maxFeatures)  { }

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
WriteStatus WriteTile(PublishedTileR outputTile)
    {
    if (m_range.IsNull())
        return WriteStatus::NoGeometry;

    m_writer.WriteBatchedModel(m_featureTable, m_range.LocalToGlobal(.5, .5, .5));

    BeFile          outputFile;

    if (BeFileStatus::Success != outputFile.Create (outputTile.GetFileName()) ||
        BeFileStatus::Success != outputFile.Write (nullptr, m_streamBuffer.data(), m_streamBuffer.size()))
        return WriteStatus::UnableToOpenFile;
   
    outputFile.Close();
    outputTile.SetPublishedRange(m_range);

    return WriteStatus::Success;
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

typedef folly::Future<WriteStatus> FutureWriteStatus;

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
WriteStatus writeCesiumTile(Context context)
    {
    TileTree::IO::RenderSystem*    renderSystem = new TileTree::IO::RenderSystem(*context.m_model);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]
        {                                
        TileTree::TileLoadStatePtr      loadState;

        return context.m_inputTile->IsNotLoaded() ? context.m_inputTile->GetRootR()._RequestTile(*context.m_inputTile, loadState, renderSystem, BeDuration()) : SUCCESS;
        })
    .then([=](BentleyStatus status)
        {
        if (SUCCESS != status)
            return WriteStatus::UnableToLoadTile;
        
        WriteStatus writeStatus = renderSystem->WriteTile(*context.m_outputTile);
        delete renderSystem;
        return writeStatus;
        })
    .then([=](WriteStatus status)
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
static FutureWriteStatus generateChildTiles (WriteStatus parentStatus, Context context)
    {
    double      parentTolerance = context.m_outputTile->GetTolerance();

    if (parentTolerance < context.m_leafTolerance)
        return folly::makeFuture(WriteStatus::Success);

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

    return folly::unorderedReduce(childFutures, WriteStatus::Success, [=](WriteStatus reduced, WriteStatus next)
        {
        return WriteStatus::Aborted == reduced || WriteStatus::Aborted == next ? WriteStatus::Aborted : WriteStatus::Success;
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
FutureWriteStatus writeCesiumTileset(ICesiumPublisher* publisher, GeometricModelP model, TransformCR transformFromDgn, double leafTolerance)
    {
    TileTree::IO::RenderSystem        renderSystem(*model);
    TileTree::RootCPtr              tileRoot = model->GetTileTree(&renderSystem);
    ClipVectorPtr                   clip;
    PublishedTilePtr                publishedRoot = new PublishedTile(*tileRoot->GetRootTile(), publisher->_GetOutputDirectory(*model));

    if (!tileRoot.IsValid())
        return folly::makeFuture(WriteStatus::NoGeometry);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
        {
        return publisher->_BeginProcessModel(*model);
        })
    .then([=](WriteStatus status)
        {
        if (WriteStatus::Success != status)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus ICesiumPublisher::WriteCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance)
    {
    return TileTree::IO::writeCesiumTileset(&publisher, const_cast<GeometricModelP> (&model), transformFromDgn, leafTolerance).get();
    }

END_TILETREE_IO_NAMESPACE

