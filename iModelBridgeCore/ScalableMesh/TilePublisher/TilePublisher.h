/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <TilePublisher/MeshTile.h>
//#include <DgnPlatform/DgnPlatformApi.h>
//#include <DgnPlatform/DgnPlatformLib.h>
//#include <DgnPlatform/DgnGeoCoord.h>
//#include <DgnPlatform/AutoRestore.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <stdio.h>
#include "../STM/Stores/ISMDataStore.h"

#if defined(__TILEPUBLISHER_LIB_BUILD__)
    #define TILEPUBLISHER_EXPORT EXPORT_ATTRIBUTE
#else
    #define TILEPUBLISHER_EXPORT /*IMPORT_ATTRIBUTE*/
#endif

//#define BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE BEGIN_BENTLEY_RENDER_NAMESPACE namespace Tile3d {
//#define END_BENTLEY_DGN_TILE3D_NAMESPACE } END_BENTLEY_RENDER_NAMESPACE
//
//BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//=======================================================================================
// Maps elements associated with vertices to indexes into a batch table in the b3dm.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct BatchIdMap
{
private:
    bmap<BentleyApi::BeInt64Id, uint16_t>   m_map;
    bvector<BentleyApi::BeInt64Id>          m_list;
    TileSource                  m_source;
public:
    BatchIdMap(TileSource source);

    uint16_t GetBatchId(BentleyApi::BeInt64Id entityId);
    void ToJson(Json::Value& value, DgnDbP db) const;
    uint16_t Count() const { return static_cast<uint16_t>(m_list.size()); }
};


struct  ScalableMeshTileNode : BentleyApi::Dgn::ModelTileNode
    {
    IScalableMeshNodePtr    m_node;
    Transform               m_transform;
    //DgnModelId              m_modelId;
    //PublishTileNode(DgnModelId modelId, SceneR scene, NodeR node, TransformCR transformDbToTile, DRange3dCR dgnRange, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent, ClipVectorCP clip)
    //    : ModelTileNode(dgnRange, transformDbToTile, depth, siblingIndex, parent, tolerance), m_scene(&scene), m_node(&node), m_clip(clip), m_modelId(modelId) { }
    ScalableMeshTileNode(/*DgnModelId modelId,*/ IScalableMeshNodePtr& node, DRange3d transformedRange, TransformCR transform, size_t siblingIndex, BentleyApi::Dgn::TileNodeP parent) :
        /*m_modelId(modelId), */m_node(node), m_transform(transform), BentleyApi::Dgn::ModelTileNode(transformedRange, transform, node->GetLevel(), siblingIndex, parent, transformedRange.XLength()* transformedRange.YLength() / node->GetPointCount())
        {}


    virtual TileMeshList _GenerateMeshes(/*TileGenerationCacheCR, DgnDbR dgnDb,*/ TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines) const override
        {
        TileMeshList        tileMeshes;
        //
        //if (m_node->GetChildrenNodes().empty())
        //    {
        //    BeAssert(false);
        //    return tileMeshes;
        //    }
        //
        //
        //for (auto& child : m_node->GetChildrenNodes())
        //    {
        //
        auto child = m_node;
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);
            //bvector<bool> clips;
            auto meshP = child->GetMesh(flags);
            if (!meshP.IsValid() || meshP->GetNbFaces() == 0) return tileMeshes;
            //if (!meshP.IsValid() || meshP->GetNbFaces() == 0) continue;
            TileMeshBuilderPtr      builder;
            TileDisplayParamsPtr    displayParams;
        
            if (child->IsTextured())
                {
                auto textureP = child->GetTextureCompressed();
                ImageSource jpgTex(ImageSource::Format::Jpeg, ByteStream(textureP->GetData(), (uint32_t)textureP->GetSize()));
                //memcpy(myImage.GetDataP(), textureP->GetData(), textureP->GetDimension().x* textureP->GetDimension().y * 3);
                //Image image(textureP->GetDimension().x, textureP->GetDimension().y, std::move(myImage), Image::Format::Rgb);
                //ImageSource jpgTex(image, ImageSource::Format::Jpeg, 100);
                //{
                //FILE*               file = fopen(("d:\\tmp\\texture_" + std::to_string(child->GetNodeId()) + ".jpg").c_str(), "w");
                //
                //fwrite(jpgTex.GetByteStream().GetData(), 1, jpgTex.GetByteStream().size(), file);
                //fclose(file);
                //}
                TileTextureImagePtr     tileTexture = TileTextureImage::Create(jpgTex);
                displayParams = TileDisplayParams::Create(0xffffff, tileTexture, true);
                }
            else
                {
                TileTextureImagePtr     tileTexture = nullptr;
                displayParams = TileDisplayParams::Create(0x007700, tileTexture, false);
                }
            builder = TileMeshBuilder::Create(displayParams, 0.0);
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*meshP->GetPolyfaceQuery()); visitor->AdvanceToNextFace();)
                builder->AddTriangle(*visitor, /*DgnMaterialId(), dgnDb, m_modelId,*/ false, twoSidedTriangles);
        
            tileMeshes.push_back(builder->GetMesh());
        //    }
        //
        return tileMeshes;
        }

    };  //  ScalableMeshTileNode

struct PublisherContext;
DEFINE_REF_COUNTED_PTR(PublisherContext);

//=======================================================================================
//! Context in which tile publishing occurs.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PublisherContext : RefCountedBase
{
    enum class Status
        {
        Success = SUCCESS,
        NoGeometry,
        Aborted,
        CantWriteToBaseDirectory,
        CantCreateSubDirectory,
        ErrorWritingScene,
        ErrorWritingNode,
        NotImplemented,
        };


protected:
    //ViewControllerPtr   m_viewController;
    BeFileName          m_outputDir;
    BeFileName          m_dataDir;
    WString             m_rootName;
    Transform           m_dbToTile;
    Transform           m_tileToEcef;
    Transform           m_tilesetTransform;
    size_t              m_maxTilesetDepth;
    size_t              m_maxTilesPerDirectory;
    bool                m_publishPolylines;

    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const { return tile.GetRelativePath(GetRootName().c_str(), fileExtension); };
//    virtual TileGenerationCacheCR _GetCache() const = 0;
    virtual bool _OmitFromTileset(TileNodeCR) const { return false; }
    virtual bool _AllTilesPublished() const { return false; }   // If all tiles are published then we can write only valid (non-empty) tree leaves and branches.

    Status Setup();
//    TILEPUBLISHER_EXPORT Status PublishViewModels (TileGeneratorR generator, TileGenerator::ITileCollector& collector, DRange3dR range, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter);
//    TILEPUBLISHER_EXPORT Status CollectOutputTiles (Json::Value& rootJson, DRange3dR rootRange, TileNodeR rootTile, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector);

//    Status PublishElements (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters);
//    Status DirectPublishModel (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, DgnModelR model, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter);

    TILEPUBLISHER_EXPORT void WriteMetadataTree (DRange3dR range, Json::Value& val, TileNodeCR tile, size_t depth);
    TILEPUBLISHER_EXPORT void WriteTileset (BeFileNameCR metadataFileName, TileNodeCR rootTile, size_t maxDepth);


public:
    TILEPUBLISHER_EXPORT PublisherContext(/*ViewControllerR viewController,*/ BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation = nullptr, bool publishPolylines = false, size_t maxTilesetDepth = 5, size_t maxTilesPerDirectory = 5000);

    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR  GetTileToEcef() const { return m_tileToEcef; }
    TransformCR  GetTilesetTransform () const { return m_tilesetTransform; }
//    ViewControllerCPtr GetViewController() const { return m_viewController; }
    DgnDbP GetDgnDb() const { return nullptr/*m_viewController.GetDgnDb()*/; }
    size_t GetMaxTilesPerDirectory () const { return m_maxTilesPerDirectory; }
    size_t GetMaxTilesetDepth() const { return m_maxTilesetDepth; }
    bool WantPolylines() const { return m_publishPolylines; }

//    TILEPUBLISHER_EXPORT static Status ConvertStatus(TileGenerator::Status input);
//    TILEPUBLISHER_EXPORT static TileGenerator::Status ConvertStatus(Status input);

    WString GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const { return _GetTileUrl(tile, fileExtension); }
//    TileGenerationCacheCR GetCache() const { return _GetCache(); }

    TILEPUBLISHER_EXPORT void GetSpatialViewJson (Json::Value& json, SpatialViewDefinitionCR view, TransformCR transform);
    TILEPUBLISHER_EXPORT Json::Value GetModelsJson (DgnModelIdSet const& modelIds);
    TILEPUBLISHER_EXPORT Json::Value GetCategoriesJson(DgnCategoryIdSet const& categoryIds);
    TILEPUBLISHER_EXPORT bool IsGeolocated () const;

    template<typename T> static Json::Value IdSetToJson(T const& ids)
        {
        Json::Value json(Json::arrayValue);
        for (auto const& id : ids)
            json.append(id.ToString());
        return json;

        }
    static Json::Value PointToJson(DPoint3dCR pt)
        {
        Json::Value json(Json::objectValue);
        json["x"] = pt.x;
        json["y"] = pt.y;
        json["z"] = pt.z;
        return json;
        }
};

//=======================================================================================
//! Publishes a single tile.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TilePublisher
{
    typedef bmap<uint32_t, Utf8String> TextureIdToNameMap;
private:
    BatchIdMap              m_batchIds;
    std::FILE*              m_outputFile;
    DPoint3d                m_centroid;
    TileMeshList            m_meshes;
    TileNodeCPtr            m_tile;
    ByteStream              m_binaryData;
    PublisherContextPtr     m_context;
    bmap<TileTextureImageCP, Utf8String>    m_textureImages;

    static WString GetNodeNameSuffix(TileNodeCR tile);
    static DPoint3d GetCentroid(TileNodeCR tile);
    static void AppendPoint(Json::Value& val, DPoint3dCR pt) { val.append(pt.x); val.append(pt.y); val.append(pt.z); }
    static void AddTechniqueParameter(Json::Value&, Utf8CP name, int type, Utf8CP semantic);
    static void AppendProgramAttribute(Json::Value&, Utf8CP);
    static void AddShader(Json::Value&, Utf8CP name, int type, Utf8CP buffer);
    static Utf8String Concat(Utf8CP prefix, Utf8StringCR suffix) { Utf8String str(prefix); str.append(suffix); return str; }

    void ProcessMeshes(Json::Value& value);
    void AddExtensions(Json::Value& value);
    void AddTextures(Json::Value& value, TextureIdToNameMap& texNames);
    void AddMeshVertexAttribute  (Json::Value& rootNode, double const* values, Utf8StringCR bufferViewId, Utf8StringCR accesorId, size_t nComponents, size_t nAttributes, char* accessorType, bool quantize, double const* min, double const* max);
    void AddBinaryData (void const* data, size_t size);

    Utf8String AddMeshShaderTechnique (Json::Value& rootNode, bool textured, bool transparent, bool ignoreLighting);
    Utf8String AddPolylineShaderTechnique (Json::Value& rootNode);

    void AddMesh(Json::Value& value, TileMeshR mesh, size_t index);
    void AppendUInt32(uint32_t value);

    Utf8String AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix);
    Utf8String AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, TileMeshCR mesh, Utf8CP suffix);

    template<typename T> void AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData);

public:
    TILEPUBLISHER_EXPORT TilePublisher(PublisherContextPtr context);
    TILEPUBLISHER_EXPORT TilePublisher(TileNodeCR tile, PublisherContext& context);
    TILEPUBLISHER_EXPORT TilePublisher(TileNodeCR tile);

    TILEPUBLISHER_EXPORT PublisherContext::Status Publish();
    TILEPUBLISHER_EXPORT PublisherContext::Status Publish(TileMeshR mesh, bvector<Byte>& outData);
    TILEPUBLISHER_EXPORT PublisherContext::Status Publish(TileMeshR mesh, Utf8StringR sceneStr);

    BeFileNameCR GetDataDirectory() const { return m_context->GetDataDirectory(); }
    //void         SetDataDirectory() { m_context->GetDataDirectory}
    WStringCR GetPrefix() const { return m_context->GetRootName(); }
    TILEPUBLISHER_EXPORT static void WriteBoundingVolume(Json::Value&, DRange3dCR);
    TILEPUBLISHER_EXPORT static void WriteJsonToFile (WCharCP fileName, Json::Value& value);
};

END_BENTLEY_SCALABLEMESH_NAMESPACE
//END_BENTLEY_DGN_TILE3D_NAMESPACE

