//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextStyle.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/ElementHandler.h>
#include <DgnPlatform/DgnCore/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationTextStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationTextStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextStyle);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! This enumerates all possible annotation text stacked fraction types.
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
    // *********************************************************************************************
    // **** ADDING MEMBERS? Consider updating: AnnotationTextStylePersistence, TextStyleInterop ****
    // *********************************************************************************************
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! This specialized collection provides direct access to AnnotationTextStyle property keys and values.
//! Unlike the higher-level AnnotationTextStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationTextStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationTextStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationTextStylePropertyBag and AnnotationTextStyleProperty.
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

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationTextBlock.
//! AnnotationTextBlock has different components, such as the block itself, paragraphs, and runs. Different properties of a style affect different components of the AnnotationTextBlock. AnnotationTextStyleProperty indicates which components the properties affect.
//! @note When creating an AnnotationTextBlock, the typical work flow is to create and store the style, and then create the AnnotationTextBlock with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationTextStyle : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationTextStyle, DictionaryElement);
public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(AnnotationTextStyle::T_Super::CreateParams);

        AnnotationTextStylePropertyBag  m_data;
        Utf8String                      m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) { }

        //! Constructor
        //! @param[in]      db    DgnDb in which the text style is to reside
        //! @param[in]      name  The name of the text style. Must be unique within the DgnDb
        //! @param[in]      data  Style properties
        //! @param[in]      descr Optional style description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name="", AnnotationTextStylePropertyBagCR data=AnnotationTextStylePropertyBag(), Utf8StringCR descr="");
    };
private:
    friend struct AnnotationTextStylePersistence;
    friend struct TextStyleInterop;

    AnnotationTextStylePropertyBag m_data;
    Utf8String m_descr;

    void Reset();
    void ResetProperties();
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT virtual uint32_t _GetMemSize() const override;

    virtual Code _GenerateDefaultCode() override { return Code(); }
public:
    explicit AnnotationTextStyle(DgnDbR db) : AnnotationTextStyle(CreateParams(db)) { }
    explicit AnnotationTextStyle(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) { }

    static AnnotationTextStylePtr Create(DgnDbR db) { return new AnnotationTextStyle(db); }
    AnnotationTextStylePtr Clone() const { return MakeCopy<AnnotationTextStyle>(); }

    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle(AnnotationTextStylePropertyBagCR overrides) const;

    AnnotationTextStyleId GetStyleId() const { return AnnotationTextStyleId(GetElementId().GetValueUnchecked()); }
    Utf8String GetName() const { return GetCode().GetValue(); }
    Utf8StringCR GetDescription() const { return m_descr; }
    void SetDescription(Utf8StringCR value) { m_descr = value; }
    void SetName(Utf8StringCR value) { SetCode(CreateStyleCode(value)); }

    DGNPLATFORM_EXPORT static Code CreateStyleCode(Utf8StringCR name);
    
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

    DGNPLATFORM_EXPORT DgnFontCR ResolveFont() const;

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationTextStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    AnnotationTextStyleCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<AnnotationTextStyle>(*this, status); }
    AnnotationTextStyleCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<AnnotationTextStyle>(*this, status); }

    DGNPLATFORM_EXPORT static AnnotationTextStyleId QueryStyleId(Code const& code, DgnDbR db);
    static AnnotationTextStyleId QueryStyleId(Utf8StringCR styleName, DgnDbR db) { return QueryStyleId(CreateStyleCode(styleName), db); }
    static AnnotationTextStyleCPtr QueryStyle(AnnotationTextStyleId styleId, DgnDbR db) { return db.Elements().Get<AnnotationTextStyle>(styleId); }
    static AnnotationTextStyleCPtr QueryStyle(Utf8StringCR styleName, DgnDbR db) { return QueryStyle(QueryStyleId(styleName, db), db); }

    DGNPLATFORM_EXPORT static bool ExistsById(AnnotationTextStyleId id, DgnDbR db);
    static bool ExistsByName(Utf8StringCR name, DgnDbR db) { return QueryStyleId(name, db).IsValid(); }

    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db);

    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationTextStyle;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        AnnotationTextStyleId GetId() const { return m_statement->GetValueId<AnnotationTextStyleId>(0); }
        Utf8CP GetName() const { return m_statement->GetValueText(1); }
        Utf8CP GetDescription() const { return m_statement->GetValueText(2); }
    };

    struct Iterator : ECSqlStatementIterator<Entry>
    {
    };

    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, bool ordered=false);
    static Iterator MakeOrderedIterator(DgnDbR db) { return MakeIterator(db, true); }
};

//! @endGroup

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for annotation text styles
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct AnnotationTextStyleHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationTextStyle, AnnotationTextStyle, AnnotationTextStyleHandler, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
