/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnAuthority.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct SystemAuthority;

//=======================================================================================
//! A DgnAuthority serves issues DgnAuthority::Codes when objects are created and cloned.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnAuthority : RefCountedBase
{
public:
    typedef AuthorityIssuedCode Code;

    struct CreateParams
    {
        DgnDbR          m_dgndb;
        DgnAuthorityId  m_id;
        DgnClassId      m_classId;
        Utf8String      m_name;

        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, DgnAuthorityId id=DgnAuthorityId()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name) { }
    };
protected:
    friend struct DgnAuthorities;
    friend struct dgn_AuthorityHandler::Authority;

    DgnDbR          m_dgndb;
    DgnAuthorityId  m_authorityId;
    DgnClassId      m_classId;
    Utf8String      m_name;
    
    DGNPLATFORM_EXPORT explicit DgnAuthority(CreateParams const&);

    void ReadProperties(Utf8StringCR jsonStr);
    Utf8String SerializeProperties() const;

    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(JsonValueR) const;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(JsonValueCR);
    DGNPLATFORM_EXPORT virtual DgnAuthorityPtr _CloneForImport(DgnDbStatus* status, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual DgnAuthority::Code _CloneCodeForImport(DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    static DgnAuthority::Code CreateCode(DgnAuthorityId authorityId, Utf8StringCR value, Utf8StringCR nameSpace) { return DgnAuthority::Code(authorityId, value, nameSpace); }
    DgnAuthority::Code CreateCode(Utf8StringCR value, Utf8StringCR nameSpace) const { return DgnAuthority::Code(m_authorityId, value, nameSpace); }

public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    DgnAuthorityId GetAuthorityId() const { return m_authorityId; }
    DgnClassId GetClassId() const { return m_classId; }
    Utf8StringCR GetName() const { return m_name; }

    DGNPLATFORM_EXPORT AuthorityHandlerR GetAuthorityHandler() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DgnAuthority::Code CloneCodeForImport(DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const { return _CloneCodeForImport(srcElem, destModel, importer); }

    DGNPLATFORM_EXPORT static DgnAuthorityPtr Import(DgnDbStatus* status, DgnAuthorityCR sourceAuthority, DgnImportContext& importer);

    DGNPLATFORM_EXPORT static DgnAuthority::Code CreateDefaultCode();
};

//=======================================================================================
//! The built-in default code-issuing authority. Codes are based on element ID + class ID.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LocalAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)

    LocalAuthority(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
//! A generic DgnAuthority which behaves like a namespace for user-/application-defined
//! element codes.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NamespaceAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)

    NamespaceAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT DgnAuthority::Code CreateCode(Utf8StringCR value, Utf8StringCR nameSpace = "") const { return T_Super::CreateCode(value, nameSpace); }

    DGNPLATFORM_EXPORT static RefCountedPtr<NamespaceAuthority> CreateNamespaceAuthority(Utf8CP name, DgnDbR dgndb);
    DGNPLATFORM_EXPORT static DgnAuthority::Code CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace="");
};

#define AUTHORITYHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
    private: virtual Dgn::DgnAuthorityP _CreateInstance(Dgn::DgnAuthority::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

namespace dgn_AuthorityHandler
{
    struct EXPORT_VTABLE_ATTRIBUTE Authority : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_Authority, Authority, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual AuthorityHandlerP _ToAuthorityHandler() override { return this; }
        virtual DgnAuthorityP _CreateInstance(DgnAuthority::CreateParams const& params) { return new DgnAuthority(params); }
    public:
        DGNPLATFORM_EXPORT static AuthorityHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);

        DgnAuthorityPtr Create(DgnAuthority::CreateParams const& params) { return _CreateInstance(params); }
    };

    struct EXPORT_VTABLE_ATTRIBUTE Local : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_LocalAuthority, LocalAuthority, Local, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Namespace : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_NamespaceAuthority, NamespaceAuthority, Namespace, Authority, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

