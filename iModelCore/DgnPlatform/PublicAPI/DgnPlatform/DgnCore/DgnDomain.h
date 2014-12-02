/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnDomain.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#include "DgnCore.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

//=======================================================================================
//! Interface adopted by a "sub-type" handler. A sub-type handler overrides a type handler
//!   for particular elements by recognizing a sub-type from level, user data linkages, etc.
//! Note: a sub-type handler MUST inherit from the Handler for the base type that it specializes.
// @bsiclass
//=======================================================================================
struct ISubTypeHandlerQuery
{
//! @remarks NOTE TO IMPLEMENTERS: This method is called when converting from V8 on import. Therefore, the element data will be in V8 format!
virtual bool _ClaimElement (ElementHandleCR el) = 0;
};

enum SubTypeHandlerPriority
{
    SUBTYPEHANDLER_PRIORITY_Highest     = 1000,
    SUBTYPEHANDLER_PRIORITY_High        = 750,
    SUBTYPEHANDLER_PRIORITY_Normal      = 500,
    SUBTYPEHANDLER_PRIORITY_Low         = 250,
    SUBTYPEHANDLER_PRIORITY_Lowest      = 0,
};

enum
    {
    LEGACY_ELEMENT_TypeHandlerMajorId        = 0x58ef,
    LEGACY_ELEMENT_SubTypeHandlerMajorId     = 0x58f0,
    };
//__PUBLISH_SECTION_START__

typedef bvector<DgnDomainCP> DomainList;

//=======================================================================================
// A list of DgnDomains that is sorted by priority.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnDomains : NonCopyableClass
    {
//__PUBLISH_SECTION_END__
private:
    friend struct DgnProject;
    friend struct DgnDomain;
    DomainList  m_domains;
    DgnProjectP m_project;

    BeSQLite::DbResult AddElementHandler (Utf8CP domain, HandlerR handler);
    BeSQLite::DbResult AddDomainToProject (DgnDomainCR domain);

public:
    DgnDomains(DgnProjectP project=NULL) : m_project(project){}

    HandlerR ResolveHandler (ElementHandlerId) const;
    DGNPLATFORM_EXPORT void AddLoadedDomain (DgnDomainCR domain);
    DGNPLATFORM_EXPORT void DropDomain (DgnDomainCR domain);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT DgnDomainCP FindDomain (Utf8CP name) const;
    DGNPLATFORM_EXPORT HandlerP FindElementHandler (ElementHandlerId id) const;
    DGNPLATFORM_EXPORT XAttributeHandlerP FindXAttributeHandler (XAttributeHandlerId) const;
    DGNPLATFORM_EXPORT DomainList const& GetDomainList() const;
    };

//=======================================================================================
// Handles locating, loading and tracking the list of "loaded" domains for opening DgnProjects.
// @bsiclass                                                    Keith.Bentley   09/11
//=======================================================================================
struct DgnDomainLoader
    {
    //! Applications can call this method to register a domain as available for use by DgnProjects.
    DGNPLATFORM_EXPORT static void RegisterLoadedDomain (DgnDomainR domain);
    DGNPLATFORM_EXPORT static void UnregisterLoadedDomain (DgnDomainR domain);

    DGNPLATFORM_EXPORT static DgnDomainCP LoadDomain (Utf8CP name, UInt32 version=0); // 0 means "latest"
    DGNPLATFORM_EXPORT static DgnDomainCR GetSystemDomain();
    DGNPLATFORM_EXPORT static DgnDomains const& GetLoadedDomains();
    DGNPLATFORM_EXPORT static HandlerR GetIllegalHandler();

    static DgnDomainCP FindDomain (Utf8CP name) {return GetLoadedDomains().FindDomain(name);}
    };

//=======================================================================================
// A DgnDomain is a set of related classes, usually implemented in a single .dll, that handles persistence, maintenance, interpretation, and display of
// domain-specific information in DgnProjects. A single DgnProject can hold information from many DgnDomains. In this context, a "domain" can
// supply the implementation a set of related concepts that may map to terms like: "discipline", "application", "schema", etc.
// Among the types of classes supplied by a "DgnDomain" are:
//  - Element Handlers
//  - Table Handlers
//  - XAttribute Handlers
// A DgnDomain has a priority. A domain's priority determines its position in a DomainList. When a DgnProject
// works with multiple domains, it queries the domains in a DomainList in order. so, the highest-priority domain
// gets the first chance to supply a handler for an element, for example. This allows an higher priority app or importer domain
// to supply a handler for an element that is considered missing by lower priority system domain.
// The ranges for domain priorities are:
// 0-99     platform domains (system, drafting)
// 100-199  vertical application domains
// 200-299  layered product domains
// 300-399  importer domains
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnDomain : NonCopyableClass
    {
//__PUBLISH_SECTION_END__
    typedef bmap<ElementHandlerId, HandlerP> Handlers;
    typedef bmap<XAttributeHandlerId,XAttributeHandlerP> XAttributeHandlers;
    friend struct DgnDomains;
protected:
    UInt32      m_version;
    Utf8String  m_name;
    Utf8String  m_description;
    Handlers    m_handlers;
    XAttributeHandlers m_xAttHandlers;
    virtual ~DgnDomain() {}

public:
    DgnDomain (Utf8CP name, Utf8CP descr, UInt32 version) : m_name(name), m_description(descr) {m_version=version;}
    DGNPLATFORM_EXPORT virtual UInt32 _GetPriority() const = 0;
    DGNPLATFORM_EXPORT virtual void _OnProjectCreated (DgnProjectR project) const;

    DGNPLATFORM_EXPORT Handlers const& GetHandlers() const;
    DGNPLATFORM_EXPORT XAttributeHandlers const& GetXAttributeHandlers() const;

    DGNPLATFORM_EXPORT BentleyStatus RegisterHandler (ElementHandlerId, HandlerR);
    DGNPLATFORM_EXPORT BentleyStatus RegisterXAttributeHandler (XAttributeHandlerId, XAttributeHandlerR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT Utf8CP GetName() const;
    DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
    DGNPLATFORM_EXPORT UInt32 GetVersion() const;
    DGNPLATFORM_EXPORT HandlerP FindElementHandler (ElementHandlerId) const;
    DGNPLATFORM_EXPORT XAttributeHandlerP FindXAttributeHandler (XAttributeHandlerId) const;
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnDraftingDomain : DgnDomain
    {
//__PUBLISH_SECTION_END__
private:
    virtual ~DgnDraftingDomain(){}
    virtual UInt32 _GetPriority() const {return 49;}

public:
    DgnDraftingDomain();
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT static DgnDraftingDomain& GetInstance();
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnSystemDomain : DgnDomain
    {
//__PUBLISH_SECTION_END__
private:
    virtual ~DgnSystemDomain() {}

public:
    virtual HandlerP _GenerateMissingHandler (ElementHandlerId);
    virtual UInt32 _GetPriority() const {return 50;}

    DgnSystemDomain();
    DGNPLATFORM_EXPORT void InitDgnCore(DgnDraftingDomain&);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT static DgnSystemDomain& GetInstance();
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
