//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/AnnotationTable.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <ECDb/ECSqlStatement.h>

DGNPLATFORM_TYPEDEFS(AnnotationTableElement);
DGNPLATFORM_TYPEDEFS(AnnotationTableAspect);
DGNPLATFORM_TYPEDEFS(AnnotationTableRow);
DGNPLATFORM_TYPEDEFS(AnnotationTableColumn);
DGNPLATFORM_TYPEDEFS(TableCellMarginValues);
DGNPLATFORM_TYPEDEFS(PropertyNames);

DGNPLATFORM_REF_COUNTED_PTR(AnnotationTableElement);

#define DGN_CLASSNAME_AnnotationTableElement "AnnotationTableElement"
#define DGN_CLASSNAME_AnnotationTableHeader  "AnnotationTableHeader"
#define DGN_CLASSNAME_AnnotationTableRow     "AnnotationTableRow"
#define DGN_CLASSNAME_AnnotationTableColumn  "AnnotationTableColumn"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

/*=================================================================================**//**
* This enum represents the distinct regions within a TextTable.  A table can hold
* a unique TextStyle for each region that will be used when adding text to an empty
* cell.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class AnnotationTableRegion
    {
    Body            = 0,
    TitleRow        = 1,
    HeaderRow       = 2,
    FooterRow       = 3,
    HeaderColumn    = 4,
    FooterColumn    = 5,
    };

/*=================================================================================**//**
* Used by TextTable::InsertRow and TextTable::InsertColumn.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableInsertDirection
    {
    Before          = 0,
    After           = 1,
    };

/*=================================================================================**//**
* Describes how a table is broken up to be displayed as sub tables.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableBreakType
    {
    None            = 0,
    Horizontal      = 1,
    Vertical        = 2,
    };

/*=================================================================================**//**
* Describes how each sub table is positioned relative to the previous sub table.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableBreakPosition
    {
    Right           = 0,
    Left            = 1,
    Above           = 2,
    Below           = 3,
    Manual          = 4,
    };

/*=================================================================================**//**
* Describes how text will be positioned within a table cell.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableCellAlignment
    {
    LeftTop         = 0,
    LeftMiddle      = 1,
    LeftBottom      = 2,
    CenterTop       = 3,
    CenterMiddle    = 4,
    CenterBottom    = 5,
    RightTop        = 6,
    RightMiddle     = 7,
    RightBottom     = 8,
    };

/*=================================================================================**//**
* Describes how text within a cell will be rotated relative to the table.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableCellOrientation
    {
    Horizontal      = 0,
    Rotate90        = 1,
    Rotate270       = 2,
    Vertical        = 3,
    };

/*=================================================================================**//**
* Describes the type of a table row or column.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableHeaderFooterType
    {
    Title           = 0,
    Header          = 1,
    Body            = 2,
    Footer          = 3,
    };

/*=================================================================================**//**
* Describes the various edge lines that surround a group of one or more table cells.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum TableCellListEdges
    {
    Top                   = 0,
    Bottom                = 1,
    Left                  = 2,
    Right                 = 3,
    Interior              = 4,
    Exterior              = 5,
    InteriorHorizontal    = 6,
    InteriorVertical      = 7,
    All                   = 50,
    };

/*=================================================================================**//**
* Used to specify the default fill of a table row.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class TableRows
    {
    Odd             = 0,
    Even            = 1,
    All             = 50,
    };

/*=================================================================================**//**
* Used to specify the minimum gap between a cell's contents and its edges.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TableCellMarginValues
{
double  m_top;      ///<    Minimum distance between the top edge of a cell and its contents.
double  m_bottom;   ///<    Minimum distance between the bottom edge of a cell and its contents.
double  m_left;     ///<    Minimum distance between the left edge of a cell and its contents.
double  m_right;    ///<    Minimum distance between the right edge of a cell and its contents.
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass
//=======================================================================================
struct PropertyDescr
    {
    int     m_propIndex;
    Utf8CP  m_propName;
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PropertyNames : bvector <Utf8String>
{
    // This serves to ensure that the property name strings are correlated
    // to the PropIndex values.  In the SELECT statement, PropIndices are 
    // directly correlated to column indices.  So it's important that the
    // first name is propIdex=0, etc. and that no gaps exist.
    PropertyNames& operator=(std::initializer_list<PropertyDescr> iList)
        {
        for (PropertyDescr const& item : iList)
            {
            BeAssert (item.m_propIndex == size());
            push_back (item.m_propName);
            }
        return *this;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          AnnotationTableSerializer
    {
private:
    AnnotationTableElementR             m_table;
/*
    bset<UInt32>                        m_usedSymbologyKeys;
    bset<UInt32>                        m_usedFillKeys;
    bool                                m_cacheXAttributes;
    bool                                m_updateRange;
    bool                                m_cleanedElement;
    bool                                m_deferStandalones;
    bvector<TextTableInstanceHolderP>   m_deferredHolders;
*/
    BentleyStatus   SerializeAspectChanges (AnnotationTableAspectR);
/*
    bool            ScheduleEdgeRuns (EdgeRunsR edgeRuns);
    void            DeleteAllPrivateInstances ();
    BentleyStatus   ScheduleDeferredInstances ();
*/
public:
    AnnotationTableSerializer (AnnotationTableElementR table/*, bool isDynamics, bool updateRange*/) : m_table(table) {};

    AnnotationTableElementR  GetElement () { return m_table; }

    DgnDbStatus       SerializeTableToDb ();
    };

//__PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass
//=======================================================================================
template<class T_Primitive> class TableValue
{
private:
    bool            m_isValid;
    T_Primitive     m_value;

protected:
    void    SetValid () { m_isValid = true; }

public:
    TableValue () { m_isValid = false; }
    TableValue (T_Primitive v) { m_isValid = true; m_value = v; }
    TableValue (TableValue& rhs) { m_isValid = rhs.m_isValid; m_value = rhs.m_value; } 

    void        Clear () { m_isValid = false; }
    bool        IsValid () const { return m_isValid; }
    bool        IsNull () const { return ! m_isValid; }
    void        SetValue (T_Primitive v) { m_value = v; SetValid(); }
    T_Primitive GetValue () const { return IsValid() ? m_value : 0; }
};

typedef TableValue<int>         TableIntValue;
typedef TableValue<uint64_t>    TableUInt64Value;
typedef TableValue<double>      TableDoubleValue;
typedef TableValue<bool>        TableBoolValue;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationTableAspect
{
private:
    bool                        m_hasChanges;
    AnnotationTableElementR     m_table;

protected:
    /*ctor*/        AnnotationTableAspect (AnnotationTableElementR t) : m_table (t), m_hasChanges(false) {}
    /*ctor*/        AnnotationTableAspect (AnnotationTableAspectCR rhs) : m_table (rhs.m_table) { m_hasChanges = rhs.m_hasChanges; }

    static BeSQLite::EC::CachedECSqlStatementPtr GetPreparedSelectStatement
        (
        Utf8StringR                 sqlString,
        bvector<Utf8String> const&  properties,
        Utf8CP                      className,
        bool                        isUniqueAspect,
        AnnotationTableElementCR    table
        );

    bool BindIfNull  (ECSqlStatement&, Utf8CP paramName, bool isNull);

    void BindInt     (ECSqlStatement&, Utf8CP paramName, TableIntValue const&);
    void BindInt64   (ECSqlStatement&, Utf8CP paramName, TableUInt64Value const&);
    void BindBool    (ECSqlStatement&, Utf8CP paramName, TableBoolValue const&);
    void BindDouble  (ECSqlStatement&, Utf8CP paramName, TableDoubleValue const&);

    void    SetHasChanges ()    { m_hasChanges = true;  }
    void    ClearHasChanges ()  { m_hasChanges = false; }

    virtual void            _AssignValue (int index, BeSQLite::EC::IECSqlValue const&) = 0;

    // To support insert, update, delete.
    virtual Utf8StringR     _GetECSqlInsertStringBuffer() = 0;
    virtual Utf8StringR     _GetECSqlUpdateStringBuffer() = 0;
    virtual Utf8StringR     _GetECSqlDeleteStringBuffer() = 0;
    virtual Utf8CP          _GetECClassName() = 0;
    virtual bvector<Utf8String> const& _GetPropertyNames() = 0;
    virtual void            _BindIdProperties (ECSqlStatement&) = 0;
    virtual void            _BindAllProperties (ECSqlStatement&) = 0;

public:
    bool    HasChanges () const { return m_hasChanges; }

    virtual bool            _IsUniqueAspect () const { return false; }
    virtual bool            _IsRequiredOnElement () const { return false; }
    virtual bool            _ShouldBePersisted ()   const { return true; }
    virtual void            _FlushChangesToProperties() {}

    BentleyStatus           InsertInDb();
    BentleyStatus           UpdateInDb();
    BentleyStatus           DeleteFromDb();

    void                    AssignProperties (ECSqlStatement const& statement);

    AnnotationTableElementR     GetTable ()       { return m_table; }
    AnnotationTableElementCR    GetTable () const { return m_table; }

};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationTableRow : AnnotationTableAspect
{
friend AnnotationTableElement;

private:
    int                     m_index;
    TableBoolValue          m_heightLock;
    TableDoubleValue        m_height;

//__PUBLISH_SECTION_END__
    enum class PropIndex
        {
        RowIndex    = 0,    // must be first
        HeightLock  = 1,    // no gaps allowed
        Height      = 2,
        };

    static bvector<Utf8String> const&      GetPropertyNames();

    static CachedECSqlStatementPtr  GetPreparedSelectStatement (AnnotationTableElementCR);
//__PUBLISH_SECTION_START__
protected:
    // AnnotationTableAspect
    virtual Utf8StringR                 _GetECSqlInsertStringBuffer();
    virtual Utf8StringR                 _GetECSqlUpdateStringBuffer();
    virtual Utf8StringR                 _GetECSqlDeleteStringBuffer();
    virtual Utf8CP                      _GetECClassName();
    virtual bvector<Utf8String> const&  _GetPropertyNames();
    virtual void                        _BindIdProperties (ECSqlStatement&);
    virtual void                        _BindAllProperties(ECSqlStatement&);
    virtual void                        _AssignValue (int, BeSQLite::EC::IECSqlValue const&);
    virtual void                        _FlushChangesToProperties() override;
    virtual bool                        _ShouldBePersisted() const override;

public:
    /*ctor*/        AnnotationTableRow (AnnotationTableElementR, int index);
    /*ctor*/        AnnotationTableRow (AnnotationTableRowCR);
    void            CopyDataFrom (AnnotationTableRowCR);

    AnnotationTableRowR operator= (AnnotationTableRowCR rhs);

//! Get the unique index that represents this row.
DGNPLATFORM_EXPORT  int                         GetIndex         () const { return m_index; }
//! Get the value of this row's height lock.  This setting controls how the row will react when contents of the table cells change size.
//! When the lock is false, the row's height will grow or shrink based on the content.  When the lock is true, the row's height will never
//! shrink as the content shrinks although it may grow but only if the content grows too large to fit.
DGNPLATFORM_EXPORT  bool                        GetHeightLock    () const;
//! Get the height of this row.
DGNPLATFORM_EXPORT  double                      GetHeight        () const;

//! Change the height of this row.  The row height cannot be made smaller than the height of the largest content including margins.
//! The height lock will be set to true for this row.  See AnnotationTableRow::GetHeightLock.
DGNPLATFORM_EXPORT  void                        SetHeight        (double);

};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationTableColumn : AnnotationTableAspect
{
friend AnnotationTableElement;

private:
    int                     m_index;
    TableBoolValue          m_widthLock;
    TableDoubleValue        m_width;

//__PUBLISH_SECTION_END__
    enum class PropIndex
        {
        ColumnIndex = 0,    // must be first
        WidthLock   = 1,    // no gaps allowed
        Width       = 2,
        };

    static bvector<Utf8String> const&      GetPropertyNames();

    static CachedECSqlStatementPtr  GetPreparedSelectStatement (AnnotationTableElementCR);
//__PUBLISH_SECTION_START__
protected:
    // AnnotationTableAspect
    virtual Utf8StringR                 _GetECSqlInsertStringBuffer();
    virtual Utf8StringR                 _GetECSqlUpdateStringBuffer();
    virtual Utf8StringR                 _GetECSqlDeleteStringBuffer();
    virtual Utf8CP                      _GetECClassName();
    virtual bvector<Utf8String> const&  _GetPropertyNames();
    virtual void                        _BindIdProperties (ECSqlStatement&);
    virtual void                        _BindAllProperties(ECSqlStatement&);
    virtual void                        _AssignValue (int, BeSQLite::EC::IECSqlValue const&);
    virtual void                        _FlushChangesToProperties() override;
    virtual bool                        _ShouldBePersisted() const override;

public:
    /*ctor*/        AnnotationTableColumn (AnnotationTableElementR, int index);
    /*ctor*/        AnnotationTableColumn (AnnotationTableColumnCR);
    void            CopyDataFrom (AnnotationTableColumnCR);

    AnnotationTableColumnR operator= (AnnotationTableColumnCR rhs);

//! Get the unique index that represents this column.
DGNPLATFORM_EXPORT  int                         GetIndex         () const { return m_index; }
//! Get the value of this column's width lock.  This setting controls how the column will react when contents of the table cells change size.
//! When the lock is false, the column's width will grow or shrink based on the content.  When the lock is true, the column's width will never
//! shrink as the content shrinks although it may grow but only if the content grows too large to fit.
DGNPLATFORM_EXPORT  bool                        GetWidthLock    () const;
//! Get the width of this column.
DGNPLATFORM_EXPORT  double                      GetWidth        () const;

//! Change the width of this column.  The column width cannot be made smaller than the width of the largest content including margins.
//! The width lock will be set to true for this column.  See AnnotationTableColumn::GetWidthLock.
DGNPLATFORM_EXPORT  void                        SetWidth        (double);

};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TableHeaderAspect : AnnotationTableAspect
{
friend AnnotationTableElement;

private:
    TableIntValue           m_rowCount;
    TableIntValue           m_columnCount;
    TableUInt64Value        m_textStyleId;
    int                     m_titleRowCount;
    int                     m_headerRowCount;
    int                     m_footerRowCount;
    int                     m_headerColumnCount;
    int                     m_footerColumnCount;
    int                     m_breakType;
    int                     m_breakPosition;
    TableDoubleValue        m_breakLength;
    TableDoubleValue        m_breakGap;
    TableBoolValue          m_repeatHeaders;
    TableBoolValue          m_repeatFooters;
    TableDoubleValue        m_defaultColumnWidth;
    TableDoubleValue        m_defaultRowHeight;
    TableDoubleValue        m_defaultMarginTop;
    TableDoubleValue        m_defaultMarginBottom;
    TableDoubleValue        m_defaultMarginLeft;
    TableDoubleValue        m_defaultMarginRight;
    int                     m_defaultCellAlignment;
    int                     m_fillSymbologyKeyOddRow;
    int                     m_fillSymbologyKeyEvenRow;
    TableUInt64Value        m_titleRowTextStyle;
    TableUInt64Value        m_headerRowTextStyle;
    TableUInt64Value        m_footerRowTextStyle;
    TableUInt64Value        m_headerColumnTextStyle;
    TableUInt64Value        m_footerColumnTextStyle;
    double                  m_backupTextHeight;
    int                     m_dataSourceProviderId;
    TableDoubleValue        m_bodyTextHeight;
    TableDoubleValue        m_titleRowTextHeight;
    TableDoubleValue        m_headerRowTextHeight;
    TableDoubleValue        m_footerRowTextHeight;
    TableDoubleValue        m_headerColumnTextHeight;
    TableDoubleValue        m_footerColumnTextHeight;

//__PUBLISH_SECTION_END__
    enum class PropIndex
        {
        RowCount                = 0,        // no gaps allowed
        ColumnCount             = 1, 
        TextStyleId             = 2,
        TitleRowCount           = 3,
        HeaderRowCount          = 4,
        FooterRowCount          = 5,
        HeaderColumnCount       = 6,
        FooterColumnCount       = 7,
        BreakType               = 8,
        BreakPosition           = 9,
        BreakLength             = 10,
        BreakGap                = 11,
        RepeatHeaders           = 12,
        RepeatFooters           = 13,
        DefaultColumnWidth      = 14,
        DefaultRowHeight        = 15,
        DefaultMarginTop        = 16,
        DefaultMarginBottom     = 17,
        DefaultMarginLeft       = 18,
        DefaultMarginRight      = 19,
        DefaultCellAlignment    = 20,
        FillSymbologyKeyOddRow  = 21,
        FillSymbologyKeyEvenRow = 22,
        TitleRowTextStyle       = 23,
        HeaderRowTextStyle      = 24,
        FooterRowTextStyle      = 25,
        HeaderColumnTextStyle   = 26,
        FooterColumnTextStyle   = 27,
        BackupTextHeight        = 28,
        DataSourceProviderId    = 29,
        BodyTextHeight          = 30,
        TitleRowTextHeight      = 31,
        HeaderRowTextHeight     = 32,
        FooterRowTextHeight     = 33,
        HeaderColumnTextHeight  = 34,
        FooterColumnTextHeight  = 35,
        };

    static bvector<Utf8String> const&      GetPropertyNames();

    static CachedECSqlStatementPtr  GetPreparedSelectStatement (AnnotationTableElementCR);

    TableHeaderAspect (AnnotationTableElementR);

    void        Invalidate ();
    void        CopyDataFrom (TableHeaderAspect const&);

    int         GetInteger (PropIndex) const;
    double      GetDouble  (PropIndex) const;
    AnnotationTextStyleId  GetStyleId (PropIndex) const;

    void    SetInteger (int, PropIndex);
    void    SetDouble (double, PropIndex);
    void    SetStyleId (AnnotationTextStyleId, PropIndex);

//__PUBLISH_SECTION_START__
protected:
    // AnnotationTableAspect
    virtual Utf8StringR                 _GetECSqlInsertStringBuffer();
    virtual Utf8StringR                 _GetECSqlUpdateStringBuffer();
    virtual Utf8StringR                 _GetECSqlDeleteStringBuffer();
    virtual Utf8CP                      _GetECClassName();
    virtual bvector<Utf8String> const&  _GetPropertyNames();
    virtual void                        _BindIdProperties (ECSqlStatement&);
    virtual void                        _BindAllProperties(ECSqlStatement&);
    virtual void                        _AssignValue (int propIndex, BeSQLite::EC::IECSqlValue const&) override;
    virtual bool                        _IsUniqueAspect () const override { return true; }
    virtual bool                        _IsRequiredOnElement () const override { return true; }

public:
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationTableElement : PhysicalElement
{
//__PUBLISH_SECTION_END__
friend AnnotationTableSerializer;
//__PUBLISH_SECTION_START__

private:

    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationTableElement, PhysicalElement) 

    TableHeaderAspect               m_tableHeader;
    bvector<AnnotationTableRow>     m_rows;
    bvector<AnnotationTableColumn>  m_columns;

    mutable bmap<AnnotationTableRegion, AnnotationTextStyleCPtr> m_textStyles;

    void                            UpdateGeometryRepresentation();
    DgnDbStatus                     SaveChanges();

    void                            Initialize (bool isNewTable);
    TableHeaderAspect&              GetHeaderAspect() { return m_tableHeader; }

    void                            SetTextStyleIdDirect (AnnotationTextStyleId val, AnnotationTableRegion region);
    AnnotationTextStyleCP           GetTextStyle (AnnotationTableRegion) const;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original) override;

    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;

    bvector<AnnotationTableRow>&          GetRowVectorR()             { return m_rows; }
    bvector<AnnotationTableRow> const&    GetRowVector() const        { return m_rows; }

    bvector<AnnotationTableColumn>&       GetColumnVectorR()          { return m_columns; }
    bvector<AnnotationTableColumn> const& GetColumnVector() const     { return m_columns; }

public:
    explicit AnnotationTableElement(CreateParams const& params);
    DGNPLATFORM_EXPORT static AnnotationTableElementPtr Create(CreateParams const& params);

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationTableElement)); }

    static AnnotationTableElementCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<AnnotationTableElement>(id); }
    static AnnotationTableElementPtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<AnnotationTableElement>(id); }
    AnnotationTableElementCPtr Insert() { return GetDgnDb().Elements().Insert<AnnotationTableElement>(*this); }
    AnnotationTableElementCPtr Update() { return GetDgnDb().Elements().Update<AnnotationTableElement>(*this); }
    DgnDbStatus Delete() { return GetDgnDb().Elements().Delete(*this); }

    bool        IsValid () const;

    void        SetRowCount     (int v);
    void        SetColumnCount  (int v);

    DGNPLATFORM_EXPORT int         GetRowCount      () const;
    DGNPLATFORM_EXPORT int         GetColumnCount   () const;
    DGNPLATFORM_EXPORT double      GetDefaultRowHeight () const;
    DGNPLATFORM_EXPORT double      GetDefaultColumnWidth () const;
    DGNPLATFORM_EXPORT AnnotationTextStyleId  GetTextStyleId   (AnnotationTableRegion) const;               //!<    Get the default text style which will be used to create text for empty cells.

    DGNPLATFORM_EXPORT void        SetTextStyleId           (AnnotationTextStyleId, AnnotationTableRegion); //!<    Change the default text style which will be used to create text for empty cells.
    DGNPLATFORM_EXPORT void        SetDefaultColumnWidth    (double);                                       //!<    Change the default column width in UORs which will be used by columns that don't have a specific width set.
    DGNPLATFORM_EXPORT void        SetDefaultRowHeight      (double);                                       //!<    Change the default row height in UORs which will be used by rows that don't have a specific height set.
    DGNPLATFORM_EXPORT void        SetDefaultMargins        (TableCellMarginValuesCR);                      //!<    Change the default cell margins in UORs which will be used by cells that don't have specific margins set.

    DGNPLATFORM_EXPORT  AnnotationTableRowCP        GetRow (int rowIndex) const;                            //!<    Get an object representing a row by its index.  Will return NULL if the index is out of range.
    DGNPLATFORM_EXPORT  AnnotationTableRowP         GetRow (int rowIndex);                                  //!<    Get an object representing a row by its index.  Will return NULL if the index is out of range.

    DGNPLATFORM_EXPORT  AnnotationTableColumnCP     GetColumn (int colIndex) const;                         //!<    Get an object representing a column by its index.  Will return NULL if the index is out of range.
    DGNPLATFORM_EXPORT  AnnotationTableColumnP      GetColumn (int colIndex);                               //!<    Get an object representing a column by its index.  Will return NULL if the index is out of range.

//! Create a new table from scratch.  All table properties will be initialized to safe default values.
//! @param rowCount         IN  The number of rows in the table.
//! @param columnCount      IN  The number of columns in the table.
//! @param textStyleId      IN  The default TextStyle.
//! @param backupTextHeight IN  Used when textStyleId refers to a style with zero text height.
//! @param params           IN  General element parameters
DGNPLATFORM_EXPORT  static  AnnotationTableElementPtr            Create(int rowCount, int columnCount, AnnotationTextStyleId textStyleId, double backupTextHeight, CreateParams const& params);


};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The ElementHandler for AnnotationTableElement
    //=======================================================================================
    struct AnnotationTableHandler : Physical
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationTableElement, AnnotationTableElement, AnnotationTableHandler, Physical, DGNPLATFORM_EXPORT);
    };
};



//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
