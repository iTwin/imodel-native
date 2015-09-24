/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMSymbologyOverrideManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include <DgnPlatform\ElementUtil.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include "DTMDisplayHandlers.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
    void ScanLevelsAndAddOrRemove (ElementHandleCR element, bool add, bool isLoading = false);
END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void CopySymbology (EditElementHandleR element, ElementHandleCR source, ElementCopyContextP copyContext, bool allowRemap)
    {
    DgnModelRefP    sourceModelRef = source.GetModelRef();
    DgnModelRefP    destinationModelRef = element.GetModelRef();

    if (copyContext)
        copyContext->GetModels (&sourceModelRef, &destinationModelRef, NULL, NULL, &element);

    ElementHandle symbologyEl;
    ElementHandle origEl (source.GetElementRef() != nullptr ? source.GetElementRef() : source.GetElementDescrCP()->h.elementRef, sourceModelRef);

    if (origEl.GetElementRef() == 0)
        origEl = source;

    DTMSymbologyOverrideManager::GetElementForSymbology (origEl, symbologyEl, /* ToDo */ destinationModelRef, allowRemap);   // This is the referenced model. This needs some thinking.

    if (element.PeekElementDescrCP() != symbologyEl.PeekElementDescrCP())
        {
        ElementHandle::XAttributeIter xAttrHandle (element);

        while (xAttrHandle.IsValid())
            {
            XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
            int id = xAttrHandle.GetId();

            if (handId.GetMajorId () == LINKAGEID_TEXTSTYLE || (handId.GetMajorId () == TMElementMajorId && 
                (handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_DISPLAYPARAMS  || handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_DISPLAYSTYLE)))
                {
                xAttrHandle.ToNext();
                element.ScheduleDeleteXAttribute (handId, id);
                }
            else
                xAttrHandle.ToNext();
            }

        ElementHandle::XAttributeIter xAttrHandle2 (symbologyEl);

        while (xAttrHandle2.IsValid())
            {
            XAttributeHandlerId handId = xAttrHandle2.GetHandlerId();
            int id = xAttrHandle2.GetId();

            if (handId.GetMajorId () == TMElementMajorId)
                {
                if (handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_DISPLAYPARAMS)
                    {                
                    DTMElementSubHandler* subHandler = DTMElementSubHandler::FindHandler(xAttrHandle2);

                    BeAssert(subHandler != 0);

                    if (subHandler->_IsSupportedFor(ElementHandlerManager::GetHandlerId(element)) == true)
                        {
                        element.ScheduleWriteXAttribute (handId, id, xAttrHandle2.GetSize(), xAttrHandle2.PeekData());
                        }
                    }
                else if (handId.GetMinorId () == XATTRIBUTES_SUBID_DTM_DISPLAYSTYLE)
                    {
                    element.ScheduleWriteXAttribute (handId, id, xAttrHandle2.GetSize(), xAttrHandle2.PeekData());
                    }
                }
            else if (handId.GetMajorId () == LINKAGEID_TEXTSTYLE)
                {
                element.ScheduleWriteXAttribute (handId, id, xAttrHandle2.GetSize(), xAttrHandle2.PeekData());
                }
            xAttrHandle2.ToNext();
            }
        }
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      1/2008
//=======================================================================================
struct UnresolvedReasonFinder : PersistentElementPath::PathProcessor
    {
    private:

        bool m_deletedID;

    public:

        UnresolvedReasonFinder ()
            {
            m_deletedID = false;
            }

        virtual void OnElementId (ElementId e, ElementRefP ref, DgnModelP) override 
            {
            if (ref == NULL)
                m_deletedID = true;
            }

        virtual void OnReferenceAttachmentId (ElementId e, ElementRefP ref, DgnModelRefP refmod, DgnModelP) override 
            {
            }

        virtual void OnModelId (ModelId m, DgnModelP c, DgnFileP f) override 
            {
            if (!c && f)
                m_deletedID = true;
            }

        bool IsElementDeleted ()
            {
            return m_deletedID; 
            }   

    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
StatusInt DTMSymbologyOverrideManager::_OnPreprocessCopy (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element, ElementCopyContextP cc)
    {
    if (xAttrHandle.GetHandlerId() == GetDisplayRefXAttributeHandlerId())
        {
        void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
        if (data)
            {
            DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
            short version;

            source.get(&version);
            PersistentElementPath pep;
            if (pep.Load (source) != SUCCESS)
                return ERROR;

            //  Return early if this XAttribute was not actually copied.
            if (pep.OnPreprocessCopy (element, cc, pep.COPYOPTION_DeepCopyAcrossFiles) != SUCCESS)
                return SUCCESS;

            //  Update XAttribute with final copied state of my PEP and my other stored data
            DataExternalizer sink;
            sink.put(version);
            pep.Store (&sink);

            toBeReplaced->Add (xAttrHandle, sink.getBytesWritten(), sink.getBuf());
            }
        }
    else
        {
        DgnModelRefP    sourceModelRef;
        DgnModelRefP    destinationModelRef;
        bool         sameCache;
        bool         sameFile;

        // Change to copy the XAttributes from the DTM Data to the local element.

        cc->GetModels (&sourceModelRef, &destinationModelRef, &sameCache, &sameFile, &element);
        // This is being merged leave this pointing to the old reference.
        if (DependencyManager::IsReferenceBeingMerged (sourceModelRef))
            return SUCCESS;

        DataExternalizer d;
        DataInternalizer source ((const byte*)xAttrHandle.PeekData(), xAttrHandle.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;
        bool changed = false;

        source.get (&version);
        d.put(version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            // I don't think this is ever needed.
            //            if (pepReferenced.OnPreprocessCopy (element, cc, pepReferenced.COPYOPTION_DeepCopyAcrossFiles) == SUCCESS)
            //                changed = true;

            if (pepMapped.OnPreprocessCopy (element, cc, pepMapped.COPYOPTION_DeepCopyAcrossFiles) == SUCCESS)
                changed = true;

            pepReferenced.Store (&d);
            pepMapped.Store (&d);
            }
        if (changed)
            toBeReplaced->Add (xAttrHandle, d.getBytesWritten(), d.getBuf());
        }

    return SUCCESS;
    }

StatusInt DTMSymbologyOverrideManager::_OnPreprocessCopyRemapIds (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element)
    {
    if (xAttrHandle.GetHandlerId() == GetDisplayRefXAttributeHandlerId())
        {
        void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
        if (data)
            {
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
        }
    else
        {
        DataExternalizer d;
        DataInternalizer source ((const byte*)xAttrHandle.PeekData(), xAttrHandle.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;
        bool changed = false;

        source.get (&version);
        d.put(version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            if (pepReferenced.ContainsRemapKeys() && pepReferenced.OnPreprocessCopyRemapIds (element) == SUCCESS)
                changed = true;

            if (pepMapped.ContainsRemapKeys() && pepMapped.OnPreprocessCopyRemapIds (element) == SUCCESS)
                changed = true;
            pepReferenced.Store (&d);
            pepMapped.Store (&d);
            }
        if (changed)
            toBeReplaced->Add (xAttrHandle, d.getBytesWritten(), d.getBuf());
        }
    return SUCCESS;
    }

void DTMSymbologyOverrideManager::_OnElementIDsChanged (XAttributeHandleR xa, ElementAndModelIdRemappingCR remapTable, ElementHandleCR element)
    {
    return;
    }

void DTMSymbologyOverrideManager::_DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xAttrHandle, DgnModelRefP homeModel)
    {
    if (homeModel == nullptr)
        return;

    if (xAttrHandle.GetHandlerId() == GetDisplayRefXAttributeHandlerId())
        {
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
    else
        {
        DataInternalizer source ((const byte*)xAttrHandle.PeekData(), xAttrHandle.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;

        source.get (&version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            pepReferenced.DisclosePointers (refs, homeModel);
            pepMapped.DisclosePointers (refs, homeModel);
            }
        }
    }

void DTMSymbologyOverrideManager::_OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected)
    {
    bool hasDeleted = false;

    for(bvector<RootChange>::const_iterator changes = _RootsChanged.begin(); changes != _RootsChanged.end(); changes++)
        {
        if (changes->changeStatus == CHANGESTATUS_Deleted)
            {
            hasDeleted = true;
            break;
            }
        }

    // Need to check that they all still exists, and delete 107 element.
    ElementHandle::XAttributeIter xAttrIter (dependent);
    xAttrIter.Search (GetDisplayInfoXAttributeHandlerId(), 1);

    if (xAttrIter.IsValid())
        {
        if (!hasDeleted)
            return;

        bool hasChanged = false;
        int count = 0;
        DataExternalizer d;
        short version = CURRENT_REFERENCE_VERSION;
        DataInternalizer source ((const byte*)xAttrIter.PeekData(), xAttrIter.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;

        d.put(version);
        source.get (&version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            UnresolvedReasonFinder unresolvedReasonFinderReferenced;
            pepReferenced.ProcessPath (unresolvedReasonFinderReferenced, dependent.GetModelRef());
            if(!unresolvedReasonFinderReferenced.IsElementDeleted())
                {
                UnresolvedReasonFinder unresolvedReasonFinderMapped;
                pepMapped.ProcessPath (unresolvedReasonFinderMapped, dependent.GetModelRef());
                if (!unresolvedReasonFinderMapped.IsElementDeleted())
                    {
                    count++;
                    pepReferenced.Store (&d);
                    pepMapped.Store (&d);
                    }
                else
                    hasChanged = true;
                }
            else
                {
                ElementHandle relatedElem = pepMapped.EvaluateElement (dependent.GetModelRef());

                if (NULL != relatedElem.GetElementRef())
                    {
                    hasChanged = true;

                    EditElementHandle relatedEditElem (relatedElem, false);

                    relatedEditElem.DeleteFromModel ();
                    }
                }
            }

        if (hasChanged)
            {
            EditElementHandle eeh (dependent, true);

            if (count == 0)
                eeh.ScheduleDeleteXAttribute (GetDisplayInfoXAttributeHandlerId(), 1);
            else
                eeh.ScheduleWriteXAttribute (GetDisplayInfoXAttributeHandlerId(), 1, (uint32_t)d.getBytesWritten(), d.getBuf());
            eeh.ReplaceInModel (eeh.GetElementRef());
            }
        }
    else
        {
        ElementHandle::XAttributeIter xAttrHandle (dependent);
        xAttrHandle.Search (GetDisplayRefXAttributeHandlerId(), 1);

        void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
        if (data)
            {
            DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
            short version;

            source.get(&version);
            PersistentElementPath pep;

            if (pep.Load (source) != SUCCESS)
                return;

            //  Return early if this XAttribute was not actually copied.
            if (pep.ContainsRemapKeys())
                //  Allow my PEP to remap to copied targets or to find its old targets again.
                    if (pep.OnPreprocessCopyRemapIds (dependent) != SUCCESS)
                        data = data;

            ElementHandle relatedElem = pep.EvaluateElement (dependent.GetModelRef());

            if (NULL == relatedElem.GetElementRef() || dependent.GetModelRef () == relatedElem.GetModelRef())
                {
                EditElementHandle thisEl (dependent.GetElementRef(), dependent.GetModelRef());
                thisEl.DeleteFromModel ();
                }
            }
        }
    }

void DTMSymbologyOverrideManager::_OnUndoRedoRootsChanged(ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected) const
    {
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
void DTMSymbologyOverrideManager::SetReferencedElement (EditElementHandleR newElem, ElementHandleCR elem)
    {
    DataExternalizer d;
    short version = CURRENT_REFERENCE_VERSION;
    d.put(version);
    PersistentElementPath pep;

    DgnAttachmentCP referenceToTargetFile = elem.GetModelRef()->AsDgnAttachmentCP ();
    pep.CaptureQualifiedFarReference (elem.GetElementRef(), referenceToTargetFile, PersistentElementPath::QFR_FLAG_IncludeNoExtraModels);

    pep.Store (&d);

    newElem.ScheduleWriteXAttribute(GetDisplayRefXAttributeHandlerId(), 1, (uint32_t)d.getBytesWritten(), d.getBuf());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
bool DTMSymbologyOverrideManager::GetReferencedElement(ElementHandleCR elem, ElementHandleR referencedElem)
    {
    ElementHandle::XAttributeIter xAttrHandle (elem, GetDisplayRefXAttributeHandlerId(), 1);

    void* data (xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);
    if (data)
        {
        DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
        short version;

        source.get(&version);
        PersistentElementPath pep;
        pep.Load (source);
        ElementHandle referencedElem2 = pep.EvaluateElement (GetModelRef(elem));
        if (referencedElem2.IsValid())
            {
            if(!GetReferencedElement (referencedElem2, referencedElem))
                referencedElem = referencedElem2;
            return true;
            }
        else
            referencedElem = elem;
        }
    else
        referencedElem = elem;
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
bool DTMSymbologyOverrideManager::GetOverridingElement (ElementHandleCR storageElem, ElementHandleCR elem, ElementHandleR relatedElem, bool allowRemap)
    {
    ElementHandle::XAttributeIter xAttrIter (storageElem);
    xAttrIter.Search (GetDisplayInfoXAttributeHandlerId(), 1);

    if (xAttrIter.IsValid())
        {
        ElementRefP elemRef = elem.GetElementRef();
        DataInternalizer source ((const byte*)xAttrIter.PeekData(), xAttrIter.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;

        if (elemRef == nullptr)
            elemRef = elem.GetElementDescrCP()->h.elementRef;

        source.get (&version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            if (allowRemap)
                {
                if (pepReferenced.ContainsRemapKeys())
                    pepReferenced.OnPreprocessCopyRemapIds (storageElem);
                if (pepMapped.ContainsRemapKeys())
                    pepMapped.OnPreprocessCopyRemapIds (storageElem);
                }

            ElementHandle theElem = pepReferenced.EvaluateElement (storageElem.GetModelRef ());
            if (elemRef == theElem.GetElementRef())
                // This causes problems if the element is in a reference file.
                    // if(pepReferenced.EqualElementRef (elem.GetElementRef(), storageElem.GetDgnModel()))
                {
                //ElementHandle testElem = pepReferenced.EvaluateElement (storageElem.GetModelRef());
                //if (testElem.GetModelRef() == elem.GetModelRef())
                //    {
                relatedElem = pepMapped.EvaluateElement (storageElem.GetModelRef());
                if (relatedElem.IsValid())
                    return true;
                //                    }
                }

            }
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
void DTMSymbologyOverrideManager::StoreOverridingElement (EditElementHandleR storageElem, ElementHandleCR elem, ElementHandleR relatedElem)
    {
    DataExternalizer d;
    short version = CURRENT_REFERENCE_VERSION;
    d.put(version);

    ElementHandle::XAttributeIter xAttrIter (storageElem);
    xAttrIter.Search (GetDisplayInfoXAttributeHandlerId(), 1);

    bool found = false;
    if (xAttrIter.IsValid())
        {
        DataInternalizer source ((const byte*)xAttrIter.PeekData(), xAttrIter.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;

        source.get (&version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            if (pepReferenced.EqualElementRef (elem.GetElementRef(), storageElem.GetDgnModelP()))
                {
                ElementHandle testElem = pepReferenced.EvaluateElement (storageElem.GetModelRef());
                if(testElem.GetModelRef() == elem.GetModelRef())
                    {
                    DgnAttachmentCP referenceToTargetFile = relatedElem.GetModelRef()->AsDgnAttachmentCP();
                    pepMapped.CaptureQualifiedFarReference (relatedElem.GetElementRef(), referenceToTargetFile, PersistentElementPath::QFR_FLAG_IncludeNoExtraModels);

                    found = true;
                    }
                pepReferenced.Store (&d);
                pepMapped.Store (&d);
                }
            else
                {
                UnresolvedReasonFinder unresolvedReasonFinder;
                pepReferenced.ProcessPath (unresolvedReasonFinder, storageElem.GetModelRef());
                if(!unresolvedReasonFinder.IsElementDeleted())
                    {
                    pepReferenced.Store (&d);
                    pepMapped.Store (&d);
                    }
                else
                    {
                    EditElementHandle relatedElem (pepMapped.EvaluateElement (storageElem.GetModelRef()), false);
                    if (NULL != relatedElem.GetElementRef())
                        {
                        relatedElem.DeleteFromModel ();
                        }
                    }
                }
            }
        }

    if (!found)
        {
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        DgnAttachmentCP referenceToTargetFile = elem.GetModelRef()->AsDgnAttachmentCP();

        pepReferenced.CaptureQualifiedFarReference (elem.GetElementRef(), referenceToTargetFile, PersistentElementPath::QFR_FLAG_IncludeNoExtraModels);

        pepReferenced.Store (&d);

        referenceToTargetFile = relatedElem.GetModelRef()->AsDgnAttachmentCP ();
        pepMapped.CaptureQualifiedFarReference (relatedElem.GetElementRef(), referenceToTargetFile, PersistentElementPath::QFR_FLAG_IncludeNoExtraModels);

        pepMapped.Store (&d);
        }

    storageElem.ScheduleWriteXAttribute (GetDisplayInfoXAttributeHandlerId(), 1, (uint32_t)d.getBytesWritten(), d.getBuf());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
void DTMSymbologyOverrideManager::DeleteOverridingElement (EditElementHandleR storageElem, ElementHandleCR elem)
    {
    DataExternalizer d;
    short version = CURRENT_REFERENCE_VERSION;
    d.put(version);

    ElementHandle::XAttributeIter xAttrIter (storageElem);
    xAttrIter.Search (GetDisplayInfoXAttributeHandlerId(), 1);

    bool found = false;
    if (xAttrIter.IsValid())
        {
        DataInternalizer source ((const byte*)xAttrIter.PeekData(), xAttrIter.GetSize());
        PersistentElementPath pepReferenced;
        PersistentElementPath pepMapped;
        short version;

        source.get (&version);

        while (!source.AtOrBeyondEOS())
            {
            pepReferenced.Load (source);
            pepMapped.Load (source);

            if(pepReferenced.EqualElementRef (elem.GetElementRef(), storageElem.GetDgnModelP()))
                {
                ElementHandle testElem = pepReferenced.EvaluateElement (storageElem.GetModelRef());
                if(testElem.GetModelRef() == elem.GetModelRef())
                    {
                    EditElementHandle relatedElem (pepMapped.EvaluateElement (storageElem.GetModelRef()), false);
                    relatedElem.DeleteFromModel();
                    found = true;
                    }
                else
                    {
                    UnresolvedReasonFinder unresolvedReasonFinder;
                    pepReferenced.ProcessPath (unresolvedReasonFinder, storageElem.GetModelRef());
                    if(!unresolvedReasonFinder.IsElementDeleted())
                        {
                        pepReferenced.Store (&d);
                        pepMapped.Store (&d);
                        }
                    }
                }
            else
                {
                UnresolvedReasonFinder unresolvedReasonFinder;
                pepReferenced.ProcessPath (unresolvedReasonFinder, storageElem.GetModelRef());
                if(!unresolvedReasonFinder.IsElementDeleted())
                    {
                    pepReferenced.Store (&d);
                    pepMapped.Store (&d);
                    }
                }
            }
        }

    if (found)
        {
        storageElem.ScheduleWriteXAttribute (GetDisplayInfoXAttributeHandlerId(), 1, (uint32_t)d.getBytesWritten(), d.getBuf());
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
bool DTMSymbologyOverrideManager::GetElementForSymbology (ElementHandleCR elem, ElementHandleR symbologyElem, DgnModelRefP destinationModel, bool allowRemap)
    {
    if (!GetModelRef (elem)->IsDgnAttachment())
        {
        symbologyElem = elem;
        return false;
        }
    DgnModelRefP modelRef = GetModelRef(elem);
    std::stack<DgnAttachmentCP> refs;
    std::stack<DgnModelRefP> modelRefs;
    ElementRefP  elemRef;
    do
        {
        DgnAttachmentCP rfP = modelRef->AsDgnAttachmentCP();
        refs.push (rfP);
        modelRef = modelRef->GetParentModelRefP();
        modelRefs.push (modelRef);
        if (destinationModel == modelRef)
            break;
        } while (modelRef->IsDgnAttachment());

        if (destinationModel != modelRef)
            {
            symbologyElem = elem;
            return false;
            }

        while (refs.size())
            {
            DgnAttachmentCP rfP = refs.top();
            modelRef = modelRefs.top();
            modelRefs.pop();
            refs.pop();

            elemRef = modelRef->GetDgnModelP()->FindByElementId (rfP->GetElementId/*GetRefElementId*/());

            ElementHandle nel (elemRef, modelRef);

            // No Element then use the next one.
            if (GetOverridingElement (nel, elem, symbologyElem, allowRemap))
                return true;
            }
        symbologyElem = elem;
        return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
bool DTMSymbologyOverrideManager::CanHaveSymbologyOverride (ElementHandleCR elem)
    {
    return elem.GetModelRef ()->IsDgnAttachment(); //ToDo && GetActivatedModel (nullptr) != elem.GetModelRef ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
StatusInt DTMSymbologyOverrideManager::CreateSymbologyOverride (ElementHandleCR elem, DgnModelRefP destinationModelRef)
    {
    if (!elem.GetModelRef()->IsDgnAttachment())
        return ERROR;

    ElementHandle originalElem;
    GetReferencedElement (elem, originalElem);

    DgnModelRefP modelRef = originalElem.GetModelRef();
    DgnAttachmentCP rfP;
    ElementRefP  elemRef;
    do
        {
        rfP = modelRef->AsDgnAttachmentCP();
        modelRef = modelRef->GetParentModelRefP();
        if (destinationModelRef == modelRef)
            break;
        } while (modelRef->IsDgnAttachment());

        if (destinationModelRef != modelRef)
            {
            // This means that this the destinationModel ref and the elem aren't related.
            return ERROR;
            }

        elemRef = modelRef->GetDgnModelP()->FindByElementId (rfP->GetElementId/*GetRefElementId*/());

        EditElementHandle nel (elemRef, modelRef);

        DTMSubElementIter iter (nel, true);

        ElementHandle symbologyElem;
        if (GetOverridingElement (nel, originalElem, symbologyElem))
            return SUCCESS;

        EditElementHandle newElem;

        newElem.SetModelRef (modelRef);
#ifdef DoWeNeedThis
        Transform trfs;
        getStorageToUORMatrix (trfs, elem.GetModelRef (), elem, false);
        // This creates a 106 element mostly because we want it to be able to hold symbology. we change it to be a 107 non graphical.
        DTMElementHandlerManager::CheckAndCreateElementDescr (newElem, IElementHandlerManager::GetHandlerId(elem), trfs);

        newElem.GetElementDescrP()->el.hdr.ehdr.type = 107;
        newElem.GetElementDescrP()->el.hdr.ehdr.isGraphics = false;
        //DTMElementHandlerManager::CreateDefaultDisplayElements (newElem);
        // Need something to reference the DTMElement from the overrided 107 element.
#endif
        ExtendedElementHandler::InitializeElement (newElem, nullptr, nullptr, true, false);
        newElem.GetElementDescrP()->el.ehdr.type = EXTENDED_NONGRAPHIC_ELM;
        newElem.GetElementDescrP()->el.ehdr.isGraphics = false;
        CopySymbology (newElem, elem, nullptr, false);

        ElementHandlerXAttribute handlerTag (ElementHandlerId (TMElementMajorId, ELEMENTHANDLER_DTMOVERRIDESYMBOLOGY), MISSING_HANDLER_PERMISSION_All_);
        ElementHandlerManager::AddHandlerToElement (newElem, handlerTag);
        //ElementHandle dataEl;
        //DTMElementHandlerManager::FindDTMData (elem, dataEl);
        //DTMReferenceXAttributeHandler::SetDTMDataReference (newElem, dataEl);
        //newElem.GetElementP()->hdr.dhdr.props.b.invisible = 1;

        SetReferencedElement (newElem, elem);
#ifdef DoWeNeedThis
        WString name;
        if (DTMElementHandlerManager::GetName (elem, name))
            DTMElementHandlerManager::SetName (newElem, &name);
#endif

        newElem.AddToModel ();
        //        newElem.GetElementDescrP();
        //        MSElementDescrP el = newElem.ExtractElementDescr();
        //        el->el.hdr.ehdr.isGraphics = true;
        //        newElem.ReplaceElementDescr (el);
        //        newElem.ReplaceInModel();

        StoreOverridingElement (nel, originalElem, newElem);
        nel.ReplaceInModel (nel.GetElementRef());
        return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/10
//=======================================================================================
StatusInt DTMSymbologyOverrideManager::DeleteSymbologyOverride (ElementHandleCR elem, DgnModelRefP destinationModelRef)
    {
    if (!elem.GetModelRef()->IsDgnAttachment())
        return ERROR;

    DgnModelRefP modelRef = elem.GetModelRef();
    DgnAttachmentCP rfP;
    ElementRefP  elemRef;
    do
        {
        rfP = modelRef->AsDgnAttachmentCP();
        modelRef = modelRef->GetParentModelRefP();
        if (destinationModelRef == modelRef)
            break;
        } while (modelRef->IsDgnAttachment());

    if (destinationModelRef != modelRef)
        {
        // This means that this the destinationModel ref and the elem aren't related.
        return ERROR;
        }

    elemRef = modelRef->GetDgnModelP()->FindByElementId (rfP->GetElementId/*GetRefElementId*/());

    EditElementHandle nel (elemRef, modelRef);

    DeleteOverridingElement (nel, elem);
    nel.ReplaceInModel (nel.GetElementRef());
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
void DTMOverrideSymbologyManager::_EditProperties (EditElementHandleR element, PropertyContextR context)
    {
    DTMElementDisplayHandler::GetInstance()._EditProperties (element, context);

    T_Super::_EditProperties(element, context);

    if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
        {
        ElementId  templateId = TemplateRefAttributes::GetReferencedTemplateIDFromHandle (element);
        EachElementTemplateArg                       etArg (templateId, PROPSCALLBACK_FLAGS_IsBaseID, context);

        if (context.DoElementTemplateCallback (&templateId, etArg))
            ElementTemplateUtils::SetReferencedTemplateID (element, templateId, etArg.GetApplyDefaultSymbology()); 
        }
    }

//=======================================================================================
// @bsimethod                                                   Piotr.Slowinski 05/11
//=======================================================================================
void DTMOverrideSymbologyManager::_QueryProperties (ElementHandleCR element, PropertyContextR context) 
    {
    DTMElementDisplayHandler::GetInstance()._QueryProperties (element, context);
    T_Super::_QueryProperties(element, context);
    if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
        {
        EachElementTemplateArg arg (TemplateRefAttributes::GetReferencedTemplateIDFromHandle (element), PROPSCALLBACK_FLAGS_IsBaseID, context);
        context.DoElementTemplateCallback (NULL, arg);
        }
    }

void DTMOverrideSymbologyManager::_OnElementLoaded (ElementHandleCR element)
    {
    ScanLevelsAndAddOrRemove (element, true, true);
    }

void DTMOverrideSymbologyManager::_OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source)
    {
    if (beforeUndoRedo)
        ScanLevelsAndAddOrRemove (*beforeUndoRedo, false);

    if (afterUndoRedo)
        ScanLevelsAndAddOrRemove (*afterUndoRedo, true);
    }

void DTMOverrideSymbologyManager::_OnHistoryRestore (ElementHandleP after, ElementHandleP before, ChangeTrackAction actionStep, BentleyDgnHistoryElementChangeType effectiveChange)
    {
    if (before)
        ScanLevelsAndAddOrRemove (*before, false);
    if (after)
        ScanLevelsAndAddOrRemove (*after, true);
    }

void DTMOverrideSymbologyManager::_OnDeleted (ElementHandleP element)
    {
    if (element)
        ScanLevelsAndAddOrRemove (*element, false);
    }

void DTMOverrideSymbologyManager::_OnAdded (ElementHandleP el)
    {
    ScanLevelsAndAddOrRemove (*el, true);
    }

void DTMOverrideSymbologyManager::_OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag)
    {
    ScanLevelsAndAddOrRemove (*oldElement, false);
    ScanLevelsAndAddOrRemove (*newElement, true);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
