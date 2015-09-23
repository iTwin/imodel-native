/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMesh.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include "ThreeMxSchemaInternal.h"


THREEMXSCHEMA_TYPEDEFS(MRMeshContext)
THREEMXSCHEMA_TYPEDEFS(MRMeshNode)
THREEMXSCHEMA_TYPEDEFS(MRMeshGeometry)
THREEMXSCHEMA_TYPEDEFS(MRMeshTexture)
THREEMXSCHEMA_TYPEDEFS(MRMeshCacheManager)

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE


typedef RefCountedPtr <struct MRMeshNode>       MRMeshNodePtr;
typedef RefCountedPtr <struct MRMeshGeometry>   MRMeshGeometryPtr;
typedef RefCountedPtr <struct MRMeshTexture>    MRMeshTexturePtr;


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
    MRMeshGeometry (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId);
    ~MRMeshGeometry();

    PolyfaceHeaderPtr           m_polyface;
    int                         m_textureId;
    QvElem*                     m_qvElem;

    PolyfaceHeaderPtr           GetPolyface () { return m_polyface; }
    void                        Draw (ViewContextR viewContext, MRMeshNodeR node, MRMeshContextCR meshContext);
    void                        DrawCut (ViewContextR viewContext, DPlane3dCR plane);

    size_t                      GetMemorySize() const;
    void                        ClearQvElems () { m_qvElem = NULL; }
    bool                        IsDisplayable () const      { return m_textureId >= 0; }
    bool                        IsCached () const           { return NULL != m_qvElem; }
    void                        ReleaseQVisionCache ();
    void                        ClearQvElemReferences ()    { m_qvElem = NULL; }

    static MRMeshGeometryPtr    Create (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId);

};  //  MRMeshGeometry

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshTexture : RefCountedBase
{
    bvector <Byte>              m_data;
    bvector <Byte>              m_compressedData;
    Point2d                     m_size;
    RenderMaterialPtr           m_material;

                                MRMeshTexture (Byte const* pData, size_t dataSize);
                                ~MRMeshTexture ();
    void                        Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext);
    void                        Activate (ViewContextR viewContext);
    size_t                      GetMemorySize() const;
    bool                        IsInitialized() const;
    void                        ReleaseQVisionCache ();

    static MRMeshTexturePtr     Create (Byte const* pData, size_t dataSize);

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

    mutable size_t      m_lastPumpTicks;
    mutable size_t      m_nodeCount;
    mutable size_t      m_pointCount;

    MRMeshContext (TransformCR transform, ViewContextR viewContext, double fixedResolution);

    TransformCR         GetTransform () const       { return m_transform; }
    bool                GetLoadSynchronous () const { return m_loadSynchronous; }
    bool                UseFixedResolution ()const  { return m_useFixedResolution; }
    double              GetFixedResolution () const { return m_fixedResolution; }
    QvCache*            GetQvCache() const          { BeAssert (false); return NULL; }

};  // MRMeshContext


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshScene
{
    WString                         m_sceneName;
    WString                         m_srs;
    WString                         m_fileName;
    DPoint3d                        m_srsOrigin;
    bvector <MRMeshNodePtr>         m_children;

    MRMeshScene ()  { m_srsOrigin.Zero(); }

    BentleyStatus   Initialize (S3SceneInfo const& sceneInfo, WCharCP fileName);
    void            DrawBoundingSpheres (ViewContextR viewContext);
    void            Draw (ViewContextR viewContext, MRMeshContextCR MeshContext);
    void            DrawMeshes (IDrawGeomP drawGeom, TransformCR transform);

    size_t          GetTextureMemorySize () const;
    size_t          GetMeshMemorySize() const;
    size_t          GetNodeCount () const;
    size_t          GetMaxDepth () const;


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
    bool                            m_mark;
    BeFileName                      m_dir;
    bool                            m_primary;                  // The root node and all descendents until displayable are marked as primary and never flushed.
    bool                            m_childrenRequested;

    MRMeshNode (S3NodeInfo const&info, struct MRMeshNode* parent) : m_info (info), m_parent (parent), m_mark (false), m_primary(false), m_childrenRequested (false) { }
    ~MRMeshNode ();

    virtual void                _SetDirectory(BeFileNameCR dir) override { m_dir = dir; }
    virtual void                _Clear() override {}
    virtual void                _PushJpegTexture (Byte const* data, size_t dataSize);
    virtual void                _AddGeometry(int nodeId, int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId) override;
    virtual void                _PushNode(const S3NodeInfo& nodeInfo);

    BeFileName                  GetFileName () const;
    BentleyStatus               Load ();
    BentleyStatus               LoadUntilDisplayable ();
    void                        RequestLoadUntilDisplayable ();
    bool                        LoadedUntilDisplayable () const;
    void                        Dump (WStringCR prefix);
    BentleyStatus               Draw (ViewContextR viewContext, MRMeshContextCR MeshContext);
    BentleyStatus               DrawCut (ViewContextR viewContext, MRMeshContextCR MeshContext, DPlane3dCR plane);
    void                        Draw (IDrawGeomR drawGeom, TransformCR tranform);
    void                        DrawMeshes (IDrawGeomP drawGeom, TransformCR transform);
    void                        DrawMeshes (ViewContextR viewContext, MRMeshContextCR MeshContext);
    void                        DrawBoundingSphere (ViewContextR viewContext);
    DRange3d                    GetRange () const;
    bool                        IsLoaded () const      { return !m_dir.empty(); }
    bool                        AreChildrenLoaded () const;
    bool                        AreVisibleChildrenLoaded (T_MeshNodeArray& visibleChildren, ViewContextR viewContext, MRMeshContextCR meshContext) const;
    bool                        IsDisplayable () const { return m_info.m_dMax > 0.0; }
    MRMeshTextureP              GetTexture (int textureId) const { return textureId >= 0 && textureId < (int) m_textures.size() ? m_textures.at(textureId).get() : NULL; }
    size_t                      GetTextureMemorySize () const;
    size_t                      GetMeshMemorySize() const;
    size_t                      GetMemorySize () const { return GetMeshMemorySize() + GetTextureMemorySize(); }
    size_t                      GetNodeCount () const;
    size_t                      GetMeshCount () const;
    size_t                      GetMaxDepth () const;
    bool                        TestVisibility (bool& isUnderMaximumSize, ViewContextR viewContext, MRMeshContextCR meshContext);
    void                        MarkVisible (size_t& visibleCount, ViewContextR viewContext, MRMeshContextCR meshContext);
    void                        ClearUnmarked ();
    void                        ClearMarks ();
    size_t                      GetMarkCount () const;
    void                        RemoveChild (MRMeshNodeP child);
    void                        Clone (MRMeshNode const& other);
    void                        ClearQvElems ();
    bool                        IsCached () const;
    void                        ReleaseQVisionCache ();
    void                        ClearQvElemReferences ();
    bool                        Validate (MRMeshNodeCP parent) const;
    void                        GetDepthMap (bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth);
    void                        Clear();
    
    static MRMeshNodePtr        Create (S3NodeInfo const& info, MRMeshNodeP parent);
    static MRMeshNodePtr        Create ();

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
    void                        FlushNonVisibleNodes ();
    void                        Debug ();
    void                        Flush ();
    void                        ProcessRequests ();



};  // MRMeshCacheManager



/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  MRMeshUtil
{
    static void                 DisplayNodeFailureWarning (WCharCP fileName) { BeAssert(false); };
    static BeFileName           ConstructNodeName (std::string const& childName, BeFileNameCP parentName);
    static BentleyStatus        ReadSceneFile (S3SceneInfo& sceneInfo, WCharCP fileName);
    static void                 GetMemoryStatistics (size_t& memoryLoad, size_t& total, size_t& available);
    static double               CalculateResolutionRatio ();


};  // MRMeshUtil



END_BENTLEY_THREEMX_SCHEMA_NAMESPACE


