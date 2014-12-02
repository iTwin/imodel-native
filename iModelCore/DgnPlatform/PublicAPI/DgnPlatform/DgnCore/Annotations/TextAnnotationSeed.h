//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationSeed.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationPropertyBag.h"

DGNPLATFORM_TYPEDEFS(TextAnnotationSeedPropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationSeedPropertyBag);
DGNPLATFORM_TYPEDEFS(TextAnnotationSeed);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationSeed);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
enum struct TextAnnotationSeedProperty
{
    FrameStyleId = 1, //<! (integer) @note ID of an AnnotationFrameStyle in the project
    LeaderStyleId = 2, //<! (integer) @note ID of an AnnotationLeaderStyle in the project
    TextStyleId = 3 //<! (integer) @note ID of an AnnotationTextStyle in the project

}; // TextAnnotationSeedProperty

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationSeedPropertyBag : public AnnotationPropertyBag
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    virtual bool _IsIntegerProperty(T_Key) const override;
    virtual bool _IsRealProperty(T_Key) const override;

public:
    DGNPLATFORM_EXPORT TextAnnotationSeedPropertyBag();
    DGNPLATFORM_EXPORT TextAnnotationSeedPropertyBag(TextAnnotationSeedPropertyBagCR);
    DGNPLATFORM_EXPORT TextAnnotationSeedPropertyBagR operator=(TextAnnotationSeedPropertyBagCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextAnnotationSeedPropertyBagPtr Create();
    DGNPLATFORM_EXPORT TextAnnotationSeedPropertyBagPtr Clone() const;
    
    DGNPLATFORM_EXPORT bool HasProperty(TextAnnotationSeedProperty) const;
    DGNPLATFORM_EXPORT void ClearProperty(TextAnnotationSeedProperty);
    DGNPLATFORM_EXPORT T_Integer GetIntegerProperty(TextAnnotationSeedProperty) const;
    DGNPLATFORM_EXPORT void SetIntegerProperty(TextAnnotationSeedProperty, T_Integer);
    DGNPLATFORM_EXPORT T_Real GetRealProperty(TextAnnotationSeedProperty) const;
    DGNPLATFORM_EXPORT void SetRealProperty(TextAnnotationSeedProperty, T_Real);

}; // TextAnnotationSeedPropertyBag

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationSeed : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct TextAnnotationSeedPersistence;

    DgnProjectP m_project;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    TextAnnotationSeedPropertyBag m_data;

    void CopyFrom(TextAnnotationSeedCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit TextAnnotationSeed(DgnProjectR);
    DGNPLATFORM_EXPORT TextAnnotationSeed(TextAnnotationSeedCR);
    DGNPLATFORM_EXPORT TextAnnotationSeedR operator=(TextAnnotationSeedCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextAnnotationSeedPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr Clone() const;
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr CreateEffectiveStyle(TextAnnotationSeedPropertyBagCR overrides) const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT DgnStyleId GetFrameStyleId() const;
    DGNPLATFORM_EXPORT void SetFrameStyleId(DgnStyleId);
    DGNPLATFORM_EXPORT DgnStyleId GetLeaderStyleId() const;
    DGNPLATFORM_EXPORT void SetLeaderStyleId(DgnStyleId);
    DGNPLATFORM_EXPORT DgnStyleId GetTextStyleId() const;
    DGNPLATFORM_EXPORT void SetTextStyleId(DgnStyleId);

}; // TextAnnotationSeed

//=======================================================================================
//! Groups methods for TextAnnotationSeed.
// @bsiclass
//=======================================================================================
struct DgnTextAnnotationSeeds
{
private:
    friend struct DgnStyles;
    
    DgnProjectR m_project;

    // Only the outer class is designed to construct this class.
    DgnTextAnnotationSeeds(DgnProjectR project) : m_project(project) {}

public:
    //! Queries the project for a text style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr QueryById(DgnStyleId) const;

    //! Queries the project for a text style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr QueryByName(Utf8CP) const;
    
    //! Determines if a text style by the ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a text style by the name exists in the project.
    //! @note This does not attempt to deserialize the style into an object.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new text style to the project. The ID in the provided style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr Insert(TextAnnotationSeedCR);

    //! Updates a text style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(TextAnnotationSeedCR);

    //! Deletes a text style from the project. The ID in the provided style is used to identify which style to delete. If a style does not exist by-ID, no action is performed.
    //! @note When a text style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

    //! Creates an iterator to iterate available text styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;

}; // DgnTextAnnotationSeeds

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
