/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudChannel.h $
|    $RCSfile: PointCloudChannel.h,v $
|   $Revision: 1.1 $
|       $Date: 2010/08/18 15:57:40 $
|     $Author: Simon.Normand $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudChannel : public RefCounted<IPointCloudChannel>
    {
    public:
        static IPointCloudChannelPtr Create(WStringCR name, WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore){return new PointCloudChannel (name, userInfo, typesize, multiple, defValue, outofCore);}
        static IPointCloudChannelPtr Create(PThandle handle, WStringCR persistentName, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore){return new PointCloudChannel (handle, persistentName, typesize, multiple, defValue, outofCore);}
        static IPointCloudChannelPtr CreateCopy (PointCloudChannel& channel, WStringCR newName, WStringCR newUserInfo){return new PointCloudChannel (channel, newName, newUserInfo);}
        
        PThandle GetHandle () const{return m_channelHandle->GetHandle ();}
        WStringCR   GetVersion () const {return m_version;}
        PointCloudChannelHandler* GetHandler () const {return m_pHandler;}

/* POINTCLOUD_WIP_GR06_ElementHandle)
        IPointCloudChannelDisplayHandler* GetDisplayHandler () const {return m_pDisplayHandler;}
        IPointCloudChannelQueryHandler* GetQueryHandler () const {return m_pQueryHandler;}
*/

    protected:
        PointCloudChannel (WStringCR name, WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore);
        PointCloudChannel (PThandle handle, WStringCR persistentName, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore);
        PointCloudChannel (PointCloudChannel& channel, WStringCR newName, WStringCR newUserInfo);
        virtual WStringCR               _GetName () const override              {return m_name;}
        virtual WStringCR               _GetPersistentName () const override    {return m_persistentName;}
        virtual WStringCR               _GetInfo () const override              {return m_userInfo;}
        virtual WStringCR               _GetCreatorTaskId () const override     {return m_taskId;}
        virtual unsigned int            _GetTypeSize () const override          {return m_typeSize;}
        virtual unsigned int            _GetMultiple ()const  override          {return m_multiple;}
        virtual void*                   _GetDefaultValue ()const  override      {return m_def;}
        virtual bool                    _IsOutOfCore () const override          {return m_outofCore;}
        virtual  void                   _SetHandler (PointCloudChannelHandler* pHandler) override
            {
            m_pHandler = pHandler;
/* POINTCLOUD_WIP_GR06_ElementHandle)
            m_pDisplayHandler = dynamic_cast<IPointCloudChannelDisplayHandler*>(m_pHandler);
            m_pQueryHandler = dynamic_cast<IPointCloudChannelQueryHandler*>(m_pHandler);
*/
            }
        virtual  PointCloudChannelHandler* _GetHandler () const override
            {
            return m_pHandler;
            }

    private:

        void BuildPersistentName ();

        PointCloudChannelHandler*       m_pHandler;
/* POINTCLOUD_WIP_GR06_ElementHandle)
        IPointCloudChannelDisplayHandler*       m_pDisplayHandler;
        IPointCloudChannelQueryHandler*         m_pQueryHandler;
*/
        RefCountedPtr<PointCloudChannelHandle>  m_channelHandle;

        WString m_version;
        WString m_name;
        WString m_persistentName;
        WString m_taskId;
        WString m_userInfo;
        void* m_def;
        bool m_outofCore;
        unsigned int m_multiple;
        unsigned int m_typeSize;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChannelsFileBuffer : public RefCounted<IChannelsFileBuffer>
    {
    public:
        static IChannelsFileBufferPtr Create (IPointCloudChannelPtrMap channels) {return new ChannelsFileBuffer(channels);}
        ~ChannelsFileBuffer();

    protected:
        ChannelsFileBuffer (IPointCloudChannelPtrMap channels);
        virtual uint64_t _GetSize () const {return m_buffersize;}
        virtual uint8_t* _GetBuffer () const {return (uint8_t*)m_buffer;}

        uint64_t m_bufferHandle;
        unsigned char* m_buffer;
        uint64_t m_buffersize;
    };

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
