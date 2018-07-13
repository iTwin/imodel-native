/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/CesiumTileWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#define JSON_Root "root"
#define JSON_GeometricError "geometricError"
#define JSON_BoundingVolume "boundingVolume"
#define JSON_Box "box"
#define JSON_Children "children"
#define JSON_Content "content"
#define JSON_Transform "transform"


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
    Utf8String              m_name;
    Render::MaterialCPtr    m_material;
    TileTextureCPtr         m_texture;

    MeshMaterial (Utf8StringCR suffix, MaterialCP material, TextureCP texture) : m_name("Material_" + suffix), m_material(material), m_texture(dynamic_cast<TileTextureCP>(texture)) { }

    bool IsTextured() const { return m_texture.IsValid(); }
    bool IgnoresLighting() const { return false; }
    Utf8StringCR GetName() const { return m_name; }
    TileTextureCPtr     GetTexture() const { return m_texture; }


};  // MeshMaterial


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct CesiumTileWriter : TileTree::IO::Writer
{
    CesiumTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : TileTree::IO::Writer(streamBuffer, model) { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddTextureImage (TileTextureCR texture, Utf8StringCR idStr)
    {
    Render::ImageSourceCR   imageSource = texture.m_imageSource;
    bool                    hasAlpha = imageSource.GetFormat() == ImageSource::Format::Png;

    Utf8String  textureId = Utf8String ("texture_") + idStr;
    Utf8String  imageId   = Utf8String ("image_")   + idStr;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + idStr;

    m_json["textures"][textureId] = Json::objectValue;
    m_json["textures"][textureId]["format"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    m_json["textures"][textureId]["internalFormat"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    m_json["textures"][textureId]["sampler"] = texture.m_repeat ? "sampler_0" : "sampler_2";
    m_json["textures"][textureId]["source"] = imageId;

    m_json["images"][imageId] = Json::objectValue;


    m_json["bufferViews"][bvImageId] = Json::objectValue;
    m_json["bufferViews"][bvImageId]["buffer"] = "binary_glTF";


    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = imageSource.GetFormat() == ImageSource::Format::Png ? "image/png" : "image/jpeg";

    Render::Texture::Dimensions     dimensions = texture.GetDimensions();
    if (0 == dimensions.width || 0 == dimensions.height)
        {
        Image       image (imageSource, hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);
        
        dimensions.width = image.GetWidth();
        dimensions.height = image.GetHeight();
        }

    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"]  = dimensions.width;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = dimensions.height;

    ByteStream const& imageData = imageSource.GetByteStream();
    m_json["bufferViews"][bvImageId]["byteOffset"] = BinaryDataSize();
    m_json["bufferViews"][bvImageId]["byteLength"] = imageData.size();
    AddBinaryData (imageData.data(), imageData.size());

    return textureId;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     CreateMaterialJson(MeshMaterialCR material, Utf8StringCR idStr)
    { 
    Json::Value         json = Json::objectValue, values = Json::objectValue;
    
    if (material.GetTexture().IsValid())
        values["tex"] = AddTextureImage(*material.GetTexture(), idStr);

    json["values"] = values;
    return json; 
    }

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
    Json::Value         primitiveJson;

    if (SUCCESS == CreateTriMesh(primitiveJson, meshArgs, meshMaterial, idStr))
        {
        m_json["materials"][meshMaterial.GetName()] = CreateMaterialJson(meshMaterial, idStr);
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
    m_buffer.Append(static_cast<uint32_t>(featureTableStr.size()));                     // feature table JSon length.
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
BentleyStatus  WriteBatchedModel(Render::FeatureTableCR featureTable)
    {
    uint32_t       startPosition = 0, lengthDataPosition = 0;

    BeginBatchedModel(startPosition, lengthDataPosition, featureTable);
    WriteGltf();
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
    mutable FeatureTable                m_featureTable;

    RenderSystem(GeometricModelR model) : m_writer(m_streamBuffer, model), m_range(DRange3d::NullRange()), m_featureTable(model.GetModelId(), s_maxFeatures)  { }

    MaterialPtr _FindMaterial(MaterialKeyCR, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _GetMaterial(RenderMaterialId, DgnDbR) const override { return nullptr; }
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    TexturePtr _FindTexture(TextureKeyCR, DgnDbR) const override { return nullptr; }
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { return nullptr; }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual Render::TargetPtr _CreateOffscreenTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return new Graphic(dgndb); }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return s_maxFeatures; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, DgnDbR, Texture::CreateParams const& params)  const override {return new TileTexture(image, params); }
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, DgnDbR, Texture::CreateParams const& params)  const override {return new TileTexture(source, bottomUp, params); }
    virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override  { return nullptr; }
    virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const override { return nullptr; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& featureTable, DRange3dCR range) const override 
    {
    m_featureTable = std::move(featureTable);
    m_range = range;
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual MaterialPtr _CreateMaterial(Material::CreateParams const&, DgnDbR) const override
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
    MeshMaterial    meshMaterial(idStr, triMesh.m_material.get(), triMesh.m_texture.get());

    m_range.Extend(triMesh.m_pointParams.GetRange());
    m_writer.AddTriMesh(m_primitives, triMesh, colorTable, meshMaterial, index); 

    return new Graphic(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb)  const override 
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WriteTile(PublishedTileR outputTile)
    {
    if (m_range.IsNull())
        return WriteStatus::NoGeometry;

    m_writer.AddPrimitivesJson(m_primitives);
    m_writer.WriteBatchedModel(m_featureTable);

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
    ICesiumPublisherP   m_publisher;
    double              m_leafTolerance;
    GeometricModelP     m_model;
    ClipVectorPtr       m_clip;

    Context(TileTree::TileP inputTile, PublishedTileP outputTile, ClipVectorCP clip, ICesiumPublisher* publisher, double leafTolerance, GeometricModelP model) : 
            m_inputTile(inputTile), m_outputTile(outputTile), m_publisher(publisher), m_leafTolerance(leafTolerance), m_model(model) { if (nullptr != clip) m_clip = clip->Clone(nullptr); }

    Context(TileTree::TileP inputTile, PublishedTileP outputTile, Context const& inContext) :
            m_inputTile(inputTile), m_outputTile(outputTile), m_clip(inContext.m_clip), m_publisher(inContext.m_publisher), m_leafTolerance(inContext.m_leafTolerance), m_model(inContext.m_model) { }


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
    m_url =   Utf8String(BeFileName (nullptr, nullptr, name.c_str(), L".b3dm"));
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
FutureWriteStatus writeCesiumTileset(ICesiumPublisher* publisher, GeometricModelP model, double leafTolerance)
    {
    auto                            renderSystem = std::make_shared<RenderSystem>(*model);
    TileTree::RootCPtr              tileRoot = model->GetTileTree(renderSystem.get());
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

        TileTree::TilePtr   inputTile = tileRoot->GetRootTile();
        Context             context(inputTile.get(), publishedRoot.get(), clip.get(), publisher, leafTolerance, model);
                                                                                                                
        return generateParentTile(context).then([=](FutureWriteStatus status) { return generateChildTiles(status.value(), context); });
        })
    .then([=](FutureWriteStatus status)
        {
        auto pRenderSys = renderSystem.get();
        UNUSED_VARIABLE(pRenderSys);

        return folly::makeFuture(publisher->_EndProcessModel(*model, *publishedRoot, status.value()));   
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus ICesiumPublisher::PublishCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance)
    {
    return TileTree::IO::writeCesiumTileset(&publisher, const_cast<GeometricModelP> (&model), leafTolerance).get();
    }


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/2017
//=======================================================================================
struct CesiumTilesetPublisher: ICesiumPublisher
{
public:
    CesiumTilesetPublisher(BeFileName outputFileName, BeFileNameCR outputDirectory, TransformCR dbToEcef) : m_outputFileName(outputFileName), m_outputDirectory(outputDirectory), m_dbToEcef(dbToEcef) { }

protected:
    BeFileName      m_outputDirectory;
    BeFileName      m_outputFileName;
    Transform       m_dbToEcef;


    BeFileName  _GetOutputDirectory(GeometricModelCR model) const override { return m_outputDirectory; }

    static void AppendPoint(Json::Value& val, DPoint3dCR pt) { val.append(pt.x); val.append(pt.y); val.append(pt.z); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBoundingVolume(Json::Value& val, DRange3dCR range)
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
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteModelMetadataTree (DRange3dR range, Json::Value& root, PublishedTileCR tile, GeometricModelCR model)
    {
    if (tile.GetIsEmpty() && tile.GetChildren().empty())
        {
        range = DRange3d::NullRange();
        return;
        }

    WString         rootName = TileUtil::GetRootNameForModel(model.GetModelId(), false);
    DRange3d        contentRange, publishedRange = tile.GetPublishedRange();

    
    // the published range represents the actual range of the published meshes. - This may be smaller than the 
    // range estimated when we built the tile tree. -- However we do not clip the meshes to the tile range.
    // so start the range out as the intersection of the tile range and the published range.
    range = contentRange = DRange3d::FromIntersection (tile.GetTileRange(), publishedRange, true);

    if (!tile.GetChildren().empty())
        {
        root[JSON_Children] = Json::arrayValue;

        // Append children to this tileset.
        for (auto& childTile : tile.GetChildren())
            {
            Json::Value         child;
            DRange3d            childRange;

            WriteModelMetadataTree (childRange, child, *childTile, model);
            if (!childRange.IsNull())
                {
                root[JSON_Children].append(child);
                range.Extend (childRange);
                }
            }
        }
    if (range.IsNull())
        return;

    root["refine"] = "REPLACE";
    root[JSON_GeometricError] = tile.GetTolerance();

    WriteBoundingVolume (root, tile.GetTileRange());
    
    if (!tile.GetIsEmpty())
        {
        // TBD.  ContentRange.
        root[JSON_Content]["url"] = tile.GetURL();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value TransformToJson(TransformCR tf)
    {
    auto matrix = DMatrix4d::From(tf);
    Json::Value json(Json::arrayValue);
    for (size_t i=0;i<4; i++)
        for (size_t j=0; j<4; j++)
            json.append (matrix.coff[j][i]);


    return json;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WriteTileset (GeometricModelCR model, PublishedTileCR rootTile)
    {
    Json::Value val, modelRoot;

    val["asset"]["version"] = "0.0";
    val["asset"]["gltfUpAxis"] = "Z";
 
    DRange3d    rootRange;
    WriteModelMetadataTree (rootRange, modelRoot, rootTile, model);

    val[JSON_Root] = std::move(modelRoot);

    val[JSON_Root][JSON_Transform] = TransformToJson(m_dbToEcef);

    return SUCCESS == TileUtil::WriteJsonToFile (m_outputFileName.c_str(), val) ? WriteStatus::Success : WriteStatus::UnableToWriteFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus InitializeDirectories()
    {
    return BeFileName::DoesPathExist(m_outputDirectory) ? BeFileName::EmptyDirectory (m_outputDirectory) :  BeFileName::CreateNewDirectory(m_outputDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus _BeginProcessModel(GeometricModelCR model)
    {
    return (BeFileNameStatus::Success == InitializeDirectories()) ? WriteStatus::Success : WriteStatus::UnableToOpenFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus _EndProcessModel(GeometricModelCR model, PublishedTileCR rootTile, WriteStatus status)
    {
    if (WriteStatus::Success != status)
        {
        BeFileName::EmptyAndRemoveDirectory (m_outputDirectory);
        return status;
        }
    return WriteTileset(model, rootTile);
    }
};  // CesiumTilesetPublisher.


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::IO::WriteStatus ICesiumPublisher::WriteCesiumTileset(BeFileName outputFileName, BeFileNameCR tileOutputDirectory, GeometricModelCR model, TransformCR dbToEcef, double leafTolerance) 
    {   
    CesiumTilesetPublisher publisher(outputFileName, tileOutputDirectory, dbToEcef);

    return ICesiumPublisher::PublishCesiumTileset(publisher, model, dbToEcef, leafTolerance);
    }    
END_TILETREE_IO_NAMESPACE

