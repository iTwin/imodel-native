/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//#include <DgnPlatform/DgnPlatformApi.h>
//#include <DgnPlatform/DgnPlatformLib.h>
//#include <DgnPlatform/DgnGeoCoord.h>
//#include <DgnPlatform/AutoRestore.h>
#include <stdio.h>

#if defined(__TILEPUBLISHER_LIB_BUILD__)
    #define TILEPUBLISHER_EXPORT EXPORT_ATTRIBUTE
#else
    #define TILEPUBLISHER_EXPORT /*IMPORT_ATTRIBUTE*/
#endif

USING_NAMESPACE_BENTLEY

//#define BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE BEGIN_BENTLEY_RENDER_NAMESPACE namespace Tile3d {
//#define END_BENTLEY_DGN_TILE3D_NAMESPACE } END_BENTLEY_RENDER_NAMESPACE
//
//BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE

namespace BentleyG06 
    {
    //=======================================================================================
    // Base class for 64 bit Ids.
    // @bsiclass                                                    Keith.Bentley   02/11
    //=======================================================================================
    struct BeInt64Id
        {
        public:
            //! @see BeInt64Id::ToString(Utf8Char*)
            static const size_t ID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

        protected:
            uint64_t m_id;

        public:
            //! Construct an invalid BeInt64Id
            BeInt64Id() { Invalidate(); }

            //! Construct a BeInt64Id from a 64 bit value.
            explicit BeInt64Id(uint64_t u) : m_id(u) {}

            //! Move constructor.
            BeInt64Id(BeInt64Id&& rhs) { m_id = rhs.m_id; }

            //! Construct a copy.
            BeInt64Id(BeInt64Id const& rhs) { m_id = rhs.m_id; }

            BeInt64Id& operator=(BeInt64Id const& rhs) { m_id = rhs.m_id; return *this; }

            bool IsValid() const { return Validate(); }

            //! Compare two BeInt64Id for equality
            bool operator==(BeInt64Id const& rhs) const { return rhs.m_id == m_id; }

            //! Compare two BeInt64Id for inequality
            bool operator!=(BeInt64Id const& rhs) const { return !(*this == rhs); }

            //! Compare two BeInt64Id
            bool operator<(BeInt64Id const& rhs) const { return m_id < rhs.m_id; }
            bool operator<=(BeInt64Id const& rhs) const { return m_id <= rhs.m_id; }
            bool operator>(BeInt64Id const& rhs) const { return m_id > rhs.m_id; }
            bool operator>=(BeInt64Id const& rhs) const { return m_id >= rhs.m_id; }

            //! Get the 64 bit value of this BeInt64Id
            uint64_t GetValue() const { BeAssert(IsValid()); return m_id; }

            //! Get the 64 bit value of this BeGuid. Does not check for valid value in debug builds.
            uint64_t GetValueUnchecked() const { return m_id; }

            //! Test to see whether this BeInt64Id is valid. 0 is not a valid id.
            bool Validate() const { return m_id != 0; }

            //! Set this BeInt64Id to an invalid value (0).
            void Invalidate() { m_id = 0; }

            //! Converts this BeInt64Id to its string representation.
            //! 
            //! Typical example:
            //!
            //!     Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
            //!     myId.ToString(idStrBuffer);
            //!
            //! @remarks The method does not have any checks that the buffer is large enough. Callers
            //! must ensure this to avoid unexpected behavior. 
            //!
            //! @param[in,out] stringBuffer The output buffer for the id string. Must be large enough
            //! to hold the maximal number of decimal digits of UInt64 plus the trailing 0 character.
            //! You can use BeInt64Id::ID_STRINGBUFFER_LENGTH to allocate the @p stringBuffer.
            void ToString(Utf8P stringBuffer) const { BeStringUtilities::FormatUInt64(stringBuffer, m_id); } //BeStringUtilities::FormatUInt64 is faster than sprintf.

                                                                                                             //! Converts this BeInt64Id to its string representation.
                                                                                                             //! @remarks Consider the overload BeInt64Id::ToString(Utf8Char*) if you want
                                                                                                             //! to avoid allocating Utf8Strings.
            Utf8String ToString() const
                {
                Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
                ToString(idStrBuffer);
                return Utf8String(idStrBuffer);
                }
        };
    }
//=======================================================================================
//! Describes the type of entity from which a tile node or mesh was produced.
//! The IDs of the source entities are recorded in the TileMesh as BeInt64Ids.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
enum class TileSource
    {
    Element,    //!< Geometry in the tile is associated with DgnElementIds
    Model,      //!< Geometry in the tile is associated with DgnModelIds
    None,       //!< No IDs are associated with geometry in the tile
    };

//=======================================================================================
// Maps elements associated with vertices to indexes into a batch table in the b3dm.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct BatchIdMap
{
private:
    bmap<BentleyG06::BeInt64Id, uint16_t>   m_map;
    bvector<BentleyG06::BeInt64Id>          m_list;
    TileSource                  m_source;
public:
    BatchIdMap(TileSource source);

    uint16_t GetBatchId(BentleyG06::BeInt64Id entityId);
    void ToJson(Json::Value& value, DgnDbR db) const;
    uint16_t Count() const { return static_cast<uint16_t>(m_list.size()); }
};

//=======================================================================================
//! A stream of bytes in a resizeable buffer. Released on destruction, never gets smaller.
//! This class is more efficient than bvector<byte> since it does not initialize the memory to zeros.
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct ByteStream
    {
    private:
        uint32_t m_size;
        uint32_t m_allocSize;
        uint8_t* m_data;
        void swap(ByteStream& rhs) { std::swap(m_size, rhs.m_size); std::swap(m_allocSize, rhs.m_allocSize); std::swap(m_data, rhs.m_data); }

    public:
        void Init() { m_size = m_allocSize = 0; m_data = nullptr; }
        ByteStream() { Init(); }
        explicit ByteStream(uint32_t size) { Init(); Resize(size); }
        ByteStream(uint8_t const* data, uint32_t size) { Init(); SaveData(data, size); }
        ByteStream(ByteStream const& other) { Init(); SaveData(other.m_data, other.m_size); }
        ~ByteStream() { Clear(); }
        ByteStream(ByteStream&& rhs) : m_size(rhs.m_size), m_allocSize(rhs.m_allocSize), m_data(rhs.m_data) { rhs.m_size = rhs.m_allocSize = 0; rhs.m_data = nullptr; }
        ByteStream& operator=(ByteStream const& other) { if (this != &other) SaveData(other.m_data, other.m_size); return *this; }
        ByteStream& operator=(ByteStream&& rhs) { ByteStream(std::move(rhs)).swap(*this); return *this; }

        //! Get the size, in bytes, of the memory allocated for this ByteStream.
        //! @note The allocated size may be larger than the currently used size returned by GetSize.
        uint32_t GetAllocSize() const { return m_allocSize; }
        uint32_t GetSize() const { return m_size; }   //!< Get the size in bytes of the current data in this ByteStream.
        uint8_t const* GetData() const { return m_data; } //!< Get a const pointer to the ByteStream.
        uint8_t* GetDataP() const { return m_data; }      //!< Get a writable pointer to the ByteStream.
        bool HasData() const { return 0 != m_size; }  //!< return false if this ByteStream is empty.
        void Clear() { FREE_AND_CLEAR(m_data); m_size = m_allocSize = 0; } //!< Return this object to an empty/uninitialized state.
        uint8_t* ExtractData() { uint8_t* data = m_data; m_data = nullptr; m_size = m_allocSize = 0; return data; }

        //! Reserve memory for this ByteStream. The stream capacity will change but not its size.
        //! @param[in] size the number of bytes to reserve
        void Reserve(uint32_t size) { if (size <= m_allocSize) return; m_data = (uint8_t*)realloc(m_data, size); m_allocSize = size; }

        //! Resize the stream. If more memory is required, the new portion won't be initialized.
        //! @param[in] newSize number of bytes
        void Resize(uint32_t newSize) { Reserve(newSize); m_size = newSize; }

        //! Save a stream of bytes into this ByteStream.
        //! @param[in] data the data to save
        //! @param[in] size number of bytes in data
        void SaveData(uint8_t const* data, uint32_t size) { m_size = 0; Append(data, size); }

        //! Append a stream of byes to the current end of this ByteStream.
        //! @param[in] data the data to save
        //! @param[in] size number of bytes in data
        void Append(uint8_t const* data, uint32_t size)
            {
            if (data)
                {
                Reserve(m_size + size);
                memcpy(m_data + m_size, data, size);
                m_size += size;
                }
            }

        bool empty() const { return !HasData(); }
        size_t size() const { return GetSize(); }
        size_t capacity() const { return GetAllocSize(); }
        void reserve(size_t size) { Reserve(static_cast<uint32_t>(size)); }
        void resize(size_t newSize) { Resize(static_cast<uint32_t>(newSize)); }
        void clear() { Clear(); }
        uint8_t const* data() const { return GetData(); }
        uint8_t* data() { return GetDataP(); }

        typedef uint8_t* iterator;
        typedef uint8_t const* const_iterator;

        iterator begin() { return data(); }
        iterator end() { return data() + size(); }
        const_iterator begin() const { return data(); }
        const_iterator end() const { return data() + size(); }
        uint8_t const& operator[](size_t i) const { return data()[i]; }
        uint8_t& operator[](size_t i) { return data()[i]; }
    };

struct TileNode;
typedef TileNode* TileNodeP, &TileNodeR;
typedef TileNode const* TileNodeCP;
typedef TileNode const& TileNodeCR;
typedef RefCountedPtr<TileNode> TileNodePtr;
typedef RefCountedCPtr<TileNode> TileNodeCPtr;

typedef bvector<TileNodePtr> TileNodeList;
typedef bvector<TileNodeP>   TileNodePList;

struct TileNode : RefCountedBase
    {
    protected:
        DRange3d            m_dgnRange;
        TileNodeList        m_children;
        size_t              m_depth;
        size_t              m_siblingIndex;
        double              m_tolerance;
        TileNodeP           m_parent;
        WString             m_subdirectory;
        Transform           m_transformFromDgn;
        mutable DRange3d    m_publishedRange;

        TileNode(TransformCR transformFromDgn) : TileNode(DRange3d::NullRange(), transformFromDgn, 0, 0, nullptr) { }
        TileNode(DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
            : m_dgnRange(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent), m_transformFromDgn(transformFromDgn), m_publishedRange(DRange3d::NullRange()) { }

        TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }

        virtual TileSource _GetSource() const = 0;
        //virtual TileMeshList _GenerateMeshes(TileGenerationCacheCR cache, DgnDbR dgndb, TileGeometry::NormalMode normalMode = TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles = false, bool doPolylines = false) const = 0;
    public:
        DRange3dCR GetDgnRange() const { return m_dgnRange; }
        DRange3d GetTileRange() const { DRange3d range = m_dgnRange; m_transformFromDgn.Multiply(range, range); return range; }
        DPoint3d GetTileCenter() const { DRange3d range = GetTileRange(); return DPoint3d::FromInterpolate(range.low, .5, range.high); }
        size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
        size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
        double GetTolerance() const { return m_tolerance; }

        TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
        TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node
        TileNodeList const& GetChildren() const { return m_children; } //!< The direct children of this node
        TileNodeList& GetChildren() { return m_children; } //!< The direct children of this node
        WStringCR GetSubdirectory() const { return m_subdirectory; }
        void SetSubdirectory(WStringCR subdirectory) { m_subdirectory = subdirectory; }
        void SetDgnRange(DRange3dCR range) { m_dgnRange = range; }
        void SetTileRange(DRange3dCR range) { Transform tf; DRange3d dgnRange = range; tf.InverseOf(m_transformFromDgn); tf.Multiply(dgnRange, dgnRange); SetDgnRange(dgnRange); }
        void SetPublishedRange(DRange3dCR publishedRange) const { m_publishedRange = publishedRange; }
        DRange3dCR GetPublishedRange() const { return m_publishedRange; }

        size_t GetNodeCount() const;
        size_t GetMaxDepth() const;
        void GetTiles(TileNodePList& tiles);
        TileNodePList GetTiles();
        WString GetNameSuffix() const;
        size_t   GetNameSuffixId() const;
        BeFileNameStatus GenerateSubdirectories(size_t maxTilesPerDirectory, BeFileNameCR dataDirectory);
        WString GetRelativePath(WCharCP rootName, WCharCP extension) const;

        TileSource GetSource() const { return _GetSource(); }
        //TileMeshList GenerateMeshes(TileGenerationCacheCR cache, DgnDbR dgndb, TileGeometry::NormalMode normalMode = TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles = false, bool doPolylines = false) const
        //    {
        //    return _GenerateMeshes(cache, dgndb, normalMode, twoSidedTriangles, doPolylines);
        //    }
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
    bool                m_publishPolylines;

    TILEPUBLISHER_EXPORT PublisherContext(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation = nullptr, bool publishPolylines = false, size_t maxTilesetDepth = 5, size_t maxTilesPerDirectory = 5000);

    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const = 0;
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
    BeFileNameCR GetDataDirectory() const { return m_dataDir; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetRootName() const { return m_rootName; }
    TransformCR  GetTileToEcef() const { return m_tileToEcef; }
    TransformCR  GetTilesetTransform () const { return m_tilesetTransform; }
    ViewControllerCR GetViewController() const { return m_viewController; }
//    DgnDbR GetDgnDb() const { return m_viewController.GetDgnDb(); }
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
//    TileMeshList            m_meshes;
    TileNodeCR              m_tile;
    ByteStream              m_binaryData;
    PublisherContext&       m_context;
    //bmap<TileTextureImageCP, Utf8String>    m_textureImages;

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

//    void AddMesh(Json::Value& value, TileMeshR mesh, size_t index);
    void AppendUInt32(uint32_t value);

//    Utf8String AddMaterial (Json::Value& rootNode, TileDisplayParamsCP displayParams, TileMeshCR mesh, Utf8CP suffix);
//    Utf8String AddTextureImage (Json::Value& rootNode, TileTextureImageCR textureImage, TileMeshCR mesh, Utf8CP suffix);

    template<typename T> void AddBufferView(Json::Value& views, Utf8CP name, T const& bufferData);

public:
    TILEPUBLISHER_EXPORT TilePublisher(TileNodeCR tile, PublisherContext& context);

    TILEPUBLISHER_EXPORT PublisherContext::Status Publish();

    BeFileNameCR GetDataDirectory() const { return m_context.GetDataDirectory(); }
    WStringCR GetPrefix() const { return m_context.GetRootName(); }
    TILEPUBLISHER_EXPORT static void WriteBoundingVolume(Json::Value&, DRange3dCR);
    TILEPUBLISHER_EXPORT static void WriteJsonToFile (WCharCP fileName, Json::Value& value);
};


//END_BENTLEY_DGN_TILE3D_NAMESPACE

