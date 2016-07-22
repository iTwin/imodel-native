/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/Annotations/TextAnnotationSeed.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(TextAnnotationSeedPropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationSeedPropertyBag);
DGNPLATFORM_TYPEDEFS(TextAnnotationSeed);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationSeed);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible TextAnnotationSeed property keys.
//! @note Unless dealing with style overrides, you will not typically use this enumeration directly. While TextAnnotationSeed provides high-level accessors to its properties, overrides are expressed directly via TextAnnotationSeedPropertyBag and TextAnnotationSeedProperty.
//! @ingroup GROUP_Annotation
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
//! @ingroup GROUP_Annotation
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

//! As an element, TextAnnotationSeed IDs are inherently DgnElementId, but create a typedef so that argument types are more obvious/natural.
//! @ingroup GROUP_Annotation
typedef DgnElementId TextAnnotationSeedId;

//=======================================================================================
//! This is used to provide seed properties when creating a TextAnnotation. Unlike a classic "style", a "seed" is only used when creating the element. Once created, elements will not react to changes in the seed.
//! @note To be valid, a seed must have all 3 style properties set. When created from scratch, all 3 styles are invalid and must therefore be set.
//! @note When creating a TextAnnotation, the typical work flow is to create and store the required styles for a seed, create and store the seed, and then create the TextAnnotation with the stored seed's ID.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationSeed : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationSeed, DefinitionElement);

private:
    friend struct TextAnnotationSeedPersistence;

    Utf8String m_description;
    TextAnnotationSeedPropertyBag m_data;

    DGNPLATFORM_EXPORT static DgnCode CreateCodeFromName(Utf8StringCR name) { return ResourceAuthority::CreateResourceCode(name, DGN_CLASSNAME_TextAnnotationSeed); }

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    virtual DgnDbStatus _OnDelete() const override { return DgnDbStatus::DeletionProhibited; /* Must be "purged" */ }
    virtual uint32_t _GetMemSize() const override { return (uint32_t)(m_description.size() + 1 + m_data.GetMemSize()); }
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override { return ResourceAuthority::IsResourceAuthority(auth); }

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationSeed); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    explicit TextAnnotationSeed(DgnDbR db) : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryDgnClassId(db), DgnCode())) {}
    explicit TextAnnotationSeed(CreateParams const& params) : T_Super(params) {}
    static TextAnnotationSeedPtr Create(DgnDbR project) { return new TextAnnotationSeed(project); }
    TextAnnotationSeedPtr CreateCopy() const { return MakeCopy<TextAnnotationSeed>(); }

    Utf8String GetName() const { return GetCode().GetValue(); }
    void SetName(Utf8CP value) { T_Super::_SetCode(CreateCodeFromName(value)); /* Only SetName is allowed to SetCode. */ }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }

    DGNPLATFORM_EXPORT DgnElementId GetFrameStyleId() const;
    DGNPLATFORM_EXPORT void SetFrameStyleId(DgnElementId);
    DGNPLATFORM_EXPORT DgnElementId GetLeaderStyleId() const;
    DGNPLATFORM_EXPORT void SetLeaderStyleId(DgnElementId);
    DGNPLATFORM_EXPORT DgnElementId GetTextStyleId() const;
    DGNPLATFORM_EXPORT void SetTextStyleId(DgnElementId);

    DGNPLATFORM_EXPORT TextAnnotationSeedPtr CreateEffectiveSeed(TextAnnotationSeedPropertyBagCR overrides) const;

    static DgnElementId QueryId(DgnDbR db, Utf8CP name) { return db.Elements().QueryElementIdByCode(CreateCodeFromName(name)); }
    static TextAnnotationSeedCPtr Get(DgnDbR db, Utf8CP name) { return Get(db, QueryId(db, name)); }
    static TextAnnotationSeedCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<TextAnnotationSeed>(id); }
    static TextAnnotationSeedPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<TextAnnotationSeed>(id); }
    static TextAnnotationSeedPtr GetForEdit(DgnDbR db, Utf8CP name) { return GetForEdit(db, QueryId(db, name)); }
    TextAnnotationSeedCPtr Insert() { return GetDgnDb().Elements().Insert<TextAnnotationSeed>(*this); }
    TextAnnotationSeedCPtr Update() { return GetDgnDb().Elements().Update<TextAnnotationSeed>(*this); }

    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass                                                    Jeff.Marker     11/2014
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
        {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct TextAnnotationSeed;

        private:
            Entry() : T_Super(nullptr) {}
            Entry(BeSQLite::EC::ECSqlStatement* stmt) : T_Super(stmt) {}

        public:
            DgnElementId GetElementId() const { return m_statement->GetValueId<DgnElementId>(0); }
            Utf8CP GetName() const { return m_statement->GetValueText(1); }
            Utf8CP GetDescription() const { return m_statement->GetValueText(2); }
        };

    typedef ECSqlStatementIterator<Entry> Iterator;

    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR);
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR);
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for text annotation seeds
    //=======================================================================================
    struct TextAnnotationSeedHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationSeed, TextAnnotationSeed, TextAnnotationSeedHandler, Element, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGN_NAMESPACE
