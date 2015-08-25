/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudClassification.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h" //always first

#include "PointCloudClassification.h"
//#include "PointCloudEditChannelUndoRedoManager.h"
//#include "PointCloudEditRasterColoringTool.h"

#include <DgnPlatform\ModelAccess.h>
#include <DgnPlatform\DelegatedElementECEnabler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <DgnPlatform\PointCloudHandler.h>

USING_NAMESPACE_BENTLEY

#define _CLASSIFICATION_FILE_EXT L"classif"
#define _DEFAULTCHANNELVALUE 66 
#define CLASSIFICATION_CHANNEL_NAME L"Classification"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

static ElementRefAppData::Key s_appDataKey;

// Static member initialization
//ChannelUndoRedoManager ChannelUndoRedoManager::s_instance;
SisterFileManager SisterFileManager::s_sinstance;
ClassificationChannelManager ClassificationChannelManager::s_instance;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ChannelAppData :  public ElementRefAppData
    {
    public:
        ChannelAppData (ClassificationChannelHandlerPtr channelHandlerP)
            :m_handlerP (channelHandlerP) {}

        ClassificationChannelHandler* GetHandler () {return m_handlerP.get();}

    protected:
        //ElementRefAppData
        virtual void    _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
            {
            uint32_t count = m_handlerP->Release ();
            if (count == 1)
                {
                ClassificationChannelManager::Get().NotifyDestruction (m_handlerP);
                }

            if (!unloadingCache)
                zone.Free (this, sizeof *this);
            }

        virtual bool _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason)  override
            {
            if (ELEMREF_CHANGE_REASON_Delete == reason)
                return true;

            // return true to free the data
            return false;
            }


    private:
        ClassificationChannelHandlerPtr  m_handlerP;
    };



ClassificationChannelManager::ClassificationChannelManager()
    {
    SisterFileManager::GetInstance().Register(&this->Get());
    //NEEDS_WORK_SM_IMPORTER : Deactivated
    //ChannelUndoRedoManager::GetInstance().Register(&this->Get());
    }

ClassificationChannelManager::~ClassificationChannelManager()
    {
    }

WString ClassificationChannelManager::_GetExtension()
    {
    return _CLASSIFICATION_FILE_EXT;
    }

void ClassificationChannelManager::_RemoveChannelHandler(EditElementHandleR curr)
    {
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler (curr);
    pData->RemoveChannelHandler(curr);
    }

void ClassificationChannelManager::_OnLoaded(ElementHandleR eh, IPointCloudChannelPtr channelPtr)
    {
    ClassificationChannelHandlerPtr ptr = ClassificationChannelHandler::Create(eh, channelPtr);

    // getting the channel handler for the element will link the channel to this element's userdata
    ClassificationChannelHandler::GetChannelHandler (eh);
    }

void ClassificationChannelManager::_OnSave(ElementHandleR eh, IPointCloudChannelPtr channelPtr, bool after)
    {
    if(!after)
        {
        ClassificationChannelHandlerPtr ptr = ClassificationChannelHandler::GetChannelHandler(eh);
        if(!ptr->HasPendingChange())
            return;
        ptr->SetHasPendingChange(false);
        }
    else
        {
        ClassificationChannelHandlerPtr ptr = ClassificationChannelHandler::GetChannelHandler(eh);
        //NEEDS_WORK_SM_IMPORTER : Deactivated
        //ChannelUndoRedoManager::GetInstance().SaveRedo(ptr->GetFullPath(), _GetExtension());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelManager::NotifyCreation (ElementHandleCR elHandle, ClassificationChannelHandlerPtr ptr)
    {
    WString channelName (SisterFileManager::GetInstance().GetChannelFileName (elHandle, _GetExtension()));
    m_items.insert (ClassificationChannelHandlerMap::value_type (channelName, ptr));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandlerPtr ClassificationChannelManager::FindForElement (ElementHandleCR elHandle)
    {
    WString channelName (SisterFileManager::GetInstance().GetChannelFileName (elHandle, ClassificationChannelManager::Get()._GetExtension()));

    ClassificationChannelHandlerMap::iterator itr = m_items.find (channelName);
    if (itr != m_items.end ())
        return itr->second;

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelManager::NotifyDestruction (ClassificationChannelHandlerPtr pHandler)
    {
    for (ClassificationChannelHandlerMap::iterator itr = m_items.begin (); itr != m_items.end(); ++itr)
        {
        if (itr->second == pHandler)
            {
            m_items.erase (itr);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelManager::_OnUnload (DgnModelRefP    modelRefP)
    {
    if (!modelRefP)
        return;

    T_StdElementRefSet eRefSet (PointCloudHandler::GetPointCloudElementRefsInModel (*modelRefP->GetDgnModelP ()));
    for (T_StdElementRefSet::iterator itr = eRefSet.begin(); itr != eRefSet.end(); ++itr)
        {
        ElementHandle  eh (*itr, modelRefP);
        ClassificationChannelHandlerPtr  channelPtr = FindForElement (eh);
        if (channelPtr.IsValid())
            channelPtr->Drop (eh);
        }

    // Create an iterator to loop through root model and/or child models
    ModelRefIteratorP iterator(new ModelRefIterator (modelRefP, MRITERATE_PrimaryChildRefs | MRITERATE_IncludeRedundant, 0));

    DgnModelRefP        childModelRef;    
    while (NULL != (childModelRef = iterator->GetNext()))
        {
        DgnAttachmentP refP(childModelRef->AsDgnAttachmentP());

        assert(refP != NULL);
    
        if (refP->GetFBOptsR().anonymous)
            continue; 

        _OnUnload (childModelRef);
        }

    delete iterator;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelManager::_ProcessElementHandle (ElementHandleR eh)
    {
    PointCloudHandler* handlerP = dynamic_cast<PointCloudHandler*>(&eh.GetHandler ());
    if (NULL == handlerP)
        return;

    WString fullPath;

    WString channelName (SisterFileManager::GetInstance().GetChannelFileName (eh, ClassificationChannelManager::Get()._GetExtension(), &fullPath));

    ClassificationChannelHandlerMap::iterator itr = m_items.find (channelName);
    if (itr != m_items.end ())
        {
        // a channel is already present for this file, getting the channel handler for the element will link the channel to this element's userdata
        ClassificationChannelHandler::GetChannelHandler (eh);
        return;
        }

    //we didn't find an existing channel, try to open a channel file for this element?
    SisterFileManager::GetInstance().OpenChannelFile(eh, fullPath, channelName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelManager::_ProcessElementChange (ChangeTrackAction     action,  XAttributeHandleCR      xAttr)
    {
    if(xAttr.GetHandlerId().GetMajorId () != XATTRIBUTEID_PointCloudHandler)
        return;

    if (action != ChangeTrackAction::XAttributeAdd)
        return;
    
    DgnModelP modelP = xAttr.GetElementRef()->GetDgnModelP();
    if (!modelP)
        return;      // We can't do anything without a model.
    
    ElementHandle eh(xAttr.GetElementRef (), modelP);
    _ProcessElementHandle (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandler::ClassificationChannelHandler (ElementHandleCR eh, IPointCloudChannelPtr channelPtr)
    :m_hasPending (false)
    {
    UChar defaultValue = _DEFAULTCHANNELVALUE;

    WString channelName (SisterFileManager::GetInstance().GetChannelFileName (eh, ClassificationChannelManager::Get()._GetExtension(), &m_filePath));

    WString userInfo(CLASSIFICATION_CHANNEL_NAME);


    if (!channelPtr.get())
        m_pChannel = PointCloudChannelManager::GetManager().CreateChannel (channelName.c_str(), userInfo.c_str(), sizeof(UChar), 1, &defaultValue, false);
    else
        m_pChannel = channelPtr;

    m_pChannel->SetHandler (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandlerPtr ClassificationChannelHandler::Create (ElementHandleCR eh)
    {
    ClassificationChannelHandlerPtr existingPtr = ClassificationChannelManager::Get().FindForElement (eh);

    if (existingPtr.get())
        return existingPtr;

    ClassificationChannelHandlerPtr ptr = new ClassificationChannelHandler(eh);
    ClassificationChannelManager::Get().NotifyCreation (eh, ptr);

    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandlerPtr ClassificationChannelHandler::Create (ElementHandleCR eh, IPointCloudChannelPtr channelPtr)
    {
    // this should be used only when the channel was open by a channel file
    ClassificationChannelHandlerPtr ptr = new ClassificationChannelHandler(eh, channelPtr);
    ClassificationChannelManager::Get().NotifyCreation (eh, ptr);

    return ptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandler::~ClassificationChannelHandler ()
    {
    if (m_hasPending)
        {
        assert (!"Unsaved channel file");
        //SaveChange ();
        //can't save here, the pointcloud is closed already
        }

    PointCloudChannelManager::GetManager().RemoveChannel (m_pChannel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelHandler::Drop(ElementHandleR elHandle)
    {
    ChannelAppData*  pData = dynamic_cast<ChannelAppData* > (elHandle.GetElementRef ()->FindAppData (s_appDataKey));
    if (pData)
        elHandle.GetElementRef ()->DropAppData (s_appDataKey);
    }

void ClassificationChannelHandler::RemoveChannelHandler(ElementHandleR eh)
    {
    if(!eh.GetElementRef())
        return;
    Drop(eh);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationChannelHandler* ClassificationChannelHandler::GetChannelHandler(ElementHandleR elHandle)
    {
    if (!elHandle.GetElementRef ())
        return NULL;

    ChannelAppData*  pData = dynamic_cast<ChannelAppData* > (elHandle.GetElementRef ()->FindAppData (s_appDataKey));
    if (!pData)
        {
        ElementRefP  elmRef (elHandle.GetElementRef ());
        assert (NULL != elmRef);
        assert (!elmRef->InDeletedCache());

        ClassificationChannelHandlerPtr handlerPtr = ClassificationChannelHandler::Create(elHandle);

        void* mem = elmRef->GetHeapZone().Alloc (sizeof ChannelAppData);
        pData = new (mem)  ChannelAppData(handlerPtr);

        elmRef->AddAppData (s_appDataKey, pData, elmRef->GetHeapZone());
        }
    return pData->GetHandler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassificationChannelHandler::_OnPreProcessClassification  (ViewContextR viewContext, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers)

    {
    SwapChannelValues (pPointCloudChannel, pPointCloudBuffers);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelHandler::_OnQuery(IPointCloudDataQueryCR query, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers)
    {
    SwapChannelValues (pPointCloudChannel, pPointCloudBuffers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationChannelHandler::SwapChannelValues (IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers)
    {
    UChar* channelBuffer = (UChar*)pPointCloudBuffers.GetChannelBuffer(pPointCloudChannel);
    UChar* classificationBuffer = (UChar*)pPointCloudBuffers.GetClassificationBuffer ();

    if (!classificationBuffer || !channelBuffer)
        return;

    UInt const bufferSize = pPointCloudBuffers.GetNumPoints();

    for (UInt j = 0; j < bufferSize; j++)
        {
        if (channelBuffer[j] != _DEFAULTCHANNELVALUE)
            {
            classificationBuffer[j] = channelBuffer[j];
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassificationChannelHandler::_CanHandle (ViewContextR viewContext, ElementHandleCR eh)
    {
    if (!eh.GetElementRef ())
        return false;

    ChannelAppData*  pData = dynamic_cast<ChannelAppData* > (eh.GetElementRef ()->FindAppData (s_appDataKey));
    if (!pData || pData->GetHandler ()!= this)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassificationChannelHandler::_CanHandle (IPointCloudDataQueryCR query, ElementHandleCR eh)
    {
    if (!eh.GetElementRef ())
        return false;

    ChannelAppData*  pData = dynamic_cast<ChannelAppData* > (eh.GetElementRef ()->FindAppData (s_appDataKey));
    if (!pData || pData->GetHandler ()!= this)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WString ClassificationChannelHandler::_GetDisplayName () const
    {
    /*NEEDS_WORK_SM_IMPORTER : What the name should be?        
    WString displayName;
    mdlResource_loadWString (&displayName, NULLRSC, MSGLISTIDS_PointCloudEditMisc, MSGMisc_PointCloudEditClassification, mdlSystem_findMdlDesc(DCPOINTCLOUDAPP_TASKID));

    return displayName;
    */

    return WString(L"");
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
