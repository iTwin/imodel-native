//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationTextStyle.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationTextStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationTextStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible annotation text stacked fraction types.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class AnnotationStackedFractionType
{
    HorizontalBar = 1,
    DiagonalBar = 2
};

//=======================================================================================
//! This enumerates all possible AnnotationTextStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationTextStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationTextStylePropertyBag and AnnotationTextStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class AnnotationTextStyleProperty
{
    ColorType = 1, //!< (integer, per-run) @note Must exist in the AnnotationColorType enumeration
    ColorValue = 2, //!< (integer, per-run) @note int64_t representation of ElementColor
    FontId = 3, //!< (integer, per-run) @note Must be a valid font ID in the project
    Height = 4, //!< (real, per-document) @note In project UORs
    LineSpacingFactor = 5, //!< (real, per-document) @note Factor of height
    IsBold = 6, //!< (integer, per-run) @note 0 or 1 boolean
    IsItalic = 7, //!< (integer, per-run) @note 0 or 1 boolean
    IsUnderlined = 8, //!< (integer, per-run) @note 0 or 1 boolean
    StackedFractionScale = 9, //!< (real, per-run) @note Factor of height
    StackedFractionType = 10, //!< (integer, per-run) @note Must exist in the AnnotationStackedFractionType enumeration
    SubScriptOffsetFactor = 11, //!< (real, per-run) @note Factor of height
    SubScriptScale = 12, //!< (real, per-run) @note Factor of height
    SuperScriptOffsetFactor = 13, //!< (real, per-run) @note Factor of height
    SuperScriptScale = 14, //!< (real, per-run) @note Factor of height
    WidthFactor = 15 //!< (real, per-document) @note Factor of height

//__PUBLISH_SECTION_END__
    // ****************************************************************************************************************************************
    // **** ADDING MEMBERS? Consider updating: AnnotationTextStylePersistence, TextStyleInterop, AnnotationTextStyle::CreateEffectiveStyle ****
    // ****************************************************************************************************************************************
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! This specialized collection provides direct access to AnnotationTextStyle property keys and values.
//! Unlike the higher-level AnnotationTextStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationTextStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationTextStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationTextStylePropertyBag and AnnotationTextStyleProperty.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationTextStylePropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    DGNPLATFORM_EXPORT virtual bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT virtual bool _IsRealProperty(T_Key) const override;

public:
    AnnotationTextStylePropertyBag() : T_Super() {}
    AnnotationTextStylePropertyBag(AnnotationTextStylePropertyBagCR rhs) : T_Super(rhs) {}
    AnnotationTextStylePropertyBagR operator=(AnnotationTextStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}
    static AnnotationTextStylePropertyBagPtr Create() { return new AnnotationTextStylePropertyBag(); }
    AnnotationTextStylePropertyBagPtr Clone() const { return new AnnotationTextStylePropertyBag(*this); }
    
    bool HasProperty(AnnotationTextStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
    void ClearProperty(AnnotationTextStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
    T_Integer GetIntegerProperty(AnnotationTextStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
    void SetIntegerProperty(AnnotationTextStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
    T_Real GetRealProperty(AnnotationTextStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
    void SetRealProperty(AnnotationTextStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }
};

//! As an element, AnnotationTextStyle IDs are inherently DgnElementId, but create a typedef so that argument types are more obvious/natural.
//! @ingroup GROUP_Annotation
typedef DgnElementId AnnotationTextStyleId;

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationTextBlock.
//! AnnotationTextBlock has different components, such as the block itself, paragraphs, and runs. Different properties of a style affect different components of the AnnotationTextBlock. AnnotationTextStyleProperty indicates which components the properties affect.
//! @note When creating an AnnotationTextBlock, the typical work flow is to create and store the style, and then create the AnnotationTextBlock with the stored style's ID.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationTextStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AnnotationTextStyle, DefinitionElement);
    
private:
    friend struct AnnotationTextStylePersistence;
    friend struct TextStyleInterop;

    Utf8String m_description;
    AnnotationTextStylePropertyBag m_data;

    static DgnCode CreateCodeFromName(Utf8StringCR name) { return ResourceAuthority::CreateResourceCode(name, BIS_CLASS_AnnotationTextStyle); }

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParams const&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;
    virtual DgnDbStatus _OnDelete() const override { return DgnDbStatus::DeletionProhibited; /* Must be "purged" */ }
    virtual uint32_t _GetMemSize() const override { return (uint32_t)(m_description.size() + 1 + m_data.GetMemSize()); }
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override { return ResourceAuthority::IsResourceAuthority(auth); }
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext&) override;

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationTextStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    
    explicit AnnotationTextStyle(DgnDbR db) : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryDgnClassId(db), DgnCode())) {}
    explicit AnnotationTextStyle(CreateParams const& params) : T_Super(params) {}
    static AnnotationTextStylePtr Create(DgnDbR db) { return new AnnotationTextStyle(db); }
    AnnotationTextStylePtr CreateCopy() const { return MakeCopy<AnnotationTextStyle>(); }

    Utf8String GetName() const { return GetCode().GetValue(); }
    void SetName(Utf8CP value) { T_Super::_SetCode(CreateCodeFromName(value)); /* Only SetName is allowed to SetCode. */ }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }
    
    DGNPLATFORM_EXPORT AnnotationColorType GetColorType() const;
    DGNPLATFORM_EXPORT void SetColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetColorValue() const;
    DGNPLATFORM_EXPORT void SetColorValue(ColorDef);
    DGNPLATFORM_EXPORT DgnFontId GetFontId() const;
    DGNPLATFORM_EXPORT void SetFontId(DgnFontId);
    DGNPLATFORM_EXPORT double GetHeight() const;
    DGNPLATFORM_EXPORT void SetHeight(double);
    DGNPLATFORM_EXPORT double GetLineSpacingFactor() const;
    DGNPLATFORM_EXPORT void SetLineSpacingFactor(double);
    DGNPLATFORM_EXPORT bool IsBold() const;
    DGNPLATFORM_EXPORT void SetIsBold(bool);
    DGNPLATFORM_EXPORT bool IsItalic() const;
    DGNPLATFORM_EXPORT void SetIsItalic(bool);
    DGNPLATFORM_EXPORT bool IsUnderlined() const;
    DGNPLATFORM_EXPORT void SetIsUnderlined(bool);
    DGNPLATFORM_EXPORT double GetStackedFractionScale() const;
    DGNPLATFORM_EXPORT void SetStackedFractionScale(double);
    DGNPLATFORM_EXPORT AnnotationStackedFractionType GetStackedFractionType() const;
    DGNPLATFORM_EXPORT void SetStackedFractionType(AnnotationStackedFractionType);
    DGNPLATFORM_EXPORT double GetSubScriptOffsetFactor() const;
    DGNPLATFORM_EXPORT void SetSubScriptOffsetFactor(double);
    DGNPLATFORM_EXPORT double GetSubScriptScale() const;
    DGNPLATFORM_EXPORT void SetSubScriptScale(double);
    DGNPLATFORM_EXPORT double GetSuperScriptOffsetFactor() const;
    DGNPLATFORM_EXPORT void SetSuperScriptOffsetFactor(double);
    DGNPLATFORM_EXPORT double GetSuperScriptScale() const;
    DGNPLATFORM_EXPORT void SetSuperScriptScale(double);
    DGNPLATFORM_EXPORT double GetWidthFactor() const;
    DGNPLATFORM_EXPORT void SetWidthFactor(double);

    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle(AnnotationTextStylePropertyBagCR overrides) const;
    DGNPLATFORM_EXPORT static AnnotationTextStylePtr CreateEffectiveStyle(AnnotationTextStyleCR docStyle, AnnotationTextStylePropertyBagCR docOverrides, AnnotationTextStyleCR parStyle, AnnotationTextStylePropertyBagCR parOverrides, AnnotationTextStyleCR runStyle, AnnotationTextStylePropertyBagCR runOverrides);
    DgnFontCR ResolveFont() const { return DgnFontManager::ResolveFont(m_dgndb.Fonts().FindFontById(GetFontId())); }

    static DgnElementId QueryId(DgnDbR db, Utf8CP name) { return db.Elements().QueryElementIdByCode(CreateCodeFromName(name)); }
    static AnnotationTextStyleCPtr Get(DgnDbR db, Utf8CP name) { return Get(db, QueryId(db, name)); }
    static AnnotationTextStyleCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<AnnotationTextStyle>(id); }
    static AnnotationTextStylePtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<AnnotationTextStyle>(id); }
    static AnnotationTextStylePtr GetForEdit(DgnDbR db, Utf8CP name) { return GetForEdit(db, QueryId(db, name)); }
    AnnotationTextStyleCPtr Insert() { return GetDgnDb().Elements().Insert<AnnotationTextStyle>(*this); }
    AnnotationTextStyleCPtr Update() { return GetDgnDb().Elements().Update<AnnotationTextStyle>(*this); }

    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass                                                    Jeff.Marker     11/2014
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
    {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationTextStyle;
    
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
    //! The handler for annotation text styles
    //=======================================================================================
    struct AnnotationTextStyleHandler : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AnnotationTextStyle, AnnotationTextStyle, AnnotationTextStyleHandler, Definition, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGN_NAMESPACE
