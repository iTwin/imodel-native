//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameStyle.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! This enumerates all possible annotation frame types.
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
//! This enumerates all possible AnnotationFrameStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationFrameStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationFrameStylePropertyBag and AnnotationFrameStyleProperty.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationFrameStyleProperty
{
    CloudBulgeFactor = 1, //!< (real) @note Factor of effective diameter
    CloudDiameterFactor = 2, //!< (real) @note Factor of text height
    FillColorId = 3, //!< (integer) @note Must be a valid color ID in the project
    FillTransparency = 4, //!< (real) @note [0.0..1.0]
    HorizontalPadding = 5, //!< (real) @note Factor of the first character's text height
    IsFillEnabled = 6, //!< (integer) @note 0 or 1 boolean
    IsStrokeCloud = 7, //!< (integer) @note 0 or 1 boolean
    IsStrokeEnabled = 8, //!< (integer) @note 0 or 1 boolean
    StrokeColorId = 9, //!< (integer) @note Must be a valid color ID in the project
    StrokeStyle = 10, //!< (integer) @note Must be a standard line code
    StrokeWeight = 11, //!< (integer) @note Must be a standard line weight
    Type = 12, //!< (integer) @note Must exist in the AnnotationFrameType enumeration
    VerticalPadding = 13 //!< (real) @note Factor of the first character's text height

}; // AnnotationFrameStyleProperty

//=======================================================================================
//! This specialized collection provides direct access to AnnotationFrameStyle property keys and values.
//! Unlike the higher-level AnnotationFrameStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationFrameStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationFrameStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationFrameStylePropertyBag and AnnotationFrameStyleProperty.
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
//! This is used to provide style properties when creating an AnnotationFrame.
//! @see DgnStyles::AnnotationFrameStyles for persistence
//! @note When creating an AnnotationFrame, the typical work flow is to create and store the style, and then create the AnnotationFrame with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationFrameStylePersistence;

    DgnDbP m_dgndb;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationFrameStylePropertyBag m_data;

    void CopyFrom(AnnotationFrameStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameStyle(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationFrameStyle(AnnotationFrameStyleCR);
    DGNPLATFORM_EXPORT AnnotationFrameStyleR operator=(AnnotationFrameStyleCR);
    
    void SetId(DgnStyleId);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFrameStylePtr Create(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr Clone() const;
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetId() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetName() const;
    DGNPLATFORM_EXPORT void SetName(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    DGNPLATFORM_EXPORT void SetDescription(Utf8CP);

    DGNPLATFORM_EXPORT double GetCloudBulgeFactor() const;
    DGNPLATFORM_EXPORT void SetCloudBulgeFactor(double);
    DGNPLATFORM_EXPORT double GetCloudDiameterFactor() const;
    DGNPLATFORM_EXPORT void SetCloudDiameterFactor(double);
    DGNPLATFORM_EXPORT ColorDef GetFillColor() const;
    DGNPLATFORM_EXPORT void SetFillColor(ColorDef);
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
    DGNPLATFORM_EXPORT ColorDef GetStrokeColor() const;
    DGNPLATFORM_EXPORT void SetStrokeColor(ColorDef);
    DGNPLATFORM_EXPORT int32_t GetStrokeStyle() const;
    DGNPLATFORM_EXPORT void SetStrokeStyle(int32_t);
    DGNPLATFORM_EXPORT uint32_t GetStrokeWeight() const;
    DGNPLATFORM_EXPORT void SetStrokeWeight(uint32_t);
    DGNPLATFORM_EXPORT AnnotationFrameType GetType() const;
    DGNPLATFORM_EXPORT void SetType(AnnotationFrameType);
    DGNPLATFORM_EXPORT double GetVerticalPadding() const;
    DGNPLATFORM_EXPORT void SetVerticalPadding(double);

    DGNPLATFORM_EXPORT void SetPadding(double);

}; // AnnotationFrameStyle

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnAnnotationFrameStyles : public DgnDbTable
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;
    
    DgnAnnotationFrameStyles(DgnDbR db) : T_Super(db) {}

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
    
    //! Queries the project for a frame style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a frame style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr QueryByName(Utf8CP) const;

    //! Creates an iterator to iterate available frame styles.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Determines if a frame style by-ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryById if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a frame style by-name exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryByName if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new frame style to the project. The ID in the provided style is ignored during insert, but is updated to the new ID when the method returns. If a style already exists by-name, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Insert(AnnotationFrameStyleR);

    //! Updates a frame style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationFrameStyleCR);

    //! Deletes a frame style from the project. If a style does not exist by-ID, no action is performed.
    //! @note When a style is removed, no attempts are currently made to normalize existing elements. Thus, elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);

}; // DgnAnnotationFrameStyles

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
