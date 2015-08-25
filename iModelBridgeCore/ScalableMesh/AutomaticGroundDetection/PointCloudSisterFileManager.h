/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudSisterFileManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once


USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiinterface                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct __declspec(novtable) IChannelFileListener
    {
    public:
        virtual void _OnLoaded(ElementHandleR eh, IPointCloudChannelPtr channelPtr);
        virtual WString _GetExtension() = 0;
        virtual void _OnSave(ElementHandleR eh, IPointCloudChannelPtr channelPtr, bool after);

        virtual void _ProcessElementHandle (ElementHandleR eh);
        virtual void _ProcessElementChange (ChangeTrackAction     action,  XAttributeHandleCR      xAttr);

        virtual void _RemoveChannelHandler(EditElementHandleR curr) = 0;

        virtual void _OnUnload (DgnModelRefP    modelRef);

    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
enum CompressType
{
    PRE_COMPRESS_DGNFILE    = 1,
    POST_COMPRESS_DGNFILE   = 2
};


class SisterFileManager
    {
    public:
        typedef std::map<WString, IChannelFileListener*> SisterFileMap;

        static SisterFileManager s_sinstance;

        SisterFileManager(){}
        ~SisterFileManager();

        static SisterFileManager& GetInstance();
        void Register(IChannelFileListener* channelFileListener);
        void UnRegister(IChannelFileListener* channelFileListener);

        void OpenChannelFile(ElementHandleR eh, WStringCR fullpath, WStringCR channelName);
        WString GetChannelFileName(ElementHandleCR eh, WStringCR extensionString, WStringP fullPath=NULL);
        WString GetChannelFullPath(EditElementHandleR eh);
        WString GetChannelExtension(EditElementHandleR eh);
        void SaveChannelToFile(ElementHandleR eh, IPointCloudChannelPtr channelPtr, WString extension);

        void ProcessModel (DgnModelRefP    modelRef, bool children=true);
        void ProcessElementHandle (ElementHandleR eh);
        void ProcessElementChange (ChangeTrackAction     action,  XAttributeHandleCR      xAttr);

        void OnUnload (DgnModelRefP    modelRef);
        void OnCompressDgnFile  (CompressType type);

    private:
        SisterFileManager(const SisterFileManager&);    // intentionally undefined
        SisterFileManager & operator=(const SisterFileManager &);   // intentionally undefined


        SisterFileMap m_mapExtensionToChannelFileListner;
    };

class PointCloudSisterFileMdlEvents
    {
    public:
        static void OnChangeTrackXAttributeUndoRedo
            (
            XAttributeHandleCP      xAttr,
            ChangeTrackAction     action,             // the action that happened
            bool                 isUndo,             // if TRUE -> this is an undo, if FALSE -> this is a redo
            ChangeTrackInfo const*  info,               // info about command that cause original change. CAN BE NULL!!!
            ChangeTrackSource     source              // the source of the change (undo, restore, merge)
            );

        static void OnChangeTrackXAttributeChanged
            (
            XAttributeHandleCP      xAttr,
            ChangeTrackInfo*        info,
            bool*                cantBeUndoneFlag
            );

        static void OnChangeTrackUndoRedo
            (
            MSElementDescrP         afterUndoRedo ,
            MSElementDescrP         beforeUndoRedo ,
            ChangeTrackAction     action ,
            bool                 isUndo ,
            ChangeTrackInfo const*  info ,
            ChangeTrackSource     source
            );


        static void OnModelChanged(DgnModelRefP modelRef, int type);
        static void OnCompressDgnFile(CompressType type);
        static void OnReferenceAttached (DgnModelRefP modelRef, DgnAttachmentAttachedReason cause);

        static void OnUnload ();

    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
