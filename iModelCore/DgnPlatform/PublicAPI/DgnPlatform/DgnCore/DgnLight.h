/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnLight.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

#define DGN_CLASSNAME_LightDefinition "LightDefinition"

DGNPLATFORM_TYPEDEFS(LightDefinition);
DGNPLATFORM_REF_COUNTED_PTR(LightDefinition);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Holds the definition of a light, with its data encoded in JSON.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LightDefinition : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_LightDefinition, DictionaryElement);
public:
    //! Holds the data which describes a light definition
    struct Data
    {
        Utf8String  m_value;
        Utf8String  m_descr;

        //! Construct with specified value as JSON and optional description
        Data(Utf8StringCR value="", Utf8StringCR descr="") { Init(value, descr); }

        //! Initialize with specified value as JSON and optional description
        void Init(Utf8StringCR value="", Utf8StringCR descr="") { m_value = value; m_descr = descr; }

        uint32_t GetMemSize() const { return static_cast<uint32_t>(sizeof(*this) + m_value.length() + m_descr.length()); }
    };

    //! Parameters used to construct a LightDefinition
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(LightDefinition::T_Super::CreateParams);

        Data m_data;

        //! Constructor from base class. Primarily for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Utf8StringCR value="", Utf8StringCR descr="") : T_Super(params), m_data(value, descr) { }

        //! Constructs parameters for a light definition with the specified values. Primarily for internal use.
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code code, DgnElementId id = DgnElementId(),
                     DgnElementId parent = DgnElementId(), Utf8StringCR value="", Utf8StringCR descr="")
            : T_Super(db, modelId, classId, code, id, parent), m_data(value, descr) { }

        //! Constructs parameters for a light definition with the specified values
        //! @param[in]      db    The DgnDb in which the light definition is to reside
        //! @param[in]      name  The name of the light definition. Must be unique among all light definitions within the DgnDb.
        //! @param[in]      value A JSON string describing the light definition.
        //! @param[in]      descr Optional description of the light definition.
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name, Utf8StringCR value="", Utf8StringCR descr="");

        Utf8StringCR GetName() const { return m_code.GetValue(); } //!< Return the light definition name.
    };

private:
    Data m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;

    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + m_data.GetMemSize(); }
    virtual Code _GenerateDefaultCode() override { return Code(); }
public:
    //! Construct a new LightDefinition with the specified parameters
    explicit LightDefinition(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    DgnLightId GetLightId() const { return DgnLightId(GetElementId().GetValue()); } //!< Returns the ID of this light definition
    Utf8String GetName() const { return GetCode().GetValue(); } //!< Returns the name of this light definition
    Utf8StringCR GetValue() const { return m_data.m_value; } //!< Returns the light data as a JSON string
    Utf8StringCR GetDescr() const { return m_data.m_descr; } //!< Returns the description of this light definition

    void SetValue(Utf8StringCR value) { m_data.m_value = value; } //!< Set the light data as a JSON string
    void SetDescr(Utf8StringCR descr) { m_data.m_descr = descr; } //!< Set the description of this light definition

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_LightDefinition); } //!< Returns the class ID used for light definitions
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< Return the class ID used for light definitions

    //! Insert this light definition into the DgnDb and return the persistent light definition
    LightDefinitionCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<LightDefinition>(*this, status); }

    //! Update this light definition in the DgnDb and return the updated persistent light definition
    LightDefinitionCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<LightDefinition>(*this, status); }

    //! Creates a Code for a light definition.
    DGNPLATFORM_EXPORT static DgnElement::Code CreateLightDefinitionCode(Utf8StringCR name, DgnDbR db);

    //! Looks up the ID of the light definition with the specified code.
    DGNPLATFORM_EXPORT static DgnLightId QueryLightId(DgnElement::Code const& code, DgnDbR db);

    //! Looks up the ID of the light definition with the specified name
    static DgnLightId QueryLightId(Utf8StringCR name, DgnDbR db) { return QueryLightId(CreateLightDefinitionCode(name, db), db); }

    //! Looks up a light definition by ID
    static LightDefinitionCPtr QueryLightDefinition(DgnLightId lightId, DgnDbR db) { return db.Elements().Get<LightDefinition>(lightId); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for light definition elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct LightDef : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_LightDefinition, LightDefinition, LightDef, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

