/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMElementHandlerManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <DgnPlatform\ElementUtil.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include "MrDTMDataRef.h"

#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>


USING_NAMESPACE_RASTER
   
enum {MAX_ELEMENTBLOCK_BYTES = 500*K};

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void DTMRegisterDisplayHandlers();
void Initializations ();

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
int DTMElementHandlerManager::s_mrDTMactivationRefCount = 0;
DTMElementHandlerManager::IDTMElementPowerPlatformExtension* DTMElementHandlerManager::s_DTMElementPowerPlatformExtension = nullptr;
DTMElementHandlerManager::IDTMIsFriendModelExtension* DTMElementHandlerManager::s_DTMIsFriendModel = nullptr;

IMultiResolutionGridManagerCreatorPtr DTMElementHandlerManager::s_multiResolutionGridManagerCreator = 0;
IRasterTextureSourceManager*          DTMElementHandlerManager::s_rasterTextureSourceManagerP = 0;
bool                                  DTMElementHandlerManager::s_isDrawForAnimation = false; 



// DTMDisplayParamsXAttributeHandler
//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 04/10
//=======================================================================================
class DTMDisplayParamsXAttributeHandler : public XAttributeHandler, public IXAttributeTransactionHandler
    {
    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: static XAttributeHandlerId GetXAttributeHandlerId() { return XAttributeHandlerId(TMElementMajorId, XATTRIBUTES_SUBID_DTM_DISPLAYPARAMS); }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: virtual IXAttributeTransactionHandler* _GetIXAttributeTransactionHandler() { return this; }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: void DeleteCache (ElementRefP elemRef, UInt32 id)
        {
        ElementHandle dataEl (elemRef, nullptr);

        if (!dataEl.GetElementCP()->hdr.ehdr.isGraphics)
            {
            ElementHandle originalEl;
            TMSymbologyOverrideManager::GetReferencedElement (dataEl, originalEl);

            RedrawElement (originalEl);
            dataEl = originalEl;
            }

        DTMDisplayCacheManager::DeleteCacheElem (dataEl, id);

        // ToDo Need to look at this. Redraw Element
        XAttributeHandlerId handlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_OVERRIDEDISPLAYREF);
        ElementHandle::XAttributeIter xAttrHandle (dataEl, handlerId, 1);

        void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
        if (data)
            {
            DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
            short version;

            source.get (&version);
            PersistentElementPath pep;
            pep.Load (source);
            dataEl = pep.EvaluateElement (GetModelRef (dataEl));

            RedrawElement (dataEl);
            }
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: virtual void _OnPreDelete (XAttributeHandleCR xAttr, TransactionType type) override
        {
        DeleteCache (xAttr.GetElementRef(), xAttr.GetId());
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: virtual void _OnPreModifyData (XAttributeHandleCR xAttr, void const* newData, UInt32  start, UInt32 length, TransactionType type) override
        {
        DeleteCache (xAttr.GetElementRef(), xAttr.GetId());
        }

    public: virtual void _OnPreReplaceData (XAttributeHandleCR xAttr, void const* newData, UInt32 newSize, TransactionType type) override
        {
        DeleteCache (xAttr.GetElementRef(), xAttr.GetId());
        }
    };

static DTMHeaderXAttributeHandler s_dtmHeaderHandlerInfoXAttributeHandler;
static DTMHeaderXAttributeHandler s_dtmHeaderHandlerInfoXAttributeHandler2;

static DTMDisplayParamsXAttributeHandler s_dtmDisplayParamsXAttributeHandler;


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
int registerElementHandlers ()
    {   
    LogInfo(L"registering ElementHandlers.");

    // Register XAttributes Handlers.
    XAttributeHandlerManager::RegisterHandler(DTMDisplayParamsXAttributeHandler::GetXAttributeHandlerId(), &s_dtmDisplayParamsXAttributeHandler);
    XAttributeHandlerManager::RegisterHandler(DTMHeaderXAttributeHandler::GetXAttributeHandlerId(), &s_dtmHeaderHandlerInfoXAttributeHandler);

    XAttributeHandlerManager::RegisterHandler(XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_TRANSLATION), &s_dtmHeaderHandlerInfoXAttributeHandler2);

    return  SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/15
//=======================================================================================
void DTMElementHandlerManager::SetIsFriendModelExtension (DTMElementHandlerManager::IDTMIsFriendModelExtension* value)
    {
    s_DTMIsFriendModel = value;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElementHandlerManager::Initialize (DTMElementHandlerManager::IDTMElementPowerPlatformExtension* DTMElementPowerPlatformExtension)
    {
    // Register the handlers
    DTMRegisterDisplayHandlers();
    Initializations ();
    s_DTMElementPowerPlatformExtension = DTMElementPowerPlatformExtension;
    registerElementHandlers ();

    // Initialize the XAttributes handler
    DTMBinaryData::Initialize();

    DTMDisplayCacheManager::Initialize();
    

    MrDTMXAttributeHandler::GetInstance()->RegisterHandlers();  
        
#ifdef ToDoDoWeNeed
    SystemCallback::SetFileSaveAsFunction (onSystemFunc_FileSave);
    SystemCallback::SetFileSaveFunction (onSystemFunc_FileSave);
    SystemCallback::SetCompressDgnFileFunction (onCompressFile);
#endif

    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
bool DTMElementHandlerManager::IsDTMElement(ElementHandleCR element)
    {
    if (dynamic_cast<DTMElementHandler*>(&element.GetHandler()))
        return true;

    ElementHandle original;

    if (TMSymbologyOverrideManager::GetReferencedElement (element, original))
        return IsDTMElement (original);
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMElementHandlerManager::GetDTMDataRef(RefCountedPtr<DTMDataRef>& outRef, ElementHandleCR element)
    {
    ElementHandle dataElement;

    // If this Element has the DTMHeader scheduled then we must use a cached version.
    if (DTMDataRefXAttribute::HasScheduledDTMHeader (element))
        {
        // Get Temporary DTMDataRef. No caching at the moment.
        outRef = DTMDataRefXAttribute::FromElemHandle (element, element);
        return SUCCESS;
        }

    outRef = DTMDataRef::GetDTMAppData (element);
    if (outRef.IsValid ())
        return SUCCESS;

    outRef = MrDTMDataRef::FromElemHandle(element);
    if (outRef.IsValid ())
        return SUCCESS;

    // First check to see if this element has a DTMData.
    LogDebugV(L"GetDTMDataRef %x", element.GetElementRef());
    if (FindDTMData (element, dataElement))
        {
        outRef = DTMDataRef::GetDTMAppData (dataElement);
        if (outRef.IsValid ())
            return SUCCESS;

        outRef = DTMDataRefXAttribute::FromElemHandle (element, dataElement);

        if (outRef.IsValid())
            return SUCCESS;

        //if (dataEl.IsPersistent() && (dataEl.GetElementRef() != nullptr && dataEl.GetElementCP()->ehdr.uniqueId != 0))
        //    {
        //    LogDebugV(L"GetDTMDataRef GotDTMDataRef %x", dataEl.GetElementRef());
        //    DTMDataRefCache* cache = DTMDataRefCachingManager::Get (dataEl);

        //    if (cache)
        //        {
        //        outRef = cache->GetDataRef (element, dataElement);
        //        if (outRef.IsValid ())
        //            return SUCCESS;
        //        }
        //    }

        //LogDebugV(L"GetDTMDataRef No Cache for %x", dataEl.GetElementRef());

        //outRef = DTMDataRefXAttribute::FromElemHandle (element, dataElement);

        //if (outRef.IsValid())
        //    return SUCCESS;
        }

    if (TMSymbologyOverrideManager::GetReferencedElement (element, dataElement))
        return GetDTMDataRef (outRef, dataElement);
    return ERROR;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood 04/10
//=======================================================================================
bool DTMElementHandlerManager::FindDTMData(ElementHandleCR element, ElementHandleR dataEl)
    {
    // ToDo?
    //if (ElementHandle::XAttributeIter(element, DTMHeaderXAttributeHandler::GetXAttributeHandlerId (), 0).IsValid())
    //    {
    //    dataEl = element;
    //    return true;
    //    }

    if (TMReferenceXAttributeHandler::GetDTMDataReference(element, dataEl))
        {
        if (dataEl.GetElementRef() == element.GetElementRef())
            return true;
        while (TMReferenceXAttributeHandler::GetDTMDataReference(dataEl, dataEl))
            {
            }
        return true;
        }

    if (dynamic_cast<DTMElementHandler*>(&element.GetHandler()))
        {
        dataEl = element;
        return true;
        }

    if (dynamic_cast<DTMElement107Handler*>(&element.GetHandler()))
        {
        dataEl = element;
        return true;
        }
    return false;
    }
    
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/10
//=======================================================================================
StatusInt DTMElementHandlerManager::ScheduleReplaceDtm (EditElementHandleR editHandle, BcDTMR dtm, bool disposeDTM)
    {
    RefCountedPtr<DTMDataRef> dataRef;
    DTMElementHandlerManager::GetDTMDataRef (dataRef, editHandle);
    DTMDataRefXAttribute* xAttributeDTMDataRef = dynamic_cast<DTMDataRefXAttribute*>(dataRef.get());
    if (xAttributeDTMDataRef)
        {
        return xAttributeDTMDataRef->ReplaceDTM(dtm, disposeDTM);
        }
    return ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElementHandlerManager::ScheduleFromMrDtmFile(DgnModelRefP dgnModelRefP, EditElementHandleR editHandle, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation)
    {
    MrDTMDataRef::ScheduleFromDtmFile(dgnModelRefP, editHandle, mrdtmDocumentPtr, inCreation);
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMElementHandlerManager::ScheduleFromDtm (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, TransformCR storageTransformation, DgnModelRefR modelRef, bool disposeDTM)
    {
    Transform trsf;
    Transform dtmTransform;

    if (!dtm.GetTransformation (dtmTransform))
        trsf.productOf (&storageTransformation, &dtmTransform);
    else
        trsf = storageTransformation;

    if (!trsf.IsIdentity() && !trsf.isUniformScale (nullptr, nullptr) )
        {
        // This is an invalid transformation.
        BeAssert (true);
        return ERROR;
        }

    if (DTMDataRefXAttribute::ScheduleFromDtm(editHandle, templateElement, dtm, trsf, modelRef, disposeDTM) != SUCCESS)
        return ERROR;

    DTMElementHandlerManager::CreateDefaultDisplayElements (editHandle);
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMElementHandlerManager::ScheduleFromDtmDirect (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, DgnModelRefR modelRef, bool disposeDTM)
    {
    Transform trsf;

    trsf.InitIdentity();

    return ScheduleFromDtm (editHandle, templateElement, dtm, trsf, modelRef, disposeDTM);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/11
//=======================================================================================
void DTMElementHandlerManager::AddToModelInOwnBlock (EditElementHandleR element, DgnModelRefP model)
    {
    if (element.GetDgnModelP()->GetControlElementsP () && element.GetDgnModelP()->GetControlElementsP ()->GetLastBlock())
        element.GetDgnModelP()->GetControlElementsP ()->GetLastBlock()->SetBlockFull();
    element.SetModelRef (model);
    element.AddToModel ();
    if (element.GetDgnModelP()->GetControlElementsP () && element.GetDgnModelP()->GetControlElementsP ()->GetLastBlock())
        element.GetDgnModelP()->GetControlElementsP ()->GetLastBlock()->SetBlockFull();
    }

void DTMElement_InitializeElement (EditElementHandleR eh, ElementHandleCP teh, DgnModelRefP destmodel, bool is3d, bool isComplexHeader)
    {
    MSElement elm;

    memset (&elm, 0, sizeof (DTMElm));
    ElementUtil::SetRequiredFields (elm, EXTENDED_ELM, LEVEL_DEFAULT_LEVEL_ID, false, is3d ? ElementUtil::ELEMDIM_3d: ElementUtil::ELEMDIM_2d);

    elm.ehdr.elementSize = elm.ehdr.attrOffset = sizeof (DTMElm) / 2;

    elm.hdr.dhdr.props.b.n = 1;

    MSElementDescrP edP = MSElementDescr::Allocate (elm, destmodel);

    if (eh.PeekElementDescrCP ())
        eh.ReplaceElementDescr (edP);
    else
        eh.SetElementDescr (edP, true, false);
    //initializeElement (eh, 0, EXTENDED_ELM, true, is3d, sizeof (DTMElm), false, true);
    //ElementUtil::InitializeGraphicsElement (eh, destmodel, EXTENDED_ELM, is3d, sizeof(DTMElm), false, true);


    eh.GetElementP ()->ehdr.isComplexHeader = isComplexHeader;
    eh.GetElementP ()->extendedElm.componentCount = 0;

    if (NULL == teh || !teh->IsValid ())
        return;

    eh.GetElementP ()->ehdr.level = teh->GetElementCP ()->ehdr.level;

    if (!teh->GetElementCP ()->ehdr.isGraphics)
        return;

    eh.GetElementP ()->hdr.dhdr.symb = teh->GetElementCP ()->hdr.dhdr.symb;
    eh.GetElementP ()->hdr.dhdr.props.b.elementClass = teh->GetElementCP ()->hdr.dhdr.props.b.elementClass;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElementHandlerManager::CheckAndCreateElementDescr (EditElementHandleR elemHandle, ElementHandleCP templateElement, const ElementHandlerId & handlerId, TransformCR trsf, DgnModelRefR modelRef)
    {
    ElementRefP elRef = elemHandle.GetElementRef ();
    if (elRef == nullptr)
        {
        DgnModelP dgnCache = modelRef.GetDgnModelP();

        // Creates a type 106 element
        DTMElement_InitializeElement(elemHandle, templateElement, &modelRef,  (!dgnCache || dgnCache->Is3d() == 0) ? false : true, false);
        
        // Make the element snappable
        elemHandle.GetElementP ()->hdr.dhdr.props.b.s = 0;

        ElementHandlerXAttribute handlerTag (handlerId, MISSING_HANDLER_PERMISSION_None);
        ElementHandlerManager::AddHandlerToElement (elemHandle, handlerTag);

        SetStorageToUORMatrix(trsf, elemHandle);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
void DTMElementHandlerManager::CreateDefaultDisplayElements (EditElementHandleR element)
    {    
    ElementHandlerId elementHandlerID(ElementHandlerManager::GetHandlerId(element));

    if (DTMElementTrianglesHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementTrianglesHandler::GetInstance()->CreateDefaultElements (element, true);

    if (DTMElementContoursHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementContoursHandler::GetInstance()->CreateDefaultElements (element, false);         

    if (DTMElementSpotsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementSpotsHandler::GetInstance()->CreateDefaultElements (element, false);
        
    if (DTMElementFeatureSpotsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))        
        DTMElementFeatureSpotsHandler::GetInstance()->CreateDefaultElements (element, false);
        
    if (DTMElementFlowArrowsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))        
        DTMElementFlowArrowsHandler::GetInstance()->CreateDefaultElements (element, false);
        
    if (DTMElementLowPointsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementLowPointsHandler::GetInstance()->CreateDefaultElements (element, false);
        
    if (DTMElementHighPointsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementHighPointsHandler::GetInstance()->CreateDefaultElements (element, false);
                
#ifdef INCLUDE_CATCHMENT
    if (DTMElementCatchmentAreasHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementCatchmentAreasHandler::GetInstance()->CreateDefaultElements (element, false);

    if (DTMElementPondsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementPondsHandler::GetInstance()->CreateDefaultElements (element, false);        
#endif

    if (DTMElementFeaturesHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementFeaturesHandler::GetInstance()->CreateDefaultElements (element, false);        

    if (DTMElementRegionsHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        {
        DTMElementRegionsHandler::GetInstance()->CreateDefaultElements (element, false);
        SUBDISPLAYHANDLER_INSTANCE(DTMElementRegionsDisplayHandler)._CreateDefaultElements (element, false);
        }

    if (DTMElementRasterDrapingHandler::GetInstance()->_IsSupportedFor(elementHandlerID))
        DTMElementRasterDrapingHandler::GetInstance()->CreateDefaultElements (element, false);               
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElementHandlerManager::CheckAndCreateElementDescr107 (EditElementHandleR elemHandle, const ElementHandlerId & handlerId, TransformCR trsf, DgnModelRefR modelRef)
    {
    ElementRefP elRef = elemHandle.GetElementRef ();
    if (elRef == nullptr)
        {
        // Creates a type 106 element
        ExtendedNonGraphicsHandler::InitializeElement (elemHandle, NULL, &modelRef);

        SetStorageToUORMatrix(trsf, elemHandle);

        ElementHandlerXAttribute handlerTag (handlerId, MISSING_HANDLER_PERMISSION_All_);
        ElementHandlerManager::AddHandlerToElement (elemHandle, handlerTag);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
//void DTMElementHandlerManager::ScheduleFromDtmReference(EditElementHandleR editHandle, ElementHandleCR dtmHandle)
//    {
//    ElementHandle dtmDataHandle;
//
//    if (DTMElementHandlerManager::FindDTMData(dtmHandle, dtmDataHandle))
//        {
//        Transform trsf;
//
//        getStorageToUORMatrix (trsf, dtmHandle.GetModelRef(), dtmHandle, false);
//
//        // Need to sort out differences between models (if any)
//        //double srcUorPerMeter = mdlModelRef_getUorPerMeter (dtmHandle.GetModelRef ());
//        //double destUorPerMeter = mdlModelRef_getUorPerMeter (editHandle.GetModelRef ());
//
//        //DPoint3d srcPtGO;
//        //DPoint3d destPtGO;
//        //mdlModelRef_getGlobalOrigin (dtmHandle.GetModelRef (), &srcPtGO);
//        //mdlModelRef_getGlobalOrigin (editHandle.GetModelRef (), &destPtGO);
//        DTMElementHandlerManager::CheckAndCreateElementDescr(editHandle, DTMElementHandler::GetElemHandlerId(), trsf);
//        if (dtmDataHandle.GetDgnModel() == dtmHandle.GetDgnModel())
//            TMReferenceXAttributeHandler::SetDTMDataReference(editHandle, dtmDataHandle);
//        else
//            TMReferenceXAttributeHandler::SetDTMDataReference (editHandle, dtmHandle);
//
//        DTMElementHandlerManager::CreateDefaultDisplayElements (editHandle);
//
//        if (editHandle.GetDisplayHandler())
//            editHandle.GetDisplayHandler()->ValidateElementRange (editHandle, true);
//        }
//    }

#ifdef ToDo_DTMIterator
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
DTMElementIterator* DTMElementHandlerManager::ScanForDTMInModel(DgnModelRefP modelRef)
    {
    return new DTMElementIterator(modelRef, false);
    }
#endif

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
void DTMElementHandlerManager::ActivateMrDTM()
    {
    s_mrDTMactivationRefCount += 1;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
void DTMElementHandlerManager::DeactivateMrDTM()
    {
    s_mrDTMactivationRefCount -= 1;
    BeAssert(s_mrDTMactivationRefCount >= 0);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
StatusInt DTMElementHandlerManager::ConvertMrDTMtoSingleResDTM(EditElementHandleR outputEh, EditElementHandleCR inputEh, long maxNbPointsInDTM, DgnModelRefP modelRef)
    {   
    RefCountedPtr<DTMDataRef> dtmDataRef;
    if (DTMElementHandlerManager::GetDTMDataRef(dtmDataRef, inputEh) != SUCCESS)
        return ERROR;     

    BeAssert(dtmDataRef->IsMrDTM());
    
    //Get the data of one sub-resolution that will be used as to represent the MrDTM in the iModel.
    BcDTMPtr singleResolutionDtm;
    MrDTMDataRef*         mrDtmDataRefP = (MrDTMDataRef*)(dtmDataRef.get());

    if(!mrDtmDataRefP->GetDtmForSingleResolution(singleResolutionDtm, maxNbPointsInDTM))
        return ERROR;

    return CreateDTMFromMrDTM(outputEh, inputEh, singleResolutionDtm, modelRef);
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 05/12
//=======================================================================================
StatusInt DTMElementHandlerManager::CreateDTMFromMrDTM(EditElementHandleR    tmEditHandle,
                                                       EditElementHandleCR   stmEditHandle,
                                                       BcDTMPtr singleResolutionDtm,                                                       
                                                       DgnModelRefP          modelRef)
    {    
    tmEditHandle.SetModelRef(modelRef);  
    
    Transform trsf;

    getStorageToUORMatrix(trsf, stmEditHandle);       

    DTMElementHandlerManager::ScheduleFromDtm (tmEditHandle, nullptr, *singleResolutionDtm, trsf, *modelRef, false);

    //Synchronize symbology                   
    ElementPropertiesGetter stmElemProperties(stmEditHandle);
    ElementPropertiesSetter tmElemProperties;    

    tmElemProperties.SetLevel(stmElemProperties.GetLevel());
    tmElemProperties.SetElementClass(stmElemProperties.GetElementClass());
    ElementPropertiesSetter::SetLocked(tmEditHandle, ElementPropertiesGetter::GetLocked(stmEditHandle));
    ElementPropertiesSetter::SetGraphicGroup(tmEditHandle, ElementPropertiesGetter::GetGraphicGroup(stmEditHandle));

   
    //viewIndepend synchronization  
    tmEditHandle.GetElementP()->hdr.dhdr.props.b.r = stmEditHandle.GetElementCP()->hdr.dhdr.props.b.r;
    //solidHole synchronization
    tmEditHandle.GetElementP()->hdr.dhdr.props.b.h = stmEditHandle.GetElementCP()->hdr.dhdr.props.b.h;
    
    LineStyleParams lsParams;

    tmElemProperties.SetColor(stmElemProperties.GetColor());
    tmElemProperties.SetWeight(stmElemProperties.GetWeight());
    tmElemProperties.SetLinestyle(stmElemProperties.GetLineStyle(&lsParams), &lsParams);
    tmElemProperties.SetTransparency(stmElemProperties.GetTransparency());
   
    tmElemProperties.Apply (tmEditHandle);

    ElementCopyContextP copyContext = 0;    
    CopySymbology(tmEditHandle, (ElementHandleCR)stmEditHandle, copyContext);    

    if (!DTMElementTrianglesHandler::GetInstance()->GetVisibility((ElementHandle)stmEditHandle) && 
        !DTMElementContoursHandler::GetInstance()->GetVisibility((ElementHandle)stmEditHandle) &&
        DTMElementRasterDrapingHandler::GetInstance()->GetVisibility((ElementHandle)stmEditHandle))
        {
        DTMElementTrianglesHandler::GetInstance()->SetVisibility(tmEditHandle, true);        
        }  
      
    return SUCCESS;          
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/10
//=======================================================================================
void DTMElementHandlerManager::SetMultiResolutionGridManagerCreator(IMultiResolutionGridManagerCreatorPtr multiResolutionGridManagerCreator)
    {                  
    s_multiResolutionGridManagerCreator = multiResolutionGridManagerCreator;
    }

void DTMElementHandlerManager::SetRasterTextureSourceManager(IRasterTextureSourceManager* rasterTextureSourceManagerP)
    {
    s_rasterTextureSourceManagerP = rasterTextureSourceManagerP;
    }

 bool DTMElementHandlerManager::IsDrawForAnimation()
    {
    return s_isDrawForAnimation;
    }

void DTMElementHandlerManager::SetDrawForAnimation(bool isDrawForAnimation)
    {
    s_isDrawForAnimation = isDrawForAnimation;
    }

void DTMElementHandlerManager::GetStorageToUORMatrix (Transform& trsf, DgnModelRefP model, ElementHandleCR element, bool withExaggeration)
    {
    getStorageToUORMatrix (trsf, model, element, withExaggeration);
    }

void DTMElementHandlerManager::GetStorageToUORMatrix (Transform& trsf, ElementHandleCR element)
    {
    getStorageToUORMatrix (trsf, element);
    }

void DTMElementHandlerManager::SetStorageToUORMatrix (const Transform& trsf, EditElementHandleR element)
    {
    setStorageToUORMatrix (trsf, element);
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
void DTMElementHandlerManager::SetName (EditElementHandleR eh, WCharCP name)
    {
    WString oldName;
    if (SUCCESS == GetName (eh, oldName))
        {
        if (oldName == name)
            return;
        }
    EditElementHandle refEl (eh, false);
    FindDTMData (eh, refEl);
    size_t len = name ? wcslen (name) : 0;
    StatusInt result = len ? \
        refEl.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_XGraphicsName, 0), 0, sizeof(wchar_t) * len, name) : \
        refEl.ScheduleDeleteXAttribute (XAttributeHandlerId (XATTRIBUTEID_XGraphicsName, 0), 0);
    BeAssert (SUCCESS == result && "SetName XAttrib error");

    if(eh.GetElementRef () != refEl.GetElementRef ())
        refEl.ReplaceInModel (refEl.GetElementRef ());
    }

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
void DTMElementHandlerManager::SetName (ElementRefP elRef, WCharCP name)
    {
    EditElementHandle element (elRef, nullptr);
    SetName (element, name);
    element.ReplaceInModel (element.GetElementRef ());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
StatusInt DTMElementHandlerManager::GetName (ElementHandleCR element, WStringR name)
    {
   ElementHandle refEl (element);

    FindDTMData (element, refEl);

    ElementHandle::XAttributeIter iterator (refEl, XAttributeHandlerId (XATTRIBUTEID_XGraphicsName, 0), 0);
    
    if (!iterator.IsValid())
        return ERROR;

    UInt32   length = iterator.GetSize() / sizeof(wchar_t);
    WCharCP  data = (WCharCP) iterator.PeekData(); 
    name.assign (data, length);
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/11
//=======================================================================================
double DTMElementHandlerManager::GetDTMLastModified(ElementHandleCR element)
    {
    const DTMElm* elm = (const DTMElm*)element.GetElementCP();
        
    if (elm->ehdr.attrOffset == sizeof (ExtendedElm) / sizeof (short))
        return elm->ehdr.lastModified;
    return elm->dtmLastModified;
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
void DTMElementHandlerManager::SetThematicDisplayStyleIndex (EditElementHandleR eh, int displayStyleIndex)
    {
    if (displayStyleIndex != -1)
        eh.ScheduleWriteXAttribute (XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_DISPLAYSTYLE), 0, sizeof (int), &displayStyleIndex);
    else
        eh.ScheduleDeleteXAttribute (XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_DISPLAYSTYLE), 0);

    if (!eh.GetElementCP()->hdr.ehdr.isGraphics)
        {
        ElementHandle originalEl;
        TMSymbologyOverrideManager::GetReferencedElement (eh, originalEl);

        RedrawElement (originalEl);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
StatusInt DTMElementHandlerManager::GetThematicDisplayStyleIndex (ElementHandleCR element, int& displayStyleIndex)
    {
    ElementHandle::XAttributeIter iterator (element, XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_DISPLAYSTYLE), 0);
    
    if (iterator.IsValid())
        {
        int* data = (int*) iterator.PeekData(); 
        displayStyleIndex = *data;
        if (displayStyleIndex != -1)
            return SUCCESS;
        }
    displayStyleIndex = -1;
    return ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/11
//=======================================================================================
bool DTMElementHandlerManager::GetElementForSymbology (ElementHandleCR element, ElementHandleR symbologyElem, DgnModelRefP destinationModel)
    {
    return TMSymbologyOverrideManager::GetElementForSymbology (element, symbologyElem, destinationModel);
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void DTMElementHandlerManager::Traverse
(
DgnModelRefP        modelRef,
bool                includeRefs,
TraverseCallBack    callback,
void                *userArgs
)
    {
    if (!callback)
        return;
    for (DTMElementIterator iter (modelRef, includeRefs); iter.MoveNext();)
        {
        if (SUCCESS != callback (iter.Current(), userArgs))
            return;
        }
    }

struct DTMIteratorArg
    {
    DgnModelRefP modelRef;
    bvector<ElementHandle>* elements;
    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
int scanCriteria_ElemRefCallback(
     ElementRefP      elemRef,
     void*           argP,
     ScanCriteria*   scP)
    {
    DTMIteratorArg* args = (DTMIteratorArg*)argP;
    bvector<ElementHandle>* elements = args->elements;

    ElementHandle element(elemRef, args->modelRef);
    if (DTMElementHandlerManager::IsDTMElement(element))
        elements->push_back(element);
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMElementIterator::ScanModel (DgnModelRefP modelRef)
    {
    // ToDo, this now needs to look at all elements in this model and all models in the dgn called DGN.
    ScanCriteria *scP = ScanCriteria::Create ();
    scP->SetModelRef (modelRef);
    scP->SetReturnType (MSSCANCRIT_ITERATE_ELMREF, false, false);
    scP->AddSingleElementTypeTest (EXTENDED_ELM);
    DTMIteratorArg args;
    args.elements = &m_elements;
    args.modelRef = modelRef;
    scP->SetElemRefCallback (scanCriteria_ElemRefCallback, (void*)&args);
    scP->Scan (NULL, NULL, NULL);
    delete scP;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
DTMElementIterator::DTMElementIterator(DgnModelRefP modelRef, bool includeRefs)
    {
    DgnModelRefList updateList;

    // Maybe we could cache the update list for model in reference??
    // get the updateList for this reference file
    modelRef->FillUpdateList (updateList, true, includeRefs);

    int modelRefCount = (int)updateList.size ();

    // step through the file and its reference files
    for (int modelRefIndex = 0; modelRefIndex < modelRefCount; ++modelRefIndex)
        {
        DgnModelRefP currentModelRef = updateList [modelRefIndex];
        ScanModel(currentModelRef);
        }

    m_index = -1;
    }

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
//DgnModelRefP GetActivatedModel (ViewContextCP context)
//    {
//    if (context && context->GetViewport())
//        return context->GetViewport()->GetTargetModel();
//
////ToDo    return DTMSessionMonitor::GetInstance().GetActive ();
//    return nullptr;
//    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
