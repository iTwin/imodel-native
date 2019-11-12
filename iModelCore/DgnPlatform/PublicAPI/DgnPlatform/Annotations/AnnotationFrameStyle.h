/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationFrameStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationFrameStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible annotation frame types.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationFrameType
{
    InvisibleBox = 1,
    Box = 2,
    Circle = 3,
    Ellipse = 4
};

//=======================================================================================
//! This enumerates all possible AnnotationFrameStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationFrameStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationFrameStylePropertyBag and AnnotationFrameStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationFrameStyleProperty
{
    CloudBulgeFactor = 1, //!< (real) @note Factor of effective diameter
    CloudDiameterFactor = 2, //!< (real) @note Factor of text height
    FillColorType = 3, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    FillColorValue = 4, //!< (integer) @note int64_t representation of ElementColor
    FillTransparency = 5, //!< (real) @note [0.0..1.0]
    HorizontalPadding = 6, //!< (real) @note Factor of the first character's text height
    IsFillEnabled = 7, //!< (integer) @note 0 or 1 boolean
    IsStrokeCloud = 8, //!< (integer) @note 0 or 1 boolean
    IsStrokeEnabled = 9, //!< (integer) @note 0 or 1 boolean
    StrokeColorType = 10, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    StrokeColorValue = 11, //!< (integer) @note Equivalent to ColorDef as an integer
    StrokeWeight = 12, //!< (integer) @note Must be a standard line weight
    Type = 13, //!< (integer) @note Must exist in the AnnotationFrameType enumeration
    VerticalPadding = 14 //!< (real) @note Factor of the first character's text height
};

//=======================================================================================
//! This specialized collection provides direct access to AnnotationFrameStyle property keys and values.
//! Unlike the higher-level AnnotationFrameStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationFrameStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationFrameStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationFrameStylePropertyBag and AnnotationFrameStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationFrameStylePropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)

protected:
    DGNPLATFORM_EXPORT bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT bool _IsRealProperty(T_Key) const override;

public:
    AnnotationFrameStylePropertyBag() : T_Super() {}
    AnnotationFrameStylePropertyBag(AnnotationFrameStylePropertyBagCR rhs) : T_Super(rhs) {}
    AnnotationFrameStylePropertyBagR operator=(AnnotationFrameStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}
    static AnnotationFrameStylePropertyBagPtr Create() { return new AnnotationFrameStylePropertyBag(); }
    AnnotationFrameStylePropertyBagPtr Clone() const { return new AnnotationFrameStylePropertyBag(*this); }

    bool HasProperty(AnnotationFrameStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
    void ClearProperty(AnnotationFrameStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
    T_Integer GetIntegerProperty(AnnotationFrameStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
    void SetIntegerProperty(AnnotationFrameStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
    T_Real GetRealProperty(AnnotationFrameStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
    void SetRealProperty(AnnotationFrameStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }
};

//! As an element, AnnotationFrameStyle IDs are inherently DgnElementId, but create a typedef so that argument types are more obvious/natural.
typedef DgnElementId AnnotationFrameStyleId;

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationFrame.
//! @note When creating an AnnotationFrame, the typical work flow is to create and store the style, and then create the AnnotationFrame with the stored style's ID.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationFrameStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AnnotationFrameStyle, DefinitionElement);

private:
    friend struct AnnotationFrameStylePersistence;

    Utf8String m_description;
    AnnotationFrameStylePropertyBag m_data;

    static DgnCode CreateCode(DefinitionModelCR model, Utf8StringCR name) { return CodeSpec::CreateCode(BIS_CODESPEC_AnnotationFrameStyle, model, name); }

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR source, CopyFromOptions const&) override;
    DgnDbStatus _OnDelete() const override { return DgnDbStatus::DeletionProhibited; /* Must be "purged" */ }
    uint32_t _GetMemSize() const override { return (uint32_t)(m_description.size() + 1 + m_data.GetMemSize()); }
    DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override { return !codeSpec.IsNullCodeSpec(); }

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationFrameStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    explicit AnnotationFrameStyle(DefinitionModelCR model) : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryDgnClassId(model.GetDgnDb()), DgnCode())) {}
    explicit AnnotationFrameStyle(CreateParams const& params) : T_Super(params) {}
    static AnnotationFrameStylePtr Create(DefinitionModelCR model) { return new AnnotationFrameStyle(model); }
    AnnotationFrameStylePtr CreateCopy() const { return MakeCopy<AnnotationFrameStyle>(); }

    DefinitionModelCPtr GetDefinitionModel() {return GetDgnDb().Models().Get<DefinitionModel>(GetModelId());}

    Utf8String GetName() const { return GetCode().GetValue().GetUtf8(); }
    void SetName(Utf8CP value) { T_Super::SetCode(CreateCode(*GetDefinitionModel(), value)); /* Only SetName is allowed to SetCode. */ }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }

    DGNPLATFORM_EXPORT double GetCloudBulgeFactor() const;
    DGNPLATFORM_EXPORT void SetCloudBulgeFactor(double);
    DGNPLATFORM_EXPORT double GetCloudDiameterFactor() const;
    DGNPLATFORM_EXPORT void SetCloudDiameterFactor(double);
    DGNPLATFORM_EXPORT AnnotationColorType GetFillColorType() const;
    DGNPLATFORM_EXPORT void SetFillColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetFillColorValue() const;
    DGNPLATFORM_EXPORT void SetFillColorValue(ColorDef);
    DGNPLATFORM_EXPORT double GetFillTransparency() const;
    DGNPLATFORM_EXPORT void SetFillTransparency(double);
    DGNPLATFORM_EXPORT double GetHorizontalPadding() const;
    DGNPLATFORM_EXPORT void SetHorizontalPadding(double);
    DGNPLATFORM_EXPORT bool IsFillEnabled() const;
    DGNPLATFORM_EXPORT void SetIsFillEnabled(bool);
    DGNPLATFORM_EXPORT bool IsStrokeCloud() const;
    DGNPLATFORM_EXPORT void SetIsStrokeCloud(bool);
    DGNPLATFORM_EXPORT bool IsStrokeEnabled() const;
    DGNPLATFORM_EXPORT void SetIsStrokeEnabled(bool);
    DGNPLATFORM_EXPORT AnnotationColorType GetStrokeColorType() const;
    DGNPLATFORM_EXPORT void SetStrokeColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetStrokeColorValue() const;
    DGNPLATFORM_EXPORT void SetStrokeColorValue(ColorDef);
    DGNPLATFORM_EXPORT uint32_t GetStrokeWeight() const;
    DGNPLATFORM_EXPORT void SetStrokeWeight(uint32_t);
    DGNPLATFORM_EXPORT AnnotationFrameType GetType() const;
    DGNPLATFORM_EXPORT void SetType(AnnotationFrameType);
    DGNPLATFORM_EXPORT double GetVerticalPadding() const;
    DGNPLATFORM_EXPORT void SetVerticalPadding(double);
    void SetPadding(double value) { SetHorizontalPadding(value); SetVerticalPadding(value); }

    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const;

    static DgnElementId QueryId(DefinitionModelCR model, Utf8CP name) { return model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, name)); }
    static AnnotationFrameStyleCPtr Get(DefinitionModelCR model, Utf8CP name) { return Get(model.GetDgnDb(), QueryId(model, name)); }
    static AnnotationFrameStyleCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<AnnotationFrameStyle>(id); }
    static AnnotationFrameStylePtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<AnnotationFrameStyle>(id); }
    static AnnotationFrameStylePtr GetForEdit(DefinitionModelCR model, Utf8CP name) { return GetForEdit(model.GetDgnDb(), QueryId(model, name)); }
    AnnotationFrameStyleCPtr Insert() { return GetDgnDb().Elements().Insert<AnnotationFrameStyle>(*this); }
    AnnotationFrameStyleCPtr Update() { return GetDgnDb().Elements().Update<AnnotationFrameStyle>(*this); }

    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass                                                    Jeff.Marker     11/2014
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
        {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationFrameStyle;

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
    //! The handler for annotation frame styles
    //=======================================================================================
    struct AnnotationFrameStyleHandler : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AnnotationFrameStyle, AnnotationFrameStyle, AnnotationFrameStyleHandler, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };
}

END_BENTLEY_DGN_NAMESPACE
