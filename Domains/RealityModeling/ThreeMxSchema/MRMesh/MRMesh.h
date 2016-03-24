/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMesh.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshContext)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshNode)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshGeometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshTexture)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshCacheManager)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshScene)

DEFINE_REF_COUNTED_PTR(MRMeshNode)
DEFINE_REF_COUNTED_PTR(MRMeshGeometry)
DEFINE_REF_COUNTED_PTR(MRMeshTexture)

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshGeometry : RefCountedBase, NonCopyableClass
{
private:
    bvector<FPoint3d> m_points;
    bvector<FPoint3d> m_normals;
    bvector<FPoint2d> m_params;
    bvector<int32_t> m_indices;
    Render::GraphicPtr m_graphic;

public:
    MRMeshGeometry(int nbVertices, FPoint3d const* positions, FPoint3d const* normals, int nbTriangles, uint32_t const* indices, FPoint2d const* textureUV, TextureCP texture, SystemP target);
    PolyfaceHeaderPtr GetPolyface() const;

    void Draw(RenderContextR, MRMeshNodeR node, MRMeshContextCR meshContext);
    BentleyStatus GetRange(DRange3dR range, TransformCR transform) const;

    size_t GetMemorySize() const;
    void ClearGraphic() {m_graphic = nullptr;}
    bool IsCached() const {return m_graphic.IsValid();}
//    bool IsDisplayable() const {return m_textureId >= 0;}
};

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshTexture : RefCountedBase, NonCopyableClass
{
private:
    ByteStream m_compressedData;
    Point2d m_size;
    TexturePtr m_texture;

public:
    MRMeshTexture(ByteCP pData, uint32_t dataSize);
    void Initialize(MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext);
    void Activate(ViewContextR viewContext);
    size_t GetMemorySize() const;
    bool IsInitialized() const;
    Point2d GetSize() const {return m_size;}
};
#endif

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshContext
{
    Transform      m_transform;
    bool           m_loadSynchronous;
    bool           m_useFixedResolution;
    double         m_fixedResolution;
    mutable size_t m_lastPumpTicks;
    mutable size_t m_nodeCount;
    mutable size_t m_pointCount;

    MRMeshContext(TransformCR transform, ViewContextR viewContext, double fixedResolution);
    TransformCR GetTransform() const {return m_transform;}
    bool GetLoadSynchronous() const {return m_loadSynchronous;}
    bool UseFixedResolution()const {return m_useFixedResolution;}
    double GetFixedResolution() const {return m_fixedResolution;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshScene : RefCountedBase, NonCopyableClass
{
private:
    DgnDbR m_db;
    Utf8String m_sceneName;
    Utf8String m_srs;
    BeFileName m_fileName;
    DPoint3d m_srsOrigin;
    Transform m_transform;
    bvector<MRMeshNodePtr> m_children;

    BentleyStatus GetProjectionTransform();

public:
    MRMeshScene(DgnDbR, S3SceneInfo const& sceneInfo, BeFileNameCR fileName);
    BentleyStatus Load(S3SceneInfo const& sceneInfo, SystemP);
    void Draw(bool& childrenScheduled, RenderContextR, MRMeshContextCR MeshContext);
    BentleyStatus _GetRange(DRange3dR range) const;
    void GetTiles(TileCallback&, double resolution) const;
    void DrawBoundingSpheres(RenderContextR);
    void DrawMeshes(Render::GraphicR graphic);
    size_t GetTextureMemorySize() const;
    size_t GetMeshMemorySize() const;
    size_t GetNodeCount() const;
    size_t GetMaxDepth() const;
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshNode :  RefCountedBase, NonCopyableClass
{
    typedef bvector<MRMeshNodePtr>  MeshNodes;
    Render::SystemP m_system;
    S3NodeInfo m_info;
    MRMeshNodeP m_parent;
    MeshNodes m_children;
    bvector<MRMeshGeometryPtr> m_meshes;
    bvector<ByteStream> m_jpegTextures;
    BeFileName m_dir;
    bool m_primary;  // The root node and all descendents until displayable are marked as primary and never flushed.
    bool m_childrenRequested;
    uint64_t m_lastUsed;

public:
    MRMeshNode(S3NodeInfo const& info, MRMeshNodeP parent, SystemP system) : m_system(system), m_info(info), m_parent(parent), m_primary(false), m_childrenRequested(false), m_lastUsed(0) {}
    ~MRMeshNode();

    void SetDirectory(BeFileNameCR dir) {m_dir = dir;}
    void AddGeometry(int32_t nodeId, int32_t nbVertices, FPoint3d const* positions, FPoint3d const* normals, int32_t nbTriangles, uint32_t const* indices, FPoint2d const* textureCoordinates, int32_t textureIndex, SystemP);
    void PushNode(const S3NodeInfo& nodeInfo) ;

    BentleyStatus Read3MXB(MxStreamBuffer&);
    BentleyStatus Read3MXB(BeFileNameCR filename);

    BeFileName GetFileName() const;
    BentleyStatus Load();
    BentleyStatus LoadUntilDisplayable();
    void RequestLoadUntilDisplayable();
    bool LoadedUntilDisplayable() const;
    void Dump(Utf8StringCR prefix);
    BentleyStatus Draw(bool& childrenScheduled, RenderContextR, MRMeshContextCR MeshContext);
    void DrawMeshes(RenderContextR, MRMeshContextCR MeshContext);
    void DrawBoundingSphere(RenderContextR) const;
    DRange3d GetRange() const;
    bool IsLoaded() const {return !m_dir.empty();}
    bool AreChildrenLoaded() const;
    bool AreVisibleChildrenLoaded(MeshNodes& visibleChildren, ViewContextR viewContext, MRMeshContextCR meshContext) const;
    bool IsDisplayable() const {return m_info.m_dMax > 0.0;}
    ByteStream* GetTextureData(int textureIndex) {return textureIndex >= 0 && textureIndex < (int) m_jpegTextures.size() ? &m_jpegTextures[textureIndex] : nullptr;}
    size_t GetTextureMemorySize() const;
    size_t GetMeshMemorySize() const;
    size_t GetMemorySize() const {return GetMeshMemorySize() + GetTextureMemorySize();}
    size_t GetNodeCount() const;
    size_t GetMeshCount() const;
    size_t GetMaxDepth() const;
    bool TestVisibility(bool& isUnderMaximumSize, ViewContextR viewContext, MRMeshContextCR meshContext);
    void RemoveChild(MRMeshNodeP child);
    void Clone(MRMeshNode const& other);
    void ClearGraphics();
    bool IsCached() const;
    bool Validate(MRMeshNodeCP parent) const;
    void GetDepthMap(bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth);
    void Clear();
    BentleyStatus GetRange(DRange3dR range, TransformCR transform) const;
    void FlushStale(uint64_t staleTime);
    void GetTiles(TileCallback&, double resolution);
    MeshNodes const& GetChildren() const {return m_children;}
    MRMeshNodeCP GetParent() const {return m_parent;}
    S3NodeInfo const& GetInfo() const {return m_info;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  MRMeshCacheManager
{
    struct MRMeshCache*         m_cache;
    static MRMeshCacheManagerR  GetManager();

    MRMeshCacheManager();
    ~MRMeshCacheManager();
    void QueueChildLoad(MRMeshNode::MeshNodes const& children, DgnViewportP viewport, TransformCR transform, SystemP);
    void RemoveRoot(MRMeshNodeR root);
    void SetRoot(MRMeshNodeR root, SystemP);
    BentleyStatus SynchronousRead(MRMeshNodeR node, BeFileNameCR fileName, SystemP);
    void RemoveRequest(MRMeshNodeR node);
    void Debug();
    void Flush(uint64_t staleTime);

    enum class RequestStatus {Finished, None, Processed};
    RequestStatus ProcessRequests();
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  MRMeshUtil
{
    static void DisplayNodeFailureWarning(WCharCP fileName) {BeAssert(false);};
    static BeFileName ConstructNodeName(Utf8StringCR childName, BeFileNameCP parentName);
    static void GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available);
    static double CalculateResolutionRatio();
    static BentleyStatus ParseTileId(Utf8StringCR name, uint32_t& tileX, uint32_t& tileY);
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
