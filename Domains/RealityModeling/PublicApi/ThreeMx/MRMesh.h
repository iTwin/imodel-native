/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once


#include <ThreeMxSchema/ThreeMxSchemaApi.h>


THREEMXSCHEMA_TYPEDEFS(MRMeshContext)
THREEMXSCHEMA_TYPEDEFS(MRMeshNode)
THREEMXSCHEMA_TYPEDEFS(MRMeshGeometry)
THREEMXSCHEMA_TYPEDEFS(MRMeshTexture)
THREEMXSCHEMA_TYPEDEFS(MRMeshCacheManager)

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE


typedef RefCountedPtr <struct MRMeshNode>       MRMeshNodePtr;
typedef RefCountedPtr <struct MRMeshGeometry>   MRMeshGeometryPtr;
typedef RefCountedPtr <struct MRMeshTexture>    MRMeshTexturePtr;
typedef RefCountedPtr <struct MRMeshScene>      MRMeshScenePtr;


enum MRMeshMinorXAttributeId
    {
    MRMeshMinorXAttributeId_PrimaryData = 0,
    MRMeshMinorXAttributeId_Clip        = 1,
    MRMeshMinorXAttributeId_ClipElement = 2,
    };


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshGeometry : RefCountedBase
{
    THREEMX_SCHEMA_EXPORT MRMeshGeometry (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId);
    THREEMX_SCHEMA_EXPORT ~MRMeshGeometry();

     PolyfaceHeaderPtr           m_polyface;
     int                         m_textureId;
     QvElem*                     m_qvElem;

    THREEMX_SCHEMA_EXPORT PolyfaceHeaderPtr           GetPolyface () { return m_polyface; }
    THREEMX_SCHEMA_EXPORT PolyfaceHeaderCP            GetPolyfaceCP() const { return m_polyface.get(); }
    THREEMX_SCHEMA_EXPORT int                         GetTextureId() const { return m_textureId; }
    THREEMX_SCHEMA_EXPORT void                        Draw (ViewContextR viewContext, MRMeshNodeR node, MRMeshContextCR meshContext);
    THREEMX_SCHEMA_EXPORT void                        DrawCut (ViewContextR viewContext, DPlane3dCR plane);
    THREEMX_SCHEMA_EXPORT BentleyStatus               GetRange (DRange3dR range, TransformCR transform) const;


    THREEMX_SCHEMA_EXPORT size_t                      GetMemorySize() const;
    THREEMX_SCHEMA_EXPORT void                        ClearQvElems () { m_qvElem = NULL; }
    THREEMX_SCHEMA_EXPORT bool                        IsDisplayable () const      { return m_textureId >= 0; }
    THREEMX_SCHEMA_EXPORT bool                        IsCached () const           { return NULL != m_qvElem; }
    THREEMX_SCHEMA_EXPORT void                        ReleaseQVisionCache ();
    THREEMX_SCHEMA_EXPORT void                        ClearQvElemReferences ()    { m_qvElem = NULL; }

    static THREEMX_SCHEMA_EXPORT MRMeshGeometryPtr    Create (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId);

};  //  MRMeshGeometry

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshTexture : RefCountedBase
{
    bvector <Byte>              m_data;         // BGRA
    bvector <Byte>              m_compressedData;
    Point2d                     m_size;
    RenderMaterialPtr           m_material;

    THREEMX_SCHEMA_EXPORT                             MRMeshTexture (Byte const* pData, size_t dataSize);
    THREEMX_SCHEMA_EXPORT                            ~MRMeshTexture ();
    THREEMX_SCHEMA_EXPORT void                        Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext);
    THREEMX_SCHEMA_EXPORT void                        Activate (ViewContextR viewContext);
    THREEMX_SCHEMA_EXPORT size_t                      GetMemorySize() const;
    THREEMX_SCHEMA_EXPORT bool                        IsInitialized() const;
    THREEMX_SCHEMA_EXPORT void                        ReleaseQVisionCache ();
    THREEMX_SCHEMA_EXPORT Point2d                     GetSize() const { return m_size; }
    THREEMX_SCHEMA_EXPORT ByteCP                      GetData() const { return &m_data[0]; }

    static THREEMX_SCHEMA_EXPORT MRMeshTexturePtr     Create (Byte const* pData, size_t dataSize);

}; //  MRMeshTexture


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshContext
{
    Transform           m_transform;
    bool                m_loadSynchronous;
    bool                m_useFixedResolution;
    double              m_fixedResolution;
    QvCache*            m_qvCache;

    mutable size_t      m_lastPumpTicks;
    mutable size_t      m_nodeCount;
    mutable size_t      m_pointCount;

    THREEMX_SCHEMA_EXPORT MRMeshContext (TransformCR transform, ViewContextR viewContext, double fixedResolution);

    THREEMX_SCHEMA_EXPORT TransformCR         GetTransform () const       { return m_transform; }
    THREEMX_SCHEMA_EXPORT bool                GetLoadSynchronous () const { return m_loadSynchronous; }
    THREEMX_SCHEMA_EXPORT bool                UseFixedResolution ()const  { return m_useFixedResolution; }
    THREEMX_SCHEMA_EXPORT double              GetFixedResolution () const { return m_fixedResolution; }
    THREEMX_SCHEMA_EXPORT QvCache*            GetQvCache() const          { return m_qvCache; }

};  // MRMeshContext


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshScene : ThreeMxScene
{
protected:
    WString                         m_sceneName;
    WString                         m_srs;
    WString                         m_fileName;
    DPoint3d                        m_srsOrigin;
    bvector <MRMeshNodePtr>         m_children;

    MRMeshScene (S3SceneInfo const& sceneInfo, WCharCP fileName);

public:

    static THREEMX_SCHEMA_EXPORT ThreeMxScenePtr  Create (S3SceneInfo const& sceneInfo, WCharCP fileName);

    virtual void    _Draw (bool& childrenScheduled, ViewContextR viewContext, MRMeshContextCR MeshContext) override;
    THREEMX_SCHEMA_EXPORT virtual BentleyStatus   _GetRange (DRange3dR range, TransformCR transform)  const override;
    THREEMX_SCHEMA_EXPORT virtual void    _GetTiles(GetTileCallback callback, double resolution) override;


    THREEMX_SCHEMA_EXPORT void            DrawBoundingSpheres (ViewContextR viewContext);
    THREEMX_SCHEMA_EXPORT void            DrawMeshes (IDrawGeomP drawGeom, TransformCR transform);

    THREEMX_SCHEMA_EXPORT size_t          GetTextureMemorySize () const;
    THREEMX_SCHEMA_EXPORT size_t          GetMeshMemorySize() const;
    THREEMX_SCHEMA_EXPORT size_t          GetNodeCount () const;
    THREEMX_SCHEMA_EXPORT size_t          GetMaxDepth () const;


};  // MRMeshScene

typedef     bvector <MRMeshNodePtr>  T_MeshNodeArray;

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshNode :  BaseMeshNode,  RefCountedBase
{
    S3NodeInfo                      m_info;
    MRMeshNodeP                     m_parent;
    T_MeshNodeArray                 m_children;
    bvector <MRMeshGeometryPtr>     m_meshes;
    bvector <MRMeshTexturePtr>      m_textures;
    BeFileName                      m_dir;
    bool                            m_primary;                  // The root node and all descendents until displayable are marked as primary and never flushed.
    bool                            m_childrenRequested;
    uint64_t                        m_lastUsed;

    THREEMX_SCHEMA_EXPORT MRMeshNode (S3NodeInfo const&info, struct MRMeshNode* parent) : m_info (info), m_parent (parent), m_primary(false), m_childrenRequested (false), m_lastUsed (0) { }
    THREEMX_SCHEMA_EXPORT ~MRMeshNode ();

    THREEMX_SCHEMA_EXPORT virtual void                _SetDirectory(BeFileNameCR dir) override { m_dir = dir; }
    THREEMX_SCHEMA_EXPORT virtual void                _Clear() override {}
    THREEMX_SCHEMA_EXPORT virtual void                _PushJpegTexture (Byte const* data, size_t dataSize);
    THREEMX_SCHEMA_EXPORT virtual void                _AddGeometry(int nodeId, int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId) override;
    THREEMX_SCHEMA_EXPORT virtual void                _PushNode(const S3NodeInfo& nodeInfo);

    THREEMX_SCHEMA_EXPORT  BeFileName                  GetFileName () const;
    THREEMX_SCHEMA_EXPORT BentleyStatus               Load ();
    THREEMX_SCHEMA_EXPORT BentleyStatus               LoadUntilDisplayable ();
    THREEMX_SCHEMA_EXPORT void                        RequestLoadUntilDisplayable ();
    THREEMX_SCHEMA_EXPORT bool                        LoadedUntilDisplayable () const;
    THREEMX_SCHEMA_EXPORT void                        Dump (WStringCR prefix);
    BentleyStatus               Draw (bool& childrenScheduled, ViewContextR viewContext, MRMeshContextCR MeshContext);
    BentleyStatus               DrawCut (ViewContextR viewContext, MRMeshContextCR MeshContext, DPlane3dCR plane);
    void                        Draw (IDrawGeomR drawGeom, TransformCR tranform);
    void                        DrawMeshes (IDrawGeomP drawGeom, TransformCR transform);
    void                        DrawMeshes (ViewContextR viewContext, MRMeshContextCR MeshContext);
    void                        DrawBoundingSphere (ViewContextR viewContext);
    THREEMX_SCHEMA_EXPORT DRange3d                    GetRange () const;
    THREEMX_SCHEMA_EXPORT bool                        IsLoaded () const      { return !m_dir.empty(); }
    THREEMX_SCHEMA_EXPORT bool                        AreChildrenLoaded () const;
    THREEMX_SCHEMA_EXPORT bool                        AreVisibleChildrenLoaded (T_MeshNodeArray& visibleChildren, ViewContextR viewContext, MRMeshContextCR meshContext) const;
    THREEMX_SCHEMA_EXPORT bool                        IsDisplayable () const { return m_info.m_dMax > 0.0; }
    THREEMX_SCHEMA_EXPORT MRMeshTextureP              GetTexture (int textureId) const { return textureId >= 0 && textureId < (int) m_textures.size() ? m_textures.at(textureId).get() : NULL; }
    THREEMX_SCHEMA_EXPORT size_t                      GetTextureMemorySize () const;
    THREEMX_SCHEMA_EXPORT size_t                      GetMeshMemorySize() const;
    THREEMX_SCHEMA_EXPORT size_t                      GetMemorySize () const { return GetMeshMemorySize() + GetTextureMemorySize(); }
    THREEMX_SCHEMA_EXPORT size_t                      GetNodeCount () const;
    THREEMX_SCHEMA_EXPORT size_t                      GetMeshCount () const;
    THREEMX_SCHEMA_EXPORT size_t                      GetMaxDepth () const;
    THREEMX_SCHEMA_EXPORT bool                        TestVisibility (bool& isUnderMaximumSize, ViewContextR viewContext, MRMeshContextCR meshContext);
    THREEMX_SCHEMA_EXPORT void                        RemoveChild (MRMeshNodeP child);
    THREEMX_SCHEMA_EXPORT void                        Clone (MRMeshNode const& other);
    THREEMX_SCHEMA_EXPORT void                        ClearQvElems ();
    THREEMX_SCHEMA_EXPORT bool                        IsCached () const;
    void                        ReleaseQVisionCache ();
    void                        ClearQvElemReferences ();
    THREEMX_SCHEMA_EXPORT bool                        Validate (MRMeshNodeCP parent) const;
    void                        GetDepthMap (bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth);
    THREEMX_SCHEMA_EXPORT void                        Clear();
    THREEMX_SCHEMA_EXPORT BentleyStatus               GetRange (DRange3dR range, TransformCR transform) const;
    THREEMX_SCHEMA_EXPORT void                        FlushStale (uint64_t staleTime);
    void                        GetTiles(GetTileCallback callback, double resolution);
    
    static THREEMX_SCHEMA_EXPORT MRMeshNodePtr        Create (S3NodeInfo const& info, MRMeshNodeP parent);
    static THREEMX_SCHEMA_EXPORT MRMeshNodePtr        Create ();

}; // MRMeshNode

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  MRMeshCacheManager
{
    struct MRMeshCache*         m_cache;
    static MRMeshCacheManager   s_manager;
    static MRMeshCacheManagerR  GetManager()  { return s_manager; }

    MRMeshCacheManager();
    ~MRMeshCacheManager();
    void                        QueueChildLoad (T_MeshNodeArray const& children, DgnViewportP viewport, TransformCR transform);
    void                        RemoveRoot (MRMeshNodeR root);
    void                        AddRoot (MRMeshNodeR root);
    BentleyStatus               SynchronousRead (MRMeshNodeR node, BeFileNameCR fileName);
    void                        RemoveRequest (MRMeshNodeR node);
    void                        Debug ();
    void                        Flush (uint64_t staleTime);

    enum class RequestStatus { Finished, None, Processed };
    RequestStatus               ProcessRequests ();



};  // MRMeshCacheManager



/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  MRMeshUtil
{
    static THREEMX_SCHEMA_EXPORT void                 DisplayNodeFailureWarning (WCharCP fileName) { BeAssert(false); };
    static THREEMX_SCHEMA_EXPORT BeFileName           ConstructNodeName (std::string const& childName, BeFileNameCP parentName);
    static THREEMX_SCHEMA_EXPORT BentleyStatus        ReadSceneFile (S3SceneInfo& sceneInfo, WCharCP fileName);
    static THREEMX_SCHEMA_EXPORT void                 GetMemoryStatistics (size_t& memoryLoad, size_t& total, size_t& available);
    static THREEMX_SCHEMA_EXPORT double               CalculateResolutionRatio ();
    static THREEMX_SCHEMA_EXPORT BentleyStatus        ParseTileId(std::string const& name, uint32_t& tileX, uint32_t& tileY);


};  // MRMeshUtil



END_BENTLEY_THREEMX_SCHEMA_NAMESPACE


