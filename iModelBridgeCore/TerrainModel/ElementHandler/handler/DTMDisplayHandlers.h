/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDisplayHandlers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel\ElementHandler\TMElementDisplayHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

/*__PUBLISH_SECTION_END__*/
struct LazyDTMDrawingInfoProvider;
#ifdef __BENTLEY_DTM_ELEMENT_BUILD__
/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
struct LazyDTMDrawingInfoProvider
{
private:

    bool                        m_initialized;

protected:

    ElementHandleCR                m_element;
    RefCountedPtr<DTMDataRef>   m_DTMDataRef;
    DTMDrawingInfo              m_info;

    LazyDTMDrawingInfoProvider (ElementHandleCR element, DTMDataRef* DTMDataRef) : \
        m_element (element), m_DTMDataRef (DTMDataRef), m_initialized (false)
        {}

    virtual void _Init (void) abstract;

public:

    DTMDrawingInfo& Get (void)
        {
        if (!m_initialized)
            {
            _Init ();
            m_initialized = true;
            }
        return m_info;
        }
    virtual ~LazyDTMDrawingInfoProvider()
        {
        }

}; // End LazyDTMDrawingInfoProvider struct

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
struct LineStyleScaleFixer
{
private:

    double          m_orgScale;
    LineStyleSymbP  m_lsP;

public:

    explicit LineStyleScaleFixer (double multi, LineStyleSymbP lsP)
        {
        if (lsP->IsScaled())
            {
            m_lsP = lsP;
            m_orgScale = m_lsP->GetScale ();
            lsP->SetScale (m_orgScale / multi);
            }
        else
            { m_lsP = NULL; }
        }

    explicit LineStyleScaleFixer (double multi, ViewContextR context)
        {
        LineStyleSymbP lsP;

        lsP = &context.GetElemMatSymb()->GetLineStyleSymbR ();
        if (lsP)
            { new (this) LineStyleScaleFixer (multi, lsP); }
        else
            { m_lsP = NULL;}
        }

    ~LineStyleScaleFixer (void)
        {
        if (m_lsP)
            { m_lsP->SetScale (m_orgScale); }
        }

}; // End LineStyleScaleFixer struct

struct BasicDrawControler
{
protected:

    ViewContextR       m_context;
    RotMatrix           m_zeroRotation;
    Transform           m_transform;

public:

    explicit BasicDrawControler (ViewContextR context) : m_context (context)
        {
        m_zeroRotation.InitIdentity (); // mdlRMatrix_fromAngle (&m_zeroRotation, 0.);
        }

    RotMatrixCR GetZeroRotation (void) const
        {
        return m_zeroRotation;
        }

    void Push (DPoint3dCR offset, ClipVectorCP clipDescrP = NULL)
        {
        m_transform.initFrom (&m_zeroRotation, &offset);
        if (NULL != clipDescrP)
            m_context.PushClip (*clipDescrP);

        m_context.PushTransform (m_transform);
        }

    void Pop (void)
        { m_context.PopTransformClip (); }

    struct Sentinel
    {
    private:

        BasicDrawControler &m_controler;

    public:
        explicit Sentinel
        (
        BasicDrawControler& controler,
        DPoint3dCR                          offset,
        ClipVectorCP                         clipDescrP = NULL
        ) : m_controler (controler)
            { m_controler.Push (offset, clipDescrP); }

        ~Sentinel (void)
            { m_controler.Pop (); }

    }; // End Sentinel struct

} ; // End BasicDrawControler class

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
struct BasicDrawSentinel
{

private:

    ViewContextR       m_context;

public:

    explicit BasicDrawSentinel
    (
    ViewContextR   context,
    TransformCP     trans,
    ClipVectorCP      clipDescrP = NULL
    ) : m_context (context)
        { 
        if (NULL != clipDescrP)
            m_context.PushClip (*clipDescrP);

        if (NULL != trans)
            m_context.PushTransform (*trans);
        }

    explicit BasicDrawSentinel
    (
    ViewContextR   context,
    TransformCR     trans,
    ClipVectorCP      clipDescrP = NULL
    ) : m_context (context)
        { 
        if (NULL != clipDescrP)
            m_context.PushClip (*clipDescrP);

        m_context.PushTransform (trans);
        }

    explicit BasicDrawSentinel
    (
    ViewContextR           context,
    DTMDrawingInfo const&   drawingInfo,
    ClipVectorCP              clipDescrP = NULL
    ) : m_context (context)
        {
        if (NULL != clipDescrP)
            m_context.PushClip (*clipDescrP);
         
        m_context.PushTransform (*drawingInfo.GetStorageToUORTransformation());
        }

    explicit BasicDrawSentinel
    (
    ViewContextR   context,
    RotMatrixCR     rotation,
    DPoint3dCR      origin,
    ClipVectorCP      clipDescrP = NULL
    ) : m_context (context)
        {
        Transform   transform;

        transform.initFrom (&rotation, &origin);

        if (NULL != clipDescrP)
            m_context.PushClip (*clipDescrP);
            
        m_context.PushTransform (transform);
        }

    ~BasicDrawSentinel (void)
        { m_context.PopTransformClip (); }

}; // End BasicDrawSentinel struct

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
struct DrawContextTransformationAndDisplayParamsSentinel
{
private:

    LineStyleScaleFixer     m_lsFixer;
    BasicDrawSentinel       m_transfromationSentinel;

    static double GetMultiFactor (DTMDrawingInfo const& drawingInfo)
        {
        return drawingInfo.ScaleStorageToUors (1.);
        }

public:

    explicit DrawContextTransformationAndDisplayParamsSentinel
    (
    ViewContextR   context,
    TransformCP     trans,
    double          multi,
    ClipVectorCP      clipDescrP = NULL
    ) : m_lsFixer (multi, context), m_transfromationSentinel (context, trans, clipDescrP)
        {}

    explicit DrawContextTransformationAndDisplayParamsSentinel
    (
    ViewContextR   context,
    TransformCR     trans,
    double          multi,
    ClipVectorCP      clipDescrP = NULL
    ) : m_lsFixer (multi, context), m_transfromationSentinel (context, trans, clipDescrP)
        {}

    explicit DrawContextTransformationAndDisplayParamsSentinel
    (
    ViewContextR           context,
    DTMDrawingInfo const&   drawingInfo,
    ClipVectorCP              clipDescrP = NULL
    ) : m_lsFixer (GetMultiFactor(drawingInfo), context),
        m_transfromationSentinel (context, drawingInfo, clipDescrP)
        {}

};

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
//class LocalToActiveTransContext : private ViewContext
//{
//private:
//
//    ViewContext*   m_context;
//
//public:
//
//    // ToDo Look at below functions
//
//    virtual void _SetupOutputs () {}
//
//    // ToDo End
//
//    explicit LocalToActiveTransContext (ViewportP viewPort, DgnModelRefP modelRef)
//        {
//        bool const PUSH_PARENTS = true;
//
//        if (NULL == (m_context = NULL == viewPort ? NULL : this))
//            { return; }
//        Attach (viewPort, DrawPurpose::NotSpecified);
//        PushModelRef (modelRef, PUSH_PARENTS);
//        }
//
//    explicit LocalToActiveTransContext (HitPathCR hitPath)
//        {
//        ViewportP viewPort;
//        bool const PUSH_PARENTS = true;
//
//        if ( NULL == (m_context = NULL == (viewPort = hitPath.GetViewport()) ? NULL : this))
//            { return; }
//        Attach (viewPort, DrawPurpose::NotSpecified);
//        PushModelRef (hitPath.GetRoot(), PUSH_PARENTS);
//        }
//
//    ~LocalToActiveTransContext (void)
//        {
//        if (IsAttached())
//            { Detach (); }
//        }
//
//    operator ViewContext* (void) const
//        { return m_context; }
//
//}; // End LocalToActiveTransContext class

typedef DrawContextTransformationAndDisplayParamsSentinel DrawSentinel;
typedef DrawContextTransformationAndDisplayParamsSentinel FullDrawSentinel; // End DrawContextTransformationAndDisplayParamsSentinel struct 

void GetViewBoxFromContext(::DPoint3d viewBoxPts[], int nbViewBoxPts, ViewContextP context, DTMUnitsConverter& conv);
bool GetVisibleFencePointsFromContext(::DPoint3d*& fencePt, int& nbPts, ViewContextP context, DTMUnitsConverter& conv, DRange3d& dtmRange);

#endif

#define SUBDISPLAYHANDLER_DECLARE_MEMBERS(__classname__,__exporter__) \
    private:   __exporter__ static __classname__*& z_PeekInstance(); \
                            static __classname__* z_CreateInstance(); \
    public:    __exporter__ static __classname__& GetInstance() {return z_Get##__classname__##Instance();}\
                            static __classname__& ReplaceInstance() {return *(z_PeekInstance()=z_CreateInstance());}\
               __exporter__ static __classname__& z_Get##__classname__##Instance();


// This macro must be included within the source file that implements an ElementHandler
#define SUBDISPLAYHANDLER_DEFINE_MEMBERS(__classname__) \
    __classname__*  __classname__::z_CreateInstance() {__classname__* instance= new __classname__(); return instance;}\
    __classname__*& __classname__::z_PeekInstance() {static __classname__* s_instance = 0; return s_instance;}\
    __classname__&  __classname__::z_Get##__classname__##Instance(){__classname__*& instance=z_PeekInstance(); if (0 == instance) instance=z_CreateInstance(); return *instance;}

#define SUBDISPLAYHANDLER_INSTANCE(__classname__) __classname__::z_Get##__classname__##Instance()

/*__PUBLISH_SECTION_START__*/
struct DTMElementSubDisplayHandler
    {
    DTMElementSubDisplayHandler (UInt16 subHandlerId)
        {
        m_subHandlerId = subHandlerId;
        }
    virtual bool _CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const;
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) abstract;
    virtual SnapStatus _OnSnap (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& drawingInfoProvider, SnapContextP context, int snapPathIndex);
    virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr);
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context);
    virtual void _GetDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, WString& string, uint32_t desiredLength);
    
    UInt16 GetSubHandlerId() const
        {
        return m_subHandlerId;
        }

    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 04/11
    //=======================================================================================
    DTMELEMENT_EXPORT static bool CanDoPickFlash(RefCountedPtr<DTMDataRef>& dtmDataRef, DrawPurpose drawPurpose);

    DTMELEMENT_EXPORT bool CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const;
    DTMELEMENT_EXPORT bool Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context);
    DTMELEMENT_EXPORT void GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr);

    DTMELEMENT_EXPORT static void DrawSubElement (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, ViewContextR context, const DTMFenceParams& fence);
    DTMELEMENT_EXPORT static void DrawSubElement (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, ViewContextR context)
        {
        DrawSubElement (element, xAttr, context, DTMFenceParams());
        }

    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 03/11
    //=======================================================================================

    DTMELEMENT_EXPORT bool CanDraw (DTMDataRef* dtmDataRef, ViewContextCR context);

    DTMELEMENT_EXPORT void Register();
    DTMELEMENT_EXPORT static DTMElementSubDisplayHandler* FindHandler (const DTMSubElementId& id);
    DTMELEMENT_EXPORT static DTMElementSubDisplayHandler* FindHandler (const ElementHandle::XAttributeIter& xAttr);        
    DTMELEMENT_EXPORT static DTMElementSubDisplayHandler* FindHandler (XAttributeHandleCR xAttr);        
    DTMELEMENT_EXPORT static DTMElementSubDisplayHandler* FindHandler (UInt16 subHandlerId);

    DTMELEMENT_EXPORT static void SetSymbology(DTMElementSubHandler::SymbologyParams& params, DTMDrawingInfo& drawingInfo, ViewContextR context, uint32_t color, int style, uint32_t weight);
    DTMELEMENT_EXPORT static bool SetSymbology(DTMElementSubHandler::SymbologyParams& params, DTMDrawingInfo& drawingInfo, ViewContextR context);
    DTMELEMENT_EXPORT static void SetSymbology(DTMElementSubHandler::SymbologyParams& params, DTMDrawingInfo& drawingInfo, ViewContextR context, Symbology const &symbology)
        {
        SetSymbology (params, drawingInfo, context, symbology.color, symbology.style, symbology.weight);
        }

private:
    static bmap<UInt16, DTMElementSubDisplayHandler*> s_subHandlers;
    UInt16 m_subHandlerId;
    };

struct DTMElementTrianglesDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementTrianglesDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _CanDraw (DTMDataRef* dtmDataRef, ViewContextCR context) const;
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementTrianglesDisplayHandler () : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYTRIANGLES)
        {
        }
    DTMElementTrianglesDisplayHandler (UInt16 subHandlerId) : DTMElementSubDisplayHandler(subHandlerId)
        {
        }

private:
    void DrawWithTexture(BcDTMP dtm, ElementHandleCR element, ViewContextR context, RefCountedPtr<DTMDataRef>& DTMDataRef, const ElementHandle::XAttributeIter& xAttr, RefCountedPtr<DTMQvCacheDetails>& details, DTMDrawingInfo& drawingInfo);
    };

struct DTMElementRegionsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementRegionsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    DTMElementRegionsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYREGIONS)
        {
        }
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
public:
    bool _CreateDefaultElements(EditElementHandleR m_element, bool visible);
    };

struct DTMElementContoursDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementContoursDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementContoursDisplayHandler () : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYCONTOURS)
        {
        }
    };

struct DTMElementFeaturesDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementFeaturesDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;

    DTMElementFeaturesDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYFEATURES)
        {
        }
    };

struct DTMElementFeatureSpotsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementFeatureSpotsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementFeatureSpotsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYFEATURESPOTS)
        {
        }
    };

struct DTMElementFlowArrowsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementFlowArrowsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementFlowArrowsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYFLOWARROWS)
        {
        }
    };

struct DTMElementHighPointsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementHighPointsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementHighPointsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYHIGHPOINTS)
        {
        }
    };

struct DTMElementLowPointsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementLowPointsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementLowPointsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYLOWPOINTS)
        {
        }
    };

struct DTMElementRasterDrapingDisplayHandler : DTMElementTrianglesDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementRasterDrapingDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const;
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementRasterDrapingDisplayHandler() : DTMElementTrianglesDisplayHandler (ELEMENTHANDLER_DTMELEMENTDISPLAYRASTERDRAPING)
        {
        }
    };

struct DTMElementSpotsDisplayHandler : DTMElementSubDisplayHandler
    {
    DEFINE_T_SUPER(DTMElementSubDisplayHandler)
    SUBDISPLAYHANDLER_DECLARE_MEMBERS (DTMElementSpotsDisplayHandler, DTMELEMENT_EXPORT);
protected:
    virtual bool _Draw (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context) override;
    //virtual void _GetPathDescription (ElementHandleCR m_element, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    virtual void _EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context) override;
    DTMElementSpotsDisplayHandler() : DTMElementSubDisplayHandler(ELEMENTHANDLER_DTMELEMENTDISPLAYSPOTS)
        {
        }
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
