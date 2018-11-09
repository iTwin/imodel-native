/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudChannel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BePointCloudInternal.h"

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/* POINTCLOUD_WIP_GR06_ElementHandle
static PointCloudChannelManager s_channelManager;
static PointCloudChannelHandlerManager   s_channelHandlerManager;
*/

#define _VERSION L"1.00"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
PointCloudChannel::PointCloudChannel (WStringCR name,  WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore)
    :m_pHandler (NULL), m_pDisplayHandler (NULL),
    m_name (name), m_typeSize (typesize), m_multiple (multiple) , m_def (defValue), m_outofCore (outofCore), m_version (_VERSION)
*/
PointCloudChannel::PointCloudChannel (WStringCR name,  WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore)
    :m_pHandler (NULL), 
    m_name (name), m_typeSize (typesize), m_multiple (multiple) , m_def (defValue), m_outofCore (outofCore), m_version (_VERSION)
    {
#ifdef BEIJING_WIP_POINTCLOUD // what can we use instead of mdlSystem_getCurrTaskID ?
    m_taskId = WString (mdlSystem_getCurrTaskID ());
#endif

    if (userInfo)
        m_userInfo = *userInfo;

    BuildPersistentName ();

    PThandle handle =  ptCreatePointChannel ((wchar_t *)m_persistentName.c_str(), typesize, multiple, defValue, outofCore);
    m_channelHandle = PointCloudChannelHandle::Create (handle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
PointCloudChannel::PointCloudChannel (PThandle handle, WStringCR persistentname, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore) 
    :m_pHandler (NULL), m_pDisplayHandler (NULL),
    m_persistentName (persistentname), m_typeSize (typesize), m_multiple (multiple) , m_def (defValue), m_outofCore (outofCore)
*/
PointCloudChannel::PointCloudChannel (PThandle handle, WStringCR persistentname, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore) 
    :m_pHandler (NULL), 
    m_persistentName (persistentname), m_typeSize (typesize), m_multiple (multiple) , m_def (defValue), m_outofCore (outofCore)
    {
    size_t pos2 = m_persistentName.find(L";", 0);
    m_version = m_persistentName.substr (0, pos2);
    size_t pos = pos2+1;
    pos2 = m_persistentName.find(L";", pos);
    m_name = m_persistentName.substr (pos, pos2-pos);
    pos = pos2+1;
    pos2 = m_persistentName.find(L";", pos);
    m_userInfo = m_persistentName.substr (pos, pos2-pos);
    pos = pos2+1;
    pos2 = m_persistentName.find(L";", pos);
    m_taskId = m_persistentName.substr (pos, pos2-pos);

    m_channelHandle = PointCloudChannelHandle::Create (handle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_GR06_ElementHandle
PointCloudChannel::PointCloudChannel (PointCloudChannel& channel, WStringCR newName, WStringCR userInfo)
    :m_pHandler (NULL), m_pDisplayHandler (NULL), m_name (newName), m_version (channel.m_version), m_userInfo (userInfo), m_taskId (channel.m_taskId),
     m_typeSize (channel.m_typeSize), m_multiple (channel.m_multiple) , m_def (channel.m_def), m_outofCore (channel.m_outofCore)
*/
PointCloudChannel::PointCloudChannel (PointCloudChannel& channel, WStringCR newName, WStringCR userInfo)
    :m_pHandler (NULL), m_name (newName), m_version (channel.m_version), m_userInfo (userInfo), m_taskId (channel.m_taskId),
     m_typeSize (channel.m_typeSize), m_multiple (channel.m_multiple) , m_def (channel.m_def), m_outofCore (channel.m_outofCore)
    {
    BuildPersistentName ();

    PThandle handle =  ptCopyPointChannel (channel.GetHandle(), (wchar_t *)m_persistentName.c_str(), m_outofCore);
    m_channelHandle = PointCloudChannelHandle::Create (handle);
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudChannel::BuildPersistentName ()
    {
    // PersistentName syntax is Version;Name;Info;TaskId;Id
    wchar_t    id[6];
    swprintf (id, 6, L"%d", rand());
    m_persistentName.append (m_version);
    m_persistentName.append (L";");
    m_persistentName.append (m_name);
    m_persistentName.append (L";");
    m_persistentName.append (m_userInfo);
    m_persistentName.append (L";");
    m_persistentName.append (m_taskId);
    m_persistentName.append (L";");
    m_persistentName.append (id);
    }



/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR   IPointCloudChannel::GetName () const          {return _GetName ();}
WStringCR   IPointCloudChannel::GetPersistentName () const {return _GetPersistentName ();}
WStringCR   IPointCloudChannel::GetInfo () const         {return _GetInfo ();}
WStringCR   IPointCloudChannel::GetCreatorTaskId () const         {return _GetCreatorTaskId ();}
unsigned int IPointCloudChannel::GetTypeSize () const      {return _GetTypeSize ();}
unsigned int IPointCloudChannel::GetMultiple ()const      {return _GetMultiple ();}
void*       IPointCloudChannel::GetDefaultValue ()const  {return _GetDefaultValue ();}
bool        IPointCloudChannel::IsOutOfCore () const     {return _IsOutOfCore ();}
void        IPointCloudChannel::SetHandler (PointCloudChannelHandler* handler)  {_SetHandler (handler);}
PointCloudChannelHandler* IPointCloudChannel::GetHandler () const   {return _GetHandler ();}


#if defined (POINTCLOUD_WIP_GR06_UserChannels)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelManager& PointCloudChannelManager::GetManager () {return s_channelManager;}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtr PointCloudChannelManager::CreateChannel (WStringCR name, WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore)
    {
    // validate channel name and info length
    size_t length =  name.length();
    if (userInfo)
        length += userInfo->length();

    if (length > MAX_CHANNELDATASTRINGLENGTH)
        return NULL;

    
    IPointCloudChannelPtr obj (PointCloudChannel::Create  (name, userInfo, typesize, multiple, defValue, outofCore));
    m_channels.insert (IPointCloudChannelPtrMap::value_type (obj->GetPersistentName (), obj));
    
    return obj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtr PointCloudChannelManager::CreateChannelCopy (IPointCloudChannelPtr existingChannel, WStringCR nameName, WStringCR userInfo)
    {
    size_t newLength = nameName.length() + userInfo.length() + existingChannel->GetPersistentName ().length ();
    newLength -= existingChannel->GetName().length() + existingChannel->GetInfo().length();
    if (newLength > MAX_CHANNELDATASTRINGLENGTH)
        return NULL;

    PointCloudChannel* pChannel (dynamic_cast<PointCloudChannel*>(existingChannel.get()));
    IPointCloudChannelPtr obj (PointCloudChannel::CreateCopy (*pChannel, nameName, userInfo));
    m_channels.insert (IPointCloudChannelPtrMap::value_type (obj->GetPersistentName (), obj));
    return obj;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    PointCloudChannelManager::RemoveChannel (IPointCloudChannelPtr channelP)
    {
    m_channels.erase (channelP->GetPersistentName ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtrMapCR    PointCloudChannelManager::GetChannels ()
    {
    return m_channels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtrMap PointCloudChannelManager::LoadFromBuffer (void* buffer, uint64_t bufferSize)
    {
    const PThandle *channelHandles = 0;
    int32_t numChannels = 0;
    ptReadChannelsFileFromBuffer (buffer, bufferSize, numChannels, &channelHandles);
    
    return LoadChannels(channelHandles, numChannels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtrMap PointCloudChannelManager::LoadFromFile (WStringCR filename)
    {
    const PThandle *channelHandles = 0;
    int32_t numChannels = 0;

    ptReadChannelsFile ( filename.c_str(), numChannels, &channelHandles);

    return LoadChannels((const uint32_t*)channelHandles, numChannels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelPtrMap PointCloudChannelManager::LoadChannels (const uint32_t* channelHandles, int32_t numChannels)
    {
    IPointCloudChannelPtrMap channelList;

    for (int32_t i = 0; i < numChannels; i++)
        {
        wchar_t name[1024];
        uint32_t typesize;
        uint32_t multiple;
        uint32_t flags;

        if (PTV_SUCCESS != ptGetChannelInfo (channelHandles[i], name, typesize, multiple, NULL, flags))
            continue; //if we can't retrieve channel info, we should ignore it

        Byte* defaultValue = (Byte*)_alloca (typesize * multiple);
        ptGetChannelInfo (channelHandles[i], name, typesize, multiple, defaultValue, flags);

        IPointCloudChannelPtr channel (PointCloudChannel::Create (channelHandles[i], WString(name), typesize, multiple, (void*)defaultValue, flags ? true : false));

        PointCloudChannel* pChannel (dynamic_cast<PointCloudChannel*>(channel.get()));
        if (pChannel->GetVersion () != WString (_VERSION))
            {
            // wrong version
            continue;
            }
        // push it in both maps
        bpair<IPointCloudChannelPtrMap::iterator,bool> ret;

        ret = m_channels.insert (IPointCloudChannelPtrMap::value_type (channel->GetPersistentName(), channel));

        //make sure that the name is not already in use.
        if (ret.second == true)
            channelList.insert (IPointCloudChannelPtrMap::value_type (channel->GetPersistentName(), channel));
        }
    return channelList;

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelManager::SaveToFile (IPointCloudChannelPtrMapCR channels, WStringCR filename)
    {
    bvector<PThandle> handles;

    for (IPointCloudChannelPtrMap::const_iterator itr (channels.begin()); itr != channels.end(); ++itr)
        {
        IPointCloudChannelPtr channel (itr->second);
        PointCloudChannel* pChannel (dynamic_cast<PointCloudChannel*>(channel.get()));
        if (pChannel)
            handles.push_back (pChannel->GetHandle());
        }
    if (handles.size()==0)
        return ERROR;

    StatusInt   res = ptWriteChannelsFile (filename.c_str(), (int32_t)handles.size(), &handles[0]);
    
    if (PTV_SUCCESS == res)
        return SUCCESS;

    // other error codes that we could return...
    //PTV_FILE_NOTHING_TO_WRITE There is no user channel data to write
    //PTV_FILE_WRITE_FAILURE There was a write failure
    //PTV_FILE_FAILED_TO_CREATE The file failed to be created. This might be due to user permissions or file write being block by the OS or Anti-virus software

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelManager::SetOutofCoreFolder (WStringCR path)
    {
    if (PTV_SUCCESS == ptSetChannelOOCFolder (path.c_str()))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelManager::LoadEmbeddedChannelsByName (IPointCloudChannelPtrMap& channels, WStringCR channelName)
    {
    MapWStringElementRef::iterator embeddedChannelItr(m_embeddedChannels.find(channelName));
    if (embeddedChannelItr == m_embeddedChannels.end())
        return ERROR;   // This channel is not registered


    DgnElementP eRef = embeddedChannelItr->second;

    if (NULL == eRef)
        return ERROR;

    // so we have an embedded channel file matching this name, was is already loaded?
    if (SUCCESS == FindChannelsByName(channels, channelName))
        {
        channels.clear ();
        return ERROR;
        }

/* POINTCLOUD_WIP_PointCloudChannelFileData
    PointCloudChannelFileData channelData;
    if (SUCCESS != channelData.InitFromElement(eRef))
        return ERROR;

    channels = LoadFromBuffer (channelData.GetBuffer (), channelData.GetBufferSize());
*/
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelManager::FindChannelsByName (IPointCloudChannelPtrMap& channels, WStringCR channelName)
    {
    IPointCloudChannelPtrMap outMap;
    for (IPointCloudChannelPtrMap::const_iterator itor = m_channels.begin(); itor != m_channels.end(); ++itor)
        {
        if (itor->second->GetName () == channelName)
            {
            channels.insert (IPointCloudChannelPtrMap::value_type (itor->first, itor->second));
            }
        }

    return channels.size () ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudChannelManager::IsEmbeddedChannelRegistered(DgnElementP host)
    {
    for (MapWStringElementRef::iterator itr = m_embeddedChannels.begin(); itr != m_embeddedChannels.end(); ++itr)
        {
        if (itr->second == host)
            return true;
        }

    return false;
    }

#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChannelsFileBuffer::ChannelsFileBuffer (IPointCloudChannelPtrMap channels)
    :m_bufferHandle (0), m_buffer(NULL), m_buffersize(0)
    {
    std::vector<PThandle> handles;

    for (IPointCloudChannelPtrMap::const_iterator itr (channels.begin()); itr != channels.end(); ++itr)
        {
        IPointCloudChannelPtr channel (itr->second);
        PointCloudChannel* pChannel (dynamic_cast<PointCloudChannel*>(channel.get()));
        if (pChannel)
            handles.push_back (pChannel->GetHandle());
        }

    m_bufferHandle = ptWriteChannelsFileToBuffer ((int32_t)handles.size(), &handles[0], m_buffer, m_buffersize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChannelsFileBuffer::~ChannelsFileBuffer ()
    {
    if (m_bufferHandle)
        ptReleaseChannelsFileBuffer (m_bufferHandle);
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t IChannelsFileBuffer::GetSize () const          {return _GetSize ();}
uint8_t*   IChannelsFileBuffer::GetBuffer () const          {return _GetBuffer ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IChannelsFileBufferPtr  IChannelsFileBuffer::CreateChannelsFileBuffer (IPointCloudChannelPtrMapCR channels)
    {
    return ChannelsFileBuffer::Create (channels);
    }

/*---------------------------------------------------------------------------------**//**
* PointCloudChannelHandlerManager
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelHandlerManager::PointCloudChannelHandlerManager() {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelHandlerManager::~PointCloudChannelHandlerManager() {}


#if defined (POINTCLOUD_WIP_GR06_UserChannels)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelHandlerManager& PointCloudChannelHandlerManager::GetManager ()
    {
    return s_channelHandlerManager;
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelHandlerManager::RegisterHandler(PointCloudChannelHandlerP handler)
    {
    for (PointCloudChannelHandlers::iterator itr(m_handlers.begin()); itr != m_handlers.end(); ++ itr)
        {
        if (*itr == handler)
            return ERROR;
        }

    m_handlers.push_back(handler);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudChannelHandlerManager::UnregisterHandler(PointCloudChannelHandlerP handler)
    {
    for (PointCloudChannelHandlers::iterator itr(m_handlers.begin()); itr != m_handlers.end(); ++ itr)
        {
        if (*itr == handler)
            {
            m_handlers.erase(itr);
            return SUCCESS;
            }
        }
    
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelHandlers const&  PointCloudChannelHandlerManager::GetHandlers() const
    {
    return m_handlers;
    }


