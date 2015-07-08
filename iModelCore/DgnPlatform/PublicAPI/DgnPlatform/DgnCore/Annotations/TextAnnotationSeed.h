/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationSeed.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
//! This enumerates all possible TextAnnotationSeed property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While TextAnnotationSeed provides high-level accessors to its properties, overrides are expressed directly via TextAnnotationSeedPropertyBag and TextAnnotationSeedProperty.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
enum class TextAnnotationSeedProperty
{
    FrameStyleId = 1, //<! (integer) @note ID of an AnnotationFrameStyle in the project
    LeaderStyleId = 2, //<! (integer) @note ID of an AnnotationLeaderStyle in the project
    TextStyleId = 3 //<! (integer) @note ID of an AnnotationTextStyle in the project
};

//=======================================================================================
//! This specialized collection provides direct access to TextAnnotationSeed property keys and values.
//! Unlike the higher-level TextAnnotationSeed, this collection deals directly with property keys and their underlying values. You must know a property's data type when using this class. The TextAnnotationSeedProperty enumeration describes each property's data type.
//! When created, this collection has no properties in it; their values are assumed to be default. In other words, this only stores deltas from defaults. In the case of overrides, it only stores the properties that are overridden, even if overridden with the same value.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While TextAnnotationSeed provides high-level accessors to its properties, overrides are expressed directly via TextAnnotationSeedPropertyBag and TextAnnotationSeedProperty.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationSeedPropertyBag : AnnotationPropertyBag
{
private:
    DEFINE_T_SUPER(AnnotationPropertyBag)
    
protected:
    DGNPLATFORM_EXPORT virtual bool _IsIntegerProperty(T_Key) const override;
    DGNPLATFORM_EXPORT virtual bool _IsRealProperty(T_Key) const override;

public:
    TextAnnotationSeedPropertyBag() : T_Super() {}
    TextAnnotationSeedPropertyBag(TextAnnotationSeedPropertyBagCR rhs) : T_Super(rhs) {}
    TextAnnotationSeedPropertyBagR operator=(TextAnnotationSeedPropertyBagCR rhs) { T_Super::operator=(rhs); return *this;}
    static TextAnnotationSeedPropertyBagPtr Create() { return new TextAnnotationSeedPropertyBag(); }
    TextAnnotationSeedPropertyBagPtr Clone() const { return new TextAnnotationSeedPropertyBag(*this); }
    
    bool HasProperty(TextAnnotationSeedProperty key) const { return T_Super::HasProperty((T_Key)key); }
    void ClearProperty(TextAnnotationSeedProperty key) { T_Super::ClearProperty((T_Key)key); }
    T_Integer GetIntegerProperty(TextAnnotationSeedProperty key) const { return T_Super::GetIntegerProperty((T_Key)key); }
    void SetIntegerProperty(TextAnnotationSeedProperty key, T_Integer value) { T_Super::SetIntegerProperty((T_Key)key, value); }
    T_Real GetRealProperty(TextAnnotationSeedProperty key) const { return T_Super::GetRealProperty((T_Key)key); }
    void SetRealProperty(TextAnnotationSeedProperty key, T_Real value) { T_Super::SetRealProperty((T_Key)key, value); }
};

//=======================================================================================
//! This is used to provide seed properties when creating a TextAnnotation. Unlike a classic "style", a "seed" is only used when creating the element. Once created, elements will not react to changes in the seed.
//! @see DgnStyles::TextAnnotationSeeds for persistence
//! @note To be valid, a seed must have all 3 style properties set. When created from scratch, all 3 styles are invalid and must therefore be set.
//! @note When creating a TextAnnotation, the typical work flow is to create and store the required styles for a seed, create and store the seed, and then create the TextAnnotation with the stored seed's ID.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationSeed : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct TextAnnotationSeedPersistence;

    DgnDbP m_dgndb;
    DgnStyleId m_id;
    Utf8String m_name;
    Utf8String m_description;
    TextAnnotationSeedPropertyBag m_data;

    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationSeedCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit TextAnnotationSeed(DgnDbR);
    TextAnnotationSeed(TextAnnotationSeedCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextAnnotationSeedR operator=(TextAnnotationSeedCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static TextAnnotationSeedPtr Create(DgnDbR project) { return new TextAnnotationSeed(project); }
    TextAnnotationSeedPtr Clone() const { return new TextAnnotationSeed(*this); }
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr CreateEffectiveStyle(TextAnnotationSeedPropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return *m_dgndb; }
    DgnStyleId GetId() const { return m_id; }
    void SetId(DgnStyleId value) { m_id = value; } //!< @private
    Utf8StringCR GetName() const { return m_name; }
    void SetName(Utf8CP value) { m_name = value; }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description = value; }

    DGNPLATFORM_EXPORT DgnStyleId GetFrameStyleId() const;
    DGNPLATFORM_EXPORT void SetFrameStyleId(DgnStyleId);
    DGNPLATFORM_EXPORT DgnStyleId GetLeaderStyleId() const;
    DGNPLATFORM_EXPORT void SetLeaderStyleId(DgnStyleId);
    DGNPLATFORM_EXPORT DgnStyleId GetTextStyleId() const;
    DGNPLATFORM_EXPORT void SetTextStyleId(DgnStyleId);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnTextAnnotationSeeds : public DgnDbTable
{
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;
    
    DgnTextAnnotationSeeds(DgnDbR db) : T_Super(db) {}

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
    
    //! Queries the project for an annotation seed by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr QueryById(DgnStyleId) const;

    //! Queries the project for an annotation seed by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT TextAnnotationSeedPtr QueryByName(Utf8CP) const;

    //! Creates an iterator to iterate available annotation seeds.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Determines if an annotation seed by-ID exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryById if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsById(DgnStyleId) const;

    //! Determines if an annotation seed by-name exists in the project.
    //! @note This does not attempt to deserialize the style into an object, and is thus faster than QueryByName if you just want to check existence.
    DGNPLATFORM_EXPORT bool ExistsByName(Utf8CP) const;

    //! Adds a new annotation seed to the project. The ID in the provided style is ignored during insert, but is updated to the new ID when the method returns. If a style already exists by-name, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Insert(TextAnnotationSeedR);

    //! Updates an annotation seed in the project. The ID in the provided style is used to identify which style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Update(TextAnnotationSeedCR);

    //! Deletes an annotation seed from the project. If a style does not exist by-ID, no action is performed.
    //! @note When a style is removed, no attempts are currently made to normalize existing elements. Thus, elements may still attempt to reference a missing style, and must be written to assume such a style doesn't exist.
    DGNPLATFORM_EXPORT BentleyStatus Delete(DgnStyleId);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
