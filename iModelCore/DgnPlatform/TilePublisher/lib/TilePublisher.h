/*--------------------------------------------------------------------------------------+                  
|
|     $Source: TilePublisher/lib/TilePublisher.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/AutoRestore.h>
#include <DgnPlatform/DgnMaterial.h>
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

typedef BeSQLite::IdSet<DgnViewId> DgnViewIdSet;

struct MeshMaterial;
struct PolylineMaterial;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct ColorIndex
{
    enum class Dimension : uint8_t
    {
        Zero = 0,   // uniform color
        One,        // only one row
        Two,        // more than one row
        None,       // empty
    };
private:
    ByteStream      m_texture;
    uint16_t        m_width = 0;
    uint16_t        m_height = 0;

    void ComputeDimensions(uint16_t nColors);
    void Build(TileMeshCR mesh, MeshMaterial const& mat);
public:
    ColorIndex(TileMeshCR mesh, MeshMaterial const& mat)
        {
        Build(mesh, mat);
        }

    ColorIndex(TileMeshCR mesh, PolylineMaterial const& mat);

    static constexpr uint16_t GetMaxWidth() { return 256; }

    ByteStream const& GetTexture() const { return m_texture; }
    Image ExtractImage() { return Image(GetWidth(), GetHeight(), std::move(m_texture), Image::Format::Rgba); }

    uint16_t GetWidth() const { return m_width; }
    uint16_t GetHeight() const { return m_height; }
    bool empty() const { return m_texture.empty(); }

    Dimension GetDimension() const
        {
        if (empty())
            return Dimension::None;
        else if (GetHeight() > 1)
            return Dimension::Two;
        else
            return GetWidth() > 1 ? Dimension::One : Dimension::Zero;
        }
};

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
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct TileMaterial
{
protected:
    Utf8String              m_name;
    ColorIndex::Dimension   m_colorDimension;
    bool                    m_hasAlpha;

    TileMaterial(Utf8StringCR name) : m_name(name) { }

    void AddColorIndexTechniqueParameters(Json::Value& technique, Json::Value& programRoot, PublishTileData& tileData) const;
public:
    Utf8StringCR GetName() const { return m_name; }
    ColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    bool HasTransparency() const { return m_hasAlpha; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
enum class PolylineType : uint8_t
{
    Simple,
    Tesselated,
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct PolylineMaterial : TileMaterial
{
private:
    PolylineType            m_type;
public:
    PolylineMaterial(TileMeshCR mesh, Utf8CP suffix);

    PolylineType GetType() const { return m_type; }

    std::string const& GetVertexShaderString() const;
    std::string const& GetFragmentShaderString() const;
    Utf8String GetTechniqueNamePrefix() const;

    bool IsSimple() const { return PolylineType::Simple == GetType(); }
    bool IsTesselated() const { return PolylineType::Tesselated == GetType(); }

    void AddTechniqueParameters(Json::Value& technique, Json::Value& programRoot, PublishTileData& tileData) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshMaterial : TileMaterial
{
    static constexpr double GetSpecularFinish() { return 0.9; }
    static constexpr double GetSpecularExponentMult() { return 48.0; }
private:
    TileTextureImageCPtr    m_texture;
    DgnMaterialCPtr         m_material;
    RgbFactor               m_rgbOverride;
    RgbFactor               m_specularColor = { 1.0, 1.0, 1.0 };
    double                  m_alphaOverride;
    double                  m_specularExponent = GetSpecularFinish() * GetSpecularExponentMult();
    bool                    m_overridesAlpha = false;
    bool                    m_overridesRgb = false;
    bool                    m_ignoreLighting;
public:
    MeshMaterial(TileMeshCR mesh, Utf8CP suffix, DgnDbR db);

    bool IsTextured() const { return m_texture.IsValid(); }
    bool HasTransparency() const { return m_hasAlpha; }
    bool IgnoresLighting() const { return m_ignoreLighting; }
    ColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    DgnMaterialCP GetDgnMaterial() const { return m_material.get(); }
    TileTextureImageCPtr GetTexture() const { return m_texture.get(); }

    bool OverridesAlpha() const { return m_overridesAlpha; }
    bool OverridesRgb() const { return m_overridesRgb; }
    double GetAlphaOverride() const { return m_alphaOverride; }
    RgbFactor const& GetRgbOverride() const { return m_rgbOverride; }
    double GetSpecularExponent() const { return m_specularExponent; }
    RgbFactor const& GetSpecularColor() const { return m_specularColor; }

    std::string const& GetVertexShaderString() const;
    std::string const& GetFragmentShaderString() const;
    Utf8String GetTechniqueNamePrefix() const;

    void AddTechniqueParameters(Json::Value& technique, Json::Value& programRoot, PublishTileData& tileData) const;
};

//=======================================================================================
//! Context in which tile publishing occurs.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PublisherContext : TileGenerator::ITileCollector
{
    enum TextureMode
        {
        Embedded = 0,
        External,
        Compressed,         
        };

    enum class Status
        {
        Success = SUCCESS,
        NoGeometry,
        Aborted,
        CantWriteToBaseDirectory,
        CantCreateSubDirectory,
        ErrorWritingScene,
        ErrorWritingNode,
        CantOpenOutputFile,
        };

    struct  Statistics
        {
        TILEPUBLISHER_EXPORT ~Statistics();

        BeMutex     m_mutex;
        double      m_textureCompressionSeconds = 0.0;
        double      m_textureCompressionMegaPixels = 0.0;
        size_t      m_pointCloudCount = 0;
        size_t      m_pointCloudTotalPoints = 0;
        size_t      m_pointCloudMaxPoints = 0;
        size_t      m_pointCloudMinPoints = 0;
        size_t      m_pointCloudTiles = 0;

        void RecordPointCloud (size_t nPoints);
        };
    Statistics                              m_statistics;

protected:
    DgnDbR                                  m_db;
    DgnViewIdSet                            m_viewIds;
    BeFileName                              m_outputDir;
    BeFileName                              m_dataDir;
    WString                                 m_rootName;
    Transform                               m_dbToTile;
    Transform                               m_spatialToEcef;
    Transform                               m_nonSpatialToEcef;
    size_t                                  m_maxTilesetDepth;
    bmap<DgnModelId, DRange3d>              m_modelRanges;
    BeMutex                                 m_mutex;
    bool                                    m_publishSurfacesOnly;
    TextureMode                             m_textureMode;

    TILEPUBLISHER_EXPORT PublisherContext(DgnDbR db, DgnViewIdSet const& viewIds, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation = nullptr, bool publishSurfacesOnly = false, size_t maxTilesetDepth = 5, TextureMode textureMode = TextureMode::Embedded);

    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const = 0;
    virtual bool _AllTilesPublished() const { return false; }   // If all tiles are published then we can write only valid (non-empty) tree leaves and branches.

    TILEPUBLISHER_EXPORT Status InitializeDirectories(BeFileNameCR dataDir);
    TILEPUBLISHER_EXPORT void CleanDirectories(BeFileNameCR dataDir);
    TILEPUBLISHER_EXPORT Status PublishViewModels (TileGeneratorR generator, DRange3dR range, double toleranceInMeters, bool surfacesOnly, ITileGenerationProgressMonitorR progressMeter);

    TILEPUBLISHER_EXPORT void WriteMetadataTree (DRange3dR range, Json::Value& val, TileNodeCR tile, size_t depth);
    TILEPUBLISHER_EXPORT void WriteTileset (BeFileNameCR metadataFileName, TileNodeCR rootTile, size_t maxDepth);
    void WriteModelsJson(Json::Value&, DgnElementIdSet const& allModelSelectors, DgnModelIdSet const& all2dModels);
    void WriteCategoriesJson(Json::Value&, DgnElementIdSet const& allCategorySelectors);
    Json::Value GetDisplayStylesJson(DgnElementIdSet const& styleIds);
    Json::Value GetDisplayStyleJson(DisplayStyleCR style);

    void GenerateJsonAndWriteTileset (Json::Value& rootJson, DRange3dR rootRange, TileNodeCR rootTile, WStringCR name);

    TILEPUBLISHER_EXPORT TileGeneratorStatus _BeginProcessModel(DgnModelCR model) override;
    TILEPUBLISHER_EXPORT TileGeneratorStatus _EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status) override;

    void WriteModelTileset(TileNodeCR rootTile);
public:
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR GetSpatialToEcef() const { return m_spatialToEcef; }
    TransformCR GetNonSpatialToEcef() const { return m_nonSpatialToEcef; }
    DgnDbR GetDgnDb() const { return m_db; }
    size_t GetMaxTilesetDepth() const { return m_maxTilesetDepth; }
    bool WantSurfacesOnly() const { return m_publishSurfacesOnly; }
    TextureMode GetTextureMode() const { return m_textureMode; }

    TILEPUBLISHER_EXPORT static Status ConvertStatus(TileGeneratorStatus input);
    TILEPUBLISHER_EXPORT static TileGeneratorStatus ConvertStatus(Status input);

    WString GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const { return _GetTileUrl(tile, fileExtension); }
    TILEPUBLISHER_EXPORT BeFileName GetDataDirForModel(DgnModelCR model, WStringP rootName=nullptr) const;
    TILEPUBLISHER_EXPORT Status GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId);
    TILEPUBLISHER_EXPORT void GetViewJson (Json::Value& json, ViewDefinitionCR view, TransformCR transform);
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

    static Json::Value TransformToJson(TransformCR);
    static Json::Value RangeToJson(DRange3dCR range)
        {
        Json::Value json(Json::objectValue);
        json["low"] = PointToJson(range.low);
        json["high"] = PointToJson(range.high);
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
    DPoint3d                m_centroid;
    TileNodeCR              m_tile;
    PublisherContext&       m_context;
    bmap<TileTextureImageCP, Utf8String>    m_textureImages;

    enum class VertexEncoding
        {
        StandardQuantization,
        UnquantizedDoubles,
        OctEncodedNormals
        };

    static WString GetNodeNameSuffix(TileNodeCR tile);
    static DPoint3d GetCentroid(TileNodeCR tile);
    static void AppendPoint(Json::Value& val, DPoint3dCR pt) { val.append(pt.x); val.append(pt.y); val.append(pt.z); }
    static void AddShader(Json::Value&, Utf8CP name, int type, Utf8CP buffer);
    static Utf8String Concat(Utf8CP prefix, Utf8StringCR suffix) { Utf8String str(prefix); str.append(suffix); return str; }

    void WritePointCloud (std::FILE* outputFile, TileMeshPointCloudR pointCloud);
    void WriteTileMeshes (std::FILE* outputFile, PublishableTileGeometryR geometry);
    void WriteBatched3dModel (std::FILE* outputFile, TileMeshList const&  meshes);
    void WritePartInstances(std::FILE* outputFile, DRange3dR publishedRange, TileMeshPartPtr& part);
    void WriteGltf(std::FILE* outputFile, PublishTileData tileData);

    void AddMeshes(PublishTileData& tileData, TileMeshList const&  geometry);
    void AddDefaultScene (PublishTileData& tileData);
    void AddExtensions(PublishTileData& tileData);
    void AddTextures(PublishTileData& tileData, TextureIdToNameMap& texNames);
    Utf8String AddMeshVertexAttributes  (PublishTileData& tileData, double const* values, Utf8CP name, Utf8CP id, size_t nComponents, size_t nAttributes, char const* accessorType, VertexEncoding encoding, double const* min, double const* max);
    void AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange);
    Utf8String AddMeshIndices(PublishTileData& tileData, Utf8CP name, bvector<uint32_t> const& indices, Utf8StringCR idStr);

    void AddMeshUInt16Attributes(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& attributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void AddMeshBatchIds (PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& attributes, Utf8StringCR idStr);
    void AddMeshColors(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& colors, Utf8StringCR idStr);

    Json::Value CreateMesh (TileMeshList const& tileMeshes, PublishTileData& tileData, size_t& primitiveIndex);
    BeFileName  GetBinaryDataFileName() const;
    Utf8String AddMeshShaderTechnique(PublishTileData& tileData, MeshMaterial const& material, bool doBatchIds);
    void AddMeshPrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddSimplePolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddTesselatedPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void TesselatePolylineSegment(bvector<DPoint3d>& origins, bvector<DVec3d>& directions, bvector<DPoint2d>& params, bvector<uint16_t>& colors, bvector<uint16_t>& attributes, bvector<uint32_t>& indices, DPoint3dCR p0, DPoint3dCR p1, DPoint3dCR p2, double& currLength, TileMeshR mesh, size_t meshIndex, bool doBatchIds);

    MeshMaterial AddMeshMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);

    PolylineMaterial AddSimplePolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    PolylineMaterial AddTesselatedPolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    PolylineMaterial AddPolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    Utf8String AddPolylineTechnique(PublishTileData& tileData, PolylineMaterial const& mat, bool doBatchIds);

    Utf8String AddTextureImage (PublishTileData& tileData, TileTextureImageCR textureImage, TileMeshCR mesh, Utf8CP suffix);
    Utf8String AddColorIndex(PublishTileData& tileData, ColorIndex& colorIndex, TileMeshCR mesh, Utf8CP suffix);
public:
    TILEPUBLISHER_EXPORT TilePublisher(TileNodeCR tile, PublisherContext& context);
    TILEPUBLISHER_EXPORT PublisherContext::Status Publish();

    BeFileNameCR GetDataDirectory() const { return m_context.GetDataDirectory(); }
    WStringCR GetPrefix() const { return m_context.GetRootName(); }
    TILEPUBLISHER_EXPORT static void WriteBoundingVolume(Json::Value&, DRange3dCR);
    static WCharCP GetBinaryDataFileExtension(bool containsParts) { return containsParts ? L"cmpt" : L"b3dm"; }

    static void AddTechniqueParameter(Json::Value&, Utf8CP name, int type, Utf8CP semantic);
    static void AppendProgramAttribute(Json::Value&, Utf8CP);
};

END_BENTLEY_DGN_TILE3D_NAMESPACE

