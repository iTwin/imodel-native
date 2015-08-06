/*------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/CVEHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//#include    <DgnPlatform/ViewHandler.h>   not in graphite
#include    <DgnPlatform/DgnCore/DgnViewport.h>
#include    <DgnPlatform/DgnCore/QvViewport.h>
//#include    <DgnPlatform/TemporaryDgnElement.h>
#include    <DgnPlatform/DgnCore/XGraphics.h>
#include    <DgnPlatform/DgnCore/DisplayStyleHandler.h>
#include    <DgnPlatform/ProxyDisplayCore.h>
#include    <DgnPlatform/MSSmartPtr.h>
//  #include    <DgnPlatform/Tcb/tcb.r.h>       // HLine settings.... => foreignformat
//#include    <RmgrTools/tools/CryptUtils.h>              // For Cryptographer  not in graphite
#include    <memory> // added in Graphite

DGNPLATFORM_TYPEDEFS (CachedVisibleEdgeOptions)
DGNPLATFORM_TYPEDEFS (ProxyDisplayModel)
DGNPLATFORM_TYPEDEFS (ProxyDataBuffer)
DGNPLATFORM_TYPEDEFS (ProxyHLEdgeSegmentId)
//DGNPLATFORM_TYPEDEFS (ProxyDisplayPath)
DGNPLATFORM_TYPEDEFS (ViewHandlerPass)   
//DGNPLATFORM_TYPEDEFS (VisibleEdgeCacheElement)

BEGIN_BENTLEY_DGN_NAMESPACE

struct CVEHLineSymbology // moved here from tcb.r.h in graphite and renamed
    {
    uint32_t        level;
    uint32_t        color;
    int32_t         style;
    uint32_t        weight;
    uint32_t        levelOverride:1;
    uint32_t        colorOverride:1;
    uint32_t        styleOverride:1;
    uint32_t        weightOverride:1;
    uint32_t        unused:28;
    };

struct  CVEHLineFlags // graphite moved this here from dgnplatform.r.h and renamed it
    {
    uint32_t        includeHidden:1;
    uint32_t        includeRules:1;
    uint32_t        calculateIntersections:1;
    uint32_t        outputToMaster:1;
    uint32_t        threeD:1;
    uint32_t        boundary:3;
    uint32_t        masterHiddenLine:1;
    uint32_t        masterDisplayHidden:1;
    uint32_t        method:3;
    uint32_t        annotations:3;
    uint32_t        smoothEdges:1;
    uint32_t        expandCustomLinestyles:1;
    uint32_t        expandHatch:1;
    uint32_t        autoOpenOutputFile:1;
    uint32_t        facetAllSurfaces:1;
    uint32_t        colorFromMaterial:1;
    uint32_t        ignoreTransparency:1;
    uint32_t        transparencyThreshold:7;
    uint32_t        exactMode:1;
    uint32_t        plotExactMode:1;
    };

struct  CVEHLineFlags2 // graphite moved this here from dgnplatform.r.h and renamed it
    {
    uint32_t        m_edgeLineStyles:1;
    uint32_t        m_cveFastMode:1;
    uint32_t        m_retainCacheWhileValidOnly:1;
    uint32_t        m_unused:29;
    };

struct CachedVisibleEdgeOptions// moved here from tcb.r.h in graphite
{
    CVEHLineFlags      m_flags;
    int32_t         m_exactHLineAccuracy;
    CVEHLineFlags2     m_flags2;
    uint32_t        m_futureUse1;
    double          m_exactHLineTolerance;
    CVEHLineSymbology  m_visible;
    CVEHLineSymbology  m_hidden;
    double          n_futureUse2[2];
 };

//#define XATTRIBUTEID_AssociationGenerator           22782           // Assigned by Mark Anderson.         removed in graphite
//                                                                                                          removed in graphite
//enum class AssociationGeneratorHandlerId                                                                  removed in graphite
//    {                                                                                                     removed in graphite
//    CVE               = 0,                                                                                removed in graphite
//    TopologyCurve     = 1,                                                                                removed in graphite
//    };                                                                                                    removed in graphite
//                                                                                                          removed in graphite
//enum class AssociationGeneratorDataId                                                                     removed in graphite
//    {                                                                                                     removed in graphite
//    CVE                  = 0,                                                                             removed in graphite
//    RefTransform         = 1,                                                                             removed in graphite
//    TopologyCurve        = 2,                                                                             removed in graphite
//    CompoundDrawState    = 3,                                                                             removed in graphite
//    };                                                                                                    removed in graphite

enum class VisibleEdgeCacheXAttrIndex
    {
    Base               = 0,
    Settings           = 0xfffe,
    OcclusionMap       = 0xffff,
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2012
+===============+===============+===============+===============+===============+======*/
//struct  IVisibleEdgeCacheLoadPreviewer
//{
//protected:
//virtual void _Display (DisplayPathCR displayPath, ViewHandlerPassCR vhPass, VisibleEdgeCacheR cache) = 0;
//
//public:
//        void Display  (DisplayPathCR displayPath, ViewHandlerPassCR vhPass, VisibleEdgeCacheR cache) { _Display (displayPath, vhPass, cache); }
//};


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     01/2012
+===============+===============+===============+===============+===============+======*/
//struct  HiddenVisibleEdgeCachePaths : T_ProxyDisplayPathVector
//{
//
//DGNPLATFORM_EXPORT                 HiddenVisibleEdgeCachePaths (VisibleEdgeCacheP cache);
//DGNPLATFORM_EXPORT                 ~HiddenVisibleEdgeCachePaths ();
//DGNPLATFORM_EXPORT bool            Contains (ProxyDisplayPathCR testPath) const;
//
//};   // HiddenVisibleEdgeCachePaths

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
//struct VisibleEdgeCacheElement
//{
//    virtual void   _VisibleEdgeCalculationDraw (ViewContextR context, ElementHandleCR eh) const = 0;
//
//};  // VisibleEdgeCacheElement


///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      01/2013
//+===============+===============+===============+===============+===============+======*/
//struct HLDisplayPath
//{
//    DgnModelP            m_root;
//    bvector<DgnElementP>    m_elements;
//
//                    HLDisplayPath () : m_root (NULL) { }
//                    HLDisplayPath (DgnModelP root, bvector <DgnElementP> elements) : m_root (root), m_elements (elements) { }
//DGNPLATFORM_EXPORT  HLDisplayPath (DisplayPathCR displayPath);   
//
//DGNPLATFORM_EXPORT  bool operator < (HLDisplayPath const&) const;
//DGNPLATFORM_EXPORT  bool Matches (DisplayPathCR) const;
//};
//
//typedef bset <HLOcclusionPathCP>   T_HLOcclusionPathSet;
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      01/2013
//+===============+===============+===============+===============+===============+======*/
//struct HLOcclusionPath 
//{
//    HLDisplayPath                   m_path;
//    mutable T_HLOcclusionPathSet    m_occluders;
//
//    HLOcclusionPath ()  { }
//    HLOcclusionPath (DgnModelP root, bvector <DgnElementP> elements) : m_path (HLDisplayPath (root, elements)) { }
//    HLOcclusionPath (DisplayPathCR displayPath) : m_path (displayPath) { };
//    HLOcclusionPath (HLDisplayPathCR displayPath) : m_path (displayPath) { };
//
//    bool operator < (HLOcclusionPath const& rhs) const { return m_path < rhs.m_path; }
//    bool Matches (DisplayPathCR displayPath) const { return m_path.Matches (displayPath); }
//
//DGNPLATFORM_EXPORT     void    AddOccluder (HLOcclusionPathCP occluder) const;
//
//};  // HLOcclusionPath


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      01/2013
+===============+===============+===============+===============+===============+======*/
//struct VisibleEdgeComparePaths 
//{
//bool operator () (const HLOcclusionPathP& path0, const HLOcclusionPathP& path1) const { return *path0 < *path1; }
//};


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      01/2013
+===============+===============+===============+===============+===============+======*/
//struct HLOcclusionPathSet : bset <HLOcclusionPathP, VisibleEdgeComparePaths> 
//{
//DGNPLATFORM_EXPORT  ~HLOcclusionPathSet ();
//DGNPLATFORM_EXPORT  HLOcclusionPathCP   GetPath (DisplayPathCR displayPath) const;
//DGNPLATFORM_EXPORT  HLOcclusionPathCP   GetPath (HLOcclusionPathCR occlusionPath) const;
//DGNPLATFORM_EXPORT  HLOcclusionPathP    GetOrInsertPath (DisplayPathCR displayPath);
//DGNPLATFORM_EXPORT  HLOcclusionPathP    GetOrInsertPath (HLOcclusionPathCR occlusionPath);
//
//};  // HLOcclusionPathSet



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      01/2013
+===============+===============+===============+===============+===============+======*/
//struct VisibleEdgeCalculationCache
//{
//private:
//    VisibleEdgeCacheCR          m_visibleEdgeCache;
//    HLOcclusionPathSet          m_currentPaths;
//    bset <HLOcclusionPathP>     m_referencedPaths;
//
//    VisibleEdgeCalculationCache (VisibleEdgeCacheCR visibleEdgeCache) : m_visibleEdgeCache (visibleEdgeCache) { }
//
//public:            
//
//DGNPLATFORM_EXPORT    static  VisibleEdgeCalculationCacheP Load (VisibleEdgeCacheCR cache, DgnViewportR viewport);
//                      static  void Delete (DgnModelP rootDgnModel, DgnElementP elementRef = NULL);
//
//DGNPLATFORM_EXPORT   ~VisibleEdgeCalculationCache ();
//
//DGNPLATFORM_EXPORT    HLOcclusionPathCP    GetPrecalculatedOcclusionPath (HLOcclusionPathCR path) const;
//DGNPLATFORM_EXPORT    VisibleEdgeCacheElementCP     GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const;
//
//};  // VisibleEdgeOcclusionMap

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
struct  VisibleEdgeCache : ProxyDisplayCacheBase // graphite: this class is used only by the importer -- never at run time
{
    uint32_t                                    m_originatingView;
    uint64_t                                    m_originatingViewGroup;
    CachedVisibleEdgeOptions                    m_options;
    //IVisibleEdgeCacheLoadPreviewer*             m_loadPreviewer;

    //static struct CachedVisibleEdgeHandler*     s_handler;


public:

//DGNPLATFORM_EXPORT static VisibleEdgeCacheP         Create (DgnModelP, CachedVisibleEdgeOptionsCR);
//DGNPLATFORM_EXPORT static StatusInt                 FreezeReference (DgnModelP modelRef, DgnViewportP viewport, CachedVisibleEdgeOptionsCP hlOptions, bool setAlwaysValid);
//DGNPLATFORM_EXPORT static StatusInt                 ThawReference (DgnModelP modelRef);
//DGNPLATFORM_EXPORT static StatusInt                 SynchReference (DgnModelP modelRef, CachedVisibleEdgeOptionsCP newOptions);
//DGNPLATFORM_EXPORT static bool                      IsFrozen (DgnModelP modelRef);
//DGNPLATFORM_EXPORT static bool                      IsModelTypeCompatible (DgnModelP modelRef);
//DGNPLATFORM_EXPORT static bool                      IsAttachmentCompatible (DgnModelP modelRef);
//DGNPLATFORM_EXPORT static bool                      OpenSettingsDialog (bool& optionsChanged, CachedVisibleEdgeOptionsR newOptions, DgnModelP modelRef);
//DGNPLATFORM_EXPORT static DisplayStyleHandlerCP     GetHandler ();
//DGNPLATFORM_EXPORT static void                      ProcessAll (int subCommand);
//DGNPLATFORM_EXPORT static StatusInt                 ShowOrHideSelections (bool doHide, bool doCopy);
//                   static void                      RegisterEditActionSource();
//                   static void                      RegisterHandler();
//DGNPLATFORM_EXPORT static StatusInt                 Delete (DgnModelP rootModel, DgnElementP elementRef = NULL);

//DGNPLATFORM_EXPORT static bool                      IsDisplayedByProxy (DgnModelP modelRef, DgnViewportR viewport);
//DGNPLATFORM_EXPORT static bool                      IsLevelUsedInProxyCache (DgnModelP modelRef, LevelId level);
//DGNPLATFORM_EXPORT static BitMaskCP                 GetLevelUsage (DgnModelP modelRef);
//                   static DisplayStyleHandlerCP     GetDisplayStyleHandler ();
//
//DGNPLATFORM_EXPORT StatusInt                        AddGPA (struct GPArray const& gpa, struct GPArrayIntervals* intervals, DisplayPath const& displayPath, ViewHandlerPassCR viewHandlerPass,  ElementGraphicsCR templateElement, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool clipped);
//DGNPLATFORM_EXPORT StatusInt                        AddCutGraphics (XGraphicsContainerCR xGraphics, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, ElementGraphicsCR templateElement, ProxyGraphicsFlags flags, bool filled);
//DGNPLATFORM_EXPORT StatusInt                        AddPassthrough (XGraphicsContainerCR xGraphics, ProxyGraphicsType, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, ElementGraphicsCR templateElement);
//DGNPLATFORM_EXPORT void                             AddOcclusionPaths (HLOcclusionPathSetCR occlusionPaths);

//void                                                Resolve ()                                                      { _Resolve (); }
//void                                                ComputeHash (DgnModelP modelRef, DgnViewportP viewport)         { _ComputeHash (modelRef, viewport); }
//void                                                ClearElementModifiedTimes (bool doClear)                        { _ClearElementModifiedTimes (doClear); }
//void                                                SetValidForView (UInt32 viewIndex, bool state)                  { _SetValidForView (viewIndex, state); }
//void                                                Draw (DisplayPath const& displayPath, ViewContextR viewContext, ViewHandlerPassCR viewHandlerPass, ViewHandlerPassR hitInfoPass) { _Draw (displayPath, viewContext, viewHandlerPass, hitInfoPass); }
//VisibleEdgeCacheElementCP                           GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const { return _GetCacheElement (displayPath, vhPass); }
//
//DGNPLATFORM_EXPORT StatusInt                        SetConcealed (DisplayPath const& displayPath, bool concealed);
//DGNPLATFORM_EXPORT bool                             GetConcealed (DisplayPath const& displayPath) const;
//DGNPLATFORM_EXPORT void                             AddConcealedPaths (T_ProxyDisplayPathVector const&);
//DGNPLATFORM_EXPORT static bool                      HiddenCopyGraphicsExist (DisplayPath const& displayPath, DgnModelP sheetDgnModel);
//DGNPLATFORM_EXPORT void                             ClearConcealed ();
//DGNPLATFORM_EXPORT bool                             ContainsConcealed () const;
//DGNPLATFORM_EXPORT void                             GetConcealedPaths (T_ProxyDisplayPathVector&) const;

//UInt32                                              GetOriginatingView () const                                     { return m_originatingView; }
//void                                                SetOriginatingView (UInt32 originatingView)                     { m_originatingView = originatingView; }
//ElementId                                           GetOriginatingViewGroup () const                                { return m_originatingViewGroup; }
//void                                                SetOriginatingViewGroup (ElementId originatingViewGroup)        { m_originatingViewGroup = originatingViewGroup; }
CachedVisibleEdgeOptionsCR                          GetOptions () const                                             { return m_options; }
//void                                                SetOptions (const CachedVisibleEdgeOptions& newOptions)         { m_options = newOptions; }
//void                                                SetLoadPreviewer (IVisibleEdgeCacheLoadPreviewer* previewer)    { m_loadPreviewer = previewer; }
//DsigRawHash const&                                  GetHashValue () const;
protected:
                                                    VisibleEdgeCache ();
//                                                    VisibleEdgeCache (DgnModelP modelRef, CachedVisibleEdgeOptionsCR options);

//    virtual StatusInt                               _AddGPA (struct GPArray const& gpa, struct GPArrayIntervals* intervals, DisplayPath const& displayPath, ViewHandlerPassCR viewHandlerPass,  ElementGraphicsCR templateElement, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool clipped) = 0;
//    virtual StatusInt                               _AddCutGraphics (XGraphicsContainerCR xGraphics, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, ElementGraphicsCR templateElement, ProxyGraphicsFlags flags, bool filled) = 0;
//    virtual StatusInt                               _AddPassthrough (XGraphicsContainerCR xGraphics, ProxyGraphicsType, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, ElementGraphicsCR templateElement) = 0;
//    virtual void                                    _AddOcclusionPaths (HLOcclusionPathSetCR occlusionPaths) = 0;
//    virtual void                                    _Resolve () = 0;
//    virtual void                                    _ComputeHash (DgnModelP modelRef, DgnViewportP viewport) = 0;
//    virtual void                                    _ClearElementModifiedTimes (bool doClear) = 0;
//    virtual void                                    _SetValidForView (UInt32 viewIndex, bool state) = 0;
//    virtual void                                    _Draw (DisplayPath const& displayPath, ViewContextR viewContext, ViewHandlerPassCR viewHandlerPass, ViewHandlerPassR hitInfoPass) = 0;
//    virtual DsigRawHash const&                      _GetHashValue () const = 0;

//    virtual void                                    _GetConcealedPaths (T_ProxyDisplayPathVector&) const = 0;
//    virtual VisibleEdgeCacheElementCP               _GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const = 0;
//
//    virtual void                                    _ClearConcealed () = 0;
//    virtual bool                                    _ContainsConcealed () const = 0;
//    virtual StatusInt                               _SetConcealed (DisplayPath const& displayPath, bool concealed) = 0;
//    virtual bool                                    _GetConcealed (DisplayPath const& displayPath) const = 0;
//    virtual void                                    _AddConcealedPaths (T_ProxyDisplayPathVector const&) = 0;
//    virtual StatusInt                               _GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, T_ProxyElementIdVector const& attachIds, T_ProxyElementIdVector const& pathIds, ProxyEdgeIdDataCR edgeId, struct GPArrayParam const& edgeParam, ViewHandlerPassCR vhPass) const = 0;

public:

struct ICreateCveElementsHelper // added in Graphite
{
typedef void* ForeignModelPtr;

//! CreateCveElements will call this to report the physical -> drawing transform.
//! @param[in] t    the viewing transform to apply to proxy graphics when viewing on a drawing
virtual void SetViewingTransform (TransformCR t) = 0;

//! CreateCveElements will call this when it begins to process CVE elements for the specified V8 attachment.
//! @return a token representing the "toModel" of this attachment
virtual ForeignModelPtr OnProcessDgnAttachment (bvector<uint64_t> const& attachmentPath) = 0;

//! Look up the DgnElementId of the specified element after it was merged into the project.
//! @param[out] newElementId the DgnElementId of the specified element after it was merged into the project.
//! @param[in] oldElementId the V8 Id of the element in its original foreign model
//! @param[in] targetModelPtr the foreign model that contains the target element 
//! @return SUCCESS if the foreign attachment and its toModel could be found
virtual BentleyStatus FindElementId (DgnElementId& newElementId, uint64_t oldElementId, ForeignModelPtr targetModelPtr) const = 0;

//! Look up the level id in the project 
virtual LevelId FindLevelId (uint32_t oldLevel, ForeignModelPtr targetModelPtr) const = 0;

};

//! Convert CVE graphics into elements in destModel. 
//! @param[in]  destModel       Where to create the CVE elements
//! @param[in]  unitsTransform  Transforms the units of the first attached model to project units. Note this is not a reference or modeling transform. It's only a units transform.
//! @param[in]  helper          provides callbacks required by CreateCveElements
DGNPLATFORM_EXPORT void CreateCveElements (DgnModelR destModel, TransformCR unitsTransform, ICreateCveElementsHelper& helper);

DGNPLATFORM_EXPORT static std::shared_ptr<VisibleEdgeCache> Restore (ElementHandleCR dgnAttachmentElement);

protected:
virtual void _CreateCveElements (DgnModelR, TransformCR unitsTransform, ICreateCveElementsHelper&) = 0;

 };  //  VisibleEdgeCache



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
//struct  CachedVisibleEdgeHashParams
//{
//DsigRawHash&                            m_hash;
//double&                                 m_newestElement;
//DgnViewportR                               m_viewport;
//CachedVisibleEdgeOptionsCR              m_options;
//double                                  m_stopIfNewer;
//Cryptographer&                          m_cryptographer;
//
//CachedVisibleEdgeHashParams (DsigRawHash& hash, double& newestElement, DgnViewportR viewport, CachedVisibleEdgeOptionsCR options, double stopIfNewer, Cryptographer& cryptographer) :
//                                        m_hash (hash), 
//                                        m_newestElement (newestElement), 
//                                        m_viewport (viewport), 
//                                        m_options (options), 
//                                        m_stopIfNewer (stopIfNewer), 
//                                        m_cryptographer (cryptographer) { }
//}; 

/*=================================================================================**//**
* This class is used for Cached Visible Edges validity checking and generation for
* references that are not displayed in the active viewgroup.
*
* CachedVisibleEdgesViewport    cveVP;
* if (SUCCESS == cveVP.Init (parentModel, proxyCache))
*     refFile->SetHiddenLineCachingOption (HIDDENLINE_CACHINGOPTION_Cached, &hlOptions, cveVP, true);
*
* @bsiclass                                                     Barry.Bentley   08/11
+===============+===============+===============+===============+===============+======*/
//struct          CachedVisibleEdgesViewport : QvViewport
//{
//private:
//    ViewInfoPtr                         m_viewInfo;
//    ViewPortInfoPtr                     m_viewPortInfo;
//
//protected: 
//    DGNPLATFORM_EXPORT virtual void          _GetScreenRect (BSIRectR rect) const;
//    DGNPLATFORM_EXPORT virtual StatusInt     _ConnectToOutput () override  {return SUCCESS;}
//    DGNPLATFORM_EXPORT virtual int           _GetIndexedLineWidth (int index) const override   { return index; }
//    DGNPLATFORM_EXPORT virtual UInt32        _GetIndexedLinePattern (int index) const override { return index; }
//    DGNPLATFORM_EXPORT virtual DgnDisplayCoreTypes::WindowP _GetWindowHandle () const override { return NULL; }
//
//public:
//    DGNPLATFORM_EXPORT StatusInt             Init (DgnModelP rootDgnModel, VisibleEdgeCacheP proxyCache);
//
//}; // CachedVisibleEdgesViewport
//

struct ProxyDisplayHandlerUtils // added in graphite
{
static void RegisterHandlers (DgnDomain& domain);
static bool IsProxyDisplayHandler (HandlerR);
};

END_BENTLEY_DGN_NAMESPACE

//__PUBLISH_SECTION_START__
#include "DgnPlatform/DgnCore/IPickGeom.h"

BEGIN_BENTLEY_DGN_NAMESPACE // added in graphite

struct  ProxyElementDisplayHandler; // added in graphite

//=======================================================================================
//! Added to a HitDetail when a section cut or other proxy graphics element is picked.
// @bsiclass                                                     Bentley Systems
//=======================================================================================
struct ProxyGraphicsElemTopology : IElemTopology // added in graphite
    {
    friend struct ProxyElementDisplayHandler;
    private:
    DgnElementId    m_targetElement;
    ClipVolumePass  m_pass;
    int             m_clipPlaneIndex;

    protected:
    DGNPLATFORM_EXPORT virtual IElemTopology* _Clone() const override;
    DGNPLATFORM_EXPORT virtual bool _IsEqual (IElemTopologyCR) const override;
    
    public:
    //! Get the 3-D element from which the selected proxy graphics were computed.
    DgnElementId GetTargetElement() const {return m_targetElement;}
    //! Get the type of proxy graphics that were selected.
    ClipVolumePass GetPass() const {return m_pass;}
    //! Get the number of the plane used to compute the selected proxy graphics.
    int GetClipPlaneIndex() const {return m_clipPlaneIndex;}
    };

END_BENTLEY_DGN_NAMESPACE
