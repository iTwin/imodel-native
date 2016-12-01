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

//-----------------------------------------------------------------------------------------
// Names of built-in Authorities. The best practice is to include the domain name as part of the authority name to ensure global uniqueness
//-----------------------------------------------------------------------------------------
#define BIS_AUTHORITY(name)                         "bis:" name
#define BIS_AUTHORITY_NullAuthority                 BIS_AUTHORITY(BIS_CLASS_NullAuthority)
#define BIS_AUTHORITY_AnnotationFrameStyle          BIS_AUTHORITY(BIS_CLASS_AnnotationFrameStyle)
#define BIS_AUTHORITY_AnnotationLeaderStyle         BIS_AUTHORITY(BIS_CLASS_AnnotationLeaderStyle)
#define BIS_AUTHORITY_AnnotationTextStyle           BIS_AUTHORITY(BIS_CLASS_AnnotationTextStyle)
#define BIS_AUTHORITY_CategorySelector              BIS_AUTHORITY(BIS_CLASS_CategorySelector)
#define BIS_AUTHORITY_DisplayStyle                  BIS_AUTHORITY(BIS_CLASS_DisplayStyle)
#define BIS_AUTHORITY_Drawing                       BIS_AUTHORITY(BIS_CLASS_Drawing)
#define BIS_AUTHORITY_DrawingCategory               BIS_AUTHORITY(BIS_CLASS_DrawingCategory)
#define BIS_AUTHORITY_GeometryPart                  BIS_AUTHORITY(BIS_CLASS_GeometryPart)
#define BIS_AUTHORITY_LightDefinition               BIS_AUTHORITY(BIS_CLASS_LightDefinition)
#define BIS_AUTHORITY_LineStyle                     BIS_AUTHORITY(BIS_CLASS_LineStyle)
#define BIS_AUTHORITY_LinkElement                   BIS_AUTHORITY(BIS_CLASS_LinkElement)
#define BIS_AUTHORITY_MaterialElement               BIS_AUTHORITY(BIS_CLASS_MaterialElement)
#define BIS_AUTHORITY_ModelSelector                 BIS_AUTHORITY(BIS_CLASS_ModelSelector)
#define BIS_AUTHORITY_InformationPartitionElement   BIS_AUTHORITY(BIS_CLASS_InformationPartitionElement)
#define BIS_AUTHORITY_Session                       BIS_AUTHORITY(BIS_CLASS_Session)
#define BIS_AUTHORITY_Sheet                         BIS_AUTHORITY(BIS_CLASS_Sheet)
#define BIS_AUTHORITY_SpatialCategory               BIS_AUTHORITY(BIS_CLASS_SpatialCategory)
#define BIS_AUTHORITY_SubCategory                   BIS_AUTHORITY(BIS_CLASS_SubCategory)
#define BIS_AUTHORITY_Subject                       BIS_AUTHORITY(BIS_CLASS_Subject)
#define BIS_AUTHORITY_TextAnnotationSeed            BIS_AUTHORITY(BIS_CLASS_TextAnnotationSeed)
#define BIS_AUTHORITY_Texture                       BIS_AUTHORITY(BIS_CLASS_Texture)
#define BIS_AUTHORITY_TrueColor                     BIS_AUTHORITY(BIS_CLASS_TrueColor)
#define BIS_AUTHORITY_ViewDefinition                BIS_AUTHORITY(BIS_CLASS_ViewDefinition)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_AuthorityHandler {struct DatabaseScope; struct ElementScope; struct ModelScope; struct Null;};

//=======================================================================================
//! A DgnAuthority issues and validates DgnCodes for elements.
//! There are 2 general types of codes issued by authorities:
//!   - User/Application-supplied: The user/application supplies a DgnCode and the authority
//!     simply enforces uniqueness and any constraints on e.g. allowable characters
//!   - Generated: The authority generates a DgnCode for an object based on the object's
//!     properties, and/or some external logic. e.g., sequence numbers.
//! Some authorities may combine both approaches. e.g., sub-category names are supplied by
//! the user or application, but their DgnCode namespaces are generated from the category
//! to which they belong.
//! @note Authorities are DgnDb-specific. That is, authority names are constant, but their DgnAuthorityIds may vary per DgnDb.
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
//! The built-in authority that issues null codes.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NullAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)
    friend struct dgn_AuthorityHandler::Null;
    friend struct DgnDb;

private:
    static DgnDbStatus Insert(DgnDbR db);

protected:
    NullAuthority(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a null code
    static DgnCode CreateCode() {return DgnCode(GetNullAuthorityId(), "", "");}

        //! Return the DgnAuthorityId of the NullAuthority
    static DgnAuthorityId GetNullAuthorityId() {return DgnAuthorityId((uint64_t)1LL);}
    //! Return true if the specified DgnAuthority is the NullAuthority
    static bool IsNullAuthority(DgnAuthorityCR authority) {return authority.GetAuthorityId() == GetNullAuthorityId();}
};

//=======================================================================================
//! A generic DgnAuthority which behaves like a namespace for user-/application-defined codes.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DatabaseScopeAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority)
    friend struct dgn_AuthorityHandler::DatabaseScope;

protected:
    DatabaseScopeAuthority(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new DatabaseScopeAuthority using the specified authorityName
    //! @note The best practice is to incorporate your domain name into the authorityName to make sure that it is unique
    DGNPLATFORM_EXPORT static DatabaseScopeAuthorityPtr Create(Utf8CP name, DgnDbR dgndb);

    //! Create a DgnCode using the DatabaseScopeAuthority of the specified name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, DgnDbCR db, Utf8StringCR value, Utf8StringCR nameSpace="");
    //! Create a DgnCode using this DatabaseScopeAuthority
    DgnCode CreateCode(Utf8StringCR value, Utf8StringCR nameSpace = "") const {return DgnCode(GetAuthorityId(), value, nameSpace);}
};

//=======================================================================================
//! A generic DgnAuthority which enforces uniquess within the scope of a DgnModel.
// @bsistruct                                                    Shaun.Sewall    11/16
//=======================================================================================
struct ModelScopeAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::ModelScope;

protected:
    DgnDbStatus _ValidateCode(DgnElementCR) const override;
    ModelScopeAuthority(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new ModelScopeAuthority using the specified authorityName
    //! @note The best practice is to incorporate your domain name into the authorityName to make sure that it is unique
    DGNPLATFORM_EXPORT static ModelScopeAuthorityPtr Create(Utf8CP authorityName, DgnDbR db);

    //! Create a DgnCode using the ModelScopeAuthority of the specified name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, DgnModelCR model, Utf8StringCR value);
    //! Create a DgnCode using the ModelScopeAuthority of the specified name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, DgnDbCR db, DgnModelId modelId, Utf8StringCR value);
    //! Create a DgnCode using this ModelScopeAuthority
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnModelCR model, Utf8StringCR value) const;
    //! Create a DgnCode using this ModelScopeAuthority
    DgnCode CreateCode(DgnModelId modelId, Utf8StringCR value) const {return DgnCode(GetAuthorityId(), value, modelId);}
};

//=======================================================================================
//! A generic DgnAuthority which enforces uniquess within another DgnElement that is providing the scope.
// @bsistruct                                                    Shaun.Sewall    11/16
//=======================================================================================
struct ElementScopeAuthority : DgnAuthority
{
    DEFINE_T_SUPER(DgnAuthority);
    friend struct dgn_AuthorityHandler::ElementScope;

protected:
    DgnDbStatus _ValidateCode(DgnElementCR) const override;
    ElementScopeAuthority(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new ElementScopeAuthority using the specified authorityName
    //! @note The best practice is to incorporate your domain name into the authorityName to make sure that it is unique
    DGNPLATFORM_EXPORT static ElementScopeAuthorityPtr Create(Utf8CP authorityName, DgnDbR db);

    //! Create a DgnCode using the ElementScopeAuthority of the specified name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, DgnElementCR scopeElement, Utf8StringCR value);
    //! Create a DgnCode using the ElementScopeAuthority of the specified name
    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP authorityName, DgnDbR db, DgnElementId scopeElementId, Utf8StringCR value);
    //! Create a DgnCode using this ElementScopeAuthority
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const;
    //! Create a DgnCode using this ElementScopeAuthority
    DgnCode CreateCode(DgnElementId scopeElementId, Utf8StringCR value) const {return DgnCode(GetAuthorityId(), value, scopeElementId);}
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

    struct EXPORT_VTABLE_ATTRIBUTE Null : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_NullAuthority, NullAuthority, Null, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE DatabaseScope : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_DatabaseScopeAuthority, DatabaseScopeAuthority, DatabaseScope, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE ModelScope : Authority
    {
        AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_ModelScopeAuthority, ModelScopeAuthority, ModelScope, Authority, DGNPLATFORM_EXPORT)
    };

    struct EXPORT_VTABLE_ATTRIBUTE ElementScope : Authority
    {
       AUTHORITYHANDLER_DECLARE_MEMBERS (BIS_CLASS_ElementScopeAuthority, ElementScopeAuthority, ElementScope, Authority, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
