/*--------------------------------------------------------------------------------------+                  
|
|     $Source: PublicAPI/TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/AutoRestore.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/ModelSpatialClassifier.h>
#include <stdio.h>

#if defined(__TILEPUBLISHER_LIB_BUILD__)
    #define TILEPUBLISHER_EXPORT EXPORT_ATTRIBUTE
#else
    #define TILEPUBLISHER_EXPORT IMPORT_ATTRIBUTE
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_RENDER

#define BEGIN_BENTLEY_TILEPUBLISHER_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace TilePublish {
#define END_BENTLEY_TILEPUBLISHER_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_BENTLEY_TILEPUBLISHER using namespace BentleyApi::Dgn::TilePublish;

BEGIN_BENTLEY_TILEPUBLISHER_NAMESPACE

typedef BeSQLite::IdSet<DgnViewId> DgnViewIdSet;

struct MeshMaterial;
struct PolylineMaterial;
struct TileMaterial;

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
        Background, // use background color.
        None,       // empty
    };
private:
    ByteStream      m_texture;
    uint16_t        m_width = 0;
    uint16_t        m_height = 0;

    void ComputeDimensions(uint16_t nColors);
    void Build(TileMeshCR mesh, TileMaterial const& mat);
public:
    ColorIndex(TileMeshCR mesh, TileMaterial const& mat)
        {
        Build(mesh, mat);
        }

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

    static Dimension CalcDimension(uint16_t nColors)
        {
        switch (nColors)
            {
            case 0:     return Dimension::None;
            case 1:     return Dimension::Zero;
            default:    return nColors <= GetMaxWidth() ? Dimension::One : Dimension::Zero;
            }
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
    Utf8String GetJsonString() const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct TileMaterial
{
protected:
    Utf8String              m_name;
    ColorIndex::Dimension   m_colorDimension;
    bool                    m_hasAlpha = false;
    TileTextureImageCPtr    m_texture;
    bool                    m_overridesAlpha = false;
    bool                    m_overridesRgb = false;
    RgbFactor               m_rgbOverride;
    double                  m_alphaOverride;
    bool                    m_adjustColorForBackground = false;

    TileMaterial(Utf8StringCR name) : m_name(name) { }

    void AddColorIndexTechniqueParameters(Json::Value& technique, Json::Value& program, PublishTileData& data) const;
    void AddTextureTechniqueParameters(Json::Value& technique, Json::Value& program, PublishTileData& data) const;

    virtual std::string _GetVertexShaderString() const = 0;

public:
    Utf8StringCR GetName() const { return m_name; }
    ColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    bool HasTransparency() const { return m_hasAlpha; }
    bool IsTextured() const { return m_texture.IsValid(); }
    TileTextureImageCPtr GetTexture() const { return m_texture.get(); }

    bool OverridesAlpha() const { return m_overridesAlpha; }
    bool OverridesRgb() const { return m_overridesRgb; }
    double GetAlphaOverride() const { return m_alphaOverride; }
    RgbFactor const& GetRgbOverride() const { return m_rgbOverride; }
    std::string GetVertexShaderString() const;
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
    double                  m_width;
    double                  m_textureLength;       // If positive, meters, if negative, pixels (Cosmetic).
protected:
    virtual std::string _GetVertexShaderString() const override;
public:
    PolylineMaterial(TileMeshCR mesh, bool is3d, Utf8CP suffix);

    PolylineType GetType() const { return m_type; }

    std::string const& GetFragmentShaderString() const;
    bool IsSimple() const { return PolylineType::Simple == GetType(); }
    bool IsTesselated() const { return PolylineType::Tesselated == GetType(); }
    Utf8String GetTechniqueNamePrefix();
    void AddTechniqueParameters(Json::Value& technique, Json::Value& programRoot, PublishTileData& tileData) const;
    Utf8String GetTechniqueNamePrefix() const;
    double GetWidth() const { return m_width; }
    double GetTextureLength() const { return m_textureLength; }

};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshMaterial : TileMaterial
{
    static constexpr double GetSpecularFinish() { return 0.9; }
    static constexpr double GetSpecularExponentMult() { return 48.0; }
private:
    RenderMaterialCPtr      m_material;
    RgbFactor               m_specularColor = { 1.0, 1.0, 1.0 };
    double                  m_specularExponent = GetSpecularFinish() * GetSpecularExponentMult();
    bool                    m_ignoreLighting;
protected:
    virtual std::string _GetVertexShaderString() const override;
public:
    MeshMaterial(TileMeshCR mesh, bool is3d, Utf8CP suffix, DgnDbR db);

    bool HasTransparency() const { return m_hasAlpha; }
    bool IgnoresLighting() const { return m_ignoreLighting; }
    ColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    RenderMaterialCP GetDgnMaterial() const { return m_material.get(); }
    double GetSpecularExponent() const { return m_specularExponent; }
    RgbFactor const& GetSpecularColor() const { return m_specularColor; }

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
    struct ModelRange
    {
        DRange3d    m_range;
        bool        m_isEcef;

        ModelRange() : m_isEcef(false) { }
        explicit ModelRange(DRange3dCR range, bool isEcef=false) : m_range(range), m_isEcef(isEcef) { }
    };

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

    struct  ClassifierInfo
        {
        uint32_t                                m_index;
        ModelSpatialClassifier                  m_classifier;
        ModelRange                              m_classifiedRange;    

        WString GetRootName() const { return WPrintfString(L"Classifier_%d", m_index); }
        ClassifierInfo(ModelSpatialClassifierCR classifier, ModelRange const& range, uint32_t index) : m_classifier(classifier), m_classifiedRange(range), m_index(index) { }
        };

    typedef bvector<ClassifierInfo>             T_ClassifierInfos;
    typedef bvector<ViewDefinitionCPtr>         T_ViewDefs;
    typedef bmap<DgnElementId, T_ViewDefs>      T_CategorySelectorMap;
    typedef bmap<DgnElementId, uint32_t>        T_ScheduleEntryMap;
    typedef bvector<T_ScheduleEntryMap>         T_ScheduleEntryMaps;

    Statistics                                  m_statistics;

protected:
    DgnDbR                                      m_db;
    DgnViewIdSet                                m_viewIds;
    BeFileName                                  m_outputDir;
    BeFileName                                  m_dataDir;
    WString                                     m_rootName;
    Transform                                   m_dbToTile;
    Transform                                   m_spatialToEcef;
    size_t                                      m_maxTilesetDepth;
    bmap<DgnModelId, ModelRange>                m_modelRanges;
    BeMutex                                     m_mutex;
    bool                                        m_publishSurfacesOnly;
    TextureMode                                 m_textureMode;
    bmap<DgnModelId, T_ClassifierInfos>         m_classifierMap;
    bmap<DgnModelId, Utf8String>                m_directUrls;
    AxisAlignedBox3d                            m_projectExtents; // ###TODO: Remove once ScalableMesh folks fix their _QueryModelRange() to produce valid result during conversion from V8
    bool                                        m_isEcef; // Hack for ScalableMeshes at YII...all coords in .bim already in ECEF, but nothing in .bim tells us that...
    bool                                        m_isGeoLocated;
    ITileGenerationFilterP                      m_generationFilter;
    ClassifierInfo*                             m_currentClassifier;
    bset<DgnSubCategoryId>                      m_usedSubCategories;
    Json::Value                                 m_schedulesJson;
    T_ScheduleEntryMaps                         m_scheduleEntryMaps;


    TILEPUBLISHER_EXPORT PublisherContext(DgnDbR db, DgnViewIdSet const& viewIds, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation = nullptr, bool publishSurfacesOnly = false, size_t maxTilesetDepth = 5, TextureMode textureMode = TextureMode::Embedded);

    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension, ClassifierInfo const* classifierInfo) const = 0;
    virtual bool _AllTilesPublished() const { return false; }   // If all tiles are published then we can write only valid (non-empty) tree leaves and branches.

    TILEPUBLISHER_EXPORT Status InitializeDirectories(BeFileNameCR dataDir);
    TILEPUBLISHER_EXPORT void CleanDirectories(BeFileNameCR dataDir);
    TILEPUBLISHER_EXPORT Status PublishViewModels (TileGeneratorR generator, DRange3dR range, double toleranceInMeters, bool surfacesOnly, ITileGenerationProgressMonitorR progressMeter);
    Status PublishClassifiers (DgnModelIdSet const& viewedModels, TileGeneratorR generator, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter);
    Status PublishClassifier(ClassifierInfo& classifierInfo, TileGeneratorR generator, double toleranceInMeters, ITileGenerationProgressMonitorR progressMeter);


    TILEPUBLISHER_EXPORT void WriteModelMetadataTree (DRange3dR range, Json::Value& val, TileNodeCR tile, size_t depth);
    TILEPUBLISHER_EXPORT void WriteTileset (BeFileNameCR metadataFileName, TileNodeCR rootTile, size_t maxDepth);
    Json::Value GetViewAttachmentsJson(Sheet::ModelCR sheet, DgnModelIdSet& attachedModels);
    void AddModelJson(Json::Value& modelsJson, DgnModelId modelId, DgnModelIdSet const& modelIds);
    WString GetRootName (DgnModelId modelId, ClassifierInfo const* classifier) const;
    ClassifierInfo const* GetCurrentClassifier() const { return m_currentClassifier; }


    void WriteModelsJson(Json::Value&, DgnElementIdSet const& allModelSelectors, DgnModelIdSet const& all2dModels);
    void WriteCategoriesJson(Json::Value&, T_CategorySelectorMap const&);
    Json::Value GetDisplayStylesJson(DgnElementIdSet const& styleIds);
    Json::Value GetDisplayStyleJson(DisplayStyleCR style);
    Json::Value GetClassifiersJson(T_ClassifierInfos const& classifiers);
    Json::Value GetAllClassifiersJson();
    bool CategoryOnInAnyView(DgnCategoryId categoryId, PublisherContext::T_ViewDefs views) const;
    void GenerateJsonAndWriteTileset (Json::Value& rootJson, DRange3dR rootRange, TileNodeCR rootTile, WStringCR name);

    TILEPUBLISHER_EXPORT TileGeneratorStatus _BeginProcessModel(DgnModelCR model) override;
    TILEPUBLISHER_EXPORT TileGeneratorStatus _EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status) override;

    BeFileName GetTilesetFileName(DgnModelId modelId);
    Utf8String GetTilesetName(DgnModelId modelId, ClassifierInfo const* classifier);
    void WriteModelTileset(TileNodeCR tile);
    void AddViewedModel(DgnModelIdSet& viewedModels, DgnModelId modelId);
    void GetViewedModelsFromView (DgnModelIdSet& viewedModels, DgnViewId viewId);
    void ExtractSchedules();


public:
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR GetSpatialToEcef() const { return m_spatialToEcef; }
    DgnDbR GetDgnDb() const { return m_db; }
    size_t GetMaxTilesetDepth() const { return m_maxTilesetDepth; }
    bool WantSurfacesOnly() const { return m_publishSurfacesOnly; }
    TextureMode GetTextureMode() const { return m_textureMode; }
    bool DoPublishAsClassifier() const { return nullptr != m_currentClassifier; }
    WString GetTileExtension (TileNodeCR tile);
    ITileGenerationFilterP GetGenerationFilter() { return m_generationFilter; }
    T_ScheduleEntryMaps& GetScheduleEntryMaps() { return m_scheduleEntryMaps; }
    ClassifierInfo* GetCurrentClassifier() { return m_currentClassifier; }
    void RecordUsage(FeatureAttributesMapCR attributes);

    bool IsSubCategoryUsed(DgnSubCategoryId subCategoryId) const { return m_usedSubCategories.find(subCategoryId) != m_usedSubCategories.end(); }

    TILEPUBLISHER_EXPORT static Status ConvertStatus(TileGeneratorStatus input);
    TILEPUBLISHER_EXPORT static TileGeneratorStatus ConvertStatus(Status input);

    WString GetTileUrl(TileNodeCR tile, WCharCP fileExtension, ClassifierInfo const* classifier) const { return _GetTileUrl(tile, fileExtension, classifier); }
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
    struct CompareImageP { bool operator() (const TileTextureImageCPtr& p0, const TileTextureImageCPtr& p1) const { return p0.get() < p1.get(); } };

    typedef bmap<TileTextureImageCPtr, Utf8String, CompareImageP>  TextureImageMap;

private:
    DPoint3d                m_centroid;
    TileNodeCR              m_tile;
    PublisherContext&       m_context;
    TextureImageMap         m_textureImages;

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
    void WriteGltf(std::FILE* outputFile, PublishTileData const& tileData);
    void WriteClassifier(std::FILE* outputFile, PublishableTileGeometryR geometry, ModelSpatialClassifierCR classifier, DRange3dCR classifiedRange);

    void AddMeshes(PublishTileData& tileData, TileMeshList const&  geometry);
    void AddDefaultScene (PublishTileData& tileData);
    void AddExtensions(PublishTileData& tileData);
    Utf8String AddMeshVertexAttributes  (PublishTileData& tileData, double const* values, Utf8CP name, Utf8CP id, size_t nComponents, size_t nAttributes, char const* accessorType, VertexEncoding encoding, double const* min, double const* max);
    void AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange);
    Utf8String AddMeshIndices(PublishTileData& tileData, Utf8CP name, bvector<uint32_t> const& indices, Utf8StringCR idStr);

    void AddMeshUInt16Attributes(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& attributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void AddMeshUInt32Attributes(PublishTileData& tileData, Json::Value& primitive, bvector<uint32_t> const& attributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void AddMeshBatchIds (PublishTileData& tileData, Json::Value& primitive, bvector<uint32_t> const& attributes, Utf8StringCR idStr);
    void AddMeshColors(PublishTileData& tileData, Json::Value& primitive, bvector<uint16_t> const& colors, Utf8StringCR idStr);

    Json::Value CreateMesh (TileMeshList const& tileMeshes, PublishTileData& tileData, size_t& primitiveIndex);
    BeFileName  GetBinaryDataFileName() const;
    Utf8String AddMeshShaderTechnique(PublishTileData& tileData, MeshMaterial const& material, bool doBatchIds);
    void AddMeshPrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddSimplePolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void AddTesselatedPolylinePrimitive(Json::Value& primitivesNode, PublishTileData& tileData, TileMeshR mesh, size_t index, bool doBatchIds);
    void TesselatePolylineSegment(bvector<DPoint3d>& origins, bvector<DVec3d>& directions, bvector<DPoint2d>& params, bvector<uint16_t>& colors, bvector<uint16_t>& attributes, bvector<uint32_t>& indices, DPoint3dCR p0, DPoint3dCR p1, DPoint3dCR p2, double& currLength, TileMeshR mesh, size_t meshIndex, bool doBatchIds);
    MeshMaterial AddMeshMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchids);
    void  AddMaterialColor(Json::Value& matJson, TileMaterial& mat, PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix);
    PolylineMaterial AddSimplePolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    PolylineMaterial AddTesselatedPolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    PolylineMaterial AddPolylineMaterial(PublishTileData& tileData, TileMeshCR mesh, Utf8CP suffix, bool doBatchIds);
    Utf8String AddPolylineTechnique(PublishTileData& tileData, PolylineMaterial const& mat, bool doBatchIds);

    Utf8String AddTextureImage (PublishTileData& tileData, TileTextureImageCPtr& textureImage, TileMeshCR mesh, Utf8CP suffix);
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

END_BENTLEY_TILEPUBLISHER_NAMESPACE

