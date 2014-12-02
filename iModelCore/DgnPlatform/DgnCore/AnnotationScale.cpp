/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/AnnotationScale.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

struct  IScaledRangeProcessor;

static StatusInt computeAnnotationScaledRangeOfComponents (DRange3dR, ElementHandleCR, double, DRange3dCR, IScaledRangeProcessor*);

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      06/2008
+===============+===============+===============+===============+===============+======*/
struct     IScaledRangeProcessor
{
virtual StatusInt OnScaledRange (ElementHandleCR, DRange3dR) = 0;
}; // IScaledRangeProcessor

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      06/2008
+===============+===============+===============+===============+===============+======*/
struct    ScaledRangeSetter : IScaledRangeProcessor
{
    bool      m_componentsOnly;
    bool      m_addToRangeTree;

    ScaledRangeSetter (bool addToRangeTree, bool co) : m_addToRangeTree(addToRangeTree), m_componentsOnly(co) {;}

    virtual StatusInt OnScaledRange (ElementHandleCR eh, DRange3dR r) override;

}; // IScaledRangeProcessor

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/05
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementRefP getElementRef (MSElementDescrCP descrIn)
    {
    if (NULL != descrIn->GetElementRef())
        return descrIn->GetElementRef();

    return descrIn->GetDgnModel().FindElementById(descrIn->Element().GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementRefP    getElementRef (ElementHandleCR eh)
    {
    ElementRefP ref = eh.GetElementRef ();
    if (NULL != ref)
        return ref;
    return getElementRef (eh.GetElementDescrCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     double_equal (double v1, double v2)
    {
    return fabs (v1 - v2) < mgds_fc_nearZero;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IAnnotationHandler::ComputeAnnotationDisplayParametersOfCurrentModel
(
AnnotationDisplayParameters& parms,
ViewContextP
)
    {
    double  elementScale;
    double  refScale;
    double  displayScale;

    refScale = elementScale = displayScale = 1.0;

    Transform   aspectRatioSkew;
    aspectRatioSkew.InitIdentity();
#ifdef DGNV10FORMAT_CHANGES_WIP
    context.ComputeAspectRatioSkew (aspectRatioSkew);
#endif

    parms.Init (elementScale, refScale, displayScale, aspectRatioSkew);

    //  -----------------------------------------------
    //  See if there is any reason why this annotation element cannot be displayed at its native scale.
    return !double_equal(parms.GetRescaleFactor(), 1.0) || parms.HasAspectRatioSkew();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   08/2006
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    AnnotationScale::SetAsXAttribute (EditElementHandleR eeh, double const * annotationScaleIn)
    {
    if (!eeh.IsValid ())
        return SUCCESS;
    XAttributeHandlerId     dataHandlerId (XATTRIBUTEID_AnnotationScale, 0);

    if (NULL == annotationScaleIn)
        return eeh.ScheduleDeleteXAttribute (dataHandlerId, 0);

    return eeh.ScheduleWriteXAttribute (dataHandlerId, 0, sizeof (double), annotationScaleIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   08/2006
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    AnnotationScale::GetFromXAttribute (double * annotationScaleOut, ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return ERROR;

    double                  annotationScale = 1.0;
    XAttributeHandlerId     dataHandlerId (XATTRIBUTEID_AnnotationScale, 0x00);

    // First look for any scheduled settings. If none scheduled, look for persistent settings.
    XAttributeChangeIter    changeIter (eh, dataHandlerId, 0);

    if (changeIter.IsValid ())
        {
    if (XAttributeChange::CHANGETYPE_Delete == changeIter->GetChangeType ())
        return ERROR;

        annotationScale = * (double *) changeIter->PeekData ();
        }
    else
        {
        ElementRefP elemRef = eh.GetElementRef ();

        if (NULL == elemRef)
            return ERROR;

        XAttributeHandle  iter (elemRef, dataHandlerId, 0);

        if (!iter.IsValid() || iter.GetSize () <= 0)
            return ERROR;

        annotationScale = * (double *) iter.PeekData ();
        }

    if (fabs (annotationScale) < mgds_fc_epsilon)
        annotationScale = 1.0;

    if (NULL != annotationScaleOut)
        *annotationScaleOut = annotationScale;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::_ApplyAnnotationScaleDifferential
(
EditElementHandleR thisElm,
double          scale
)
    {
    DPoint3d org;
    thisElm.GetDisplayHandler ()->GetTransformOrigin (thisElm, org);

    RotMatrix scaling;
    scaling.initFromScale (scale);

    Transform trans;
    trans.initFromMatrixAndFixedPoint (&scaling, &org);

    TransformInfo   transInfo (trans);

    return thisElm.GetHandler().ApplyTransform (thisElm, transInfo);
    }

#ifdef DGNPROJECT_MODELS_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IAnnotationHandler::_OnPreprocessCopyAnnotationScale
(
EditElementHandleR     thisElm,
CopyContextP        ccP
)
    {
#if defined (SET_DYNAMIC_RANGE_ON_ALL_ANNOTATIONS)
    if (!thisElm.GetElementCP()->hdr.dhdr.props.b.dynamicRange)
        return SUCCESS;
#endif

    double originalScale;
    if (!GetAnnotationScale (&originalScale, thisElm))
        return SUCCESS;

    if (ccP->GetDisableAnnotationScaling())
        return SUCCESS;


#ifdef GRAPHITE_NEEDS_WORK_SHEETS
    DgnModelP sourceModel      = ccP->GetSourceDgnModel ();
    double sourceScale = dgnModel_getAnnotationScale (sourceModel);
#else
    double sourceScale = originalScale;
#endif

    //  If the element's annotation scale does not match the source model's annotation scale, then this is the legacy case of multiple annotation scales.
    //  Do not change it. The bug was that when such an annotation was copied within the same model, the new annotation reverted to the model's annotation scale.
    if (!double_equal(sourceScale, originalScale))
        return SUCCESS;

#ifdef GRAPHITE_NEEDS_WORK_SHEETS
    DgnModelP destinationModel = ccP->GetDestinationDgnModel ();
    double destScale = dgnModel_getAnnotationScale (destinationModel);
#else
    double destScale = originalScale;
#endif

    //  The annotation scale of an annotation element must match the annotation scale of the model that contains it.
    if (!double_equal(destScale, originalScale))
        {
        UpdateSpecifiedAnnotationScale (thisElm, destScale);

        //  If the handler refused to apply the new scale, stop here.
        double newScale;
        if (!GetAnnotationScale (&newScale, thisElm) || !double_equal(destScale, newScale))
            return SUCCESS;
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IAnnotationHandler::GetAnnotationScaleChange
(
bool&               newScaleFlag,
double&             newScale,
double&             scaleChange,
ElementHandleCR     elemHandle,
TransformInfoCR     trans
)
    {
    ChangeAnnotationScale *  changeContext = mdlChangeAnnotationScale_new (elemHandle.GetDgnModelP ());
    mdlChangeAnnotationScale_setAction (changeContext, trans.GetAnnotationScaleAction (), trans.GetAnnotationScale ());

    double      currScaleValue  = 1.0;
    bool        currScaleFlag = GetAnnotationScale (&currScaleValue, elemHandle);

    StatusInt status = mdlChangeAnnotationScale_getEffectiveValues (changeContext, &newScale, &newScaleFlag, &scaleChange, currScaleValue, currScaleFlag);

    mdlChangeAnnotationScale_free (&changeContext);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                    Sam.Wilson      02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IAnnotationHandler::ComputeAnnotationDisplayParameters (AnnotationDisplayParameters& parms, ElementHandleCR elHandle, ViewContextR context) const
    {
    //  -----------------------------------------------
    // Is this an annotation element? If not, there's nothing to do. If so, get its native scale.
    // -----------------------------------------------
    double      elementScale;
    if (!GetAnnotationScale (&elementScale, elHandle))
        {
        parms.Init ();
        return false;
        }

    if (elementScale <= mgds_fc_nearZero)
        elementScale = 1.0;

    //  -----------------------------------------------
    //  Get the re-scaling and distortions that will be applied to the view
    // -----------------------------------------------
    double      displayScale = 1.0;

    Transform   aspectRatioSkew;
    aspectRatioSkew.InitIdentity();
#ifdef DGNV10FORMAT_CHANGES_WIP
    context.ComputeAspectRatioSkew (aspectRatioSkew);
#endif

    parms.Init (elementScale, elementScale, displayScale, aspectRatioSkew);

    //  -----------------------------------------------
    //  See if there is any reason why this annotation element cannot be displayed at its native scale.
    // -----------------------------------------------
    return !double_equal(parms.GetRescaleFactor(), 1.0) || parms.HasAspectRatioSkew();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
// static
void    IAnnotationStrokeForCache::PushTransformToRescale
(
ViewContextR                        context,
DPoint3dCR                          originForScale,
AnnotationDisplayParameters const&  parms,
bool                                divideByElementScale
)
    {
    double scaleFactor = divideByElementScale? parms.GetRescaleFactor(): parms.GetDesiredScale();

    RotMatrix scaleMatrix;
    scaleMatrix.initFromScale (scaleFactor);

    Transform scaleTransform;
    scaleTransform.initFromMatrixAndFixedPoint (&scaleMatrix, &originForScale);

    context.PushTransform (scaleTransform);

    //  Account for aspectRatioSkew -- must be done last!
#ifdef DGNV10FORMAT_CHANGES_WIP
    context.ReplaceAspectRatioSkewWithTranslationEffect (originForScale, parms.GetAspectRatioSkew());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearHandler (ElementRefP ref)
    {
    if (NULL == ref)
        return;

    ref->SetHandler (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::_AddAnnotationScale
(
EditElementHandleR eh,
DgnModelP    model
)
    {
#ifdef WIP_V10_MODELINFO
    if (GetAnnotationScale (NULL, eh))
        return ERROR;

#if defined (SET_DYNAMIC_RANGE_ON_ALL_ANNOTATIONS)
    eh.GetElementP()->hdr.dhdr.props.b.dynamicRange = true;
#endif


    //double scale = mdlDgnModel_getEffectiveAnnotationScale (model);
    double scale = model? dgnModel_getAnnotationScale (model): 1.0;

    TransformInfo       transInfo;
    transInfo.SetOptions (TRANSFORM_OPTIONS_DimValueMatchSource | TRANSFORM_OPTIONS_ApplyAnnotationScale);
    transInfo.SetAnnotationScale (scale);
    transInfo.SetAnnotationScaleAction (AnnotationScaleAction::Add);

    StatusInt status = eh.GetHandler().ApplyTransform (eh, transInfo);

    if (SUCCESS != status)
        return status;

    clearHandler (getElementRef (eh));
#endif
    BeAssert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::_UpdateSpecifiedAnnotationScale
(
EditElementHandleR elemHandle,
double          scale
)
    {
    if (!GetAnnotationScale (NULL, elemHandle))
        return ERROR;

    TransformInfo       transInfo;
    transInfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale);
    transInfo.SetAnnotationScale (scale);
    transInfo.SetAnnotationScaleAction (AnnotationScaleAction::Update);

    return elemHandle.GetHandler().ApplyTransform (elemHandle, transInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::_UpdateAnnotationScale
(
EditElementHandleR elemHandle
)
    {
#ifdef WIP_V10_MODELINFO
    DgnModelP model = elemHandle.GetDgnModel();
    if (NULL == model)
        return ERROR;

    double scale = dgnModel_getAnnotationScale (model);

    return _UpdateSpecifiedAnnotationScale (elemHandle, scale);
#endif
    BeAssert (false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::_RemoveAnnotationScale
(
EditElementHandleR eh
)
    {
    TransformInfo       transInfo;
    transInfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale);
    transInfo.SetAnnotationScale (1.0);
    transInfo.SetAnnotationScaleAction (AnnotationScaleAction::Remove);

    StatusInt status = eh.GetHandler().ApplyTransform (eh, transInfo);
    if (SUCCESS != status)
        return status;

    AnnotationScale::SetAsXAttribute (eh, NULL);    // make sure the XAttribute, if any, has been removed

#if defined (SET_DYNAMIC_RANGE_ON_ALL_ANNOTATIONS)
    eh.GetElementP()->hdr.dhdr.props.b.dynamicRange = false;
    //***TBD recompute actual range and update hdr.dhdr.range!
#endif

    clearHandler (getElementRef (eh));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationDisplayParameters::Init ()
    {
    desiredScale = 1.0;
    elementScale = 1.0;
    aspectRatioSkew.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationDisplayParameters::Init (double e, double r, double d, Transform const& a)
    {
    desiredScale = r / d;
    elementScale = e;
    aspectRatioSkew = a;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationDisplayParameters::RemoveAspectRatioSkew ()
    {
    aspectRatioSkew.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationDisplayParameters::SwapScales ()
    {
    std::swap (elementScale, desiredScale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool AnnotationDisplayParameters::HasAspectRatioSkew () const
    {
    return TO_BOOL( !bsiTransform_isIdentity (&aspectRatioSkew) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
StrokeAnnotationElm::StrokeAnnotationElm
(
IAnnotationHandler*         annotationHandlerIn,
ElementHandleCR             elementIn,
ViewContextR                contextIn,
IAnnotationStrokeForCache*  elementStrokerIn,
UInt32                      qvIndexBase
) :
    m_element (elementIn),
    m_context (contextIn)
    {
    m_annotationHandler = annotationHandlerIn;
    m_elementStroker    = elementStrokerIn;
    m_qvIndexBase       = qvIndexBase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static int     indexDouble (double v)
    {
    static bvector<double> s_doubles;

    if (s_doubles.empty())
        s_doubles.push_back (1.0);

    for (bvector<double>::iterator i = s_doubles.begin(); i != s_doubles.end(); ++i)
        {
        if (double_equal (*i, v))
            return static_cast<int>(std::distance (s_doubles.begin(), i));
        }

    if (s_doubles.size() > 100)
        return -1;

    s_doubles.push_back (v);

    return static_cast<int>(s_doubles.size() - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static double getExagg (Transform const& aspectRatioSkew)
    {
    RotMatrix auxRot;
    bsiTransform_getMatrix (&aspectRatioSkew, &auxRot);
    DVec3d exaggerated;
    bsiRotMatrix_getNormalizedColumns (&auxRot, &auxRot, &exaggerated);
    return exaggerated.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void    StrokeAnnotationElm::DrawUsingContext ()
    {
#ifndef NDEBUG
    // NOTE: Can't use qvElem from text node and still hilite individual text components...
    ElementRefP         elRef = m_context.GetCurrentElement ();
    ElementRefP         parentRef = (elRef ? (elRef->IsComplexComponent () ? elementRef_getOutermostParent (elRef) : elRef) : NULL);
    ElementHiliteState  hiliteState = (parentRef ? parentRef->IsHilited () : HILITED_None);
    bool                ignoreCache = (HILITED_ComponentsOnly == hiliteState || HILITED_ComponentsNormal == hiliteState);
#endif

    if (!m_annotationHandler->ComputeAnnotationDisplayParameters (m_parms, m_element, m_context))
        {
        // Need to add checks for range
        //  Note: 0 is always the qv index for element's native geometry. We call this "unscaled."
        //          Actually, the element may have been scaled in its home model. Regardless, CalculateScaleFactors
        //          has determined that we want to see the element's geometry as is. Perhaps that's because
        //          the reference annotation scale exactly matches the element's home model scale.
        //          "Unscaled" really just means "don't apply any scale factor to it here."

#ifndef NDEBUG
        if (ignoreCache)
            {
            CachedDrawHandle drawHandle(&m_element);
            m_elementStroker->_StrokeForCache (drawHandle, m_context, 0.0);
            }
        else
#endif
            {
            ElementRefP ref = m_element.GetElementRef();
            bool complexWithDER = (ref && ref->IsComplexHeader() && ref->HasDerivedRange());
            if (complexWithDER)
                {
                CachedDrawHandle drawHandle(&m_element);
                m_elementStroker->_StrokeForCache (drawHandle, m_context, 0.0); // The complex itself is not an annotation, but some of its components are annotations.
                }
            else
                m_context.DrawCached (CachedDrawHandle(&m_element), *m_elementStroker, m_qvIndexBase);
            }
        return;
        }

    // Need to add checks for range

    // NB: The QV cache index must depend on both ref annotation scale and current display scale.
    //      Changing either one will change how annotation elements draw themselves.
    // NB: When viewing an annotation through a reference attachment with ref annotation scale=ON,
    //      we might have to scale the element up or down, in order to match the master model's annotation scale.
    //      Therefore, the appearance of the annotation ALSO depends on the element's native scale.
    //      (For efficiency reasons, uStn scales an element's persistent geometry to match its home model's scale. That's its "native" scale.)
    //      For example, the reference annotation scale may be 1.0, but that's only half of the story.
    //      If the element's native scale is 5.0, then we have to scale it down 5 times before we display it.
    //      Therefore, we want a qvIndex that corresponds to 1/5, not 1.
    int qvIndex = indexDouble (m_parms.GetRescaleFactor());

    // NB: When a view handler imposes an aspectRatioSkew, we might apply "exaggeration" (i.e., non-uniform scaling)
    //      or mirroring or who knows what. This might be handled by just manipulating the view display transform, but
    //      it might also entail modifying the element (e.g., transforming it). Therefore, to be safe, we must make the
    //      QV cache index depend on the aspectRatioSkew.
    //  ***NEEDS WORK: If the only effect of the aspectRatioSkew is to modify the view display transform, then we don't need
    //      a separately cached representation of the element. How can we know if we can make this assumption??
    // NB: We must have a distinct set of qv cache keys for each exaggeration scale, since exaggeration
    //      can do something different to an annotation element than ordinary annotation scaling will do.
    double exagg = getExagg (m_parms.GetAspectRatioSkew());
    if (!double_equal (exagg, 1.0))
        {
        int qvIndexExagg = indexDouble (exagg);
        if (qvIndexExagg != -1)
            qvIndex |= (1+qvIndexExagg)<<16;
        }

    CachedDrawHandle drawHandle(&m_element);

#ifndef NDEBUG
    if (ignoreCache)
        this->_StrokeForCache (drawHandle, m_context, 0.0);
    else
#endif
    m_context.DrawCached (drawHandle, *this, qvIndex+m_qvIndexBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void    StrokeAnnotationElm::_StrokeForCache (CachedDrawHandleCR drawHandle, ViewContextR context, double pixelSize)
    {
    BeAssert(drawHandle.GetElementHandleCP() != NULL);
    m_elementStroker->StrokeScaledForCache (*drawHandle.GetElementHandleCP(), context, m_parms);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelAppData::Key     s_cacheMaxRescaleFactorKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheMaxRescaleFactor : DgnModelAppData
{
    double m_maxRescaleFactor;
    
    virtual void        _OnCleanup (DgnModelR host) override
        {
        delete this;
        }

    virtual void         _OnElementAdded (DgnModelR, PersistentElementRef&, bool isGraphicsList) override;

    static void          Set (DgnModelR cache, double v)
        {
        CacheMaxRescaleFactor* rsf = (CacheMaxRescaleFactor*) cache.FindAppData (s_cacheMaxRescaleFactorKey);

        if (NULL == rsf)
            {
            // Note: At one time, the CacheMaxRescaleFactor object was allocated in the heapzone of the cache.
            //  That proved problematic (TR#294883) because there was a case when CacheMaxRescaleFactor::Set was called 
            //  with a cache that was never filled. When that cache was emptied, the heapzone was
            //  cleared (WITHOUT calling our _OnCleanup, because calling AppData's _OnEmpty is skipped when the cache is unfilled),
            //  leaving CacheMaxRescaleFactor in the AppData list but deleted. The crash happened when the cache destructor tried to call our _OnCleanup.
            rsf = new CacheMaxRescaleFactor();
            cache.AddAppData (s_cacheMaxRescaleFactorKey, rsf);
            }

        rsf->m_maxRescaleFactor = v;
        }

    static StatusInt     Get (double& v, DgnModelR cache)
        {
        CacheMaxRescaleFactor* rsf = (CacheMaxRescaleFactor*) cache.FindAppData (s_cacheMaxRescaleFactorKey);

        if (NULL == rsf)
            return ERROR;

        v = rsf->m_maxRescaleFactor;
        return SUCCESS;
        }

    static StatusInt    UpdateScaledRangeOfElement (DRange3dR, DgnModelR, PersistentElementRef&, IScaledRangeProcessor* proc);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IAnnotationHandler::ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR drange, double rescale, DPoint3d const* originForScale)
    {
    // Some annotation types do not necessarily scale symmetrically about their range center. 
    // For example, in the case of TFS#14031, when a dimension element is scaled up, its text
    // moves outside the estimated scaled range. To fix that problem, we are using a simple
    // heuristic to apply an additional doubling factor to that estimated range
    // so the annotation is drawn entirely inside.
    double rescaleBuffer = 2.0;

    drange = eh.GetElementCP()->GetRange();
    if (NULL == originForScale)
        {
        rescale *= rescaleBuffer;
        bsiDRange3d_scaleAboutCenter (&drange, &drange, rescale);
        }
    else
        {
        RotMatrix scr;
        bsiRotMatrix_initFromScale (&scr, rescale);
        Transform sct;
        bsiTransform_initFromMatrixAndFixedPoint (&sct, &scr, originForScale);
        DRange3d scaledRange;
        bsiTransform_multiplyRange (&sct, &scaledRange, &drange);

        scaledRange.scaleAboutCenter (&scaledRange, rescaleBuffer);

        drange.unionOf (&drange, &scaledRange);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  IAnnotationHandler::_ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR drange, double rescale)
    {
    if (!GetAnnotationScale (NULL, eh))
        return ERROR;
    return ComputeAnnotationScaledRange (eh, drange, rescale, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IAnnotationHandler::ComputeAnnotationScaledRange (ElementHandleCR e, DRange3dR d, double v)
    {
    return _ComputeAnnotationScaledRange(e,d,v);
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      04/2008
+===============+===============+===============+===============+===============+======*/
class AnnotationRescaledRange : public DerivedElementRange
    {
    virtual void        _OnRemoveDerivedRange (PersistentElementRef const& host, bool deletedCache, HeapZone& zone) override
        {
        if (!deletedCache)
            zone.Free (this, sizeof (*this));
        }

    virtual void        _UpdateDerivedRange (PersistentElementRef& host) override
        {
        //  Note: do NOT update the range of my components. The derived range on each component will get its own callback
        CacheMaxRescaleFactor::UpdateScaledRangeOfElement (m_range, *host.GetDgnModelP(), host, NULL);
        }

public:
    static StatusInt Set (PersistentElementRef&, DRange3dCR, bool isAlreadyInRangeTree);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
// static
StatusInt AnnotationRescaledRange::Set (PersistentElementRef& host, DRange3dCR range, bool updateRangeTree)
    {
    if (updateRangeTree)
        OnDerivedRangeChangePre (host);

    AnnotationRescaledRange* arr = NULL;

    DerivedElementRange* cder = host.GetDerivedElementRange ();
    if (NULL != cder)
        {
        arr = dynamic_cast<AnnotationRescaledRange*> (cder);
        if (NULL == arr)
            {
            BeAssert (false && "element already has a competing DerivedElementRange");
/*<=*/      return ERROR;
            }
        }
    else
        {
        HeapZone& zone = host.GetDgnProject()->Models().ElementPool().GetHeapZone();
        arr = new (zone.Alloc (sizeof(AnnotationRescaledRange))) AnnotationRescaledRange;
        if (host.AddDerivedRange (*arr) != SUCCESS)
/*<=*/      return ERROR;
        }

    arr->m_range = range;

    if (updateRangeTree)
        OnDerivedRangeChangePost (host);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScaledRangeSetter::OnScaledRange (ElementHandleCR eh, DRange3dR r)
    {
    PersistentElementRef* ce = (PersistentElementRef*) eh.GetElementRef();
    bool isComponent = ce->IsComplexComponent ();
    if (!isComponent)
        {
        if (m_componentsOnly)
            return SUCCESS;
        }
                                                                 //  VVVVVVVVVVVVV ***TRICKY: components are not in the range tree
    return AnnotationRescaledRange::Set (*ce, r, m_addToRangeTree && !isComponent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt computeAnnotationScaledRange
(                               // <= SUCCESS if scaled range was computed; ERROR if not or if proc returned non-zero
DRange3dR      newRange,       // <= scaled range
ElementHandleCR    eh,
double          rescaleFactor,
IScaledRangeProcessor* proc = NULL
)
    {
    DisplayHandlerP dhandler = eh.GetDisplayHandler ();
    if (NULL == dhandler)
        return ERROR;

    IAnnotationHandler* ah = dhandler->GetIAnnotationHandler (eh);
    if (NULL != ah)
        {
        DRange3d dnewRange;
        if (ah->ComputeAnnotationScaledRange (eh, dnewRange, rescaleFactor) != SUCCESS)
            return ERROR; // not actually behaving as an annotation

        newRange = dnewRange;
        }
    else
        {
        if (computeAnnotationScaledRangeOfComponents (newRange, eh, rescaleFactor, eh.GetElementCP()->GetRange(), proc) != SUCCESS)
            return ERROR; // no annotations found
        }

    // An expanded range was computed
    if (NULL != proc)
        {
        if (proc->OnScaledRange (eh, newRange) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             7/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void elemUtil_unionScanRanges (DRange3dP outrange, DRange3dCP inrange)
    {
    if (inrange->low.x < outrange->low.x)
        outrange->low.x = inrange->low.x;
    if (inrange->low.y < outrange->low.y)
        outrange->low.y = inrange->low.y;
    if (inrange->low.z < outrange->low.z)
        outrange->low.z = inrange->low.z;

    if (inrange->high.x > outrange->high.x)
        outrange->high.x = inrange->high.x;
    if (inrange->high.y > outrange->high.y)
        outrange->high.y = inrange->high.y;
    if (inrange->high.z > outrange->high.z)
        outrange->high.z = inrange->high.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt computeAnnotationScaledRangeOfComponents (DRange3dR newRange, ElementHandleCR eh, double rescaleFactor, DRange3dCR currentRange, IScaledRangeProcessor* proc)
    {
    newRange = currentRange;
    bool any = false;                                           // NB: Do NOT iterate scdef components!
    for (ChildElemIter child (eh, ExposeChildrenReason::Query); child.IsValid (); child = child.ToNext ())
        {
        DRange3d childRange;
        if (SUCCESS == computeAnnotationScaledRange (childRange, child, rescaleFactor, proc))
            {
            elemUtil_unionScanRanges (&newRange, &childRange);
            any = true;
            }
        }
    return any? SUCCESS: ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CacheMaxRescaleFactor::UpdateScaledRangeOfElement (DRange3dR range, DgnModelR cache, PersistentElementRef& elem, IScaledRangeProcessor* proc)
    {
    double maxRescaleSoFar;
    if (Get (maxRescaleSoFar, cache) != SUCCESS)
        return ERROR;

    ElementHandle eh (&elem);
    if (computeAnnotationScaledRange (range, eh, maxRescaleSoFar, proc) != SUCCESS)
        return ERROR;

    BeAssert (!elem.IsDynamicRange() && "dynamic range element cannot have a derived range");

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void    CacheMaxRescaleFactor::_OnElementAdded (DgnModelR hostCache, PersistentElementRef& elem, bool isGraphicsList)
    {
    if (!isGraphicsList)
        return;

    //  ***TRICKY: When updating the range of a new elem, we must also update the derived ranges of its components
    ScaledRangeSetter setDerivedRangeOnHostAndComponents (/*addToRangeTree*/false, /*updateComponentsOnly*/false);

    DRange3d __;
    UpdateScaledRangeOfElement (__, hostCache, elem, &setDerivedRangeOnHostAndComponents);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeAnnotationScale *     BentleyApi::mdlChangeAnnotationScale_new
(
DgnModelP                modelRefIn
)
    {
    ChangeAnnotationScale * changeContext;

    changeContext = (ChangeAnnotationScale *) malloc (sizeof (*changeContext));
    memset (changeContext, 0, sizeof (*changeContext));
    changeContext->modelRef = modelRefIn;
    changeContext->traverseContext = CAS_TRAVERSE_ELEMENTS_ALL;
    changeContext->filter = CAS_FILTER_NONE;
    changeContext->filterAnnotationScale = 1.0;
    changeContext->action = AnnotationScaleAction::Update;
    changeContext->newAnnotationScale = 1.0;

    return changeContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeAnnotationScale *     BentleyApi::mdlChangeAnnotationScale_copy
(
ChangeAnnotationScale *     changeContextIn
)
    {
    if (NULL == changeContextIn)
        return NULL;

    ChangeAnnotationScale * changeContext;

    changeContext = (ChangeAnnotationScale *) malloc (sizeof (*changeContext));
    memcpy (changeContext, changeContextIn, sizeof (*changeContext));
    if (changeContext->numElementRef > 0 && NULL != changeContext->elementRefArray)
        {
        changeContext->elementRefArray = (ElementRefP*) malloc (sizeof (*changeContext->elementRefArray) * changeContext->numElementRef);
        memcpy (changeContext->elementRefArray, changeContextIn->elementRefArray, sizeof (*changeContext->elementRefArray) * changeContext->numElementRef);
        }

    return changeContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_free
(
ChangeAnnotationScale **    changeContextIn
)
    {
    ChangeAnnotationScale * changeContext;
    if (NULL == changeContextIn || NULL == (changeContext = *changeContextIn))
        return ERROR;
    if (NULL != changeContext->elementRefArray)
        free (changeContext->elementRefArray);

    *changeContextIn = NULL;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP        BentleyApi::mdlChangeAnnotationScale_getDgnModel
(
ChangeAnnotationScale const *   changeContextIn
)
    {
    if (NULL == changeContextIn)
        return NULL;
    return changeContextIn->modelRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_setTraverseElementContext
(
ChangeAnnotationScale *     changeContextIn,
CASTraverseElementContext   traverseContextIn,
int                         numElementRefIn,
ElementRefP const *          elementRefArrayIn
)
    {
    if (NULL == changeContextIn)
        return ERROR;

    changeContextIn->traverseContext = traverseContextIn;

    changeContextIn->numElementRef = numElementRefIn;
    if (NULL != changeContextIn->elementRefArray)
        free (changeContextIn->elementRefArray);

    if (numElementRefIn > 0 && NULL != elementRefArrayIn)
        {
        changeContextIn->elementRefArray = (ElementRefP *) malloc (numElementRefIn * sizeof (elementRefArrayIn[0]));
        BeAssert (changeContextIn->elementRefArray);
        if (NULL == changeContextIn->elementRefArray)
            return ERROR;

        memcpy (changeContextIn->elementRefArray, elementRefArrayIn, numElementRefIn * sizeof (elementRefArrayIn[0]));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_getTraverseElementContext
(
ChangeAnnotationScale const *   changeContextIn,
CASTraverseElementContext *     traverseContextOut,
int *                           numElementRefOut,
ElementRefP **                   elementRefArrayOut
)
    {
    if (NULL == changeContextIn)
        return ERROR;
    if (NULL != traverseContextOut)
        *traverseContextOut = changeContextIn->traverseContext;
    if (NULL != numElementRefOut)
        *numElementRefOut = changeContextIn->numElementRef;
    if (NULL != elementRefArrayOut)
        *elementRefArrayOut = changeContextIn->elementRefArray;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_setFilter
(
ChangeAnnotationScaleP     changeContextIn,
CASFilter                   filterIn,
double                      filterAnnotationScaleIn
)
    {
    if (NULL == changeContextIn)
        return ERROR;
    changeContextIn->filter = filterIn;
    changeContextIn->filterAnnotationScale = filterAnnotationScaleIn;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_getFilter
(
ChangeAnnotationScaleCP   changeContextIn,
CASFilter *                     filterOut,
double *                        filterAnnotationScaleOut
)
    {
    if (NULL == changeContextIn)
        return ERROR;
    if (NULL != filterOut)
        *filterOut = changeContextIn->filter;
    if (NULL != filterAnnotationScaleOut)
        *filterAnnotationScaleOut = changeContextIn->filterAnnotationScale;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_setAction
(
ChangeAnnotationScale *     changeContextIn,
AnnotationScaleAction       actionIn,
double                      newAnnotationScaleIn
)
    {
    if (NULL == changeContextIn)
        return ERROR;

    if (newAnnotationScaleIn <= 0 && actionIn != AnnotationScaleAction::Remove)
        return ERROR;

    changeContextIn->action = actionIn;
    changeContextIn->newAnnotationScale = newAnnotationScaleIn;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_getAction
(
ChangeAnnotationScale const *   changeContextIn,
AnnotationScaleAction *         actionOut,
double *                        newAnnotationScaleOut
)
    {
    if (NULL == changeContextIn)
        return ERROR;

    if (NULL != actionOut)
        *actionOut = changeContextIn->action;
    if (NULL != newAnnotationScaleOut)
        *newAnnotationScaleOut = changeContextIn->newAnnotationScale;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlChangeAnnotationScale_getEffectiveValues
(
ChangeAnnotationScaleCP   changeContextIn,
double *                        newAnnotationScaleOut,
bool    *                       newUseAnnotationScaleOut,
double *                        annotationScaleChangeOut,
double                          currAnnotationScale,
bool                            currUseAnnotationScale
)
    {
    double                      annotationScaleFilterValue  = 1.0;
    CASFilter                   filter                      = CAS_FILTER_NONE;

    mdlChangeAnnotationScale_getFilter (changeContextIn, &filter, &annotationScaleFilterValue);
    if (CAS_FILTER_ANNOTATION_SCALE_SPECIFIED == filter && 0 != annotationScaleFilterValue &&
        fabs (annotationScaleFilterValue - currAnnotationScale) > mgds_fc_epsilon)
        return ERROR;

    double                      annScaleChange          = 1.0;
    double                      newAnnotationScale      = 1.0;
    bool                        newUseAnnotationScale   = true;
    AnnotationScaleAction       action                  = AnnotationScaleAction::Update;

    // Apply the new scale
    mdlChangeAnnotationScale_getAction (changeContextIn, &action, &newAnnotationScale);
    newUseAnnotationScale = currUseAnnotationScale;
    switch (action)
        {
        case    AnnotationScaleAction::Add:
            /* bypass if annotation scale is already set */
            if (currUseAnnotationScale)
                return ERROR;

            annScaleChange = newAnnotationScale;
            newUseAnnotationScale = true;
            break;
        case    AnnotationScaleAction::Update:
            /* bypass if annotation scale is not currently set */
            if (!currUseAnnotationScale)
                return ERROR;
            if (fabs (currAnnotationScale) < mgds_fc_epsilon)
                return ERROR;

            annScaleChange = newAnnotationScale / currAnnotationScale;
            if (fabs (annScaleChange - 1.0) < mgds_fc_epsilon)
                return ERROR;

            break;
        case    AnnotationScaleAction::Remove:
            /* bypass if annotation scale is not currently set */
            if (!currUseAnnotationScale)
                return ERROR;
            if (fabs (currAnnotationScale) < mgds_fc_epsilon)
                return ERROR;

            newUseAnnotationScale = false;
            annScaleChange = 1.0 / currAnnotationScale;
            break;
        }

    if (NULL != annotationScaleChangeOut)
        *annotationScaleChangeOut = annScaleChange;

    if (NULL != newAnnotationScaleOut)
        *newAnnotationScaleOut = newAnnotationScale;

    if (NULL != newUseAnnotationScaleOut)
        *newUseAnnotationScaleOut = newUseAnnotationScale;

    return SUCCESS;
    }

/*

What is a "Derived Range"?
-------------------------

An element can have a temporary, computed range during a session that is different from its stored range.
A computed range is captured and stored on the element's elementref as a DerivedElementRange app data object.
An element that has one of these objects is said to have a derived range.

The ElementRef::GetDerivedRange method looks for the DerivedElementRange app data first and defaults to the stored range.

The range index and other range-related logic in MicroStation call ElementRef::GetDerivedRange in order to get an
element's range.

Note that a derived range is not the same as a dynamic range.
- An element that has a derived range is in the range tree (headers only). A dynamic range is not in the range tree.
- For a derived range, MicroStation calls element's handler to compute the scaled range and does not call the handler after that. For a dynamic range, MicroStation calls the element's handler every time it needs the element's range.

When is a derived range needed?

An annotation element can be rescaled on the fly when it draws. This happens if the annotation is in a reference and
the reference is attached using active annotation scale. Since an element must not draw outside of its range, we must
expand its range. Since annotation rescaling is meant to affect only the appearance of the element at draw time, we must
not modify the element's hdr.range data. Instead, we add a DerivedElementRange app data object to its elementref.

When do we update an element's range?
------------------------------------

1. On reference display (annotationScale_setDerivedRanges)

    If active annotation scale is on for the reference and the referenced cache has a smaller annotation scale than the parent cache,
    then annotation elements in the reference will scale themselves up as they draw. These elements are already in the elem range index
    of the reference. Therefore, to prevent them from drawing outside their ranges, we must update their ranges in the reference range index.
    For each annotation in the reference, we
        1) take the element out of the range index,
        2) compute the scaled up range and add it as a DerivedElementRange app data object to the elementref, and
        3) add the element to the range index under the new range.

    We also add app data to the referenced cache. This is the CacheMaxRescaleFactor object. This object keeps track of the max
    rescaling we've applied to elements in this cache so far. It also reacts to element adds.

2. On add to activated reference (CacheMaxRescaleFactor::_OnElementAdded)

    When a new annotation element is added to an activated reference, we must assign it a derived range, to ensure that it does not draw
    outside its range. On adding a new displayable, a cache will invoke CacheMaxRescaleFactor::_OnElementAdded. This method is invoked
    *before* the element is added to the elem range index. The _OnElementAdded method, computes and adds a derived range to the new elementref.
    The cache then picks up and uses this derived range when adding the new elemento the range index.

3. On modifying elements in activated reference (DerivedElementRange::_UpdateDerivedRange)

    When an existing annotation element is modified in an activated reference, must must keep its derived range, if any, in synch with its
    stored range. If the element is moved, then the derived range must be moved. If the element is scaled, then the derived range must
    be scaled. The cache invokes the DerivedElementRange::_UpdateDerivedRange method *just before* an element is re-added to the range index because
    of a modification. The _UpdateDerivedRange method should update the range held in DerivedElementRange object.

Derived Ranges of Complex Components
------------------------------------

Complex components are not entered into the elem range index. Nevertheless, components can have derived ranges.

A complex component can be an annotation and can be rescaled. Like any element, a rescaled annotation component must not draw outside of its
range. Therefore, a rescaled annotation component must have a derived range (if possible). annotationScale_setDerivedRanges and CacheMaxRescaleFactor::_OnElementAdded
will assign a derived range to anntotation components initially, and DerivedElementRange::_UpdateDerivedRange will maintain them.
annotationScale_setDerivedRanges and CacheMaxRescaleFactor::_OnElementAdded are called only on header elements. They must therefore actively discover
components and assign derived ranges to them. DerivedElementRange::_UpdateDerivedRange does not iterate components, as dgncache calls it on headers and components.

Shared cell components are an exception to this rule. We cannot add a derived range to the components in this case, because a single definition is used by many
instances. Instead, the instance tells ViewContext not to trust the range of components when doing visibility testing.

*/

/*

    Remove reference scaling on copy

If the attachment was set up to use "active annotation scale":

    If the attachment was scaled, then this annotation would have re-scaled itself at display time so as to ignore the attachment scaling and
    appear to be the same size as annotations in the parent file. When this element is copied, MicroStation
    will scale it by the attachment scale. We must offset that scale, so that this element continues to display
    at the same size as other annotations in the destination file.

If the attachment was NOT set up to use "active annotation scale":

    This annotation would not have appeared to be the same size as annotations in the parent file.
    Now that I have changed its annotation scale to match the parent file, I must scale the element
    to make it appear to be the same size as it was when seen through the attachment. (Note: MicroStation will
    take care of applying the reference attachment scale, if any.)

*/
