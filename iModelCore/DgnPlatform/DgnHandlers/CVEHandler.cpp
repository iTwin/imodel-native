/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/CVEHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DgnPlatformInternal.h"


//#include    <DgnPlatform/DisplayHandler.h>
#include    <DgnPlatform/DgnCore/DgnRangeTree.h>
//#include    <DgnPlatform/TemporaryElementRef.h>
//#include    <RmgrTools/tools/CryptUtils.h>
#include    <Bentley/BeConsole.h>
#include    <DgnPlatform/CVEHandler.h>// added in graphite
#include    <DgnPlatform/DgnCore/SimplifyViewDrawGeom.h>// added in graphite

DGNPLATFORM_TYPEDEFS(ProxyDisplayModel) // added in graphite
DGNPLATFORM_TYPEDEFS(ProxyElement)
DGNPLATFORM_TYPEDEFS(ProxyModelPass)
DGNPLATFORM_TYPEDEFS(ProxyGraphics)
DGNPLATFORM_TYPEDEFS(ProxyGraphicsPiece)
DGNPLATFORM_TYPEDEFS(ProxyElementElemRef)
DGNPLATFORM_TYPEDEFS(ProxyElementMap)
//DGNPLATFORM_TYPEDEFS(ProxyDisplayHitInfo)
DGNPLATFORM_TYPEDEFS(ProxyLineStyleParams)
DGNPLATFORM_TYPEDEFS(ProxyEdgeIdData)
DGNPLATFORM_TYPEDEFS(VisibleEdgeCacheImpl)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


#define PROXYELEMENTHANDLER_NOTEXPORTED             // There is no need to export the internal element handlers.

typedef bvector<ElementRefP>                T_ElementRefVector;
typedef bvector<ProxyDisplayModelP>         T_ProxyModelVector;
typedef bvector<byte>                       T_ByteVector;
typedef bvector <DisplayPathP>              T_DisplayPathVector;
typedef CVEHLineSymbology const*               HLineSymbologyCP;

typedef T_ProxyElementIdVector const&       ElemIdPathCR;

//static UInt32               s_currentProxyCacheVersionNumber = 51;  unused var removed in graphite
static UInt32               s_currentProxyModelVersionNumber = 51;
//static UInt32               s_currentHashAlgorithmVersion = 1;      // Hash algorithm was updated was updated after SS3 (version 0).  unused var removed in graphite


#define FIRST_VERSION_WITH_VIEW 15

#define DIGITAL_SIGNATURE_MAX_RAW_HASH          20  /* holds a 20-byte SHA1 hash */

#define PROXY_ELEMENT_GRAPHICS_XATTRID          0   // added in graphite
#define PROXY_ELEMENT_DETAILS_XATTRID           1   // added in graphite

/*---------------------------------------------------------------------------------*//**
* @Description A buffer to store an unsigned hash. It is not stored. This is used to cache
* the results of a partial hash, to be picked up and augmented and check later.
* @bsistruct                    SamWilson                       02/02
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DsigRawHash
    {
    /**  @Description If false, hash value is not computed. */
    bool        isComputed;
    /**  @Description Hash algorithm used:
<ul>
<li> DIGITAL_SIGNATURE_ALG_RSA_MD5          MD5 hashing with RSA encryption
<li> DIGITAL_SIGNATURE_ALG_RSA_SHA1         SHA1 hashing with RSA encryption
</ul>
    */
    UInt32      algid;
    /**  @Description The number of bytes of hash data in b */
    ULong32     len;
    /**  @Description An MD5 or SHA1 hash value (unencrypted and with no OID!) */
    byte        b[DIGITAL_SIGNATURE_MAX_RAW_HASH];
    };


enum ProxySymbologyOverride
    {
    ProxySymbologyOverride_None         = 0,
    ProxySymbologyOverride_Concealed,
    ProxySymbologyOverride_NotConcealed,
    };

enum ProxyDisplayFlagsMask
    {
    ProxyDisplayFlagsMask_None              = 0,
    ProxyDisplayFlagsMask_DisplayConcealed  = 0x0001 << 0,
    ProxyDisplayFlagsMask_ForceConcealed    = 0x0001 << 1,
    } ;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   resolveColorTBGR (UInt32 unresolved, DgnModelP modelRef)
    {
    IntColorDef  colorDef;

    if (NULL == modelRef ||
        SUCCESS != modelRef->GetDgnProject().Colors().Extract (&colorDef, NULL, NULL, NULL, NULL, unresolved))
        {
        BeAssert (false);
        return 0x00ffffff;
        }

    return colorDef.m_int;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    setDisplayParamsLineColor (ElemDisplayParamsR displayParams, UInt32 color, DgnModelP symbologyDgnModel, DgnModelP currDgnModel)
    {
    IntColorDef  colorDef;
    if (symbologyDgnModel->GetDgnProject().Colors().IsTrueColor (colorDef, color))
        displayParams.SetLineColorTBGR (resolveColorTBGR (color, symbologyDgnModel));
    else
        displayParams.SetLineColor (color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static double   getToleranceScaleFactor (DgnModelP refModel)
//    {
//    double          scale = 1.0;
//
//    if (NULL != refModel)
//        modelInfo_getUorScaleBetweenModels  (&scale, refModel->GetRoot(), refModel->GetDgnModelP ());
//
//    return  scale;
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//CachedVisibleEdgeOptions::CachedVisibleEdgeOptions () { memset(this, 0, sizeof(*this)); }
//
//CachedVisibleEdgeOptions::CachedVisibleEdgeOptions (TcbCR tcb, DgnModelP modelRef)
//    {
//    DgnAttachmentP      dgnAttachment;
//    EditElementHandle   attachmentEh;
//
//    // TFS# 12946 - look for CVE settings from previous caching.
//    if (NULL != (dgnAttachment = modelRef->AsDgnAttachmentP()) &&
//        SUCCESS == dgnAttachment->FindDgnAttachmentElement (attachmentEh))
//        {
//        ElementHandle::XAttributeIter xAttributeIter (attachmentEh, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::Settings);
//
//        if (xAttributeIter.IsValid() && xAttributeIter.GetSize() == sizeof (*this))
//            {
//            memcpy (this, xAttributeIter.PeekData(), sizeof (*this));
//            return;
//            }
//        }
//
//
//    memset (this, 0, sizeof (*this));
//    m_flags               = tcb.hiddenLine.flags;
//    m_flags2              = tcb.hiddenLine.flags2;
//    m_exactHLineAccuracy  = tcb.exactHLineAccuracy;
//    m_exactHLineTolerance = tcb.exactHLineTolerance;
//
//    // Don't get Symbology overrides from tcb - default to WYSIWYG.
//
//
//    // we are getting the tolerance from the tcb, which is in the root model's unit system.
//    // but the calculations are all done in the reference model's unit system, so we need to convert.
//    if (0.0 == m_exactHLineTolerance)
//        m_exactHLineTolerance = dgnModel_getUorPerMeter (modelRef->GetDgnModelP ()) / 100.0;   // Default to one centimetor.
//    else
//        m_exactHLineTolerance *= getToleranceScaleFactor (modelRef);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt                       VisibleEdgeCache::AddGPA (struct GPArray const& gpa, struct GPArrayIntervals* intervals, DisplayPath const& displayPath, ViewHandlerPassCR viewHandlerPass,  DgnElementCR templateElement, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool clipped)
//                                    { return _AddGPA (gpa, intervals, displayPath, viewHandlerPass, templateElement, flags, segmentId, cds, clipped); }
//StatusInt                       VisibleEdgeCache::AddCutGraphics (XGraphicsContainerCR xGraphics, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement, ProxyGraphicsFlags flags, bool filled)
//                                    { return _AddCutGraphics (xGraphics, vhPass, displayPath, templateElement, flags, filled); }
//StatusInt                       VisibleEdgeCache::AddPassthrough (XGraphicsContainerCR xGraphics, ProxyGraphicsType type, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement)
//                                    { return _AddPassthrough (xGraphics, type, vhPass, displayPath, templateElement); }
//void                            VisibleEdgeCache::AddOcclusionPaths (HLOcclusionPathSetCR occlusionPaths)  { _AddOcclusionPaths (occlusionPaths); }

//DsigRawHash const&              VisibleEdgeCache::GetHashValue () const { return _GetHashValue (); }


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   06/2007
+===============+===============+===============+===============+===============+======*/
//struct  CacheValidityState
//{
//    bool                                m_newerElements;        // there are newer elements than when the cache was created.
//    bool                                m_differentElementSet;  // the set of elements that are displayed is different from when the cache was created.
//    bool                                m_attachmentChanged;    // the attachment parameters for the reference have changed.
//    bool                                m_missingReferences;    // there are some references that were present when the cache was created that are no longer found.
//    int                                 m_childModelCount;
//    int                                 m_sumOfChangeCounts;
//    double                              m_latestFileTime;
//
//                                        CacheValidityState ();
//                                        CacheValidityState (DgnModelP modelRef);
//    void                                Clear ();
//    bool                                Equals (const CacheValidityState& other) const;
//    void                                CopyFrom (const CacheValidityState& source);
//    void                                AccumulateChildState (DgnModelP parentModel);
//
//};  // ValidityState


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLDisplayPath::HLDisplayPath (DisplayPathCR displayPath)
//    {
//#ifdef DEBUG_IDS
//    ElementId       id = displayPath.GetPathElem(0)->GetElementId();
//
//    if (id == 0)
//        return;
//#endif
//
//    m_root = displayPath.GetRoot();
//    m_elements.resize (displayPath.GetCount());
//    for (int i=0; i<displayPath.GetCount(); i++)
//        m_elements[i] = displayPath.GetPathElem(i);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool HLDisplayPath::operator < (HLDisplayPath const& rhs) const
//    {
//    if (m_root != rhs.m_root)
//        return m_root < rhs.m_root;
//
//    if (m_elements.size () != rhs.m_elements.size())
//        return m_elements.size() < rhs.m_elements.size();
//
//    for (size_t i=0; i<m_elements.size(); i++)
//
//#ifdef DEBUG_IDS
//        {
//        ElementId       id = m_elements[i]->GetElementId(), rhsId = rhs.m_elements[i]->GetElementId();
//
//        if (id != rhsId)
//            return id < rhsId;
//        }
//#else
//        if (m_elements[i] != rhs.m_elements[i])
//            return m_elements[i] < rhs.m_elements[i];
//#endif
//    return false;
//    }
//
//
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      01/2013
//+===============+===============+===============+===============+===============+======*/
//struct  SavedOcclusionPath
//{
//    HLOcclusionPathP   m_path;
//    bvector <UInt32>   m_occluderIndices;
//
//SavedOcclusionPath () : m_path (NULL) { }
//SavedOcclusionPath (HLOcclusionPathCR path, bvector<UInt32> occluderIndices) : m_path (new HLOcclusionPath (path)), m_occluderIndices (occluderIndices) { }
//SavedOcclusionPath (DgnModelP root, bvector<ElementRefP> elements, bvector<UInt32>& occluderIndices)
//    {
//    m_path = new HLOcclusionPath (root, elements);
//    m_occluderIndices  = occluderIndices;
//    }
//
//
//};  // SavedOcclusionPath
//
//
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      01/2013
//+===============+===============+===============+===============+===============+======*/
//struct SavedOcclusionPaths : bvector <SavedOcclusionPath>
//{
//
//~SavedOcclusionPaths ()
//    {
//    for (SavedOcclusionPath path: *this)
//        delete path.m_path;
//    }
//
//};


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley   06/2007
+===============+===============+===============+===============+===============+======*/
struct  VisibleEdgeCacheImpl : VisibleEdgeCache
{
friend struct CachedVisibleEdges;

// when IsValidForView is called for a particular view, we look find m_rootModels child models, add up the sum of their change counts and find the latest file time.
// If those are all the same as they were last time, we can return m_validForView[iView]. Otherwise we have to recalculate the current hashValue and newestElement
// and compare them to the values for the cache.
private:
    ProxyDisplayModelP                  m_rootProxyModel;
    //T_ProxyModelMap                     m_proxyModelMap;
    //mutable CacheValidityState          m_validityState[MAX_VIEWS];
    //double                              m_creationTime;
    //bool                                m_obsoleteVersion;
    //bool                                m_obsoleteHash;
    //double                              m_rootUnitScaleFactor;
    //mutable SavedOcclusionPaths*        m_occlusionPaths;

    //mutable DsigRawHash                 m_hashValue;
    //mutable double                      m_newestElement;

public:                                 VisibleEdgeCacheImpl ();
//                                        VisibleEdgeCacheImpl (DgnModelP rootModel, CachedVisibleEdgeOptionsCR options);
    virtual                             ~VisibleEdgeCacheImpl ();


//DGNPLATFORM_EXPORT void                 Dump () const;
//DGNPLATFORM_EXPORT static bool          HiddenCopyGraphicsExist (DisplayPath const& displayPath, DgnModelP sheetDgnModel);

protected:
    //virtual void                        _GetConcealedPaths (T_ProxyDisplayPathVector&) const override;

    //virtual StatusInt                   _AddGPA (struct GPArray const& gpa, struct GPArrayIntervals* intervals, DisplayPath const& displayPath, ViewHandlerPassCR viewHandlerPass,  DgnElementCR templateElement, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool clipped) override;
    //virtual StatusInt                   _AddCutGraphics (XGraphicsContainerCR xGraphics, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement, ProxyGraphicsFlags flags, bool filled) override;
    //virtual void                        _AddOcclusionPaths (HLOcclusionPathSetCR occlusionPaths) override;
    //virtual StatusInt                   _AddPassthrough (XGraphicsContainerCR xGraphics, ProxyGraphicsType, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement) override;
    //virtual double                      _GetRootUnitScaleFactor () const override;
    //virtual bool                        _ContainsModel (DgnModelCR) const override;
    //virtual bool                        _IsObsoleteVersion() const override { return m_obsoleteVersion; }
    //virtual ProxyCacheStatus            _GetCacheStatusForViewport (ViewportP viewport) const override;
    //virtual StatusInt                   _GetRange (DRange3dR range) const override;
    //virtual double                      _GetCreationTime () const override { return m_creationTime; }
    //virtual StatusInt                   _SetAlwaysValid (DgnModelP refModel, ElementRefP elRef) override;        // used when we're accepting the cache for use in an iModel.
    //virtual bool                        _GetAlwaysValid () const override;
    //virtual StatusInt                   _SetAlwaysInvalid (DgnModelP refModel, ElementRefP elRef) override;      // used when we're accepting an out-of-date cache for inclusion in an iModel.
    //virtual bool                        _GetAlwaysInvalid () const override;
    //virtual bool                        _AnyCachedChildReferencesMissing () const override;
    //virtual StatusInt                   _Save (DgnModelP rootModel, ElementRefP elementRef) const override;
    //virtual void                        _ClearConcealed () override;
    //virtual bool                        _ContainsConcealed () const override;
    //virtual void                        _Resolve () override;
    //virtual void                        _ClearElementModifiedTimes (bool doClear) override;
    //virtual void                        _ComputeHash (DgnModelP modelRef, ViewportP viewport) override;
    //virtual void                        _SetValidForView (UInt32 viewIndex, bool state) override;
    //virtual void                        _Draw (DisplayPath const& displayPath, ViewContextR context, ViewHandlerPassCR viewHandlerPass, ViewHandlerPassR hitInfoPass) override;
    //virtual VisibleEdgeCacheElementCP   _GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const;
    //virtual void                        _ClearValidityState () override;
    //virtual ModelInfoCP                 _GetModelInfoCP (DgnModelCR modelRef) const override;
    //virtual LevelCacheP                 _GetLevelCache (DgnModelCR) override;
    //virtual StatusInt                   _SetConcealed (DisplayPath const& displayPath, bool concealed);
    //virtual bool                        _GetConcealed (DisplayPath const& displayPath) const;
    //virtual void                        _AddConcealedPaths (T_ProxyDisplayPathVector const&);
    //WStringCP                           _GetLevelName (DgnModelP modelRef, LevelId levelId) const override;
    //StatusInt                           _GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, T_ProxyElementIdVector const& attachIds, T_ProxyElementIdVector const& pathIds, ProxyEdgeIdDataCR edgeId, struct GPArrayParam const& edgeParam, ViewHandlerPassCR vhPass) const;
    //DsigRawHash const&                  _GetHashValue () const { return m_hashValue; }
    virtual void _CreateCveElements (DgnModelR, TransformCR unitsTransform, ICreateCveElementsHelper&); // added in graphite

public:
    static VisibleEdgeCacheImplP        Restore (ElementHandleCR el, DgnModelP rootModel);
    //ProxyDisplayModelP                  AddModel (DgnModelP, ProxyDisplayModelP parent);
    //void                                TestValid (bool& newerElement, bool& differentElementSet, bool& attachmentChanged, bool& missingReferences, DgnModelP rootModel, ViewportP viewport) const;
    //void                                AddToModelMap (DgnModelP modelRef, ProxyDisplayModelP proxyModel) { m_proxyModelMap[modelRef] = proxyModel; }

    //void                                SetAttachmentParameters (DgnAttachmentR ref);
    //bool                                UseWhileOutOfSynch() const { return !m_options.m_flags2.m_retainCacheWhileValidOnly; }
    //bool                                IsVisible (DisplayPath const& displayPath, ViewHandlerPassCR vhPass);
    //BitMaskCP                           GetLevelUsage (DgnModelP modelRef) const;
    //UInt32                              GetViewFlags () const;
    //bool                                DrawDgnModel (ViewContextR context, DgnModelR modelRef, DgnModelListP includeList, bool includeRefs, ViewHandlerPassR hitInfoPass);
    //bool                                VisitHitPath (HitPathCR hitPath, ViewContextR context);
    //void                                SaveOcclusionPaths (ElementRefP elementRef, DgnModelP rootDgnModel) const;

    //static bool                         OwnsXAttribute (XAttributeHandlerId handlerId, UInt32 xAttrId);
    //static bool                         ForceChangeToDisassociatedDependents (DgnModelP rootDgnModel);

    ProxyDisplayModelP GetRootProxyDisplayModelP() {return m_rootProxyModel;}

};  // VisibleEdgeCacheImpl

//typedef bmap <DgnModelP, UInt32>     T_DgnModelMap;
//typedef bvector <DgnModelP>          T_DgnModelVector;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static void getAttachmentIds (T_ProxyElementIdVector& ids, DgnModelP modelRef, DgnModelP rootDgnModel)
//    {
//    DgnAttachmentP       refFile;
//
//    if (modelRef == rootDgnModel || NULL == modelRef || NULL == (refFile = modelRef->AsDgnAttachmentP()))
//        return;
//
//    getAttachmentIds (ids, modelRef->GetParentDgnModelP (), rootDgnModel);
//    ids.push_back (refFile->GetElementId());
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCacheP   VisibleEdgeCache::Create (DgnModelP modelRef, CachedVisibleEdgeOptionsCR options) { return new VisibleEdgeCacheImpl (modelRef, options); }

VisibleEdgeCache::VisibleEdgeCache () : ProxyDisplayCacheBase (), m_originatingView (MAX_VIEWS), m_originatingViewGroup (0)/*, m_loadPreviewer (NULL)*/ { }
//VisibleEdgeCache::VisibleEdgeCache (DgnModelP modelRef, CachedVisibleEdgeOptionsCR options) : ProxyDisplayCacheBase (modelRef), m_originatingView (MAX_VIEWS), m_originatingViewGroup (0), m_loadPreviewer (NULL), m_options (options) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
//static void pushDgnModelAndParents (ViewContextR context, DgnModelP modelRef, DgnModelP rootModel)
//    {
//    if (NULL != modelRef && modelRef != rootModel)
//        {
//        pushDgnModelAndParents (context, modelRef->GetParentDgnModelP(), rootModel);
//        context.PushDgnModel (modelRef, false);
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//static Cryptographer*  getCryptographer ()
//    {
//    static  Cryptographer*  s_cryptographer;
//    if (NULL == s_cryptographer)
//        {
//        s_cryptographer = new Cryptographer();
//        s_cryptographer->initializeForHashOnly();
//        }
//    return s_cryptographer;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static LevelId  getDisplayableLevel (ElementRefP elemRef, DgnModelP modelRef)
//    {
//    if (0 != elemRef->GetUnstableMSElementCP()->GetLevel())
//        return elemRef->GetUnstableMSElementCP()->GetLevel();
//
//    ElementHandle      eh (elemRef, modelRef), componentEh;
//    if (ComplexHeaderDisplayHandler::GetComponentForDisplayParams (componentEh, eh))
//        return componentEh.GetElementCP()->GetLevel();
//
//    return INVALID_LEVEL_ID;
//    }

StatusInt   copyDsigRawHash (DsigRawHash& value, byte const*& dataP, byte const* dataEndP)                                   { return ProxyRestoreUtil::CopyData (&value, sizeof (value), dataP, dataEndP); }
StatusInt   copyCachedVisibleEdgeOptions (CachedVisibleEdgeOptions& value, byte const*& dataP, byte const* dataEndP)         { return ProxyRestoreUtil::CopyData (&value, sizeof (value), dataP, dataEndP); }



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphicsPiece
{
    Symbology           m_symbology;
    UInt16              m_flags;
    LineStyleParams*    m_styleParam;
    /*LevelId*/UInt32             m_virtualLevel;

    enum FlagsMask
        {
        FlagsMask_StyleParamPresent      = 0x0001 << 0,
        FlagsMask_FilterPlot             = 0x0001 << 1,
        FlagsMask_SolidFillEdge          = 0x0001 << 2,
        FlagsMask_ConstructionClass      = 0x0001 << 3,
        FlagsMask_VirtualLevelPresent    = 0x0001 << 4,
        FlagsMask_PatternClass           = 0x0001 << 5,
        FlagsMask_Uncloned               = 0x0001 << 15,
        } ;

    virtual                                 ~ProxyGraphicsPiece() { DELETE_AND_CLEAR (m_styleParam); };
    virtual void                            Draw (ViewContextR context, ElementHandleCR, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType proxyType) = 0;
    virtual void                            Dump () = 0;
    //virtual StatusInt                       GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId,  GPArrayParamCP edgeParam, ProxyGraphicsType proxyType) = 0;
    static  ProxyGraphicsPiece*             Restore (ProxyGraphicsType type, byte const* dataP, byte const* dataEnd, /*LevelId*/UInt32 hostLevel);
    DgnElementR                              CreateTemplateElement (DgnElementR templateElement) { return CreateTemplateElement (templateElement, m_symbology, m_styleParam, m_virtualLevel); }
    bool                                    DoFilterPlot () const           { return 0 != (m_flags & FlagsMask_FilterPlot); }
    bool                                    IsConstructionClass () const    { return 0 != (m_flags & FlagsMask_ConstructionClass); }
    bool                                    IsPatternClass () const         { return 0 != (m_flags & FlagsMask_PatternClass); }
    bool                                    IsSolidFillEdge () const        { return 0 != (m_flags & FlagsMask_SolidFillEdge); }
    bool                                    IsUncloned () const             { return 0 != (m_flags & FlagsMask_Uncloned); }
    Symbology const&                        GetSymbology () const           { return m_symbology; }
    /*LevelId*/UInt32                                 GetLevel () const               { return m_virtualLevel; }
    DgnElementClass                         GetClass () const               { return IsConstructionClass() ? DgnElementClass::Construction : (IsPatternClass() ? DgnElementClass::PatternComponent : DgnElementClass::Primary); }
    LineStyleParams*                        GetStyleParam () const          { return m_styleParam; }
    //void                                    VisitAsElement (ViewContextR context, ElementHandleCR, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType proxyType) ;

    virtual void _ApplyTransform (TransformCR trans) = 0;// added in graphite


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsPiece (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags) : m_symbology (templateElement.GetSymbology()), m_styleParam (NULL), m_virtualLevel(templateElement.GetLevelValue()), m_flags (0)
    {
    LineStyleParams          styleParam;

    if (SUCCESS == LineStyleLinkageUtil::GetParamsFromElement (&styleParam, &templateElement))
        {
        m_flags |= FlagsMask_StyleParamPresent;
        m_styleParam = new LineStyleParams (styleParam);
        }

#ifdef NEEDS_WORK_LEVEL_PLOT
    BoolInt         doPlot = true;

    if (NULL != modelRef && SUCCESS == mdlLevel_getPlot (&doPlot, modelRef, templateElement.GetLevel()) && ! doPlot)
        m_flags |= FlagsMask_FilterPlot;
#endif

    if (flags.IsSolidFillEdge())
        m_flags |= FlagsMask_SolidFillEdge;

    if (flags.IsConstructionClass())
        m_flags |= FlagsMask_ConstructionClass;

    if (!flags.IsPrecalculated())
        m_flags |= FlagsMask_Uncloned;

    if (DgnElementClass::PatternComponent == static_cast<DgnElementClass>(templateElement.GetElementClass()))
        m_flags |= FlagsMask_PatternClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsPiece (DgnElementCR templateElement, UInt16 flags) : m_symbology (templateElement.GetSymbology()), m_styleParam (NULL), m_flags (flags), m_virtualLevel(templateElement.GetLevelValue())
    {
    LineStyleParams          styleParam;

    if (SUCCESS == LineStyleLinkageUtil::GetParamsFromElement (&styleParam, &templateElement))
        {
        m_flags |= FlagsMask_StyleParamPresent;
        m_styleParam = new LineStyleParams (styleParam);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool Matches (DgnElementCR templateElement, DgnModelP modelRef)
    {
#ifdef NEEDS_WORK_LEVEL_PLOT
    BoolInt     doPlot = true;
    bool        doFilterPlot = false;

    if (NULL != modelRef &&
        SUCCESS == mdlLevel_getPlot (&doPlot, modelRef, templateElement.GetLevel()))
        doFilterPlot = (FALSE == doPlot);

    return doFilterPlot != DoFilterPlot();
#else
    return true;
#endif

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    SavePiece (ProxyDataBuffer& buffer, /*LevelId*/UInt32 hostLevel)
    {
    BeAssert (!IsUncloned());

    buffer.Write ((UInt16) (m_flags | ((NULL == m_styleParam) ? 0 : FlagsMask_StyleParamPresent) | ((hostLevel == m_virtualLevel) ? 0 : FlagsMask_VirtualLevelPresent)));
    buffer.Write (&m_symbology, sizeof (m_symbology));

    if (NULL != m_styleParam)
        {
        size_t      styleParamsBTF  = buffer.Write ((UInt32) 0);

        Int32       paramBytes;
        byte        paramData[sizeof(LineStyleParams)];

        if ((paramBytes = LineStyleLinkageUtil::AppendModifiers (paramData, m_styleParam, true)) > 0)
            buffer.Write (paramData, paramBytes);

        buffer.UpdateBytesToFollow (styleParamsBTF);
        }
    if (m_virtualLevel != hostLevel)
        buffer.Write (m_virtualLevel);


    Save (buffer);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementR CreateTemplateElement (DgnElementR templateElement, Symbology const& symbology, LineStyleParams const* styleParam, /*LevelId*/UInt32 level)
    {
    memset (&templateElement, 0, sizeof (Line_3d));

    ElementUtil::SetRequiredFields (templateElement, LINE_ELM, LevelId(), false, ElementUtil::ELEMDIM_3d);
    templateElement.SetSizeWordsNoAttributes(sizeof (Line_3d)/2);

    templateElement.SetSymbology(symbology);
    templateElement.SetLevel(level);
    if (NULL != styleParam)                             // TR# 333885 - Don't pass NULL to lineStyle_setStyleParams - it will set with active params!
        LineStyleLinkageUtil::SetStyleParams (&templateElement, const_cast <LineStyleParams*> (styleParam));

    return templateElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual void    ClonePiece (ElementCopyContextP copyContext)
//    {
//    if (IsUncloned())
//        {
//        DgnModelP        sourceModel, destModel;
//
//        copyContext->GetModels (&sourceModel, &destModel, NULL, NULL);
//        m_symbology.color = copyContext->RemapColorIndex (m_symbology.color);
//        m_symbology.style = LineStyleUtil::RemapLineStyleID (m_symbology.style, sourceModel, destModel);
//        m_flags &= ~FlagsMask_Uncloned;
//        }
//    }


//protected:
    virtual void                            Save (ProxyDataBuffer& buffer) = 0;
//    virtual void                            Clone (ElementCopyContextP copyContext) = 0;

};  //  ProxyGraphicsPiece


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct  ProxyDisplayXGraphics : ProxyGraphicsPiece
{
    XGraphicsContainerP     m_xGraphics;

    virtual             ~ProxyDisplayXGraphics ()                                                                           { delete m_xGraphics; }
                         ProxyDisplayXGraphics (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags) : ProxyGraphicsPiece (templateElement, modelRef, flags)   { m_xGraphics = new XGraphicsContainer(); }
//    virtual void        Clone (ElementCopyContextP copyContext) override                                                           { m_xGraphics->DoClone (*copyContext); }

virtual void _ApplyTransform (TransformCR trans); // added in graphite

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyDisplayXGraphics (DgnElementCR templateElement, UInt16 flags, byte const* dataP, UInt32 dataSize) : ProxyGraphicsPiece (templateElement, flags)
    {
    m_xGraphics = new XGraphicsContainer (dataP, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyDisplayXGraphics (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags, XGraphicsContainerCR xGraphics) : ProxyGraphicsPiece (templateElement, modelRef, flags)
    {
    m_xGraphics = new XGraphicsContainer(xGraphics);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual StatusInt   GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId, GPArrayParamCP edgeParam, ProxyGraphicsType proxyType) override
//    {
//    return m_xGraphics->ExtractProxyCutEdge (edgeGraphics, edgeId);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    Save (ProxyDataBuffer& buffer) override
    {
    buffer.Write (m_xGraphics->GetData(), m_xGraphics->GetDataSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    Draw (ViewContextR context, ElementHandleCR el, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType  proxyType) override
    {
#ifndef NDEBUG
    /*ElementId*/UInt64         elementId = el.GetElementCP()->GetElementId().GetValueUnchecked();
    static bool                 s_dumpXGraphics;
    static /*ElementId*/UInt64            s_dumpId = (UInt64)-1;

    if (s_dumpXGraphics || elementId == s_dumpId)
        m_xGraphics->Dump (el.GetDgnModelP());
#endif

#ifdef WIP_CVE_SYMBOLOGY
    ElemDisplayParamsP          displayParams = context.GetCurrentDisplayParams();
    FillDisplay                 fillDisplay = (ProxyGraphicsType_CutFill == proxyType || ProxyGraphicsType_PassthroughUnderlay == proxyType) ? FillDisplay::Always : FillDisplay::Never;
    bool                        hasStyleParams = (NULL != m_styleParam);
    Symbology                   dSymb;

    dSymb.style  = displayParams->GetLineStyle ();
    dSymb.color  = displayParams->GetLineColor ();
    dSymb.weight = displayParams->GetWeight ();

    if (0 != memcmp (&m_symbology, &dSymb, sizeof (m_symbology)) ||
        (fillDisplay != displayParams->GetFillDisplay ()) ||
        IsSolidFillEdge() ||
        (hasStyleParams != (NULL != displayParams->GetLineStyleParams ())) ||
        (hasStyleParams && 0 != memcmp (m_styleParam, displayParams->GetLineStyleParams (), sizeof (*m_styleParam))))
        {
        DgnModelP    symbologyDgnModel = el.GetDgnModelP();//IsUncloned() ? el.GetDgnModelP() :  proxyCache.GetParentDgnModelP();

        displayParams->SetFillDisplay (fillDisplay);
        displayParams->SetWeight (m_symbology.weight);
        displayParams->SetLineStyle (m_symbology.style, hasStyleParams ? m_styleParam : NULL);

        if (NULL != symbologyDgnModel)
            displayParams->SetLineStyleDgnModel (*context.GetCurrentModel (), *symbologyDgnModel);

        if (NULL != context.GetViewport() && IsSolidFillEdge())
            {
            displayParams->SetLineColorTBGR (context.GetViewport()->GetSolidFillEdgeColor (m_symbology.color, symbologyDgnModel));
            }
        else
            {
            setDisplayParamsLineColor (*displayParams, m_symbology.color, symbologyDgnModel, context.GetCurrentModel());
            }
        if (displayParams->IsLineColorTBGR())
            displayParams->SetFillColorTBGR (displayParams->GetLineColorTBGR());
        else
            displayParams->SetFillColor (displayParams->GetLineColor());


        context.CookDisplayParams ();
        }
    context.ActivateOverrideMatSymb ();
#endif

    m_xGraphics->DrawProxy (context, el, qvIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    Dump () override
    {
    m_xGraphics->Dump (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static ProxyDisplayXGraphics*   Restore (DgnElementCR templateElement, UInt16 flags, byte const* dataP, byte const* dataEndP)
    {
    if (dataEndP - dataP <= sizeof (XGraphicsHeader))
        throw ProxyRestoreUtil::ReadError ();

    return new ProxyDisplayXGraphics (templateElement, flags, dataP, (UInt32) (dataEndP - dataP));
    }

};  //  ProxyDisplayXGraphics


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphicsPassthrough : ProxyDisplayXGraphics
{

    ProxyGraphicsPassthrough (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags, XGraphicsContainerCR xGraphics) : ProxyDisplayXGraphics (templateElement, modelRef, flags, xGraphics) { }
    ProxyGraphicsPassthrough (DgnElementCR templateElement, UInt16 flags, byte const* dataP, UInt32 dataSize) : ProxyDisplayXGraphics (templateElement, flags, dataP, dataSize) { }

    //virtual StatusInt  GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId,  GPArrayParamCP edgeParam, ProxyGraphicsType proxyType) override { return ERROR; }
};  // ProxyGraphicsPassthrough


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphicsEdgeOrWire : ProxyDisplayXGraphics
{

    ProxyGraphicsEdgeOrWire (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags) : ProxyDisplayXGraphics (templateElement, modelRef, flags)  { }
    ProxyGraphicsEdgeOrWire (DgnElementCR templateElement, UInt16 flags, byte const* dataP, UInt32 dataSize) : ProxyDisplayXGraphics (templateElement, flags, dataP, dataSize) { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static ProxyGraphicsEdgeOrWire*   Restore (DgnElementCR templateElement, UInt16 flags, byte const* dataP, byte const* dataEndP)
    {
    if (dataEndP - dataP <= sizeof (XGraphicsHeader))
        throw ProxyRestoreUtil::ReadError ();

    return new ProxyGraphicsEdgeOrWire (templateElement, flags, dataP, (UInt32) (dataEndP - dataP));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId, GPArrayParamCP edgeParam, ProxyGraphicsType proxyType)
//    {
//    ProxyHLEdgeSegmentId  segmentId;
//
//    if (SUCCESS != segmentId.Init (edgeId) || proxyType != segmentId.m_edgeId.GetProxyGraphicsType())
//        return ERROR;
//
//    return m_xGraphics->ExtractProxyGPArray (edgeGraphics, segmentId, edgeParam);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ApplySymbologyOverrides (Symbology const& symbology, ViewContextR context, OvrMatSymbR overrideMatSymb, CVEHLineSymbology const& symbologyOverrides)
    {
    UInt32 flags = overrideMatSymb.GetFlags();

    if (symbologyOverrides.colorOverride && 0 == (flags & MATSYMB_OVERRIDE_Color))
        context.SetIndexedLineColor (overrideMatSymb, symbologyOverrides.color);

    if (symbologyOverrides.styleOverride && 0 == (flags & MATSYMB_OVERRIDE_Style))
        context.SetIndexedLinePattern (overrideMatSymb, symbologyOverrides.style);

    if (symbologyOverrides.weightOverride && 0 == (flags & MATSYMB_OVERRIDE_RastWidth))
        context.SetIndexedLineWidth (overrideMatSymb, symbologyOverrides.weight);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplySymbologyOverrides (ViewContextR context, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType  proxyType)
    {
    OvrMatSymbP             overrideMatSymb = context.GetOverrideMatSymb();

    overrideMatSymb->SetProxy (proxyType == ProxyGraphicsType_HiddenEdge || proxyType == ProxyGraphicsType_VisibleEdge, proxyType == ProxyGraphicsType_HiddenEdge || proxyType == ProxyGraphicsType_HiddenWire);

    switch (proxyType)
        {
        case ProxyGraphicsType_HiddenWire:
        case ProxyGraphicsType_HiddenEdge:
            ApplySymbologyOverrides (m_symbology, context, *overrideMatSymb, proxyCache.GetOptions().m_hidden);
            break;

        case ProxyGraphicsType_VisibleWire:
        case ProxyGraphicsType_VisibleEdge:
            ApplySymbologyOverrides (m_symbology, context, *overrideMatSymb, proxyCache.GetOptions().m_visible);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    Draw (ViewContextR context, ElementHandleCR el, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType  proxyType) override
    {
    UInt32      saveOverrideFlags = context.GetOverrideMatSymb()->GetFlags ();

    ApplySymbologyOverrides (context, proxyCache, proxyType);
    ProxyDisplayXGraphics::Draw (context, el, qvIndex, proxyCache, proxyType);

    context.GetOverrideMatSymb()->SetFlags (saveOverrideFlags);
    }
};  //  ProxyGraphicsEdgeOrWire

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyDisplayEdgeOrWire
{
    GPArraySmartP           m_curve;
    GPArrayIntervals        m_segments;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyDisplayEdgeOrWire (GPArrayCR curve, GPArrayIntervalsCP intervals)
    {
    m_curve->CopyContentsOf (const_cast <GPArrayP> (&curve));
    AddIntervals (intervals);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void AddIntervals (GPArrayIntervalsCP intervals)
    {
    if (NULL != intervals)
        {
        for (GPArrayIntervals::const_iterator curr = intervals->begin(); curr != intervals->end(); curr++)
            m_segments.push_back (*curr);
        }
    }
};


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2011
+===============+===============+===============+===============+===============+======*/
struct EdgeMapKey
{
    ProxyHLEdgeSegmentId    m_segmentId;
    CompoundDrawStatePtr    m_compoundDrawState;

    EdgeMapKey() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeMapKey (ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds) : m_segmentId (segmentId)
    {
    if (NULL != cds)
        m_compoundDrawState = CompoundDrawState::Create (*cds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (EdgeMapKey const& rhs) const
    {
    if (!m_compoundDrawState.IsNull() &&
        !rhs.m_compoundDrawState.IsNull() &&
        ! (*m_compoundDrawState == *rhs.m_compoundDrawState))
        {
        return *m_compoundDrawState < *rhs.m_compoundDrawState;
        }
    if (m_compoundDrawState.IsNull() != rhs.m_compoundDrawState.IsNull())
        return !m_compoundDrawState.IsNull();

    return m_segmentId < rhs.m_segmentId;
    }

};  // EdgeMapKey

typedef bmap <EdgeMapKey, ProxyDisplayEdgeOrWire*> T_ProxyEdgeMap;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphicsEdgeOrWireCollector : ProxyGraphicsEdgeOrWire
{

DEFINE_T_SUPER(ProxyGraphicsEdgeOrWire)

    T_ProxyEdgeMap              m_clippedEdges;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsEdgeOrWireCollector (DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags) : ProxyGraphicsEdgeOrWire (templateElement, modelRef, flags)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
~ProxyGraphicsEdgeOrWireCollector ()
    {
    for (T_ProxyEdgeMap::iterator curr = m_clippedEdges.begin(); curr != m_clippedEdges.end(); curr++)
        delete curr->second;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AddGPA  (GPArrayCR gpa, GPArrayIntervalsP intervals, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool isClipped)
    {
    static bool         s_skipMerge = false;

    if (s_skipMerge || (isClipped && segmentId.m_edgeId.IsAssociable()))
        {
        EdgeMapKey    mapKey(segmentId, cds);

        T_ProxyEdgeMap::iterator found = m_clippedEdges.find (mapKey);

        if (found == m_clippedEdges.end())
            {
            m_clippedEdges[mapKey] = new ProxyDisplayEdgeOrWire (gpa, intervals);
            }
        else
            {
#ifdef DEBUG_PASS_IDS
            BeAssert (false);
#endif
            BeAssert (gpa.GetCount() == found->second->m_curve->GetCount());
            found->second->AddIntervals (intervals);
            }
        }
    else
        {
        SaveCurves (*m_xGraphics, gpa, intervals, segmentId, NULL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    Draw (ViewContextR context, ElementHandleCR el, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType  proxyType) override
    {
    ProxyGraphicsEdgeOrWire::Draw (context, el, qvIndex, proxyCache, proxyType);

    if (!m_clippedEdges.empty())
        {
        UInt32              saveOverrideFlags = context.GetOverrideMatSymb()->GetFlags ();
        XGraphicsContainer  tempXGraphics;

        ApplySymbologyOverrides (context, proxyCache, proxyType);

        for (T_ProxyEdgeMap::iterator curr = m_clippedEdges.begin(), end = m_clippedEdges.end(); curr != end; curr++)
            SaveCurves (tempXGraphics, *curr->second->m_curve, &curr->second->m_segments, curr->first.m_segmentId, curr->first.m_compoundDrawState.get());

        context.ActivateOverrideMatSymb ();
        tempXGraphics.DrawProxy (context, el, qvIndex);

        context.GetOverrideMatSymb()->SetFlags (saveOverrideFlags);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    Save (ProxyDataBuffer& buffer) override
    {
    for (T_ProxyEdgeMap::iterator curr = m_clippedEdges.begin(), end = m_clippedEdges.end(); curr != end; curr++)
        SaveCurves (*m_xGraphics, *curr->second->m_curve, &curr->second->m_segments, curr->first.m_segmentId, curr->first.m_compoundDrawState.get());

    BeAssert (!m_xGraphics->IsEmpty());
    ProxyGraphicsEdgeOrWire::Save (buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    SaveCurves (XGraphicsContainerR container, GPArrayCR gpa, GPArrayIntervalsP segments, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateP cds)
    {
    if (NULL == segments || segments->empty() || segments->front() == GPArrayInterval (gpa))
        {
        container.AddProxyCurve (gpa, NULL, segmentId, cds);
        }
    else
        {
        GPArrayParam            param (0.0);
        static double           s_minimumCombineLength = 1.0E-4, s_minimumOutputLength = 1.0E-3;
        GPArraySegmentLengths   segmentLengths(gpa);

        std::sort (segments->begin(), segments->end(), GPArrayInterval::Compare);

        for (GPArrayIntervals::const_iterator  interval = segments->begin(), end = segments->end(), next; interval != end; interval = next)
            {
            GPArrayParam      startInterval = interval->m_start, endInterval = interval->m_end;
            for (next = interval + 1; next != end && segmentLengths.LengthFromParam (next->m_start) < segmentLengths.LengthFromParam (endInterval) + s_minimumCombineLength; next++)
                if (next->m_end.GetValue() > endInterval.GetValue())
                    endInterval = next->m_end;

            GPArrayInterval     outputInterval (startInterval, endInterval, &gpa);

            if (segmentLengths.IntervalLength (outputInterval) > s_minimumOutputLength)
                container.AddProxyCurve (gpa, &outputInterval, segmentId, cds);

            param = endInterval;
            }
        }
    BeAssert (!container.IsEmpty());
    }


};  // ProxyGraphicsEdgeOrWireCollector


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsPiece*  ProxyGraphicsPiece::Restore (ProxyGraphicsType type, byte const* dataP, byte const* dataEndP, /*LevelId*/UInt32 hostLevel)
    {
    UInt16              flags = 0;
    LineStyleParams*         styleParamP = NULL;

    ProxyRestoreUtil::CopyData (flags, dataP, dataEndP);

    AlignedArray<> symbologyBuf;
    Symbology const*    symbology = (Symbology const*) symbologyBuf.GetAlignedData (dataP, sizeof(Symbology));
    dataP += sizeof (Symbology);

    if (0 != (flags & FlagsMask_StyleParamPresent))
        {
        UInt32              styleParamBTF;
        LineStyleParams          styleParam;

        ProxyRestoreUtil::CopyData (styleParamBTF, dataP, dataEndP);
        if (dataP + styleParamBTF > dataEndP)
            throw ProxyRestoreUtil::ReadError ();

        if (0 != styleParamBTF)
            {
            memset (&styleParam, 0, sizeof (styleParam));
            LineStyleLinkageUtil::ExtractModifiers (styleParamP = &styleParam, const_cast <byte*> (dataP), true);
            dataP += styleParamBTF;
            }
        }

    /*LevelId*/UInt32     level;

    if (0 != (flags & FlagsMask_VirtualLevelPresent))
        {
        UInt32 levelIdValue;
        ProxyRestoreUtil::CopyData (levelIdValue, dataP, dataEndP);
        level = /*LevelId*/UInt32 (levelIdValue);
        }
    else
        level = hostLevel;

    //MSElement           templateElement;  No! this blows out the stack on mobile devices!
    byte rawBuf[sizeof (Line_3d)];
    AlignedArray<> templateElementBuf;
    auto& templateElement = *(DgnElementP)templateElementBuf.GetAlignedData (rawBuf, sizeof(rawBuf));
    CreateTemplateElement (templateElement, *symbology, styleParamP, level);

    switch (type)
        {
        case ProxyGraphicsType_Cut:
        case ProxyGraphicsType_CutFill:
            return ProxyDisplayXGraphics::Restore (templateElement, flags, dataP, dataEndP);

        case ProxyGraphicsType_VisibleEdge:
        case ProxyGraphicsType_HiddenEdge:
        case ProxyGraphicsType_VisibleWire:
        case ProxyGraphicsType_HiddenWire:
            return ProxyGraphicsEdgeOrWire::Restore (templateElement, flags, dataP, dataEndP);

        case ProxyGraphicsType_PassthroughAnnotation:
        case ProxyGraphicsType_PassthroughUnderlay:
            return ProxyGraphicsPassthrough::Restore (templateElement, flags, dataP, dataEndP);

        default:
            return NULL;
        }
    }

typedef std::multimap <ProxyGraphicsType, ProxyGraphicsPiece*>       T_ProxyGraphicsMap;
typedef std::pair <ProxyGraphicsType, ProxyGraphicsPiece*>           T_ProxyGraphicsMapPair;

static  Symbology   s_defaultSymbology;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyGraphics
{
    T_ProxyGraphicsMap              m_pieces;

    Symbology const&    GetSymbology() const  { return m_pieces.empty() ? s_defaultSymbology : m_pieces.begin()->second->m_symbology; }
    LineStyleParams const*   GetStyleParam() const { return m_pieces.empty() ? NULL : m_pieces.begin()->second->m_styleParam; }
                        ProxyGraphics ()  {}
    void                Draw (ElementHandleCR el, ViewContextR context, VisibleEdgeCacheImplR proxyCache, ProxySymbologyOverride symbologyOverride);
    void                Draw (T_ProxyGraphicsMap::iterator it, ElementHandleCR el, UInt32 qvIndex, ViewContextR context, VisibleEdgeCacheImplR proxyCache, ProxySymbologyOverride symbologyOverride);

    void ApplyTransform (TransformCR);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphics (byte const*& dataP, byte const* dataEndP, /*LevelId*/UInt32 hostLevel)
    {
    while (dataP < dataEndP)
        {
        UInt32      type;
        UInt32      graphicsBTF;

        if (SUCCESS == ProxyRestoreUtil::CopyData (type, dataP, dataEndP) &&
            SUCCESS == ProxyRestoreUtil::CopyData (graphicsBTF, dataP, dataEndP) &&
            type >= ProxyGraphicsType_Min && type < ProxyGraphicsType_Max)
            {
            m_pieces.insert (T_ProxyGraphicsMapPair ((ProxyGraphicsType) type, ProxyGraphicsPiece::Restore ((ProxyGraphicsType) type, dataP, dataP + graphicsBTF, hostLevel)));
            dataP += graphicsBTF;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
~ProxyGraphics()
    {
    for (T_ProxyGraphicsMap::iterator curr = m_pieces.begin(); curr != m_pieces.end(); curr++)
        DELETE_AND_CLEAR (curr->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void        Clone (ElementCopyContextP copyContext)
//    {
//    for (T_ProxyGraphicsMap::iterator curr = m_pieces.begin(); curr != m_pieces.end(); curr++)
//        curr->second->ClonePiece (copyContext);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            Dump ()
    {
    for (T_ProxyGraphicsMap::iterator curr = m_pieces.begin(); curr != m_pieces.end(); curr++)
        curr->second->Dump ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Save (ProxyDataBuffer& buffer, /*LevelId*/UInt32 hostLevel) const
    {
    for (T_ProxyGraphicsMap::const_iterator curr = m_pieces.begin(), end = m_pieces.end();  curr != end; curr++)
        {
        buffer.Write ((UInt32) curr->first);

        size_t      graphicsBTFLocation  = buffer.Write ((UInt32) 0);                           // xGraphicsBytes To Follow
        curr->second->SavePiece (buffer, hostLevel);
        buffer.UpdateBytesToFollow (graphicsBTFLocation);
        }
    return SUCCESS;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt AddGPA  (GPArrayCR gpa, GPArrayIntervalsP intervals, DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool isClipped)
//    {
//    ProxyGraphicsType                   type  = segmentId.m_edgeId.GetProxyGraphicsType();
//    T_ProxyGraphicsMap::iterator        found = m_pieces.find(type);
//    ProxyGraphicsEdgeOrWireCollector*   pGraphics;
//
//    if (found == m_pieces.end() || ! found->second->Matches (templateElement, modelRef))
//        m_pieces.insert (T_ProxyGraphicsMapPair (type, pGraphics = new ProxyGraphicsEdgeOrWireCollector (templateElement, modelRef, flags)));
//    else
//        pGraphics = dynamic_cast <ProxyGraphicsEdgeOrWireCollector*> (found->second);
//
//    return pGraphics->AddGPA (gpa, intervals, segmentId, cds, isClipped);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   AddCutGraphics (XGraphicsContainerCR xGraphics, DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags, bool filled)
//    {
//    ProxyGraphicsType               type = filled ? ProxyGraphicsType_CutFill : ProxyGraphicsType_Cut;
//
//    m_pieces.insert (T_ProxyGraphicsMapPair (type, new ProxyDisplayXGraphics (templateElement, modelRef, flags, xGraphics)));
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   AddPassthrough (XGraphicsContainerCR xGraphics, ProxyGraphicsType type, DgnElementCR templateElement, DgnModelP modelRef, ProxyGraphicsFlags flags)
//    {
//    m_pieces.insert (T_ProxyGraphicsMapPair (type, new ProxyDisplayXGraphics (templateElement, modelRef, flags, xGraphics)));
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId,  GPArrayParamCP edgeParam)
//    {
//    for (T_ProxyGraphicsMap::iterator curr = m_pieces.begin(), end = m_pieces.end();  curr != end; curr++)
//        if (SUCCESS == curr->second->GetProxyGraphicsFromEdgeId (edgeGraphics, edgeId, edgeParam, curr->first))
//            return SUCCESS;
//
//    return ERROR;
//    }

};  // ProxyGraphics


enum
    {
    ProxyElementFlagsMask_Concealed      = 0x0001 << 0,
    ProxyElementFlagMask_Root            = 0x0001 << 2,
    };

///*=================================================================================**//**
//* @bsiclass                                                     Ray.Bentley     01/08
//+===============+===============+===============+===============+===============+======*/
//struct GetLocalTransformViewContext : NullContext
//{
//SimplifyViewDrawGeom    m_output;
//
//TransformCP   GetCurrTrans() { return GetCurrLocalToFrustumTransformCP(); }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//GetLocalTransformViewContext ()
//    {
//    m_IViewDraw = &m_output;
//    m_IDrawGeom = &m_output;
//    m_output.SetViewContext (this);
//    }
// };  // GetLocalTransformViewContext
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//static  TransformCP     getLocalTransform (DisplayPath const& displayPath, int index)
//    {
//    GetLocalTransformViewContext       context;
//    ElementHandle                      elHandle (displayPath.GetPathElem(index), displayPath.GetRoot());
//    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (elHandle.GetHandler ());
//
//    if (extension)
//        extension->_PushDisplayEffects (elHandle, context);
//
//    return NULL == context.GetCurrLocalToFrustumTransformCP() ? NULL : new Transform (*context.GetCurrLocalToFrustumTransformCP());
//    }

static int s_proxyElements;
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct  ProxyElement //: VisibleEdgeCacheElement
{
private:
    double                          m_lastModified;
    UInt32                          m_level;
    ProxyGraphicsP                  m_graphics;
    struct ProxyElementMap*         m_children;
    UInt32                          m_flags;
    TransformCP                     m_transform;
    VisibleEdgeCacheImplR           m_proxyCache;
    //ProxyElementElemRefP            m_transientElemRef;

public:
    ProxyElement (VisibleEdgeCacheImplR cache) : m_proxyCache (cache), m_lastModified (0.0)/*, m_level(INVALID_LEVEL)*/, m_graphics (NULL), m_children (NULL), m_flags (0), m_transform(NULL)/*, m_transientElemRef (NULL)*/ {s_proxyElements++; }
    ~ProxyElement ();

    //virtual void                    _VisibleEdgeCalculationDraw (ViewContextR context, ElementHandleCR eh) const override;

    //ProxyElementP                   FindElement (DisplayPath const& displayPath, int index, TransformP transform = NULL) const;
    //ProxyElementP                   FindElement (ElemIdPathCR path, int index, TransformR transform) const;
    //ProxyElementP                   FindOrCreateElement (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, DgnElementCR templateElement, int index);
    StatusInt                       Save (ProxyDataBuffer& buffer) const;
    StatusInt                       Restore (VisibleEdgeCacheImplR cache, byte const*& dataP, byte const* dataEndP, DgnModelP dgnModel, DgnModelP modelRef);
    void                            Draw (ViewContextR context, ElementHandleCR eh) const;
    //void                            DrawStatic (ViewContextR context, DgnModelR rootParentDgnModel, DgnModelR modelRef, UInt32 displayFlags);
    //bool                            FlashHitEdge (ViewContextR context, ProxyEdgeIdDataCR edgeId, ElementHandleCR eh);
    //bool                            IsVisible (DisplayPath const& displayPath, int index);
    //void                            SetTransientElemRef (ProxyElementElemRefP elemRef) { m_transientElemRef = elemRef; }
    //void                            ResolveTransient (ElementId elementId, DgnModelP modelRef);
    //void                            ClearElementModifiedTimes (DgnModelP dgnModel);
    //void                            GetConcealedPaths (T_ProxyDisplayPathVector& paths, ProxyDisplayPathR currPath) const;
    //bool                            ContainsConcealed () const;
    //void                            ClearConcealed ();

    inline ProxyGraphicsP           GetGraphics ()          { return m_graphics; }
    inline ProxyElementMap*         GetChildren ()          { return m_children; }
    inline double                   GetLastModified ()      { return m_lastModified; }
    inline bool                     IsRoot ()               { return 0 != (m_flags & ProxyElementFlagMask_Root); }
    //inline bool                     IsConcealed () const    { return 0 != (m_flags & ProxyElementFlagsMask_Concealed); }
    inline /*LevelId*/UInt32                  GetLevel()              { return m_level; }
    //inline ProxyElementElemRefP     GetTransientElemRef()   { return m_transientElemRef; }
    Symbology const&                GetSymbology ()         { return (NULL == m_graphics) ? s_defaultSymbology : m_graphics->GetSymbology(); }
    LineStyleParams const*          GetStyleParam ()        { return (NULL == m_graphics) ? NULL : m_graphics->GetStyleParam(); }
    //DgnModelP                    GetSymbologyDgnModel()  { return GetParentDgnModelP(); }

    void                            Dump (/*ElementId*/UInt64 id, size_t childLevel);
    //void                            Clone (ElementCopyContextP copyContext);
    TransformCP                     GetTransform() const { return m_transform; }

    void ApplyTransform (TransformCR trans); // added in graphite
    void Draw0 (ViewContextR context, ElementHandleCR eh) const; // added in graphite

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElement (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, int index) : m_proxyCache (cache), m_graphics (NULL), m_children (NULL), m_flags (0)
//    {
//    ElementRefP      elemRef = displayPath.GetPathElem (index);
//    DgnElementCP     element = elemRef->GetUnstableMSElementCP();
//
//    s_proxyElements++;
//
//    BeAssert (element->IsGraphic());
//    m_lastModified = element->ehdr.lastModified;
//    m_level        = getDisplayableLevel (elemRef, displayPath.GetRoot());
//    m_level = 64;
//    m_transform    = getLocalTransform (displayPath, index);
//    if (0 == index)
//        m_flags |= ProxyElementFlagMask_Root;
//
//    m_transientElemRef = NULL;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool        InSynch (ElementRefP elRef) const
//    {
//    Elm_hdr const*        ehdr = elRef->GetElementHeaderCP();
//
//    return 0.0 == m_lastModified || ehdr->lastModified == m_lastModified;
//    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyGraphicsP           GetOrCreateGraphics ()
//    {
//    if (NULL == m_graphics)
//        m_graphics = new ProxyGraphics ();
//
//    return m_graphics;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR edgeId,  GPArrayParamCP edgeParam)
//    {
//    return NULL == m_graphics ? ERROR : m_graphics->GetProxyGraphicsFromEdgeId (edgeGraphics, edgeId, edgeParam);
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool SetConcealed (bool concealed)
//    {
//    if (concealed == IsConcealed())
//        return false;
//
//    if (concealed)
//        m_flags |= ProxyElementFlagsMask_Concealed;
//    else
//        m_flags &= ~ProxyElementFlagsMask_Concealed;
//
//    return true;
//    }
};  // ProxyElement

typedef bmap </*ElementId*/UInt64,  ProxyElementP>     T_ElemIdProxyMap;

static VisibleEdgeCacheImpl s_fakeCache; // added in graphite

/*=================================================================================**//**
* Information about how a ProxyElement was computed.
* @bsiclass                                                     Sam.Wilson      02/2014
+===============+===============+===============+===============+===============+======*/
struct ProxyElementDetails // added in graphite
{
private:
    bool                m_valid;
    ViewHandlerPass     m_viewHandlerPass;
    double m_zlow;
    double m_zhigh;

public:
    ProxyElementDetails() : m_valid(false) {;}
    ProxyElementDetails (ViewHandlerPassCR p, double zl, double zh) : m_valid(true), m_viewHandlerPass(p), m_zlow(zl), m_zhigh(zh) {;}
    bool IsValid() const {return m_valid;}
    void Store (EditElementHandleR);
    BentleyStatus Load (ElementHandleCR);
    double GetZlow() const {return m_zlow;}
    double GetZHigh() const {return m_zhigh;}
    ViewHandlerPassCR GetViewHandlerPass() const {return m_viewHandlerPass;}
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyElementDisplayHandler : DisplayHandler
{

    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (ProxyElementDisplayHandler, PROXYELEMENTHANDLER_NOTEXPORTED)

    ProxyElementDisplayHandler () { }

    virtual void  _GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength) override;
    virtual void  _Draw (ElementHandleCR el, ViewContextR context) override;
    virtual void  _GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials) override;
    static ElementHandlerId GetElemHandlerId() {return ElementHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_CachedVisibleEdgeHandler);} // added in graphite
    BentleyStatus RestoreProxyElement (ProxyElement& proxy, ElementHandleCR el); // added in graphite
    void StoreProxyElement (EditElementHandleR eeh, ProxyElement const& proxy); // added in graphite
    void StoreProxyTarget (EditElementHandleR el, ElementId); // added in graphite
    ElementId GetProxyTarget (ElementHandleCR el); // added in graphite

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            _GetTypeName (WStringR string, UInt32 desiredLength) override    { string = L"ProxyElementDisplayHandler";}//g_dgnHandlersResources->GetString (MSGID_OriginalElementUnavailable); }


};  // ProxyElementDisplayHandler

ELEMENTHANDLER_DEFINE_MEMBERS(ProxyElementDisplayHandler)


///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      07/2011
//+===============+===============+===============+===============+===============+======*/
//struct ProxyElemRef : ElementRef
//{
//
//protected:
//    ProxyElemRef(DgnModelP model) : ElementRef(model,/*isGraphics*/true) {;}
//    ~ProxyElemRef ()     { ClearAllAppData(GetHeapZone(), false); }
//
//public:
//    ExtendedElm                 m_element;
//
//    //virtual DgnModelP           _GetDgnModel () override        { return NULL;}
//    virtual ElementRefType      _GetRefType () override         { return ELEMENT_REF_TYPE_ProxyDisplay; }
//    virtual DgnElementCP         _GetElemPtrC() const override   { return (DgnElementCP) &m_element; }
//    virtual SubElementRefVecP   _GetSubElements() const         { return NULL; }
//    virtual ElementRefP         _GetMyParent() const            { return NULL; }
//    virtual UInt32 _AddRef() const {return 1;}
//    virtual UInt32 _Release() const {return 1;}
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//virtual QvCache*    _GetMyQvCache ()
//    {
//    static  QvCache*                        s_qvCache;
//
//    if (!s_qvCache)
//        s_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache ();
//
//    return s_qvCache;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//virtual HeapZone&  _GetHeapZone() override
//    {
//    static HeapZone s_noHeapZone;
//
//    return s_noHeapZone;
//    }
//
//}; // ProxyElemRef
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      07/2011
//+===============+===============+===============+===============+===============+======*/
//struct ProxyElementElemRef : ProxyElemRef
//{
//
//    ProxyElementR                           m_proxyElement;
//    static  ProxyElementDisplayHandler&     s_handler;
//
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementElemRef (ElementId elementId, DgnModelP modelRef, ProxyElementR proxyElement) : ProxyElemRef(modelRef), m_proxyElement (proxyElement)
//    {
//    m_handler = &s_handler;
//
//    memset (&m_element, 0, sizeof (m_element));
//    ElementUtil::SetRequiredFields (*((DgnElementP) &m_element), EXTENDED_ELM, LevelId(proxyElement.GetLevel()), false, ElementUtil::ELEMDIM_3d);
//    m_element.GetSizeWords() = m_element.GetAttributeOffset() = sizeof (m_element) / 2;
//    m_element.ehdr.uniqueId = elementId.GetValueUnchecked();
//    m_element.ehdr.lastModified = -1.0;     // Transients are always unsynched.
//    m_element.dhdr.symb  = proxyElement.GetSymbology();
//    m_element.GetLevel() = proxyElement.GetLevel();
//
//    EditElementHandle      el (this, modelRef);
//
//    DRange3d            dRange;
//    dRange.init ();
//
//    el.GetDisplayHandler()->CalcElementRange (el, dRange, NULL);
//    DataConvert::DRange3dToScanRange (m_element.dhdr.range, dRange);
//    }
//
//
//};  // ProxyElementElemRef
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      07/2011
//+===============+===============+===============+===============+===============+======*/
//struct ProxyGraphicsPieceDisplayHandler : DisplayHandler
//{
//    DEFINE_T_SUPER(DisplayHandler)
//    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (ProxyGraphicsPieceDisplayHandler, PROXYELEMENTHANDLER_NOTEXPORTED)
//
//    virtual void  _Draw (ElementHandleCR el, ViewContextR context) override;
//    virtual void  _GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials) override;
//
//};  // ProxyPieceDisplayHandler
//
//
//ELEMENTHANDLER_DEFINE_MEMBERS(ProxyGraphicsPieceDisplayHandler)
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      07/2011
//+===============+===============+===============+===============+===============+======*/
//struct ProxyGraphicsPieceElemRef: ProxyElemRef
//{
//
//    ProxyGraphicsPieceR                         m_proxyPiece;
//    UInt32                                      m_qvIndex;
//    ProxyGraphicsType                           m_type;
//    VisibleEdgeCacheImplR                       m_cache;
//    static  ProxyGraphicsPieceDisplayHandler&   s_handler;
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyGraphicsPieceElemRef (ProxyGraphicsPieceR proxyPiece, UInt32 qvIndex, VisibleEdgeCacheImplR cache, ProxyGraphicsType type, DgnModelP modelRef) : ProxyElemRef(modelRef), m_proxyPiece (proxyPiece), m_qvIndex (qvIndex), m_cache (cache), m_type (type)
//    {
//    m_handler = &s_handler;
//
//    EditElementHandle      eeh;
//    ExtendedElementHandler::InitializeElement (eeh, NULL, modelRef, true);
//    eeh.GetElementCP()->CopyTo ((DgnElementR) m_element);
//
//    m_element.dhdr.symb  = proxyPiece.GetSymbology();
//    m_element.dhdr.props.b.elementClass = static_cast<UInt16>(proxyPiece.GetClass());
//    m_element.GetLevel() = proxyPiece.GetLevel();
//    m_element.dhdr.range.low.x  = m_element.dhdr.range.low.y  = m_element.dhdr.range.low.z  = IMINI8;
//    m_element.dhdr.range.high.x = m_element.dhdr.range.high.y = m_element.dhdr.range.high.z = IMAXI8;
//
//    if (NULL == modelRef)
//        {
//        WStringCP       levelName;
//    
//        m_element.GetLevel() = LEVEL_DEFAULT_LEVEL_ID;
//    
//        if (NULL != (levelName = cache.GetLevelName (modelRef, proxyPiece.GetLevel())))
//            {
//            LevelHandle levelHandle = modelRef->GetLevelCache().GetLevelByName(levelName->c_str());
//    
//            if (levelHandle.IsValid ())
//                m_element.GetLevel() = levelHandle.GetLevelId();
//            }
//        }
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    Draw (ViewContextR context, ElementHandleCR el)
//    {
//    m_proxyPiece.Draw (context, ElementHandle (el.GetElementRef(), m_cache.GetRootModel()), m_qvIndex, m_cache, m_type);
//    }
//
//};  // ProxyGraphicsPieceElemRef
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void ProxyGraphicsPieceDisplayHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
//    {
//    DgnElementCP                 el = thisElm.GetElementCP();
//    ProxyGraphicsPieceElemRef*  proxyElemRef = (ProxyGraphicsPieceElemRef*) thisElm.GetElementRef();
//    ProxyGraphicsPieceR         proxyPiece = proxyElemRef->m_proxyPiece;
//    DgnModelP                symbologyDgnModel = proxyElemRef->m_cache.GetRootModel();
//
//    params.Init ();
//    setDisplayParamsLineColor (params, el->GetSymbology().color, symbologyDgnModel, thisElm.GetDgnModelP());
//    params.SetLineStyle (el->GetSymbology().style, proxyPiece.GetStyleParam ());       // CHECK...Is this the correct (attachment) modelRef??
//    params.SetLineStyleDgnModel (*thisElm.GetDgnModelP(), *symbologyDgnModel);
//    params.SetWeight (el->GetSymbology().weight);
//    //params.SetLevel (el->GetLevel());
//    params.SetLevelSymbIgnored (true);
//    params.SetElementClass ((DgnElementClass) el->GetElementClass());
//    params.SetLevelSymbIgnored (true);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      07/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyGraphicsPiece::VisitAsElement (ViewContextR context, ElementHandleCR el, UInt32 qvIndex, VisibleEdgeCacheImplR proxyCache, ProxyGraphicsType proxyType)
//    {
//    ProxyGraphicsPieceElemRef elemRef  = ProxyGraphicsPieceElemRef (*this, qvIndex,  proxyCache, proxyType, el.GetDgnModelP());
//
//    context.VisitElemHandle (ElementHandle (&elemRef, el.GetDgnModelP()), false, false);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Ray.Bentley     05/2009.
//+---------------+---------------+---------------+---------------+---------------+------*/
//void  ProxyGraphicsPieceDisplayHandler::_Draw (ElementHandleCR el, ViewContextR context)
//    {
//    ((ProxyGraphicsPieceElemRef*) el.GetElementRef())->Draw (context, el);
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void  ProxyElementDisplayHandler::_Draw (ElementHandleCR el, ViewContextR context)
    {
    ProxyElement proxy (s_fakeCache);
    if (RestoreProxyElement (proxy, el) != BSISUCCESS)
        {
        T_Super::_Draw (el, context);
        return;
        }

    ProxyElementDetails details;
    details.Load(el);

    if (context.GetViewport() != NULL)                                                                                          
        {                                                                                                                       
        auto hyper = dynamic_cast<HypermodelingViewController*> (&context.GetViewport()->GetViewControllerR());                  
        if (hyper != NULL)                                                                                                      
            {
            if (!hyper->ShouldDrawProxyGraphics (details.GetViewHandlerPass().GetPass(), details.GetViewHandlerPass().m_clipPlaneIndex))
                return;

            //hyper->SetOverrideMatSymb (context);
            }                                                                                                                   
        }                                                                                                                       

    ProxyGraphicsElemTopology topo;
    if (context.GetDrawPurpose() == DrawPurpose::Pick)
        {
        topo.m_targetElement = ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler).GetProxyTarget (el);
        topo.m_pass = details.GetViewHandlerPass().GetPass();
        topo.m_clipPlaneIndex = details.GetViewHandlerPass().m_clipPlaneIndex;
        context.SetElemTopology (&topo);
        }

    proxy.Draw (context, el);

    if (context.GetDrawPurpose() == DrawPurpose::Pick)
        context.SetElemTopology (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void   ProxyElementDisplayHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    DgnElementCP     el = thisElm.GetElementCP ();
    //ProxyElementR   proxyElement = ((ProxyElementElemRef*) thisElm.GetElementRef())->m_proxyElement;
    ProxyElement proxyElement (s_fakeCache); // added in graphite
    if (RestoreProxyElement (proxyElement, thisElm) != BSISUCCESS)// added in graphite
        return;
    //DgnModelP    symbologyDgnModel = proxyElement.GetSymbologyDgnModel ();
    DgnModelP    symbologyDgnModel = thisElm.GetDgnModelP(); // added in graphite

    params.Init ();
    setDisplayParamsLineColor (params, el->GetSymbology().color, symbologyDgnModel, thisElm.GetDgnModelP());
    params.SetLineStyle (el->GetSymbology().style, proxyElement.GetStyleParam ());
    params.SetWeight (el->GetSymbology().weight);
    params.SetLevelSubLevelId (LevelSubLevelId(LevelId(el->GetLevel())));
    params.SetLevelSymbIgnored (true);
    params.SetElementClass ((DgnElementClass) el->GetElementClass());
    params.SetLevelSymbIgnored (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static WString getProxyGraphicsDescription (ClipVolumePass pass)
    {
    DgnCoreL10N::Number msgid = DgnCoreL10N::IDS_SectionGraphicsOther;
    switch (pass)
        {
        case ClipVolumePass::Cut:            msgid = DgnCoreL10N::IDS_SectionGraphicsCut;        break;
        case ClipVolumePass::InsideForward:  msgid = DgnCoreL10N::IDS_SectionGraphicsForward;    break;
        case ClipVolumePass::InsideBackward: msgid = DgnCoreL10N::IDS_SectionGraphicsBackward;   break;
        case ClipVolumePass::Outside:        msgid = DgnCoreL10N::IDS_SectionGraphicsOutside;    break;
        }

    if (msgid != DgnCoreL10N::IDS_SectionGraphicsOther)
        return DgnCoreL10N::GetStringW (msgid);

    return L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2009.
+---------------+---------------+---------------+---------------+---------------+------*/
void  ProxyElementDisplayHandler::_GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength)
    {
    //ProxyElementElemRefP        transientElemRef = (ProxyElementElemRef*) el.GetElementRef();
    //ProxyElementR               proxyElement = transientElemRef->m_proxyElement;
    //
    //if (proxyElement.IsRoot())
    //    {
    //    int                 messageNumber = MSGID_OriginalElementUnavailable;
    //    DgnModelP           dgnModel;
    //    DgnModelP        modelRef = el.GetDgnModelP();
    //    
    //    if (NULL != (dgnModel = modelRef->GetDgnModelP()))
    //        {
    //        ElementRefP      cacheElemRef;
    //    
    //        if (NULL == (cacheElemRef = dgnModel->FindElementById (transientElemRef->GetElementId())) || cacheElemRef->IsDeletedAny())
    //            messageNumber = MSGID_OriginalElementHasBeenDeleted;
    //        else
    //            messageNumber = MSGID_OriginalElementHasBeenModified;
    //    
    //        }
    //    
    //    string.assign (WString (L" >>") + g_dgnHandlersResources->GetString (messageNumber) + WString (L" <<"));
    //    }
    //else
    //    {
    //    string = WString ();            // Only display once for outer elements.
    //    }

    ProxyElementDetails details;
    details.Load(el);
    WString desc = getProxyGraphicsDescription (details.GetViewHandlerPass().GetPass());

    ElementId targetElementId = ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler).GetProxyTarget (el);
    PersistentElementRefPtr targetRef = el.GetDgnProject()->Models().GetElementById (targetElementId);
    if (!targetRef.IsValid())
        string = desc;
    else
        {
        ElementHandle target (targetRef.get());
        target.GetHandler().GetDescription (target, string, desiredLength);
        string.append (L"\\").append (desc);
        }
    }


//ProxyElementDisplayHandler&         ProxyElementElemRef::s_handler       = ELEMENTHANDLER_INSTANCE (ProxyElementDisplayHandler);
//ProxyGraphicsPieceDisplayHandler&   ProxyGraphicsPieceElemRef::s_handler = ELEMENTHANDLER_INSTANCE (ProxyGraphicsPieceDisplayHandler);


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyElementMap
{
    T_ElemIdProxyMap        m_elementRefMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~ProxyElementMap ()
    {
    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
        delete curr->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//void    ClearElementModifiedTimes (DgnModelP dgnModel)
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->ClearElementModifiedTimes (dgnModel);
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void    Clone (ElementCopyContextP copyContext)
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->Clone (copyContext);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//void ClearConcealed ()
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->SetConcealed (false);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool ContainsConcealed () const
//    {
//    for (T_ElemIdProxyMap::const_iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        if (curr->second->ContainsConcealed())
//            return true;
//
//    return false;
//    }
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    GetConcealedPaths (T_ProxyDisplayPathVector& paths, ProxyDisplayPathR currPath) const
//    {
//    for (T_ElemIdProxyMap::const_iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->GetConcealedPaths (paths, currPath);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void   ResolveTransients (DgnModelP modelRef)
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->ResolveTransient (curr->first, modelRef);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementP   FindElement (ElemIdPathCR path, int index, TransformR transform) const
//    {
//    T_ElemIdProxyMap::const_iterator    proxyElement = m_elementRefMap.find (path[index]);
//
//    if (m_elementRefMap.end() == proxyElement)
//        return NULL;
//
//    if (++index == path.size())
//        {
//        return proxyElement->second;
//        }
//    else
//        {
//        if (NULL != proxyElement->second->GetTransform())
//            transform.productOf (&transform, proxyElement->second->GetTransform());
//
//        return proxyElement->second->FindElement (path, index, transform);
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool IsConcealed (DisplayPath const& displayPath, int index) const
//    {
//    ElementRefP                         elementRef = displayPath.GetPathElem (index);
//    T_ElemIdProxyMap::const_iterator    proxyElement = m_elementRefMap.find (elementRef->GetElementId());
//
//    if (m_elementRefMap.end() == proxyElement)
//        return false;
//
//    if (proxyElement->second->IsConcealed())
//        return true;
//
//    if (++index < displayPath.GetCount() && NULL != proxyElement->second->GetChildren())
//        return proxyElement->second->GetChildren()->IsConcealed (displayPath, index);
//
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementP   FindElement (DisplayPath const& displayPath, int index, TransformP transform = NULL) const
//    {
//    ElementRefP                         elementRef = displayPath.GetPathElem (index);
//    T_ElemIdProxyMap::const_iterator    proxyElement = m_elementRefMap.find (elementRef->GetElementId());
//
//    if (m_elementRefMap.end() == proxyElement)
//        return NULL;
//
//    if (++index == displayPath.GetCount())
//        {
//        return proxyElement->second;
//        }
//    else
//        {
//        if (NULL != transform && NULL != proxyElement->second->GetTransform())
//            transform->productOf (transform, proxyElement->second->GetTransform());
//
//        return proxyElement->second->FindElement (displayPath, index, transform);
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementP        FindOrCreateElement (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, DgnElementCR templateElement, int index)
//    {
//    ElementRefP                     elementRef = displayPath.GetPathElem (index);
//    ElementId                       elementId  = elementRef->GetElementId();
//    T_ElemIdProxyMap::iterator      found = m_elementRefMap.find (elementId);
//    ProxyElementP                   proxyElement;
//
//    if (m_elementRefMap.end() == found)
//        {
//        m_elementRefMap[elementId] = (proxyElement = new ProxyElement (cache, displayPath, index));
//        }
//    else
//        {
//        proxyElement = found->second;
//        }
//
//    return (++index == displayPath.GetCount())?  proxyElement : proxyElement->FindOrCreateElement (cache, displayPath, templateElement, index);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyGraphicsP   FindOrCreateGraphics (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, DgnElementCR templateElement)
//    {
//    return FindOrCreateElement (cache, displayPath, templateElement, 0)->GetOrCreateGraphics ();
//    }
//
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ElemIdPathCR path, ProxyEdgeIdDataCR edgeId, GPArrayParamCP edgeParam, TransformR transform) const
//    {
//    ProxyElementP    element;
//
//    return (NULL == (element = FindElement (path, 0, transform))) ? ERROR : element->GetProxyGraphicsFromEdgeId (edgeGraphics, edgeId, edgeParam);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Dump (size_t childLevel = 0)
    {
    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
        curr->second->Dump (curr->first, childLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    Save (ProxyDataBuffer& buffer) const
    {
    size_t      mapBTFLocation = buffer.Write ((UInt32) 0);                          // Map BTF

    buffer.Write ((UInt32) m_elementRefMap.size());
    for (T_ElemIdProxyMap::const_iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
        {
        buffer.Write (curr->first);
        curr->second->Save (buffer);
        }

    buffer.UpdateBytesToFollow (mapBTFLocation);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Restore (VisibleEdgeCacheImplR proxyCache, byte const*& dataP, byte const* dataEndP, DgnModelP dgnModel, DgnModelP modelRef)
    {
    UInt32      mapBytesToFollow;
    byte const*       nextMapData;

    if (SUCCESS != ProxyRestoreUtil::CopyData (mapBytesToFollow, dataP, dataEndP) ||
        (nextMapData = dataP + mapBytesToFollow) > dataEndP)
        return ERROR;

    UInt32                  elementCount;

    if (SUCCESS != ProxyRestoreUtil::CopyData (elementCount, dataP, nextMapData))
        return ERROR;

    for (UInt32 i=0; i<elementCount; i++)
        {
        /*ElementId*/UInt64               elementId;

        if (SUCCESS != ProxyRestoreUtil::CopyData (elementId, dataP, nextMapData))
            return ERROR;

        StatusInt       status;
        ProxyElementP   element = new ProxyElement (proxyCache);

        if (SUCCESS != (status = element->Restore (proxyCache, dataP, nextMapData, dgnModel, modelRef)))
            {
            delete element;
            return status;
            }

        //element->SetTransientElemRef (new ProxyElementElemRef (elementId, modelRef, *element));
        m_elementRefMap[elementId] = element;
        }


    return SUCCESS;

    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void        VisibleEdgeCalculationDraw (ViewContextR context, ElementHandleCR eh)
//    {
//    // This is only necessary for the (stupid) text node children.
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        curr->second->_VisibleEdgeCalculationDraw(context, eh);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void        Visit (ViewContextR context, DgnModelP modelRef)
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        {
//        ProxyElementElemRefP        transientElemRef;
//
//        if (NULL != (transientElemRef = curr->second->GetTransientElemRef()))
//            context.VisitElemHandle (ElementHandle (transientElemRef, modelRef), false, false);       // TR# 325685 - don't check range on children.
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//virtual void    DrawStatic (ViewContextR context, DgnModelR rootParentDgnModel, DgnModelR modelRef, UInt32 displayFlags)
//    {
//    for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//        if (!context.FilterRangeIntersection (ElementHandle (curr->second->GetTransientElemRef(), &modelRef)))
//            curr->second->DrawStatic (context, rootParentDgnModel, modelRef, displayFlags);
//    }

void ApplyTransform (TransformCR trans); // added in graphite

};  // ProxyElementMap



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void        ProxyElement::Clone (ElementCopyContextP copyContext)
//    {
//    if (NULL != m_children)
//        m_children->Clone (copyContext);
//
//    if (NULL != m_graphics)
//        m_graphics->Clone (copyContext);
//    }



///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyElement::ClearConcealed ()
//    {
//    SetConcealed (false);
//
//    if (NULL != m_children)
//        m_children->ClearConcealed ();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool ProxyElement::ContainsConcealed () const
//    {
//    return IsConcealed() || (NULL != m_children && m_children->ContainsConcealed());
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyElement::GetConcealedPaths (T_ProxyDisplayPathVector& paths, ProxyDisplayPathR currPath) const
//    {
//    currPath.m_path.push_back (m_transientElemRef->GetElementId());
//
//    if (IsConcealed())
//        paths.push_back (new ProxyDisplayPath (currPath));
//    else if (NULL != m_children)
//        m_children->GetConcealedPaths (paths, currPath);
//
//    currPath.m_path.pop_back ();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyElement::ResolveTransient (ElementId elementId, DgnModelP modelRef)
//    {
//    if (NULL != m_children)
//        m_children->ResolveTransients (modelRef);
//
//    m_transientElemRef = new ProxyElementElemRef (elementId, modelRef, *this);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyElement::ClearElementModifiedTimes (DgnModelP dgnModel)
//    {
//    ElementRefP      cacheElemRef;
//
//    if (NULL == dgnModel ||
//        NULL == m_transientElemRef ||
//        NULL == (cacheElemRef = dgnModel->FindByElementId(m_transientElemRef->GetElementId())) ||
//        InSynch (cacheElemRef))
//        m_lastModified = 0.0;
//
//    if (NULL != m_children)
//        m_children->ClearElementModifiedTimes (dgnModel);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProxyElement::Dump (/*ElementId*/UInt64 elementId, size_t childLevel)
    {
    for (size_t i=0; i<childLevel; i++)
        BeConsole::Printf ("    ");

    BeConsole::Printf ("Element - ID: %I64d\n", elementId);
    if (NULL != m_graphics)
        m_graphics->Dump();

    if (NULL != m_children)
        m_children->Dump (childLevel+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyElement::~ProxyElement ()
    {
    DELETE_AND_CLEAR (m_graphics);
    DELETE_AND_CLEAR (m_children);
    DELETE_AND_CLEAR (m_transform);
//    DELETE_AND_CLEAR (m_transientElemRef);
    s_proxyElements--;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementP   ProxyElement::FindElement (DisplayPath const& displayPath, int index, TransformP transform) const { return NULL == m_children ? NULL : m_children->FindElement (displayPath, index, transform); }
//ProxyElementP   ProxyElement::FindElement (ElemIdPathCR path, int index, TransformR transform) const { return NULL == m_children ? NULL : m_children->FindElement (path, index, transform); }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyElementP    ProxyElement::FindOrCreateElement (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, DgnElementCR templateElement, int index)
//    {
//    if (NULL == m_children)
//        m_children = new ProxyElementMap ();
//
//    return m_children->FindOrCreateElement (cache, displayPath, templateElement, index);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool    ProxyElement::IsVisible (DisplayPath const& displayPath, int index)
//    {
//    ProxyElementP        child;
//
//    if (NULL != m_children &&
//        NULL != (child = m_children->FindElement (displayPath, index)))
//        return child->IsVisible (displayPath, index+1);
//
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void ProxyElement::_VisibleEdgeCalculationDraw (ViewContextR context, ElementHandleCR eh) const
//    {
//    if (NULL != m_graphics)
//        m_graphics->Draw (eh, context, m_proxyCache, ProxySymbologyOverride_None);
//
//    // The stupid-assed text nodes dont expose their children, but visit them to display.  Ick.
//    if (NULL != m_children)
//        {
//        if (NULL != m_transform)
//            {
//            context.PushTransform (*m_transform);
//            m_children->VisibleEdgeCalculationDraw (context, eh);
//            context.PopTransformClip ();
//            }
//        else
//            {
//            m_children->VisibleEdgeCalculationDraw (context, eh);
//            }
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyElement::Draw0 (ViewContextR context, ElementHandleCR eh) const
    {
    if (NULL != m_graphics)
        m_graphics->Draw (eh, context, m_proxyCache, ProxySymbologyOverride_None);

    if (NULL != m_children) //  && NULL != m_transientElemRef)
        {
        if (NULL != m_transform)
            {
            context.PushTransform (*m_transform);
            //m_children->Visit (context, eh.GetDgnModelP());
            for (auto child : m_children->m_elementRefMap)
                child.second->Draw0 (context, eh);
                
            context.PopTransformClip ();
            }
        else
            {
            //m_children->Visit (context, eh.GetDgnModelP());
            for (auto child : m_children->m_elementRefMap)
                child.second->Draw0 (context, eh);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyElement::Draw (ViewContextR context, ElementHandleCR eh) const
    {
    Draw0 (context, eh);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool    ProxyElement::FlashHitEdge (ViewContextR context, ProxyEdgeIdDataCR edgeId, ElementHandleCR eh)
//    {
//    XGraphicsContainer    xGraphics;
//
//    if (SUCCESS != GetProxyGraphicsFromEdgeId (xGraphics, edgeId, NULL))
//        return false;
//
//    xGraphics.DrawProxy (context, eh, 0);
//    return true;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    ProxyElement::DrawStatic (ViewContextR context, DgnModelR rootParentDgnModel, DgnModelR modelRef, UInt32 displayFlags)
//    {
//    ProxySymbologyOverride symbologyOverride;
//
//    if (0 != (displayFlags & ProxyDisplayFlagsMask_DisplayConcealed))
//        {
//        if (IsConcealed())
//            displayFlags |= ProxyDisplayFlagsMask_ForceConcealed;
//
//        symbologyOverride = (0 != (displayFlags & ProxyDisplayFlagsMask_ForceConcealed)) ? ProxySymbologyOverride_Concealed : ProxySymbologyOverride_NotConcealed;
//        }
//    else
//        {
//        if (IsConcealed())
//            return;
//
//        symbologyOverride = ProxySymbologyOverride_None;
//        }
//
//    if (NULL == m_transientElemRef)
//        {
//        BeAssert (false);
//        return;
//        }
//
//    DgnModelP       dgnModel;
//    ElementRefP     cacheElemRef;
//
//    if (NULL != (dgnModel = &modelRef) &&
//        NULL != (cacheElemRef = dgnModel->FindElementById (m_transientElemRef->GetElementId())) &&
//        ! cacheElemRef->IsDeleted() /*&& InSynch (cacheElemRef)*/)
//        {
//        context.PushPath (cacheElemRef);
//        }
//    else
//        {
//        context.PushPath (m_transientElemRef);
//        }
//
//    if (NULL != m_graphics)
//        {
//        if (DrawPurpose::Plot != context.GetDrawPurpose())
//            context.CookElemDisplayParams (ElementHandle (m_transientElemRef, &modelRef));    // &rootParentDgnModel));
//
//        m_graphics->Draw (ElementHandle (m_transientElemRef, &modelRef), context, m_proxyCache, symbologyOverride);
//        }
//
//    if (NULL != m_children)
//        {
//        if (NULL != m_transform)
//            {
//            context.PushTransform (*m_transform);
//            m_children->DrawStatic (context, rootParentDgnModel, modelRef, displayFlags);
//            context.PopTransformClip ();
//            context.ValidateScanRange ();
//            }
//        else
//            {
//
//            m_children->DrawStatic (context, rootParentDgnModel, modelRef, displayFlags);
//            }
//        }
//    context.PopPath ();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    ProxyElement::Save (ProxyDataBuffer& buffer) const
    {
    size_t      elementBTFLocation = buffer.Write ((UInt32) 0);                             // Element BTF
    buffer.Write (m_flags);                                                                 // Flags.
    buffer.Write (m_lastModified);
    buffer.Write (m_level);

    size_t      transformBTFLocation = buffer.Write ((UInt32) 0);                           // transform BTF

    if (NULL != m_transform)
        buffer.Write (m_transform, sizeof (*m_transform));

    buffer.UpdateBytesToFollow (transformBTFLocation);

    size_t      graphicsBTFLocation = buffer.Write ((UInt32) 0);                            // Graphics BTF

    if (NULL != m_graphics)
        m_graphics->Save (buffer, m_level);

    buffer.UpdateBytesToFollow (graphicsBTFLocation);

    size_t      childrenBTFLocation = buffer.Write ((UInt32) 0);                            // Children BTF

    if (NULL != m_children)
        m_children->Save (buffer);

    buffer.UpdateBytesToFollow (childrenBTFLocation);
    buffer.UpdateBytesToFollow (elementBTFLocation);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyElement::Restore (VisibleEdgeCacheImplR proxyCache, byte const*& dataP, byte const* dataEndP, DgnModelP /*dgnModel*/, DgnModelP /*modelRef*/)
    {
    UInt32      elementBTF, graphicsBTF, childrenBTF, transformBTF;
    byte const*       nextElementP;

    if (SUCCESS != ProxyRestoreUtil::CopyData (elementBTF, dataP, dataEndP) ||
        (nextElementP = dataP + elementBTF) > dataEndP)
        return ERROR;

    ProxyRestoreUtil::CopyData (m_flags, dataP, nextElementP);
    ProxyRestoreUtil::CopyData (m_lastModified, dataP, nextElementP);
    ProxyRestoreUtil::CopyData (m_level, dataP, nextElementP);

    ProxyRestoreUtil::CopyData (transformBTF, dataP, nextElementP);
    if (transformBTF == sizeof (Transform))
        m_transform = new Transform(*((TransformCP) dataP));

    dataP += transformBTF;

    byte const*       childrenP;
    if (SUCCESS != ProxyRestoreUtil::CopyData (graphicsBTF, dataP, nextElementP) ||
        (childrenP = dataP + graphicsBTF) > nextElementP)
        return ERROR;

    if (0 != graphicsBTF)
        m_graphics = new ProxyGraphics (dataP, childrenP, m_level);

    if (SUCCESS != ProxyRestoreUtil::CopyData (childrenBTF, dataP, nextElementP) ||
        dataP + childrenBTF != nextElementP)
        return ERROR;

    if (0 == childrenBTF)
        return SUCCESS;

    m_children = new ProxyElementMap();

    return m_children->Restore (proxyCache, dataP, nextElementP, NULL,NULL);//dgnModel, modelRef);
    }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
//struct ProxyRangeTreeTraverser : DRTElementRefTraverser
//{
//    ViewContextR        m_viewContext;
//    ProxyElementMapR    m_elementMap;
//    DgnModelR        m_rootParentDgnModel;
//    DgnModelR        m_modelRef;
//    UInt32              m_displayFlags;
//
//    ProxyRangeTreeTraverser (ViewContextR context, ProxyElementMapR elementMap, DgnModelR rootParentDgnModel, DgnModelR modelRef, UInt32 displayFlags)
//                    : m_viewContext (context),
//                      m_elementMap (elementMap),
//                      m_rootParentDgnModel (rootParentDgnModel),
//                      m_modelRef (modelRef),
//                      m_displayFlags (displayFlags) { }
//
//    virtual bool _CheckRangeIndexNode (ScanRangeCR range, bool is3d) override { return !m_viewContext.CheckStop () && ScanTestResult::Pass == const_cast <ScanCriteriaP> (m_viewContext.GetScanCriteria())->CheckRange (range, is3d); }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//virtual bool _VisitElementRef (ElementRefP elementRef) override
//    {
//    ((ProxyElementElemRefP) elementRef)->m_proxyElement.DrawStatic (m_viewContext, m_rootParentDgnModel, m_modelRef, m_displayFlags);
//    return !m_viewContext.CheckStop();
//    }
//
//
//};  // ProxyRangeTreeTraverser

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct ProxyModelPass : ProxyElementMap
{
    DgnRangeTreeP       m_rangeTree;

    ProxyModelPass () : m_rangeTree (NULL)  { }
    ~ProxyModelPass ()                      { DELETE_AND_CLEAR (m_rangeTree); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void    LoadRangeTree ()
//    {
//    if (NULL == m_rangeTree)
//        {
//        m_rangeTree = new DgnRangeTree (true);
//
//        for (T_ElemIdProxyMap::iterator curr = m_elementRefMap.begin(), end = m_elementRefMap.end(); curr != end; curr++)
//            {
//            ProxyElementElemRefP  elemRef;
//
//            if (NULL != (elemRef = curr->second->GetTransientElemRef()))
//                m_rangeTree->AddElementRef (elemRef->GetUnstableMSElementCP()->hdr.dhdr.range, elemRef);
//            }
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual void    DrawStatic (ViewContextR context, DgnModelR rootParentDgnModel, DgnModelR modelRef, UInt32 displayFlags) override
//    {
//    ScanCriteriaP       scanCriteria;
//
//    LoadRangeTree();
//    context.ValidateScanRange ();
//
//    if (NULL == (scanCriteria = const_cast <ScanCriteriaP> (context.GetScanCriteria())) || !scanCriteria->GetScanType().testRange)
//        {
//        ProxyElementMap::DrawStatic (context, rootParentDgnModel, modelRef, displayFlags);
//        }
//    else
//        {
//        ProxyRangeTreeTraverser traverser (context, *this, rootParentDgnModel, modelRef, displayFlags);
//
//        m_rangeTree->TraverseElementRefs (traverser);
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   GetRange (DRange3dR range)
//    {
//    LoadRangeTree ();
//
//    ScanRangeCP     scanRange;
//    if (NULL == (scanRange = m_rangeTree->GetRange()))
//        return ERROR;
//
//    DataConvert::ScanRangeToDRange3d (range, *scanRange);
//    return SUCCESS;
//    }

};  // ProxyModelPass

typedef bmap <ViewHandlerPass, ProxyModelPassP> T_ProxyPassMap;
typedef bmap </*LevelId*/UInt32, WString> T_LevelNameMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void        reportHashValue (DsigRawHash const& hashValue, char* label)
//    {
//    byte const*           bP = &hashValue.b[0];
//
//    BeConsole::Printf ("Hash Value: %hs is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", label, *bP, *(bP+1), *(bP+2), *(bP+3), *(bP+4), *(bP+5), *(bP+6), *(bP+7));
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void        reportHashValue (CryptographerHash& hashCalculator, double stage)
//    {
//    DsigRawHash     parameterHash;
//    hashCalculator.save (parameterHash);
//    byte*           bP = &parameterHash.b[0];
//    BeConsole::Printf ("Hash stage %03.2f is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", stage, *bP, *(bP+1), *(bP+2), *(bP+3), *(bP+4), *(bP+5), *(bP+6), *(bP+7));
//    }
//
//#if defined (DEBUG_HASH)
//#define REPORT_HASH_VALUE(a,b) reportHashValue(a,b)
//#else
//#define REPORT_HASH_VALUE(a,b)
//#endif


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   11/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//static double   roundDouble (double input)
//    {
//    // round to the nearest 100th of a UOR.
//    return floor ((input*100.0) + .5) / 100.0;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   11/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//static void            hash3dClipPoint (DPoint3dCR point, DPoint3dCR origin, CryptographerHash& refHashCalculator)
//    {
//    DPoint3d    val;
//    val.x = roundDouble (point.x - origin.x);
//    val.y = roundDouble (point.y - origin.y);
//    val.z = roundDouble (point.z - origin.z);
//    refHashCalculator.hash ((const byte*)&val, sizeof (val));
//    }
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   11/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//void            hash2dClipPoint (DPoint2dCR point, DPoint3dCR origin, CryptographerHash& refHashCalculator)
//    {
//    DPoint2d    val;
//    val.x = roundDouble (point.x - origin.x);
//    val.y = roundDouble (point.y - origin.y);
//    refHashCalculator.hash ((const byte*)&val, sizeof (val));
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   11/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//void            addClipElementToHash (ElementHandleCR clipEH, CryptographerHash& refHashCalculator, DPoint3dCR refOrigin)
//    {
//    DgnElementCP     element = clipEH.GetElementCP();
//
//    // NOTE: We can't use the lastModified time if it's a type 106 or a shape, because those are
//    //       saved view subclasses that generate clipping, and they get rewritten a lot.
//    if (SHAPE_ELM == element->GetLegacyType())
//        {
//        if (element->Is3d())
//            {
//            refHashCalculator.hash ((const byte*)&element->ToLine_String_3d().numverts, sizeof (element->ToLine_String_3d().numverts));
//            for (DPoint3dCP thisPoint = &element->ToLine_String_3d().vertice[0], endPoint = &element->ToLine_String_3d().vertice[element->ToLine_String_3d().numverts]; thisPoint < endPoint; thisPoint++)
//                hash3dClipPoint (*thisPoint, refOrigin, refHashCalculator);
//            }
//        else
//            {
//            refHashCalculator.hash ((const byte*)&element->ToLine_String_3d().numverts, sizeof (element->ToLine_String_3d().numverts));
//            for (DPoint2dCP thisPoint = &element->ToLine_String_2d().vertice[0], endPoint = &element->ToLine_String_2d().vertice[element->ToLine_String_2d().numverts]; thisPoint < endPoint; thisPoint++)
//                hash2dClipPoint (*thisPoint, refOrigin, refHashCalculator);
//            }
//        }
//    else if (EXTENDED_ELM == element->GetLegacyType())
//        {
//        ElementHandle::XAttributeIter xAttributeIter (clipEH);
//
//        while (xAttributeIter.IsValid())
//            {
//            UInt32  xAttrSize = xAttributeIter.GetSize();
//            if (0 < xAttrSize)
//                refHashCalculator.hash ((const byte*) xAttributeIter.PeekData(), (size_t) xAttrSize);
//
//            xAttributeIter.ToNext();
//            }
//        }
//    else
//        {
//        refHashCalculator.hash ((const byte*) &element->ehdr.lastModified, sizeof (element->ehdr.lastModified));
//        }
//    }
//
//void            hashRotMatrix (RotMatrixCR transform, CryptographerHash& refHashCalculator);
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   12/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//static void     hashDisplayStyle (bool& fromView, CryptographerHash& refHashCalculator, Int32 styleIndex, DgnAttachmentR ref, bool fromParent, double hashStage)
//    {
//    if (-1 == styleIndex)
//        {
//        fromView = true;
//        return;
//        }
//
//    DgnModelP    sourceModel;
//    if (NULL == (sourceModel = fromParent ? ref.GetParentDgnModelP() : &ref))
//        return;
//
//    DgnFileP        sourceFile;
//    if (NULL == (sourceFile = sourceModel->GetDgnFileP()))
//        return;
//
//    DisplayStyleCP  displayStyle;
//    if (NULL == (displayStyle = DisplayStyleManager::GetDisplayStyleByIndex (styleIndex, *sourceFile)))
//        return;
//
//    displayStyle->HashDisplayParameters (refHashCalculator);
//    REPORT_HASH_VALUE (refHashCalculator, (hashStage + .02));
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   09/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//static void     hashClipVolumeOverride (bool& fromView, ClipVolumeOverridesCR thisOverrides, CryptographerHash& refHashCalculator, DgnAttachmentR ref, bool fromParent, double hashStage)
//    {
//    // strip out the bits that don't matter to the display of visible edge caches.
//    ClipVolumeOverrides     partial = thisOverrides;
//    partial.m_flags.m_disableLocate = 0;
//    partial.m_flags.m_disableSnap   = 0;
//
//    refHashCalculator.hash ((const byte *)&partial, sizeof(partial));
//
//    REPORT_HASH_VALUE (refHashCalculator, (hashStage + .01));
//
//    // if this clipVolumOverride indicates that it is displayed, hash the actual display style too.
//    if (thisOverrides.m_flags.m_display)
//        hashDisplayStyle (fromView, refHashCalculator, thisOverrides.GetDisplayStyleIndex(), ref, fromParent, hashStage);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   08/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//void            attachmentParameterHashFromReference (DsigRawHash& parameterHash, DgnAttachmentR ref)
//    {
//    CryptographerHash   refHashCalculator;
//    refHashCalculator.init (*getCryptographer(), DIGITAL_SIGNATURE_ALG_RSA_MD5);
//
//
//    double      scale = ref.GetDisplayScale();
//    refHashCalculator.hash ((const byte *)&scale, sizeof (scale));
//
//    REPORT_HASH_VALUE (refHashCalculator, 1.0);
//    hashRotMatrix (ref.GetRotMatrixCR(), refHashCalculator);
//    REPORT_HASH_VALUE (refHashCalculator, 2.0);
//    refHashCalculator.hash ((const byte *)&ref.GetCameraPosition(), sizeof (DPoint3d));
//    REPORT_HASH_VALUE (refHashCalculator, 3.0);
//
//    double      cameraFocalLength = ref.GetCameraFocalLength();
//    refHashCalculator.hash ((const byte *) &cameraFocalLength, sizeof (cameraFocalLength));
//
//    byte    useAnnotationScale = ref.UseAnnotationScale() ? 1 : 0;
//    refHashCalculator.hash (&useAnnotationScale, (size_t) 1);
//
//    REPORT_HASH_VALUE (refHashCalculator, 4.0);
//
//    DPoint2dCP  clipPoints;
//    int         numClipPoints = ref.GetClipPointsCP (clipPoints);
//
//    // if there's a clip element, hash the elementId and the last modified time of the element itself.
//    ElementId                   clipElementId;
//    if (0 != (clipElementId = ref.GetClipElementId()))
//        {
//        refHashCalculator.hash ((const byte *)&clipElementId, sizeof(ElementId));
//        DgnModelP    parentDgnModel;
//        ElementRefP     clipElemRef;
//
//        if ((NULL != (parentDgnModel = ref.GetParentDgnModelP())) &&
//             (NULL != (clipElemRef = parentDgnModel->GetDgnModelP()->FindElementByID (clipElementId))) )
//            {
//            addClipElementToHash (ElementHandle (clipElemRef, parentDgnModel), refHashCalculator, ref.GetStoredMasterOrigin());
//
//            // make sure we don't hash in both an clip elementId and clip points
//            numClipPoints = 0;
//            }
//        }
//
//    REPORT_HASH_VALUE (refHashCalculator, 5.0);
//
//    double  clipFront = ref.HasFrontClip() ? ref.GetStoredFrontClipDepth()  : 0.0;
//    double  clipBack  = ref.HasBackClip() ?  ref.GetStoredBackClipDepth()   : 0.0;
//    refHashCalculator.hash ((const byte *)&clipFront, sizeof(clipFront));
//    refHashCalculator.hash ((const byte *)&clipBack, sizeof(clipBack));
//    REPORT_HASH_VALUE (refHashCalculator, 5.1);
//
//    // NOTE: This checks for identity within a tolerance, so we don't conclude there's a difference based on "fuzz" in our clip rotation.
//    RotMatrixCR clipRotation = ref.GetClipRotMatrix();
//    if (!clipRotation.isIdentity())
//        refHashCalculator.hash ((const byte *)&clipRotation, sizeof(RotMatrix));
//
//    REPORT_HASH_VALUE (refHashCalculator, 5.2);
//
//
//    // We ignore masks during caching so that it is possible to mask/unmask without having regenerating.
//    // Therefore we'll only has the outer clip vertices (which are designated by all vertices prior
//    // to a disconnect.
//    int         outerClipCount;
//    for (outerClipCount = 0; outerClipCount < numClipPoints && clipPoints[outerClipCount].x != DISCONNECT; outerClipCount++)
//        ;
//
//    refHashCalculator.hash ((const byte *)&outerClipCount, sizeof(outerClipCount));
//
//    REPORT_HASH_VALUE (refHashCalculator, 6.0);
//
//    if (0 != outerClipCount)
//        refHashCalculator.hash ((const byte *)clipPoints, outerClipCount * sizeof(DPoint2d));
//
//    REPORT_HASH_VALUE (refHashCalculator, 7.0);
//
//    // Hash all of the stuff that makes a difference in the DynamicViewSettings and save that to the AttachParameters.
//    DynamicViewSettingsCR   dvSettings = ref.GetDynamicViewSettingsCR();
//
//    refHashCalculator.hash ((const byte *)&dvSettings.m_flags, sizeof(dvSettings.m_flags));
//    REPORT_HASH_VALUE (refHashCalculator, 7.1);
//
//    bool                    fromView   = false;
//    bool                    fromParent = dvSettings.GetFromParent();
//    hashClipVolumeOverride (fromView, dvSettings.m_forward, refHashCalculator, ref, fromParent, 7.1);
//    REPORT_HASH_VALUE (refHashCalculator, 7.2);
//
//    hashClipVolumeOverride (fromView, dvSettings.m_backward, refHashCalculator, ref, fromParent, 7.2);
//    REPORT_HASH_VALUE (refHashCalculator, 7.3);
//
//    hashClipVolumeOverride (fromView, dvSettings.m_cut, refHashCalculator, ref, fromParent, 7.3);
//    REPORT_HASH_VALUE (refHashCalculator, 7.4);
//
//    hashClipVolumeOverride (fromView, dvSettings.m_outside, refHashCalculator, ref, fromParent, 7.4);
//    REPORT_HASH_VALUE (refHashCalculator, 7.5);
//
//    refHashCalculator.hash ((const byte *)&fromParent, sizeof(fromParent));
//    REPORT_HASH_VALUE (refHashCalculator, 8.0);
//
//    ElementId               elementId;
//    EditElementHandle       elemHandle;
//
//    if (0 != (elementId = dvSettings.GetClipBoundElementId()))
//        {
//        if (SUCCESS == dvSettings.GetClipBoundElemHandle (elemHandle, &ref))
//            addClipElementToHash (elemHandle, refHashCalculator, ref.GetStoredMasterOrigin());
//        }
//    else
//        {
//        fromView = true;
//        }
//    refHashCalculator.hash ((const byte *)&elementId, sizeof(elementId));
//    REPORT_HASH_VALUE (refHashCalculator, 9.0);
//
//    if (0 != (elementId = dvSettings.GetClipMaskElementId()))
//        {
//        if (SUCCESS == dvSettings.GetClipMaskElemHandle (elemHandle, &ref))
//            addClipElementToHash (elemHandle, refHashCalculator, ref.GetStoredMasterOrigin());
//        }
//    refHashCalculator.hash ((const byte *)&elementId, sizeof(elementId));
//    REPORT_HASH_VALUE (refHashCalculator, 10.0);
//
//    // is the fromView displayStyleIndex used? (It is if any of the displayed overrides have -1, or if there's no ClipBoundElementId).
//    if (fromView)
//        {
//        Int32                   displayStyleIndex = dvSettings.GetDisplayStyleIndex();
//        refHashCalculator.hash ((const byte *)&displayStyleIndex, sizeof(displayStyleIndex));
//        hashDisplayStyle (fromView, refHashCalculator, displayStyleIndex, ref, fromParent, 10.0);
//        }
//
//    XAttributeHandlerId viewHandlerId = dvSettings.GetViewHandlerId();
//    refHashCalculator.hash ((const byte *)&viewHandlerId, sizeof(viewHandlerId));
//
//    REPORT_HASH_VALUE (refHashCalculator, 11.0);
//
//    XAttributesHolder       xAttrHolder;
//
//    if (SUCCESS == ref.ExtractProxyHashXAttributes (xAttrHolder))
//        {
//        XAttributeChangeSetP    changeSet   = xAttrHolder.QueryXAttributeChangeSet();
//        double                  hashStage = 0.01;
//        if (NULL != changeSet)
//            {
//            for (XAttributeChangeSet::T_ConstIterator iterator = changeSet->Begin (); iterator != changeSet->End(); iterator++)
//                {
//                if (XAttributeChange::CHANGETYPE_Write == iterator->GetChangeType())
//                    {
//#if defined (DEBUG_HASH)
//                    BeConsole::Printf ("Handler Id: %x, Id: %x, Size: %d\n", iterator->GetHandlerId().GetId(), iterator->GetId(), iterator->GetSize());
//#endif
//                    refHashCalculator.hash ((const byte*)iterator->PeekData(), (size_t) iterator->GetSize());
//
//                    REPORT_HASH_VALUE (refHashCalculator, 11.0 + hashStage);
//                    hashStage += .01;
//                    }
//                }
//            }
//        }
//
//    REPORT_HASH_VALUE (refHashCalculator, 12.0);
//
//#if defined (DEBUG_HASH)
//    BeConsole::Printf ("\n");
//#endif
//
//    refHashCalculator.save (parameterHash);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   08/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//static int      CompareHash (const DsigRawHash& hash1, const DsigRawHash& hash2)
//    {
//    if (hash1.algid != hash2.algid)
//        return -1;
//    if (hash1.len   != hash2.len)
//        return -1;
//    if (hash1.len > DIGITAL_SIGNATURE_MAX_RAW_HASH)
//        return -1;
//
//    for (UInt32 iHashByte=0; iHashByte < hash1.len; iHashByte++)
//        {
//        if (hash1.b[iHashByte] != hash2.b[iHashByte])
//            return -1;
//        }
//    return 0;
//    }
//

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
//struct ProxyDisplayHitInfo : IViewHandlerHitInfo
//{
//    ViewportP                           m_viewport;
//    DynamicViewStateStack               m_stateStack;
//    ViewHandlerPass                     m_viewHandlerPass;
//    ProxyGraphicsType                   m_graphicsType;
//    DgnAttachmentP                      m_proxyRootRef;
//
//                                        ProxyDisplayHitInfo ();
//                                        ProxyDisplayHitInfo (ProxyDisplayHitInfo const& rhs);
//
//    virtual                             ~ProxyDisplayHitInfo ();
//
//    virtual bool                        _IsSnappable (HitPathCR) const override { return true; }
//    virtual DPlane3dCP                  _GetCuttingPlane () const override      { return NULL; }
//    virtual ElementHandleCP             _GetClipElement () const override       { return NULL; }
//    virtual CutPlaneTag                 _GetCutPlaneTag () const override       { return CutPlaneTag(); }
//
//    virtual void                        _OnDrawCutPlane (ViewContextP, CutPlaneTag const&) override {}
//    virtual void                        _OnDrawCutGraphics (ElementHandleCR solid, ICutPlaneR, XGraphicsContainerCR cutGraphics) override {}
//    virtual SnapStatus                  _OnSnap (SnapContextR context) const override;
//    virtual StatusInt                   _OnCreateAssociationToSnap (HitPathR, DgnModelP) const override;
//    virtual IViewHandlerHitInfo*        _Clone() const override {return new ProxyDisplayHitInfo (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//void    Init (ViewContextR context, VisibleEdgeCacheImplCR proxyCache, ProxyGraphicsType graphicsType)
//    {
//    m_viewport = context.GetViewport();
//    m_proxyRootRef = proxyCache.GetRootRefFile();
//    m_graphicsType = graphicsType;
//    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//virtual void _OnGetInfoString (WStringR pathDescr, DisplayPath const& displayPath, WCharCP delimiter) const override
//    {
//    WString         prefix;
//
//    int         msgid = -1;
//
//    switch (m_viewHandlerPass.m_pass)
//        {
//        case ClipVolumePass::Cut:                         msgid = MSGID_SectionGraphicsCut;        break;
//        case ClipVolumePass::InsideForward:               msgid = MSGID_SectionGraphicsForward;    break;
//        case ClipVolumePass::InsideBackward:              msgid = MSGID_SectionGraphicsBackward;   break;
//        case ClipVolumePass::Outside:                     msgid = MSGID_SectionGraphicsOutside;    break;
//        case ViewHandlerPass_Underlay:              break;
//        }
//
//    if (msgid > 0)
//        prefix.append (g_dgnHandlersResources->GetString (msgid));
//
//    msgid = -1;
//    switch (m_graphicsType)
//        {
//        case ProxyGraphicsType_Cut:                         msgid = MSGID_CachedCutGeometry;    break;
//        case ProxyGraphicsType_CutFill:                     msgid = MSGID_CachedCutGeometry;    break;
//        case ProxyGraphicsType_VisibleEdge:                 msgid = MSGID_CachedVisibleEdge;    break;
//        case ProxyGraphicsType_HiddenEdge:                  msgid = MSGID_CachedHiddenEdge;     break;
//        case ProxyGraphicsType_VisibleWire:                 msgid = MSGID_CachedVisibleWire;    break;
//        case ProxyGraphicsType_HiddenWire:                  msgid = MSGID_CachedHiddenWire;     break;
//        case ProxyGraphicsType_PassthroughAnnotation:       msgid = MSGID_CachedAnnotation;     break;
//        case ProxyGraphicsType_PassthroughUnderlay:         msgid = MSGID_CachedUnderlay;       break;
//        }
//
//    if (msgid > 0)
//        prefix = prefix + WString (L" (") +  g_dgnHandlersResources->GetString (msgid) + WString (L")");
//
//
//    pathDescr.insert (0, delimiter);
//    pathDescr.insert (0, prefix);
//    }
//};  // ProxyDisplayHitInfo


struct V8ViewFlags
    {
    UInt32      deprecated1:1;
    UInt32      fast_text:1;                //!< Shows or hides text elements. Note the inversion (e.g. "fast" text means don't show text elements).
    UInt32      deprecated2:1;
    UInt32      line_wghts:1;               //!< Controls whether line weights are used (e.g. control whether elements with non-zero line weights draw normally, or as weight 0).
    UInt32      patterns:1;                 //!< Shows or hides pattern elements.
    UInt32      text_nodes:1;               //!< Shows or hides text node numbers and origins. These are decorations that can be shown to identify all text node elements.
    UInt32      ed_fields:1;                //!< Shows or hides the underlines that denote a text enter data field.
    UInt32      on_off:1;                   //!< Current open/closed state of view.
    UInt32      deprecated3:1;
    UInt32      grid:1;                     //!< Shows or hides the grid. The grid settings are a design file setting.
    UInt32      lev_symb:1;                 //!< Controls whether level overrides are used (e.g. use the element level's symbology vs. the element's symbology).
    UInt32      deprecated4:1;
    UInt32      constructs:1;               //!< Shows or hides elements that are in the construction class (controlled on a per-element basis).
    UInt32      dimens:1;                   //!< Shows or hides dimension elements.
    UInt32      fast_cell:1;                //!< Controls whether cells display as a bounding box instead of showing their actual content.
    UInt32      def:1;                      //!< Whether viewController is defined (i.e. view definition is valid).
    UInt32      fill:1;                     //!< Controls whether the fills on filled elements are displayed.
    UInt32      deprecated5:1;
    UInt32      auxDisplay:1;               //!< Shows or hides the ACS triad.
    UInt32      deprecated6:1;
    UInt32      deprecated7:1;
    UInt32      reserved:1;                 //!< For future use
    UInt32      camera:1;                   //!< Controls whether camera settings are applied to the view's frustum.
    UInt32      renderMode:6;               //!< Controls the render mode of the view; see the MSRenderMode enumeration. This is typically controlled through a display style.
    UInt32      background:1;               //!< Shows or hides the background image. The image is a design file setting, and may be undefined.
    UInt32      refBoundaryDisplay:1;       //!< Shows or hides the boundaries of reference clips and clip volumes.
    UInt32      deprecated8:1;
    UInt32      deprecated9:1;
    UInt32      deprecated10:1;
    UInt32      deprecated11:1;
    UInt32      deprecated12:1;
    UInt32      textureMaps:1;              //!< Controls whether to display texture maps for material assignments.
    UInt32      deprecated13:1;
    UInt32      transparency:1;             //!< Controls whether element transparency is used (e.g. control whether elements with transparency draw normally, or as opaque).
    UInt32      deprecated14:1;
    UInt32      inhibitLineStyles:1;        //!< Controls whether custom line styles are used (e.g. control whether elements with custom line styles draw normally, or as solid lines). Note the inversion.
    UInt32      deprecated15:1;
    UInt32      patternDynamics:1;          //!< Controls whether associative patthern display in dynamics (performance optimization)
    UInt32      deprecated16:1;
    UInt32      tagsOff:1;                  //!< Shows or hides tag elements. Note the inversion.
    UInt32      renderDisplayEdges:1;       //!< Shows or hides visible edges in the shaded render mode. This is typically controlled through a display style.
    UInt32      renderDisplayHidden:1;      //!< Shows or hides hidden edges in the shaded render mode. This is typically controlled through a display style.
    UInt32      isNamed_deprecated:1;       //!< Ignored post-V8i
    UInt32      deprecated17:1;
    UInt32      overrideBackground:1;       //!< Controls whether the view's custom background color is used. This is typically controlled through a display style.
    UInt32      noFrontClip:1;              //!< Controls whether the front clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    UInt32      noBackClip:1;               //!< Controls whether the back clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    UInt32      noClipVolume:1;             //!< Controls whether the clip volume is applied. Note the inversion. Elements beyond will not be displayed.
    UInt32      useDisplaySet:1;            //!< Controls whether the active display set is used (e.g. limiting the display of elements to those in the display set).
    UInt32      associativeClip:1;          //!< Controls whether the clip volume, if associated to an element should automatically update if/when the clip element is modified.
    UInt32      minimized:1;                //!< Current minimized state of view.
    UInt32      maximized:1;                //!< Current maximized state of view.
    UInt32      renderDisplayShadows:1;     //!< Shows or hides shadows. This is typically controlled through a display style.
    UInt32      reserved2:1;                //!< For future use
    UInt32      hiddenLineStyle:3;          //!< Controls the line style (only line codes 0-7 are allowed) of hidden lines in the shaded render mode. This is typically controlled through a display style.
    UInt32      inhibitRenderMaterials:1;   //!< Controls whether element materials are used (e.g. control whether elements with materials draw normally, or as if they have no material).
    UInt32      ignoreSceneLights:1;        //!< Controls whether the custom scene lights or the default lighting scheme are used. Note the inversion.
    UInt32      reserved3:32;               //!< For future use
    };

// this is taken from 08.11.09. The members are directly in DgnAttachment in later versions.
struct  RefDisplayParameters
{
/*Fb_opts*/UInt32                 fb_opts;
/*Fd_opts*/UInt32                 fd_opts;
V8ViewFlags               disp_flags[8];
DPoint3d                mast_org;
DPoint3d                ref_org;
double                  storedScale;
double                  scale;
RotMatrix               trns_mtrx;
UShort                  nestDepth;
UShort                  levelDisplayFlag;
/*RefDisplayFlags*/UInt32         flags;
DPoint3d                cameraPosition;
double                  cameraFocalLength;
Int32                   priority;
LevelId                 boundaryLevel;
Symbology               boundarySymbology;
/*ElementId*/UInt64               colorTableId_deprecated;
Int16                   hsvValueAdjustment;
Int16                   hsvSaturationAdjustment;
Int16                   baseNestDepth;
Int16                   hsvHueSetting;
Int16                   hsvAdjustFlags;


Int16                   reserved1;
UInt32                  reserved2;
//
//void    FromAttachment (DgnAttachmentCR attachment)
//    {
//    fb_opts                 = attachment.m_fbOpts;
//    fd_opts                 = attachment.m_fdOpts;
//    memcpy (disp_flags, attachment.m_viewFlags, sizeof (disp_flags));
//    mast_org                = attachment.m_masterOrigin;
//    ref_org                 = attachment.m_refOrigin;
//    storedScale             = attachment.m_storedScale;
//    scale                   = attachment.m_calculatedScale;
//    trns_mtrx               = attachment.m_rotationMatrix;
//    nestDepth               = attachment.m_nestDepth;
//    levelDisplayFlag        = 0;
//    flags                   = attachment.m_displayFlags;
//    cameraPosition          = attachment.m_cameraPosition;
//    cameraFocalLength       = attachment.m_cameraFocalLength;
//    priority                = attachment.m_displayPriority;
//    boundaryLevel           = attachment.m_boundaryLevel;
//    boundarySymbology       = attachment.m_boundarySymbology;
//    colorTableId_deprecated = 0;
//    hsvValueAdjustment      = attachment.m_hsvValueAdjustment;
//    hsvSaturationAdjustment = attachment.m_hsvSaturationAdjustment;
//    baseNestDepth           = attachment.m_baseNestDepth;
//    hsvHueSetting           = attachment.m_hsvHueSetting;
//    hsvAdjustFlags          = attachment.m_hsvAdjustFlags;
//    reserved1               = 0;
//    reserved2               = 0;
//    }
//
//void    ToAttachment (DgnAttachmentR attachment) const
//    {
//    attachment.m_fbOpts                     = fb_opts;
//    attachment.m_fdOpts                     = fd_opts;
//    memcpy (attachment.m_viewFlags, disp_flags, sizeof (attachment.m_viewFlags));
//    attachment.m_masterOrigin               = mast_org;
//    attachment.m_refOrigin                  = ref_org;
//    attachment.m_storedScale                = storedScale;
//    attachment.m_calculatedScale            = scale;
//    attachment.m_rotationMatrix             = trns_mtrx;
//    attachment.m_nestDepth                  = nestDepth;
//    attachment.m_displayFlags               = flags;
//    attachment.m_cameraPosition             = cameraPosition;
//    attachment.m_cameraFocalLength          = cameraFocalLength;
//    attachment.m_displayPriority            = priority;
//    attachment.m_boundaryLevel              = boundaryLevel;
//    attachment.m_boundarySymbology          = boundarySymbology;
//    attachment.m_hsvValueAdjustment         = hsvValueAdjustment;
//    attachment.m_hsvSaturationAdjustment    = hsvSaturationAdjustment;
//    attachment.m_baseNestDepth              = baseNestDepth;
//    attachment.m_hsvHueSetting              = hsvHueSetting;
//    attachment.m_hsvAdjustFlags             = hsvAdjustFlags;
//    }
};

//typedef RefCountedPtr <ProxyLevelCache>     ProxyLevelCachePtr;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2011
+===============+===============+===============+===============+===============+======*/
struct  ProxyDisplayModel
{
    //DgnModelP                    m_modelRef;
    T_ProxyPassMap                  m_passMap;
    /*ElementId*/UInt64                       m_attachmentId;
    Transform                       m_transformToParent;
    WString                         m_logicalName;
    WString                         m_fileName;
    //RefDisplayParameters            m_refDisplay;
    //UInt32                          m_synchChangeCount;
    //double                          m_synchSaveTime;
    ProxyDisplayModelP              m_parent;
    T_ProxyModelVector              m_children;
    T_LevelNameMap                  m_levelNames;
    //ModelInfoPtr                    m_modelInfo;
    //ProxyLevelCachePtr              m_proxyLevelCache;
    //DsigRawHash                     m_refParameterHash;
    bool                            m_dirty;
    mutable BitMaskP                m_levelUsage;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
*
*   Create ProxyDisplayModel from saved data.
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyDisplayModel
(
/*ElementId*/UInt64               attachmentId,
TransformCR             transformToParent,
WStringCR               logicalName,
WStringCR               fileName,
RefDisplayParameters&   refDisplay,
DgnModelP            rootModel,
ProxyDisplayModelP      parent,
/*ModelInfoPtr*/void*            modelInfo,
DsigRawHash&            parameterHash
) : m_dirty (false)
    {
    Init ();
    BeAssert (0 != attachmentId);
    m_attachmentId          = attachmentId;
    m_transformToParent     = transformToParent;
    m_logicalName           = logicalName;
    m_fileName              = fileName;
    m_parent                = parent;
    //m_modelRef              = (NULL == parent) ? rootModel : ProxyRestoreUtil::FindModelChild (parent->m_modelRef, attachmentId);
    //m_refDisplay            = refDisplay;
    //m_modelInfo             = modelInfo;
    //m_refParameterHash      = parameterHash;

    //if (NULL == m_modelRef)
    //    {
    //    DgnAttachmentP              newAttachment;
    //    DgnDocumentMonikerPtr       moniker = DgnDocumentMoniker::CreateFromFileName (fileName.c_str());
    //    DgnFileP                    rootParentFile = rootModel->GetParentDgnModelP()->GetDgnFileP();
    //
    //    BeAssert (NULL != rootParentFile);
    //
    //    parent->m_modelRef->CreateDgnAttachment (newAttachment, *moniker, NULL, false);
    //
    //    refDisplay.ToAttachment (*newAttachment);
    //
    //
    //    m_modelRef = newAttachment;
    //    newAttachment->SetElementId (attachmentId);
    //    newAttachment->SetProxySourceFile (rootParentFile);
    //    newAttachment->SetLogicalName (logicalName.c_str());
    //    m_proxyLevelCache = ProxyLevelCache::Create (*this, *rootParentFile);
    //    }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//*
//*   Create ProxyDisplayModel from current modelRef (during cache creation).
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayModel (DgnModelP modelRef, ProxyDisplayModelP parent) : m_dirty (true)
//    {
//    Init ();
//
//    DgnAttachmentP  refFile;
//    
//    if (NULL == (refFile = modelRef->AsDgnAttachmentP()))
//        {
//        BeAssert(false);
//        return;
//        }
//    refFile->GetTransformToParent (m_transformToParent, true);
//    refFile->GetLogicalString (m_logicalName);
//    
//    if (NULL != refFile->GetDgnFileP())
//        m_fileName = refFile->GetDgnFileP()->GetFileName();
//
//    m_modelRef     = modelRef;
//    m_attachmentId = refFile->GetElementId();
//    m_parent       = parent;
//    m_refDisplay.FromAttachment (*refFile);
//    m_modelInfo =  (NULL == modelRef->GetDgnModelP()) ? NULL :  modelRef->GetDgnModelP()->GetModelInfo().MakeCopy();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    Init ()
    {
    //m_modelRef = NULL;
    m_parent = NULL;
    //m_synchChangeCount = 0xffffffff;
    //m_synchSaveTime    = 0.0;
    m_transformToParent.initIdentity();
    m_levelUsage = NULL;
    //memset ((void*) &m_refParameterHash, 0, sizeof (m_refParameterHash));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
~ProxyDisplayModel()
    {
#ifdef WIP_NEEDS_WORK
    // since we may have created transient element refs within the proxyModelPass, we need to
    // get rid of any elementRefs that select may be holding on to.
    locateElm_clearFrom (m_modelRef);
    if (select_containsAnyFrom (m_modelRef))
        select_clearAll ();
#endif

    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
        delete curr->second;

    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
        delete *curr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayModelCP     FindChild (ElemIdPathCR  path, size_t index, TransformR transform) const
//    {
//    if (0 == index)
//        return this;
//
//    for (T_ProxyModelVector::const_iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        if ((*curr)->m_attachmentId == path[index-1])
//            {
//            transform.productOf (&(*curr)->m_transformToParent, &transform);
//            return (*curr)->FindChild (path, index-1, transform);
//            }
//
//    return NULL;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//WStringCP   GetLevelName (LevelId LevelId) const
//    {
//    T_LevelNameMap::const_iterator  found = m_levelNames.find (LevelId);
//
//    return m_levelNames.end() == found ? NULL : &found->second;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void    AddLevelName (DgnModelP modelRef, LevelId level)
//    {
//#ifdef WIP_NEEDS_WORK
//    WChar     levelName[4096];
//
//    if (m_levelNames.find (level) == m_levelNames.end() &&
//        SUCCESS == mdlLevel_getName (levelName, DIM(levelName), modelRef, level))
//        m_levelNames[level] = WString (levelName);
//#endif
//    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyGraphicsP GetOrCreateProxyGraphics (VisibleEdgeCacheImplR cache, DisplayPath const& displayPath, DgnElementCR templateElement, ViewHandlerPassCR pass)
//    {
//    BeAssert (pass.m_clipPlaneIndex == 0 || pass.m_pass == ClipVolumePass::Cut);
//    T_ProxyPassMap::iterator      found = m_passMap.find (pass);
//
//    for (int i=0; i<displayPath.GetCount(); i++)
//        AddLevelName (displayPath.GetRoot(), getDisplayableLevel (displayPath.GetPathElem(i), displayPath.GetRoot()));
//
//    AddLevelName (displayPath.GetRoot(), templateElement.GetLevel());
//
//    m_dirty = true;
//    if (m_passMap.end() == found)
//        return (m_passMap[pass] = new ProxyModelPass())->FindOrCreateGraphics (cache, displayPath, templateElement);
//    else
//        return found->second->FindOrCreateGraphics (cache, displayPath, templateElement);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//bool    ContainsConcealed () const
//    {
//    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        if (curr->second->ContainsConcealed ())
//            return true;
//
//
//    for (T_ProxyModelVector::const_iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        if ((*curr)->ContainsConcealed())
//            return true;
//
//
//    return false;
//    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   GetConcealed (DisplayPath const& displayPath) const
//    {
//    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        {
//        if (curr->second->IsConcealed (displayPath, 0))
//            return true;
//        }
//
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//void    ClearConcealed ()
//    {
//    if (ContainsConcealed())
//        {
//        m_dirty = true;
//        for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//            curr->second->ClearConcealed ();
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   SetConcealed (DisplayPath const& displayPath, bool concealed)
//    {
//    StatusInt       status = ERROR;
//
//    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        {
//        ProxyElementP    proxyElement;
//
//        if (NULL != (proxyElement = curr->second->FindElement (displayPath, 0)))
//            {
//            if (proxyElement->SetConcealed (concealed))
//                {
//                m_dirty = true;
//                status = SUCCESS;
//                }
//            }
//        }
//
//    return status;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   SetConcealed (ElemIdPathCR path)
//    {
//    StatusInt       status = ERROR;
//
//    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        {
//        ProxyElementP    proxyElement;
//        Transform        transform;
//
//        if (NULL != (proxyElement = curr->second->FindElement (path, 0, transform)))
//            {
//            if (proxyElement->SetConcealed (true))
//                {
//                m_dirty = true;
//                status = SUCCESS;
//                }
//            }
//        }
//
//    return status;
//    }
//

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//void    GetConcealedPaths (T_ProxyDisplayPathVector& paths) const
//    {
//    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        {
//        if (curr->second->ContainsConcealed ())
//            {
//            ProxyDisplayPath        currPath (m_modelRef);
//            curr->second->GetConcealedPaths (paths, currPath);
//            }
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool    IsVisible (DisplayPath const& displayPath, ViewHandlerPassCR vhPass)
//    {
//    T_ProxyPassMap::iterator        found = m_passMap.find (vhPass);
//    ProxyElementP            proxyElement;
//
//    if (found == m_passMap.end() || NULL == (proxyElement = found->second->FindElement (displayPath, 0)))
//        return false;
//
//    return proxyElement->IsVisible (displayPath, 1);
//
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void    Draw (DisplayPath const& displayPath, ViewContextR context, T_ProxyPassMap::iterator it)
//    {
//    ProxyElementP            proxyElement;
//
//    if (NULL == (proxyElement = it->second->FindElement (displayPath, 0)))
//        return;
//
//    ElementRefP      elRef;
//
//    if (NULL == (elRef = proxyElement->GetTransientElemRef()))   // When we are previewing the TransientElemRef is not yet created - but the symbology has not yet been cloned.
//        elRef = displayPath.GetTailElem();                       // to the root modelRef so it OK to use the real display path element for cooking.
//
//    ElementHandle      elHandle (elRef, displayPath.GetRoot());
//
//    context.CookElemDisplayParams (elHandle);
//    proxyElement->Draw (context, elHandle);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool    VisitHitPath (HitPathCR hitPath, ViewContextR context)
//    {
//    T_ProxyPassMap::iterator    found;
//    ProxyElementP               proxyElement;
//    ProxyDisplayHitInfoCP       proxyHitInfo;
//    Transform                   transform;
//
//    transform.initIdentity ();
//
//    if (NULL == (proxyHitInfo = dynamic_cast <ProxyDisplayHitInfoCP> (hitPath.GetViewHandlerHitInfo())) ||
//        m_passMap.end() == (found = m_passMap.find (proxyHitInfo->m_viewHandlerPass)) ||
//         NULL == (proxyElement = found->second->FindElement (hitPath, 0, &transform)))
//        return false;
//
//    ViewContext::DgnModelMark   modelRefMark (context);
//    if (SUCCESS != context.SetToDgnModel (m_modelRef))
//        return false;
//
//    ElementHandle      eh (proxyElement->GetTransientElemRef(), hitPath.GetRoot());
//
//    context.CookElemDisplayParams (eh);
//    context.ActivateOverrideMatSymb ();
//
//    if (!transform.isIdentity())
//        context.PushTransform (transform);
//
//    CurvePrimitiveIdCP  curveId;
//
//    if (hitPath.GetComponentMode () && NULL != (curveId = hitPath.GetGeomDetail ().GetCurvePrimitiveId ()))
//        {
//        bvector<UInt8>     idData;
//        ProxyEdgeIdData     edgeId;
//
//        curveId->Store (idData);
//        if (SUCCESS == edgeId.Init (&idData.front(), idData.size()))
//            {
//            proxyElement->FlashHitEdge (context, edgeId, eh);
//            return true;
//            }
//        }
//
//    proxyElement->Draw (context, eh);
//    return true;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    Draw (DisplayPath const& displayPath, ViewContextR context, ViewHandlerPassCR viewHandlerPass, ViewHandlerPassR hitInfoPass)
//    {
//    T_ProxyPassMap::iterator        found;
//
//    if (m_passMap.end() != (found = m_passMap.find (viewHandlerPass)))
//        {
//        hitInfoPass = viewHandlerPass;
//        Draw (displayPath, context, found);
//        }
//    else if (ClipVolumePass::None == viewHandlerPass.GetViewHandlerPass())
//        {
//        ProxyElementElemRefP  transientElemRef = dynamic_cast <ProxyElementElemRefP> (displayPath.GetTailElem());
//
//        for (T_ProxyPassMap::iterator  curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//            {
//            ProxyElementP    proxyElement;
//
//            if (NULL != (proxyElement = curr->second->FindElement (displayPath, 0)))
//                {
//                if (NULL == transientElemRef || transientElemRef == proxyElement->GetTransientElemRef ())
//                    {
//                    ElementHandle      elHandle (displayPath.GetTailElem(), displayPath.GetRoot());
//
//                    hitInfoPass = curr->first;
//                    context.CookElemDisplayParams (elHandle);
//                    proxyElement->Draw (context, elHandle);
//                    }
//                }
//            }
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ElemIdPathCR path, ProxyEdgeIdDataCR edgeId, GPArrayParamCP edgeParam, ViewHandlerPassCR vhPass, TransformR transform) const
//    {
//    T_ProxyPassMap::const_iterator found =  m_passMap.find (vhPass);
//
//    return (found == m_passMap.end()) ? ERROR : found->second->GetProxyGraphicsFromEdgeId (edgeGraphics, path, edgeId, edgeParam, transform);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            Dump ()
    {
    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
        {
        static char const* s_passNames[] = { "None", "Inside Forward", "Inside Backward", "Outside", "Inside", "Cut"};

        BeConsole::Printf ("Pass: %hs\n", s_passNames[static_cast<UInt32>(curr->first.m_pass)]);
        curr->second->Dump ();
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveModelInfo (ProxyDataBuffer& buffer)
    {
    // this is a separate subroutine so the modelElement doesn't take stack space in the recursive Save method.
    //if (!m_modelInfo.IsNull())
    //    {
    //    DgnElement       modelElement;
    //
    //    m_modelInfo->ToModelElement (modelElement, false, false);
    //    buffer.Write (&modelElement, modelElement.Size ());
    //    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Save (ElementRefP elemRef, UInt32& xAttributeIndex, DgnModelP rootDgnModel, DgnModelP parentDgnModel, ProxyDataBuffer& buffer)
    {
    //if (m_dirty)
    //    {
    //    parentDgnModel->GetDgnFileP()->SaveExtendedColorMap();
    //
    //    buffer.Clear ();
    //
    //    size_t      modelBTFLocation  = buffer.Write ((UInt32) 0);                           // Model Bytes To Follow
    //    size_t      headerBTFLocation = buffer.Write ((UInt32) 0);                           // Header Bytes To Follow
    //    buffer.Write (s_currentProxyModelVersionNumber);                                     // Version Number
    //    buffer.Write ((UInt32) 0);                                                           // Model Flags.
    //
    //    BeAssert (0 != m_attachmentId);
    //    buffer.Write (m_attachmentId);
    //    buffer.Write (m_transformToParent);
    //    buffer.Write (m_logicalName);
    //    buffer.Write (m_fileName);
    //    buffer.Write (&m_refDisplay, sizeof (m_refDisplay));
    //
    //
    //    // save the current reference attachment state hash.  (Calculate here so that the occlusion map is included).
    //    attachmentParameterHashFromReference (m_refParameterHash, *m_modelRef->AsDgnAttachmentP());
    //
    //    buffer.Write (&m_refParameterHash, sizeof(m_refParameterHash));
    //
    //    buffer.Write ((UInt32) m_children.size());
    //
    //    size_t      modelElementBTFLocation = buffer.Write ((UInt32) 0);
    //
    //    SaveModelInfo (buffer);
    //
    //    buffer.UpdateBytesToFollow (modelElementBTFLocation);
    //    buffer.UpdateBytesToFollow (headerBTFLocation);
    //
    //    size_t     levelsBTFLocation = buffer.Write ((UInt32) 0);                           // Header Bytes To Follow
    //
    //    buffer.Write ((UInt32) m_levelNames.size());
    //    for (T_LevelNameMap::iterator curr = m_levelNames.begin(), end = m_levelNames.end(); curr != end; curr++)
    //        {
    //        buffer.Write (curr->first);
    //        buffer.Write (curr->second);
    //        }
    //
    //    buffer.UpdateBytesToFollow (levelsBTFLocation);
    //
    //
    //    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
    //        {
    //        size_t      passBTFLocation  = buffer.Write ((UInt32) 0);                       // Pass Bytes To Follow
    //        buffer.Write ((UInt32) curr->first.m_pass);
    //        buffer.Write ((UInt32) curr->first.m_clipPlaneIndex);
    //        BeAssert (curr->first.m_clipPlaneIndex == 0 || curr->first.m_pass == ClipVolumePass::Cut);
    //
    //        curr->second->Save (buffer);
    //        buffer.UpdateBytesToFollow (passBTFLocation);
    //        }
    //
    //    buffer.UpdateBytesToFollow (modelBTFLocation);
    //    buffer.SaveCompressedToXAttributes (elemRef, CachedVisibleEdgeCacheId(), xAttributeIndex);
    //    m_dirty = false;
    //    }
    //xAttributeIndex++;
    //
    //for (size_t i=0; i<m_children.size(); i++)
    //    m_children[i]->Save (elemRef, xAttributeIndex, rootDgnModel, parentDgnModel, buffer);
    //
    //return SUCCESS;
    BeAssert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//void    ClearElementModifiedTimes (bool onlyIfInSynch)
//    {
//    m_dirty = true;
//
//    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        curr->second->ClearElementModifiedTimes ((!onlyIfInSynch || NULL == m_modelRef) ? NULL : m_modelRef->GetDgnModelP());
//
//    for (size_t i=0; i<m_children.size(); i++)
//        m_children[i]->ClearElementModifiedTimes (onlyIfInSynch);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//static DgnModelP  RestoreDgnModelFromAttachmentIds (byte const*& dataP, byte const* dataEndP, DgnModelP rootDgnModel)
//    {
//    UInt32          attachmentIdSize;
//    
//    ProxyRestoreUtil::CopyData (attachmentIdSize, dataP, dataEndP);
//    
//    ElementId                attachmentId;
//    T_ProxyElementIdVector   attachmentIds;
//    
//    for (UInt32 i=0; i<attachmentIdSize; i++)
//        {
//        ProxyRestoreUtil::CopyData (attachmentId, dataP, dataEndP);
//    
//        attachmentIds.push_back (attachmentId);
//        }
//    
//    DgnModelP modelRef = rootDgnModel;
//    for (size_t i=0; i<attachmentIds.size(); i++)
//        if (NULL == (modelRef =ProxyRestoreUtil::FindModelChild (modelRef, attachmentIds[i])))
//            break;          // This will occur if model has been detached (and sometimes during merge)...
//    
//    return modelRef;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static ProxyDisplayModelP Restore (VisibleEdgeCacheImplR proxyCache, ElementHandleCR el, UInt32& xAttributeIndex, DgnModelP rootDgnModel, ProxyDisplayModelP parent)
    {
    ElementHandle::XAttributeIter      xAttr (el, CachedVisibleEdgeCacheId(), xAttributeIndex++);

    if (!xAttr.IsValid())
        return NULL;

    size_t                  uncompressedSize;
    byte*                   buffer;
    ProxyDisplayModelP      newProxyModel = NULL;

    if (SUCCESS != CompressedXAttribute::GetUncompressedSizeFromData (xAttr.PeekData(), &uncompressedSize) ||
        NULL == (buffer = (byte*) malloc (uncompressedSize)))
        {
        BeAssert (false);
        return NULL;
        }

    if (SUCCESS != CompressedXAttribute::ExtractBufferFromData (xAttr.PeekData(), xAttr.GetSize(), buffer, uncompressedSize))
        {
        BeAssert (false);
        free (buffer);
        return NULL;
        }

    try
        {
        byte const*           dataP = (byte const*) buffer;
        byte const*           dataEndP = dataP + uncompressedSize;
        UInt32          modelBytesToFollow, headerBytesToFollow, modelHeaderBytesToFollow, passBytesToFollow, version, flags;

        ProxyRestoreUtil::CopyData (modelBytesToFollow, dataP, dataEndP);

        byte const*           nextModelDataP = dataP + modelBytesToFollow;

        if (nextModelDataP > dataEndP)
            throw ProxyRestoreUtil::ReadError ();

        if (SUCCESS != ProxyRestoreUtil::CopyData (headerBytesToFollow, dataP, nextModelDataP) || dataP +  headerBytesToFollow > nextModelDataP)
            throw ProxyRestoreUtil::ReadError ();

        byte const*                   levelDataP = dataP + headerBytesToFollow;
        UInt32                  childCount;
        /*ElementId*/UInt64               attachmentId;
        WString                 logicalName, fileName;
        Transform               transformToParent;
        RefDisplayParameters    refDisplay;
        //ModelInfoPtr            modelInfo;
        DsigRawHash             refParameterHash;

        ProxyRestoreUtil::CopyData (version, dataP, nextModelDataP);

        if (version != s_currentProxyModelVersionNumber)
            throw ProxyRestoreUtil::ReadError ();

        ProxyRestoreUtil::CopyData (flags, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyData (attachmentId, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyData (transformToParent, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyString (logicalName, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyString (fileName, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyData (&refDisplay, sizeof (refDisplay), dataP, nextModelDataP);
        copyDsigRawHash (refParameterHash, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyData (childCount, dataP, nextModelDataP);
        ProxyRestoreUtil::CopyData (modelHeaderBytesToFollow, dataP, nextModelDataP);

        //if (0 != modelHeaderBytesToFollow)
        //    modelInfo = ModelInfo::Create (*((ModelElement*) dataP), rootDgnModel->GetDgnFileP());

        dataP += modelHeaderBytesToFollow;

        newProxyModel = new ProxyDisplayModel (attachmentId, transformToParent, logicalName, fileName, refDisplay, rootDgnModel, parent, /*modelInfo*/NULL, refParameterHash);

        BeAssert (dataP == levelDataP);
        UInt32          levelBytesToFollow;
        ProxyRestoreUtil::CopyData (levelBytesToFollow, dataP, nextModelDataP);

        byte const*           levelEndDataP = dataP + levelBytesToFollow;

        UInt32          levelCount;
        ProxyRestoreUtil::CopyData (levelCount, dataP, levelEndDataP);
        for (UInt32 i=0; i<levelCount; i++)
            {
            LevelId     levelId;
            WString     levelName;
            UInt32 levelIdValue;
            ProxyRestoreUtil::CopyData (levelIdValue, dataP, levelEndDataP);
            ProxyRestoreUtil::CopyString (levelName, dataP, levelEndDataP);

            newProxyModel->m_levelNames[levelIdValue] = levelName;
            }

        //DgnModelP            modelRef = newProxyModel->m_modelRef;
        //DgnModelP               dgnModel = (NULL == modelRef) ? NULL : modelRef->GetDgnModelP();

        while (dataP < nextModelDataP)
            {
            byte const*           nextPassDataP;
            UInt32          pass, clipPlaneIndex;

            if (SUCCESS != ProxyRestoreUtil::CopyData (passBytesToFollow, dataP, nextModelDataP) ||
                (nextPassDataP = dataP + passBytesToFollow) > nextModelDataP ||
                SUCCESS != ProxyRestoreUtil::CopyData (pass, dataP, nextPassDataP))
                throw ProxyRestoreUtil::ReadError ();

            ProxyRestoreUtil::CopyData (clipPlaneIndex, dataP, nextPassDataP);

            ProxyModelPassP        elements = new ProxyModelPass();
            if (SUCCESS != elements->Restore (proxyCache, dataP, nextPassDataP, NULL,NULL))//dgnModel, modelRef))
                {
                delete elements;
                throw ProxyRestoreUtil::ReadError ();
                }

            newProxyModel->m_passMap[ViewHandlerPass ((ClipVolumePass) pass, clipPlaneIndex)] = elements;
            }
        BeAssert (dataP == nextModelDataP);
        //proxyCache.AddToModelMap (modelRef, newProxyModel);

        for (size_t i=0; i<childCount; i++)
            {
            ProxyDisplayModelP  child;

            if (NULL == (child = Restore (proxyCache, el, xAttributeIndex, rootDgnModel, newProxyModel)))
                {
                DELETE_AND_CLEAR (newProxyModel);
                break;
                }

            newProxyModel->m_children.push_back (child);
            }
        }

    catch (ProxyRestoreUtil::ReadError)
        {
        DELETE_AND_CLEAR (newProxyModel);
        }

    free (buffer);
    return newProxyModel;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//TransformP     GetCacheRootTransform (TransformR transform)
//    {
//    transform = m_transformToParent;
//
//    for (ProxyDisplayModelP parent = m_parent; NULL != parent && NULL != parent->m_parent; parent = parent->m_parent)
//        transform.productOf (&parent->m_transformToParent, &transform);
//
//    return &transform;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    DrawStaticChild (ViewContextR context, DgnModelR rootParentDgnModel, ClipVolumePass pass, ViewHandlerPassR hitInfoPass, UInt32 displayFlags)
//    {
//    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end && !context.CheckStop(); curr++)
//        {
//        if (curr->first.m_pass == pass)
//            {
//            ViewContext::DgnModelMark   modelRefMark (context);
//            Transform                   cacheRootTransform;
//
//            context.SetPathRoot (m_modelRef);
//
//            context.PushTransform (*GetCacheRootTransform (cacheRootTransform));
//            hitInfoPass = curr->first;
//            curr->second->DrawStatic (context, rootParentDgnModel, *m_modelRef, displayFlags);
//            }
//        }
//    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end() && !context.CheckStop(); curr++)
//        (*curr)->DrawStaticChild (context, rootParentDgnModel, pass, hitInfoPass, displayFlags);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//void    CloneAndResolveTransients (DgnModelP targetDgnModel)
//    {
//    ElementCopyContext    copyContext(targetDgnModel);
//
//    copyContext.SetSourceDgnModel (m_modelRef);
//    copyContext.SetWriteElements (false);
//    copyContext.SetTransformToDestination (true);
//
//    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(); curr != m_passMap.end(); curr++)
//        {
//        curr->second->Clone (&copyContext);
//        curr->second->ResolveTransients (m_modelRef);
//        }
//
//    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        (*curr)->CloneAndResolveTransients (targetDgnModel);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    PassGeometryExists (ClipVolumePass pass) const
    {
    for (T_ProxyPassMap::const_iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
        if (curr->first.m_pass == pass)
            return true;

    for (T_ProxyModelVector::const_iterator curr = m_children.begin(); curr != m_children.end(); curr++)
        if ((*curr)->PassGeometryExists (pass))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool    DrawStatic (ViewContextR context, DgnModelR cacheRootDgnModel, DgnModelListP includeList, bool includeRefs, VisibleEdgeCacheImplR cache, ViewHandlerPassR hitInfoPass, UInt32 displayFlags)
//    {
//    DynamicViewSettingsCP    dvSettings = context.GetDynamicViewSettings (&cacheRootDgnModel);
//
//    if (&cacheRootDgnModel != m_modelRef ||
//        NULL == (dvSettings = context.GetDynamicViewSettings (m_modelRef)))         // Neither of these conditions should ever occur.
//        {
//        BeAssert (false);
//        return false;
//        }
//    DgnModelP        rootParentDgnModel = cacheRootDgnModel.GetParentDgnModelP();
//    DgnModelP        rootParentDgnModel = &cacheRootDgnModel;
//
//    context.OnNewDgnModel (&cacheRootDgnModel);
//
//    DisplayStylePtr     wireFrameStyle = DisplayStyle::CreateHardCodedDefault (MSRenderMode::Wireframe, rootParentDgnModel->GetDgnProject());
//
//    context.PushDisplayStyle (wireFrameStyle.get(), &cacheRootDgnModel, false);
//
//    // The level symbology overrides are already set in the cache - Avoid applying them again here.
//    ViewFlags           viewFlags = *context.GetViewFlags();
//    viewFlags.lev_symb = FALSE;
//    context.SetViewFlags (&viewFlags);
//
//    for (int pass = static_cast<int>(ViewHandlerPass_Underlay); pass >= 0; pass--)
//        {
//        ClipVolumePass  clipVolumePass = (ClipVolumePass) pass;
//
//        if (PassGeometryExists(clipVolumePass))
//            {
//            ViewContext::DgnModelMark    mark (context);
//
//            for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//                {
//                if (pass == static_cast<UInt32>(curr->first.m_pass))
//                    {
//                    hitInfoPass = curr->first;
//                    curr->second->DrawStatic (context, *rootParentDgnModel, *m_modelRef, displayFlags);
//                    }
//                }
//
//            if (includeRefs)
//                for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//                    (*curr)->DrawStaticChild (context, *rootParentDgnModel, (ClipVolumePass) pass,  hitInfoPass, displayFlags);
//
//            }
//        }
//
//    context.PopDisplayStyle();
//
//    return true;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   GetRange (DRange3dR range)
//    {
//    range.init();
//
//    DRange3d        passRange;
//    for (T_ProxyPassMap::iterator curr = m_passMap.begin(), end = m_passMap.end(); curr != end; curr++)
//        if (SUCCESS == curr->second->GetRange (passRange))
//            range.extend (&passRange);
//
//    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        {
//        DRange3d    childRange;
//
//        if (SUCCESS == (*curr)->GetRange (childRange))
//            {
//            (*curr)->m_transformToParent.multiply (&childRange, &childRange);
//            range.extend (&childRange);
//            }
//        }
//    return range.isNull() ? ERROR : SUCCESS;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool        MissingReference ()
//    {
//    if ( (NULL == m_modelRef) || (DgnModelType::Proxy == m_modelRef->GetDgnModelType()) )
//        return true;
//
//    DgnAttachmentP  refP;
//    if ( (NULL == (refP = m_modelRef->AsDgnAttachmentP())) || refP->IsMissingFile())
//        return true;
//
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool        AnyMissingAttachments ()
//    {
//    // is this reference missing?
//    if (MissingReference())
//        return true;
//
//    // check for missing children.
//    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        {
//        if ((*curr)->AnyMissingAttachments())
//            return true;
//        }
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool        AttachmentParametersDiffer ()
//    {
//    DgnAttachmentP  refP;
//    if ( (NULL == (refP = m_modelRef->AsDgnAttachmentP())) || refP->IsMissingFile())
//        {
//        // these cases should have been sorted out by the missingAttachments check, which is done first.
//        BeAssert (false);
//        return true;
//        }
//
//    DsigRawHash             currentHash;
//    attachmentParameterHashFromReference (currentHash, *refP);
//    if (0 != CompareHash (m_refParameterHash, currentHash))
//        return true;
//
//    // check child references
//    for (T_ProxyModelVector::iterator curr = m_children.begin(); curr != m_children.end(); curr++)
//        {
//        if ((*curr)->AttachmentParametersDiffer())
//            return true;
//        }
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//BitMaskCP       GetLevelUsage () const
//    {
//    if (NULL == m_levelUsage)
//        {
//        m_levelUsage = BitMask::Create ( FALSE);
//        for (T_LevelNameMap::const_iterator curr = m_levelNames.begin(), end = m_levelNames.end(); curr != end; curr++)
//            {
//            if (0 == curr->first)
//                {
//                BeAssert (false);         // Should never occur.
//                }
//            else
//                {
//                m_levelUsage->SetBit ( curr->first - 1,  TRUE);
//                }
//            }
//        }
//    return m_levelUsage;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCacheElementCP  GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const
//    {
//    T_ProxyPassMap::const_iterator        found = m_passMap.find (vhPass);
//
//    return (found == m_passMap.end()) ? NULL : found->second->FindElement (displayPath, 0);
//    }

void CreateCveElements (DgnModelR, TransformCR transformToProject, TransformCR unitsTransform, VisibleEdgeCache::ICreateCveElementsHelper&, bvector<UInt64> const&); // added in graphite

}; //    ProxyDisplayModel


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//ModelInfoCP     VisibleEdgeCacheImpl::_GetModelInfoCP (DgnModelCR modelRef) const
//    {
//    T_ProxyModelMap::const_iterator found = m_proxyModelMap.find (const_cast <DgnModelP> (&modelRef));
//
//    return found == m_proxyModelMap.end() ? NULL : found->second->m_modelInfo.get();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//LevelCacheP VisibleEdgeCacheImpl::_GetLevelCache (DgnModelCR modelRef)
//    {
//    if (NULL != modelRef.AsDgnModelCP())
//        return NULL;            // Return NULL to use actual level cache.
//
//    T_ProxyModelMap::const_iterator found = m_proxyModelMap.find (const_cast <DgnModelP> (&modelRef));
//
//    return found == m_proxyModelMap.end() ? NULL : found->second->m_proxyLevelCache.get();
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//BitMaskCP       VisibleEdgeCacheImpl::GetLevelUsage (DgnModelP modelRef) const
//    {
//    T_ProxyModelMap::const_iterator found = m_proxyModelMap.find (modelRef);
//
//    return found == m_proxyModelMap.end() ? NULL : found->second->GetLevelUsage();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
VisibleEdgeCacheImpl::VisibleEdgeCacheImpl () : m_rootProxyModel (NULL)//, m_obsoleteVersion (false), m_obsoleteHash (false), m_occlusionPaths (NULL)
    {
    //m_creationTime = 0.0;
    //m_rootUnitScaleFactor   = 1.0;;
    //memset (&m_hashValue, 0, sizeof (m_hashValue));
    //m_newestElement         = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCacheImpl::VisibleEdgeCacheImpl (DgnModelP rootModel, CachedVisibleEdgeOptionsCR options) : VisibleEdgeCache (rootModel, options), m_rootProxyModel (NULL), m_occlusionPaths (NULL)
//    {
//    // these members are to help us determine whether the cache is valid.
//    m_newestElement = 0.0;
//    memset ((void*) &m_hashValue, 0, sizeof (m_hashValue));
//
//    m_creationTime  = osTime_getCurrentMillis();     // Used to determine when a cache change necessitates association updates.
//    m_obsoleteVersion = m_obsoleteHash = false;
//
//    if (NULL == rootModel->AsDgnAttachmentP() || SUCCESS != rootModel->AsDgnAttachmentP()->GetScaleByUnitsFactor (m_rootUnitScaleFactor))
//        m_rootUnitScaleFactor = 1.0;
//
//    m_rootProxyModel = AddModel (rootModel, NULL);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayModelP   VisibleEdgeCacheImpl::AddModel (DgnModelP modelRef, ProxyDisplayModelP parent)
//    {
//    ProxyDisplayModelP      newModel = m_proxyModelMap[modelRef] = new ProxyDisplayModel (modelRef, parent);
//    DgnAttachmentArrayP    children;
//
//    if (NULL != (children = modelRef->GetDgnAttachmentsP()))
//        {
//        for (size_t i=0; i<children->size(); i++)
//            {
//            DgnModelP    childModel = children->at(i);
//
//            // skip any references that don't have a DgnCache - they will not contribute any graphics, so we don't care about them.
//            if (NULL != childModel->GetDgnModelP())
//                newModel->m_children.push_back (AddModel (childModel, newModel));
//            }
//        }
//    return newModel;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool                VisibleEdgeCacheImpl::_AnyCachedChildReferencesMissing () const
//    {
//    // This is called to determine whether we want to allow synchronization of an out of date cache.
//    // We don't want to allow it if there were references present when we created the cache that we now can't find.
//
//    // if we have no root proxy model, don't prevent synchronize.
//    if (NULL == m_rootProxyModel)
//        return false;
//
//    return m_rootProxyModel->AnyMissingAttachments();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
VisibleEdgeCacheImpl::~VisibleEdgeCacheImpl ()
    {
    DELETE_AND_CLEAR (m_rootProxyModel);
    }

//bool    VisibleEdgeCacheImpl::_ContainsModel (DgnModelCR modelRef) const { return m_proxyModelMap.find (const_cast <DgnModelP> (&modelRef)) != m_proxyModelMap.end(); }


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//void    VisibleEdgeCacheImpl::Dump () const
//    {
//    BeConsole::Printf (">>>>>>>>>>> Proxy Cache Dump <<<<<<<<<<<<<<<<<<<<\n");
//    for (T_ProxyModelMap::const_iterator curr = m_proxyModelMap.begin(), end = m_proxyModelMap.end(); curr != end; curr++)
//        {
//        BeConsole::Printf ("Model: %s\n", curr->first->GetModelName());
//        curr->second->Dump ();
//        }
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    VisibleEdgeCacheImpl::_ContainsConcealed () const
//    {
//    return NULL != m_rootProxyModel && m_rootProxyModel->ContainsConcealed ();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
//void    VisibleEdgeCacheImpl::_ClearConcealed ()
//    {
//    for (T_ProxyModelMap::iterator curr = m_proxyModelMap.begin(), end = m_proxyModelMap.end(); curr != end; curr++)
//        curr->second->ClearConcealed ();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt    VisibleEdgeCacheImpl::_SetConcealed (DisplayPath const& displayPath, bool concealed)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    return(m_proxyModelMap.end() == found) ? ERROR : found->second->SetConcealed (displayPath, concealed);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    VisibleEdgeCacheImpl::_GetConcealed (DisplayPath const& displayPath) const
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    return found != m_proxyModelMap.end() && found->second->GetConcealed (displayPath);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
// void    VisibleEdgeCacheImpl::_GetConcealedPaths (T_ProxyDisplayPathVector& paths) const
//    {
//    for (T_ProxyModelMap::const_iterator curr = m_proxyModelMap.begin(), end = m_proxyModelMap.end(); curr != end; curr++)
//        curr->second->GetConcealedPaths (paths);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+--------------+---------------+---------------+---------------+---------------+------*/
//void    VisibleEdgeCacheImpl::_AddConcealedPaths (T_ProxyDisplayPathVector const& paths)
//    {
//    for (T_ProxyDisplayPathVector::const_iterator curr = paths.begin(); curr != paths.end(); curr++)
//        {
//        T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find ((*curr)->m_rootModel);
//
//        if (found != m_proxyModelMap.end())
//            found->second->SetConcealed ((*curr)->m_path);
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt VisibleEdgeCacheImpl::_AddGPA (GPArrayCR gpa, GPArrayIntervalsP intervals, DisplayPath const& displayPath, ViewHandlerPassCR viewHandlerPass, DgnElementCR templateElement, ProxyGraphicsFlags flags, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateCP cds, bool isClipped)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    if (m_proxyModelMap.end() != found)
//        {
//        StatusInt status;
//
//        if (SUCCESS == (status = found->second->GetOrCreateProxyGraphics (*this, displayPath, templateElement, viewHandlerPass)->AddGPA (gpa, intervals, templateElement, displayPath.GetRoot(), flags, segmentId, cds, isClipped)))
//            {
//            // &&
//            //NULL != m_loadPreviewer)
//            //m_loadPreviewer->Display (displayPath, viewHandlerPass, *this);
//            }
//
//        return status;
//        }
//
//    BeAssert(false);
//    return ERROR;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt VisibleEdgeCacheImpl::_AddCutGraphics (XGraphicsContainerCR xGraphics, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement, ProxyGraphicsFlags flags, bool filled)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    // Note - we can't do Preview display for cutGraphics as we don't have the correct cut planes pushed (caching will be incorrect etc.).
//    if (m_proxyModelMap.end() != found)
//        return found->second->GetOrCreateProxyGraphics (*this, displayPath, templateElement, vhPass)->AddCutGraphics (xGraphics, templateElement, displayPath.GetRoot(), flags, filled);
//
//    BeAssert(false);
//    return ERROR;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt VisibleEdgeCacheImpl::_AddPassthrough (XGraphicsContainerCR xgContainer, ProxyGraphicsType type, ViewHandlerPassCR vhPass, DisplayPath const& displayPath, DgnElementCR templateElement)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    if (m_proxyModelMap.end() != found)
//        {
//        StatusInt status;
//
//        if (SUCCESS == (status = found->second->GetOrCreateProxyGraphics (*this, displayPath, templateElement, vhPass)->AddPassthrough (xgContainer, type, templateElement, displayPath.GetRoot(), ProxyGraphicsFlags())))
//            {
//            // &&
//            //NULL != m_loadPreviewer)
//            //m_loadPreviewer->Display (displayPath, vhPass, *this);
//            }
//        return status;
//        }
//
//    BeAssert(false);
//    return ERROR;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+--------------+---------------+---------------+---------------+---------------+------*/
//void  VisibleEdgeCacheImpl::_AddOcclusionPaths (HLOcclusionPathSetCR occlusionPaths)
//    {
//    UInt32                              index = 0;
//    bmap <HLOcclusionPathCP, UInt32>    pathMap;
//
//
//    for (HLOcclusionPathCP path: occlusionPaths)
//        pathMap[path] = index++;
//
//    m_occlusionPaths = new SavedOcclusionPaths();
//
//    m_occlusionPaths->reserve (occlusionPaths.size());
//    for (HLOcclusionPathP path: occlusionPaths)
//        {
//        bset <UInt32>           occluderSet;
//        bvector <UInt32>        occluderIndices;
//
//
//        for (HLOcclusionPathCP occlusionPath: path->m_occluders)
//            {
//            bmap <HLOcclusionPathCP, UInt32>::iterator found = pathMap.find (occlusionPath);
//
//            if (found == pathMap.end())
//                {
//                BeAssert (false);
//                continue;
//                }
//
//            UInt32      index = found->second;
//
//            BeAssert (occluderSet.find(index) == occluderSet.end());
//            occluderIndices.push_back(index);
//            occluderSet.insert (index);
//            }
//
//        m_occlusionPaths->push_back (SavedOcclusionPath (*path, occluderIndices));
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt  VisibleEdgeCacheImpl::_GetProxyGraphicsFromEdgeId (XGraphicsContainerR edgeGraphics, ElemIdPathCR attachIds, ElemIdPathCR pathIds, ProxyEdgeIdDataCR edgeId, struct GPArrayParam const& edgeParam, ViewHandlerPassCR vhPass) const
//    {
//    StatusInt               status;
//    ProxyDisplayModelCP     model;
//    TransformInfo           transformInfo;
//    
//    if (NULL == m_rootProxyModel)
//        return ERROR;
//    
//    transformInfo.GetTransformR().initIdentity();
//    
//    if (NULL == (model = static_cast <ProxyDisplayModelCP> (m_rootProxyModel->FindChild (attachIds, attachIds.size(), transformInfo.GetTransformR()))))
//        return ERROR;
//    
//    if (SUCCESS == (status = model->GetProxyGraphicsFromEdgeId (edgeGraphics, pathIds, edgeId, &edgeParam, vhPass, transformInfo.GetTransformR())) &&
//        !transformInfo.GetTransformR().isIdentity ())
//        edgeGraphics.OnTransform (transformInfo);
//    
//    return status;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    VisibleEdgeCacheImpl::IsVisible (DisplayPath const& displayPath, ViewHandlerPassCR vhPass)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    return (m_proxyModelMap.end() != found) ? found->second->IsVisible (displayPath, vhPass) : false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//bool    VisibleEdgeCacheImpl::VisitHitPath (HitPathCR hitPath, ViewContextR context)
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (hitPath.GetRoot());
//
//    context.SetColorMapForDgnModel (hitPath.GetRoot());//GetParentDgnModelP());
//    return (m_proxyModelMap.end() != found) ? found->second->VisitHitPath (hitPath, context) : false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//void VisibleEdgeCacheImpl::_Draw (DisplayPath const& displayPath, ViewContextR context, ViewHandlerPassCR vhPass, ViewHandlerPassR hitInfoPass)
//    {
//    //if (!IsValidForViewContext (context, false))
//    //    return;
//
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    if (found != m_proxyModelMap.end())
//        found->second->Draw (displayPath, context, vhPass, hitInfoPass);
//    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2013
+--------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCacheElementCP      VisibleEdgeCacheImpl::_GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const
//    {
//    T_ProxyModelMap::const_iterator     found = m_proxyModelMap.find (displayPath.GetRoot());
//
//    return (found == m_proxyModelMap.end()) ? NULL : found->second->GetCacheElement (displayPath, vhPass);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt VisibleEdgeCacheImpl::_GetRange (DRange3dR range) const { return NULL == m_rootProxyModel ? ERROR : m_rootProxyModel->GetRange (range); }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//void      VisibleEdgeCacheImpl::_Resolve ()
//    {
//   if (NULL != m_rootProxyModel)
//       m_rootProxyModel->CloneAndResolveTransients (GetParentDgnModelP());
//   
//   DgnAttachmentP          rootRef;
//   if (NULL != (rootRef = GetRootRefFile()))
//       {
//       DynamicViewSettings     dvSettings (rootRef->GetDynamicViewSettingsCR());
//   
//       dvSettings.RemapToParent (m_rootModel);
//       }
//   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//WStringCP      VisibleEdgeCacheImpl::_GetLevelName (DgnModelP modelRef, LevelId LevelId) const
//    {
//    T_ProxyModelMap::const_iterator   found = m_proxyModelMap.find (modelRef);
//
//    return (m_proxyModelMap.end() == found) ? NULL : found->second->GetLevelName (LevelId);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//UInt32  VisibleEdgeCacheImpl::GetViewFlags () const
//    {
//    //return (NULL == GetRootRefFile() || !GetRootRefFile()->GetDisplayConcealed()) ? ProxyDisplayFlagsMask_None : ProxyDisplayFlagsMask_DisplayConcealed;
//    return ProxyDisplayFlagsMask_None;
//    }
//
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//bool  VisibleEdgeCacheImpl::DrawDgnModel (ViewContextR context, DgnModelR modelRef, DgnModelListP includeList, bool includeRefs, ViewHandlerPassR hitInfoPass)
//    {
//    UInt32          displayFlags = GetViewFlags();
//
//    switch (context.GetDrawPurpose())
//        {
//        case DrawPurpose::ExportVisibleEdges:
//        case DrawPurpose::CaptureGeometry:
//            displayFlags &= ~ProxyDisplayFlagsMask_DisplayConcealed;     // Never capture hidden. - TR# 332593
//            break;
//
//        case DrawPurpose::ProxyHashExtraction:
//            return false;
//        }
//
//    return m_rootModel == &modelRef && NULL != m_rootProxyModel ? m_rootProxyModel->DrawStatic (context, modelRef, includeList, includeRefs, *this, hitInfoPass, displayFlags) : false;
//    }

static  size_t      s_futureReservedSectionCount = 4;

//void updateAssociationGenerators (ElementRefP attachElem, DgnModelP attachModel) { /* NEEDS_WORK */ }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//ElementRefP     findAttachmentElementRef (DgnModelP rootDgnModel)
//    {
//    DgnModelP    parentDgnModel;
//    DgnModelP       parentModel;
//    ElementRefP     elemRef;
//
//    if ((NULL == (parentDgnModel = rootDgnModel->GetParentDgnModelP()) || (NULL == (parentModel = parentDgnModel->GetDgnModelP()))))
//        {
//        BeAssert (false);
//        return NULL;
//        }
//
//    DgnAttachmentP  refP;
//    if (NULL == (refP = rootDgnModel->AsDgnAttachmentP()))
//        {
//        BeAssert (false);
//        return NULL;
//        }
//
//    if (NULL == (elemRef = parentModel->FindByElementId (refP->GetElementId())))
//        {
//        BeAssert (false);
//        }
//    return elemRef;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt      VisibleEdgeCacheImpl::_Save (DgnModelP rootDgnModel, ElementRefP elemRef) const
//    {
//    static      bool s_dump;
//
//    if (s_dump)
//        Dump ();
//
//    // save the proxy cache as an XAttribute on the reference file element.
//    if (NULL == elemRef &&
//        NULL == (elemRef = findAttachmentElementRef (rootDgnModel)))
//        return ERROR;
//
//    ProxyDataBuffer           buffer;
//    UInt32                    cacheFlags = s_currentHashAlgorithmVersion;   // Was zero for SS3 - added hash value version for Vancouver as the hashing algorithm changed.  Could use upper bits in future.
//
//    buffer.Clear ();
//
//    buffer.Write (s_currentProxyCacheVersionNumber);
//    buffer.Write (cacheFlags);
//
//    // write out the hash
//    buffer.Write (&m_hashValue, sizeof(m_hashValue));
//
//    // write out the newest element time
//    buffer.Write (m_newestElement);
//
//    // write out the creation time
//    buffer.Write (m_creationTime);
//
//    // write out the options that were used when the cache was created.
//    buffer.Write (&m_options, sizeof (m_options));
//
//    // write out the originating view and view group.
//    buffer.Write (m_originatingView);
//    buffer.Write (m_originatingViewGroup);
//
//    // write out the root unit scale factor
//    buffer.Write (m_rootUnitScaleFactor);
//
//    // write out some blank variable length sections for future expansion.
//    for (size_t i=0; i<s_futureReservedSectionCount; i++)
//        buffer.Write (0);
//
//    buffer.SaveToXAttributes (elemRef, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::Base);
//
//    // Save the occlusion map (for incremental calculations) - Do this before the models as it is stored to XAttr that effects the ref parameter hash.
//    if (NULL != m_occlusionPaths)
//        SaveOcclusionPaths (elemRef, rootDgnModel);
//
//    // now write out the models.
//    UInt32              xAttributeIndex = 1;
//
//    m_rootProxyModel->Save (elemRef, xAttributeIndex, rootDgnModel, rootDgnModel->GetParentDgnModelP(), buffer);
//
//    // Clear out any old models (if the number of children were reduced).
//    XAttributeHandlerId     xAttrHandlerId = CachedVisibleEdgeCacheId();
//
//    for (; true; xAttributeIndex++)
//        {
//        XAttributeHandle it (elemRef, xAttrHandlerId, xAttributeIndex);
//
//        if (it.IsValid())
//            ITxnManager::GetCurrentTxn().DeleteXAttribute (it);
//        else
//            break;
//        }
//
//    // Store hidden line settings in seperate XAttribute (so that toggle from cache to dynamic back to cache will preserve).  TFS# 12946/Fosters.
//    ProxyDataBuffer     optionBuffer;
//    optionBuffer.Write (&GetOptions(), sizeof (CachedVisibleEdgeOptions));
//    optionBuffer.SaveToXAttributes (elemRef, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::Settings);
//
//    // Association generators need to update themselves in order to notify their dependents.
//    updateAssociationGenerators (elemRef, rootDgnModel->GetParentDgnModelP()->GetDgnModelP());
//
//    return SUCCESS;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//void    deleteCacheXAttributes (ElementRefP elemRef)
//    {
//    XAttributeHandlerId     xAttrHandlerId = CachedVisibleEdgeCacheId();
//
//    for (UInt32 i=0; true; i++)
//        {
//        XAttributeHandle it (elemRef, xAttrHandlerId, i);
//
//        if (it.IsValid())
//            ITxnManager::GetCurrentTxn().DeleteXAttribute (it);
//        else
//            break;
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt      VisibleEdgeCache::Delete (DgnModelP rootDgnModel, ElementRefP inputElementRef)
//    {
//    ElementRefP     elemRef;
//
//    if (NULL == (elemRef = inputElementRef) &&
//        NULL == (elemRef = findAttachmentElementRef (rootDgnModel)))
//        return ERROR;
//
//    deleteCacheXAttributes (elemRef);
//
//    if (NULL == inputElementRef)
//        updateAssociationGenerators (elemRef,  rootDgnModel->GetParentDgnModelP()->GetDgnModelP());
//
//    VisibleEdgeCalculationCache::Delete (rootDgnModel, elemRef);
//
//    return SUCCESS;
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
VisibleEdgeCacheImplP      VisibleEdgeCacheImpl::Restore (ElementHandleCR el, DgnModelP rootModel)
    {
    ElementHandle::XAttributeIter      xAttr (el, CachedVisibleEdgeCacheId(), 0);

    if (!xAttr.IsValid())
        return NULL;

    byte const*           pData = (byte const*) xAttr.PeekData();
    size_t          dataSize = xAttr.GetSize();

    byte const*           pDataEnd = pData + dataSize;
    UInt32          cacheVersion, hashVersion;

    ProxyRestoreUtil::CopyData (cacheVersion, pData, pDataEnd);
    ProxyRestoreUtil::CopyData (hashVersion, pData, pDataEnd);

    // there's nothing useful we can do with a cache older than version 15.
    if (cacheVersion < FIRST_VERSION_WITH_VIEW)
        return NULL;

    VisibleEdgeCacheImplP         cache = new VisibleEdgeCacheImpl ();
    cache->m_rootModel = rootModel;

    try
        {
        DsigRawHash hashValueUnused;
        double doubleValueUnused;
        UInt32 uint32ValueUnused;
        UInt64 uint64ValueUnused;

        copyDsigRawHash (/*cache->m_hashValue*/hashValueUnused, pData, pDataEnd);

        ProxyRestoreUtil::CopyData (/*cache->m_newestElement*/doubleValueUnused, pData, pDataEnd);
        ProxyRestoreUtil::CopyData (/*cache->m_creationTime*/doubleValueUnused, pData, pDataEnd);

        copyCachedVisibleEdgeOptions (cache->m_options, pData, pDataEnd);

        ProxyRestoreUtil::CopyData (/*cache->m_originatingView*/uint32ValueUnused, pData, pDataEnd);
        ProxyRestoreUtil::CopyData (/*cache->m_originatingViewGroup*/uint64ValueUnused, pData, pDataEnd);
        ProxyRestoreUtil::CopyData (/*cache->m_rootUnitScaleFactor*/doubleValueUnused, pData, pDataEnd);

        // we don't want to discard the cache entirely, because it has the originating view, which we will need to resynch.
        // but we don't bother with trying to restore any of the model caches.
        //if (cacheVersion != s_currentProxyCacheVersionNumber)
        //    {
        //    cache->m_obsoleteVersion = true;
        //    return cache;
        //    }
        //
        //if (hashVersion != s_currentHashAlgorithmVersion)
        //    cache->m_obsoleteHash = true;
        //
        // Room for some future variable length data.
        for (size_t i=0; i<s_futureReservedSectionCount; i++)
            {
            UInt32      sectionSize;

            ProxyRestoreUtil::CopyData (sectionSize, pData, pDataEnd);
            pData += sectionSize;
            }

        }
    catch (...)
        {
        DELETE_AND_CLEAR (cache);
        return NULL;
        }

    UInt32              xAttributeIndex = 1;
    if (NULL == (cache->m_rootProxyModel = ProxyDisplayModel::Restore (*cache, el, xAttributeIndex, rootModel, NULL)))
        DELETE_AND_CLEAR (cache);

    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//static bool     IsEmptyHash (const DsigRawHash& testHash)
//    {
//    if (testHash.isComputed != 0)
//        return false;
//    if (testHash.algid != 0)
//        return false;
//    if (testHash.len   != 0)
//        return false;
//
//    return true;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void            VisibleEdgeCacheImpl::TestValid (bool& newerElement, bool& differentElementSet, bool& attachmentChanged, bool& missingReferences, DgnModelP modelRef, ViewportP viewport) const
//    {
//    // the missing reference condition is more of a concern, so figure that out first.
//    if (true == (missingReferences = m_rootProxyModel->AnyMissingAttachments()))
//        return;
//
//    // the attachment paramteers are relatively quick to test, test those next.
//    if (true == (attachmentChanged = m_rootProxyModel->AttachmentParametersDiffer()))
//        return;
//
//    // compare the hash that we have saved to the hash
//    if (NULL != viewport && NULL != modelRef && NULL != modelRef->AsDgnAttachmentP())
//        {
//        DsigRawHash                 currentHashValue;
//        double                      currentNewestElement = 0.0;
//        CachedVisibleEdgeHashParams hashParams (currentHashValue, currentNewestElement, *viewport, m_options, m_newestElement, *getCryptographer());
//
//        // compute the hash, aborting early if we encounter an element newer than the current m_newestElement.
//        T_HOST.GetDgnAttachmentAdmin()._ComputeCachedVisibleEdgeHash (hashParams, *modelRef->AsDgnAttachmentP());
//
//        // if any element newer than the newest one we encountered before, the style is invalid.
//        newerElement = (currentNewestElement > m_newestElement);
//        differentElementSet = (0 != CompareHash (currentHashValue, m_hashValue));
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void    CacheValidityState::AccumulateChildState (DgnModelP parentModel)
//    {
//    DgnAttachmentArrayP        children;
//
//    if (NULL != (children = parentModel->GetDgnAttachmentsP()))
//        {
//        for (DgnAttachmentArray::const_iterator curr = children->begin(), end = children->end(); curr != end; curr++)
//            {
//            DgnModelP    childModel;
//
//            if (NULL == (childModel = *curr))
//                continue;
//
//            m_childModelCount++;
//
//            DgnModelP   dgnModel;
//            if (NULL != (dgnModel = childModel->GetDgnModelP()))
//                {
//                m_sumOfChangeCounts += dgnModel->GetChangeCount();
//                double  thisFileTime = dgnModel->GetDgnFileP()->GetLastFileTime();
//                if (thisFileTime > m_latestFileTime)
//                    m_latestFileTime = thisFileTime;
//
//                AccumulateChildState (childModel);
//                }
//            }
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//CacheValidityState::CacheValidityState ()
//    {
//    Clear();
//    }
//
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//CacheValidityState::CacheValidityState (DgnModelP modelRef)
//    {
//    DgnModelP   dgnModel;
//    if (NULL == (dgnModel = modelRef->GetDgnModelP()))
//        {
//        Clear();
//        }
//    else
//        {
//        m_newerElements         = true;
//        m_differentElementSet   = true;
//        m_attachmentChanged     = true;
//        m_missingReferences     = true;
//        m_childModelCount       = 0;
//        m_sumOfChangeCounts     = dgnModel->GetChangeCount();
//        m_latestFileTime        = dgnModel->GetDgnFileP()->GetLastFileTime();
//        AccumulateChildState (modelRef);
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void CacheValidityState::Clear ()
//    {
//    m_newerElements         = true;
//    m_differentElementSet   = true;
//    m_attachmentChanged     = true;
//    m_missingReferences     = true;
//    m_childModelCount       = 0;
//    m_sumOfChangeCounts     = 0;
//    m_latestFileTime        = 0.0;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool CacheValidityState::Equals (const CacheValidityState& other) const
//    {
//    return ( (m_childModelCount == other.m_childModelCount) && (m_sumOfChangeCounts == other.m_sumOfChangeCounts) && (m_latestFileTime == other.m_latestFileTime) );
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void CacheValidityState::CopyFrom (const CacheValidityState& source)
//    {
//    m_newerElements         = source.m_newerElements;
//    m_differentElementSet   = source.m_differentElementSet;
//    m_attachmentChanged     = source.m_attachmentChanged;
//    m_missingReferences     = source.m_missingReferences;
//    m_childModelCount       = source.m_childModelCount;
//    m_sumOfChangeCounts     = source.m_sumOfChangeCounts;
//    m_latestFileTime        = source.m_latestFileTime;
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//double          VisibleEdgeCacheImpl::_GetRootUnitScaleFactor () const                            { return m_rootUnitScaleFactor; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyCacheStatus   VisibleEdgeCacheImpl::_GetCacheStatusForViewport (ViewportP viewport) const
//    {
//    if (m_obsoleteVersion)
//        return ProxyCacheStatus::ObsoleteVersion;
//
//    if (NULL != viewport && m_rootModel->GetRoot() != viewport->GetRootModel())
//        return ProxyCacheStatus::NotUsedInView;
//
//    if (NULL == m_rootModel->GetDgnModelP())
//        {
//        if (NULL != m_rootModel->AsDgnAttachmentCP() && m_rootModel->AsDgnAttachmentCP()->IsStaticProxy())
//            return ProxyCacheStatus::Disconnected;
//        else if (DgnAttachment::GetLoadCveAttachmentsDisabled())
//            return ProxyCacheStatus::ReferenceLoadDisabled;
//        else
//            return ProxyCacheStatus::ReferenceNotFound;
//        }
//
//    // if the ProxyStyleCache has an m_hashValue that is all zeroes, that indicates that it is to be used regardless of whether it is valid or not,
//    //  and the interrogation of whether it is valid or not should always return "UpToDate". That setting is used when we use cached visible edges in an iModel.
//    if (IsEmptyHash (m_hashValue))
//        return ProxyCacheStatus::UpToDate;
//
//    if (m_obsoleteHash)
//        return ProxyCacheStatus::ObsoleteValidityHash;
//
//    // calculate the current state.
//    CacheValidityState       thisState (m_rootModel);
//    CacheValidityState*      validityState = &m_validityState[MAX (0, (NULL == viewport ? m_originatingView : viewport->GetViewNumber()))];
//
//    if ( !validityState->Equals (thisState))
//        {
//        // something is different, we have to recalculate.
//
//        // save the current "state" information.
//        validityState->CopyFrom (thisState);
//
//        // calculate validity.
//        this->TestValid (validityState->m_newerElements, validityState->m_differentElementSet, validityState->m_attachmentChanged, validityState->m_missingReferences, m_rootModel, viewport);
//        }
//
//    if (validityState->m_missingReferences)
//        return ProxyCacheStatus::MissingReferences;
//
//    if (validityState->m_attachmentChanged)
//        return  ProxyCacheStatus::AttachmentChanged;
//
//    if (validityState->m_newerElements)
//        return ProxyCacheStatus::ModelChanged;
//
//    if (validityState->m_differentElementSet)
//        return ProxyCacheStatus::ViewChanged;
//
//    return ProxyCacheStatus::UpToDate;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void            VisibleEdgeCacheImpl::_ClearValidityState()
//    {
//    for (UInt32 iView=0; iView < MAX_VIEWS; iView++)
//        m_validityState[iView].Clear();
//    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void    VisibleEdgeCacheImpl::_ComputeHash (DgnModelP modelRef, ViewportP viewport)
//    {
//    if (NULL != viewport && NULL != modelRef && NULL != modelRef->AsDgnAttachmentP())
//        {
//        CachedVisibleEdgeHashParams hashParams (m_hashValue, m_newestElement, *viewport, m_options, 0.0, *getCryptographer());
//
//        T_HOST.GetDgnAttachmentAdmin()._ComputeCachedVisibleEdgeHash (hashParams, *modelRef->AsDgnAttachmentP());
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//void   VisibleEdgeCacheImpl::_ClearElementModifiedTimes (bool doClear)
//    {
//    m_rootProxyModel->ClearElementModifiedTimes (doClear);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   VisibleEdgeCacheImpl::_SetAlwaysValid (DgnModelP modelRef, ElementRefP elementRef)
//    {
//    if (NULL == modelRef ||
//        (NULL == elementRef && NULL == (elementRef = findAttachmentElementRef (modelRef))))
//        {
//        BeAssert (false);
//        return ERROR;
//        }
//    // to indicate that a cache is always valid, we zero out its entire hash member.
//    memset ((void*) &m_hashValue, 0, sizeof (m_hashValue));
//    _ClearElementModifiedTimes (false);             // Clear all element modified times so these are also considered "Always Valid".
//    return Save (modelRef, elementRef);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            VisibleEdgeCacheImpl::_GetAlwaysValid () const
//    {
//    return (IsEmptyHash (m_hashValue));
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       VisibleEdgeCacheImpl::_SetAlwaysInvalid (DgnModelP modelRef, ElementRefP elementRef)
//    {
//    if (NULL == modelRef ||
//        (NULL == elementRef && NULL == (elementRef = findAttachmentElementRef (modelRef))))
//        {
//        BeAssert (false);
//        return ERROR;
//        }
//
//    // to indicate that a cache is always valid, we zero out its entire hash member.
//    memset ((void*) &m_hashValue, 0, sizeof (m_hashValue));
//    m_hashValue.isComputed = 1;
//    _ClearElementModifiedTimes (true);             // Clear the element modified times (only for elements currently in synch).
//    return Save (modelRef, elementRef);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            VisibleEdgeCacheImpl::_GetAlwaysInvalid () const
//    {
//    if (m_hashValue.isComputed != 1 ||
//        m_hashValue.algid != 0 ||
//        m_hashValue.len   != 0)
//        return false;
//
//    return true;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
//void            VisibleEdgeCacheImpl::_SetValidForView (UInt32 viewIndex, bool state)
//    {
//    if (state)
//        {
//        CacheValidityState      currentState (m_rootModel);
//
//        currentState.m_newerElements        = false;
//        currentState.m_differentElementSet  = false;
//        currentState.m_attachmentChanged    = false;
//        currentState.m_missingReferences    = false;
//        m_validityState[viewIndex].CopyFrom (currentState);
//        }
//    else
//        {
//        // clear it (setting it to be calculated the first time it's used).
//        CacheValidityState      blankState;
//
//        m_validityState[viewIndex].CopyFrom (blankState);
//        }
//    }

///*=================================================================================**//**
//* @bsiclass                                                     BrandonBohrer   06/2011
//+===============+===============+===============+===============+===============+======*/
//struct CachedVisibleEdgeHandlerKey : public DisplayStyleHandlerKey
//{
//private:
//
//    DgnModelR    m_modelRef;
//
//public:
//
//    CachedVisibleEdgeHandlerKey (DgnModelR modelRef, DisplayStyleHandlerCR handler) : DisplayStyleHandlerKey (handler), m_modelRef (modelRef) { }
//
//    static  DisplayStyleHandlerKeyPtr  Create (DgnModelR modelRef, DisplayStyleHandlerCR handler) { return new CachedVisibleEdgeHandlerKey (modelRef, handler); }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual bool    Matches (DisplayStyleHandlerKey const& other) const override
//    {
//    if (!DisplayStyleHandlerKey::Matches (other))
//        return false;
//
//    CachedVisibleEdgeHandlerKey const*    otherKey = static_cast <CachedVisibleEdgeHandlerKey const*> (&other);
//
//    return  &m_modelRef == &otherKey->m_modelRef;
//    }
//};
//
//
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      06/2011
//+===============+===============+===============+===============+===============+======*/
//struct  CachedVisibleEdgeHandler  :  DisplayStyleHandler, ProxyDgnAttachmentHandler
//{
//
//    mutable ProxyDisplayHitInfo      m_currentHitInfo;
//
//    virtual ~CachedVisibleEdgeHandler () { }
//    virtual XAttributeHandlerId    _GetHandlerId () const { return CachedVisibleEdgeHandlerId(); }
//
//    virtual bool _HandleChildDisplay (DisplayStyleHandlerSettingsP) const override                       { return true; }
//    virtual bool _DoOcclusionSorting (ViewContextR context, DgnModelR modelRef) const override    { return false; }
//    virtual bool _RequiresInitializeRendering (DgnModelR modelRef) const                              { return false; }
//    virtual WString _GetName () const override     { return  g_dgnHandlersResources->GetString (MSGID_CachedVisibleEdgesName);  }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual  ProxyDisplayCacheBaseP   _RestoreProxyCache (EditElementHandleR eh, DgnAttachmentR attachment) const override
//    {
//    return VisibleEdgeCacheImpl::Restore (eh, &attachment);
//
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//DisplayStyleHandlerKeyPtr  _GetCacheKey (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const
//    {
//    return CachedVisibleEdgeHandlerKey::Create (modelRef, *this);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool   DrawElementProxy (ElementHandleCR el, ViewContextR context)
//    {
//    if (!VisibleEdgeCacheImpl::IsValidForViewContext (context, false) ||
//        NULL != dynamic_cast <ProxyElemRef*> (el.GetElementRef()))
//        return false;
//
//    ElementRefP      elementRef;
//    DisplayPath     const*  displayPath = context.GetCurrDisplayPath();
//
//#ifdef NDEBUG_NOTNOW
//    ElementId       elementId = el.GetElementCP()->ehdr.uniqueId;
//    ElementId       rootId    = (NULL == displayPath || 0 == displayPath->GetCount()) ? 0 : displayPath->GetPathElem(0)->GetElementId();
//
//    static ElementId    s_debugElementId = 0;
//
//    if (s_debugElementId != 0 && rootId != s_debugElementId)
//        return true;
//#endif
//
//    if (NULL == displayPath ||
//        0 == displayPath->GetCount() ||
//        NULL == (elementRef  = displayPath->GetTailElem()) ||
//        elementRef != el.GetElementRef() ||                                       // Compound Elements are not persistent (and not pushed on display path).
//        (elementRef->GetRefType() != ELEMENT_REF_TYPE_Persistent && elementRef->GetRefType() != ELEMENT_REF_TYPE_ProxyDisplay))
//        return false;
//
//    DgnModelP            modelRef   = displayPath->GetRoot();
//    ProxyDisplayCacheBaseP  proxyCache;
//
//    if (NULL != modelRef &&
//        NULL != modelRef->AsDgnAttachmentP() &&
//        NULL != modelRef->AsDgnAttachmentP()->FindProxyHandler (&proxyCache, NULL))
//        {
//        ((VisibleEdgeCacheImplP) proxyCache)->Draw (*displayPath, context, ViewHandlerPass (context), m_currentHitInfo.m_viewHandlerPass);
//        }
//
//    return true;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual void    _PushStyle (DisplayStyleHandlerSettingsP settings, ViewContextR context) const override
//    {
//    context.SetFilterLODFlag (FILTER_LOD_Off);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual StatusInt   _VisitPath (DisplayPathCP path, ViewContextR context) const override
//    {
//   ProxyDisplayCacheBaseP      proxyCache;
//   DgnModelP                modelRef = path->GetRoot();
//   HitPathCP                   hitPath;
//
//
//    if ((DrawPurpose::Flash == context.GetDrawPurpose()  || DrawPurpose::Hilite == context.GetDrawPurpose()) &&
//        VisibleEdgeCacheImpl::IsValidForViewContext (context, true) &&
//        NULL != (hitPath = dynamic_cast <HitPathCP> (path)) &&
//        NULL != modelRef &&
//        NULL != modelRef->AsDgnAttachmentP() &&
//        NULL != modelRef->AsDgnAttachmentP()->FindProxyHandler (&proxyCache, NULL))
//        {
//        return ((VisibleEdgeCacheImplP) proxyCache)->VisitHitPath (*hitPath, context) ? SUCCESS : ERROR;
//        }
//    return ERROR;
//    }
//
//virtual bool   _DrawElement (ElementHandleCR el, ViewContextR context) const override      { return        const_cast <CachedVisibleEdgeHandler*> (this)->DrawElementProxy (el, context); }
//virtual bool   _DrawElementCut (ElementHandleCR el, ViewContextR context) const override   { return        const_cast <CachedVisibleEdgeHandler*> (this)->DrawElementProxy (el, context); }
//virtual struct IViewHandlerHitInfo*   _GetViewHandlerHitInfo (DisplayStyleHandlerSettingsCP settings,       DPoint3dCR hitPoint) const override { return new ProxyDisplayHitInfo (m_currentHitInfo); }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual bool  _DrawDgnModel (ViewContextR context, DgnModelR modelRef, DgnModelListP includeList, bool useUpdateSequence, bool includeRefs) const override
//    {
//    ProxyDisplayCacheBaseP     proxyCache;
//
//    if (VisibleEdgeCacheImpl::IsValidForViewContext (context, true) &&
//        NULL != modelRef.AsDgnAttachmentP() &&
//        NULL != modelRef.AsDgnAttachmentP()->FindProxyHandler (&proxyCache, NULL))
//        {
//        return ((VisibleEdgeCacheImplP) proxyCache)->DrawDgnModel (context, modelRef, includeList, includeRefs, m_currentHitInfo.m_viewHandlerPass);
//        }
//
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual void    _Push (ViewContextR context, DgnAttachmentCR dgnAttachment) const
//    {
//    if (NULL != dgnAttachment.FindProxyHandler (NULL, context.GetViewport()))
//        {
//        DisplayStylePtr     wireFrameStyle = DisplayStyle::CreateHardCodedDefault (MSRenderMode::Wireframe, *dgnAttachment.GetDgnFileP());
//
//        wireFrameStyle->GetOverridesR().SetDisplayStyleHandlerP (this);
//        wireFrameStyle->GetOverridesR().m_flags.m_useDisplayHandler = true;
//        context.PushDisplayStyle (wireFrameStyle.get(),  const_cast <DgnAttachmentP> (&dgnAttachment), false);
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual DgnModelP    _GetSymbologyDgnModel (ViewContextR context) const
//    {
//    DgnModelP               modelRef = context.GetCurrentModel();
//    ProxyDisplayCacheBaseP     proxyCache;
//
//    if (NULL != modelRef &&
//        NULL != modelRef->AsDgnAttachmentP() &&
//        NULL != modelRef->AsDgnAttachmentP()->FindProxyHandler (&proxyCache, NULL))
//        {
//        return proxyCache->GetParentDgnModelP();
//        }
//
//    return modelRef;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual StatusInt   _GetReferenceRange (DRange3dR range, DgnAttachmentCR ref) const override
//    {
//    DRange3d               cacheRange;
//    Transform              parentTransform;
//    ProxyDisplayCacheBaseP proxyCache;
//
//    if (NULL == (proxyCache = ((DgnAttachmentR) ref).GetProxyCache()) || SUCCESS != proxyCache->GetRange (cacheRange))
//        return ERROR;
//
//    ((DgnAttachmentR) ref).GetTransformToParent (parentTransform, true);
//    parentTransform.multiply (&range, &cacheRange);
//
//    return SUCCESS;
//    }
//
//};  //  CachedVisibleEdgeHandler
//
//DisplayStyleHandlerCP  VisibleEdgeCache::GetDisplayStyleHandler () { return s_handler; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void       ProxyGraphics::Draw (ElementHandleCR el, ViewContextR context, VisibleEdgeCacheImplR proxyCache, ProxySymbologyOverride symbologyOverride)
    {
    UInt32          qvIndex = 0;
    IViewOutputP    viewOutput = NULL;
    bool            disableZ = NULL != context.GetViewport() &&                      // Disable ZBuffer if we are going to draw both filled cut and wires.
                               NULL != (viewOutput = context.GetViewport()->GetIViewOutput()) &&
                               !context.GetViewport()->Is3dView() &&                     // added in graphite - never turn off z buffer in a 3d view.
                               m_pieces.find (ProxyGraphicsType_CutFill) != m_pieces.end() &&
                               m_pieces.find (ProxyGraphicsType_Cut) != m_pieces.end();

    for (T_ProxyGraphicsMap::iterator curr = m_pieces.begin(), end = m_pieces.end();  curr != end; curr++)
        {
        //if (DrawPurpose::Pick == context.GetDrawPurpose())
        //     VisibleEdgeCache::s_handler ->m_currentHitInfo.Init (context, proxyCache, curr->first);

        if (disableZ && curr->first == ProxyGraphicsType_Cut)
            {
            viewOutput->EnableZTesting (false);
            Draw (curr, el, qvIndex++, context, proxyCache, symbologyOverride);
            viewOutput->EnableZTesting (true);
            }
        else
            {
            Draw (curr, el, qvIndex++, context, proxyCache, symbologyOverride);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void       ProxyGraphics::Draw (T_ProxyGraphicsMap::iterator it, ElementHandleCR el, UInt32 qvIndex, ViewContextR context, VisibleEdgeCacheImplR proxyCache, ProxySymbologyOverride symbologyOverride)
    {
    OvrMatSymbP             overrideMatSymb = context.GetOverrideMatSymb();

    switch (symbologyOverride)
        {
        case ProxySymbologyOverride_Concealed:
            {
            static  UInt32      s_concealedColor = 0xffff00;
            static  UInt32      s_concealedWidth = 4;

            overrideMatSymb->SetLineColorTBGR (s_concealedColor);
            overrideMatSymb->SetFillColorTBGR (s_concealedColor);
            overrideMatSymb->SetWidth (s_concealedWidth);

            break;
            }

        case ProxySymbologyOverride_NotConcealed:
            {
            static                  UInt32 s_dimmedColor = 0x00808080;

            overrideMatSymb->SetFillColorTBGR (s_dimmedColor);
            overrideMatSymb->SetLineColorTBGR (s_dimmedColor);
            break;
            }
        }


    //switch (context.GetDrawPurpose())
        //{
        //case DrawPurpose::CaptureGeometry:
        //    it->second->VisitAsElement (context, el, qvIndex, proxyCache, it->first);
        //    break;
        //
        //case DrawPurpose::Plot:
        //    if (!it->second->DoFilterPlot())                          // Level Table may not be present.
        //        it->second->VisitAsElement (context, el, qvIndex, proxyCache, it->first);
        //
        //    break;
        //
        //default:
            it->second->Draw (context, el, qvIndex, proxyCache, it->first);
        //    break;
        //}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//CachedVisibleEdgeHandler*  VisibleEdgeCache::s_handler = new CachedVisibleEdgeHandler ();



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            VisibleEdgeCache::IsDisplayedByProxy (DgnModelP modelRef, ViewportR viewport)
//    {
//    return NULL != modelRef->AsDgnAttachmentP() && NULL != modelRef->AsDgnAttachmentP()->FindProxyHandler (NULL, &viewport);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//BitMaskCP       VisibleEdgeCache::GetLevelUsage (DgnModelP modelRef)
//    {
//    VisibleEdgeCacheImplP      cache;
//
//    for (; NULL != modelRef;  modelRef = modelRef->GetParentDgnModelP())
//        if (NULL != modelRef->AsDgnAttachmentP() &&
//            NULL != (cache = dynamic_cast <VisibleEdgeCacheImplP> (modelRef->AsDgnAttachmentP()->GetProxyCache())))
//            return  cache->GetLevelUsage (modelRef);
//
//    return NULL;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            VisibleEdgeCache::IsLevelUsedInProxyCache (DgnModelP modelRef, LevelId level)
//    {
//    VisibleEdgeCacheImplP      cache;
//
//    for (; NULL != modelRef;  modelRef = modelRef->GetParentDgnModelP())
//        if (NULL != modelRef->AsDgnAttachmentP() &&
//            NULL != (cache = dynamic_cast <VisibleEdgeCacheImplP> (modelRef->AsDgnAttachmentP()->GetProxyCache())) &&
//            NULL != cache->GetLevelName (modelRef, level))
//            return true;
//
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayHitInfo::ProxyDisplayHitInfo ()    { }
//ProxyDisplayHitInfo::~ProxyDisplayHitInfo ()   { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayHitInfo::ProxyDisplayHitInfo (ProxyDisplayHitInfo const& rhs) :
//    m_viewport (rhs.m_viewport),
//    m_stateStack (rhs.m_stateStack),
//    m_viewHandlerPass (rhs.m_viewHandlerPass),
//    m_proxyRootRef (rhs.m_proxyRootRef),
//    m_graphicsType (rhs.m_graphicsType)
//    {
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
//SnapStatus  ProxyDisplayHitInfo::_OnSnap (SnapContextR context) const   { return context.DoDefaultDisplayableSnap (0); }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      09/2011
+===============+===============+===============+===============+===============+======*/
//struct ProxyGeneratorIdVector :T_ProxyElementIdVector
//{
//
//    ProxyGeneratorIdVector () { }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//ProxyGeneratorIdVector (HitPathCR path)
//    {
//    for (int i=0; i<path.GetCount(); i++)
//        push_back (path.GetPathElem(i)->GetElementId());
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//ProxyGeneratorIdVector (DgnModelP sourceModel, DgnModelP baseModel)
//    {
//    for (; sourceModel != baseModel && NULL != sourceModel; sourceModel = sourceModel->GetParentDgnModelP())
//        if (NULL != sourceModel->AsDgnAttachmentP())
//            push_back (sourceModel->AsDgnAttachmentP()->GetElementId());
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//DgnModelP    GetChildDgnModel (DgnModelP modelRef, size_t index)
//    {
//    if (index == size())
//        return modelRef;
//
//    DgnModelP    child;
//    return  (NULL == (child = ProxyRestoreUtil::FindModelChild (modelRef, at (size() - index - 1)))) ? NULL : GetChildDgnModel (child, index+1);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//void        Store (DataExternalizer& sink) const
//    {
//    sink.put ((UInt32) size());
//    for (size_t i=0; i<size(); i++)
//        sink.put (at(i));
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   Load (DataLoader& source)
//    {
//    UInt32  count;
//    source.get (&count);
//    for (UInt32 i=0; i<count; i++)
//        {
//        ElementId       id;
//
//        source.get (&id);
//        push_back(id);
//        }
//    return SUCCESS;
//    }
//
//}; // ProxyGeneratorIdVector

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//struct ProxyDisplayGeneratorInstance
//{
//
//private:
//    enum            {  VersionMaskShift = 24   };
//    enum    Flags   { FLAGS_None=0, FLAGS_OutOfDate=1, FLAGS_VersionMask=(0xff<<VersionMaskShift) };
//    enum { Version = 2 };
//
//    //  Persistent data
//    PersistentElementPath   m_proxyCachePEP;
//    ProxyEdgeIdData         m_edgeId;               // Identifies an edge within the section graphics generated by the cut
//    ProxyGeneratorIdVector  m_sourceAttachIds;
//    ProxyGeneratorIdVector  m_sourcePathIds;
//    UInt32                  m_segmentIndex;         // For extracting correct visible GPAs segments
//    double                  m_segmentParam;         // For extracting correct visible GPAs segments
//    UInt16                  m_pass;
//    UInt16                  m_clipPlaneIndex;
//    UInt32                  m_flags;
//    double                  m_proxyCacheTime;
//    Transform               m_refTransform;
//
//    //  Transient data
//    DgnModelP            m_homeDgnModel;
//    DgnAttachmentP          m_proxyCacheRef;
//    ViewHandlerPass         m_vhPass;
//
//    // Private methods;
//    void                        SetVersion ()                   { m_flags &= ~FLAGS_VersionMask; m_flags |= (Version<<VersionMaskShift); }
//    int                         GetVersion () const             { return (m_flags & FLAGS_VersionMask) >> VersionMaskShift; }
//
//public:
//    StatusInt                   Load (void const* data, size_t nData) {DataLoader  loader ((const byte*) data, nData); return Load (loader); }
//    StatusInt                   Load (XAttributeHandleCR xa)    { return Load (xa.PeekData(), xa.GetSize());}
//    StatusInt                   LoadAndResolve (ElementHandleCR host) { return (SUCCESS == Load (host) && SUCCESS == Resolve (host.GetDgnModelP())) ? SUCCESS : ERROR; }
//                                ProxyDisplayGeneratorInstance () : m_flags (0), m_proxyCacheRef (NULL), m_homeDgnModel (NULL), m_proxyCacheTime (0.0) { SetVersion (); }
//    void                        DisclosePointers (T_StdElementRefSet* refs, DgnModelP homeModel) { m_proxyCachePEP.DisclosePointers (refs, homeModel);};
//    DgnAttachmentP              GetProxyCacheRef () const { return m_proxyCacheRef; }
//    ProxyEdgeIdDataCR           GetCurvePrimitiveId () const { return m_edgeId; }
//    bool                        ProxyCacheNotPresent () { return NULL == m_proxyCacheRef || NULL == m_proxyCacheRef->GetProxyCache(); }
//    TransformCR                 GetRefTransform () const { return m_refTransform; }
//
//    static XAttributeHandlerId  GetXAttributeHandlerId ()                   { return  XAttributeHandlerId (XATTRIBUTEID_AssociationGenerator, (UInt32) AssociationGeneratorDataId::CVE); }
//    static XAttributeHandlerId  GetRefTransformXAttributeHandlerId ()       { return  XAttributeHandlerId (XATTRIBUTEID_AssociationGenerator, (UInt32) AssociationGeneratorDataId::RefTransform); }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayGeneratorInstance (HitPathCR hitPath, DgnAttachmentR proxyCacheRef, ProxyEdgeIdDataCR edgeId, ViewHandlerPassCR vhPass, DgnModelP homeModel) :
//        m_proxyCachePEP (proxyCacheRef.GetParentDgnModelP(), proxyCacheRef.GetElementId()),
//        m_proxyCacheRef (&proxyCacheRef),
//        m_sourceAttachIds (hitPath.GetRoot(), &proxyCacheRef),
//        m_sourcePathIds (hitPath),
//        m_homeDgnModel (homeModel),
//        m_edgeId (edgeId),
//        m_vhPass (vhPass),
//        m_flags (0)
//
//    {
//    m_proxyCacheTime = (NULL == proxyCacheRef.GetProxyCache()) ? 0.0 :  proxyCacheRef.GetProxyCache()->GetCreationTime();
//    m_segmentIndex = (UInt32) hitPath.GetGeomDetail().GetSegmentNumber();
//    m_segmentParam = hitPath.GetGeomDetail().GetCloseParam();
//
//    m_pass = (UInt16) vhPass.m_pass;
//    m_clipPlaneIndex = (UInt16) vhPass.m_clipPlaneIndex;
//
//    mdlRefFile_getTransformToParent (&m_refTransform, &proxyCacheRef, m_homeDgnModel->AsDgnAttachmentP());
//    SetVersion();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//void        Store (DataExternalizer& sink) const
//    {
//    sink.put (m_flags);
//    BeAssert (GetVersion() == Version && "we never write out data in an old format");
//    m_proxyCachePEP.Store (&sink);
//    m_sourceAttachIds.Store (sink);
//    m_sourcePathIds.Store (sink);
//    m_edgeId.Store (sink);
//    sink.put (m_segmentIndex);
//    sink.put (m_segmentParam);
//    sink.put (m_pass);
//    sink.put (m_clipPlaneIndex);
//    sink.put (m_proxyCacheTime);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   Load (DataLoader& source)
//    {
//    source.get (&m_flags);
//    if (GetVersion() != Version)
//        {
////      assert (false && "Ignoring old version CVE association instance");      // TBD. Add upgrade logic after version is released.
//        return ERROR;
//        }
//    if (SUCCESS != m_proxyCachePEP.Load (source))
//        return ERROR;
//
//    m_sourceAttachIds.Load (source);
//    m_sourcePathIds.Load (source);
//    m_edgeId.Load (source);
//    source.get (&m_segmentIndex);
//    source.get (&m_segmentParam);
//    source.get (&m_pass);
//    source.get (&m_clipPlaneIndex);
//    source.get (&m_proxyCacheTime);
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//DgnModelP    FindParentDgnModel (DgnModelP  homeModel)
//    {
//    for (DgnModelP   test = m_proxyCacheRef; NULL != test; test = test->GetParentDgnModelP())
//        if (test->GetDgnModelP() == homeModel)
//            return test;
//
//    return NULL;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       Resolve (DgnModelP homeDgnModel)
//    {
//    ElementHandle      proxyCacheAttachEh = m_proxyCachePEP.EvaluateElement (homeDgnModel);
//
//    if (!proxyCacheAttachEh.IsValid())
//        return ERROR;
//
//    if (NULL == (m_proxyCacheRef = DgnAttachment::FindByElementId (proxyCacheAttachEh.GetDgnModelP(), proxyCacheAttachEh.GetElementCP()->ehdr.uniqueId)))
//        {
//        // The modelRef passed by the dependency manager is not necessarily the correct DgnAttachment - look for the right one here.
//        for (DgnAttachmentP  referencedBy: proxyCacheAttachEh.GetDgnModelP()->GetDgnModelP()->GetReferencedBy())
//            if (NULL != (m_proxyCacheRef = DgnAttachment::FindByElementId (referencedBy, proxyCacheAttachEh.GetElementCP()->ehdr.uniqueId)))
//                break;
//        }
//    if (NULL == m_proxyCacheRef ||
//        NULL == (m_homeDgnModel = FindParentDgnModel (homeDgnModel->GetDgnModelP())))            // TR# 330109 - after merge the generator could be orphaned from the proxy cache.
//        return ERROR;
//
//    m_vhPass.m_pass = (ClipVolumePass) m_pass;
//    m_vhPass.m_clipPlaneIndex = m_clipPlaneIndex;
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       ScheduleWrite (EditElementHandle& eh)
//    {
//    DataExternalizer sink;
//    Store (sink);
//
//    return  (SUCCESS == eh.ScheduleWriteXAttribute (GetXAttributeHandlerId(), 0, sink.getBytesWritten(), sink.getBuf()) &&
//             SUCCESS == eh.ScheduleWriteXAttribute (GetRefTransformXAttributeHandlerId(), 0, sizeof (m_refTransform), &m_refTransform)) ? SUCCESS : ERROR;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       Load (ElementHandleCR sourceEh)
//    {
//    ElementHandle::XAttributeIter refXa (sourceEh, GetRefTransformXAttributeHandlerId(), 0);
//
//    if (!refXa.IsValid() || sizeof (Transform) != refXa.GetSize())
//        m_refTransform.initIdentity();
//    else
//        memcpy (&m_refTransform, refXa.PeekData(), sizeof (m_refTransform));
//
//    //  Entire state is stored in XA
//    ElementHandle::XAttributeIter xa (sourceEh, GetXAttributeHandlerId(), 0);
//    if (!xa.IsValid())
//        return ERROR;
//
//    return Load (xa.PeekData(), xa.GetSize());
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//CurveVectorPtr GetCurveVector ()
//    {
//    XGraphicsContainer          edgeGraphics;
//    ProxyDisplayCacheBaseP      proxyCache;
//
//    if (NULL == m_proxyCacheRef ||
//        NULL == (proxyCache = m_proxyCacheRef->GetProxyCache()) ||
//        SUCCESS != proxyCache->GetProxyGraphicsFromEdgeId (edgeGraphics, m_sourceAttachIds, m_sourcePathIds, m_edgeId, GPArrayParam (m_segmentIndex, m_segmentParam), m_vhPass))
//        return CurveVectorPtr ();
//
//    return  edgeGraphics.GetCurveVector();
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    RefTransformUpdated (EditElementHandleR  eh)
//    {
//    Transform       refTransform;
//    mdlRefFile_getTransformToParent (&refTransform, m_proxyCacheRef, m_homeDgnModel->AsDgnAttachmentP());
//
//    if (! m_refTransform.isEqual (&refTransform, 1.0E-6, 1.0E-3))
//        {
//        m_refTransform = refTransform;
//        ScheduleWrite (eh);
//        return true;
//        }
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    ProxyCacheUpdated (EditElementHandleR  eh)
//    {
//    double      proxyCacheTime = (NULL == m_proxyCacheRef || NULL == m_proxyCacheRef->GetProxyCache()) ? -1.0 :  m_proxyCacheRef->GetProxyCache()->GetCreationTime();
//
//    if (proxyCacheTime != m_proxyCacheTime)
//        {
//        m_proxyCacheTime = proxyCacheTime;
//        ScheduleWrite (eh);
//        return true;
//        }
//
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool    Updated (EditElementHandleR  eh)
//    {
//    return RefTransformUpdated (eh) || ProxyCacheUpdated (eh);
//    }
//
//};  // ProxyDisplayGeneratorInstance
//
///*=================================================================================**//**
//* @bsiclass                                                     Ray.Bentley     09/2011
//+===============+===============+===============+===============+===============+======*/
//struct          ProxyDisplayGeneratorHandler : DisplayHandler,
//                                               IAssocPointPathElement,
//                                               XAttributeHandler,
//                                               IXAttributePointerContainerHandler,
//                                               ICurvePathQuery,
//                                               IDependencyHandler
//{
//
//DEFINE_T_SUPER(DgnPlatform::DisplayHandler)
//ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (ProxyDisplayGeneratorHandler, PROXYELEMENTHANDLER_NOTEXPORTED)
//
//static ElementHandlerId         GetElementHandlerId () { return  ElementHandlerId (XATTRIBUTEID_AssociationGenerator, (UInt32) AssociationGeneratorHandlerId::CVE); }
//virtual bool                    _DependsOn (ElementHandleCR genEh, T_StdElementIDSet const& ids) const override { return false; }
//virtual IDependencyHandler*     _GetIDependencyHandler ()  override {return this;}
//virtual StatusInt               _OnPreprocessCopy (IReplaceXAttribute* replacer, XAttributeHandleCR xa, ElementHandleCR instanceEh, ElementCopyContextP cc) override           { return SUCCESS; }
//virtual StatusInt               _OnPreprocessCopyRemapIds (IReplaceXAttribute* replacer, XAttributeHandleCR xa, ElementHandleCR instanceEh) override                    { return SUCCESS; }
//virtual void                    _OnElementIDsChanged (XAttributeHandle& xa, ElementAndModelIdRemappingCR remapTable, ElementHandle const& instanceEh) override          {  }
//virtual void                    _OnUndoRedoRootsChanged (ElementHandle& dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) const override { }
//virtual void                    _GetTypeName (WStringR string, UInt32 desiredLength) override   { string = L"CVE Association Generator"; }
//virtual bool                    _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) {return false;}
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                   RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual BentleyStatus _GetCurveVector (ElementHandleCR generatorEh, CurveVectorPtr& curves) override
//    {
//#ifndef NDEBUG
//    ElementId   dependentId = generatorEh.GetElementRef()->GetElementId();
//
//    if (0 == dependentId)
//        return ERROR;
//#endif
//
//    ProxyDisplayGeneratorInstance generator;
//
//    if (SUCCESS != generator.LoadAndResolve (generatorEh))
//        return ERROR;
//
//    curves = generator.GetCurveVector ();
//
//    return (curves.IsValid () ? SUCCESS : ERROR);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Brien.Bastings                  11/2009
//+---------------+---------------+---------------+---------------+---------------+------*/
//virtual BentleyStatus   _GetPathTransform (ElementHandleCR eh, TransformR trans) const override
//    {
//    ProxyDisplayGeneratorInstance   generator;
//
//    if (SUCCESS != generator.LoadAndResolve (eh))
//        return ERROR;
//
//    trans = generator.GetRefTransform();
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual void   _DisclosePointers (T_StdElementRefSet* refs, XAttributeHandle const& xa, DgnModelP homeModel) override
//    {
//    ProxyDisplayGeneratorInstance instance;
//
//    if (SUCCESS == instance.Load (xa))
//        instance.DisclosePointers (refs, homeModel);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Sam.Wilson      06/2008
//+---------------+---------------+---------------+---------------+---------------+------*/
//static bool    findChangeType (bvector<IDependencyHandler::RootChange> const&  rootsChanged, IDependencyHandler::ChangeStatus change)
//    {
//    for (bvector<IDependencyHandler::RootChange>::const_iterator it = rootsChanged.begin(); it != rootsChanged.end(); ++it)
//        if (it->changeStatus == change)
//            return true;
//
//    return false;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//virtual void   _OnRootsChanged (ElementHandle& dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) override
//    {
//    EditElementHandle generatorEh (dependent.GetElementRef(), dependent.GetDgnModelP());
//
//#ifndef NDEBUG
//    ElementId   dependentId = dependent.GetElementRef()->GetElementId();
//
//    if (0 == dependentId)
//        return;
//#endif
//
//    if (findChangeType (rootsChanged, CHANGESTATUS_Deleted))
//        {
//        if (!generatorEh.GetElementRef()->IsDeletedAny())
//            generatorEh.DeleteFromModel ();              // The reference file was deleted.
//        }
//    else
//        {
//        ProxyDisplayGeneratorInstance   generator;
//
//        if (SUCCESS == generator.LoadAndResolve (generatorEh) &&  generator.Updated (generatorEh))
//            generatorEh.ReplaceInModel (generatorEh.GetElementRef());
//        }
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//static StatusInt   CreateInstance (EditElementHandleR eeh, DgnAttachmentR proxyCacheRef, DgnModelP homeModel, HitPathCR hitPath, ProxyEdgeIdDataCR edgeId, ViewHandlerPass vhPass)
//    {
//    ExtendedElementHandler::InitializeElement (eeh, NULL, homeModel, homeModel->Is3d ());
//
//    eeh.GetElementP ()->IsInvisible() = true;
//
//
//    //  Append this handler.
//    ElementHandlerXAttribute exattr (GetElementHandlerId(), MISSING_HANDLER_PERMISSION_None);
//    ElementHandlerManager::AddHandlerToElement (eeh, exattr);
//
//    //  Append recipe data
//    ProxyDisplayGeneratorInstance (hitPath, proxyCacheRef, edgeId, vhPass, homeModel).ScheduleWrite (eeh);
//
//    return SUCCESS;
//    }
//
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      11/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//void   _OnElementLoaded (ElementHandleCR generatorEh) override
//    {
//    ProxyDisplayGeneratorInstance   generator;
//
//    // If the proxy cache no longer exists send rootChanged message so that dimensions display as broken.
//    if (SUCCESS != generator.LoadAndResolve (generatorEh) || generator.ProxyCacheNotPresent ())
//        DependencyManager::RootChanged (generatorEh.GetElementRef());
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//static void    RegisterHandler ()
//    {
//    ElementHandlerManager::RegisterHandler (ProxyDisplayGeneratorHandler::GetElementHandlerId(), ELEMENTHANDLER_INSTANCE (ProxyDisplayGeneratorHandler));
//    XAttributeHandlerManager::RegisterHandler (ProxyDisplayGeneratorInstance::GetXAttributeHandlerId(), &ELEMENTHANDLER_INSTANCE (ProxyDisplayGeneratorHandler));
//    XAttributeHandlerManager::RegisterPointerContainerHandler (ProxyDisplayGeneratorInstance::GetXAttributeHandlerId(), &ELEMENTHANDLER_INSTANCE (ProxyDisplayGeneratorHandler));
//    }
//
//};  // ProxyDisplayGeneratorHandler
//
//ELEMENTHANDLER_DEFINE_MEMBERS(ProxyDisplayGeneratorHandler)
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      10/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//void                   VisibleEdgeCache::RegisterHandler ()
//    {
//    ProxyDgnAttachmentHandlerManager::GetManager().RegisterHandler (*(s_handler = new CachedVisibleEdgeHandler()), CachedVisibleEdgeHandlerId());
//
//    ProxyDisplayGeneratorHandler::RegisterHandler ();
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      09/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//StatusInt ProxyDisplayHitInfo::_OnCreateAssociationToSnap (HitPathR hitPath, DgnModelP homeModel) const
//    {
//    // Get the information about the edge that was picked...
//    CurvePrimitiveIdCP      curveId;
//
//    if (NULL == (curveId = hitPath.GetGeomDetail ().GetCurvePrimitiveId ()))
//        return ERROR;
//
//    bvector <UInt8>         curveIdData;
//    ProxyEdgeIdData         edgeId;
//
//    curveId->Store (curveIdData);
//    edgeId.Init (&curveIdData.front(), curveIdData.size());
//
//    //  Create (or find) a helper object that points to this computed edge.
//    EditElementHandle proxyGeneratorEh;
//
//    hitPath.SetCursorIndex (hitPath.GetCount()-1);
//    if (SUCCESS != ProxyDisplayGeneratorHandler::CreateInstance (proxyGeneratorEh, *m_proxyRootRef, homeModel, hitPath, edgeId, m_viewHandlerPass))
//        return ERROR;
//
//    if (!proxyGeneratorEh.IsPersistent () &&
//        SUCCESS != proxyGeneratorEh.AddToModel ())
//        return ERROR;
//
//    ElementHiliteState hs = hitPath.GetCursorElem()->IsHilited (hitPath.GetRoot());
//    if (hs != HILITED_None)
//        hitPath.GetCursorElem()->SetHilited (hitPath.GetRoot(), HILITED_None);
//
//    hitPath.SetPath (proxyGeneratorEh.GetElementRef(), homeModel);
//
//    return SUCCESS;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//HiddenVisibleEdgeCachePaths::HiddenVisibleEdgeCachePaths (VisibleEdgeCacheP cache)
//    {
//    if (NULL != cache)
//        cache->GetConcealedPaths (*this);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//HiddenVisibleEdgeCachePaths::~HiddenVisibleEdgeCachePaths ()
//    {
//    for (iterator curr = begin(); curr != end(); curr++)
//        delete *curr;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool    HiddenVisibleEdgeCachePaths::Contains (ProxyDisplayPathCR testPath) const
//    {
//    for (const_iterator curr = begin(); curr != end(); curr++)
//        if ((*curr)->IsSamePath (testPath))
//            return true;
//
//    return false;
//    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   08/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt   CachedVisibleEdgesViewport::Init (DgnModelP rootDgnModel, VisibleEdgeCacheP proxyCache)
//    {
//    // if we have a proxy cache, use the view group that it was created with, if we can find it
//    ElementId   viewGroupId = 0;
//    UInt32      viewIndex   = 0;   // any view.
//
//    if (NULL != proxyCache)
//        {
//        viewGroupId = proxyCache->GetOriginatingViewGroup ();
//        viewIndex   = proxyCache->GetOriginatingView();
//        }
//
//    ModelId                 modelId     = rootDgnModel->GetModelId();
//    DgnFileP                dgnFile     = rootDgnModel->GetDgnFileP();
//    ViewGroupCollectionCR   vgc         = dgnFile->GetViewGroups();
//    ElementId               preferredId = dgnFile->GetActiveViewGroupId();
//
//    ViewGroupPtr            viewGroup   = vgc.FindLastModifiedMatchingModel (preferredId, modelId, false, viewIndex);
//
//    if (!viewGroup.IsValid())
//        return ERROR;
//
//    // find lowest view
//    if (viewGroup->GetViewInfo(viewIndex).GetRootModelId() != modelId)
//        {
//        for (UInt32 iView = 0; iView < MAX_VIEWS; iView++)
//            {
//            if (modelId == viewGroup->GetViewInfo(iView).GetRootModelId())
//                {
//                viewIndex = iView;
//                break;
//                }
//            }
//        }
//
//    m_viewInfo = &viewGroup->GetViewInfoR (viewIndex);
//    m_viewPortInfo = &viewGroup->GetViewPortInfoR (viewIndex);
//
//    // we need the level masks, which are lazy-loaded as of 08.11.09
//    m_viewInfo->GetLevelMasksR();
//    m_viewInfo->SetRootModel (rootDgnModel->AsDgnModelP());
//    m_viewNumber = viewIndex;
//
//    _AllocateOutput ();
//
//#if defined (NEEDSWORK_VANCOUVER)
//    // turn off HUD Markers.
//    MstnHUDManager::GetManager().SetMarkerDisplay (*this, false);
//#endif
//
//    // set up the view from the viewInfo.
//    SetupFromViewInfo ();
//    InitViewSettings (false);
//
//    return SUCCESS;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   04/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//void       CachedVisibleEdgesViewport::_GetScreenRect (BSIRect& rect) const
//    {
//    // We have no window, so we need to make up a view rect.
//    rect.origin.x = rect.origin.y = 0;
//    rect.corner.x = 1024;
//    rect.corner.y = 768;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCalculationCache::~VisibleEdgeCalculationCache ()
//    {
//    m_currentPaths.clear();
//
//    for (HLOcclusionPathP path: m_referencedPaths)
//        delete path;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//void VisibleEdgeCacheImpl::SaveOcclusionPaths (ElementRefP elementRef, DgnModelP rootDgnModel) const
//    {
//    ProxyDataBuffer     buffer;
//
//    buffer.Write (&m_hashValue, sizeof (m_hashValue));
//
//    T_DgnModelMap       modelRefMap;
//    UInt32              modelRefIndex = 0;
//
//    // Write out the modelRef map.
//    size_t              modelRefMapLocation = buffer.Write (0);
//    buffer.Write((UInt32) m_proxyModelMap.size());
//
//    for (T_ProxyModelMap::const_iterator curr = m_proxyModelMap.begin(), end = m_proxyModelMap.end(); curr != end; curr++)
//        {
//        modelRefMap[curr->first] = modelRefIndex++;
//        T_ProxyElementIdVector attachmentIds;
//
//        getAttachmentIds (attachmentIds, curr->first, rootDgnModel);
//
//        buffer.Write ((UInt16) attachmentIds.size());                             // Attachment ID size (0 for root).
//        for (size_t i=0; i<attachmentIds.size(); i++)
//            buffer.Write (attachmentIds[i]);                                      // Attachment IDs.
//
//        DsigRawHash        refParameterHash;
//        attachmentParameterHashFromReference (refParameterHash, *curr->first->AsDgnAttachmentP());
//
//        buffer.Write (&refParameterHash, sizeof (DsigRawHash));
//        }
//
//    buffer.UpdateBytesToFollow (modelRefMapLocation);
//
//    size_t              elementMapLocation = buffer.Write (0);
//
//    buffer.Write ((UInt32) m_occlusionPaths->size());
//    for (SavedOcclusionPath path: *m_occlusionPaths)
//        {
//        buffer.Write (modelRefMap[path.m_path->m_path.m_root]);       // DgnModel index.
//        buffer.Write ((UInt16) path.m_path->m_path.m_elements.size());
//        for (ElementRefP elementRef: path.m_path->m_path.m_elements)
//            {
//            buffer.Write (elementRef->GetElementId());
//            buffer.Write (elementRef->GetLastModified());
//            }
//        buffer.Write ((UInt32) path.m_occluderIndices.size());
//        for (UInt32 occluderIndex: path.m_occluderIndices)
//            buffer.Write (occluderIndex);
//        }
//    buffer.UpdateBytesToFollow (elementMapLocation);
//    buffer.SaveCompressedToXAttributes (elementRef, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::OcclusionMap);
//    DELETE_AND_CLEAR (m_occlusionPaths);
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//DgnModelP    modelRefFromIds (bvector<ElementId> const& ids, DgnModelP rootDgnModel)
//    {
//    DgnModelP    modelRef = rootDgnModel;
//
//    for (ElementId id: ids)
//        {
//        if (NULL == (modelRef = ProxyRestoreUtil::FindModelChild (modelRef, id)))
//            return NULL;
//        }
//    return modelRef;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//void VisibleEdgeCalculationCache::Delete (DgnModelP modelRef, ElementRefP elemRef)
//    {
//    if (NULL != elemRef ||
//        NULL != (elemRef = findAttachmentElementRef (modelRef)))
//        {
//        XAttributeHandle    xAttrHandle (elemRef, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::OcclusionMap);
//
//        // This handle was used in development - This could robably could be removed at some point.
//        XAttributeHandle    legacyHandle (elemRef, XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, 22), 0);
//
//        if (legacyHandle.IsValid())
//            ITxnManager::GetCurrentTxn().DeleteXAttribute (legacyHandle);
//        }
//    }
//
///*=================================================================================**//**
//* @bsiclass                                                     RayBentley      06/2012
//+===============+===============+===============+===============+===============+======*/
//struct OcclusionLoadContext : NullContext
//{
//    OcclusionLoadContext (ViewportR viewport) { Attach (&viewport, DrawPurpose::NotSpecified); }
//    ~OcclusionLoadContext ()                  { Detach (); }
//
//
//    NullOutput  m_output;
//
//    virtual void _SetupOutputs () override {SetIViewDraw (m_output);}
//
//
//};  // OcclusionLoadContext
//
//
//typedef bmap <UInt32, SavedOcclusionPath>   T_SavedPathMap;
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCalculationCacheP VisibleEdgeCalculationCache::Load (VisibleEdgeCacheCR cache, ViewportR viewport)
//    {
//    ElementRefP     elemRef;
//
//
//    if (NULL == (elemRef = findAttachmentElementRef (cache.GetRootModel())))
//        return NULL;
//
//    ElementHandle                   eh (elemRef, cache.GetRootModel());
//    ElementHandle::XAttributeIter   xAttr (eh, CachedVisibleEdgeCacheId(), (UInt32) VisibleEdgeCacheXAttrIndex::OcclusionMap);
//
//    if (!xAttr.IsValid())
//        return NULL;
//
//    size_t                  uncompressedSize;
//
//    if (SUCCESS != CompressedXAttribute::GetUncompressedSizeFromData (xAttr.PeekData(), &uncompressedSize))
//        {
//        BeAssert (false);
//        return NULL;
//        }
//    bvector <byte>  buffer (uncompressedSize);
//
//    if (SUCCESS != CompressedXAttribute::ExtractBufferFromData (xAttr.PeekData(), xAttr.GetSize(), &buffer.front(), uncompressedSize))
//        {
//        BeAssert (false);
//        return NULL;
//        }
//
//    byte const*                               dataP = &buffer.front();
//    byte const*                               dataEndP = dataP + uncompressedSize;
//    DsigRawHash                         hashValue;
//    T_SavedPathMap                      currentPaths;
//    OcclusionLoadContext                context (viewport);
//
//    try
//        {
//        copyDsigRawHash (hashValue, dataP, dataEndP);
//
//        if (0 != CompareHash (cache.GetHashValue(), hashValue))
//            {
//            BeAssert (false);
//            return NULL;
//            }
//
//        bvector <ElementId>         ids;
//        bvector <double>            lastModified;
//        bvector <UInt32>            occluders;
//        bmap <UInt32, DgnModelP> modelRefMap;
//        UInt32                      modelRefBTF, modelRefCount, elementsBTF, elementCount;
//        byte const*                       modelDataEndP;
//        byte const*                       elementDataEndP;
//
//        if (SUCCESS != ProxyRestoreUtil::CopyData (modelRefBTF, dataP, dataEndP) ||
//            (modelDataEndP = dataP + modelRefBTF) > dataEndP)
//            return NULL;
//
//        // Read DgnModels.
//        ProxyRestoreUtil::CopyData (modelRefCount, dataP, modelDataEndP);
//        for (UInt32 i=0; i<modelRefCount; i++)
//            {
//            UInt16              idCount;
//            DgnModelP        modelRef;
//            DsigRawHash         savedRefParameterHash;
//
//            ProxyRestoreUtil::CopyData (idCount, dataP, modelDataEndP);
//            ids.resize (idCount);
//            ProxyRestoreUtil::CopyData (&ids.front(), idCount * sizeof (ElementId), dataP, modelDataEndP);
//
//            if (NULL == (modelRef = modelRefFromIds (ids, cache.GetRootModel())))
//                throw ProxyRestoreUtil::ReadError ();
//
//            copyDsigRawHash (savedRefParameterHash, dataP, modelDataEndP);
//
//
//            DsigRawHash         currentRefParameterHash;
//
//            attachmentParameterHashFromReference (currentRefParameterHash, *modelRef->AsDgnAttachmentP());
//
//            if (0 != CompareHash (currentRefParameterHash, savedRefParameterHash))
//                return NULL;
//
//            modelRefMap[i] = modelRef;
//            }
//        if (dataP != modelDataEndP)
//            throw ProxyRestoreUtil::ReadError ();
//
//        // Read Elements.
//        if (SUCCESS != ProxyRestoreUtil::CopyData (elementsBTF, dataP, dataEndP) ||
//            (elementDataEndP = dataP + elementsBTF) > dataEndP)
//            throw ProxyRestoreUtil::ReadError ();
//
//        ProxyRestoreUtil::CopyData (elementCount, dataP, elementDataEndP);
//
//        DgnModelP        currentDgnModel = NULL;
//        for (UInt32 elementIndex=0; elementIndex < elementCount; elementIndex++)
//            {
//            UInt16              idCount;
//            UInt32              modelRefIndex, occluderCount;
//
//            ProxyRestoreUtil::CopyData (modelRefIndex, dataP, elementDataEndP);
//            ProxyRestoreUtil::CopyData (idCount, dataP, elementDataEndP);
//            ids.resize (idCount);
//            lastModified.resize (idCount);
//
//            for (size_t j=0; j<idCount; j++)
//                {
//#ifdef NDEBUG
//                ProxyRestoreUtil::CopyData (ids[j], dataP, elementDataEndP);
//#else
//                ElementId id;
//                ProxyRestoreUtil::CopyData (id, dataP, elementDataEndP);
//                ids[j] = id;
//#endif
//                ProxyRestoreUtil::CopyData (lastModified[j], dataP, elementDataEndP);
//                }
//            ProxyRestoreUtil::CopyData (occluderCount, dataP, elementDataEndP);
//            occluders.resize (occluderCount);
//
//            for (size_t j=0; j<occluderCount; j++)
//                {
//                UInt32              occluder;
//
//                ProxyRestoreUtil::CopyData (occluder, dataP, elementDataEndP);
//                occluders[j] = occluder;
//                }
//
//            bmap <UInt32, DgnModelP>::iterator foundDgnModel = modelRefMap.find (modelRefIndex);
//            DgnModelP               dgnModel;
//            bvector <ElementRefP>   elementRefs;
//
//            if (foundDgnModel == modelRefMap.end() ||
//                NULL == (dgnModel = foundDgnModel->second->GetDgnModelP()))
//                continue;
//
//            if (currentDgnModel != foundDgnModel->second)
//                context.OnNewDgnModel (currentDgnModel = foundDgnModel->second);
//
//            for (size_t j=0; j<idCount; j++)
//                {
//
//                PersistentElementRefP       elementRef;
//
//                if (NULL == (elementRef = dgnModel->FindByElementId (ids[j])) ||
//                    elementRef->IsDeletedAny () ||
//                    elementRef->GetLastModified() != lastModified[j])
//                    break;
//
//                ElementHandle       eh (elementRef, foundDgnModel->second);
//                DisplayHandlerP     displayHandler;
//                if (NULL == (displayHandler = eh.GetDisplayHandler ()) || !displayHandler->IsVisible (eh, context, false, true, true))
//                    break;
//
//                elementRefs.push_back (elementRef);
//                }
//            if (elementRefs.size() == idCount)
//                currentPaths[elementIndex] = SavedOcclusionPath (foundDgnModel->second, elementRefs, occluders);
//            }
//        }
//
//    catch (ProxyRestoreUtil::ReadError)
//        {
//        return NULL;
//        }
//
//    VisibleEdgeCalculationCacheP  calculationCache = new VisibleEdgeCalculationCache (cache);
//
//    typedef bpair <UInt32, SavedOcclusionPath> T_PathPair;
//    size_t      debugIndex = 0;
//
//    for (T_SavedPathMap::iterator curr = currentPaths.begin(), end = currentPaths.end(); curr != end; curr++, debugIndex++)
//        {
//        HLOcclusionPath    path;
//
//        for (UInt32 occluder: curr->second.m_occluderIndices)
//            {
//            T_SavedPathMap::iterator found = currentPaths.find (occluder);
//
//            if (found == currentPaths.end())
//                break;
//
//            curr->second.m_path->AddOccluder (found->second.m_path);
//            }
//        if (curr->second.m_occluderIndices.size() == curr->second.m_path->m_occluders.size())
//            {
//            calculationCache->m_currentPaths.insert (curr->second.m_path);
//            calculationCache->m_referencedPaths.insert (curr->second.m_path);
//            for (HLOcclusionPathCP occlusionPath: curr->second.m_path->m_occluders)
//                calculationCache->m_referencedPaths.insert (const_cast <HLOcclusionPathP> (occlusionPath));
//            }
//        else
//            {
//            curr->second.m_path->m_occluders.clear();
//            }
//        }
//    // Free unused paths.
//    for (T_SavedPathMap::iterator curr = currentPaths.begin(), end = currentPaths.end(); curr != end; curr++)
//        if (calculationCache->m_referencedPaths.find (curr->second.m_path) == calculationCache->m_referencedPaths.end())
//            delete (curr->second.m_path);
//
//    return calculationCache;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathCP    VisibleEdgeCalculationCache::GetPrecalculatedOcclusionPath (HLOcclusionPathCR path) const
//    {
//    return m_currentPaths.GetPath (path);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//VisibleEdgeCacheElementCP  VisibleEdgeCalculationCache::GetCacheElement (DisplayPath const& displayPath, ViewHandlerPassCR vhPass) const
//    {
//    return m_visibleEdgeCache.GetCacheElement (displayPath, vhPass);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathSet::~HLOcclusionPathSet ()
//    {
//    for (HLOcclusionPathSet::iterator curr = begin(), pEnd = end(); curr != pEnd; curr++)
//        delete *curr;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathCP   HLOcclusionPathSet::GetPath (DisplayPathCR displayPath) const
//    {
//    return GetPath (HLOcclusionPath (displayPath));
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathCP   HLOcclusionPathSet::GetPath (HLOcclusionPathCR occlusionPath) const
//    {
//    HLOcclusionPathSet::const_iterator   found = find (const_cast <HLOcclusionPathP> (&occlusionPath));
//
//    return found == end() ? NULL : *found;
//    }
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathP   HLOcclusionPathSet::GetOrInsertPath (DisplayPathCR displayPath)
//    {
//    return GetOrInsertPath (HLOcclusionPath (displayPath));
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//HLOcclusionPathP   HLOcclusionPathSet::GetOrInsertPath (HLOcclusionPathCR occlusionPath)
//    {
//    HLOcclusionPathSet::iterator   found = find (const_cast <HLOcclusionPathP> (&occlusionPath));
//
//    if (found != end())
//        return *found;
//
//    HLOcclusionPathP newPath;
//
//    insert (newPath = new HLOcclusionPath (occlusionPath.m_path));   // Note. Never copy occluders. These are always handled explicitly.
//
//    return newPath;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool HLDisplayPath::Matches (DisplayPathCR displayPath) const
//    {
//    if (m_root != displayPath.GetRoot() || m_elements.size() != displayPath.GetCount())
//        return false;
//
//    for (int i=0; i < displayPath.GetCount(); i++)
//        if (m_elements[i] != displayPath.GetPathElem (i))
//            return false;
//
//    return true;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//void    HLOcclusionPath::AddOccluder (HLOcclusionPathCP occluder) const
//    {
//    if (occluder != this)
//        m_occluders.insert (occluder);
//
//    }
//
//
//
//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   11/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//void            hashRotMatrix (RotMatrixCR transform, CryptographerHash& refHashCalculator)
//    {
//    UInt32      rounded[9];
//    double      const *dP = &transform.form3d[0][0], *dPEnd = dP+9;
//    static      double   s_doubleScale = 1.0E6;
//
//    for (UInt32 *outP = rounded; dP < dPEnd; dP++, outP++)
//        *outP = (UInt32) (*dP * s_doubleScale);
//
//    refHashCalculator.hash ((const byte*) rounded, sizeof (rounded));
//    }
//

END_BENTLEY_DGNPLATFORM_NAMESPACE

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<VisibleEdgeCache> VisibleEdgeCache::Restore (ElementHandleCR dgnAttachmentElement) // added in Graphite
    {
    return std::shared_ptr<VisibleEdgeCache> (VisibleEdgeCacheImpl::Restore (dgnAttachmentElement, dgnAttachmentElement.GetDgnModelP()));
    }

//*---------------------------------------------------------------------------------**//**
// Note on units and coordinate systems.
// 
// Background:
// -----------
// "Proxy graphics" are graphics that are computed from elements and then stored.
// Proxy graphics are produced (in V8) by intersecting a physical model and its reference attachments with zero or more cut planes and a set of clip planes.
// The results are visible edges viewed in a view parallel to the front/back clip planes. The cut planes, if any, are parallel to the view plane. 
// That produces several different kinds of proxy graphics, including cut, forward, and backward graphics. 
// Proxy graphics of all kinds are 3-D geometry. They have a location in space.
// All kinds proxy graphics are wires. Cut graphics are planar. There may be multiple (bounded) cut planes. All cut planes are parallel to the view plane.
// Other kinds of proxy graphics are not planar.
// Proxy graphics of all kinds are stored in XGraphicsContainers. The symbology of the proxy graphics is cooked into the XGraphics.
// 
// What we get from V8:
// -------------------
// In V8, proxy graphics are associated with reference attachments. 
// The proxy graphics for an attachment and all of its nested attachments are captured in a "VisibleEdgeCache".  A VisibleEdgeCache contains 
// a tree of "ProxyDisplayModels" which contain the proxy graphics for the primary attachment and all of its nested attachments. That is, the 
// VisibleEdgeCache points to a root ProxyDisplayModel, and the root points to zero or more child ProxyDisplayModels, each of which can have children of its own.
// All proxy graphics computed from a given physical model are stored in a ProxyDisplayModel object. We call the physical model the "target model". 
// 
// In V8, proxy graphics are defined in the coordinate system and units of the target model.
// So, the proxy graphics contained in this ProxyDisplayModel were stored in V8 in the coordinate system and units of this target V8 model.
// The proxy graphics for any nested attachments (m_children) were stored in V8 in the coordinate systems and units of their target models.
//
// This ProxyDisplayModel contains proxy graphics computed from all elements in the target model that intersected the cut planes and/or clip planes.
// (There is one set of cut planes and clip planes (defined by a named view in V8) which is used by the entire VisibleEdgeCache. This set of planes
// produces all of the proxy graphics for all target models.)
// Proxy graphics in the V8 file are grouped by "passes", where a pass is specific to a type of proxy graphics (cut, forward, backward, etc.) and 
// a clip plane. Proxy graphics in V8 are grouped by target elementids within each pass.
// 
// All of the proxy graphics for a given target element are stored in a ProxyElement, which (indirectly) holds a set of XGraphicsContainers.
// 
// @param destModel             Where to store the proxy graphics as elements
// @param transformToProject    How to transform the graphics in THIS model to project coordinates
// @param helper                How to report target models and look up DgnDb element ids
// @param pathToParentModel     The V8 dgnattachment path to the target model
// @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyDisplayModel::CreateCveElements (DgnModelR destModel, TransformCR transformToProject, TransformCR unitsTransform, VisibleEdgeCache::ICreateCveElementsHelper& helper, bvector<UInt64> const& pathToParentModel) // added in graphite
    {
    SectionDrawingModel* drawing = dynamic_cast<SectionDrawingModel*>(&destModel);
    if (drawing == NULL)
        {
        BeAssert (false);
        return;
        }

    bvector<UInt64> pathToThisModel (pathToParentModel);
    pathToThisModel.push_back (m_attachmentId);

    // Note: The *caller* passes in 'transformToProject', the transform that converts *this* ProxyDisplayModel to project coordinates.

    auto targetModelPtr = helper.OnProcessDgnAttachment (pathToThisModel);

    for (auto passMapEntry : m_passMap)
        {
        ViewHandlerPass const& pass = passMapEntry.first;
        ProxyElementMap* elements = passMapEntry.second;
        for (auto element : elements->m_elementRefMap)
            {
            UInt64 oldTargetId = element.first;
            ProxyElementP proxy = element.second;

            // We will store the proxy graphics in a type-106 element that is specific to this pass on this target element.
            EditElementHandle eeh;
            ExtendedElementHandler::InitializeElement (eeh, NULL, destModel, /*is3d*/false);
            eeh.GetElementDescrP()->SetElementHandler (&ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler));

            eeh.GetElementP()->SetLevel(helper.FindLevelId (proxy->GetLevel(), targetModelPtr).GetValue());

            //  Store the proxy graphics in *drawing* coordinates ... but not flattened!
            proxy->ApplyTransform (transformToProject);
            ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler).StoreProxyElement (eeh, *proxy);

            //  Store a pointer to the target element
            ElementId targetId;
            helper.FindElementId (targetId, oldTargetId, targetModelPtr);
            if (targetId.IsValid()) // will be invalid in case the target model itself was not merged in, which happens if it is not shown in any physical view.
                ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler).StoreProxyTarget (eeh, targetId);

            //  Set the new type-106 element's range, and remember the true z range.
            eeh.GetElementDescrP()->ElementR().SetIs3d(true); // *** TRICKY: we want the z-range
            eeh.GetDisplayHandler()->ValidateElementRange (eeh.GetElementDescrP());
            DRange3d trueRange = eeh.GetElementDescrP()->Element().GetRange();
            eeh.GetElementDescrP()->ElementR().SetIs3d(false); // *** TRICKY: go back to being 2-D

            ProxyElementDetails details (pass, trueRange.low.z, trueRange.high.z);
            details.Store (eeh);

            DRange3d range =  trueRange;
            unitsTransform.Multiply (range.low);
            unitsTransform.Multiply (range.high);
            drawing->AddToZRange (range.low.z);                   // keep track of the model's total z range
            drawing->AddToZRange (range.high.z);

            eeh.AddToModel();                                           // (Note: AddToModel automatically zeroes out z-coordinate of range)
            }
        }

    for (auto child : m_children)
        {
        //  Tell the child model how to transform itself into project coordinates.
        Transform transformChildToProject = Transform::FromProduct (transformToProject, child->m_transformToParent);

        child->CreateCveElements (destModel, transformChildToProject, unitsTransform, helper, pathToThisModel);
        }
    }

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
void VisibleEdgeCacheImpl::_CreateCveElements (DgnModelR destModel, TransformCR unitsTransform, ICreateCveElementsHelper& helper) // added in graphite
    {
    // Note on units: When we draw the CVE elements and anntotation elements in a physical view (hypermodeling)
    //         they have to be in project units. It would be natural to apply the project unit transform to both CVE graphics
    //         and annotation elements here, but we can't. Annotations that are marked as having annotation scale
    //          can't be scaled. (And I don't know how to fight them.) To work around that, we build the project units transform into the viewing transform.

    //  Report the viewing transform.
    //  "transformToParent" means transform the proxy graphics, which are defined in the coordinate system of the V8 3D model, into the drawing's LCS.
    //  That is, this transform rotates the 3D cut graphics so that the cut plane is parallel to the viewing plane (a drawing is always viewed in "top" view) with z up.
    //  fromDrawingTo3DV8 is the INVERSE of this transform. So, it carries geometry (and annotations) in the drawing's LCS back into V8 3-space, lying in a plane coincident with the cut plane.
    Transform fromDrawingTo3DV8;
    fromDrawingTo3DV8.InverseOf (m_rootProxyModel->m_transformToParent);

    DrawingModel* drawing = dynamic_cast<DrawingModel*> (&destModel);
    if (drawing == NULL)
        {
        BeAssert(false);
        return;
        }

    helper.SetViewingTransform (Transform::FromProduct (unitsTransform, fromDrawingTo3DV8));     // TRICKY: apply V8 -> DGNDB project units conversion at draw time!

    Transform toDrawingFrom3DV8;
    toDrawingFrom3DV8.InverseOf (fromDrawingTo3DV8);

    bvector<UInt64> primaryAttachmentPathIsEmpty;
    m_rootProxyModel->CreateCveElements (destModel, toDrawingFrom3DV8, unitsTransform, helper, primaryAttachmentPathIsEmpty);
    }

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
void VisibleEdgeCache::CreateCveElements (DgnModelR destModel, TransformCR unitsTransform, ICreateCveElementsHelper& helper) {_CreateCveElements(destModel,unitsTransform,helper);} // added in graphite

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyDisplayHandlerUtils::RegisterHandlers (DgnDomain& domain) // added in graphite
    {
    domain.RegisterHandler (ProxyElementDisplayHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler));
    }

//*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      02/14
//+---------------+---------------+---------------+---------------+---------------+------*/
bool ProxyDisplayHandlerUtils::IsProxyDisplayHandler (HandlerR handler) // added in graphite
    {
    return &handler == &ELEMENTHANDLER_INSTANCE(ProxyElementDisplayHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyElement::ApplyTransform (TransformCR trans) // added in graphite
    {
    if (m_graphics != NULL)
        m_graphics->ApplyTransform (trans);
    if (m_children != NULL)
        m_children->ApplyTransform (trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyGraphics::ApplyTransform (TransformCR trans) // added in graphite
    {
    for (auto piece : m_pieces)
        piece.second->_ApplyTransform (trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyElementMap::ApplyTransform (TransformCR trans) // added in graphite
    {
    for (auto p : m_elementRefMap)
        p.second->ApplyTransform(trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyDisplayXGraphics::_ApplyTransform (TransformCR trans) // added in graphite
    {
    m_xGraphics->OnTransform (TransformInfo(trans));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId ProxyElementDisplayHandler::GetProxyTarget (ElementHandleCR el)
    {
    PersistentElementPath pep;
    PersistentElementPathXAttributeHandler::GetPersistentElementPath (pep, el, 0);
    return pep.GetFirstElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyElementDisplayHandler::StoreProxyTarget (EditElementHandleR eeh, ElementId eid)
    {
    PersistentElementPath pep (eid);
    PersistentElementPathXAttributeHandler::ScheduleWritePersistentElementPath (eeh, 0, pep, PersistentElementPath::COPYOPTION_RemapRootsWithinSelection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void  ProxyElementDetails::Store (EditElementHandleR eeh)
    {
    BeAssert(IsValid());
    Json::Value json (Json::objectValue);
    json["pass"]  = static_cast<UInt32>(m_viewHandlerPass.GetPass()); 
    json["plane"] = m_viewHandlerPass.m_clipPlaneIndex;
    json["zlow"] = m_zlow;
    json["zhigh"] = m_zhigh;
    Utf8String passDataSerialized = Json::FastWriter::ToString(json);
    eeh.ScheduleWriteXAttribute (CachedVisibleEdgeHandlerId(), PROXY_ELEMENT_DETAILS_XATTRID, passDataSerialized.size()+1, passDataSerialized.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ProxyElementDetails::Load (ElementHandleCR el)
    {
    m_valid = false;
    ElementHandle::XAttributeIter xa (el, CachedVisibleEdgeHandlerId(), PROXY_ELEMENT_DETAILS_XATTRID);
    if (!xa.IsValid())
        return BSIERROR;

    Utf8String jsonStr;
    Json::Value  jsonObj (Json::objectValue);
    Json::Reader::Parse ((char*)xa.PeekData(), jsonObj);
    ViewHandlerPass pass;
    m_viewHandlerPass.m_pass = static_cast<ClipVolumePass>(jsonObj["pass"].asUInt());
    m_viewHandlerPass.m_clipPlaneIndex = jsonObj["plane"].asUInt();
    m_zlow = jsonObj["zlow"].asDouble();
    m_zhigh = jsonObj["zhigh"].asDouble();
    m_valid = true;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void  ProxyElementDisplayHandler::StoreProxyElement (EditElementHandleR eeh, ProxyElement const& proxy)
    {
    ProxyDataBuffer buffer;
    proxy.Save (buffer);
    eeh.ScheduleWriteXAttribute (CachedVisibleEdgeHandlerId(), PROXY_ELEMENT_GRAPHICS_XATTRID, buffer.GetDataSize(), buffer.GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ProxyElementDisplayHandler::RestoreProxyElement (ProxyElement& proxy, ElementHandleCR el)
    {
    ElementHandle::XAttributeIter xa (el, CachedVisibleEdgeHandlerId(), PROXY_ELEMENT_GRAPHICS_XATTRID);
    if (!xa.IsValid())
        return BSIERROR;
    byte const* data = static_cast<byte const*>(xa.PeekData());
    return proxy.Restore (s_fakeCache, data, data+xa.GetSize(), el.GetDgnModelP(), el.GetDgnModelP()) == SUCCESS? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int ProxyGraphicsElemTopology::_Compare (IElemTopologyCR const& otherTopo) const
    {
    auto rhs = dynamic_cast<ProxyGraphicsElemTopology const*>(&otherTopo);
    if (rhs == NULL)
        return 1;
    if (m_targetElement.GetValue() > rhs->m_targetElement.GetValue())
        return 1;
    if (m_targetElement.GetValue() < rhs->m_targetElement.GetValue())
        return -1;

    if (m_pass > rhs->m_pass)
        return 1;
    if (m_pass < rhs->m_pass)
        return -1;

    return m_clipPlaneIndex - rhs->m_clipPlaneIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IElemTopology* ProxyGraphicsElemTopology::_Clone () const
    {
    auto p = new ProxyGraphicsElemTopology;
    p->m_clipPlaneIndex = m_clipPlaneIndex;
    p->m_pass = m_pass;
    p->m_targetElement = m_targetElement;
    return p;
    }

