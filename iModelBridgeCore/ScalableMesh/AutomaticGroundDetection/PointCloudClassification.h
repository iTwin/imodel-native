/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudClassification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once

#include "PointCloudSisterFileManager.h"


USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define CLASSIFCHANNEL_KEY L"ClassificationChannel";


class ClassificationChannelHandler;
typedef Bentley::RefCountedPtr<ClassificationChannelHandler>   ClassificationChannelHandlerPtr;

typedef std::map<WString, ClassificationChannelHandlerPtr> ClassificationChannelHandlerMap;
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ClassificationChannelManager : public IChannelFileListener
    {
    static  ClassificationChannelManager s_instance;
    ClassificationChannelManager ();
    ClassificationChannelManager(const ClassificationChannelManager &);             // intentionally undefined
    ClassificationChannelManager & operator=(const ClassificationChannelManager &); // intentionally undefined
    ~ClassificationChannelManager();


    public:
        static ClassificationChannelManager& Get () {return s_instance;}

        void NotifyCreation (ElementHandleCR elHandle, ClassificationChannelHandlerPtr ptr);
        void NotifyDestruction (ClassificationChannelHandlerPtr pHandler);
        ClassificationChannelHandlerPtr FindForElement (ElementHandleCR elHandle);

        virtual void _ProcessElementHandle (ElementHandleR eh) override;
        virtual void _ProcessElementChange (ChangeTrackAction     action,  XAttributeHandleCR      xAttr) override;

        virtual void _OnUnload (DgnModelRefP    modelRef) override;

        virtual void _OnLoaded(ElementHandleR eh, IPointCloudChannelPtr channelPtr) override;
        virtual WString _GetExtension() override;
        virtual void _OnSave(ElementHandleR eh, IPointCloudChannelPtr channelPtr, bool after) override;

        virtual void _RemoveChannelHandler(EditElementHandleR curr) override;


    private:
        int CollectElmsCallback (ElementRefP elemRef, void *callbackArg, ScanCriteria    *scP);

        ClassificationChannelHandlerMap m_items;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class ClassificationChannelHandler :    public IPointCloudChannelQueryHandler,
                                        public IPointCloudChannelDisplayHandler,
                                        public PointCloudChannelHandler,
                                        public Bentley::RefCountedBase
    {
    ClassificationChannelHandler (ElementHandleCR eh, IPointCloudChannelPtr channelPtr=NULL);

    WString m_filePath;

    public:
        static ClassificationChannelHandlerPtr Create (ElementHandleCR eh);
        static ClassificationChannelHandlerPtr Create (ElementHandleCR eh, IPointCloudChannelPtr channelPtr);

        virtual ~ClassificationChannelHandler ();

        static ClassificationChannelHandler* GetChannelHandler (ElementHandleR elHandle);

        void Drop (ElementHandleR elHandle);

        bool HasPendingChange () {return m_hasPending;}
        void SetHasPendingChange (bool isChanged) {m_hasPending = isChanged;}

        void RemoveChannelHandler(ElementHandleR eh);

        IPointCloudChannelPtr GetChannel () {return m_pChannel;}

        WString GetFullPath() {return m_filePath;}


    private:
        ClassificationChannelHandler (ClassificationChannelHandler const& obj); //disabled
        ClassificationChannelHandler& operator=(ClassificationChannelHandler const&); //disabled

        void SwapChannelValues (IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers);

    protected:
        IPointCloudChannelPtr    m_pChannel;
        bool                    m_hasPending;

    public:
        //IPointCloudChannelDisplayHandler
        virtual bool _OnPreProcessClassification (ViewContextR viewContext, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) override;
        virtual void _OnDisplay(ViewContextR viewContext, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) override {return;}
        virtual bool _CanHandle (ViewContextR context, ElementHandleCR eh) override;

        //IPointCloudChannelQueryHandler
        virtual void _OnQuery (IPointCloudDataQueryCR query, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) override;
        virtual bool _CanHandle (IPointCloudDataQueryCR query, ElementHandleCR eh) override;
        virtual WString _GetDisplayName () const override;
        virtual WString _GetKey () const override {return CLASSIFCHANNEL_KEY;}


        //PointCloudChannelHandler
        virtual uint32_t _GetRequiredChannels() const override { return ((uint32_t)PointCloudChannelId::Classification | (uint32_t)PointCloudChannelId::Xyz); }
        virtual uint32_t _GetModifiedChannels() const override { return (uint32_t)PointCloudChannelId::Classification; }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
