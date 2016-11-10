/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnAuthority.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_AuthorityHandler {struct Drawing; struct Link; struct Partition; struct Session; struct Sheet;};

//=======================================================================================
//! A DgnAuthority issues and validates DgnCodes for coded objects like elements and models.
//! There are 2 general types of codes issued by authorities:
//!   - User/Application-supplied: The user/application supplies a DgnCode and the authority
//!     simply enforces uniqueness and any constraints on e.g. allowable characters
//!   - Generated: The authority generates a DgnCode for an object based on the object's
//!     properties, and/or some external logic. e.g., sequence numbers.
//! Some authorities may combine both approaches. e.g., sub-category names are supplied by
//! the user or application, but their DgnCode namespaces are generated from the category
//! to which they belong.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnAuthority : RefCountedBase
{
public:
    struct CreateParams
    {
        DgnDbR          m_dgndb;
        DgnAuthorityId  m_id;
        DgnClassId      m_classId;
        Utf8String      m_name;

        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, DgnAuthorityId id=DgnAuthorityId()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name) {}
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

    DGNPLATFORM_EXPORT virtual DgnDbStatus _CloneCodeForImport(DgnCodeR clonedCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual DgnDbStatus _ValidateCode(DgnElementCR element) const;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _RegenerateCode(DgnCodeR regeneratedCode, DgnElementCR element) const;

    static DgnCode CreateCode(DgnAuthorityId authorityId, Utf8StringCR value, Utf8StringCR nameSpace) { return DgnCode(authorityId, value, nameSpace); }

    DgnCode CreateCode(Utf8StringCR value, Utf8StringCR nameSpace="") const { return DgnCode(m_authorityId, value, nameSpace); }
public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    DgnAuthorityId GetAuthorityId() const { return m_authorityId; }
    DgnClassId GetClassId() const { return m_classId; }
    Utf8StringCR GetName() const { return m_name; }

    DGNPLATFORM_EXPORT AuthorityHandlerR GetAuthorityHandler() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DGNPLATFORM_EXPORT DgnDbStatus ValidateCode(DgnElementCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus RegenerateCode(DgnCodeR newCode, DgnElementCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus CloneCodeForImport(DgnCodeR newCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT static DgnAuthorityPtr Import(DgnDbStatus* status, DgnAuthorityCR sourceAuthority, DgnImportContext& importer);

    //! Identifies actions which may be restricted for authorities created by a handler for a missing subclass of DgnAuthority
    struct RestrictedAction : DgnDomain::Handler::RestrictedAction
    {
        DEFINE_T_SUPER(DgnDomain::Handler::RestrictedAction);

        static const uint64_t ValidateCode = T_Super::NextAvailable; //!< Validate code issued by this authority
        static const uint64_t RegenerateCode = ValidateCode << 1; //!< Regenerate a code issued by this authority
        static const uint64_t CloneCode = RegenerateCode << 1; //!< Clone a code issued by this authority

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name); //!< Parse action name from ClassHasHandler custom attribute
    };
};

//=======================================================================================
//! The built-in default code-issuing authority.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LocalAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)

    LocalAuthority(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A generic DgnAuthority which behaves like a namespace for user-/application-defined
//! codes.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NamespaceAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)

    NamespaceAuthority(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT DgnCode CreateCode(Utf8StringCR value, Utf8StringCR nameSpace = "") const { return T_Super::CreateCode(value, nameSpace); }

    DGNPLATFORM_EXPORT static RefCountedPtr<NamespaceAuthority> CreateNamespaceAuthority(Utf8CP name, DgnDbR dgndb);
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace="");
};

//=======================================================================================
//! The default code-issuing authority for InformationPartitionElements.
// @bsistruct                                                    Shaun.Sewall    10/16
//=======================================================================================
struct PartitionAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::Partition;

protected:
    DgnDbStatus _ValidateCode(DgnElementCR) const override;
    PartitionAuthority(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DgnCode CreatePartitionCode(Utf8StringCR codeValue, DgnElementId scopeId);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetPartitionAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for LinkElements.
// @bsistruct                                                    Shaun.Sewall    11/16
//=======================================================================================
struct LinkAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::Link;

protected:
    LinkAuthority(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DgnCode CreateLinkCode(Utf8StringCR codeValue, DgnElementId scopeId);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetLinkAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for Drawing elements.
// @bsistruct                                                    Shaun.Sewall    11/16
//=======================================================================================
struct DrawingAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::Drawing;

protected:
    DgnDbStatus _ValidateCode(DgnElementCR) const override;
    DrawingAuthority(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DgnCode CreateDrawingCode(Utf8StringCR codeValue, DgnElementId scopeId);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetDrawingAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for Sheet elements.
// @bsistruct                                                    Shaun.Sewall    11/16
//=======================================================================================
struct SheetAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::Sheet;

protected:
    DgnDbStatus _ValidateCode(DgnElementCR) const override;
    SheetAuthority(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DgnCode CreateSheetCode(Utf8StringCR codeValue, DgnElementId scopeId);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetSheetAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for materials.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct MaterialAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    MaterialAuthority(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT static DgnCode CreateMaterialCode(Utf8StringCR paletteName, Utf8StringCR materialName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetMaterialAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for categories and sub-categories.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct CategoryAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
protected:
    virtual DgnDbStatus _ValidateCode(DgnElementCR) const override;
public:
    CategoryAuthority(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT static DgnCode CreateCategoryCode(Utf8StringCR categoryName, Utf8StringCR nameSpace="");
    DGNPLATFORM_EXPORT static DgnCode CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR subCategoryName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetCategoryAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for named resources such as styles.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct ResourceAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    ResourceAuthority(CreateParams const& params) : T_Super(params) {}
    DGNPLATFORM_EXPORT static DgnCode CreateResourceCode(Utf8StringCR resourceName, Utf8StringCR namespaceName);
    static DgnCode CreateDisplayStyleCode(Utf8StringCR name) {return CreateResourceCode(name,"dstyle");}
    static DgnCode CreateModelSelectorCode(Utf8StringCR name) {return CreateResourceCode(name,"models");}
    static DgnCode CreateCategorySelectorCode(Utf8StringCR name) {return CreateResourceCode(name,"catgs");}
    DGNPLATFORM_EXPORT static DgnCode CreateViewDefinitionCode(Utf8StringCR name);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetResourceAuthorityId();
    static bool IsResourceAuthority(DgnAuthorityCR auth) {return auth.GetAuthorityId() == GetResourceAuthorityId();}
};

//=======================================================================================
//! The default code-issuing authority for DgnTrueColors.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct TrueColorAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    TrueColorAuthority(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT static DgnCode CreateTrueColorCode(Utf8StringCR colorName, Utf8StringCR colorBookName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetTrueColorAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for DgnGeometryParts.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct GeometryPartAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    GeometryPartAuthority(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT static DgnCode CreateGeometryPartCode(Utf8StringCR name, Utf8StringCR nameSpace);
    static DgnCode CreateEmptyCode() { return CreateGeometryPartCode("", ""); }
    DGNPLATFORM_EXPORT static DgnAuthorityId GetGeometryPartAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for Session elements.
// @bsistruct                                                    Shaun.Sewall    10/16
//=======================================================================================
struct SessionAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::Session;

protected:
    SessionAuthority(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DgnCode CreateSessionCode(Utf8StringCR codeValue);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetSessionAuthorityId();
};

//=======================================================================================
// Macro used for declaring the members of a DgnAuthority handler
//=======================================================================================
#define AUTHORITYHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
    private: virtual Dgn::DgnAuthorityP _CreateInstance(Dgn::DgnAuthority::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//=======================================================================================
//! DgnAuthority handlers
//=======================================================================================
namespace dgn_AuthorityHandler
{
    struct EXPORT_VTABLE_ATTRIBUTE Authority : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (BIS_CLASS_Authority, Authority, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual AuthorityHandlerP _ToAuthorityHandler() override { return this; }
        virtual DgnAuthorityP _CreateInstance(DgnAuthority::CreateParams const& params) { return new DgnAuthority(params); }
    public:
        virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnAuthority::RestrictedAction::Parse(name); }
        DGNPLATFORM_EXPORT static AuthorityHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);

        DgnAuthorityPtr Create(DgnAuthority::CreateParams const& params) { return _CreateInstance(params); }
    };

    struct EXPORT_VTABLE_ATTRIBUTE Local : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_LocalAuthority, LocalAuthority, Local, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Namespace : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_NamespaceAuthority, NamespaceAuthority, Namespace, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Material : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_MaterialAuthority, MaterialAuthority, Material, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE GeometryPart : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_GeometryPartAuthority, GeometryPartAuthority, GeometryPart, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Link : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_LinkAuthority, LinkAuthority, Link, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Partition : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_PartitionAuthority, PartitionAuthority, Partition, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE TrueColor : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_TrueColorAuthority, TrueColorAuthority, TrueColor, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Resource : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_ResourceAuthority, ResourceAuthority, Resource, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Category : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_CategoryAuthority, CategoryAuthority, Category, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Session : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_SessionAuthority, SessionAuthority, Session, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Drawing : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_DrawingAuthority, DrawingAuthority, Drawing, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Sheet : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_SheetAuthority, SheetAuthority, Sheet, Authority, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

