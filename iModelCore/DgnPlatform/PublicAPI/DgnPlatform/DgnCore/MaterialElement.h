/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/MaterialElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
// NOTE: This file exists only temporarily, until the DgnMaterials table is removed and replaced with element-based materials

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

#define DGN_CLASSNAME_MaterialElement "MaterialElement"

DGNPLATFORM_TYPEDEFS(DgnMaterial);
DGNPLATFORM_REF_COUNTED_PTR(DgnMaterial);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Defines a material. Material elements are stored in resource models and identified
//! by a palette name and material name. Each material's name must be unique within its
//! palette, and each palette name unique within its DgnDb.
//! @bsistruct                                                  Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnMaterial : DgnElement
{
    DEFINE_T_SUPER(DgnElement);
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_MaterialElement, DgnElement);
public:
    struct Data
    {
        Utf8String  m_value;
        Utf8String  m_descr;

        Data(Utf8StringCR value="", Utf8StringCR descr="") { Init(value, descr); }
        void Init(Utf8StringCR value="", Utf8StringCR descr="") { m_value = value; m_descr = descr; }
    };

    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnMaterial::T_Super::CreateParams);

        Data    m_data;

        explicit CreateParams(DgnElement::CreateParams const& params, Utf8StringCR value="", Utf8StringCR descr="") : T_Super(params), m_data(value, descr) { }

        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code code, DgnElementId id = DgnElementId(),
                     DgnElementId parent = DgnElementId(), Utf8StringCR value="", Utf8StringCR descr="")
            : T_Super(db, modelId, classId, code, id, parent), m_data(value, descr) { }

        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, DgnModelId modelId, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value,
                    DgnElementId parentMaterialId=DgnElementId(), Utf8StringCR descr="");

        Utf8String GetPaletteName() const { return m_code.GetNameSpace(); }
        Utf8String GetMaterialName() const { return m_code.GetValue(); }
    };

private:
    Data    m_data;

    static void GetParams(bvector<Utf8CP>& params);
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual void _GetSelectParams(bvector<Utf8CP>& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, BeSQLite::EC::ECSqlSelectParameters const& selectParams) override;
    DGNPLATFORM_EXPORT virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId) override;

    virtual uint32_t _GetMemSize() const override { return static_cast<uint32_t>(sizeof(*this) + m_data.m_value.length() + m_data.m_descr.length()); }
public:
    explicit DgnMaterial(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    Utf8String GetPaletteName() const { return GetCode().GetNameSpace(); }
    Utf8String GetMaterialName() const { return GetCode().GetValue(); }

    Utf8StringCR GetValue() const { return m_data.m_value; }
    Utf8StringCR GetDescr() const { return m_data.m_descr; }
    DgnMaterialCPtr GetParentMaterial() const { return GetDgnDb().Elements().Get<DgnMaterial>(GetParentId()); }

    void SetValue(Utf8StringCR value) { m_data.m_value = value; }
    void SetDescr(Utf8StringCR descr) { m_data.m_descr = descr; }

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_MaterialElement); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); }

    DgnMaterialCPtr Insert(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Insert<DgnMaterial>(*this, status); }
    DgnMaterialCPtr Update(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Update<DgnMaterial>(*this, status); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for material elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct Material : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_MaterialElement, DgnMaterial, Material, Element, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

