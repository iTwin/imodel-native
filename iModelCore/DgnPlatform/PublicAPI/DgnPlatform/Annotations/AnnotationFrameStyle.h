/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/Annotations/AnnotationFrameStyle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationsCommon.h"
#include "AnnotationPropertyBag.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>

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
//! @note When creating an AnnotationFrame, the typical work flow is to create and store the style, and then create the AnnotationFrame with the stored style's ID.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationFrameStyle : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationFrameStyle, DictionaryElement);
public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(AnnotationFrameStyle::T_Super::CreateParams);

        AnnotationFrameStylePropertyBag m_data;
        Utf8String                      m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) { }

        //! Constructor
        //! @param[in]      db    DgnDb in which the style is to reside
        //! @param[in]      name  The name of the style. Must be unique within the DgnDb
        //! @param[in]      data  Style properties
        //! @param[in]      descr Optional style description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name="", AnnotationFrameStylePropertyBagCR data=AnnotationFrameStylePropertyBag(), Utf8StringCR descr="");
    };
private:
    friend struct AnnotationFrameStylePersistence;

    AnnotationFrameStylePropertyBag m_data;
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
    explicit AnnotationFrameStyle(DgnDbR db) : AnnotationFrameStyle(CreateParams(db)) { }
    explicit AnnotationFrameStyle(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) { }

    static AnnotationFrameStylePtr Create(DgnDbR db) { return new AnnotationFrameStyle(db); }
    AnnotationFrameStylePtr Clone() const { return MakeCopy<AnnotationFrameStyle>(); }

    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle(AnnotationFrameStylePropertyBagCR overrides) const;

    DgnDbR GetDbR() const { return GetDgnDb(); }
    AnnotationFrameStyleId GetStyleId() const { return AnnotationFrameStyleId(GetElementId().GetValueUnchecked()); }
    Utf8String GetName() const { return GetCode().GetValue(); }
    Utf8StringCR GetDescription() const { return m_descr; }
    void SetDescription(Utf8StringCR value) { m_descr = value; }
    void SetName(Utf8StringCR value) { SetCode(CreateStyleCode(value)); }

    DGNPLATFORM_EXPORT static Code CreateStyleCode(Utf8StringCR name);

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

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationFrameStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    AnnotationFrameStyleCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<AnnotationFrameStyle>(*this, status); }
    AnnotationFrameStyleCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<AnnotationFrameStyle>(*this, status); }

    DGNPLATFORM_EXPORT static AnnotationFrameStyleId QueryStyleId(Code const& code, DgnDbR db);
    static AnnotationFrameStyleId QueryStyleId(Utf8StringCR styleName, DgnDbR db) { return QueryStyleId(CreateStyleCode(styleName), db); }
    static AnnotationFrameStyleCPtr QueryStyle(AnnotationFrameStyleId styleId, DgnDbR db) { return db.Elements().Get<AnnotationFrameStyle>(styleId); }
    static AnnotationFrameStyleCPtr QueryStyle(Utf8StringCR styleName, DgnDbR db) { return QueryStyle(QueryStyleId(styleName, db), db); }

    DGNPLATFORM_EXPORT static bool ExistsById(AnnotationFrameStyleId id, DgnDbR db);
    static bool ExistsByName(Utf8StringCR name, DgnDbR db) { return QueryStyleId(name, db).IsValid(); }

    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db);

    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct AnnotationFrameStyle;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        AnnotationFrameStyleId GetId() const { return m_statement->GetValueId<AnnotationFrameStyleId>(0); }
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
    //! The handler for annotation frame styles
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct AnnotationFrameStyleHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationFrameStyle, AnnotationFrameStyle, AnnotationFrameStyleHandler, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
