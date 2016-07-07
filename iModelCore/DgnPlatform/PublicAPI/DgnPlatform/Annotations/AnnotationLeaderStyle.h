//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationLeaderStyle.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

DGNPLATFORM_TYPEDEFS(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStylePropertyBag);
DGNPLATFORM_TYPEDEFS(AnnotationLeaderStyle);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderStyle);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible annotation leader line types.
//! @ingroup GROUP_Annotation
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
//! @ingroup GROUP_Annotation
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
//! @ingroup GROUP_Annotation
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
//! @ingroup GROUP_Annotation
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

//! As an element, AnnotationLeaderStyle IDs are inherently DgnElementId, but create a typedef so that argument types are more obvious/natural.
//! @ingroup GROUP_Annotation
typedef DgnElementId AnnotationLeaderStyleId;

//=======================================================================================
//! This is used to provide style properties when creating an AnnotationLeader.
//! @note When creating an AnnotationLeader, the typical work flow is to create and store the style, and then create the AnnotationLeader with the stored style's ID.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationLeaderStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AnnotationLeaderStyle, DefinitionElement);

private:
    friend struct AnnotationLeaderStylePersistence;

    Utf8String m_description;
    AnnotationLeaderStylePropertyBag m_data;

    static DgnCode CreateCodeFromName(Utf8StringCR name) { return ResourceAuthority::CreateResourceCode(name, BIS_CLASS_AnnotationLeaderStyle); }

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
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationLeaderStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }
    
    explicit AnnotationLeaderStyle(DgnDbR db) : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryDgnClassId(db), DgnCode())) {}
    explicit AnnotationLeaderStyle(CreateParams const& params) : T_Super(params) {}
    static AnnotationLeaderStylePtr Create(DgnDbR db) { return new AnnotationLeaderStyle(db); }
    AnnotationLeaderStylePtr CreateCopy() const { return MakeCopy<AnnotationLeaderStyle>(); }

    Utf8String GetName() const { return GetCode().GetValue(); }
    void SetName(Utf8CP value) { T_Super::_SetCode(CreateCodeFromName(value)); /* Only SetName is allowed to SetCode. */ }
    Utf8StringCR GetDescription() const { return m_description; }
    void SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }

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

    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle(AnnotationLeaderStylePropertyBagCR overrides) const;

    static DgnElementId QueryId(DgnDbR db, Utf8CP name) { return db.Elements().QueryElementIdByCode(CreateCodeFromName(name)); }
    static AnnotationLeaderStyleCPtr Get(DgnDbR db, Utf8CP name) { return Get(db, QueryId(db, name)); }
    static AnnotationLeaderStyleCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<AnnotationLeaderStyle>(id); }
    static AnnotationLeaderStylePtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<AnnotationLeaderStyle>(id); }
    static AnnotationLeaderStylePtr GetForEdit(DgnDbR db, Utf8CP name) { return GetForEdit(db, QueryId(db, name)); }
    AnnotationLeaderStyleCPtr Insert() { return GetDgnDb().Elements().Insert<AnnotationLeaderStyle>(*this); }
    AnnotationLeaderStyleCPtr Update() { return GetDgnDb().Elements().Update<AnnotationLeaderStyle>(*this); }

    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass                                                    Jeff.Marker     11/2014
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
        {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationLeaderStyle;

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
    //! The handler for annotation leader styles
    //=======================================================================================
    struct AnnotationLeaderStyleHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AnnotationLeaderStyle, AnnotationLeaderStyle, AnnotationLeaderStyleHandler, Element, DGNPLATFORM_EXPORT);

    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGN_NAMESPACE
