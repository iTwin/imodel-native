/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationLeaderStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible annotation leader line types.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
enum class AnnotationLeaderLineType
{
    None = 1,
    Straight = 2,
    Curved = 3
};

//=======================================================================================
//! This enumerates all possible annotation leader terminator types.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
enum class AnnotationLeaderTerminatorType
{
    None = 1,
    OpenArrow = 2,
    ClosedArrow = 3
};

//=======================================================================================
//! This enumerates all possible AnnotationLeaderStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationLeaderStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationLeaderStylePropertyBag and AnnotationLeaderStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
enum class AnnotationLeaderStyleProperty
{
    LineColorType = 1, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    LineColorValue = 2, //!< (integer) @note int64_t representation of ElementColor
    LineType = 3, //!< (integer) @note Must exist in the AnnotationLeaderLineType enumeration
    LineWeight = 4, //!< (integer) @note Must be a standard line weight
    TerminatorColorType = 5, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    TerminatorColorValue = 6, //!< (integer) @note int64_t representation of ElementColor
    TerminatorScaleFactor = 7, //!< (real) @note Generally describes the length of the side of the box encompassing the terminator, as a factor of the first character's text height
    TerminatorType = 8, //!< (integer) @note Must exist in the AnnotationLeaderTerminatorType enumeration
    TerminatorWeight = 9 //!< (integer) @note Must be a standard line weight
};

//=======================================================================================
//! This specialized collection provides direct access to AnnotationLeaderStyle property keys and values.
//! Unlike the higher-level AnnotationLeaderStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationLeaderStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationLeaderStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationLeaderStylePropertyBag and AnnotationLeaderStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationLeaderStylePropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)

protected:
    DGNPLATFORM_EXPORT bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT bool _IsRealProperty(T_Key) const override;

public:
    AnnotationLeaderStylePropertyBag() : T_Super() {}
    AnnotationLeaderStylePropertyBag(AnnotationLeaderStylePropertyBagCR rhs) : T_Super(rhs) {}
    AnnotationLeaderStylePropertyBagR operator=(AnnotationLeaderStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}
    static AnnotationLeaderStylePropertyBagPtr Create() { return new AnnotationLeaderStylePropertyBag(); }
    AnnotationLeaderStylePropertyBagPtr Clone() const { return new AnnotationLeaderStylePropertyBag(*this); }

    bool HasProperty(AnnotationLeaderStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
    void ClearProperty(AnnotationLeaderStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
    T_Integer GetIntegerProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
    void SetIntegerProperty(AnnotationLeaderStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
    T_Real GetRealProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
    void SetRealProperty(AnnotationLeaderStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }
};

//! As an element, AnnotationLeaderStyle IDs are inherently DgnElementId, but create a typedef so that argument types are more obvious/natural.
//! @ingroup GROUP_Annotation
typedef DgnElementId AnnotationLeaderStyleId;

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationLeader.
//! @note When creating an AnnotationLeader, the typical work flow is to create and store the style, and then create the AnnotationLeader with the stored style's ID.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationLeaderStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AnnotationLeaderStyle, DefinitionElement);

private:
    friend struct AnnotationLeaderStylePersistence;

    Utf8String m_description;
    AnnotationLeaderStylePropertyBag m_data;

    static DgnCode CreateCode(DefinitionModelCR model, Utf8StringCR name) { return CodeSpec::CreateCode(BIS_CODESPEC_AnnotationLeaderStyle, model, name); }

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR source, CopyFromOptions const&) override;
    DgnDbStatus _OnDelete() const override { return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited; /* Must be "purged" */ }
    uint32_t _GetMemSize() const override { return (uint32_t)(m_description.size() + 1 + m_data.GetMemSize()); }
    DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override { return !codeSpec.IsNullCodeSpec(); }

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationLeaderStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    explicit AnnotationLeaderStyle(DefinitionModelCR model) : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryDgnClassId(model.GetDgnDb()), DgnCode())) {}
    explicit AnnotationLeaderStyle(CreateParams const& params) : T_Super(params) {}
    static AnnotationLeaderStylePtr Create(DefinitionModelCR model) { return new AnnotationLeaderStyle(model); }
    AnnotationLeaderStylePtr CreateCopy() const { return MakeCopy<AnnotationLeaderStyle>(); }

    DefinitionModelCPtr GetDefinitionModel() {return GetDgnDb().Models().Get<DefinitionModel>(GetModelId());}

    Utf8String GetName() const { return GetCode().GetValue().GetUtf8(); }
    void SetName(Utf8CP value) { T_Super::SetCode(CreateCode(*GetDefinitionModel(), value)); /* Only SetName is allowed to SetCode. */ }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }

    DGNPLATFORM_EXPORT AnnotationColorType GetLineColorType() const;
    DGNPLATFORM_EXPORT void SetLineColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetLineColorValue() const;
    DGNPLATFORM_EXPORT void SetLineColorValue(ColorDef);
    DGNPLATFORM_EXPORT AnnotationLeaderLineType GetLineType() const;
    DGNPLATFORM_EXPORT void SetLineType(AnnotationLeaderLineType);
    DGNPLATFORM_EXPORT uint32_t GetLineWeight() const;
    DGNPLATFORM_EXPORT void SetLineWeight(uint32_t);
    DGNPLATFORM_EXPORT AnnotationColorType GetTerminatorColorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetTerminatorColorValue() const;
    DGNPLATFORM_EXPORT void SetTerminatorColorValue(ColorDef);
    DGNPLATFORM_EXPORT double GetTerminatorScaleFactor() const;
    DGNPLATFORM_EXPORT void SetTerminatorScaleFactor(double);
    DGNPLATFORM_EXPORT AnnotationLeaderTerminatorType GetTerminatorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorType(AnnotationLeaderTerminatorType);
    DGNPLATFORM_EXPORT uint32_t GetTerminatorWeight() const;
    DGNPLATFORM_EXPORT void SetTerminatorWeight(uint32_t);

    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const;

    static DgnElementId QueryId(DefinitionModelCR model, Utf8CP name) { return model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, name)); }
    static AnnotationLeaderStyleCPtr Get(DefinitionModelCR model, Utf8CP name) { return Get(model.GetDgnDb(), QueryId(model, name)); }
    static AnnotationLeaderStyleCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<AnnotationLeaderStyle>(id); }
    static AnnotationLeaderStylePtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<AnnotationLeaderStyle>(id); }
    static AnnotationLeaderStylePtr GetForEdit(DefinitionModelCR model, Utf8CP name) { return GetForEdit(model.GetDgnDb(), QueryId(model, name)); }
    AnnotationLeaderStyleCPtr Insert() { return GetDgnDb().Elements().Insert<AnnotationLeaderStyle>(*this); }

    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
        {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationLeaderStyle;

        private:
            Entry() : T_Super(nullptr) {}
            Entry(BeSQLite::EC::ECSqlStatement* stmt) : T_Super(stmt) {}

        public:
            DgnElementId GetElementId() const { return m_statement->GetValueId<DgnElementId>(0); }
            Utf8CP GetName() const { return m_statement->GetValueText(1); }
            Utf8CP GetDescription() const { return m_statement->GetValueText(2); }
        };

    typedef ECSqlStatementIterator<Entry> Iterator;

    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR);
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR);
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for annotation leader styles
    //=======================================================================================
    struct AnnotationLeaderStyleHandler : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AnnotationLeaderStyle, AnnotationLeaderStyle, AnnotationLeaderStyleHandler, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };
}

END_BENTLEY_DGN_NAMESPACE
