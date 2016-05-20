/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnTrueColor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"
#include "ECSqlStatementIterator.h"

DGNPLATFORM_TYPEDEFS(DgnTrueColor);
DGNPLATFORM_REF_COUNTED_PTR(DgnTrueColor);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A DgnTrueColor is a named color stored in a DgnDb. Named Colors are RGB values (no
//! transparency) organized by "color book" name and color name.  The entries in the table are identified by DgnTrueColorId's.
//! Once a True Color is defined, it may not be changed or deleted. Note that there may be multiple enties in the table with the same RGB value.
//! However, for a given book name, there may not be two entries with the same name.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnTrueColor : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_TrueColor, DefinitionElement);
public:
    //! Parameters used to construct a DgnTrueColor
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnTrueColor::T_Super::CreateParams);

        ColorDef m_colorDef;

        //! Constructor from base class. Chiefly for internal use.
        CreateParams(DgnElement::CreateParams const& params, ColorDef colorDef = ColorDef()) : T_Super(params), m_colorDef(colorDef) { }

        //! Constructor
        //! @param[in]      db       The DgnDb in which the color is to reside
        //! @param[in]      colorDef The color value
        //! @param[in]      name     The name of the color
        //! @param[in]      book     The name of the containing color book
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, ColorDef colorDef, Utf8StringCR name, Utf8StringCR book="");
    };

private:
    ColorDef m_colorDef;
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override { return DgnDbStatus::WrongElement; }
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    
    virtual DgnDbStatus _OnUpdate(DgnElementCR) override { return DgnDbStatus::WrongElement; }
    virtual DgnDbStatus _OnDelete() const override { return DgnDbStatus::DeletionProhibited; }
    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + static_cast<uint32_t>(sizeof(m_colorDef)); }
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override { return TrueColorAuthority::GetTrueColorAuthorityId() == auth.GetAuthorityId(); }
public:
    //! Construct a new DgnTrueColor with the specified parameters.
    explicit DgnTrueColor(CreateParams const& params) : T_Super(params), m_colorDef(params.m_colorDef) { }

    DgnTrueColorId GetColorId() const { return DgnTrueColorId(GetElementId().GetValueUnchecked()); } //!< The ID of this color
    Utf8String GetName() const { return GetCode().GetValue(); } //!< The name of this color
    Utf8String GetBook() const { return GetCode().GetNamespace(); } //!< The name of this color's color book
    ColorDef GetColorDef() const { return m_colorDef; } //!< The value of this color

    //! Creates a code for a color with the given name and book name
    static DgnCode CreateColorCode(Utf8StringCR name, Utf8StringCR book) { return TrueColorAuthority::CreateTrueColorCode(name, book); }

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TrueColor); } //!< The class ID associated with true colors within the given DgnDb
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< The class ID associated with true colors within the given DgnDb

    //! Inserts this color into the database and returns the persistent copy
    DgnTrueColorCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<DgnTrueColor>(*this, status); }

    //! Look up a color ID by DgnCode
    DGNPLATFORM_EXPORT static DgnTrueColorId QueryColorId(DgnCode const& code, DgnDbR db);

    //! Look up a color ID by name + book name
    static DgnTrueColorId QueryColorId(Utf8StringCR name, Utf8StringCR book, DgnDbR db) { return QueryColorId(CreateColorCode(name, book), db); }

    //! Find the first DgnTrueColorId that has a given ColorDef value.
    //! @return A DgnTrueColorId for the supplied color value. If no entry in the table has the given value, the DgnTrueColorId will be invalid.
    //! @note If the table holds more than one entry with the same value, it is undefined which DgnTrueColorId is returned.
    DGNPLATFORM_EXPORT static DgnTrueColorId FindMatchingColor(ColorDef colorDef, DgnDbR db);

    //! Look up a color by ID
    static DgnTrueColorCPtr QueryColor(DgnTrueColorId colorId, DgnDbR db) { return db.Elements().Get<DgnTrueColor>(colorId); }

    //! Look up a color by name + book name
    static DgnTrueColorCPtr QueryColorByName(Utf8StringCR name, Utf8StringCR book, DgnDbR db) { return QueryColor(QueryColorId(name, book, db), db); }

    //! Count the number of colors in the database
    //! @param[in]      db   The DgnDb in which to query
    //! @param[in]      book Optionally filter colors by a specific book name
    //! @return The number of colors in the DgnDb, optionally limited to a single color book.
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db, Utf8CP book=nullptr);

    //! An entry in a DgnTrueColor::Iterator
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct DgnTrueColor;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        DgnTrueColorId GetId() const { return m_statement->GetValueId<DgnTrueColorId>(0); } //!< The color ID
        Utf8CP GetName() const { return m_statement->GetValueText(1); } //!< The color name
        Utf8CP GetBook() const { return m_statement->GetValueText(2); } //!< The color book name
        DGNPLATFORM_EXPORT ColorDef GetColorDef() const; //!< The color value
    };

    //! An iterator over DgnTrueColors within a DgnDb
    struct Iterator : ECSqlStatementIterator<Entry>
    {

    };

    //! Create an iterator over true colors within a DgnDb
    //! @param[in]      db      The DgnDb in which to query
    //! @param[in]      book    If non-null, iteration is limited to colors with the given book name
    //! @param[in]      ordered If true, colors will be iterated in order by book name, then color name
    //! @return An iterator with the specified options.
    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, Utf8CP book=nullptr, bool ordered=false);

    //! Create an ordered iterator over true colors within a DgnDb
    //! @param[in]      db      The DgnDb in which to query
    //! @param[in]      book    If non-null, iteration is limited to colors with the given book name
    //! @return An iterator with the specified options, ordered by book name, then color name.
    static Iterator MakeOrderedIterator(DgnDbR db, Utf8CP book=nullptr) { return MakeIterator(db, book, true); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for named colors
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct TrueColor : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_TrueColor, DgnTrueColor, TrueColor, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

