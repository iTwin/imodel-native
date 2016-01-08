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

DGNPLATFORM_TYPEDEFS(ICodedObject);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface adopted by an object which possesses an AuthorityIssuedCode, such as a model
//! or element.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ICodedObject
{
    typedef AuthorityIssuedCode Code;
protected:
    virtual DgnDbR _GetDgnDb() const = 0; //!< Return the DgnDb in which this object resides
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR authority) const = 0; //!< Return whether this object supports codes issued by the specified authority.
    virtual Code _GenerateDefaultCode() const = 0; //!< Generate a code for this object on insertion, when no code has yet been assigned
    virtual Code const& _GetCode() const = 0; //!< Return this object's Code
    virtual DgnDbStatus _SetCode(Code const& code) = 0; //!< Set the code directly if permitted. Do not perform any validation of the code itself.
    virtual DgnElementCP _ToDgnElement() const { return nullptr; }
    virtual DgnModelCP _ToDgnModel() const { return nullptr; }
public:
    DgnDbR GetDgnDb() const { return _GetDgnDb(); }
    bool SupportsCodeAuthority(DgnAuthorityCR authority) const { return _SupportsCodeAuthority(authority); }
    Code GenerateDefaultCode() const { return _GenerateDefaultCode(); }
    Code const& GetCode() const { return _GetCode(); }
    DgnElementCP ToDgnElement() const { return _ToDgnElement(); }
    DgnModelCP ToDgnModel() const { return _ToDgnModel(); }

    DGNPLATFORM_EXPORT DgnDbStatus SetCode(Code const& newCode);
    DGNPLATFORM_EXPORT DgnDbStatus ValidateCode() const;
    DGNPLATFORM_EXPORT DgnAuthorityCPtr GetCodeAuthority() const;
};

//=======================================================================================
//! A DgnAuthority issues and validates Codes for coded objects like elements and models.
//! There are 2 general types of codes issued by authorities:
//!   - User/Application-supplied: The user/application supplies a Code and the authority
//!     simply enforces uniqueness and any constraints on e.g. allowable characters
//!   - Generated: The authority generates a Code for an object based on the object's
//!     properties, and/or some external logic. e.g., sequence numbers.
//! Some authorities may combine both approaches. e.g., sub-category names are supplied by
//! the user or application, but their Code namespaces are generated from the category
//! to which they belong.
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

    DGNPLATFORM_EXPORT virtual DgnDbStatus _ValidateCode(ICodedObjectCR codedObject) const;
    virtual DgnAuthority::Code _RegenerateCode(ICodedObjectCR codedObject) const { return codedObject.GetCode(); }

    static DgnAuthority::Code CreateCode(DgnAuthorityId authorityId, Utf8StringCR value, Utf8StringCR nameSpace) { return DgnAuthority::Code(authorityId, value, nameSpace); }

    DgnAuthority::Code CreateCode(Utf8StringCR value, Utf8StringCR nameSpace="") const { return DgnAuthority::Code(m_authorityId, value, nameSpace); }
public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    DgnAuthorityId GetAuthorityId() const { return m_authorityId; }
    DgnClassId GetClassId() const { return m_classId; }
    Utf8StringCR GetName() const { return m_name; }

    DGNPLATFORM_EXPORT AuthorityHandlerR GetAuthorityHandler() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DgnDbStatus ValidateCode(ICodedObjectCR obj) const { return _ValidateCode(obj); }
    DgnAuthority::Code RegenerateCode(ICodedObjectCR obj) const { return _RegenerateCode(obj); }
    DgnAuthority::Code CloneCodeForImport(DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const { return _CloneCodeForImport(srcElem, destModel, importer); }

    DGNPLATFORM_EXPORT static DgnAuthorityPtr Import(DgnDbStatus* status, DgnAuthorityCR sourceAuthority, DgnImportContext& importer);
};

//=======================================================================================
//! The built-in default code-issuing authority.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LocalAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)

    LocalAuthority(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
//! A generic DgnAuthority which behaves like a namespace for user-/application-defined
//! codes.
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

//=======================================================================================
//! The default code-issuing authority for DgnModels.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct ModelAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
protected:
    virtual DgnDbStatus _ValidateCode(ICodedObjectCR obj) const override;
public:
    ModelAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateModelCode(Utf8StringCR modelName);
};

//=======================================================================================
//! The default code-issuing authority for materials.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct MaterialAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    MaterialAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateMaterialCode(Utf8StringCR paletteName, Utf8StringCR materialName);
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
    virtual DgnDbStatus _ValidateCode(ICodedObjectCR obj) const override;
public:
    CategoryAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateCategoryCode(Utf8StringCR categoryName);
    DGNPLATFORM_EXPORT static Code CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR subCategoryName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetCategoryAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for named resources such as styles.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct ResourceAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
protected:
    virtual DgnDbStatus _ValidateCode(ICodedObjectCR obj) const override;
public:
    ResourceAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateResourceCode(Utf8StringCR resourceName, Utf8StringCR resourceECClassName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetResourceAuthorityId();
    static bool IsResourceAuthority(DgnAuthorityCR auth) { return auth.GetAuthorityId() == GetResourceAuthorityId(); }
};

//=======================================================================================
//! The default code-issuing authority for DgnTrueColors.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct TrueColorAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    TrueColorAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateTrueColorCode(Utf8StringCR colorName, Utf8StringCR colorBookName);
    DGNPLATFORM_EXPORT static DgnAuthorityId GetTrueColorAuthorityId();
};

//=======================================================================================
//! The default code-issuing authority for Component Definitions.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct ComponentAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
public:
    ComponentAuthority(CreateParams const& params) : T_Super(params) { }

    DGNPLATFORM_EXPORT static Code CreateVariationCode(Utf8StringCR solutionId, Utf8StringCR componentDefName);
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

    struct EXPORT_VTABLE_ATTRIBUTE Material : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_MaterialAuthority, MaterialAuthority, Material, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Component : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ComponentAuthority, ComponentAuthority, Component, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Model : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ModelAuthority, ModelAuthority, Model, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE TrueColor : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_TrueColorAuthority, TrueColorAuthority, TrueColor, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Resource : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ResourceAuthority, ResourceAuthority, Resource, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE Category : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_CategoryAuthority, CategoryAuthority, Category, Authority, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

