//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameStyle.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationPropertyBag.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationFrameStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStyle);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationFrameType
{
    InvisibleBox = 1,
    Box = 2,
    Circle = 3,
    Ellipse = 4

}; // AnnotationFrameType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationFrameStyleProperty
{
    CloudBulgeFactor = 1, //<! (real) @note Factor of effective diameter
    CloudDiameterFactor = 2, //<! (real) @note Factor of text height
    FillColorId = 3, //<! (integer) @note Must be a valid color ID in the project
    FillTransparency = 4, //<! (real) @note [0.0..1.0]
    HorizontalPadding = 5, //<! (real) @note Factor of the first character's text height
    IsFillEnabled = 6, //<! (integer) @note 0 or 1 boolean
    IsStrokeCloud = 7, //<! (integer) @note 0 or 1 boolean
    IsStrokeEnabled = 8, //<! (integer) @note 0 or 1 boolean
    StrokeColorId = 9, //<! (integer) @note Must be a valid color ID in the project
    StrokeStyle = 10, //<! (integer) @note Must be a standard line code
    StrokeWeight = 11, //<! (integer) @note Must be a standard line weight
    Type = 12, //<! (integer) @note Must exist in the AnnotationFrameType enumeration
    VerticalPadding = 13 //<! (real) @note Factor of the first character's text height

}; // AnnotationFrameStyleProperty

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameStylePropertyBag : public AnnotationPropertyBag
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    virtual bool _IsIntegerProperty(T_Key) const override;
    virtual bool _IsRealProperty(T_Key) const override;

public:
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBag();
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBag(AnnotationFrameStylePropertyBagCR);
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBagR operator=(AnnotationFrameStylePropertyBagCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFrameStylePropertyBagPtr Create();
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBagPtr Clone() const;
    
    DGNPLATFORM_EXPORT bool HasProperty(AnnotationFrameStyleProperty) const;
    DGNPLATFORM_EXPORT void ClearProperty(AnnotationFrameStyleProperty);
    DGNPLATFORM_EXPORT T_Integer GetIntegerProperty(AnnotationFrameStyleProperty) const;
    DGNPLATFORM_EXPORT void SetIntegerProperty(AnnotationFrameStyleProperty, T_Integer);
    DGNPLATFORM_EXPORT T_Real GetRealProperty(AnnotationFrameStyleProperty) const;
    DGNPLATFORM_EXPORT void SetRealProperty(AnnotationFrameStyleProperty, T_Real);

}; // AnnotationFrameStylePropertyBag

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationFrameStylePersistence;

    DgnProjectP m_project;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationFrameStylePropertyBag m_data;

    void CopyFrom(AnnotationFrameStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameStyle(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationFrameStyle(AnnotationFrameStyleCR);
    DGNPLATFORM_EXPORT AnnotationFrameStyleR operator=(AnnotationFrameStyleCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFrameStylePtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr Clone() const;
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT double GetCloudBulgeFactor() const;
    DGNPLATFORM_EXPORT void SetCloudBulgeFactor(double);
    DGNPLATFORM_EXPORT double GetCloudDiameterFactor() const;
    DGNPLATFORM_EXPORT void SetCloudDiameterFactor(double);
    DGNPLATFORM_EXPORT UInt32 GetFillColorId() const;
    DGNPLATFORM_EXPORT void SetFillColorId(UInt32);
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
    DGNPLATFORM_EXPORT UInt32 GetStrokeColorId() const;
    DGNPLATFORM_EXPORT void SetStrokeColorId(UInt32);
    DGNPLATFORM_EXPORT Int32 GetStrokeStyle() const;
    DGNPLATFORM_EXPORT void SetStrokeStyle(Int32);
    DGNPLATFORM_EXPORT UInt32 GetStrokeWeight() const;
    DGNPLATFORM_EXPORT void SetStrokeWeight(UInt32);
    DGNPLATFORM_EXPORT AnnotationFrameType GetType() const;
    DGNPLATFORM_EXPORT void SetType(AnnotationFrameType);
    DGNPLATFORM_EXPORT double GetVerticalPadding() const;
    DGNPLATFORM_EXPORT void SetVerticalPadding(double);

    DGNPLATFORM_EXPORT void SetPadding(double);

}; // AnnotationFrameStyle

//=======================================================================================
//! Groups methods for AnnotationFrameStyle.
// @bsiclass
//=======================================================================================
struct DgnAnnotationFrameStyles
{
private:
    friend struct DgnStyles;

    DgnProjectR m_project;

    // Only the outer class is designed to construct this class.
    DgnAnnotationFrameStyles(DgnProjectR project) : m_project(project) {}

public:
    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr QueryByName(Utf8CP) const;

    //! Determines if a text style by the ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by the name exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr Insert(AnnotationFrameStyleCR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationFrameStyleCR);

    //! Deletes a text style from the project. The ID in the provided style is used to identify which style to delete. If a style does not exist by-ID, no action is performed.
    //! @note When a text style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

    //! Creates an iterator to iterate available text styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;

};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
