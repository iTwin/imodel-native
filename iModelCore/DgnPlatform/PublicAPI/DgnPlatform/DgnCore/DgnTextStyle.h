/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnTextStyle.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/RefCounted.h>
#include <DgnPlatform/DgnCore/TextParam.h>

//__PUBLISH_SECTION_END__

#include <bitset>

//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS(DgnTextStyle)
DGNPLATFORM_REF_COUNTED_PTR(DgnTextStyle)
DGNPLATFORM_TYPEDEFS(DgnTextStylePropertyMask)
DGNPLATFORM_REF_COUNTED_PTR(DgnTextStylePropertyMask)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @note Unless otherwise noted, "Factor" properties are factors of text height.
// @bsiclass                                                    Jeff.Marker     12/2013
//=======================================================================================
enum struct DgnTextStyleProperty
{
    BackgroundBorderColor = 1, //!< (UInt32) 
    BackgroundBorderLineStyle = 2, //!< (Int32) 
    BackgroundBorderPaddingFactor = 3, //!< (DPoint2d) 
    BackgroundBorderWeight = 4, //!< (UInt32) 
    BackgroundFillColor = 5, //!< (UInt32) 
    CharacterSpacingType = 6, //!< (UInt32) @note Must exist in the CharacterSpacingType enumeration.
    CharacterSpacingValueFactor = 7, //!< (double) 
    Color = 8, //!< (UInt32) 
    CustomSlantAngle = 9, //!< (double) 
    Font = 10, //!< (DgnFontCP) @note Can also be passed for a UInt32; the project will be used to acquire a number for the underlying DgnFont object.
    HasColor = 11, //!< (bool) 
    Height = 12, //!< (double) 
    IsBackwards = 13, //!< (bool) 
    IsBold = 14, //!< (bool) 
    IsFullJustification = 15, //!< (bool) 
    IsItalics = 16, //!< (bool) 
    IsOverlined = 17, //!< (bool) 
    IsSubScript = 18, //!< (bool) 
    IsSuperScript = 19, //!< (bool) 
    IsUnderlined = 20, //!< (bool) 
    IsUpsideDown = 21, //!< (bool) 
    IsVertical = 22, //!< (bool) 
    Justification = 23, //!< (UInt32) @note Must exist in the TextElementJustification enumeration.
    LineSpacingType = 24, //!< (UInt32) @note Must exist in the DgnLineSpacingType enumeration.
    LineSpacingValueFactor = 25, //!< (double) 
    MaxCharactersPerLine = 26, //!< (UInt32) 
    OverlineColor = 27, //!< (UInt32) 
    OverlineLineStyle = 28, //!< (Int32) 
    OverlineOffsetFactor = 29, //!< (double) 
    OverlineWeight = 30, //!< (UInt32) 
    RunOffsetFactor = 31, //!< (DPoint2d) 
    ShouldUseBackground = 32, //!< (bool) 
    ShouldUseOverlineStyle = 33, //!< (bool) 
    ShouldUseUnderlineStyle = 34, //!< (bool) 
    ShxBigFont = 35, //!< (DgnFontCP) @note Can also be passed for a UInt32; the project will be used to acquire a number for the underlying DgnFont object.
    UnderlineColor = 36, //!< (UInt32) 
    UnderlineLineStyle = 37, //!< (Int32) 
    UnderlineOffsetFactor = 38, //!< (double) 
    UnderlineWeight = 39, //!< (UInt32) 
    WidthFactor = 40, //!< (double) 

//__PUBLISH_SECTION_END__
    Count = 40, //!< 

//__PUBLISH_SECTION_START__
    BackgroundBorderPadding = -1, //!< (DPoint2d) @note This is a derived property (based on one or more other properties).
    CharacterSpacingValue = -2, //!< (double) @note This is a derived property (based on one or more other properties).
    LineSpacingValue = -3, //!< (double) @note This is a derived property (based on one or more other properties).
    OverlineOffset = -4, //!< (double) @note This is a derived property (based on one or more other properties).
    RunOffset = -5, //!< (DPoint2d) @note This is a derived property (based on one or more other properties).
    UnderlineOffset = -6, //!< (double) @note This is a derived property (based on one or more other properties).
    Width = -7, //!< (double) @note This is a derived property (based on one or more other properties).

}; // DgnTextStyleProperty

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/2013
//=======================================================================================
struct DgnTextStylePropertyMask : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    std::bitset<(size_t)DgnTextStyleProperty::Count> m_propertyFlags;
    
    DgnTextStylePropertyMask();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT static DgnTextStylePropertyMaskPtr Create();
    DGNPLATFORM_EXPORT DgnTextStylePropertyMaskPtr Clone() const;
    DGNPLATFORM_EXPORT bool IsPropertySet(DgnTextStyleProperty) const;
    DGNPLATFORM_EXPORT bool AreAnyPropertiesSet() const;
    DGNPLATFORM_EXPORT void SetProperty(DgnTextStyleProperty, bool);
    DGNPLATFORM_EXPORT void SetAllProperties(bool);
    DGNPLATFORM_EXPORT void LogicalOr(DgnTextStylePropertyMaskCR);
}; // DgnTextStylePropertyMask

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/2013
//=======================================================================================
struct DgnTextStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    friend struct DgnTextStylePersistence;

    DgnProjectP m_project;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    
    UInt32 m_backgroundBorderColor;
    Int32 m_backgroundBorderLineStyle;
    DPoint2d m_backgroundBorderPaddingFactor;
    UInt32 m_backgroundBorderWeight;
    UInt32 m_backgroundFillColor;
    DgnFontCP m_shxBigFont;
    CharacterSpacingType m_characterSpacingType;
    double m_characterSpacingValueFactor;
    UInt32 m_color;
    double m_customSlantAngle;
    DgnFontCP m_font;
    bool m_hasColor;
    double m_height;
    bool m_isBackwards;
    bool m_isBold;
    bool m_isFullJustification;
    bool m_isItalics;
    bool m_isOverlined;
    bool m_isSubScript;
    bool m_isSuperScript;
    bool m_isUnderlined;
    bool m_isUpsideDown;
    bool m_isVertical;
    TextElementJustification m_justification;
    DgnLineSpacingType m_lineSpacingType;
    double m_lineSpacingValueFactor;
    UInt32 m_maxCharactersPerLine;
    UInt32 m_overlineColor;
    Int32 m_overlineLineStyle;
    double m_overlineOffsetFactor;
    UInt32 m_overlineWeight;
    DPoint2d m_runOffsetFactor;
    bool m_shouldUseBackground;
    bool m_shouldUseOverlineStyle;
    bool m_shouldUseUnderlineStyle;
    UInt32 m_underlineColor;
    Int32 m_underlineLineStyle;
    double m_underlineOffsetFactor;
    UInt32 m_underlineWeight;
    double m_widthFactor;

protected:
    DGNPLATFORM_EXPORT DgnTextStyle(DgnProjectR);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, bool&) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, bool);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, UInt32&) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, UInt32);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, Int32&) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, Int32);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, DPoint2dR) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, DPoint2dCR);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, double&) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, double);
    DGNPLATFORM_EXPORT virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty, DgnFontCP&) const;
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty, DgnFontCP);
    DGNPLATFORM_EXPORT virtual DgnTextStylePtr _Clone() const;

public:
    DGNPLATFORM_EXPORT bool ProcessProperties(PropertyContextR, bool canChange);
    DGNPLATFORM_EXPORT bool ProcessPropertiesForClone(PropertyContextR, bool applyScale);
    DGNPLATFORM_EXPORT void Scale(double);
    DGNPLATFORM_EXPORT void SetId(DgnStyleId);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT static DgnTextStylePtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT DgnTextStylePtr Clone() const;
    DGNPLATFORM_EXPORT DgnProjectR GetProject() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, bool&) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, bool);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, UInt32&) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, UInt32);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, Int32&) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, Int32);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, DPoint2dR) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, DPoint2dCR);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, double&) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, double);
    DGNPLATFORM_EXPORT BentleyStatus GetPropertyValue(DgnTextStyleProperty, DgnFontCP&) const;
    DGNPLATFORM_EXPORT BentleyStatus SetPropertyValue(DgnTextStyleProperty, DgnFontCP);
    DGNPLATFORM_EXPORT DgnTextStylePropertyMaskPtr Compare(DgnTextStyleCR) const;
    DGNPLATFORM_EXPORT void CopyPropertyValuesFrom(DgnTextStyleCR);
    DGNPLATFORM_EXPORT void CopyPropertyValuesFrom(DgnTextStyleCR, DgnTextStylePropertyMaskCR);
}; // DgnTextStyle

//__PUBLISH_SECTION_END__

//=======================================================================================
//! Utility methods to persist DgnTextStyle in various formats.
// @bsiclass                                                    Jeff.Marker     12/2013
//=======================================================================================
struct DgnTextStylePersistence
{
    //=======================================================================================
    //! @deprecated This supports older elements and file formats, and should not be used in new code.
    // @bsiclass                                                    Jeff.Marker     12/2013
    //=======================================================================================
    struct Legacy
    {
        DGNPLATFORM_EXPORT static LegacyTextStyle ToLegacyStyle(DgnTextStyleCR);
        DGNPLATFORM_EXPORT static void FromLegacyStyle(DgnTextStyleR, LegacyTextStyleCR);
        DGNPLATFORM_EXPORT static LegacyTextStyleOverrideFlags ToLegacyMask(DgnTextStylePropertyMaskCR);
        DGNPLATFORM_EXPORT static DgnTextStylePropertyMaskPtr FromLegacyMask(LegacyTextStyleOverrideFlagsCR);
        DGNPLATFORM_EXPORT static TextParamWide ToTextParamWide(DgnTextStyleCR, DPoint2dP fontSize, UInt32* maxCharactersPerLine, UInt32 fallbackFontNumber = -1, UInt32 fallbackShxBigFontNumber = -1);
        DGNPLATFORM_EXPORT static void FromTextParamWide(DgnTextStyleR, TextParamWideCR, DPoint2dCR fontSize, UInt32 maxCharactersPerLine);
        DGNPLATFORM_EXPORT static TextParamWide ToTextParamWide(LegacyTextStyleCR, DgnProjectCR, DPoint2dP fontSize, UInt32* maxCharactersPerLine);
        DGNPLATFORM_EXPORT static LegacyTextStyle FromTextParamWide(TextParamWideCR, DPoint2dCR fontSize, UInt32 maxCharactersPerLine);
    }; // Legacy

    DGNPLATFORM_EXPORT static Utf8String ToJson(DgnTextStyleCR);
    DGNPLATFORM_EXPORT static BentleyStatus FromJson(DgnTextStyleR, Utf8CP);
}; // DgnTextStylePersistence

//=======================================================================================
//! This is an internal support class. If the font IDs for the style are not valid (e.g. they reference the source project in the middle of a clone), this object stores fonts as their IDs, not as objects (like the normal DgnTextStyle class).
//! This is very limiting because the common DgnFontCP overloads of the property accessors will NOT work, but it can be useful in very limited scopes to aid in cloning or remapping.
// @bsiclass                                                    Jeff.Marker     12/2013
//=======================================================================================
struct NoFontObjectDgnTextStyle : public DgnTextStyle
{
    DEFINE_T_SUPER(DgnTextStyle)

private:
    UInt32 m_fontNumber;
    UInt32 m_shxBigFontNumber;

protected:
    virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty property, UInt32& value) const override;
    virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty property, UInt32 value) override;
    virtual BentleyStatus _GetPropertyValue(DgnTextStyleProperty property, DgnFontCP& value) const override;
    virtual BentleyStatus _SetPropertyValue(DgnTextStyleProperty property, DgnFontCP value) override;
    virtual DgnTextStylePtr _Clone() const override;

public:
    DGNPLATFORM_EXPORT NoFontObjectDgnTextStyle(DgnProjectR project);
    }; // NoFontObjectDgnTextStyle

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Groups methods for Text Styles.
// @bsiclass
//=======================================================================================
struct DgnTextStyles
{
private:
    friend struct DgnStyles;

    DgnProjectR m_project;

    //! Only the outer class is designed to construct this class.
    DgnTextStyles(DgnProjectR project) : m_project(project) {}

public:
//__PUBLISH_SECTION_END__
    //! Adds a new text style to the project, attempting to honor the provided ID. If a text style already exists by-name or by-id, no action is performed.
    //! @note This should only be used by importers that are attempting to preserve already-unique IDs.
    DGNPLATFORM_EXPORT BentleyStatus InsertWithId(DgnTextStyleCR);

    //! Determines if a text style by the ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by the name exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;
//__PUBLISH_SECTION_START__

    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT DgnTextStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT DgnTextStylePtr QueryByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT DgnTextStylePtr Insert(DgnTextStyleCR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(DgnTextStyleCR);

    //! Deletes a text style from the project. The ID in the provided style is used to identify which style to delete. If a style does not exist by-ID, no action is performed.
    //! @note When a text style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

    //! Creates an iterator to iterate available text styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
