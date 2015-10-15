/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMReferenceXAttributeHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <stdafx.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include <TerrainModel/ElementHandler/DTMReferenceXAttributeHandler.h>

#define CURRENT_REFERENCE_VERSION 1
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
inline int GetType (ElementHandleCR element)
    {
    BeAssert (element.IsValid ());
    return element.GetElementType();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
void DeleteDTMDataElement (ElementHandleR el)
    {
    EditElementHandle eeh (el, false);
    eeh.DeleteFromModel();
    }

ElementRefP DTMReferenceXAttributeHandler::s_ignoreXAttributeHandlerDelete;

void DTMReferenceXAttributeHandler::SetIgnoreXAttributeHandlerDelete (ElementRefP elRef)
    {
    s_ignoreXAttributeHandlerDelete = elRef;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::RemoveDTMDataReference(EditElementHandleR el)
    {
    el.ScheduleDeleteXAttribute(GetXAttributeHandlerId (), GetXAttributeId());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::SetDTMDataReference(EditElementHandleR el, ElementHandleCR dataEl)
    {
    DataExternalizer d;
    short version = CURRENT_REFERENCE_VERSION;
    d.put(version);
    PersistentElementPath pep; //(el.GetModelRef(), dataEl.GetElementRef());

    if (dataEl.IsValid ())
        {
        DgnAttachmentCP referenceToTargetFile = nullptr; //mdlRefFile_getInfo (GetModelRef(dataEl));
        pep.CaptureQualifiedFarReference(dataEl.GetElementRef(), referenceToTargetFile, PersistentElementPath::QFR_FLAG_IncludeNoExtraModels); // Was CaptureQualifiedFarReferenceEX false, false);
        }
    pep.Store(&d);

    el.ScheduleWriteXAttribute(GetXAttributeHandlerId (), GetXAttributeId(), (uint32_t)d.getBytesWritten(), d.getBuf());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
bool DTMReferenceXAttributeHandler::GetDTMDataReference(ElementHandleCR el, ElementHandleR dataEl)
    {
    ElementHandle::XAttributeIter xAttrHandle(el, GetXAttributeHandlerId (), GetXAttributeId());

    void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);

    if (data)
        {
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        pep.Load (source);
        DgnModelRef* model = GetModelRef(el);
        if (!model)
            return false;

        dataEl = pep.EvaluateElement(model);

        return true;
        }

    return false;
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================       
ElementHandle DTMReferenceXAttributeHandler::ExtractReferencedElement (XAttributeHandleCR xAttr, DgnModelRefP model, bool checkForDeleted)
    {
    ElementHandle dataEl;
    void* data(xAttr.IsValid() ? (void*)xAttr.PeekData() : nullptr);
    if (data)
        {
        DataInternalizer source ((byte*)data, xAttr.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        pep.Load (source);
        ElementHandle el(xAttr.GetElementRef(), model);
        model = GetModelRef(el);
        if (!model)
            return dataEl;
        dataEl = pep.EvaluateElement (model);

        if (checkForDeleted && !dataEl.GetElementRef()) // The element may have been deleted.
            {
            T_StdElementRefSet ref;
            pep.DisclosePointers (&ref, model);
            if (ref.size())
                dataEl = ElementHandle (*ref.begin (), model);
            }
        BeAssert (dataEl.GetElementRef()); // This shouldn't happen
        }
    return dataEl;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
ElementHandle DTMReferenceXAttributeHandler::ExtractReferencedElement (const void* data, uint32_t size, ElementRefP ref, DgnModelRefP model)
    {
    ElementHandle dataEl;
    DataInternalizer source ((byte*)data, size);
    short version;

    source.get(&version);
    PersistentElementPath pep;
    pep.Load (source);
    ElementHandle el(ref, model);
    dataEl = pep.EvaluateElement(GetModelRef(el));

    BeAssert (dataEl.GetElementRef());
    return dataEl;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
int DTMReferenceXAttributeHandler::RemoveReferenceCount (ElementHandleR el)
    {
    BeAssert (EXTENDED_NONGRAPHIC_ELM == GetType(el));
    XAttributeHandle xAttrHandle(el.GetElementRef(), GetRefCountXAttributeHandlerId (), GetRefCountXAttributeId());
    int* data(xAttrHandle.IsValid() ? (int*)xAttrHandle.PeekData() : nullptr);

    if (data)
        {
        int count = *((int*)data);
        count--;
        ITxnManager::GetCurrentTxn().ReplaceXAttributeData(xAttrHandle, &count, sizeof(count));
        return count;
        }
    return 0;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::AddReferenceCount (ElementHandleR el)
    {
    BeAssert (EXTENDED_NONGRAPHIC_ELM == GetType(el));
    XAttributeHandle xAttrHandle(el.GetElementRef(), GetRefCountXAttributeHandlerId (), GetRefCountXAttributeId());
    int* data(xAttrHandle.IsValid() ? (int*)xAttrHandle.PeekData() : nullptr);

    if (data)
        {
        int count = *((int*)data);
        count++;
        ITxnManager::GetCurrentTxn().ReplaceXAttributeData(xAttrHandle, &count, sizeof(count));
        }
    else
        {
        int count = 1;
        ITxnManager::GetCurrentTxn().AddXAttribute(el.GetElementRef(), GetRefCountXAttributeHandlerId (), GetRefCountXAttributeId (), &count, sizeof(count));
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void    DTMReferenceXAttributeHandler::_OnUndoRedoRootsChanged
(
ElementHandleR                    dependent,
bvector<RootChange> const&        _RootsChanged,
bvector<XAttributeHandle> const&  xAttrsAffected
) const
    {
    DTMDisplayCacheManager::DeleteCacheElem (dependent);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnPreHostChange (XAttributeHandleCR xAttr, TransactionType type)
    {
    ElementHandle dataEl = ExtractReferencedElement (xAttr);
    DTMDisplayCacheManager::DeleteCacheElem (dataEl);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnPreHostDelete (XAttributeHandleCR xAttr, TransactionType type)
    {
    ElementHandle dataEl = ExtractReferencedElement (xAttr);

    if (dataEl.IsValid())
        {
        DTMDisplayCacheManager::DeleteCacheElem (dataEl);

        if (type == TRANSACTIONTYPE_Action && EXTENDED_NONGRAPHIC_ELM == GetType (dataEl))
            {
            if (!RemoveReferenceCount (dataEl))
                {
                DeleteDTMDataElement (dataEl);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnPostAdd (XAttributeHandleCR xAttr, TransactionType type)
    {
    if (type != TRANSACTIONTYPE_Action)
        return;

    ElementHandle dataEl = ExtractReferencedElement (xAttr);

    if (dataEl.IsValid () && EXTENDED_NONGRAPHIC_ELM == GetType (dataEl))
        {
        AddReferenceCount (dataEl);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/11
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnPreReplaceData (XAttributeHandleCR xAttr, void const* newData, uint32_t newSize, TransactionType type)
    {
    // Need to Add Reference to the new one.
    ElementHandle oldDataEl = ExtractReferencedElement (xAttr);

    if (oldDataEl.IsValid())
        {
        DTMDisplayCacheManager::DeleteCacheElem (oldDataEl);
        }

    if (type == TRANSACTIONTYPE_Action)
        {
        ElementHandle newDataEl = ExtractReferencedElement (newData, newSize, xAttr.GetElementRef(), nullptr);

        if (oldDataEl.GetElementRef() != newDataEl.GetElementRef())
            {
            if (oldDataEl.IsValid())
                {
                if (!RemoveReferenceCount (oldDataEl))
                    {
                    DeleteDTMDataElement (oldDataEl);
                    }
                }
            if (newDataEl.IsValid () && EXTENDED_NONGRAPHIC_ELM == GetType (newDataEl))
                {
                AddReferenceCount (newDataEl);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMReferenceXAttributeHandler::_OnPreprocessCopy (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element, ElementCopyContextP cc)
    {
    DgnModelRefP    sourceModelRef;
    DgnModelRefP    destinationModelRef;
    bool         sameCache;
    bool         sameFile;
    bool            doDeepCopy = true;

    // Change to copy the XAttributes from the DTM Data to the local element.

    cc->GetModels (&sourceModelRef, &destinationModelRef, &sameCache, &sameFile, &element);

    if (DependencyManager::IsReferenceBeingMerged (sourceModelRef))
        {
        doDeepCopy = !destinationModelRef->Is3d();
        }
    else
        {
        ElementHandle origElem (element.GetElementDescrCP()->h.elementRef, sourceModelRef);
        ElementHandle refElem;
        DTMSymbologyOverrideManager::GetReferencedElement (origElem, refElem);
        if (DependencyManager::IsReferenceBeingMerged (origElem.GetModelRef()))
            doDeepCopy = false;
        }
    
    if ((cc->GetCloneOptions () & ElementCopyContext::CLONE_OPTIONS_CopyingReferences) == ElementCopyContext::CLONE_OPTIONS_CopyingReferences)
        doDeepCopy = false;

    if (!doDeepCopy)
        {
        void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
        if (data)
            {
            ExtractReferencedElement (xAttrHandle, sourceModelRef, true);
            DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
            short version;

            source.get(&version);
            PersistentElementPath pep;
            if (pep.Load (source) != SUCCESS)
                return ERROR;

            if (pep.OnPreprocessCopy (element, cc, pep.COPYOPTION_DeepCopyAcrossFiles) == SUCCESS)
                {
                //  Update XAttribute with final copied state of my PEP and my other stored data
                DataExternalizer sink;
                sink.put(version);
                pep.Store (&sink);

                toBeReplaced->Add (xAttrHandle, sink.getBytesWritten(), sink.getBuf());
                }
            }

        return SUCCESS;
        }

    EditElementHandle oldDtmEl (ExtractReferencedElement (xAttrHandle, sourceModelRef, true), true);
    ElementHandle::XAttributeIter xAttrDataHandle (oldDtmEl);

    while (xAttrDataHandle.IsValid())
        {
        XAttributeHandlerId handId = xAttrDataHandle.GetHandlerId();
        int id = xAttrDataHandle.GetId();
        if (handId.GetMajorId () == TMElementMajorId)
            {
            if ((handId.GetMinorId () >= XATTRIBUTES_SUBID_DTM_HEADER && handId.GetMinorId () <= XATTRIBUTES_SUBID_DTM_FLISTARRAY) || handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP)
                {
                const void* data = xAttrDataHandle.PeekData ();
                uint32_t size = xAttrDataHandle.GetSize ();

                xAttrDataHandle.ToNext();
                toBeReplaced->Add (handId, id, size, data);
                }
            else
                xAttrDataHandle.ToNext();
            }
        else
            xAttrDataHandle.ToNext();
        }

    // Don't add the Reference XAttribute.
    return ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnPreDelete (XAttributeHandleCR xAttr, TransactionType type)
    {
    if (s_ignoreXAttributeHandlerDelete == xAttr.GetElementRef())
        return;

    if (type == TRANSACTIONTYPE_Action)
        {
        ElementHandle dataEl = ExtractReferencedElement (xAttr);

        if (dataEl.IsValid () && EXTENDED_NONGRAPHIC_ELM == GetType (dataEl))
            {
            if (!RemoveReferenceCount (dataEl))
                {
                DeleteDTMDataElement (dataEl);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMReferenceXAttributeHandler::_OnPreprocessCopyRemapIds (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element)
    {
    void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
    if (data)
        {
        ExtractReferencedElement (xAttrHandle, element.GetModelRef());
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        if (pep.Load (source) != SUCCESS)
            return ERROR;

        //  Return early if this XAttribute was not actually copied.
        if (!pep.ContainsRemapKeys())
            return SUCCESS;

        //  Allow my PEP to remap to copied targets or to find its old targets again.
        if (pep.OnPreprocessCopyRemapIds (element) != SUCCESS)
            return ERROR;

        //  Update XAttribute with final copied state of my PEP and my other stored data
        DataExternalizer sink;
        sink.put(version);
        pep.Store (&sink);

        toBeReplaced->Add (xAttrHandle, sink.getBytesWritten(), sink.getBuf());
        }
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnElementIDsChanged (XAttributeHandleR xAttrHandle, ElementAndModelIdRemappingCR remapTable, ElementHandleCR element)
    {
    void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
    if (data)
        {
        ExtractReferencedElement (xAttrHandle, element.GetModelRef());
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        if (pep.Load (source) != SUCCESS)
            return;

        if (!pep.OnElementIDsChanged (remapTable, element))
            return;

        //  Update XAttribute with final copied state of my PEP and my other stored data
        DataExternalizer sink;
        sink.put(version);
        pep.Store (&sink);

        ITxnManager::GetCurrentTxn().ReplaceXAttributeData (xAttrHandle, sink.getBuf(), (uint32_t)sink.getBytesWritten());
        }
    return;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xAttrHandle, DgnModelRefP homeModel)
    {
    if (homeModel == nullptr)
        return;

    void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);

    if (data)
        {
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        pep.Load (source);

        pep.DisclosePointers (refs, homeModel);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMReferenceXAttributeHandler::_OnRootsChanged(ElementHandleR dependent, bvector<IDependencyHandler::RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected)
    {
    DTMDisplayCacheManager::DeleteCacheElem (dependent);
    if (ITxnManager::GetManager().IsUndoInProgress())
        return;

    bool replaceElement = false;

    EditElementHandle eeh (dependent, true);
    eeh.GetElementDescrP();

    for(bvector<IDependencyHandler::RootChange>::const_iterator iter = _RootsChanged.begin(); iter != _RootsChanged.end(); iter++)
        {
        if (iter->changeStatus == CHANGESTATUS_Changed)
            replaceElement = true;
        }

    DisplayHandlerP handler = eeh.GetDisplayHandler();
    if (handler)
        {
        double previousLastModifiedTime;

        if ((sizeof (ExtendedElm) / 2) != eeh.GetElementDescrCP()->el.ehdr.attrOffset)
            {
            const DTMElm* dtmElement = (const DTMElm*)&eeh.GetElementDescrCP()->el;
            previousLastModifiedTime = dtmElement->dtmLastModified;
            }
        else
            replaceElement = true;

        ScanRange rng = eeh.GetElementDescrCP()->el.hdr.dhdr.range;
        if (handler->ValidateElementRange (eeh, false) == SUCCESS)
            {
            ScanRange rng2 = eeh.GetElementDescrCP()->el.hdr.dhdr.range;
            if (memcmp(&rng, &rng2, sizeof(ScanRange)) != 0)
                replaceElement = true;
            }
        if (!replaceElement)
            {
            const DTMElm* dtmElement = (const DTMElm*)&eeh.GetElementDescrCP()->el;
            replaceElement = (previousLastModifiedTime != dtmElement->dtmLastModified);
            }
        }

    if (replaceElement)
        eeh.ReplaceInModel (dependent.GetElementRef());
    }


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE