/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameStyle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationFrameStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! This enumerates all possible annotation frame types.
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
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationFrameStylePropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    DGNPLATFORM_EXPORT virtual bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT virtual bool _IsRealProperty(T_Key) const override;

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

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationFrame.
//! @see DgnStyles::AnnotationFrameStyles for persistence
//! @note When creating an AnnotationFrame, the typical work flow is to create and store the style, and then create the AnnotationFrame with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameStyle : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationFrameStylePersistence;

    DgnDbP m_dgndb;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationFrameStylePropertyBag m_data;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFrameStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameStyle(DgnDbR);
    AnnotationFrameStyle(AnnotationFrameStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFrameStyleR operator=(AnnotationFrameStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFrameStylePtr Create(DgnDbR project) { return new AnnotationFrameStyle(project); }
    AnnotationFrameStylePtr Clone() const { return new AnnotationFrameStyle(*this); }
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return *m_dgndb; }
    DgnStyleId GetId() const { return m_id; }
    void SetId(DgnStyleId value) { m_id = value; } //!< @private
    Utf8StringCR GetName() const { return m_name; }
    void SetName(Utf8CP value) { m_name = value; }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description = value; }

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
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnAnnotationFrameStyles : public DgnDbTable
{
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;
    
    DgnAnnotationFrameStyles(DgnDbR db) : T_Super(db) {}

public:
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
            DgnStyleId GetId() const { Verify(); return m_sql->GetValueId<DgnStyleId>(0); }
            Utf8CP GetName() const { Verify(); return m_sql->GetValueText(1); }
            Utf8CP GetDescription() const { Verify(); return m_sql->GetValueText(2); }
            Entry const& operator* () const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(NULL, false); }
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    };
    
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
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
