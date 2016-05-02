/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudHandle.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|   NOTE: This file was moved from $(SrcRoot)MstnPlatform\PPModules\PointCloud\Component\PointCloudHandler\PointCloudHandle.h
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
template <class HandleDestroyer> class PointCloudHandle : public RefCounted<IRefCounted>
    {
    public:
        static PointCloudHandle<HandleDestroyer>* Create(PThandle handle) { return new PointCloudHandle(handle); }
        PThandle GetHandle() const { return m_handle; }
/* POINTCLOUD_WIP - Embedded Point Clouds
        void SetForeignFileReader (Bentley::DgnPlatform::ForeignFileReaderPtr reader) {m_reader = reader;}
*/        
    protected:
        PointCloudHandle(PThandle handle) : m_handle(handle) {}
        ~PointCloudHandle ()
            {
            m_destroyer(m_handle);
            }
        

    private:
        PointCloudHandle(); // disabled
        PointCloudHandle(PointCloudHandle const&);  // disabled
        PointCloudHandle& operator=(PointCloudHandle const&);   // disabled

        PThandle        m_handle;
        HandleDestroyer m_destroyer;
/* POINTCLOUD_WIP - Embedded Point Clouds
        Bentley::DgnPlatform::ForeignFileReaderPtr m_reader;
*/
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct DestroyChannelHandle
    {
    public:
        void operator()(PThandle handle) { ptDeletePointChannel(handle); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct DestroyMetaDataHandle
    {
    public:
        void operator()(PThandle handle) { ptFreeMetaData(handle); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct DestroySceneHandle
    {
    public:
        void operator()(PThandle handle) { ptRemoveScene(handle); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct DestroyQueryHandle
    {
    public:
        void operator()(PThandle handle) { ptDeleteQuery(handle); }
    };


typedef PointCloudHandle<DestroyChannelHandle>  PointCloudChannelHandle;
typedef PointCloudHandle<DestroyMetaDataHandle> PointCloudMetaDataHandle;
typedef PointCloudHandle<DestroySceneHandle>    PointCloudSceneHandle;
typedef PointCloudHandle<DestroyQueryHandle>    PointCloudQueryHandle;

typedef PointCloudChannelHandle*                PointCloudChannelHandleP;
typedef PointCloudMetaDataHandle*               PointCloudMetaDataHandleP;
typedef PointCloudSceneHandle*                  PointCloudSceneHandleP;
typedef PointCloudQueryHandle*                  PointCloudQueryHandleP;


typedef RefCountedPtr<PointCloudChannelHandle>    PointCloudChannelHandlePtr;
typedef RefCountedPtr<PointCloudMetaDataHandle>   PointCloudMetaDataHandlePtr;
typedef RefCountedPtr<PointCloudSceneHandle>      PointCloudSceneHandlePtr;
typedef RefCountedPtr<PointCloudQueryHandle>  PointCloudQueryHandlePtr;

//Used Only for tests
typedef PointCloudHandle<DestroyQueryHandle>    PointCloudFrustumQueryHandle;

END_BENTLEY_BEPOINTCLOUD_NAMESPACE