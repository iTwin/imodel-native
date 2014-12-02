/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PersistentElementPathXAttributeHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/XAttributeHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* A generic handler for an XAttribute that holds a single pointer to another element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class PersistentElementPathXAttributeHandler : public XAttributeHandler
{
private:
    //  Singleton
    PersistentElementPathXAttributeHandler () {;}

//    virtual StatusInt   _OnPreprocessCopy         (IReplaceXAttribute*, XAttributeHandleCR, ElementHandleCR, CopyContextP) override; removed in graphite
public:

    //! Called by DgnPlatform host ONCE!
    static     void        StaticInitialize ();

    //! Query the XAttributeHandlerId that identifies this generic handler.
    DGNPLATFORM_EXPORT static     XAttributeHandlerId GetHandlerId ();

    //! Add an XAttribute containining a PersistentElementPath to the specified host element
    //! @param hostElement IN    The host element to which XAttribute is added or scheduled
    //! @param path IN           The path to be added as an XAttribute to the host element.
    //! @param xAttrId     INOUT ID for the new XAttribute. Pass XAttributeHandle::INVALID_XATTR_ID if you want
    //!                          a unique ID to be generated for you.
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks Calls TXNMGR.AddXAttribute
    DGNPLATFORM_EXPORT static     StatusInt   AddPersistentElementPath (UInt32& xAttrId, ElementRefP hostElement, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Update the PersistentElementPath in the specified XAttribute data
    //! @param xa   IN    The XAttribute to be updated
    //! @param path IN    The new path to be stored in the XAttribute's data
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks Calls TXNMGR.ReplaceXAttributeData
    DGNPLATFORM_EXPORT static     StatusInt   SetPersistentElementPath (XAttributeHandleR xa, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Schedule an add or replace of an XAttribute containining a PersistentElementPath on the specified host element
    //! @param hostElement IN    The host element to which XAttribute is added or scheduled
    //! @param path IN           The path to be added as an XAttribute to the host element.
    //! @param xAttrId     IN    ID for the new XAttribute. Pass XAttributeHandle::INVALID_XATTR_ID if you want a unique ID to be generated for you.
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks The XAttribute is written to the model when the caller writes or adds hostElement to the model.
    DGNPLATFORM_EXPORT static     StatusInt   ScheduleWritePersistentElementPath (EditElementHandleR hostElement, UInt32 xAttrId, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Load PersistentElementPath from XAttribute data
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentElementPath (PersistentElementPath&, XAttributeHandleCR);

    //! Load PersistentElementPath from XAttribute data
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentElementPath (PersistentElementPath&, UInt32 xaDataSize, void const* xaData);

    //! Load PersistentElementPath by looking up a PersistentElementPathXAttributeHandler XAttribute identified by the specified index
    //! @param path        OUT  The path found.
    //! @param hostElement IN   The host element that holds the XAttribute to look up
    //! @param xAttrId     IN   The ID of the PersistentElementPathXAttributeHandler XAttribute to find.
    //! @return non-zero if the host element does not hold a PersistentElementPathXAttributeHandler XAttribute with the specified ID or if the XAttribute could not be loaded.
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentElementPath (PersistentElementPath& path, ElementHandleCR hostElement, UInt32 xAttrId);
};

/*=================================================================================**//**
* A generic handler for an XAttribute that holds a single snap to another element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class PersistentSnapPathXAttributeHandler : public XAttributeHandler
{
private:
    //  Singleton
    PersistentSnapPathXAttributeHandler () {;}

//    virtual StatusInt   _OnPreprocessCopy         (IReplaceXAttribute*, XAttributeHandleCR, ElementHandleCR, CopyContextP) override; removed in graphite

public:

    //! Called by DgnPlatform host ONCE!
    static     void        StaticInitialize ();

    //! Query the XAttributeHandlerId that identifies this generic handler.
    DGNPLATFORM_EXPORT static    XAttributeHandlerId GetHandlerId ();

    //! Add an XAttribute containining a PersistentSnapPath to the specified host element
    //! @param hostElement IN    The host element to which XAttribute is added
    //! @param path IN           The path to be added as an XAttribute to the host element.
    //! @param xAttrId     INOUT ID for the new XAttribute. Pass XAttributeHandle::INVALID_XATTR_ID if you want
    //!                          a unique ID to be generated for you.
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks Calls TXNMGR.AddXAttribute
    DGNPLATFORM_EXPORT static    StatusInt   AddPersistentSnapPath (UInt32& xAttrId, ElementRefP hostElement, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Update the PersistentSnapPath in the specified XAttribute data
    //! @param xa   IN    The XAttribute to be updated
    //! @param path IN    The new path to be stored in the XAttribute's data
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks Calls TXNMGR.ReplaceXAttributeData
    DGNPLATFORM_EXPORT static     StatusInt   SetPersistentSnapPath (XAttributeHandleR xa, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Schedule an add or replace of an XAttribute containining a PersistentSnapPath on the specified host element
    //! @param hostElement IN    The host element handle on which XAttribute write is scheduled
    //! @param path IN           The path to be added as an XAttribute to the host element.
    //! @param xAttrId     IN    ID for the new XAttribute. Pass XAttributeHandle::INVALID_XATTR_ID if you want a unique ID to be generated for you.
    //! @param ccOpt       IN    how to handle the target when the host is copied
    //! @remarks The XAttribute is written to the model when the caller writes or adds hostElement to the model.
    DGNPLATFORM_EXPORT static    StatusInt   ScheduleWritePersistentSnapPath (EditElementHandleR hostElement, UInt32 xAttrId, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles);

    //! Load PersistentSnapPath from XAttribute data
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentSnapPath (PersistentSnapPath&, XAttributeHandleCR);

    //! Load PersistentSnapPath from XAttribute data
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentSnapPath (PersistentSnapPath&, UInt32 xaDataSize, void const* xaData);

    //! Load PersistentSnapPath by looking up a PersistentSnapPathXAttributeHandler XAttribute identified by the specified index
    //! @param path        OUT  The path found.
    //! @param hostElement IN   The host Snap that holds the XAttribute to look up
    //! @param xAttrId     IN   The ID of the PersistentSnapPathXAttributeHandler XAttribute to find.
    //! @return non-zero if the host element does not hold a PersistentSnapPathXAttributeHandler XAttribute with the specified ID or if the XAttribute could not be loaded.
    DGNPLATFORM_EXPORT static     StatusInt   GetPersistentSnapPath (PersistentSnapPath& path, ElementHandleCR hostElement, UInt32 xAttrId);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

