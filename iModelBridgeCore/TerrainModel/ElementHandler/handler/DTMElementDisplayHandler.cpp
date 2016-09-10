/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMElementDisplayHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <Bentley/BeTimeUtilities.h>

#include "MrDTMDataRef.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
    void ScanLevelsAndAddOrRemove (ElementHandleCR element, bool add, bool isLoading = false);
END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

#pragma region HighlightRefAppData

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 06/11
//=======================================================================================
struct HighlightRefAppData : public ElementRefAppData
{
private:
    bmap<UInt32, ElementHiliteState> m_map;
    static ElementRefAppData::Key m_key;
public:

    static ElementRefAppData::Key& GetKey()
        {
        return m_key;
        }

    virtual void _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
        {
        delete this;
        }

    void SetHighlight(UInt32 id, ElementHiliteState hiliteState)
        {
        if (hiliteState == HILITED_None)
            {
            bmap<UInt32, ElementHiliteState>::iterator it = m_map.find(id);
            if (it != m_map.end())
                {
                m_map.erase(it);
                }
            }
        else
            m_map[id] = hiliteState;
        }

    ElementHiliteState GetHighlight(UInt32 id)
        {
        bmap<UInt32, ElementHiliteState>::const_iterator it = m_map.find(id);

        if (it != m_map.end())
            {
            return it->second;
            }
        return HILITED_None;
        }
    bool IsEmpty()
        {
        return m_map.size() == 0;
        }
};

ElementRefAppData::Key HighlightRefAppData::m_key;

#pragma endregion

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DeleteDTMXAttributes (EditElementHandle& element)
    {
    ElementHandle::XAttributeIter iter (element);
    while (iter.IsValid())
        {
        XAttributeHandlerId id = iter.GetHandlerId();
        if (id.GetMajorId() == TMElementMajorId)
            {
            UInt32 attrId = iter.GetId();
            iter.ToNext();
            switch(id.GetMinorId())
                {
                case XATTRIBUTES_SUBID_DTM_HEADER:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_FEATUREARRAY:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_POINTARRAY:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_NODEARRAY:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_CLISTARRAY:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_FLISTARRAY:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP:
                    element.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                    break;
                }
            }
        else
            iter.ToNext();
        }
    }

bool UpdateDTMLastModified (EditElementHandle& element, double newTime)
    {
    // Add the lastModifiedDate to the Descriptor.
    DTMElm* dtmEl = (DTMElm*)element.GetElementP();
    if ((sizeof (ExtendedElm) / 2) == dtmEl->ehdr.attrOffset)
        {
        MSElement newEl;

        ElementUtil::SetRequiredFields (newEl, &element, EXTENDED_ELM, sizeof (DTMElm), true, false, false, dtmEl->dhdr.props.b.is3d ? ElementUtil::ELEMDIM_3d : ElementUtil::ELEMDIM_2d, element.GetModelRef ());
        dtmEl = (DTMElm*)&newEl;
        dtmEl->dtmLastModified = newTime;
        element.ReplaceElement (&newEl);
        return true;
        }

    if (newTime > dtmEl->dtmLastModified)
        {
        dtmEl->dtmLastModified = newTime;
        return true;
        }
    return false;
    }

    //=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
bool IsValidTransformation (TransformCP transform)
    {
    if (transform)
        {
        DPoint3d fixedPoint;
        DPoint3d directionVector;
        double radians;
        double scale;
        RotMatrix mat;

        transform->getMatrix (&mat);

        if (mat.isIdentity ())
            return true;

        if(transform->isUniformScaleAndRotateAroundLine (&fixedPoint, &directionVector, &radians, &scale))
            {
            DVec3d upV;
            DVec3d matZ;
            upV.init (0, 0, scale);
            mat.getRow (&matZ, 2);

            if (matZ.isEqual (&upV, 1.0e-10))
                {
                return true;
                }
            }
        }
    return false;
    }


struct DTMElementHandlerTransformManipulatorExtension : public ITransformManipulatorExtension
{
virtual ITransformManipulatorPtr _GetITransformManipulator(ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) override;
};

/*=================================================================================-----*
* @bsiclass                 MrDTMTransformManipulator
+===============+===============+===============+===============+===============+======*/
 struct MrDTMTransformManipulator : RefCounted <ITransformManipulator>
     {
     DisplayHandler*         m_Handler;
     AgendaOperation         m_opType;
     AgendaModify            m_modify;

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 06/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    MrDTMTransformManipulator::MrDTMTransformManipulator
        (
        DisplayHandler*       pHandler,
        AgendaOperation       opType,
        AgendaModify          modify
        )
        {
        m_Handler = pHandler;
        m_opType    = opType;
        m_modify    = modify;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 06/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    MrDTMTransformManipulator::~MrDTMTransformManipulator ()
        {
        m_Handler = NULL;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 06/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool MrDTMTransformManipulator::_AcceptElement
        (
        ElementHandleCR     eh,
        WStringP            cantAcceptReasonP,
        ElementAgendaP      agenda
        ) override
        {
        if (NULL != cantAcceptReasonP)
            {
            *cantAcceptReasonP = TerrainModelElementResources::GetString(MSG_TERRAINMODEL_STMNotAllowedForOperation);
            }

        // reject by default.
        return false;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 06/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool MrDTMTransformManipulator::_IsTransformGraphics (ElementHandleCR eh, TransformInfoCR trans) const override
        {
        return m_Handler->IsTransformGraphics(eh, trans);
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 04/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt MrDTMTransformManipulator::_OnTransform (EditElementHandleR eeh, TransformInfoCR tInfo, ViewContextP context) override
        {
        StatusInt status(m_Handler->ApplyTransform (eeh, tInfo));

        return status;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Chantal.Poulin 04/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    static MrDTMTransformManipulator* Create (DisplayHandler* handler, AgendaOperation opType, AgendaModify modify)
        {
        return new MrDTMTransformManipulator(handler, opType, modify);
        }
    };

/*--------------------------------------------------------------------------------------*
// @bsimethod                                                   Chantal.Poulin 04/11
+---------------+---------------+---------------+---------------+---------------+------*/
struct MrDTMElementTransformManipulatorExtension : public ITransformManipulatorExtension
{
virtual ITransformManipulatorPtr _GetITransformManipulator (ElementHandleCR eh, AgendaOperation opType,
                                                            AgendaModify modify, DisplayPathCP path) override
    {
    // return manipulator only for operation we want to react on.
    switch (opType)
        {
        case AgendaOperation::Clipboard:
        case AgendaOperation::Translate:
        case AgendaOperation::DragDrop:
        case AgendaOperation::Scale:
        case AgendaOperation::Rotate:
        case AgendaOperation::Mirror:
        case AgendaOperation::Array:
        case AgendaOperation::Stretch:
            break;

        default:
            return NULL;
        }

    //TRICKY: Fix TR 177330: We want default implementation when we accept input element. Thus if we accept input
    //                       element, we will NOT return a manipulator and default implementation will be call.
    //                       If element is not accepted, we return a manipulator; element will be rejected by it
    //                       at a later time...(when accept is called).
    //(see TransformTool::OnElementModifyWithContext).
    MrDTMTransformManipulator* pManip = MrDTMTransformManipulator::Create(eh.GetDisplayHandler(), opType, modify);

    if(pManip->AcceptElement(eh, NULL, NULL))
        {
        pManip->Release();
        pManip = NULL;
        }

    return pManip;
    }
};

/*--------------------------------------------------------------------------------------*
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MrDTMElementIModelPublishExtension : public IIModelPublishExtension
{
    /*--------------------------------------------------------------------------------------*
    * @bsimethod                                                   Chantal.Poulin 09/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt _HandleIModelPublish (EditElementHandleR outputEh, EditElementHandleCR inputEh, DgnModelCP destinationModel, bool forceRePublish) override
        {
        // convert it in single Res DTM
        return DTMElementHandlerManager::ConvertMrDTMtoSingleResDTM(outputEh, inputEh, MAX_NB_POINTS_FOR_OVERVIEW, inputEh.GetModelRef());
        }
};


///// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
struct ViewContextLazyDTMDrawingInfoProvider : public LazyDTMDrawingInfoProvider
{
    ViewContext*    m_viewContext;

    ViewContextLazyDTMDrawingInfoProvider
    (
    ElementHandleCR                element,
    DTMDataRef*                    DTMDataRef,
    ViewContext*                   viewContext
    ) : LazyDTMDrawingInfoProvider (element, DTMDataRef), m_viewContext (viewContext)
        {}

    virtual void _Init (void) override
        {
        DTMElementDisplayHandler::GetDTMDrawingInfo (m_info, m_element, m_DTMDataRef.get(), m_viewContext);
        }

}; // End ViewContextLazyDTMDrawingInfoProvider struct

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
struct HitPathLazyDTMDrawingInfoProvider : public LazyDTMDrawingInfoProvider
{

    HitPathCR       m_hitPath;

    HitPathLazyDTMDrawingInfoProvider
    (
    ElementHandleCR                element,
    DTMDataRef*   DTMDataRef,
    HitPathCR                   hitPath
    ) : LazyDTMDrawingInfoProvider (element, DTMDataRef), m_hitPath (hitPath)
        {}

    virtual void _Init (void) override
        {
        DTMElementDisplayHandler::GetDTMDrawingInfo (m_info, m_element, m_DTMDataRef.get(), nullptr, &m_hitPath);
        }

}; // End HitPathLazyDTMDrawingInfoProvider struct

//=======================================================================================
// @bsiclass                                                   Piotr.Slowinski 05/11
//=======================================================================================
class DTMAnnotationHandler : public IAnnotationHandler
{
public: static DTMAnnotationHandler s_singleton;
private: bool _GetAnnotationScale ( double *scale, ElementHandleCR element ) const override
    {
    if ( scale )
        *scale = dgnModel_getEffectiveAnnotationScale (element.GetModelRef ()->GetDgnModelP () );
    return true;
    }
}; // End DTMAnnotationHandler class

DTMAnnotationHandler DTMAnnotationHandler::s_singleton;

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
IAnnotationHandlerP GetSharedAnnotationHandler ( void )
    {
    return &DTMAnnotationHandler::s_singleton;
    }

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForCacheHull : IDTMStrokeForCache
    {
    private:
        BcDTMP m_dtmElement;
        DTMDrawingInfo& m_drawingInfo;
        ViewContextP context;
        bool m_hasDisplayedSomething;

    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMStrokeForCacheHull (BcDTMP DTMDataRefXAttribute, DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
            {
            m_dtmElement = DTMDataRefXAttribute;
            m_hasDisplayedSomething = false;
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        static int draw(DTMFeatureType dtmFeatureType,int numTriangles,int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP)
            {
            UInt32 numPerFace = 3;
            bool   twoSided = false;
            size_t indexCount = numMeshFaces - 3;
            size_t pointCount = numMeshPts;
            DPoint3dCP pPoint = meshPtsP;
            Int32 const* pPointIndex = (Int32*)meshFacesP;
            size_t normalCount = numMeshPts;
            DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
            Int32 const* pNormalIndex = (Int32*)meshFacesP;

            PolyfaceQueryCarrier polyCarrier (numPerFace, twoSided, indexCount, pointCount, pPoint, pPointIndex, normalCount, pNormal, pNormalIndex);
            ViewContextP context = (ViewContextP)userP;
            context->GetIDrawGeom().DrawPolyface (polyCarrier);
            return SUCCESS;
            }
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //
        // Strokes the DTM for the cache
        // At the moment, everything is handled by this stroke method. In the future, each
        // part (triangle, features, ...), will have its own stroking method.
        //
        //=======================================================================================
        void _StrokeForCache (ElementHandleCR element, ViewContextR context, double pixelSize)
            {
            if (context.CheckStop())
                return;

            // Get the DTM element
            if (m_dtmElement == nullptr)
                return;

            // Get the unmanaged handle.... everything must be very fast here.
            BcDTMP bcDTM = m_dtmElement;

            // Display the hull
            DTMPointArray boundary;
            bcDTM->GetBoundary (boundary);
            DPoint3d *hullP = boundary.data();
            int hullPointCount = (int)boundary.size();

            // Push the transformation matrix to transform the coordinates to UORS.
            DrawSentinel    sentinel (context, m_drawingInfo);

            //m_drawingInfo.FullStorageToUors (hullP, hullPointCount);
            m_hasDisplayedSomething = (hullPointCount != 0);

            if (context.GetDrawPurpose () == DrawPurpose::FenceAccept)
                {
                BcDTMPtr dtm = BcDTM::Create (hullPointCount, 100);
                DTMFeatureId featureId;
                dtm->AddLinearFeature (DTMFeatureType::Hull, hullP, hullPointCount, &featureId);
                dtm->Triangulate();
                bcdtmInterruptLoad_triangleShadeMeshForQVCacheFromDtmObject((BC_DTM_OBJ*)dtm->GetTinHandle(), 65000,2,1,&draw, false, DTMFenceType::None, DTMFenceOption::None, nullptr, 0, &context);
                context.DrawStyledLineString3d (hullPointCount, hullP, nullptr);
                }
            else
                context.DrawStyledLineString3d (hullPointCount, hullP, nullptr);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        bool HasDisplayedSomething()
            {
            return m_hasDisplayedSomething;
            }
    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/11
//=======================================================================================
int geoconvertFunction (DPoint3d* pts, size_t numPoints, void* userP)
    {
    IGeoCoordinateReprojectionHelper* helper = (IGeoCoordinateReprojectionHelper*)userP;
    helper->ReprojectPoints (pts, nullptr, nullptr, pts, (int)numPoints);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------****
* @bsimethod                                                   Alain.Robert  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DTMElementDisplayHandler::_OnGeoCoordinateReprojection (EditElementHandleR element, IGeoCoordinateReprojectionHelper& geo, bool inChain)
    {
    RefCountedPtr<DTMDataRef> dtmDataRef;

    DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element);

    if (dtmDataRef.IsNull())
        return REPROJECT_DataError;

    MrDTMDataRef* mrDTMDataRef = dynamic_cast<MrDTMDataRef* >(&*dtmDataRef);
    if (mrDTMDataRef != nullptr)
        {
        // The element is a MRDTM ... this means that the
        // DTM definition is located on file and that it may contain its own spatial reference.
        // The internal coordinates are not then transformed but the extent of the element need to be
        // updated.
        mrDTMDataRef->Reproject(geo.GetSourceGCS(), geo.GetDestinationGCS());
        element.GetDisplayHandler()->ValidateElementRange(element, true);
        return REPROJECT_Success;
        }
    else
        {
        DPoint3d origin;

        ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler).GetTransformOrigin (element, origin);

        TransformInfo   transform;
        transform.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale);
        if (geo.GetLocalTransform (&transform.GetTransformR(), origin, NULL, true, true) == SUCCESS)
            {
            if (ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler).ApplyTransform (element, transform) == SUCCESS)
                return REPROJECT_Success;
            }

        DTMPtr dtm;
        dtmDataRef->GetDTMReferenceStorage (dtm);
        BcDTMPtr bcdtm = dtm->GetBcDTM();

        if (dtmDataRef->GetElement().GetDgnModelP()->IsReadOnly ())
            {
            Transform mtrx;

            DTMElementHandlerManager::GetStorageToUORMatrix (mtrx, element.GetModelRef(), element, false);
            bcdtm = bcdtm->Clone ();

            bcdtm->TransformUsingCallback (&geoconvertFunction, &geo);
            ElementRefP ref = element.GetElementRef ();
            element.SetElementRef (nullptr, element.GetModelRef());
            BcDTMPtr dtm = BcDTM::CreateFromDtmHandle (*bcdtm->GetTinHandle());
            DTMDataRefXAttribute::ScheduleFromDtm (element, &element, *dtm, mtrx, *element.GetModelRef(), true);
            TMReferenceXAttributeHandler::RemoveDTMDataReference (element);
            element.SetElementRef (ref, element.GetModelRef());
            }
        else
            {
            bcdtm->SetMemoryAccess (DTMAccessMode::Write);

            bcdtm->TransformUsingCallback (&geoconvertFunction, &geo);
            }
        element.GetDisplayHandler()->ValidateElementRange(element, true);

        return REPROJECT_Success;
        }
    }

struct DTMElementHandlerTransformManipulator : RefCounted<ITransformManipulator>
    {
    DEFINE_T_SUPER(ITransformManipulator)
     DisplayHandler*         m_handler;
     AgendaOperation         m_opType;
     AgendaModify            m_modify;

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Daryl.Holmwood 08/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    DTMElementHandlerTransformManipulator
        (
        DisplayHandler* handler,
        AgendaOperation opType,
        AgendaModify    modify
        )
        {
        m_handler = handler;
        m_opType = opType;
        m_modify = modify;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Daryl.Holmwood 08/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~DTMElementHandlerTransformManipulator ()
        {
        m_handler = NULL;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Daryl.Holmwood 08/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo) const
        {
        return m_handler->IsTransformGraphics (elHandle, tInfo);
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Daryl.Holmwood 08/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt _OnTransform (EditElementHandleR element, TransformInfoCR tInfo, ViewContextP context)
        {
        // This is because of a bug when you copy a TM in a reference file to a 2d Model.
         if (element.GetElementType() != 106)
            {
            element.ResetHandler();

            DisplayHandler* handler = dynamic_cast<DisplayHandler*>(&element.GetHandler ());
            return !handler ? ERROR : handler->ApplyTransform (element, tInfo);
            }

        StatusInt ret = m_handler->ApplyTransform (element, tInfo);

        if (SUCCESS == ret)
            {
            RefCountedPtr<DTMDataRef> dtmDataRef;
            DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element);

            if ((dtmDataRef != 0) &&
                (dtmDataRef->IsMrDTM() == true) &&
                ((((MrDTMDataRef*)dtmDataRef.get())->CanDisplayMrDTM() == false) ||
                 (((MrDTMDataRef*)dtmDataRef.get())->IsAnchored() == true)))
                {
                return ret;
                }

            if (dtmDataRef->GetElement().GetElementRef() != element.GetElementRef())
                {
                Transform mtrx;

                DTMElementHandlerManager::GetStorageToUORMatrix (mtrx, element.GetModelRef(), element, false);

                EditElementHandle dataElem (dtmDataRef->GetElement().GetElementRef(), dtmDataRef->GetElement().GetModelRef());
                DTMElementHandlerManager::SetStorageToUORMatrix (mtrx, dataElem);
                dataElem.ReplaceInModel(dataElem.GetElementRef());
                }
            }
        return ret;
        }

    /*--------------------------------------------------------------------------------------*
    // @bsimethod                                                   Daryl.Holmwood 08/11
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _AcceptElement (ElementHandleCR elHandle, WStringP cantAcceptReasonP, ElementAgendaP agenda)
        {
        if (m_opType == AgendaOperation::Mirror || m_opType == AgendaOperation::Stretch)
            {
//            mdlOutput_messageCenter (OutputMessagePriority::Error, msgW.ToChar((char*)_alloca (msgW.GetNumBytes())), NULL, false /*openAlertBox*/);

            if (NULL != cantAcceptReasonP)
                {
                *cantAcceptReasonP = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_NotAllowed);
                }

            return false;
            }
        return T_Super::_AcceptElement (elHandle, cantAcceptReasonP, agenda);
        }

    static DTMElementHandlerTransformManipulator* Create (DisplayHandler* handler, AgendaOperation opType, AgendaModify    modify)
        {
        return new DTMElementHandlerTransformManipulator(handler, opType, modify);
        }
    };

ITransformManipulatorPtr DTMElementHandlerTransformManipulatorExtension::_GetITransformManipulator(ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path)
    {
    return DTMElementHandlerTransformManipulator::Create (elHandle.GetDisplayHandler(), opType, modify);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood
//=======================================================================================
bool PushTransformationMatrix (ViewContextP context, ElementHandleCR element, DgnModelRefP destinationModel, DMatrix4d& localMatToRoot)
    {
    DgnModelRefP modelRef = element.GetModelRef();
    std::stack<DgnAttachmentCP> refs;
    std::stack<DgnModelRefP> models;

    if (destinationModel == modelRef)
        return true;

    while (true)
        {
        DgnAttachmentCP rfP = modelRef->AsDgnAttachmentCP();
        if(rfP == nullptr)
            break;

        refs.push (rfP);
        models.push (modelRef);
        modelRef = modelRef->GetParentModelRefP();
        };

    bool push = destinationModel == modelRef;
    bool isInViewlet = !modelRef->Is3d (); //context ? context->InViewlet() : false;
    while (refs.size())
        {
        DgnAttachmentCP rfP = refs.top();
        DgnModelRefP model = models.top();
        refs.pop();
        models.pop ();

        if (destinationModel == model)
            push = true;

        Transform   refTrans;
        rfP->GetTransformToParent (refTrans, true);

        DMatrix4d refTrans2;
        bsiDMatrix4d_initFromTransform (&refTrans2, &refTrans);
        localMatToRoot.productOf (&localMatToRoot, &refTrans2);

        if (!modelRef->Is3d ()) //context && (!isInViewlet && context->InViewlet()))
            {
            isInViewlet = true;
            }
        }
    return true;
    }

void SetColourMap (ViewContextR context, DgnModelRefP modelRef)
    {
    // Need to set the colourMap of the Viewport back to the original element.
    ViewportP viewport = context.GetViewport();

    if (viewport)
        {
        DgnColorMapP   colorMap = DgnColorMap::GetForDisplay (modelRef);

        if (!colorMap)
            {
            BeAssert (!"Color map should not be NULL, check that root model is setup for this viewport.");
            }
        else
            {
            // Restore colour map
            viewport->GetIViewOutput()->DefineColorMap (colorMap);
            viewport->GetIViewOutput()->ModifyColorMapEntry (DgnColorMap::INDEX_Background, viewport->GetBackgroundColor ());
            }
        }
    }

void DTMElementDisplayHandler::GetDTMDrawingInfo (DTMDrawingInfo& info, ElementHandleCR element, DTMDataRef* DTMDataRef, ViewContextP context, HitPathCP hitPath)
    {
    // Get overriding Symbology Element if there is one.
    ElementHandle symbologyEl;
    ElementHandle originalEl;
    DgnModelRefP ref = GetActivatedModel (element, context);

    originalEl = element;
    TMSymbologyOverrideManager::GetReferencedElement (element, originalEl);

    Transform trsf;
    DTMElementHandlerManager::GetStorageToUORMatrix (trsf, GetModelRef(originalEl), originalEl);

    TMSymbologyOverrideManager::GetElementForSymbology (originalEl, symbologyEl, ref);

    ElementHandle refElem = originalEl;
    DMatrix4d localMatToRoot;

    if (context && context->GetViewport())
        context->GetCurrLocalToActiveTrans (localMatToRoot);
    else if (hitPath)
        {
        hitPath->GetLocalToActive (localMatToRoot, hitPath->GetViewport (), hitPath->GetCursorIndex());
        }
    else
        {
        localMatToRoot.initIdentity();
        PushTransformationMatrix (nullptr, element, GetActivatedModel (element, nullptr), localMatToRoot);
        }

    info = DTMDrawingInfo (DTMDataRef, trsf, localMatToRoot, originalEl, symbologyEl);
    bool isVisible = true;
    if (context)
        {
        DRange3d drange;

        DTMDataRef->GetExtents (drange);

        ::DPoint3d* fencePtP = 0;
        int nbPts;

        if (GetVisibleFencePointsFromContext (fencePtP, nbPts, context, info, drange) == false)
            {
            isVisible = false;
            }
        else
            {
            info.SetFence (DTMFenceParams (DTMFenceType::Block, DTMFenceOption::Overlap, fencePtP, nbPts));

            BeAssert(fencePtP != 0);
            delete [] fencePtP;
            }
        }
    info.SetIsVisible (isVisible);

    if (context && symbologyEl.GetModelRef() != originalEl.GetModelRef())
        {
        SetColourMap (*context, symbologyEl.GetModelRef());
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
void DTMElementDisplayHandler::DrawSubElement (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, ViewContextR context, const DTMFenceParams& fence, DTMSubElementContext& subElementContext)
    {
    //DontCare    ViewContext::ModelRefMark mark2 (*context);
    //DontCare    ViewContext::ContextMark mark (*context, element);

    // Create a DTM element from the XAttributes (this is is a very lightweight operation that
    // just assigns the dtm internal arrays to their addresses inside the XAttributes).
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

    if (DTMDataRef != nullptr)
        {
        if (DTMDataRef->IsMrDTM())
            {
            RefCountedPtr<MrDTMDataRef> mrDTMDataRef((MrDTMDataRef*)DTMDataRef.get());

            //MrDTM should not be displayed
            if (mrDTMDataRef->CanDisplayMrDTM() == false)
                {
                //If the MrDTM is selected display its 3D bounding box.
                if (element.GetElementRef() != NULL && element.GetElementRef()->IsInSS(element.GetModelRef()))
                    {
                    DRange3d range3d;
                    mrDTMDataRef->GetExtents(range3d);

                    DTMUnitsConverter conv (element.GetModelRef());
                    conv.FullStorageToUors (range3d.low);
                    conv.FullStorageToUors (range3d.high);

                    DPoint3d  p[8];
                    p[0].x = p[3].x = p[4].x = p[5].x = range3d.low.x;
                    p[1].x = p[2].x = p[6].x = p[7].x = range3d.high.x;
                    p[0].y = p[1].y = p[4].y = p[7].y = range3d.low.y;
                    p[2].y = p[3].y = p[5].y = p[6].y = range3d.high.y;
                    p[0].z = p[1].z = p[2].z = p[3].z = range3d.low.z;
                    p[4].z = p[5].z = p[6].z = p[7].z = range3d.high.z;
                    context.DrawBox(p, true);
                    }

                return;
                }

            //Check if the MrDTM display for the view is turned off.
            if ((context.GetViewport() != 0) &&
                (mrDTMDataRef->IsVisibleForView(context.GetViewport()->GetViewNumber()) == false))
                {
                return;
                }
            }

        if (nullptr == subElementContext.drawingInfo)
            {
            subElementContext.drawingInfo = new DTMDrawingInfo ();
            GetDTMDrawingInfo (*subElementContext.drawingInfo, element, DTMDataRef.get (), &context);
            }
        DTMDrawingInfo& info = *subElementContext.drawingInfo;

        if (!info.IsVisible ())
            return;
        if (fence.points != nullptr)
            {
            DPoint3d* points = (DPoint3d*)_alloca (sizeof (DPoint3d) * fence.numPoints);

            DMatrix4d localMatToRoot, m2;
            context.GetCurrLocalToActiveTrans (localMatToRoot);
            m2.qrInverseOf (&localMatToRoot);
            m2.multiplyAndRenormalize (points, fence.points, fence.numPoints);

            info.FullUorsToStorage (points, fence.numPoints);

            if (fence.fenceType == DTMFenceType::Block)
                {
                // Check that it is still block after transformation
                }
            info.SetFence (DTMFenceParams (DTMFenceType::Shape/*fence.fenceType*/, fence.fenceOption, points, fence.numPoints));
            }

        DTMElementSubDisplayHandler* hand = DTMElementSubDisplayHandler::FindHandler(xAttr);
        if (hand)
            {
            hand->_Draw (element, xAttr, info, context);
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
bool DTMElementDisplayHandler::DrawPart (ViewContextR context, DTMDrawingInfo& info, bool hasHilite, DTMSubElementIter& iter)
    {
    bool ret = false;
    try
        {
        DTMElementSubDisplayHandler* hand = DTMElementSubDisplayHandler::FindHandler (iter);
        if (!hand || !hand->CanDraw (info.GetDTMDataRef (), context))
            return false;

        ElementHandle element = info.GetOriginalElement ();
        ElementRefP elRef = info.GetOriginalElement ().GetElementRef ();
        IPickGeom* pick = context.GetIPickGeom ();
        if (context.GetIPickGeom ())
            {
            GeomDetailP detail = pick ? &pick->GetGeomDetail () : nullptr;
            if (detail)
                detail->SetElemArg (iter.GetId ());
            }

        ElementHiliteState  hiliteState = HILITED_None;
        ElementHiliteState  prevHiliteState = elRef ? elRef->IsHilited (info.GetOriginalElement ().GetModelRef ()) : HILITED_None;
        double prevTrans = -1;

        if (element.GetElementRef ())
            hiliteState = GetHighlight (info.GetSymbologyElement ().GetElementRef (), iter.GetId ());

        if (prevHiliteState != HILITED_None)
            {
            bool removeHilite = true;

            // Keep hilite for boundarires.
            if (hiliteState == HILITED_None && iter.GetCurrentId ().GetHandlerId () == DTMElementFeaturesHandler::GetInstance ()->GetSubHandlerId ())
                {
                DTMElementFeaturesHandler::DisplayParams dp (iter);

                if (dp.GetTag () == DTMElementFeaturesHandler::Boundary)
                    removeHilite = false;
                }
            if (removeHilite && elRef)
                elRef->SetHilited (info.GetOriginalElement ().GetModelRef (), HILITED_None);

            context.CookElemDisplayParams (element);
            context.ActivateOverrideMatSymb ();
            }
        else
            context.CookElemDisplayParams (element);

        if (hasHilite && hiliteState == HILITED_None)
            {
            ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams ();
            prevTrans = elParams->GetTransparency ();
            elParams->SetTransparency (1 - ((1 - elParams->GetTransparency ()) * (0.2)));
            context.CookDisplayParams ();
            }

        ret = hand->_Draw (element, iter, info, context);

        if (hasHilite && hiliteState == HILITED_None)
            {
            ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams ();
            elParams->SetTransparency (prevTrans);
            context.CookDisplayParams ();
            }

        if (prevHiliteState != HILITED_None && elRef)
            elRef->SetHilited (info.GetOriginalElement ().GetModelRef (), prevHiliteState);

        }
    catch (...)
        {
        BeAssert (false && "Crash in drawing Part");
        }
    return ret;
    }

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
inline bvector <DTMSubElementId> GetIds (ElementHandleCR element, bool includeInvisible = true)
    {
    bvector <DTMSubElementId>  ids;

    ids.reserve (10);
    DTMSubElementIter iter (element, includeInvisible);
    for (; iter.IsValid(); iter.ToNext())
        {
        if (ids.capacity() == ids.size())
            ids.reserve (ids.capacity() + 10);
        ids.push_back (iter.GetCurrentId());
        }
    return ids;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
// ToDo Vancouvervoid DTMElementDisplayHandler::_ModifyingSubElement (EditElementHandleR handle, const DTMSubElementId & xAttrId, const DTMElementSubHandler::SymbologyParams& params) const
// ToDo Vancouver    {
// ToDo Vancouver    const DTMElementSubHandler::SymbologyParamsAndTextStyle* newDp = dynamic_cast<const DTMElementSubHandler::SymbologyParamsAndTextStyle*>(&params);
// ToDo Vancouver
// ToDo Vancouver    if (newDp)
// ToDo Vancouver        {
// ToDo Vancouver        RefCountedPtr<DTMElementSubHandler::SymbologyParams> oldParams = DTMElementSubHandler::GetParams (ElementHandle::XAttributeIter (handle, DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId(), xAttrId.GetId()));
// ToDo Vancouver        const DTMElementSubHandler::SymbologyParamsAndTextStyle* oldDp = dynamic_cast<const DTMElementSubHandler::SymbologyParamsAndTextStyle*>(oldParams.get());
// ToDo Vancouver        if (oldDp->GetTextStyleID () != newDp->GetTextStyleID ())
// ToDo Vancouver            AddDTMTextStyle (handle, newDp->GetTextStyleID (), xAttrId.GetId());
// ToDo Vancouver        }
// ToDo Vancouver    }


BentleyStatus DTMElementDisplayHandler::_GeneratePresentation (EditElementHandleR eh, ViewContextR viewContext)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;

    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, eh);
    if (DTMDataRef.IsNull())
        return ERROR;

    //An empty MrDTM cannot be drawn.
    if ((DTMDataRef != nullptr) &&
        (DTMDataRef->IsMrDTM() == true) &&
        (((MrDTMDataRef*)DTMDataRef.get())->GetMrDTMState() == MRDTM_STATE_EMPTY))
        return ERROR;

    DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(GetHull, viewContext));
    BcDTMP dtm = 0;

    if (dtmPtr != 0)
        dtm = dtmPtr->GetBcDTM();

    if (dtm)
        {
        DTMDrawingInfo info;

        GetDTMDrawingInfo (info, eh, DTMDataRef.get(), &viewContext);

        DTMStrokeForCacheHull hullStroker(dtm, info);
        hullStroker._StrokeForCache(eh, viewContext, viewContext.GetViewport() ? viewContext.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);

        if (!hullStroker.HasDisplayedSomething())
            {
            DrawScanRange (viewContext, eh, DTMDataRef);
            }
        }
    return SUCCESS;
    }

void DTMElementDisplayHandler::_Draw (ElementHandleCR element, ViewContextR context)
    {
    static bool inThematicDraw = false;
    DrawPurpose purpose = context.GetDrawPurpose ();
    ViewContext::ModelRefMark mark2 (context);

    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.

    // Create a DTM element from the XAttributes (this is is a very lightweight operation that
    // just assigns the dtm internal arrays to their addresses inside the XAttributes).
    if ( purpose == DrawPurpose::ChangedPre )
        {
        context.DrawElementRange(element.GetElementCP());
        return;
        }

    RefCountedPtr<DTMDataRef> DTMDataRef;

    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

    //An empty MrDTM cannot be drawn.
    if ((DTMDataRef != nullptr) &&
        (DTMDataRef->IsMrDTM() == true) &&
        (((MrDTMDataRef*)DTMDataRef.get())->GetMrDTMState() == MRDTM_STATE_EMPTY))
        {
        return;
        }

    if (DrawPurpose::FitView == context.GetDrawPurpose ())
        {
        // If this has a dynamicRange don't draw the range, this is probably because the DTM is empty.
        if (!element.GetElementCP()->hdr.dhdr.props.b.dynamicRange)
            DrawScanRange (context, element, DTMDataRef);

        return;
        }

    if (DTMDataRef != nullptr)
        {
        if (DTMDataRef->IsMrDTM())
            {
            RefCountedPtr<MrDTMDataRef> mrDTMDataRef((MrDTMDataRef*)DTMDataRef.get());

            //MrDTM should not be displayed
            if (mrDTMDataRef->CanDisplayMrDTM() == false)
                {
                //If the MrDTM is selected display its 3D bounding box.
                if (element.GetElementRef() != NULL && element.GetElementRef()->IsInSS(element.GetModelRef()))
                    {
                    DRange3d range3d;
                    mrDTMDataRef->GetExtents(range3d);

                    DTMUnitsConverter conv (element.GetModelRef());
                    conv.FullStorageToUors (range3d.low);
                    conv.FullStorageToUors (range3d.high);

                    ViewContext::ContextMark mark (context, element);

                    DPoint3d  p[8];
                    p[0].x = p[3].x = p[4].x = p[5].x = range3d.low.x;
                    p[1].x = p[2].x = p[6].x = p[7].x = range3d.high.x;
                    p[0].y = p[1].y = p[4].y = p[7].y = range3d.low.y;
                    p[2].y = p[3].y = p[5].y = p[6].y = range3d.high.y;
                    p[0].z = p[1].z = p[2].z = p[3].z = range3d.low.z;
                    p[4].z = p[5].z = p[6].z = p[7].z = range3d.high.z;
                    context.DrawBox(p, true);
                    }

                return;
                }

            //Check if the MrDTM display for the view is turned off.
            if ((context.GetViewport() != 0) &&
                (mrDTMDataRef->IsVisibleForView(context.GetViewport()->GetViewNumber()) == false))
                {
                return;
                }
            }
        bool hasDisplayedSomething = false;

        // Get overriding Symbology Element if there is one.
        ElementHandle symbologyEl;

        DgnModelRefP ref = nullptr;

        if (context.GetViewport ())
            ref = context.GetViewport()->GetRootModel ();

        /// is Override element but reference has been activated

        ElementHandle original = element;

        DTMDrawingInfo info;
        bool forceTriangleDraw = purpose == DrawPurpose::CutXGraphicsCreate || purpose == DrawPurpose::Measure || purpose == DrawPurpose::VisibilityCalculation || purpose == DrawPurpose::ComputeModelRefRange;

        GetDTMDrawingInfo (info, element, DTMDataRef.get(), &context);

        if(!info.IsVisible())
            {
            if (info.GetSymbologyElement().GetModelRef() != info.GetOriginalElement().GetModelRef())
                SetColourMap (context, info.GetOriginalElement().GetModelRef());
            return;
            }

        // Stop old elements from displaying.
        if (info.GetOriginalElement ().GetElementRef() != element.GetElementRef())
            return;

        symbologyEl = info.GetSymbologyElement ();

        int dsIndex = -1;

        DTMElementHandlerManager::GetThematicDisplayStyleIndex (symbologyEl, dsIndex);
        bool isHilited = false;

        if (context.GetCurrHiliteState () != HILITED_None || purpose == DrawPurpose::Unhilite)
            isHilited = true;

        if(isHilited && purpose != DrawPurpose::Flash)
            {
            if (!inThematicDraw)
                {
                IPickGeom* pick = context.GetIPickGeom();
                if (context.GetIPickGeom())
                    {
                    GeomDetailP detail = pick ? &pick->GetGeomDetail() : nullptr;
                    if (detail)
                        detail->SetElemArg(-2);
                    }

                // Use different caches for each part of the DTM.
                DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(GetHull, context));
                BcDTMP dtm = nullptr;

                if (dtmPtr.IsValid())
                    {
                    dtm = dtmPtr->GetBcDTM();
                    }

                if (dtm)
                    {
                    DTMStrokeForCacheHull hullStroker(dtm, info);
                    //context.DrawCached (element, hullStroker, 0, pickMode, DrawExpense::High);
                        hullStroker._StrokeForCache(element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                    if (!hullStroker.HasDisplayedSomething())
                        {
                        DrawScanRange (context, element, DTMDataRef);
                        }
                    }
                if (purpose == DrawPurpose::Hilite || purpose == DrawPurpose::Unhilite)
                    return;
                }
            }


        if (dsIndex != -1 && !inThematicDraw)
            {
            inThematicDraw = true;
            try
                {
                Int32          styleIndex = dsIndex;
                DisplayStyleCP displayStyle = nullptr;

                if (NULL != symbologyEl.GetModelRef() &&
                    NULL != (displayStyle = DisplayStyleManager::GetDisplayStyleByIndex (styleIndex, *(DgnFileP)symbologyEl.GetDgnFileP())))
                    {
                    context.PushDisplayStyle (displayStyle, symbologyEl.GetModelRef(), false);

                    // TFS# 103339 - If thematic material override exists, clear it here.
                    // Not a great fix.  If handlers are going to push display styles we'll need a method for notifying them of push/pop.
                    OvrMatSymbP overrideMatSymb = context.GetOverrideMatSymb();
                    overrideMatSymb->Clear ();
                    context.GetIViewDraw().ActivateOverrideMatSymb(overrideMatSymb);
                    }

                ElementHiliteState prevHiliteState = HILITED_None;
                ElementRefP elemRef = element.GetElementRef ();
                if (nullptr != elemRef)
                    {
                    prevHiliteState = element.GetElementRef ()->IsHilited (element.GetModelRef ());
                    if (prevHiliteState != HILITED_None)
                        element.GetElementRef()->SetHilited (element.GetModelRef(), HILITED_None);
                    }
                context.CookElemDisplayParams(element);
                context.ActivateOverrideMatSymb();
                if (! context.OverrideElementDisplay (element))
                    element.GetDisplayHandler()->Draw (element, context); // Stroke the element (and it's children)

                if (nullptr != elemRef && prevHiliteState != HILITED_None)
                    element.GetElementRef()->SetHilited (element.GetModelRef(), prevHiliteState);

                if (NULL != displayStyle)
                    context.PopDisplayStyle();
                }
            catch(...)
                {
                }
                inThematicDraw = false;
            }

        int displayPart = -1;
        if (purpose != DrawPurpose::FenceAccept)
            {
            bool hasHilite = HasHighlight (info.GetSymbologyElement().GetElementRef());
            DisplayPathCP path = context.GetSourceDisplayPath();
            HitPathCP hitPath = dynamic_cast<HitPathCP>(path);

            if (hitPath)
                displayPart = hitPath->GetGeomDetail().GetElemArg();

            ViewContext::ContextMark mark (context, element);
            std::list<DTMSubElementIter> subElements;

            {
            DTMSubElementIter iter ( symbologyEl,  forceTriangleDraw);
            bool needsContourSwap = false;
            bool hasMajorContours = false;

            for (; iter.IsValid(); iter.ToNext())
                {
                if ( inThematicDraw || forceTriangleDraw )
                    {
                    if (iter.GetCurrentId().GetHandlerId() == DTMElementTrianglesHandler::GetInstance()->GetSubHandlerId())
                        subElements.push_back (iter);
                    if (iter.GetCurrentId().GetHandlerId() == DTMElementRasterDrapingHandler::GetInstance()->GetSubHandlerId())
                        subElements.push_back (iter);
                    }
                else
                    {
                    if ((dsIndex != -1) &&
                        ((iter.GetCurrentId().GetHandlerId() == DTMElementTrianglesHandler::GetInstance()->GetSubHandlerId())
                        || (iter.GetCurrentId().GetHandlerId() == DTMElementRasterDrapingHandler::GetInstance()->GetSubHandlerId())
                        ))
                        {
                        DTMElementSubDisplayHandler* hand = DTMElementSubDisplayHandler::FindHandler(iter);
                        if (hand && hand->CanDraw (DTMDataRef.get(), context))
                            {
                            DTMElementSubHandler::SymbologyParams params (iter);

                            if (DTMElementSubDisplayHandler::SetSymbology (params, info, context))
                                hasDisplayedSomething = true;
                            }
                        continue;
                        }

                    if (iter.GetCurrentId().GetHandlerId() == DTMElementContoursHandler::GetInstance()->GetSubHandlerId())
                        {
                        DTMElementContoursHandler::DisplayParams params (iter);

                        if (params.GetIsMajor())
                            {
                            hasMajorContours = true;
                            subElements.push_back (iter);
                            }
                        else
                            {
                            if (hasMajorContours)
                                {
                                needsContourSwap = true;
                                subElements.push_front (iter);
                                }
                            else
                                subElements.push_back (iter);
                            }
                        }
                    else
                        subElements.push_back (iter);
                    }
                }
            }
            for(std::list<DTMSubElementIter>::iterator iter = subElements.begin(); iter != subElements.end(); iter++)
                {
                if ((displayPart == -1 || displayPart == iter->GetId()))
                    {
                    if (DrawPart (context, info, hasHilite, *iter))
                        hasDisplayedSomething = true;
                    }
                }

            }

        if (info.GetSymbologyElement().GetModelRef() != info.GetOriginalElement().GetModelRef())
            SetColourMap (context, info.GetOriginalElement().GetModelRef());

        if (!inThematicDraw && ((!hasDisplayedSomething || DrawPurpose::Pick == context.GetDrawPurpose()) && displayPart < 0 && (!isHilited || displayPart == -2)))
            {
            if (((DrawPurpose::Pick == context.GetDrawPurpose()) || (DrawPurpose::Flash == context.GetDrawPurpose())) &&
                 (DTMElementSubDisplayHandler::CanDoPickFlash(DTMDataRef, context.GetDrawPurpose()) == false))
                return;

            context.CookElemDisplayParams(element);

            IPickGeom* pick = context.GetIPickGeom();
            if (context.GetIPickGeom())
                {
                GeomDetailP detail = pick ? &pick->GetGeomDetail() : nullptr;
                if (detail)
                    detail->SetElemArg(-2);
                }
            // Use different caches for each part of the DTM.
            DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(GetHull, context));
            BcDTMP dtm = 0;

            if (dtmPtr != 0)
                dtm = dtmPtr->GetBcDTM();

            DTMElementFeaturesHandler::DisplayParams boundaryDisplayParams(element);

            if (DTMElementFeaturesHandler::GetSubElement (element, DTMElementFeaturesHandler::Boundary, boundaryDisplayParams))
                if (!DTMElementSubDisplayHandler::SetSymbology (boundaryDisplayParams, info, context) && !isHilited)    // do we want to draw if the level is off?
                    return;

            if (dtm)
                {
                DTMStrokeForCacheHull hullStroker(dtm, info);
                    hullStroker._StrokeForCache(element, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
                if (!hullStroker.HasDisplayedSomething())
                    {
                    DrawScanRange (context, element, DTMDataRef);
                    }
                }
            }
        }
    else
        {
        ElementHandle original;
        TMSymbologyOverrideManager::GetReferencedElement (element, original);
        // Only draw invalid elements for the original element and not for the active symbology.
        if (original.GetElementRef() == element.GetElementRef())
            {
            SCOverride          flags;
            Symbology           symb;
            ElemHeaderOverrides ovr;

            memset (&flags, 0, sizeof (flags));

            flags.color  = 1;
            flags.weight = 1;
            flags.style  = 1;

            symb.color   = 3;
            symb.style   = 2;
            symb.weight  = 4;

            ovr.Init (&flags, 0, 0, 0, (DgnElementClass) DgnElementClass::Primary, &symb, NULL);

            context.PushOverrides (&ovr);
            context.CookElemDisplayParams(element);
            context.ActivateOverrideMatSymb();

            DrawScanRange (context, element, DTMDataRef);
            }
        }
    }

StatusInt DTMElementDisplayHandler::_DrawCut (ElementHandleCR thisElm, ICutPlaneR cutPlane, ViewContextR context)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, thisElm);

    if (DTMDataRef.IsValid())
        {
        IDrawCutInfo*   drawCutInfo = context.GetIDrawCutInfo ();

        if (NULL != drawCutInfo)
            drawCutInfo->_SetWasNotCut (true);

        if (!thisElm.GetElementRef())
            return ERROR;

        if (NULL != drawCutInfo)
            drawCutInfo->_SetWasNotCut (false);

        DPlane3d dplane;
        cutPlane._GetPlane(dplane, true);

        if (fabs(dplane.normal.z) < mgds_fc_nearZero)
            {
            CutGraphicsContainerCP       cutGraphics;
            if (NULL == (cutGraphics = context.GetCutGraphicsCache (thisElm, context.GetCurrDisplayPath(), cutPlane)))
                {
                if (!cutPlane._OverlapsElement (thisElm.GetElementCP()))
                    return SUCCESS;         // Should we fail permanently?

                Transform trsf;
                DTMElementHandlerManager::GetStorageToUORMatrix (trsf, GetModelRef(thisElm), thisElm);

                ClipMask clipMask;
                DRange2d range2d;
                RotMatrix matrix;
                cutPlane._GetClipRange(clipMask, range2d, matrix);

                DRange3d planeRange = DRange3d::From(range2d.low.x, range2d.low.y, 0, range2d.high.x, range2d.high.y, 0);
                double left = planeRange.low.x;
                double right = planeRange.high.x;
                DVec3d xDir;

                matrix.GetColumn(xDir, 0);
                DPoint3d res[2];
                res[0] = DPoint3d::FromSumOf(dplane.origin, 1, xDir, left);
                res[1] = DPoint3d::FromSumOf(dplane.origin, 1, xDir, right);

                Transform trans;

                trans.InverseOf (trsf);
                trans.multiply(res, 2);

                DRange3d range;
                DTMDataRef->GetExtents (range);
                IDTM* idtm = DTMDataRef->GetDTMStorage (None, context);
                if (idtm)
                    {
                    DTMDrapedLinePtr result;

                    if (idtm->GetDTMDraping()->DrapeLinear(result, res, 2) == DTM_SUCCESS)
                        {
                        Int64 count = result->GetPointCount();
                        BeAssert(count <= INT_MAX);
                        DPoint3d* pts = (DPoint3d*)_alloca((int)count * sizeof(DPoint3d));
                        int ptNum = 0;
                        context.BeginAddToCutGraphicsCache();
                        XGraphicsRecorder xgRecorder(thisElm.GetModelRef());
                        xgRecorder.GetContext()->CookElemDisplayParams(thisElm);
                        xgRecorder.SetUseCache(true);

                        for (int i = 0; i < count; i++)
                            {
                            DTMDrapedLineCode code = DTMDrapedLineCode::External;

                            result->GetPointByIndex(pts[ptNum], nullptr, &code, i);

                            if (code == DTMDrapedLineCode::External || code == DTMDrapedLineCode::Void)
                                {
                                if (ptNum != 0)
                                    xgRecorder.GetContext()->DrawStyledLineString3d((int)ptNum, pts, nullptr);
                                ptNum = 0;
                                }
                            else
                                {
                                trsf.multiply(&pts[ptNum]);
                                ptNum++;
                                }
                            }
                        if (ptNum != 0)
                            xgRecorder.GetContext()->DrawStyledLineString3d((int)ptNum, pts, nullptr);


                        XGraphicsContainer const& xgContainer = xgRecorder.GetContainer();
                        bvector<XGraphicsContainer>  xgContainerVector;

                        xgContainer.ExtractPrimitives(xgContainerVector);

                        for (bvector<XGraphicsContainer>::const_iterator region = xgContainerVector.begin(); region != xgContainerVector.end(); ++region)
                            context.AddToCutGraphicsCache(thisElm, const_cast<XGraphicsContainer&>(*region), cutPlane);

                        if (NULL == (cutGraphics = context.EndAddToCutGraphicsCache(thisElm, cutPlane)))
                            {
                            BeAssert(false);
                            return ERROR;
                            }
                        }
                    }
                else
                    return ERROR;
                }
            context.DrawCutGraphicsCache (thisElm, context.GetCurrDisplayPath(), *cutGraphics, cutPlane);

            return SUCCESS;
            }
        }
        return T_Super::_DrawCut(thisElm, cutPlane, context);
    }

void DTMElementDisplayHandler::_GetPathDescription (ElementHandleCR element, WStringR string, const DisplayPath* path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr)
    {
    const HitPath* hitPath = dynamic_cast<const HitPath*>(path);
    if (hitPath)
        {
        int id = hitPath->GetGeomDetail().GetElemArg();

        ElementHandle original, symbologyEl;
        TMSymbologyOverrideManager::GetReferencedElement (element, original);
        TMSymbologyOverrideManager::GetElementForSymbology (original, symbologyEl, GetActivatedModel (element, nullptr));

        ElementHandle::XAttributeIter iter(symbologyEl, DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId(), id);
        if (iter.IsValid())
            {
            DTMElementSubDisplayHandler* hand = DTMElementSubDisplayHandler::FindHandler(iter);
            if (hand)
                {
                RefCountedPtr<DTMDataRef> DTMDataRef;
                DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

                if (DTMDataRef != nullptr)
                    {
                    HitPathLazyDTMDrawingInfoProvider ldip (element, DTMDataRef.get(), *hitPath);
                    WString subString;

                    ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler).GetDescription (element, string, 100);  // start with element's description
                    hand->_GetPathDescription (symbologyEl, iter, ldip, subString, *hitPath, levelStr, modelStr, groupStr, delimiterStr);
                    string.append(delimiterStr).append (subString);
                    if (levelStr && '\0' != levelStr[0]) string.append(delimiterStr).append(levelStr);
                    if (modelStr && '\0' != modelStr[0]) string.append(delimiterStr).append(modelStr);
                    if (groupStr && '\0' != groupStr[0]) string.append(delimiterStr).append(groupStr);
                    return;
                    }
                }
            }
        }
    ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler).GetDescription (element, string, 100);  // start with element's description
    if (levelStr && '\0' != levelStr[0]) string.append(delimiterStr).append(levelStr);
    if (modelStr && '\0' != modelStr[0]) string.append(delimiterStr).append(modelStr);
    if (groupStr && '\0' != groupStr[0]) string.append(delimiterStr).append(groupStr);
    }

SnapStatus DTMElementDisplayHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    SnapPathP   snap = context->GetSnapPath ();
    if (snap)
        {
        ElementHandle  elHandle (snap->GetPathElem(snapPathIndex), snap->GetRoot ());
        int id = snap->GetGeomDetail().GetElemArg();

        ElementHandle::XAttributeIter iter(elHandle, DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId(), id);
        if (iter.IsValid())
            {
            DTMElementSubDisplayHandler* hand = DTMElementSubDisplayHandler::FindHandler(iter);
            if (hand)
                {
                RefCountedPtr<DTMDataRef> DTMDataRef;
                DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, elHandle);

                if (DTMDataRef != nullptr)
                    {
                    ViewContextLazyDTMDrawingInfoProvider vclip (elHandle, DTMDataRef.get(), context);
                    return hand->_OnSnap (elHandle, iter, vclip, context, snapPathIndex);
                    }
                }
            }
        }

    return SnapStatus::Aborted;
    }

StatusInt DTMElementDisplayHandler::_OnTransform (EditElementHandleR element, TransformInfo const& transform)
    {
    //An MrDTM that cannot be displayed or is anchored cannot be transformed.
    RefCountedPtr<DTMDataRef> dtmDataRef;
    DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element);
    if ((dtmDataRef != 0) &&
        (dtmDataRef->IsMrDTM() == true) &&
        ((((MrDTMDataRef*)dtmDataRef.get())->CanDisplayMrDTM() == false) ||
         (((MrDTMDataRef*)dtmDataRef.get())->IsAnchored() == true)))
        {
        return SUCCESS;
        }

    if (dtmDataRef.IsNull() || !IsValidTransformation (transform.GetTransform()))
        return ERROR;
    Transform mtrx;

    DTMElementHandlerManager::GetStorageToUORMatrix (mtrx, element.GetModelRef(), element, false);

    Transform trfs;
    trfs.productOf (transform.GetTransform(), &mtrx);
    DTMElementHandlerManager::SetStorageToUORMatrix (trfs, element);

    UpdateDTMLastModified (element, BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble ());

    return SUCCESS;
    }

bool DTMElementDisplayHandler::_IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo)
    {
    //An MrDTM that cannot be displayed or is anchored cannot be transformed.
    RefCountedPtr<DTMDataRef> dtmDataRef;
    DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, elHandle);
    if ((dtmDataRef != 0) &&
        (dtmDataRef->IsMrDTM() == true) &&
        ((((MrDTMDataRef*)dtmDataRef.get())->CanDisplayMrDTM() == false) ||
         (((MrDTMDataRef*)dtmDataRef.get())->IsAnchored() == true)))
        {
        return false;
        }
    if (IsValidTransformation (tInfo.GetTransform()))
        return true;
    return false;
    }

StatusInt DTMElementDisplayHandler::_OnFenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    return ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
StatusInt getClipPoints (ClipPrimitiveCR clip, FenceParamsP fenceParams, bvector<DPoint3d>& fencePts)
    {
    ClipPolygonCP       clipPolygon;

    if (NULL == (clipPolygon = clip.GetPolygon()))
        return ERROR;

    fencePts.resize (clipPolygon->size ());
    for (size_t i = 0; i<clipPolygon->size (); i++)
        fencePts[i].Init (clipPolygon->at (i));

    if (NULL != clip.GetTransformFromClip())
        clip.GetTransformFromClip ()->multiply (fencePts.data (), (int)fencePts.size ());

    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/15
//=======================================================================================
StatusInt DTMElementDisplayHandler::_FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options)
    {
    return Handler::_FenceClip (inside, outside, eh, fp, options);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
StatusInt DTMElementDisplayHandler::_OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

    if (DTMDataRef != nullptr && !DTMDataRef->IsMrDTM() && fp->GetClipVector().IsValid() && !fp->GetClipVector()->empty())
        {
        bvector<DPoint3d> fencePts;

        getClipPoints (*fp->GetClipVector()->front(), fp, fencePts);

        DTMDrawingInfo drawingInfo;
        GetDTMDrawingInfo (drawingInfo, element, DTMDataRef.get(), nullptr); //ToDo??? LocalToActiveTransContext(fp->GetViewport(), element.GetModelRef()));

        Transform trans;

        trans.inverseOf (fp->GetTransform());
        for (auto& pt : fencePts)
            {
            DPoint3d tempPt = pt;
            if (!DTMDataRef->GetProjectedPointOnDTM (tempPt, element, fp->GetViewport (), pt))
                pt = tempPt;
            }

        trans.multiply (fencePts.data(), (int)fencePts.size());
        drawingInfo.FullUorsToStorage (fencePts.data (), (int)fencePts.size ());
        Transform trsf;
        getStorageToUORMatrix (trsf, GetModelRef(element), element);

        DTMPtr dtm;

        if (inside)
            {
            EditElementHandle insideEl;

            if (dtm.IsNull())
                DTMDataRef->GetDTMReferenceStorage (dtm);

            BcDTMP bcdtm = dtm->GetBcDTM();
            BcDTMPtr newDTM = nullptr;
            bcdtm->ClipByPointString (newDTM, fencePts.data (), (int)fencePts.size (), !fp->GetClipVector ()->front ()->IsMask () ? DTMClipOption::External : DTMClipOption::Internal);

            insideEl.SetModelRef (element.GetModelRef());
            if (newDTM.IsValid())
                {
                DTMDataRefXAttribute::ScheduleFromDtm (insideEl, &element, *newDTM, trsf, *element.GetModelRef(), true);
                TMReferenceXAttributeHandler::RemoveDTMDataReference (insideEl);
                CopySymbology (insideEl, element, nullptr, false);
                insideEl.GetElementDescrP()->h.elementRef = element.GetElementDescrCP()->h.elementRef;
                inside->Insert (insideEl);
                }
            }

        if (outside)
            {
            EditElementHandle outsideEl;

            if (dtm.IsNull())
                DTMDataRef->GetDTMReferenceStorage (dtm);

            BcDTMP bcdtm = dtm->GetBcDTM();
            BcDTMPtr newDTM = nullptr;
            bcdtm->ClipByPointString (newDTM, fencePts.data (), (int)fencePts.size (), fp->GetClipVector ()->front ()->IsMask () ? DTMClipOption::External : DTMClipOption::Internal);

            outsideEl.SetModelRef (element.GetModelRef());
            if (newDTM.IsValid())
                {
                DTMDataRefXAttribute::ScheduleFromDtm (outsideEl, &element, *newDTM, trsf, *element.GetModelRef (), true);
                TMReferenceXAttributeHandler::RemoveDTMDataReference (outsideEl);
                CopySymbology (outsideEl, element, nullptr, false);
                outsideEl.GetElementDescrP()->h.elementRef = element.GetElementDescrCP()->h.elementRef;
                outside->Insert (outsideEl);
                }
            }

        return SUCCESS;
        }
    return ERROR;
    }

void DTMElementDisplayHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    DropGeometry info (DropGeometry::OPTION_AppData);

    ElementAgenda out;

    // This is to force the lines in drop to be 3d and not 2d.
//    eeh.SetModelRef (nullptr);
    if (SUCCESS != ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler).Drop (eeh, out, info) || out.size() == 0)
        return;

    //// Need to delete all DTM Data.
    ElementHandle::XAttributeIter xAttrHandle (eeh);

    while (xAttrHandle.IsValid())
        {
        XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
        int id = xAttrHandle.GetId();

        if (handId.GetMajorId () == TMElementMajorId)
            {
            xAttrHandle.ToNext();
            eeh.ScheduleDeleteXAttribute (handId, id);
            }
        else
            xAttrHandle.ToNext();
        }

    ElementHandlerManager::RemoveHandlerFromElement (eeh);

    EditElementHandle& elm = eeh;
    NormalCellHeaderHandler::CreateOrphanCellElement (elm, nullptr, false, *eeh.GetModelRef ());

    for (ElementAgenda::iterator it = out.begin(); it != out.end(); it++)
        {
        it->GetHandler (MISSING_HANDLER_PERMISSION_Transform).ConvertTo2d (*it, flattenTrans, flattenDir);

        NormalCellHeaderHandler::AddChildElement (elm, *it);
        }

    NormalCellHeaderHandler::SetCellOriginAndRange (elm);

    //ToDo How to move the elm to eeh.
    }

StatusInt DTMElementDisplayHandler::_OnPreprocessCopy (EditElementHandleR element, ElementCopyContextP copyContext)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef(DTMDataRef, element);

    if ((DTMDataRef != nullptr) && (DTMDataRef->IsMrDTM() == true))
        {
        }
    else
        {
        CopySymbology (element, element, copyContext, true);
        return SUCCESS;
        }
    return T_Super::_OnPreprocessCopy (element, copyContext);
    }

StatusInt DTMElementDisplayHandler::_OnPostProcessCopy (EditElementHandleR element, ElementCopyContextP copyContext)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef(DTMDataRef, element);

    if ((DTMDataRef != nullptr) && (DTMDataRef->IsMrDTM() == true))
        {
        }
    else
        {
        CopySymbology (element, element, copyContext, true);
        }
    return T_Super::_OnPostProcessCopy (element, copyContext);
    }

struct XAttributeId
    {
    XAttributeHandlerId handId;
    int id;

    XAttributeId (XAttributeHandlerId handId, int id) : handId(handId), id(id)
        {
        }
    };
ITransactionHandler::PreActionStatus  DTMElementDisplayHandler::_OnAdd (EditElementHandleR element)
    {
    DTMDataRefXAttribute::ProcessAddedElementWithReference (element);
    ElementHandle::XAttributeIter xAttrHandle(element, DTMHeaderXAttributeHandler::GetXAttributeHandlerId (), 0);
    if (xAttrHandle.IsValid ())
        {
        Transform trsf;
        EditElementHandle dtmDataEl;
        dtmDataEl.SetModelRef (element.GetModelRef());
        getStorageToUORMatrix(trsf, element);
        DTMElementHandlerManager::CheckAndCreateElementDescr107 (dtmDataEl, DTMElement107Handler::GetElemHandlerId (), trsf, *dtmDataEl.GetModelRef ());
        DTMElementHandlerManager::AddToModelInOwnBlock (dtmDataEl, element.GetModelRef());
        ElementHandle::XAttributeIter xAttrHandle (element);
        bvector<XAttributeId> deleteMap;
        DTMXAttributeHandler::StartTMPersist ();
        while (xAttrHandle.IsValid ())
            {
            XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
            int id = xAttrHandle.GetId();
            bool moveXAttribute = false;
            if (handId.GetMajorId () == TMElementMajorId)
                {
                if ((handId.GetMinorId () >= XATTRIBUTES_SUBID_DTM_HEADER && handId.GetMinorId () <= XATTRIBUTES_SUBID_DTM_FLISTARRAY) || handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP)
                    moveXAttribute = true;
                }
            else if (handId.GetMajorId() == XATTRIBUTEID_XGraphicsName)
                moveXAttribute = true;

            if (moveXAttribute)
                {
                const void* data = xAttrHandle.PeekData ();
                UInt32 size = xAttrHandle.GetSize ();

                ITxnManager::GetCurrentTxn().AddXAttribute (dtmDataEl.GetElementRef(), handId, id, data, size);
                deleteMap.push_back (XAttributeId (handId, id));
                }
            xAttrHandle.ToNext();
            }

        for (bvector<XAttributeId>::iterator it = deleteMap.begin(); it != deleteMap.end(); it++)
            element.CancelWriteXAttribute (it->handId, it->id);
        TMReferenceXAttributeHandler::SetDTMDataReference (element, dtmDataEl);
        DTMXAttributeHandler::EndTMPersist ();
        }
    ScanLevelsAndAddOrRemove (element, true);

    ValidatePresentation (element);

    return PRE_ACTION_Ok;
    }

ITransactionHandler::PreActionStatus DTMElementDisplayHandler::_OnReplace (EditElementHandleR element, ElementHandleCR oldElement)
    {
    ElementHandle::XAttributeIter xAttrHandle(element, DTMHeaderXAttributeHandler::GetXAttributeHandlerId (), 0);

    if (xAttrHandle.IsValid ())
        {
        Transform trsf;
        EditElementHandle dtmDataEl;

        if (TMReferenceXAttributeHandler::GetDTMDataReference (oldElement, dtmDataEl))
            {
            DeleteDTMXAttributes (dtmDataEl);
            dtmDataEl.ReplaceInModel(dtmDataEl.GetElementRef());
            }
        else
            {
            dtmDataEl.SetModelRef (element.GetModelRef ());
            getStorageToUORMatrix (trsf, element);
            DTMElementHandlerManager::CheckAndCreateElementDescr107 (dtmDataEl, DTMElement107Handler::GetElemHandlerId (), trsf, *dtmDataEl.GetModelRef ());
            DTMElementHandlerManager::AddToModelInOwnBlock (dtmDataEl, element.GetModelRef ());
            }

        ElementHandle::XAttributeIter xAttrHandle (element);

        bvector<XAttributeId> deleteMap;
        DTMXAttributeHandler::StartTMPersist();
        while (xAttrHandle.IsValid())
            {
            XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
            int id = xAttrHandle.GetId();
            bool moveXAttribute = false;

            if (handId.GetMajorId () == TMElementMajorId)
                {
                if ((handId.GetMinorId () >= XATTRIBUTES_SUBID_DTM_HEADER && handId.GetMinorId () <= XATTRIBUTES_SUBID_DTM_FLISTARRAY) || handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP)
                    moveXAttribute = true;
                }
            else if (handId.GetMajorId () == XATTRIBUTEID_XGraphicsName)
                moveXAttribute = true;

            if (moveXAttribute)
                {
                const void* data = xAttrHandle.PeekData ();
                UInt32 size = xAttrHandle.GetSize ();

                ITxnManager::GetCurrentTxn().AddXAttribute (dtmDataEl.GetElementRef(), handId, id, data, size);
                deleteMap.push_back (XAttributeId (handId, id));
                }
            xAttrHandle.ToNext();
            }

        for (bvector<XAttributeId>::iterator it = deleteMap.begin (); it != deleteMap.end (); it++)
            {
            if (ERROR == element.CancelWriteXAttribute (it->handId, it->id))
                {
                element.ScheduleDeleteXAttribute (it->handId, it->id);
                }
            }

        TMReferenceXAttributeHandler::SetDTMDataReference (element, dtmDataEl);
        DTMXAttributeHandler::EndTMPersist();

        DTMXAttributeHandler::ReloadData (dtmDataEl.GetElementRef ());
        }


    ValidatePresentation (element);

    return PRE_ACTION_Ok;
    }

void DTMElementDisplayHandler::_OnDeleted (ElementHandleP element)
    {
    ScanLevelsAndAddOrRemove (*element, false);
    //RefCountedPtr<DTMDataRef> dataRefPtr;
    //DTMElementHandlerManager::GetDTMDataRef (dataRefPtr, *element);
    //if (dataRefPtr != nullptr)
    //    {
    //    //Ensure that the cache release the MrDTMDataRef associated with the element
    //    //so that the pointer to the iDTM file is released.
    //    DTMDataRefCachingManager::Remove(*element);
    //    }
    }

BentleyStatus DTMElementDisplayHandler::_ValidateElementRange (EditElementHandleR element, bool setZ)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;

    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);
    if (DTMDataRef != nullptr)
        {
        DRange3d range3d;
        if (DTMDataRef->GetExtents (range3d))
            {
            Transform trfs;
            DTMElementHandlerManager::GetStorageToUORMatrix(trfs, element.GetModelRef(), element, true);
            trfs.multiply(&range3d, &range3d);
            //
            MSElementP  msElem = element.GetElementP();
            elemUtil_setRange(&msElem->hdr.dhdr.range, &range3d, setZ, true);
            msElem->hdr.dhdr.props.b.dynamicRange = false;
            }
        else
            {
            range3d.init();
            MSElementP  msElem = element.GetElementP();
            elemUtil_setRange(&msElem->hdr.dhdr.range, &range3d, setZ, true);
            msElem->hdr.dhdr.props.b.dynamicRange = true;
            }

        UpdateDTMLastModified (element, DTMDataRef->GetLastModified ());
        }

    return SUCCESS;
    }

void DTMElementDisplayHandler::_GetDescription (ElementHandleCR element, WString& string, UInt32 desiredLength)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, element);

    //If not application activates the display of the MrDTM, add some information about
    //this fact.
    if ((DTMDataRef != nullptr) &&
        (DTMDataRef->IsMrDTM() == true))
        {
        if (DTMElementHandlerManager::GetMrDTMActivationRefCount() == 0)
            {
            string = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_ApplicationRequired);
            }
        else
            {
            string = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_ScalableTerrainModel);
            }
        }
    else
        if(DTMDataRef != nullptr)
            {
            GetTypeName ( string, desiredLength );
            }
        else
            {
            string = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_InvalidVersion);
            }

    WString name;
    DTMElementHandlerManager::GetName (element, name);
    if (!name.empty ())
        {
        size_t length = MIN (name.length (), desiredLength - string.length() - 3);
        wchar_t *buf = (wchar_t*)_alloca ((length + 1) * sizeof (wchar_t));
        wcsncpy (buf, name.c_str (), length);
        buf[length] = L'\0';
        string.append (L" : ").append (buf);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
bool DTMElementDisplayHandler::GetProjectedPointOnDTM (DPoint3dR pointOnDTM, ElementHandleCR thisElm, ViewportP viewport, DPoint3dCR testPoint)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, thisElm);

    if (!DTMDataRef.IsValid())
        { return false; }

    DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(None));
    BcDTMP dtm = 0;

    if (dtmPtr != 0)
        { dtm = dtmPtr->GetBcDTM(); }

    if (!dtm)
        { return false; }

    DTMDrawingInfo drawingInfo;
    DPoint3d startPt;
    DPoint3d endPt;

    GetDTMDrawingInfo (drawingInfo, thisElm, DTMDataRef.get(), nullptr); //ToDo LocalToActiveTransContext (viewport, thisElm.GetModelRef()));
    viewport->RootToNpc (&startPt, &testPoint, 1);
    endPt = startPt;
    endPt.z = 1;
    startPt.z = -1;

    viewport->NpcToRoot (&startPt, &startPt, 1);
    viewport->NpcToRoot (&endPt, &endPt, 1);

    drawingInfo.RootToStorage (startPt);
    drawingInfo.RootToStorage (endPt);

    if (startPt.x != endPt.x || startPt.y != endPt.y)
        {
        // Intersect line with the DTM Range
        DRange3d range;
        DPoint3d sP;
        DPoint3d eP;
        DPoint3d point;
        dtm->GetRange (range);

        endPt.x -= startPt.x;
        endPt.y -= startPt.y;
        endPt.z -= startPt.z;
        if (!range.intersectRay (nullptr, nullptr, &sP, &eP, &startPt, &endPt))
            { return false; }

        // Non Top View
        DPoint3d trianglePts[4];
        long drapedType;
        BC_DTM_OBJ* bcDTM = dtm->GetTinHandle();
        bool voidFlag;

        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&sP), ((DPoint3d*)&eP), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != false)
            { return false; }

        startPt = point;
        }

    // TopView
    DPoint3d trianglePts[4];
    int drapedType;

    double elevation;
    if (DTM_SUCCESS != dtm->DrapePoint (&elevation, nullptr, nullptr, trianglePts, drapedType, startPt))
        { return false; }

    startPt.z = elevation;
    drawingInfo.StorageToRoot (pointOnDTM = startPt);
    return true;
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementDisplayHandler::_OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source)
    {
    if (beforeUndoRedo)
        ScanLevelsAndAddOrRemove (*beforeUndoRedo, false);

    if (afterUndoRedo)
        ScanLevelsAndAddOrRemove (*afterUndoRedo, true);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementDisplayHandler::_OnUndoRedoFinished (ElementRefP element, bool isUndo)
    {
    ElementHandle elem (element, nullptr);
    elem.ResetHandler ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementDisplayHandler::_OnHistoryRestore (ElementHandleP after, ElementHandleP before, ChangeTrackAction actionStep, BentleyDgnHistoryElementChangeType effectiveChange)
    {
    if (before)
        ScanLevelsAndAddOrRemove (*before, false);
    if (after)
        ScanLevelsAndAddOrRemove (*after, true);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/11
//=======================================================================================
void DTMElementDisplayHandler::_OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag)
    {
    ScanLevelsAndAddOrRemove (*oldElement, false);
    ScanLevelsAndAddOrRemove (*newElement, true);

    if (newElement && newElement->GetElementType () != EXTENDED_ELM)
        {
        EditElementHandle el (*newElement, true);

        if (ElementHandlerManager::GetHandlerId (el).GetMajorId () == TMElementMajorId)
            {
            // This must have been dropped, delete all xattributes, and clear Handler.
            ElementHandlerManager::RemoveHandlerFromElement (el);
            el.ResetHandler (); el.GetHandler();

            ElementHandle::XAttributeIter xAttrHandle (el);

            while (xAttrHandle.IsValid())
                {
                XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
                int id = xAttrHandle.GetId();

                if (handId.GetMajorId () == TMElementMajorId || handId.GetMajorId () == LINKAGEID_TEXTSTYLE )
                    {
                    xAttrHandle.ToNext();
                    el.ScheduleDeleteXAttribute (handId, id);
                    }
                else
                    xAttrHandle.ToNext();
                }
            el.ReplaceInModel (el.GetElementRef());
            }
        }
    }

bool DTMElementDisplayHandler::HasHighlight(ElementRefP el)
    {
    if (!el)
        return false;
    HighlightRefAppData* data = (HighlightRefAppData*)el->FindAppData(HighlightRefAppData::GetKey());

    return data != nullptr;
    }

void DTMElementDisplayHandler::SetHighlight(ElementRefP el, UInt32 id, ElementHiliteState hiliteState)
    {
    if (!el)
        return;
    HighlightRefAppData* data = (HighlightRefAppData*)el->FindAppData(HighlightRefAppData::GetKey());

    if (data == nullptr)
        {
        if (hiliteState == HILITED_None)
            return;
        HeapZone& zone = el->GetHeapZone();
        data = new HighlightRefAppData();
        el->AddAppData (HighlightRefAppData::GetKey(), data, zone);
        }
    data->SetHighlight(id, hiliteState);
    if (data->IsEmpty())
        {
        // None left so remove it.
        el->DropAppData (HighlightRefAppData::GetKey());
        }
    }

ElementHiliteState DTMElementDisplayHandler::GetHighlight(ElementRefP el, UInt32 id)
    {
    if (el)
        {
        HighlightRefAppData* data = (HighlightRefAppData*)el->FindAppData(HighlightRefAppData::GetKey());

        if (data != nullptr)
            return data->GetHighlight(id);
        }
    return HILITED_None;
    }

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
IAnnotationHandlerP DTMElementDisplayHandler::_GetIAnnotationHandler ( ElementHandleCR el )
    {
    return &DTMAnnotationHandler::s_singleton;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/14
//=======================================================================================
bool DTMElementDisplayHandler::IsDTMElement (ElementHandleCR element)
    {
    return DTMElementHandlerManager::IsDTMElement (element);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/14
//=======================================================================================
StatusInt DTMElementDisplayHandler::GetDTMDataRef (RefCountedPtr<DTMDataRef>& outRef, ElementHandleCR element)
    {
    return DTMElementHandlerManager::GetDTMDataRef (outRef, element);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/14
//=======================================================================================
BentleyStatus DTMElementDisplayHandler::CreateDTMElement (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, DgnModelRefR modelRef, bool disposeDTM)
    {
    return (BentleyStatus)DTMElementHandlerManager::ScheduleFromDtmDirect (editHandle, templateElement, dtm, modelRef, disposeDTM);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElement107Handler::_OnDeleted(ElementHandleP element)
    {
// ToDo    RefCountedPtr<DTMDataRef> dataRefPtr;
// ToDo    DTMElementHandlerManager::GetDTMDataRef (dataRefPtr, *element);
// ToDo    if (dataRefPtr != nullptr)
// ToDo        {
// ToDo        //Ensure that the cache release the MrDTMDataRef associated with the element
// ToDo        //so that the pointer to the iDTM file is released.
// ToDo        DTMDataRefCachingManager::Remove(*element);
// ToDo        }
    }

/*--------------------------------------------------------------------------------------*
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class TMDeleteManipulator : public Bentley::RefCounted <IDeleteManipulator>
    {
    protected: virtual StatusInt _OnDelete (EditElementHandleR elHandle) override
        {
        elHandle.SetElementDescr (NULL, false, false);
        return SUCCESS;
        }
    protected: bool _AcceptElement (ElementHandleCR elHandle, WStringP cantAcceptReason, ElementAgendaP agenda)
        {
        return true;
        }
    public:
        static TMDeleteManipulator* Create () { return new TMDeleteManipulator (); }
    }; // End TMDeleteManipulator class

#define SINGLE_INSTANCE_DEFINE(_classname_) \
protected: _classname_ (void) {} \
public: static _classname_*& _Instance (void) {static _classname_* s_inst = 0; return s_inst; } \
        static _classname_& GetInstance (void) \
            { return 0!=_Instance() ? (*_Instance()) : *(_Instance() = new _classname_()); } \
        static void ReleaseInstance (void) \
            { delete _Instance(); _Instance() = 0;}

/*--------------------------------------------------------------------------------------*
* @bsiclass                                     Piotr.Slowinski                 09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct TMHandlerDeleteManipulatorExtension : IDeleteManipulatorExtension
    {
    SINGLE_INSTANCE_DEFINE (TMHandlerDeleteManipulatorExtension)

    //=======================================================================================
    // @bsimethod                                            Sylvain.Pucci      08/2005
    //=======================================================================================
    virtual IDeleteManipulatorPtr _GetIDeleteManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) override
        {
        return TMDeleteManipulator::Create ();
        }
    }; // End TMHandlerDeleteManipulatorExtension struct

DTMSubElementContext::DTMSubElementContext ()
    {
    drawingInfo = nullptr;
    }
DTMSubElementContext::~DTMSubElementContext ()
    {
    if (drawingInfo)
        delete drawingInfo;
    }

static DTMElementDisplayHandler& s_DTMElementDisplayHandlerSingleton = ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler);
static DTMElement107Handler& s_DTMElement107HandlerSingleton = ELEMENTHANDLER_INSTANCE (DTMElement107Handler);
ELEMENTHANDLER_DEFINE_MEMBERS (DTMElementDisplayHandler);
ELEMENTHANDLER_DEFINE_MEMBERS (DTMElement107Handler);

static MrDTMElementDisplayHandler& s_MrDTMElementDisplayHandlerSingleton = ELEMENTHANDLER_INSTANCE (MrDTMElementDisplayHandler);
ELEMENTHANDLER_DEFINE_MEMBERS (MrDTMElementDisplayHandler);

void Initializations ()
    {
    ElementHandlerManager::RegisterHandler (s_DTMElementDisplayHandlerSingleton.GetElemHandlerId(), ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler));
    ITransformManipulatorExtension::RegisterExtension (ELEMENTHANDLER_INSTANCE (DTMElementDisplayHandler), *new DTMElementHandlerTransformManipulatorExtension());
    IDeleteManipulatorExtension::RegisterExtension(ELEMENTHANDLER_INSTANCE(DTMElementDisplayHandler), TMHandlerDeleteManipulatorExtension::GetInstance());

    ElementHandlerManager::RegisterHandler(s_DTMElement107HandlerSingleton.GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(DTMElement107Handler));
    IDeleteManipulatorExtension::DropExtension(ELEMENTHANDLER_INSTANCE(DTMElement107Handler));

    ElementHandlerManager::RegisterHandler (s_MrDTMElementDisplayHandlerSingleton.GetElemHandlerId(), ELEMENTHANDLER_INSTANCE (MrDTMElementDisplayHandler));
    ITransformManipulatorExtension::RegisterExtension(ELEMENTHANDLER_INSTANCE(MrDTMElementDisplayHandler), *new MrDTMElementTransformManipulatorExtension());
    IIModelPublishExtension::RegisterExtension(ELEMENTHANDLER_INSTANCE(MrDTMElementDisplayHandler), *new MrDTMElementIModelPublishExtension());

    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE