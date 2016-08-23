/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <iostream>
#include <fstream>

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
//! Publishes the contents of a DgnDb view as a Cesium tileset.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TilesetPublisher : TileGenerator::ITileCollector
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
    };
private:
    ViewControllerR     m_viewController;
    BeFileName          m_outputDir;
    BeFileName          m_dataDir;
    WString             m_rootName;
    TileGeneratorP      m_generator = nullptr;
    Status              m_acceptTileStatus = Status::Success;
    Transform           m_dbToTile;
    Transform           m_tileToEcef;

    virtual TileGenerator::Status _AcceptTile(TileNodeCR tile) override;

    Status Setup();
    Status WriteWebApp(TransformCR transform, bvector<WString>& viewedTileSetNames);
    void OutputStatistics(TileGenerator::Statistics const& stats) const;

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   08/16
    //=======================================================================================
    struct ProgressMeter : TileGenerator::IProgressMeter
    {
    private:
        Utf8String          m_taskName;
        TilesetPublisher&   m_publisher;
        uint32_t            m_lastNumCompleted = 0xffffffff;
        
        virtual void _IndicateProgress(uint32_t completed, uint32_t total) override;
        virtual void _SetTaskName(TileGenerator::TaskName task) override;
        virtual bool _WasAborted() override { return TilesetPublisher::Status::Success != m_publisher.GetTileStatus(); }
    public:
        explicit ProgressMeter(TilesetPublisher& publisher) : m_publisher(publisher) { }
    };
public:
    TilesetPublisher(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName);

    Status Publish();
    TileGenerator::Status PublishViewedModel (WStringR tileSetName, DgnModelR model);

    Status GetTileStatus() const { return m_acceptTileStatus; }
    TileGeometryCacheP GetGeometryCache() { return nullptr != m_generator ? &m_generator->GetGeometryCache() : nullptr; }
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR  GetTileToEcef() const { return m_tileToEcef; }
    DgnDbR GetDgnDb() { return m_viewController.GetDgnDb(); }

    static Status ConvertStatus(TileGenerator::Status input);
    static TileGenerator::Status ConvertStatus(Status input);
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
    std::ofstream           m_outputFile;
    DPoint3d                m_centroid;
    TileMeshList            m_meshes;
    TileNodeCR              m_tile;
    ByteStream              m_binaryData;
    TilesetPublisher&       m_context;


    static WString GetNodeNameSuffix(TileNodeCR tile);
    static DPoint3d GetCentroid(TileNodeCR tile);
    static void AppendPoint(Json::Value& val, DPoint3dCR pt) { val.append(pt.x); val.append(pt.y); val.append(pt.z); }
    static void WriteBoundingVolume(Json::Value&, TileNodeCR);
    static void AddTechniqueParameter(Json::Value&, Utf8CP name, int type, Utf8CP semantic);
    static void AppendProgramAttribute(Json::Value&, Utf8CP);
    static void AddShader(Json::Value&, Utf8CP name, int type, Utf8CP buffer);
    static Utf8String Concat(Utf8CP prefix, Utf8StringCR suffix) { Utf8String str(prefix); str.append(suffix); return str; }

    WString GetTileMetadataName(TileNodeCR tile) const;
    void ProcessMeshes(Json::Value& value);
    void AddExtensions(Json::Value& value);
    void AddTextures(Json::Value& value, TextureIdToNameMap& texNames);
    Utf8String AddMeshShaderTechnique (Json::Value& rootNode, bool textured, bool transparent);
    Utf8String AddPolylineShaderTechnique (Json::Value& rootNode);

    void AddMesh(Json::Value& value, TileMeshR mesh, size_t index);
    void AppendUInt32(uint32_t value);
    void WriteMetadata(Json::Value&, TileNodeCR, double tolerance, WStringCR b3dmPath);
    Utf8String AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, bool isPolyline, Utf8CP suffix);
    Utf8String AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, Utf8CP suffix);

    template<typename T> void AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData);
public:
    TilePublisher(TileNodeCR tile, TilesetPublisher& context);

    TilesetPublisher::Status Publish();

    BeFileNameCR GetDataDirectory() const { return m_context.GetDataDirectory(); }
    WStringCR GetPrefix() const { return m_context.GetRootName(); }
    TileGeometryCacheR GetGeometryCache() { return *m_context.GetGeometryCache(); }
};


END_BENTLEY_DGN_TILE3D_NAMESPACE

