//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextStyle.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum struct AnnotationStackedFractionType
{
    HorizontalBar = 1,
    DiagonalBar = 2
    
}; // AnnotationStackedFractionType

//=======================================================================================
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
    IsSubScript = 7, //!< (integer, per-run) @note 0 or 1 boolean
    IsSuperScript = 8, //!< (integer, per-run) @note 0 or 1 boolean
    IsUnderlined = 9, //!< (integer, per-run) @note 0 or 1 boolean
    StackedFractionScale = 10, //!< (real, per-run) @note Factor of height
    StackedFractionType = 11, //!< (integer, per-run) @note Must exist in the AnnotationStackedFractionType enumeration
    SubScriptOffsetFactor = 12, //!< (real, per-run) @note Factor of height
    SubScriptScale = 13, //!< (real, per-run) @note Factor of height
    SuperScriptOffsetFactor = 14, //!< (real, per-run) @note Factor of height
    SuperScriptScale = 15, //!< (real, per-run) @note Factor of height
    WidthFactor = 16 //!< (real, per-document) @note Factor of height

}; // AnnotationTextStyleProperty

//=======================================================================================
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
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextStylePersistence;

    DgnProjectP m_project;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationTextStylePropertyBag m_data;

    void CopyFrom(AnnotationTextStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextStyle(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationTextStyle(AnnotationTextStyleCR);
    DGNPLATFORM_EXPORT AnnotationTextStyleR operator=(AnnotationTextStyleCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextStylePtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr Clone() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle(AnnotationTextStylePropertyBagCR overrides) const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT UInt32 GetColorId() const;
    DGNPLATFORM_EXPORT void SetColorId(UInt32);
    DGNPLATFORM_EXPORT UInt32 GetFontId() const;
    DGNPLATFORM_EXPORT void SetFontId(UInt32);
    DGNPLATFORM_EXPORT double GetHeight() const;
    DGNPLATFORM_EXPORT void SetHeight(double);
    DGNPLATFORM_EXPORT double GetLineSpacingFactor() const;
    DGNPLATFORM_EXPORT void SetLineSpacingFactor(double);
    DGNPLATFORM_EXPORT bool IsBold() const;
    DGNPLATFORM_EXPORT void SetIsBold(bool);
    DGNPLATFORM_EXPORT bool IsItalic() const;
    DGNPLATFORM_EXPORT void SetIsItalic(bool);
    DGNPLATFORM_EXPORT bool IsSubScript() const;
    DGNPLATFORM_EXPORT void SetIsSubScript(bool);
    DGNPLATFORM_EXPORT bool IsSuperScript() const;
    DGNPLATFORM_EXPORT void SetIsSuperScript(bool);
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
struct DgnAnnotationTextStyles
{
private:
    friend struct DgnStyles;

    DgnProjectR m_project;

    // Only the outer class is designed to construct this class.
    DgnAnnotationTextStyles(DgnProjectR project) : m_project(project) {}

public:
    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationTextStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationTextStylePtr QueryByName(Utf8CP) const;

    //! Determines if a text style by the ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by the name exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT AnnotationTextStylePtr Insert(AnnotationTextStyleCR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationTextStyleCR);

    //! Deletes a text style from the project. The ID in the provided style is used to identify which style to delete. If a style does not exist by-ID, no action is performed.
    //! @note When a text style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

    //! Creates an iterator to iterate available text styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;

};


//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
