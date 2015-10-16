//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderStyle.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/ElementHandler.h>
#include <DgnPlatform/DgnCore/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationLeaderStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! This enumerates all possible annotation leader line types.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationLeaderLineType
{
    None = 1,
    Straight = 2,
    Curved = 3
};

//=======================================================================================
//! This enumerates all possible annotation leader terminator types.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationLeaderTerminatorType
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
enum class AnnotationLeaderStyleProperty
{
    LineColorType = 1, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    LineColorValue = 2, //!< (integer) @note int64_t representation of ElementColor
    LineType = 3, //!< (integer) @note Must exist in the AnnotationLeaderLineType enumeration
    LineWeight = 4, //!< (integer) @note Must be a standard line weight
    TerminatorColorType = 5, //!< (integer) @note Must exist in the AnnotationColorType enumeration
    TerminatorColorValue = 6, //!< (integer) @note int64_t representation of ElementColor
    TerminatorScaleFactor = 7, //!< (real) @note Generally describes the length of the side of the box encompassing the terminator, as a factor of the first character's text height
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
//! @note When creating an AnnotationLeader, the typical work flow is to create and store the style, and then create the AnnotationLeader with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationLeaderStyle : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationLeaderStyle, DictionaryElement);
public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(AnnotationLeaderStyle::T_Super::CreateParams);

        AnnotationLeaderStylePropertyBag m_data;
        Utf8String m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) { }

        //! Constructor
        //! @param[in]      db    DgnDb in which the style is to reside
        //! @param[in]      name  The name of the style. Must be unique within the DgnDb
        //! @param[in]      data  Style properties
        //! @param[in]      descr Optional style description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name="", AnnotationLeaderStylePropertyBagCR data=AnnotationLeaderStylePropertyBag(), Utf8StringCR descr="");
    };
private:
    friend struct AnnotationLeaderStylePersistence;

    AnnotationLeaderStylePropertyBag m_data;
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
    explicit AnnotationLeaderStyle(DgnDbR db) : AnnotationLeaderStyle(CreateParams(db)) { }
    explicit AnnotationLeaderStyle(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) { }

    static AnnotationLeaderStylePtr Create(DgnDbR project) { return new AnnotationLeaderStyle(project); }
    AnnotationLeaderStylePtr Clone() const { return MakeCopy<AnnotationLeaderStyle>(); }

    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return GetDgnDb(); }
    AnnotationLeaderStyleId GetStyleId() const { return AnnotationLeaderStyleId(GetElementId().GetValueUnchecked()); }
    Utf8String GetName() const { return GetCode().GetValue(); }
    Utf8StringCR GetDescription() const { return m_descr; }
    void SetDescription(Utf8StringCR value) { m_descr = value; }
    void SetName(Utf8StringCR value) { SetCode(CreateStyleCode(value, GetDgnDb())); }

    DGNPLATFORM_EXPORT static Code CreateStyleCode(Utf8StringCR name, DgnDbR db);

    DGNPLATFORM_EXPORT AnnotationColorType GetLineColorType() const;
    DGNPLATFORM_EXPORT void SetLineColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetLineColorValue() const;
    DGNPLATFORM_EXPORT void SetLineColorValue(ColorDef);
    DGNPLATFORM_EXPORT AnnotationLeaderLineType GetLineType() const;
    DGNPLATFORM_EXPORT void SetLineType(AnnotationLeaderLineType);
    DGNPLATFORM_EXPORT uint32_t GetLineWeight() const;
    DGNPLATFORM_EXPORT void SetLineWeight(uint32_t);
    DGNPLATFORM_EXPORT AnnotationColorType GetTerminatorColorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorColorType(AnnotationColorType);
    DGNPLATFORM_EXPORT ColorDef GetTerminatorColorValue() const;
    DGNPLATFORM_EXPORT void SetTerminatorColorValue(ColorDef);
    DGNPLATFORM_EXPORT double GetTerminatorScaleFactor() const;
    DGNPLATFORM_EXPORT void SetTerminatorScaleFactor(double);
    DGNPLATFORM_EXPORT AnnotationLeaderTerminatorType GetTerminatorType() const;
    DGNPLATFORM_EXPORT void SetTerminatorType(AnnotationLeaderTerminatorType);
    DGNPLATFORM_EXPORT uint32_t GetTerminatorWeight() const;
    DGNPLATFORM_EXPORT void SetTerminatorWeight(uint32_t);

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationLeaderStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    AnnotationLeaderStyleCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<AnnotationLeaderStyle>(*this, status); }
    AnnotationLeaderStyleCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<AnnotationLeaderStyle>(*this, status); }

    DGNPLATFORM_EXPORT static AnnotationLeaderStyleId QueryStyleId(Code const& code, DgnDbR db);
    static AnnotationLeaderStyleId QueryStyleId(Utf8StringCR styleName, DgnDbR db) { return QueryStyleId(CreateStyleCode(styleName, db), db); }
    static AnnotationLeaderStyleCPtr QueryStyle(AnnotationLeaderStyleId styleId, DgnDbR db) { return db.Elements().Get<AnnotationLeaderStyle>(styleId); }
    static AnnotationLeaderStyleCPtr QueryStyle(Utf8StringCR styleName, DgnDbR db) { return QueryStyle(QueryStyleId(styleName, db), db); }

    DGNPLATFORM_EXPORT static bool ExistsById(AnnotationLeaderStyleId id, DgnDbR db);
    static bool ExistsByName(Utf8StringCR name, DgnDbR db) { return QueryStyleId(name, db).IsValid(); }

    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db);

    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationLeaderStyle;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        AnnotationLeaderStyleId GetId() const { return m_statement->GetValueId<AnnotationLeaderStyleId>(0); }
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
    //! The handler for annotation leader styles
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct AnnotationLeaderStyleHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationLeaderStyle, AnnotationLeaderStyle, AnnotationLeaderStyleHandler, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
