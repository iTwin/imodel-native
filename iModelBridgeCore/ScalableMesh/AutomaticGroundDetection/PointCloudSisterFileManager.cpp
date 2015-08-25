/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudSisterFileManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshPCH.h" //always first

#include "PointCloudSisterFileManager.h"

#include <DgnPlatform\DelegatedElementECEnabler.h>
#include <DgnPlatform\ModelAccess.h>

#include <ScalableMesh\ScalableMeshLib.h>
#include <ScalableMesh\ScalableMeshAdmin.h>


USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <DgnPlatform\PointCloudHandler.h>





USING_NAMESPACE_BENTLEY

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SisterFileManager& SisterFileManager::GetInstance()
    {
    return s_sinstance;
    }

SisterFileManager::~SisterFileManager()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString SisterFileManager::GetChannelFileName(ElementHandleCR eh, WStringCR extensionString, WStringP fullPath)
    {
    IPointCloudQuery* pQuery = dynamic_cast<IPointCloudQuery*>(&eh.GetHandler());
    PointCloudPropertiesPtr attributes = pQuery->GetPointCloudProperties(eh);

    // Resolve the path
    Bentley::WString inFilename = attributes->GetFileMoniker().ResolveDisplayName();
    
    WString devStr, dirStr, nameStr;    
    BeFileName::ParseName (&devStr, &dirStr, &nameStr, 0, inFilename.c_str());   

    WString channelName(nameStr);
    channelName.append(L".").append(extensionString.c_str());

    if(fullPath)
        {
        WString path;
        BeFileName::BuildName (path, devStr.c_str(), dirStr.c_str(), nameStr.c_str(), extensionString.c_str());
        fullPath->append(path.c_str());
        }

    return channelName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::Register(IChannelFileListener* channelFileListener)
    {
    if(m_mapExtensionToChannelFileListner.find(channelFileListener->_GetExtension()) != m_mapExtensionToChannelFileListner.end())
            assert("!This extension already exist...");
    m_mapExtensionToChannelFileListner.insert(SisterFileMap::value_type(channelFileListener->_GetExtension(), channelFileListener));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::UnRegister(IChannelFileListener* channelFileListener)
    {
    m_mapExtensionToChannelFileListner.erase(channelFileListener->_GetExtension());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::OpenChannelFile(ElementHandleR eh, WStringCR fullpath, WStringCR channelName)
    {
    IPointCloudChannelPtrMap chMap = PointCloudChannelManager::GetManager().LoadFromFile(fullpath.c_str());
    if(!chMap.size())
        return;

    if(chMap.size() != 1)
        {
        assert(!"Ignoring channel file with more than one channel");
        for(IPointCloudChannelPtrMap::iterator channelItr (chMap.begin()); channelItr != chMap.end(); ++channelItr)
            PointCloudChannelManager::GetManager().RemoveChannel(channelItr->second);
        return;
        }
        
    WString extStr;

    BeFileName::ParseName (NULL, NULL, NULL, &extStr, channelName.c_str());

    SisterFileMap::iterator itExtToFile;
    if((itExtToFile = m_mapExtensionToChannelFileListner.find(extStr.c_str())) != m_mapExtensionToChannelFileListner.end())
        {
        itExtToFile->second->_OnLoaded(eh, chMap.begin()->second);
        }
    else
        assert(!"no extension registred");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::SaveChannelToFile(ElementHandleR eh, IPointCloudChannelPtr channelPtr, WString extension)
    {
    SisterFileMap::iterator itExtToFile;
    if((itExtToFile = m_mapExtensionToChannelFileListner.find(extension)) != m_mapExtensionToChannelFileListner.end())
        {
        itExtToFile->second->_OnSave(eh, channelPtr, false);
        IPointCloudChannelPtrMap channelMap;
        WString fullPath;
        GetChannelFileName(eh, extension, &fullPath);
        channelMap.insert (IPointCloudChannelPtrMap::value_type (channelPtr->GetPersistentName (), channelPtr));
        PointCloudChannelManager::GetManager().SaveToFile (channelMap, fullPath.c_str());

        itExtToFile->second->_OnSave(eh, channelPtr, true);
        }
    else
        assert(!"no extension registred");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::ProcessElementHandle(ElementHandleR eh)
    {
    SisterFileMap::iterator itExtToFile;
    for(itExtToFile = m_mapExtensionToChannelFileListner.begin(); itExtToFile != m_mapExtensionToChannelFileListner.end(); itExtToFile++)
        {
        itExtToFile->second->_ProcessElementHandle(eh);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::ProcessElementChange(ChangeTrackAction action, XAttributeHandleCR xAttr)
    {
    SisterFileMap::iterator itExtToFile;
    for(itExtToFile = m_mapExtensionToChannelFileListner.begin(); itExtToFile != m_mapExtensionToChannelFileListner.end(); itExtToFile++)
        {
        itExtToFile->second->_ProcessElementChange(action, xAttr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::ProcessModel(DgnModelRefP modelRef, bool processChildren)
    {
    T_StdElementRefSet eRefSet (PointCloudHandler::GetPointCloudElementRefsInModel (*modelRef->GetDgnModelP ()));
    for (T_StdElementRefSet::const_iterator itr = eRefSet.begin(); itr != eRefSet.end(); ++itr)
        {
        ElementHandle  eh (*itr, modelRef);

        SisterFileMap::iterator itExtToFile;
        for(itExtToFile = m_mapExtensionToChannelFileListner.begin(); itExtToFile != m_mapExtensionToChannelFileListner.end(); itExtToFile++)
            {
             itExtToFile->second->_ProcessElementHandle(eh);
            }
        }

    if (!processChildren)
        return;
   
    // Create an iterator to loop through root model and/or child models
    ModelRefIteratorP iterator(new ModelRefIterator (modelRef, MRITERATE_PrimaryChildRefs | MRITERATE_IncludeRedundant, 0));

    DgnModelRefP        childModelRef;    
    while (NULL != (childModelRef = iterator->GetNext()))
        {
        DgnAttachmentP refP(modelRef->AsDgnAttachmentP());

        assert(refP != NULL);
    
        if (refP->GetFBOptsR().anonymous)
            continue; 
        
        ProcessModel (childModelRef, processChildren);
        }

    delete iterator;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SisterFileManager::OnUnload(DgnModelRefP modelRef)
    {
    SisterFileMap::iterator itExtToFile;
    for(itExtToFile = m_mapExtensionToChannelFileListner.begin(); itExtToFile != m_mapExtensionToChannelFileListner.end(); itExtToFile++)
        {
        itExtToFile->second->_OnUnload(modelRef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnChangeTrackUndoRedo
    (
    MSElementDescrP afterUndoRedo,
    MSElementDescrP beforeUndoRedo,
    ChangeTrackAction action,
    bool isUndo,
    ChangeTrackInfo const* info,
    ChangeTrackSource source
    )
    {
    if (source != ChangeTrackSource::UndoRedo)
        return;

    if (isUndo && (action == ChangeTrackAction::Delete))
        {
        // undo of delete, it's like a attach but the xAttribute changetrack code isn't called
        MSElementDescrP descP = afterUndoRedo;
        if (NULL == descP)
            descP = beforeUndoRedo;

        if (NULL == descP || EXTENDED_ELM != descP->el.ehdr.type)
            return;

        ElementHandle eh (descP->h.elementRef, descP->h.dgnModelRef);
        PointCloudHandler* handlerP = dynamic_cast<PointCloudHandler*>(&eh.GetHandler ());
        if (NULL == handlerP)
            return;

        SisterFileManager::GetInstance().ProcessElementHandle(eh);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnChangeTrackXAttributeChanged
    (
    XAttributeHandleCP      xAttr,
    ChangeTrackInfo*        info,
    bool*                cantBeUndoneFlag
    )
    {
    SisterFileManager::GetInstance().ProcessElementChange(info->action, *xAttr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    PointCloudSisterFileMdlEvents::OnChangeTrackXAttributeUndoRedo
    (
    XAttributeHandleCP      xAttr,
    ChangeTrackAction     action,             // the action that happened
    bool                 isUndo,             // if TRUE -> this is an undo, if FALSE -> this is a redo
    ChangeTrackInfo const*  info,               // info about command that cause original change. CAN BE NULL!!!
    ChangeTrackSource     source              // the source of the change (undo, restore, merge)
    )
    {
    ChangeTrackAction netAction(action);
    if(ChangeTrackAction::XAttributeDelete == action && isUndo)
        netAction = ChangeTrackAction::XAttributeAdd;
    else if(ChangeTrackAction::XAttributeAdd == action && isUndo)
        netAction = ChangeTrackAction::XAttributeDelete;

    switch(source)
        {
    case ChangeTrackSource::UndoRedo:
        {
        SisterFileManager::GetInstance().ProcessElementChange(netAction, *xAttr);
        }
        break;

    case ChangeTrackSource::HistoryRestore:
    case ChangeTrackSource::HistoryMerge:
        break;
    default:
        assert(!"Unknown ChangeTrackSource in OnChangeTrackXAttributeUndoRedo");
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnCompressDgnFile (CompressType type)
    {
    if (type != POST_COMPRESS_DGNFILE)
        return;
    
    DgnModelRefP    modelRef (mdlModelRef_getRootParent (ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()));
    if (modelRef)
        SisterFileManager::GetInstance().ProcessModel (modelRef, false);        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnModelChanged(DgnModelRefP    modelRef, int changeType)
    {
    //NEEDS_WORK_SM_IMPORTER : Temporary deactivated
    /*
    if (changeType != MODEL_CHANGE_Active)
        return;
    SisterFileManager::GetInstance().ProcessModel(modelRef);
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnReferenceAttached (DgnModelRefP modelRef, DgnAttachmentAttachedReason cause)
    {
    SisterFileManager::GetInstance().ProcessModel(modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudSisterFileMdlEvents::OnUnload ()
    {    
    SisterFileManager::GetInstance().OnUnload(mdlModelRef_getRootParent (ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()));
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
