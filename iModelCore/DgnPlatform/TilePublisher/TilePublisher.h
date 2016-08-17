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
// A cache of textures converted to JPEG.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TextureCache
{
public:
    struct TextureKey
    {
        TileDisplayParams   m_params;

        explicit TextureKey(TileDisplayParamsCP params) { if (nullptr != params) m_params = *params; }
        TextureKey() { }

        bool operator<(TextureKey const& rhs) const { return m_params < rhs.m_params; }
    };

    struct Texture
    {
    private:
        ByteStream          m_data;
        uint32_t            m_width;
        uint32_t            m_height;
    public:
        Texture() { }
        Texture(ByteStream&& data, uint32_t width, uint32_t height) : m_data(std::move(data)), m_width(width), m_height(height) { }

        ByteStream const& GetData() const { return m_data; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
    };
private:
    BeMutex                     m_mutex;
    bvector<Texture>            m_textures;
    bmap<TextureKey, uint32_t>  m_textureMap;
public:
    BeMutex& GetMutex() { return m_mutex; }
    Texture const* GetTextureJPEG(uint32_t textureId) const { return textureId < m_textures.size() ? &m_textures[textureId] : nullptr; }
    void PrepareMeshTextures(TileMeshList& meshes, bvector<uint32_t>& texIds, TileGeometryCacheCR geomCache);
    size_t Count() const { return m_textures.size(); }
};

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
    TextureCache        m_textureCache;
    TileGeneratorP      m_generator = nullptr;
    Status              m_acceptTileStatus = Status::Success;
    Transform           m_dbToTile;
    Transform           m_tileToEcef;

    virtual TileGenerator::Status _AcceptTile(TileNodeCR tile) override;

    Status Setup();
    Status WriteWebApp(TransformCR transform);

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

    Status GetTileStatus() const { return m_acceptTileStatus; }
    TextureCache& GetTextureCache() { return m_textureCache; }
    TileGeometryCacheP GetGeometryCache() { return nullptr != m_generator ? &m_generator->GetGeometryCache() : nullptr; }
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }

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
    bvector<uint32_t>       m_textureIds;
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
    void AddShaders(Json::Value& value, bool isTextured);
    Json::Value AddShaderTechnique (Json::Value& rootNode, bool textured, bool transparent);
    void AddMesh(Json::Value& value, TileMeshR mesh, size_t id, uint32_t texId, TextureIdToNameMap& texNames);
    void AppendUInt32(uint32_t value);
    void WriteMetadata(Json::Value&, TileNodeCR, double tolerance, WStringCR b3dmPath);
    template<typename T> void AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData);
public:
    TilePublisher(TileNodeCR tile, TilesetPublisher& context);

    TilesetPublisher::Status Publish();

    TextureCache& GetTextureCache() { return m_context.GetTextureCache(); }
    BeFileNameCR GetDataDirectory() const { return m_context.GetDataDirectory(); }
    WStringCR GetPrefix() const { return m_context.GetRootName(); }
    TileGeometryCacheR GetGeometryCache() { return *m_context.GetGeometryCache(); }
};

END_BENTLEY_DGN_TILE3D_NAMESPACE

