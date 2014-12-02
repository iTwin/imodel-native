//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderStyle.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationPropertyBag.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationLeaderStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStyle);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderLineType
{
    None = 1,
    Straight = 2,
    Curved = 3

}; // AnnotationLeaderLineType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderTerminatorType
{
    None = 1,
    OpenArrow = 2,
    ClosedArrow = 3

}; // AnnotationLeaderTerminatorType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderStyleProperty
{
    LineColorId = 1, //<! (integer) @note Must be a valid color ID in the project
    LineStyle = 2, //<! (integer) @note Must be a standard line code
    LineType = 3, //<! (integer) @note Must exist in the AnnotationLeaderLineType enumeration
    LineWeight = 4, //<! (integer) @note Must be a standard line weight
    TerminatorColorId = 5, //<! (integer) @note Must be a valid color ID in the project
    TerminatorScaleFactor = 6, //<! (real) @note Generally describes the length of the side of the box encompassing the terminator, as a factor of the first character's text height
    TerminatorStyle = 7, //<! (integer) @note Must be a standard line code
    TerminatorType = 8, //<! (integer) @note Must exist in the AnnotationLeaderTerminatorType enumeration
    TerminatorWeight = 9 //<! (integer) @note Must be a standard line weight

}; // AnnotationLeaderStyleProperty

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderStylePropertyBag : public AnnotationPropertyBag
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    virtual bool _IsIntegerProperty(T_Key) const override;
    virtual bool _IsRealProperty(T_Key) const override;

public:
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBag();
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBag(AnnotationLeaderStylePropertyBagCR);
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagR operator=(AnnotationLeaderStylePropertyBagCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderStylePropertyBagPtr Create();
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagPtr Clone() const;
    
    DGNPLATFORM_EXPORT bool HasProperty(AnnotationLeaderStyleProperty) const;
    DGNPLATFORM_EXPORT void ClearProperty(AnnotationLeaderStyleProperty);
    DGNPLATFORM_EXPORT T_Integer GetIntegerProperty(AnnotationLeaderStyleProperty) const;
    DGNPLATFORM_EXPORT void SetIntegerProperty(AnnotationLeaderStyleProperty, T_Integer);
    DGNPLATFORM_EXPORT T_Real GetRealProperty(AnnotationLeaderStyleProperty) const;
    DGNPLATFORM_EXPORT void SetRealProperty(AnnotationLeaderStyleProperty, T_Real);

}; // AnnotationLeaderStylePropertyBag

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationLeaderStylePersistence;

    DgnProjectP m_project;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationLeaderStylePropertyBag m_data;

    void CopyFrom(AnnotationLeaderStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeaderStyle(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationLeaderStyle(AnnotationLeaderStyleCR);
    DGNPLATFORM_EXPORT AnnotationLeaderStyleR operator=(AnnotationLeaderStyleCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderStylePtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr Clone() const;
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT UInt32 GetLineColorId() const;
    DGNPLATFORM_EXPORT void SetLineColorId(UInt32);
    DGNPLATFORM_EXPORT Int32 GetLineStyle() const;
    DGNPLATFORM_EXPORT void SetLineStyle(Int32);
    DGNPLATFORM_EXPORT AnnotationLeaderLineType GetLineType() const;
    DGNPLATFORM_EXPORT void SetLineType(AnnotationLeaderLineType);
    DGNPLATFORM_EXPORT UInt32 GetLineWeight() const;
    DGNPLATFORM_EXPORT void SetLineWeight(UInt32);
    DGNPLATFORM_EXPORT UInt32 GetTerminatorColorId() const;
    DGNPLATFORM_EXPORT void SetTerminatorColorId(UInt32);
    DGNPLATFORM_EXPORT double GetTerminatorScaleFactor() const;
    DGNPLATFORM_EXPORT void SetTerminatorScaleFactor(double);
    DGNPLATFORM_EXPORT Int32 GetTerminatorStyle() const;
    DGNPLATFORM_EXPORT void SetTerminatorStyle(Int32);
    DGNPLATFORM_EXPORT AnnotationLeaderTerminatorType GetTerminatorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorType(AnnotationLeaderTerminatorType);
    DGNPLATFORM_EXPORT UInt32 GetTerminatorWeight() const;
    DGNPLATFORM_EXPORT void SetTerminatorWeight(UInt32);

}; // AnnotationLeaderStyle

//=======================================================================================
//! Groups methods for AnnotationLeaderStyle.
// @bsiclass
//=======================================================================================
struct DgnAnnotationLeaderStyles
{
private:
    friend struct DgnStyles;

    DgnProjectR m_project;

    // Only the outer class is designed to construct this class.
    DgnAnnotationLeaderStyles(DgnProjectR project) : m_project(project) {}

public:
    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr QueryByName(Utf8CP) const;

    //! Determines if a text style by the ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by the name exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr Insert(AnnotationLeaderStyleCR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationLeaderStyleCR);

    //! Deletes a text style from the project. The ID in the provided style is used to identify which style to delete. If a style does not exist by-ID, no action is performed.
    //! @note When a text style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

    //! Creates an iterator to iterate available text styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
