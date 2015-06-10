//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextStyle.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationPropertyBag.h"

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
enum struct AnnotationStackedFractionType
{
    HorizontalBar = 1,
    DiagonalBar = 2
    
}; // AnnotationStackedFractionType

//=======================================================================================
//! This enumerates all possible AnnotationTextStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationTextStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationTextStylePropertyBag and AnnotationTextStyleProperty.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum struct AnnotationTextStyleProperty
{
    ColorId = 1, //!< (integer, per-run) @note Must be a valid color ID in the project
    FontId = 2, //!< (integer, per-run) @note Must be a valid font ID in the project
    Height = 3, //!< (real, per-document) @note In project UORs
    LineSpacingFactor = 4, //!< (real, per-document) @note Factor of height
    IsBold = 5, //!< (integer, per-run) @note 0 or 1 boolean
    IsItalic = 6, //!< (integer, per-run) @note 0 or 1 boolean
    // IsSubScript = 7 -- deprecated in 06 in favor of a run property
    // IsSuperScript = 8 -- deprecated in 06 in favor of a run property
    IsUnderlined = 9, //!< (integer, per-run) @note 0 or 1 boolean
    StackedFractionScale = 10, //!< (real, per-run) @note Factor of height
    StackedFractionType = 11, //!< (integer, per-run) @note Must exist in the AnnotationStackedFractionType enumeration
    SubScriptOffsetFactor = 12, //!< (real, per-run) @note Factor of height
    SubScriptScale = 13, //!< (real, per-run) @note Factor of height
    SuperScriptOffsetFactor = 14, //!< (real, per-run) @note Factor of height
    SuperScriptScale = 15, //!< (real, per-run) @note Factor of height
    WidthFactor = 16 //!< (real, per-document) @note Factor of height

//__PUBLISH_SECTION_END__
    // *********************************************************************************************
    // **** ADDING MEMBERS? Consider updating: AnnotationTextStylePersistence, TextStyleInterop ****
    // *********************************************************************************************
//__PUBLISH_SECTION_START__

}; // AnnotationTextStyleProperty

//=======================================================================================
//! This specialized collection provides direct access to AnnotationTextStyle property keys and values.
//! Unlike the higher-level AnnotationTextStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationTextStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationTextStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationTextStylePropertyBag and AnnotationTextStyleProperty.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStylePropertyBag : public AnnotationPropertyBag
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    virtual bool _IsIntegerProperty(T_Key) const override;
    virtual bool _IsRealProperty(T_Key) const override;

public:
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBag();
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBag(AnnotationTextStylePropertyBagCR);
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR operator=(AnnotationTextStylePropertyBagCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextStylePropertyBagPtr Create();
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagPtr Clone() const;
    
    DGNPLATFORM_EXPORT bool HasProperty(AnnotationTextStyleProperty) const;
    DGNPLATFORM_EXPORT void ClearProperty(AnnotationTextStyleProperty);
    DGNPLATFORM_EXPORT T_Integer GetIntegerProperty(AnnotationTextStyleProperty) const;
    DGNPLATFORM_EXPORT void SetIntegerProperty(AnnotationTextStyleProperty, T_Integer);
    DGNPLATFORM_EXPORT T_Real GetRealProperty(AnnotationTextStyleProperty) const;
    DGNPLATFORM_EXPORT void SetRealProperty(AnnotationTextStyleProperty, T_Real);

}; // AnnotationTextStylePropertyBag

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationTextBlock.
//! AnnotationTextBlock has different components, such as the block itself, paragraphs, and runs. Different properties of a style affect different components of the AnnotationTextBlock. AnnotationTextStyleProperty indicates which components the properties affect.
//! @see DgnStyles::AnnotationTextStyles for persistence
//! @note When creating an AnnotationTextBlock, the typical work flow is to create and store the style, and then create the AnnotationTextBlock with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextStylePersistence;
    friend struct TextStyleInterop;

    DgnDbP m_dgndb;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationTextStylePropertyBag m_data;

    void CopyFrom(AnnotationTextStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextStyle(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationTextStyle(AnnotationTextStyleCR);
    DGNPLATFORM_EXPORT AnnotationTextStyleR operator=(AnnotationTextStyleCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextStylePtr Create(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr Clone() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle(AnnotationTextStylePropertyBagCR overrides) const;
    DGNPLATFORM_EXPORT DgnFontCR ResolveFont() const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT ColorDef GetColor() const;
    DGNPLATFORM_EXPORT void SetColor(ColorDef);
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

}; // AnnotationTextStyle

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnAnnotationTextStyles : public DgnDbTable
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;
    
    DgnAnnotationTextStyles(DgnDbR db) : T_Super(db) {}

public:
//__PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Iterator : public BeSQLite::DbTableIterator
    {
    private:
        DEFINE_T_SUPER(BeSQLite::DbTableIterator);

    public:
        Iterator(DgnDbCR db) : T_Super((BeSQLite::DbCR)db) {}

        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct Entry : public DbTableIterator::Entry, public std::iterator<std::input_iterator_tag,Entry const>
        {
        private:
            DEFINE_T_SUPER(DbTableIterator::Entry);
            friend struct Iterator;
            
            Entry(BeSQLite::StatementP sql, bool isValid) : T_Super(sql, isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnStyleId GetId() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            Entry const& operator* () const { return *this; }

        }; // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(NULL, false); }
        DGNPLATFORM_EXPORT size_t QueryCount() const;

    }; // Iterator
    
    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationTextStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationTextStylePtr QueryByName(Utf8CP) const;

    //! Creates an iterator to iterate available text styles.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Determines if a text style by-ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryById if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by-name exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryByName if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored during insert, but is updated to the new ID when the method returns. If a style already exists by-name, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Insert(AnnotationTextStyleR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationTextStyleCR);

    //! Deletes a text style from the project. If a style does not exist by-ID, no action is performed.
    //! @note When a style is removed, no attempts are currently made to normalize existing elements. Thus, elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

}; // DgnAnnotationTextStyles

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
