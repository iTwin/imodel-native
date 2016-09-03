/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/AutoRestore.h>
#include <stdio.h>

#if defined(__TILEPUBLISHER_LIB_BUILD__)
    #define TILEPUBLISHER_EXPORT EXPORT_ATTRIBUTE
#else
    #define TILEPUBLISHER_EXPORT IMPORT_ATTRIBUTE
#endif

USING_NAMESPACE_BENTLEY

#define BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE BEGIN_BENTLEY_RENDER_NAMESPACE namespace Tile3d {
#define END_BENTLEY_DGN_TILE3D_NAMESPACE } END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE

//=======================================================================================
// Maps elements associated with vertices to indexes into a batch table in the b3dm.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct BatchIdMap
{
private:
    bmap<DgnElementId, uint16_t> m_map;
    bvector<DgnElementId>       m_list;
public:
    BatchIdMap();

    uint16_t GetBatchId(DgnElementId elemId);
    void ToJson(Json::Value& value) const;
    uint16_t Count() const { return static_cast<uint16_t>(m_list.size()); }
};

//=======================================================================================
//! Context in which tile publishing occurs.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PublisherContext
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
    ViewControllerR     m_viewController;
    BeFileName          m_outputDir;
    BeFileName          m_dataDir;
    WString             m_rootName;
    Transform           m_dbToTile;
    Transform           m_tileToEcef;
    Transform           m_tilesetTransform;
    size_t              m_maxTilesetDepth;
    size_t              m_maxTilesPerDirectory;

    TILEPUBLISHER_EXPORT PublisherContext(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName, size_t s_maxTilesetDepth = 5, size_t maxTilesPerDirectory = 5000);

    virtual TileGeometryCacheP _GetGeometryCache() = 0;
    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const = 0;

    TILEPUBLISHER_EXPORT Status Setup();

    TILEPUBLISHER_EXPORT Status PublishViewModels (TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters, TileGenerator::IProgressMeter& progressMeter);
    Status PublishElements (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters);
    Status DirectPublishModel (Json::Value& rootJson, DRange3dR rootRange, WStringCR name, DgnModelR model, TileGeneratorR generator, TileGenerator::ITileCollector& collector, double toleranceInMeters, TileGenerator::IProgressMeter& progressMeter);
    Status CollectOutputTiles (Json::Value& rootJson, DRange3dR rootRange, TileNodePtr& rootTile, WStringCR name, TileGeneratorR generator, TileGenerator::ITileCollector& collector);

public:
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR  GetTileToEcef() const { return m_tileToEcef; }
    TransformCR  GetTilesetTransform () const { return m_tilesetTransform; }
    DgnDbR GetDgnDb() { return m_viewController.GetDgnDb(); }
    size_t GetMaxTilesPerDirectory () const { return m_maxTilesPerDirectory; }
    size_t GetMaxTilesetDepth() const { return m_maxTilesetDepth; }

    TILEPUBLISHER_EXPORT static Status ConvertStatus(TileGenerator::Status input);
    TILEPUBLISHER_EXPORT static TileGenerator::Status ConvertStatus(Status input);

    TileGeometryCacheP GetGeometryCache() { return _GetGeometryCache(); }
    WString GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const { return _GetTileUrl(tile, fileExtension); }
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
    TileNodeCR              m_tile;
    ByteStream              m_binaryData;
    PublisherContext&       m_context;

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
    void WriteMetadataTree (Json::Value& val, TileNodeCR tile, size_t depth);
    void WriteTileset (BeFileNameCR metadataFileName, size_t maxDepth);

    Utf8String AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, bool isPolyline, Utf8CP suffix);
    Utf8String AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, Utf8CP suffix);

    template<typename T> void AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData);
public:

    TILEPUBLISHER_EXPORT TilePublisher(TileNodeCR tile, PublisherContext& context);

    TILEPUBLISHER_EXPORT PublisherContext::Status Publish();

    BeFileNameCR GetDataDirectory() const { return m_context.GetDataDirectory(); }
    WStringCR GetPrefix() const { return m_context.GetRootName(); }
    TileGeometryCacheR GetGeometryCache() { BeAssert(nullptr != m_context.GetGeometryCache()); return *m_context.GetGeometryCache(); }
    static void WriteBoundingVolume(Json::Value&, DRange3dCR);
    static void WriteJsonToFile (WCharCP fileName, Json::Value& value);
};


END_BENTLEY_DGN_TILE3D_NAMESPACE

