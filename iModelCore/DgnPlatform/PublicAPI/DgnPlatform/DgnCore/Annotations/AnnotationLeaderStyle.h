//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderStyle.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! This enumerates all possible annotation leader line types.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderLineType
{
    None = 1,
    Straight = 2,
    Curved = 3
};

//=======================================================================================
//! This enumerates all possible annotation leader terminator types.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderTerminatorType
{
    None = 1,
    OpenArrow = 2,
    ClosedArrow = 3
};

//=======================================================================================
//! This enumerates all possible AnnotationLeaderStyle property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationLeaderStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationLeaderStylePropertyBag and AnnotationLeaderStyleProperty.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderStyleProperty
{
    LineColorId = 1, //!< (integer) @note Must be a valid color ID in the project
    LineStyle = 2, //!< (integer) @note Must be a standard line code
    LineType = 3, //!< (integer) @note Must exist in the AnnotationLeaderLineType enumeration
    LineWeight = 4, //!< (integer) @note Must be a standard line weight
    TerminatorColorId = 5, //!< (integer) @note Must be a valid color ID in the project
    TerminatorScaleFactor = 6, //!< (real) @note Generally describes the length of the side of the box encompassing the terminator, as a factor of the first character's text height
    TerminatorStyle = 7, //!< (integer) @note Must be a standard line code
    TerminatorType = 8, //!< (integer) @note Must exist in the AnnotationLeaderTerminatorType enumeration
    TerminatorWeight = 9 //!< (integer) @note Must be a standard line weight
};

//=======================================================================================
//! This specialized collection provides direct access to AnnotationLeaderStyle property keys and values.
//! Unlike the higher-level AnnotationLeaderStyle, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The AnnotationLeaderStyleProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While AnnotationLeaderStyle provides high-level accessors to its properties, overrides are expressed directly via AnnotationLeaderStylePropertyBag and AnnotationLeaderStyleProperty.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationLeaderStylePropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    DGNPLATFORM_EXPORT virtual bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT virtual bool _IsRealProperty(T_Key) const override;

public:
    AnnotationLeaderStylePropertyBag() : T_Super() {}
    AnnotationLeaderStylePropertyBag(AnnotationLeaderStylePropertyBagCR rhs) : T_Super(rhs) {}
    AnnotationLeaderStylePropertyBagR operator=(AnnotationLeaderStylePropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}
    static AnnotationLeaderStylePropertyBagPtr Create() { return new AnnotationLeaderStylePropertyBag(); }
    AnnotationLeaderStylePropertyBagPtr Clone() const { return new AnnotationLeaderStylePropertyBag(*this); }
    
    bool HasProperty(AnnotationLeaderStyleProperty key) const { return T_Super::HasProperty((T_Key)key); }
    void ClearProperty(AnnotationLeaderStyleProperty key) { T_Super::ClearProperty((T_Key)key); }
    T_Integer GetIntegerProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
    void SetIntegerProperty(AnnotationLeaderStyleProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
    T_Real GetRealProperty(AnnotationLeaderStyleProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
    void SetRealProperty(AnnotationLeaderStyleProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }
};

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationLeader.
//! @see DgnStyles::AnnotationLeaderStyles for persistence
//! @note When creating an AnnotationLeader, the typical work flow is to create and store the style, and then create the AnnotationLeader with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderStyle : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationLeaderStylePersistence;

    DgnDbP m_dgndb;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    AnnotationLeaderStylePropertyBag m_data;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLeaderStyleCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeaderStyle(DgnDbR);
    AnnotationLeaderStyle(AnnotationLeaderStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLeaderStyleR operator=(AnnotationLeaderStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLeaderStylePtr Create(DgnDbR project) { return new AnnotationLeaderStyle(project); }
    AnnotationLeaderStylePtr Clone() const { return new AnnotationLeaderStyle(*this); }
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return *m_dgndb; }
    DgnStyleId GetId() const { return m_id; }
    void SetId(DgnStyleId value) { m_id = value; } //!< @private
    Utf8StringCR GetName() const { return m_name; }
    void SetName(Utf8CP value) { m_name = value; }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description = value; }

    DGNPLATFORM_EXPORT ColorDef GetLineColor() const;
    DGNPLATFORM_EXPORT void SetLineColor(ColorDef);
    DGNPLATFORM_EXPORT int32_t GetLineStyle() const;
    DGNPLATFORM_EXPORT void SetLineStyle(int32_t);
    DGNPLATFORM_EXPORT AnnotationLeaderLineType GetLineType() const;
    DGNPLATFORM_EXPORT void SetLineType(AnnotationLeaderLineType);
    DGNPLATFORM_EXPORT uint32_t GetLineWeight() const;
    DGNPLATFORM_EXPORT void SetLineWeight(uint32_t);
    DGNPLATFORM_EXPORT ColorDef GetTerminatorColor() const;
    DGNPLATFORM_EXPORT void SetTerminatorColor(ColorDef);
    DGNPLATFORM_EXPORT double GetTerminatorScaleFactor() const;
    DGNPLATFORM_EXPORT void SetTerminatorScaleFactor(double);
    DGNPLATFORM_EXPORT int32_t GetTerminatorStyle() const;
    DGNPLATFORM_EXPORT void SetTerminatorStyle(int32_t);
    DGNPLATFORM_EXPORT AnnotationLeaderTerminatorType GetTerminatorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorType(AnnotationLeaderTerminatorType);
    DGNPLATFORM_EXPORT uint32_t GetTerminatorWeight() const;
    DGNPLATFORM_EXPORT void SetTerminatorWeight(uint32_t);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnAnnotationLeaderStyles : public DgnDbTable
{
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;
    
    DgnAnnotationLeaderStyles(DgnDbR db) : T_Super(db) {}

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
    
    //! Queries the project for a leader style by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr QueryById(DgnStyleId) const;

    //! Queries the project for a leader style by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr QueryByName(Utf8CP) const;

    //! Creates an iterator to iterate available leader styles.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Determines if a leader style by-ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryById if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if a leader style by-name exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryByName if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new leader style to the project. The ID in the provided style is ignored during insert, but is updated to the new ID when the method returns. If a style already exists by-name, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Insert(AnnotationLeaderStyleR);

    //! Updates a leader style in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(AnnotationLeaderStyleCR);

    //! Deletes a leader style from the project. If a style does not exist by-ID, no action is performed.
    //! @note When a style is removed, no attempts are currently made to normalize existing elements. Thus, elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
