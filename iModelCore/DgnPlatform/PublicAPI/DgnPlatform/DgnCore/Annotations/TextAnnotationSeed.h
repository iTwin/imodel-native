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
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/ElementHandler.h>
#include <DgnPlatform/DgnCore/ECSqlStatementIterator.h>

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
//! @note To be valid, a seed must have all 3 style properties set. When created from scratch, all 3 styles are invalid and must therefore be set.
//! @note When creating a TextAnnotation, the typical work flow is to create and store the required styles for a seed, create and store the seed, and then create the TextAnnotation with the stored seed's ID.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextAnnotationSeed : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationSeed, DictionaryElement);
public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(TextAnnotationSeed::T_Super::CreateParams);

        TextAnnotationSeedPropertyBag m_data;
        Utf8String m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) { }

        //! Constructor
        //! @param[in]      db    DgnDb in which the style is to reside
        //! @param[in]      name  The name of the style. Must be unique within the DgnDb
        //! @param[in]      data  Style properties
        //! @param[in]      descr Optional style description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name="", TextAnnotationSeedPropertyBagCR data=TextAnnotationSeedPropertyBag(), Utf8StringCR descr="");
    };
private:
    friend struct TextAnnotationSeedPersistence;

    TextAnnotationSeedPropertyBag m_data;
    Utf8String m_descr;

    void Reset();
    void ResetProperties();
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT virtual uint32_t _GetMemSize() const override;

    virtual Code _GenerateDefaultCode() override { return Code(); }
public:
    explicit TextAnnotationSeed(DgnDbR db) : TextAnnotationSeed(CreateParams(db)) { }
    explicit TextAnnotationSeed(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) { }

    static TextAnnotationSeedPtr Create(DgnDbR project) { return new TextAnnotationSeed(project); }
    TextAnnotationSeedPtr Clone() const { return MakeCopy<TextAnnotationSeed>(); }

    DGNPLATFORM_EXPORT TextAnnotationSeedPtr CreateEffectiveStyle(TextAnnotationSeedPropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return GetDgnDb(); }
    TextAnnotationSeedId GetSeedId() const { return TextAnnotationSeedId(GetElementId().GetValueUnchecked()); }
    Utf8String GetName() const { return GetCode().GetValue(); }
    Utf8StringCR GetDescription() const { return m_descr; }
    void SetDescription(Utf8StringCR value) { m_descr = value; }
    void SetName(Utf8StringCR value) { SetCode(CreateCodeForSeed(value, GetDgnDb())); }

    DGNPLATFORM_EXPORT static Code CreateCodeForSeed(Utf8StringCR name, DgnDbR db);

    DGNPLATFORM_EXPORT AnnotationFrameStyleId GetFrameStyleId() const;
    DGNPLATFORM_EXPORT void SetFrameStyleId(AnnotationFrameStyleId);
    DGNPLATFORM_EXPORT AnnotationLeaderStyleId GetLeaderStyleId() const;
    DGNPLATFORM_EXPORT void SetLeaderStyleId(AnnotationLeaderStyleId);
    DGNPLATFORM_EXPORT AnnotationTextStyleId GetTextStyleId() const;
    DGNPLATFORM_EXPORT void SetTextStyleId(AnnotationTextStyleId);

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationSeed); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    TextAnnotationSeedCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<TextAnnotationSeed>(*this, status); }
    TextAnnotationSeedCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<TextAnnotationSeed>(*this, status); }

    DGNPLATFORM_EXPORT static TextAnnotationSeedId QuerySeedId(Code const& code, DgnDbR db);
    static TextAnnotationSeedId QuerySeedId(Utf8StringCR styleName, DgnDbR db) { return QuerySeedId(CreateCodeForSeed(styleName, db), db); }
    static TextAnnotationSeedCPtr QuerySeed(TextAnnotationSeedId styleId, DgnDbR db) { return db.Elements().Get<TextAnnotationSeed>(styleId); }
    static TextAnnotationSeedCPtr QuerySeed(Utf8StringCR styleName, DgnDbR db) { return QuerySeed(QuerySeedId(styleName, db), db); }

    DGNPLATFORM_EXPORT static bool ExistsById(TextAnnotationSeedId id, DgnDbR db);
    static bool ExistsByName(Utf8StringCR name, DgnDbR db) { return QuerySeedId(name, db).IsValid(); }

    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db);

    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct TextAnnotationSeed;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        TextAnnotationSeedId GetId() const { return m_statement->GetValueId<TextAnnotationSeedId>(0); }
        Utf8CP GetName() const { return m_statement->GetValueText(1); }
        Utf8CP GetDescription() const { return m_statement->GetValueText(2); }
    };

    struct Iterator : ECSqlStatementIterator<Entry>
    {
    };

    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, bool ordered=false);
    static Iterator MakeOrderedIterator(DgnDbR db) { return MakeIterator(db, true); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for text annotation seeds
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct TextAnnotationSeedHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TextAnnotationSeed, TextAnnotationSeed, TextAnnotationSeedHandler, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
