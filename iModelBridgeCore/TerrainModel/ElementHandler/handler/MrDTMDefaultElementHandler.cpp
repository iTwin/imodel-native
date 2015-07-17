/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MrDTMDefaultElementHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h" 

#include <TerrainModel\ElementHandler\DTMDataRef.h>
#include "MrDTMDataRef.h"
#include <DgnPlatform\TerrainModel\TMElementHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MrDTMDgnModelAppData::MrDTMDgnModelAppData()
    {

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelAppData::Key const& MrDTMDgnModelAppData::GetKey () 
    { 
    static DgnModelAppData::Key s_key; 
    return s_key; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MrDTMDgnModelAppData::GetNbMrDTM ()
    {
    return m_elementRefList.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MrDTMDgnModelAppData& MrDTMDgnModelAppData::GetAppData(DgnModelR model)               
    {
    MrDTMDgnModelAppData* appData = (MrDTMDgnModelAppData*) model.FindAppData (MrDTMDgnModelAppData::GetKey ());
    if(appData != NULL)
        return *appData;

    // If we do not have an appdata that means no MrDTM exist in this DgnModel.
    model.AddAppData (MrDTMDgnModelAppData::GetKey (), appData = new MrDTMDgnModelAppData ());
    return *appData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
T_StdElementRefSet const& MrDTMDgnModelAppData::GetMrDTMElementRefList() const
    {
    return m_elementRefList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MrDTMDgnModelAppData::_OnCleanup (DgnModelR host)
    { 
    delete this; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MrDTMDgnModelAppData::_OnSaveModelProperties (DgnModelR host, ModelInfoCR original)
    {
    for (T_StdElementRefSet::const_iterator i = m_elementRefList.begin(); i != m_elementRefList.end(); ++i)
        {
        ElementHandle  eh (*i);
        RefCountedPtr<DTMDataRef> ref;
        ref = DTMDataRef::GetDTMAppData (eh);
        if (ref.IsValid ())
            ref->UpdateAfterModelUnitDefinitionChange();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool MrDTMDgnModelAppData::AddMrDTMElementRef(ElementRefP elemRefP)
    {
    return m_elementRefList.insert(elemRefP).second;     // Return true if added.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MrDTMDgnModelAppData::RemoveMrDTMElementRef(ElementRefP elemRefP)
    {
    return m_elementRefList.erase(elemRefP);    // Return the number of elements erased.
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 05/11
//=======================================================================================
void MrDTMElementDisplayHandler::_GetTypeName(WStringR string, UInt32 desiredLength) 
    {
    MrDTMDefaultElementHandler::GetInstance().GetTypeName(string, desiredLength);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 05/11
//=======================================================================================
ElementHandlerId MrDTMElementDisplayHandler::GetElemHandlerId() 
    {
    return ElementHandlerId (MrDTMElementMajorId, ELEMENTHANDLER_MRDTMELEMENT);
    } 

//=======================================================================================
// @bsimethod                                                   Chantal.Poulin 02/12
//=======================================================================================
bool MrDTMElementDisplayHandler::_IsSupportedOperation (ElementHandleCP eh, Element::SupportOperation stype)
    {
    // N.B. If cell is supported, we must also change {dgnfileio\lib\history\utils.cpp} isNeverComplexComponent()
    if (SupportOperation::CellGroup == stype)       
        return false;   

    return T_Super::_IsSupportedOperation (eh, stype);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre   10/11
//=======================================================================================
void MrDTMElementDisplayHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {       
    //It is not possible to have a STM element in a 2D model.
    ElementHandle::XAttributeIter xAttrHandle (eeh);

    while (xAttrHandle.IsValid())
        {
        XAttributeHandlerId handId = xAttrHandle.GetHandlerId();
        int id = xAttrHandle.GetId();

        if (handId.GetMajorId () == TMElementMajorId)
            {
            bool hasNext = xAttrHandle.ToNext();
            eeh.ScheduleDeleteXAttribute (handId, id);
            if (!hasNext)
                break; //ToDo LookAt
            }
        else
            {
            if (!xAttrHandle.ToNext())
                break; //ToDo LookAt
            }
        }

    eeh.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MrDTMElementDisplayHandler::_OnElementLoaded (ElementHandleCR eh)
    {
    T_Super::_OnElementLoaded(eh);

    // Keep track of this loaded MrDTM element (add it to the point cloud list)
    MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*eh.GetDgnModelP());
    dgnModelData.AddMrDTMElementRef(eh.GetElementRef());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet  06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MrDTMElementDisplayHandler::_OnAdded (ElementHandleP eh)
    {
    T_Super::_OnAdded(eh);

    // A MrDTM element was added to the model. Add it to the MrDTM list.
    MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*eh->GetDgnModelP());
    dgnModelData.AddMrDTMElementRef(eh->GetElementRef());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet  06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MrDTMElementDisplayHandler::_OnDeleted (ElementHandleP eh)
    {
    T_Super::_OnDeleted(eh);

    // A MrDTM element was removed from the model. Remove it from the MrDTM list.
    MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*eh->GetDgnModelP());
    dgnModelData.RemoveMrDTMElementRef(eh->GetElementRef());
    }
  
//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre   11/11
//=======================================================================================
void MrDTMElementDisplayHandler::OnModelRefActivate (DgnModelRefR newModelRef, DgnModelRefP oldModelRef)
    {       
    if (oldModelRef != 0)
        {
        MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*oldModelRef->GetDgnModelP());

        T_StdElementRefSet elementRefSet = dgnModelData.GetMrDTMElementRefList();

        T_StdElementRefSet::iterator elemRefIter(elementRefSet.begin());
        T_StdElementRefSet::iterator elemRefIterEnd(elementRefSet.end());

        while (elemRefIter != elemRefIterEnd)
            {
            ElementHandle  eh (*elemRefIter, oldModelRef);

            RefCountedPtr<DTMDataRef> ref;
            StatusInt result = DTMElementHandlerManager::GetDTMDataRef (ref, eh);

            assert(result == SUCCESS && ref->IsMrDTM() == true);
                
            RefCountedPtr<IMrDTMDataRef> mrRef = (IMrDTMDataRef*)ref.get();

            mrRef->ClearCachedMaterials();        
            elemRefIter++;
            }
        }


    MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*newModelRef.GetDgnModelP());

    T_StdElementRefSet elementRefSet = dgnModelData.GetMrDTMElementRefList();

    T_StdElementRefSet::iterator elemRefIter(elementRefSet.begin());
    T_StdElementRefSet::iterator elemRefIterEnd(elementRefSet.end());

    while (elemRefIter != elemRefIterEnd)
        {
        ElementHandle  eh (*elemRefIter, &newModelRef);

        RefCountedPtr<DTMDataRef> ref;
        StatusInt result = DTMElementHandlerManager::GetDTMDataRef (ref, eh);

        assert(result == SUCCESS && ref->IsMrDTM() == true);
            
        RefCountedPtr<IMrDTMDataRef> mrRef = (IMrDTMDataRef*)ref.get();

        mrRef->ClearCachedMaterials();        
        elemRefIter++;
        }        
    }


//=======================================================================================
// @bsimethod                                                   Chantal.Poulin   02/12
//=======================================================================================
void MrDTMElementDisplayHandler::OnModelRefActivated (DgnModelRefR newModelRef, DgnModelRefP oldModelRef)
    {
    //Unactivated model 
    if (oldModelRef != 0)
        {
        MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*oldModelRef->GetDgnModelP());

        T_StdElementRefSet elementRefSet = dgnModelData.GetMrDTMElementRefList();

        T_StdElementRefSet::iterator elemRefIter(elementRefSet.begin());
        T_StdElementRefSet::iterator elemRefIterEnd(elementRefSet.end());

        while (elemRefIter != elemRefIterEnd)
            {
            ElementHandle  eh (*elemRefIter, oldModelRef);

            RefCountedPtr<DTMDataRef> ref;
            StatusInt result = DTMElementHandlerManager::GetDTMDataRef (ref, eh);

            assert(result == SUCCESS && ref->IsMrDTM() == true);
                
            RefCountedPtr<MrDTMDataRef> mrRef = (MrDTMDataRef*)ref.get();

            mrRef->SetReadOnly (true, false);  

            elemRefIter++;
            }
        }

    //Active model     
    MrDTMDgnModelAppData& dgnModelData = MrDTMDgnModelAppData::GetAppData(*newModelRef.GetDgnModelP());

    T_StdElementRefSet elementRefSet = dgnModelData.GetMrDTMElementRefList();

    T_StdElementRefSet::iterator elemRefIter(elementRefSet.begin());
    T_StdElementRefSet::iterator elemRefIterEnd(elementRefSet.end());

    while (elemRefIter != elemRefIterEnd)
        {
        ElementHandle  eh (*elemRefIter, &newModelRef);

        RefCountedPtr<DTMDataRef> ref;
        StatusInt result = DTMElementHandlerManager::GetDTMDataRef (ref, eh);

        assert(result == SUCCESS && ref->IsMrDTM() == true);
            
        RefCountedPtr<MrDTMDataRef> mrRef = (MrDTMDataRef*)ref.get();

        bool isReadOnly = true;      

        EditElementHandle editElementHandle(mrRef->GetElement().GetElementRef(), &newModelRef);
        ElementHandle::XAttributeIter xAttrIterReadOnly(editElementHandle);

        XAttributeHandlerId xAttrMrDTMDetailsHandlerId = XAttributeHandlerId(MrDTMElementMajorId, XATTRIBUTES_SUBID_MRDTM_DETAILS);                    

        //The read only flag need to be read
        if (xAttrIterReadOnly.Search(xAttrMrDTMDetailsHandlerId, MRDTM_DETAILS_IS_READ_ONLY))
            {
            memcpy(&isReadOnly, (byte*)xAttrIterReadOnly.PeekData(), sizeof(isReadOnly));       
            }

        mrRef->SetReadOnly (isReadOnly, false);  
        
        elemRefIter++;
        }        
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool MrDTMElementDisplayHandler::IsHighQualityDisplayForMrDTM()
    {
    WString cfgValue;
    if (SUCCESS == ConfigurationManager::GetVariable(cfgValue, L"STM_PRESENTATION_QUALITY") && 
        wcscmp(cfgValue.c_str(), L"1") == 0)
        {
        return true;
        }

    return false;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
