/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCore.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A two-part identifier, consisting of a \em major ID and a \em minor ID. The \em major ID
//! is a LINKAGEID issued by Bentley Select. The \em minor ID is chosen by the handler implementer.
//! @bsiclass
//=======================================================================================
struct  HandlerId
{
private:
    UInt32      m_id;
public:
//__PUBLISH_SECTION_END__
    explicit HandlerId (UInt32 id) {m_id=id;}

//__PUBLISH_SECTION_START__
    enum {INVALID_ID=0};    //! 0 is considered to be an invalid ID.

    //! Define a handler ID.
    //! @param[in] major major ID issued by Bentley Systems.
    //! @param[in] minor minor ID chosen by handler implementer.
    HandlerId (UInt16 major, UInt16 minor) {m_id = ((UInt32)major<<16)|minor;}
    //! Query the full 32-bit ID.
    UInt32 GetId () const {return m_id;}
    //! Query the major ID portion of the ID
    UInt16 GetMajorId () const {return (UInt16) (m_id >> 16);}
    //! Query the minor ID portion of the ID
    UInt16 GetMinorId () const {return (UInt16) (0xffff & m_id);}
    //! Test two HandlerIds for equality
    bool operator==(HandlerId const& other) const {return (m_id == other.m_id);}
    bool operator!=(HandlerId const& other) const {return (m_id != other.m_id);}
    bool operator< (HandlerId const& other) const {return m_id < other.m_id;}
    bool operator> (HandlerId const& other) const {return m_id > other.m_id;}
    //! Test if the HandlerId has a non-zero major ID portion
    bool IsValid () const {return GetMajorId() != INVALID_ID;}
};

//=======================================================================================
//! A two-part ID used by Element Handlers.
//! @see IElementHandlerManager
//! @bsiclass
//=======================================================================================
struct  ElementHandlerId : public HandlerId
{
//__PUBLISH_SECTION_END__
    explicit ElementHandlerId (UInt32 id) : HandlerId(id) {}

//__PUBLISH_SECTION_START__
    //! Zero-initialize an ElementHandlerId prior to reading its data from a persistent data source
    ElementHandlerId () : HandlerId (INVALID_ID, 0) {}

    //! Define an ElementHandlerId
    ElementHandlerId (UInt16 major, UInt16 minor) : HandlerId (major, minor) {}
};

//=======================================================================================
//! A two-part ID used by XAttribute Handlers.
//! @see XAttributeHandle
//! @bsiclass
//=======================================================================================
struct  XAttributeHandlerId : public HandlerId
{
//__PUBLISH_SECTION_END__
    explicit XAttributeHandlerId (UInt32 id) : HandlerId(id) {}

//__PUBLISH_SECTION_START__
    //! Zero-initialize an XAttributeHandlerId
    XAttributeHandlerId () : HandlerId (INVALID_ID, 0) {}

    //! Define an XAttributeHandlerId
    XAttributeHandlerId (UInt16 major, UInt16 minor) : HandlerId (major, minor) {}
};

//=======================================================================================
//! A two-part ID used by DgnModel Model Handlers.
//! @bsiclass
//=======================================================================================
struct  DgnModelHandlerId : public HandlerId
{
    //!  Constructor for DgnModelHandlerId
    DgnModelHandlerId (UInt16 major, UInt16 minor) : HandlerId (major, minor) {}
};

//=======================================================================================
//! How to create a DgnPlatformURI
//=======================================================================================
enum DgnResourceURICreationStrategy
    {
    DGN_RESOURCE_URI_CREATION_STRATEGY_ById          =1,     //!< Prefer the target resource's built-in identifier (ElementId or ECInstanceId)
    DGN_RESOURCE_URI_CREATION_STRATEGY_ByProvenanceId=2,     //!< For elements only, prefer the target element's original ID from the element provenance table.
    DGN_RESOURCE_URI_CREATION_STRATEGY_ByGlobalId    =4,     //!< Prefer the target resource's "global ID". For elements, this is the global ID of the primary ECInstance.
    DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKey =8,     //!< Prefer the target resource's "business key". For elements, this is the business key of the primary ECInstance.

    //! Use business key or global id (in that order of preference). Do not use the built-in identifier.
    DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKeyOrGlobalId
        = (DGN_RESOURCE_URI_CREATION_STRATEGY_ByGlobalId | DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKey),

    //! Use business key, global id, original ID, or identifier, in that order of preference.
    DGN_RESOURCE_URI_CREATION_STRATEGY_Any
        = (DGN_RESOURCE_URI_CREATION_STRATEGY_ById | DGN_RESOURCE_URI_CREATION_STRATEGY_ByProvenanceId |
            DGN_RESOURCE_URI_CREATION_STRATEGY_ByGlobalId | DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKey),

    //! Use business key, global id, or original ID, in that order of preference.
    DGN_RESOURCE_URI_CREATION_STRATEGY_AnyExceptId
        = (DGN_RESOURCE_URI_CREATION_STRATEGY_ByProvenanceId | DGN_RESOURCE_URI_CREATION_STRATEGY_ByGlobalId |
            DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKey)
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
